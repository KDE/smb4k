/*
 *  smb4kbookmarkeditor  -  Bookmark editor
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// KDE includes
#include <KLocalizedString>
#include <KConfigGroup>
#include <KWindowConfig>

// Qt includes
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWindow>

// application specific includes
#include "smb4kbookmarkeditor.h"
#include "core/smb4ksettings.h"


Smb4KBookmarkEditor::Smb4KBookmarkEditor(QWidget* parent)
: QDialog(parent)
{
    setWindowTitle(i18n("Bookmark Editor"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_mainWidget = new Smb4KConfigPageBookmarks(this);

    layout->addWidget(m_mainWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    QPushButton *saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(saveButton, &QPushButton::clicked, this, &Smb4KBookmarkEditor::slotAccepted);
    connect(cancelButton, &QPushButton::clicked, this, &Smb4KBookmarkEditor::slotRejected);

    layout->addWidget(buttonBox);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
}

void Smb4KBookmarkEditor::slotAccepted()
{
    m_mainWidget->saveBookmarks();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    accept();
}

void Smb4KBookmarkEditor::slotRejected()
{
    reject();
}



