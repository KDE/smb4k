/*
    This class handles the bookmarks.

    SPDX-FileCopyrightText: 2004-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmarkhandler.h"
#include "smb4kbookmark.h"
#include "smb4khost.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QDir>
#include <QFile>
#include <QMutableListIterator>
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

    read();

    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileRemoved, this, &Smb4KBookmarkHandler::slotProfileRemoved);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileMigrated, this, &Smb4KBookmarkHandler::slotProfileMigrated);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::activeProfileChanged, this, &Smb4KBookmarkHandler::slotActiveProfileChanged);
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

void Smb4KBookmarkHandler::addBookmark(const BookmarkPtr &bookmark)
{
    if (bookmark && add(bookmark)) {
        write();
        Q_EMIT updated();
    }
}

void Smb4KBookmarkHandler::addBookmarks(const QList<BookmarkPtr> &list, bool replace)
{
    if (replace) {
        QMutableListIterator<BookmarkPtr> it(d->bookmarks);

        while (it.hasNext()) {
            BookmarkPtr bookmark = it.next();

            if (!Smb4KSettings::useProfiles() || bookmark->profile() == Smb4KSettings::activeProfile()) {
                it.remove();
                bookmark.clear();
            }
        }
    }

    bool added = false;

    for (const BookmarkPtr &bookmark : list) {
        if (add(bookmark)) {
            added = true;
        }
    }

    if (added) {
        write();
        Q_EMIT updated();
    }
}

void Smb4KBookmarkHandler::removeBookmark(const BookmarkPtr &bookmark)
{
    if (bookmark && remove(bookmark)) {
        write();
        Q_EMIT updated();
    }
}

void Smb4KBookmarkHandler::removeCategory(const QString &name)
{
    if (!name.isEmpty() && remove(name)) {
        write();
        Q_EMIT updated();
    }
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
        if (b->label().toUpper() == label.toUpper()) {
            bookmark = b;
            break;
        }
    }

    return bookmark;
}

QList<BookmarkPtr> Smb4KBookmarkHandler::bookmarkList() const
{
    QList<BookmarkPtr> bookmarks;

    if (Smb4KSettings::useProfiles()) {
        for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
            if (bookmark->profile() == Smb4KProfileManager::self()->activeProfile()) {
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

bool Smb4KBookmarkHandler::add(const BookmarkPtr &bookmark)
{
    bool addedBookmark = false;

    if (findBookmarkByUrl(bookmark->url()).isNull()) {
        if (bookmark->profile().isEmpty()) {
            bookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
        }

        if (!bookmark->label().isEmpty() && findBookmarkByLabel(bookmark->label())) {
            Smb4KNotification::bookmarkLabelInUse(bookmark);
            bookmark->setLabel(bookmark->label() + QStringLiteral(" (1)"));
        }

        d->bookmarks << bookmark;
        addedBookmark = true;
    } else {
        Smb4KNotification::bookmarkExists(bookmark);
    }

    return addedBookmark;
}

bool Smb4KBookmarkHandler::remove(const BookmarkPtr &bookmark)
{
    bool removedBookmark = false;
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);

    while (it.hasNext()) {
        BookmarkPtr b = it.next();

        if ((!Smb4KSettings::useProfiles() || b->profile() == Smb4KProfileManager::self()->activeProfile())
            && QString::compare(bookmark->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                b->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                Qt::CaseInsensitive)
                == 0
            && bookmark->categoryName() == b->categoryName()) {
            it.remove();
            removedBookmark = true;
            b.clear();
        }
    }

    return removedBookmark;
}

bool Smb4KBookmarkHandler::remove(const QString &name)
{
    bool removedCategory = false;
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);

    while (it.hasNext()) {
        BookmarkPtr b = it.next();

        if ((!Smb4KSettings::useProfiles() || b->profile() == Smb4KProfileManager::self()->activeProfile())
            && b->categoryName() == name) {
            it.remove();
            removedCategory = true;
            b.clear();
        }
    }

    return removedCategory;
}

void Smb4KBookmarkHandler::read()
{
    while (!d->bookmarks.isEmpty()) {
        d->bookmarks.takeFirst().clear();
    }

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
                        BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark());
                        bookmark->setProfile(xmlReader.attributes().value(QStringLiteral("profile")).toString());
                        bookmark->setCategoryName(xmlReader.attributes().value(QStringLiteral("category")).toString());

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
}

void Smb4KBookmarkHandler::write()
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

void Smb4KBookmarkHandler::slotProfileRemoved(const QString &name)
{
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);

    while (it.hasNext()) {
        const BookmarkPtr &bookmark = it.next();

        if (name == bookmark->profile()) {
            it.remove();
        }
    }

    write();
    Q_EMIT updated();
}

void Smb4KBookmarkHandler::slotProfileMigrated(const QString &oldName, const QString &newName)
{
    for (const BookmarkPtr &bookmark : qAsConst(d->bookmarks)) {
        if (oldName == bookmark->profile()) {
            bookmark->setProfile(newName);
            continue;
        }
    }

    write();
    Q_EMIT updated();
}

void Smb4KBookmarkHandler::slotActiveProfileChanged(const QString &name)
{
    Q_UNUSED(name);
    Q_EMIT updated();
}
