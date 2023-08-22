/*
 *  Bookmark editor
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kbookmarkeditor.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KWindowConfig>

Smb4KBookmarkEditor::Smb4KBookmarkEditor(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Bookmark Editor"));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_mainWidget = new Smb4KConfigPageBookmarks(this);

    connect(m_mainWidget, &Smb4KConfigPageBookmarks::bookmarksModified, this, &Smb4KBookmarkEditor::slotEnabledButtons);

    layout->addWidget(m_mainWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    m_saveButton->setEnabled(false);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_saveButton, &QPushButton::clicked, this, &Smb4KBookmarkEditor::slotAccepted);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KBookmarkEditor::slotRejected);

    layout->addWidget(buttonBox);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();

        QMap<QString, QStringList> completionItems;
        completionItems[QStringLiteral("CategoryCompletion")] = group.readEntry("CategoryCompletion", Smb4KBookmarkHandler::self()->categoryList());
        completionItems[QStringLiteral("LabelCompletion")] = group.readEntry("LabelCompletion", QStringList());

        // Since 3.3.0: For backward compatibility. Remove later.
        if (group.hasKey("IPCompletion")) {
            completionItems[QStringLiteral("IpAddressCompletion")] = group.readEntry("IPCompletion", QStringList());
            group.deleteEntry("IPCompletion");
        } else {
            completionItems[QStringLiteral("IpAddressCompletion")] = group.readEntry("IpAddressCompletion", QStringList());
        }

        completionItems[QStringLiteral("LoginCompletion")] = group.readEntry("LoginCompletion", QStringList());
        completionItems[QStringLiteral("WorkgroupCompletion")] = group.readEntry("WorkgroupCompletion", QStringList());

        m_mainWidget->setCompletionItems(completionItems);
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
}

void Smb4KBookmarkEditor::slotEnabledButtons()
{
    m_saveButton->setEnabled(m_mainWidget->bookmarksChanged());
}

void Smb4KBookmarkEditor::slotAccepted()
{
    m_mainWidget->saveBookmarks();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    QMap<QString, QStringList> completionItems = m_mainWidget->getCompletionItems();

    group.writeEntry("CategoryCompletion", completionItems[QStringLiteral("CategoryCompletion")]);
    group.writeEntry("LabelCompletion", completionItems[QStringLiteral("LabelCompletion")]);
    group.writeEntry("IpAddressCompletion", completionItems[QStringLiteral("IpAddressCompletion")]);
    group.writeEntry("LoginCompletion", completionItems[QStringLiteral("LoginCompletion")]);
    group.writeEntry("WorkgroupCompletion", completionItems[QStringLiteral("WorkgroupCompletion")]);

    accept();
}

void Smb4KBookmarkEditor::slotRejected()
{
    reject();
}
