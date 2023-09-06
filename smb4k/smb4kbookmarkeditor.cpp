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

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), "BookmarkEditor");
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");

    if (completionGroup.exists()) {
        QMap<QString, QStringList> completionItems;
        completionItems[QStringLiteral("CategoryCompletion")] = completionGroup.readEntry("CategoryCompletion", Smb4KBookmarkHandler::self()->categoryList());
        completionItems[QStringLiteral("LabelCompletion")] = completionGroup.readEntry("LabelCompletion", QStringList());
        completionItems[QStringLiteral("IpAddressCompletion")] = completionGroup.readEntry("IpAddressCompletion", QStringList());
        completionItems[QStringLiteral("LoginCompletion")] = completionGroup.readEntry("LoginCompletion", QStringList());
        completionItems[QStringLiteral("WorkgroupCompletion")] = completionGroup.readEntry("WorkgroupCompletion", QStringList());

        m_mainWidget->setCompletionItems(completionItems);
    }
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

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), "BookmarkEditor");
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");
    QMap<QString, QStringList> completionItems = m_mainWidget->completionItems();

    completionGroup.writeEntry("CategoryCompletion", completionItems[QStringLiteral("CategoryCompletion")]);
    completionGroup.writeEntry("LabelCompletion", completionItems[QStringLiteral("LabelCompletion")]);
    completionGroup.writeEntry("IpAddressCompletion", completionItems[QStringLiteral("IpAddressCompletion")]);
    completionGroup.writeEntry("LoginCompletion", completionItems[QStringLiteral("LoginCompletion")]);
    completionGroup.writeEntry("WorkgroupCompletion", completionItems[QStringLiteral("WorkgroupCompletion")]);

    accept();
}

void Smb4KBookmarkEditor::slotRejected()
{
    reject();
}
