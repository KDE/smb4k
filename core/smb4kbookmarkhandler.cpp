/*
    This class handles the bookmarks.

    SPDX-FileCopyrightText: 2004-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmarkhandler.h"
#include "smb4kbookmark.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4khost.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMutableListIterator>
#include <QPointer>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

class Smb4KBookmarkHandlerPrivate
{
public:
    QList<BookmarkPtr> bookmarks;
};

class Smb4KBookmarkHandlerStatic
{
public:
    Smb4KBookmarkHandler instance;
};

Q_GLOBAL_STATIC(Smb4KBookmarkHandlerStatic, p);

Smb4KBookmarkHandler::Smb4KBookmarkHandler(QObject *parent)
    : QObject(parent)
    , d(new Smb4KBookmarkHandlerPrivate)
{
    QString path = dataLocation();

    QDir dir;

    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    readBookmarkList();

    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileRemoved, this, &Smb4KBookmarkHandler::slotProfileRemoved);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileMigrated, this, &Smb4KBookmarkHandler::slotProfileMigrated);
}

Smb4KBookmarkHandler::~Smb4KBookmarkHandler()
{
    while (!d->bookmarks.isEmpty()) {
        d->bookmarks.takeFirst().clear();
    }
}

Smb4KBookmarkHandler *Smb4KBookmarkHandler::self()
{
    return &p->instance;
}

void Smb4KBookmarkHandler::addBookmark(const SharePtr &share)
{
    if (share) {
        QList<SharePtr> shares;
        shares << share;
        addBookmarks(shares);
    }
}

void Smb4KBookmarkHandler::addBookmark(const BookmarkPtr &bookmark)
{
    if (bookmark) {
        //
        // Create a list that will be passed to addBookmarks()
        //
        QList<BookmarkPtr> bookmarks;

        //
        // Check if the share has already been bookmarked and skip it if it
        // already exists
        //
        BookmarkPtr knownBookmark = findBookmarkByUrl(bookmark->url());

        if (knownBookmark) {
            Smb4KNotification::bookmarkExists(knownBookmark);
            return;
        }

        //
        // Copy the bookmark, add the correct profile (may be empty) and
        // add it to the list.
        //
        BookmarkPtr newBookmark = BookmarkPtr(bookmark);
        newBookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
        bookmarks << newBookmark;

        //
        // Add the bookmark
        //
        addBookmarks(bookmarks, false);
    }
}

void Smb4KBookmarkHandler::addBookmarks(const QList<SharePtr> &list)
{
    //
    // Prepare the list of bookmarks that should be added
    //
    QList<BookmarkPtr> newBookmarks;

    for (const SharePtr &share : list) {
        //
        // Printer shares cannot be bookmarked
        //
        if (share->isPrinter()) {
            Smb4KNotification::cannotBookmarkPrinter(share);
            continue;
        }

        //
        // Process homes shares
        //
        if (share->isHomesShare() && !Smb4KHomesSharesHandler::self()->specifyUser(share, true)) {
            continue;
        }

        //
        // Check if the share has already been bookmarked and skip it if it
        // already exists
        //
        BookmarkPtr knownBookmark = findBookmarkByUrl(share->isHomesShare() ? share->homeUrl() : share->url());

        if (knownBookmark) {
            Smb4KNotification::bookmarkExists(knownBookmark);
            continue;
        }

        BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark(share.data()));
        bookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
        newBookmarks << bookmark;
    }

    if (!newBookmarks.isEmpty()) {
        addBookmarks(newBookmarks, false);

        while (!newBookmarks.isEmpty()) {
            newBookmarks.takeFirst().clear();
        }
    }
}

void Smb4KBookmarkHandler::addBookmarks(const QList<BookmarkPtr> &list, bool replace)
{
    //
    // Process the incoming list.
    // In case the internal list should be replaced, clear the internal
    // list first.
    //
    if (replace) {
        QMutableListIterator<BookmarkPtr> it(d->bookmarks);

        while (it.hasNext()) {
            BookmarkPtr bookmark = it.next();
            removeBookmark(bookmark);
        }
    }

    //
    // Copy all bookmarks that are not in the list
    //
    for (const BookmarkPtr &bookmark : list) {
        //
        // Check if the bookmark label is already in use
        //
        if (!bookmark->label().isEmpty() && findBookmarkByLabel(bookmark->label())) {
            Smb4KNotification::bookmarkLabelInUse(bookmark);
            bookmark->setLabel(bookmark->label() + QStringLiteral(" (1)"));
        }

        //
        // Check if we have to add the bookmark
        //
        BookmarkPtr existingBookmark = findBookmarkByUrl(bookmark->url());

        if (!existingBookmark) {
            d->bookmarks << bookmark;
            Q_EMIT bookmarkAdded(bookmark);
        }
    }

    //
    // Save the bookmark list and emit the updated() signal
    //
    writeBookmarkList();
    Q_EMIT updated();
}

void Smb4KBookmarkHandler::removeBookmark(const BookmarkPtr &bookmark)
{
    if (bookmark) {
        for (int i = 0; i < d->bookmarks.size(); ++i) {
            if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == d->bookmarks.at(i)->profile())
                && QString::compare(d->bookmarks.at(i)->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    bookmark->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    Qt::CaseInsensitive)
                    == 0
                && QString::compare(bookmark->categoryName(), d->bookmarks.at(i)->categoryName(), Qt::CaseInsensitive) == 0) {
                BookmarkPtr bookmark = d->bookmarks.takeAt(i);
                Q_EMIT bookmarkRemoved(bookmark);
                bookmark.clear();
                break;
            }
        }

        // Write the list to the bookmarks file.
        writeBookmarkList();
        Q_EMIT updated();
    }
}

void Smb4KBookmarkHandler::removeCategory(const QString &name)
{
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);

    while (it.hasNext()) {
        const BookmarkPtr &bookmark = it.next();

        if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == bookmark->profile())
            || QString::compare(bookmark->categoryName(), name, Qt::CaseInsensitive) == 0) {
            Q_EMIT bookmarkRemoved(bookmark);
            it.remove();
        }
    }

    // Write the list to the bookmarks file.
    writeBookmarkList();
    Q_EMIT updated();
}

void Smb4KBookmarkHandler::writeBookmarkList()
{
    QFile xmlFile(dataLocation() + QDir::separator() + QStringLiteral("bookmarks.xml"));

    if (!d->bookmarks.isEmpty()) {
        if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&xmlFile);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement(QStringLiteral("bookmarks"));
            xmlWriter.writeAttribute(QStringLiteral("version"), QStringLiteral("3.1"));

            for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
                if (!bookmark->url().isValid()) {
                    Smb4KNotification::invalidURLPassed();
                    continue;
                }

                xmlWriter.writeStartElement(QStringLiteral("bookmark"));
                xmlWriter.writeAttribute(QStringLiteral("profile"), bookmark->profile());
                xmlWriter.writeAttribute(QStringLiteral("category"), bookmark->categoryName());

                xmlWriter.writeTextElement(QStringLiteral("workgroup"), bookmark->workgroupName());
                xmlWriter.writeTextElement(QStringLiteral("url"), bookmark->url().toString(QUrl::RemovePassword | QUrl::RemovePort));
                xmlWriter.writeTextElement(QStringLiteral("ip"), bookmark->hostIpAddress());
                xmlWriter.writeTextElement(QStringLiteral("label"), bookmark->label());

                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndDocument();

            xmlFile.close();
        } else {
            Smb4KNotification::openingFileFailed(xmlFile);
        }
    } else {
        xmlFile.remove();
    }
}

void Smb4KBookmarkHandler::readBookmarkList()
{
    //
    // Clear the list of bookmarks
    //
    while (!d->bookmarks.isEmpty()) {
        d->bookmarks.takeFirst().clear();
    }

    //
    // Locate the XML file and read the bookmarks
    //
    QFile xmlFile(dataLocation() + QDir::separator() + QStringLiteral("bookmarks.xml"));

    if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader xmlReader(&xmlFile);

        while (!xmlReader.atEnd()) {
            xmlReader.readNext();

            if (xmlReader.isStartElement()) {
                if (xmlReader.name() == QStringLiteral("bookmarks")
                    && (xmlReader.attributes().value(QStringLiteral("version")) != QStringLiteral("3.0")
                        && xmlReader.attributes().value(QStringLiteral("version")) != QStringLiteral("3.1"))) {
                    xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
                    break;
                } else {
                    if (xmlReader.name() == QStringLiteral("bookmark")) {
                        QString profile = xmlReader.attributes().value(QStringLiteral("profile")).toString();

                        BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark());
                        bookmark->setProfile(profile);

                        if (xmlReader.attributes().hasAttribute(QStringLiteral("group"))) {
                            // For backward compatibility (since Smb4K 3.0.72)
                            // TODO: Remove with Smb4K >> 3.1
                            bookmark->setCategoryName(xmlReader.attributes().value(QStringLiteral("group")).toString());
                        } else {
                            bookmark->setCategoryName(xmlReader.attributes().value(QStringLiteral("category")).toString());
                        }

                        while (!(xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("bookmark"))) {
                            xmlReader.readNext();

                            if (xmlReader.isStartElement()) {
                                if (xmlReader.name() == QStringLiteral("workgroup")) {
                                    bookmark->setWorkgroupName(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("url")) {
                                    bookmark->setUrl(QUrl(xmlReader.readElementText()));
                                } else if (xmlReader.name() == QStringLiteral("login")) {
                                    // For backward compatibility (since Smb4K 3.1.71)
                                    // TODO: Remove with Smb4K >> 3.2
                                    bookmark->setUserName(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("ip")) {
                                    bookmark->setHostIpAddress(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("label")) {
                                    bookmark->setLabel(xmlReader.readElementText());
                                }

                                continue;
                            } else {
                                continue;
                            }
                        }

                        d->bookmarks << bookmark;
                    } else {
                        continue;
                    }
                }
            } else {
                continue;
            }
        }

        xmlFile.close();

        if (xmlReader.hasError()) {
            Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
        }
    } else {
        if (xmlFile.exists()) {
            Smb4KNotification::openingFileFailed(xmlFile);
        }
    }

    Q_EMIT updated();
}

BookmarkPtr Smb4KBookmarkHandler::findBookmarkByUrl(const QUrl &url)
{
    BookmarkPtr bookmark;
    QList<BookmarkPtr> temporalBookmarkList = bookmarkList();

    if (!url.isEmpty() && url.isValid() && !temporalBookmarkList.isEmpty()) {
        for (const BookmarkPtr &b : qAsConst(temporalBookmarkList)) {
            // NOTE: Since also user provided URLs can be bookmarked, we cannot use
            // QUrl::matches() here, because it does not allow for case insensitive
            // comparison.
            if (QString::compare(url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 b->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                == 0) {
                bookmark = b;
                break;
            }
        }
    }

    return bookmark;
}

BookmarkPtr Smb4KBookmarkHandler::findBookmarkByLabel(const QString &label)
{
    BookmarkPtr bookmark;
    QList<BookmarkPtr> temporalBookmarkList = bookmarkList();

    for (const BookmarkPtr &b : qAsConst(temporalBookmarkList)) {
        if (QString::compare(b->label().toUpper(), label.toUpper()) == 0) {
            bookmark = b;
            break;
        }
    }

    return bookmark;
}

QList<BookmarkPtr> Smb4KBookmarkHandler::bookmarkList() const
{
    QList<BookmarkPtr> bookmarks;

    update();

    if (Smb4KSettings::useProfiles()) {
        for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
            if (bookmark->profile() == Smb4KSettings::activeProfile()) {
                bookmarks << bookmark;
            }
        }
    } else {
        bookmarks = d->bookmarks;
    }

    return bookmarks;
}

QList<BookmarkPtr> Smb4KBookmarkHandler::bookmarkList(const QString &categoryName) const
{
    QList<BookmarkPtr> bookmarks;
    QList<BookmarkPtr> temporalBookmarkList = bookmarkList();

    for (const BookmarkPtr &bookmark : qAsConst(temporalBookmarkList)) {
        if (categoryName == bookmark->categoryName()) {
            bookmarks << bookmark;
        }
    }

    return bookmarks;
}

QStringList Smb4KBookmarkHandler::categoryList() const
{
    QStringList categories;
    QList<BookmarkPtr> temporalBookmarkList = bookmarkList();

    for (const BookmarkPtr &bookmark : qAsConst(temporalBookmarkList)) {
        if (!categories.contains(bookmark->categoryName())) {
            categories << bookmark->categoryName();
        }
    }

    return categories;
}

bool Smb4KBookmarkHandler::isBookmarked(const SharePtr &share)
{
    if (findBookmarkByUrl(share->url())) {
        return true;
    }

    return false;
}

void Smb4KBookmarkHandler::update() const
{
    for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
        HostPtr host = findHost(bookmark->hostName(), bookmark->workgroupName());

        if (host) {
            if (host->hasIpAddress() && bookmark->hostIpAddress() != host->ipAddress()) {
                bookmark->setHostIpAddress(host->ipAddress());
            }
        }
    }
}

void Smb4KBookmarkHandler::slotProfileRemoved(const QString &name)
{
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);

    while (it.hasNext()) {
        const BookmarkPtr &bookmark = it.next();

        if (name == bookmark->profile()) {
            it.remove();
        }
    }

    // Write the new list to the file.
    writeBookmarkList();
}

void Smb4KBookmarkHandler::slotProfileMigrated(const QString &oldName, const QString &newName)
{
    for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
        if (oldName == bookmark->profile()) {
            bookmark->setProfile(newName);
            continue;
        }
    }

    // Write the new list to the file.
    writeBookmarkList();
}
