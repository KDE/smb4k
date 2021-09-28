/*
    Private helper class(es) for the profile manager.
    -------------------
    begin                : Mi Aug 12 2014
    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kprofilemanager_p.h"
#include "smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

Smb4KProfileMigrationDialog::Smb4KProfileMigrationDialog(const QStringList &from, const QStringList &to, QWidget *parent)
    : QDialog(parent)
    , m_from_list(from)
    , m_to_list(to)
{
    //
    // Set the window title
    //
    setWindowTitle(i18n("Profile Migration Assistant"));

    //
    // Setup the view
    //
    setupView();

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "ProfileMigrationDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KProfileMigrationDialog::~Smb4KProfileMigrationDialog()
{
}

void Smb4KProfileMigrationDialog::setupView()
{
    //
    // The layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);

    //
    // The description
    //
    QWidget *description = new QWidget(this);
    QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
    descriptionLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *pixmap = new QLabel(description);
    QPixmap pix = KDE::icon("format-list-unordered").pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(pix);
    pixmap->setAlignment(Qt::AlignBottom);

    QLabel *label = new QLabel(i18n("Migrate all relevant settings of one profile to another."));
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignBottom);

    descriptionLayout->addWidget(pixmap, 0);
    descriptionLayout->addWidget(label, Qt::AlignBottom);

    //
    // The input widgets
    //
    QWidget *editors = new QWidget(this);
    QGridLayout *editorsLayout = new QGridLayout(editors);
    editorsLayout->setSpacing(5);
    editorsLayout->setContentsMargins(0, 0, 0, 0);
    editorsLayout->setColumnStretch(0, 0);
    editorsLayout->setColumnStretch(1, 1);

    QLabel *from = new QLabel(i18n("Old Profile:"), editors);
    editorsLayout->addWidget(from, 0, 0);

    m_from_box = new KComboBox(editors);

    if (m_from_list.size() == 1 && m_from_list.first().isEmpty()) {
        m_from_box->addItem(i18n("<Default Profile>"));
    } else {
        if (m_to_list.size() == 1 && m_to_list.first().isEmpty()) {
            m_from_box->addItem(i18n("<All Profiles>"));
        } else {
            m_from_box->addItems(m_from_list);
        }
    }

    editorsLayout->addWidget(m_from_box, 0, 1);

    QLabel *to = new QLabel(i18n("New Profile:"), editors);
    editorsLayout->addWidget(to, 1, 0);

    m_to_box = new KComboBox(editors);

    if (m_to_list.size() == 1 && m_to_list.first().isEmpty()) {
        m_to_box->addItem(i18n("<Default Profile>"));
    } else {
        m_to_box->addItems(m_to_list);
        m_to_box->setCurrentText(Smb4KProfileManager::self()->activeProfile());
    }

    editorsLayout->addWidget(m_to_box, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);

    m_ok_button->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_cancel_button->setShortcut(Qt::Key_Escape);

    m_ok_button->setDefault(true);
    m_ok_button->setEnabled(!m_to_box->currentText().isEmpty());

    layout->addWidget(description, 0);
    layout->addWidget(editors, 0);
    layout->addWidget(buttonBox, 0);

    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

QString Smb4KProfileMigrationDialog::from() const
{
    if (m_from_box->currentText() == i18n("<Default Profile>")) {
        return QString();
    }

    return m_from_box->currentText();
}

QString Smb4KProfileMigrationDialog::to() const
{
    if (m_to_box->currentText() == i18n("<Default Profile>")) {
        return QString();
    }

    return m_to_box->currentText();
}

void Smb4KProfileMigrationDialog::slotOkClicked()
{
    KConfigGroup group(Smb4KSettings::self()->config(), "ProfileMigrationDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    accept();
}
