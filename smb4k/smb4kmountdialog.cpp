/*
 *  Mount dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kmountdialog.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4knotification.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHostAddress>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KMountDialog::Smb4KMountDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Mount Dialog"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);

    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("media-mount")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignBottom);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    QLabel *descriptionText = new QLabel(this);
    descriptionText->setText(i18n("Enter the location and optionally the IP address and workgroup to mount a share."));
    descriptionText->setWordWrap(true);
    descriptionText->setAlignment(Qt::AlignBottom);
    descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionText);

    layout->addWidget(descriptionWidget);

    QWidget *inputWidget = new QWidget(this);

    QGridLayout *inputWidgetLayout = new QGridLayout(inputWidget);
    inputWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *locationLabel = new QLabel(i18n("Location:"), inputWidget);
    m_locationInput = new KLineEdit(inputWidget);
    m_locationInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_locationInput->setClearButtonEnabled(true);
    m_locationInput->setFocus();

    connect(m_locationInput, &KLineEdit::textChanged, this, &Smb4KMountDialog::slotEnableButtons);
    connect(m_locationInput, &KLineEdit::editingFinished, this, &Smb4KMountDialog::slotLocationEntered);

    QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), inputWidget);
    m_ipAddressInput = new KLineEdit(inputWidget);
    m_ipAddressInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_ipAddressInput->setClearButtonEnabled(true);

    connect(m_ipAddressInput, &KLineEdit::editingFinished, this, &Smb4KMountDialog::slotIpAddressEntered);

    QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), inputWidget);
    m_workgroupInput = new KLineEdit(inputWidget);
    m_workgroupInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_workgroupInput->setClearButtonEnabled(true);

    connect(m_workgroupInput, &KLineEdit::editingFinished, this, &Smb4KMountDialog::slotWorkgroupEntered);

    inputWidgetLayout->addWidget(locationLabel, 0, 0);
    inputWidgetLayout->addWidget(m_locationInput, 0, 1);
    inputWidgetLayout->addWidget(ipAddressLabel, 1, 0);
    inputWidgetLayout->addWidget(m_ipAddressInput, 1, 1);
    inputWidgetLayout->addWidget(workgroupLabel, 2, 0);
    inputWidgetLayout->addWidget(m_workgroupInput, 2, 1);

    layout->addWidget(inputWidget);

    m_bookmarkWidget = new QWidget(this);
    m_bookmarkWidget->setVisible(false);

    QVBoxLayout *bookmarkWidgetLayout = new QVBoxLayout(m_bookmarkWidget);
    bookmarkWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *horizontalLine = new QFrame(m_bookmarkWidget);
    horizontalLine->setFrameShape(QFrame::HLine);

    m_bookmarkShare = new QCheckBox(i18n("Bookmark this share"), m_bookmarkWidget);

    connect(m_bookmarkShare, &QCheckBox::clicked, this, &Smb4KMountDialog::slotEnableBookmarkInputWidget);

    m_bookmarkInputWidget = new QWidget(m_bookmarkWidget);
    m_bookmarkInputWidget->setEnabled(false);

    QGridLayout *bookmarkInputWidgetLayout = new QGridLayout(m_bookmarkInputWidget);
    bookmarkInputWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *bookmarkLabelLabel = new QLabel(i18n("Label:"), m_bookmarkInputWidget);
    m_bookmarkLabelInput = new KLineEdit(m_bookmarkInputWidget);
    m_bookmarkLabelInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_bookmarkLabelInput->setClearButtonEnabled(true);

    connect(m_bookmarkLabelInput, &KLineEdit::editingFinished, this, &Smb4KMountDialog::slotLabelEntered);

    QLabel *bookmarkCategoryLabel = new QLabel(i18n("Category:"), m_bookmarkInputWidget);
    m_bookmarkCategoryInput = new KComboBox(m_bookmarkInputWidget);
    m_bookmarkCategoryInput->setEditable(true);
    m_bookmarkCategoryInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_bookmarkCategoryInput->lineEdit()->setClearButtonEnabled(true);

    connect(m_bookmarkCategoryInput->lineEdit(), &QLineEdit::editingFinished, this, &Smb4KMountDialog::slotCategoryEntered);

    bookmarkInputWidgetLayout->addWidget(bookmarkLabelLabel, 0, 0);
    bookmarkInputWidgetLayout->addWidget(m_bookmarkLabelInput, 0, 1);
    bookmarkInputWidgetLayout->addWidget(bookmarkCategoryLabel, 1, 0);
    bookmarkInputWidgetLayout->addWidget(m_bookmarkCategoryInput, 1, 1);

    bookmarkWidgetLayout->addWidget(horizontalLine);
    bookmarkWidgetLayout->addWidget(m_bookmarkShare);
    bookmarkWidgetLayout->addWidget(m_bookmarkInputWidget);

    layout->addWidget(m_bookmarkWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_bookmarkButton = buttonBox->addButton(i18nc("Bookmark a share in the mount dialog.", "Bookmark >>"), QDialogButtonBox::ActionRole);

    m_okButton->setEnabled(false);
    m_bookmarkButton->setEnabled(false);

    connect(m_okButton, &QPushButton::clicked, this, &Smb4KMountDialog::slotAccepted);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KMountDialog::slotRejected);
    connect(m_bookmarkButton, &QPushButton::clicked, this, &Smb4KMountDialog::slotBookmarkButtonClicked);

    layout->addWidget(buttonBox);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();

        m_locationInput->completionObject()->setItems(group.readEntry("LocationCompletion", QStringList()));
        m_ipAddressInput->completionObject()->setItems(group.readEntry("IPAddressCompletion", QStringList()));
        m_workgroupInput->completionObject()->setItems(group.readEntry("WorkgroupCompletion", QStringList()));
        m_bookmarkLabelInput->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));
        m_bookmarkCategoryInput->completionObject()->setItems(group.readEntry("CategoryCompletion", Smb4KBookmarkHandler::self()->categoryList()));
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KMountDialog::~Smb4KMountDialog()
{
}

bool Smb4KMountDialog::isValidLocation(const QString &text)
{
    QString userInput = text;

    if (userInput.startsWith(QStringLiteral("\\"))) {
        userInput.replace(QStringLiteral("\\"), QStringLiteral("/"));
    }

    QUrl url = QUrl::fromUserInput(userInput).adjusted(QUrl::StripTrailingSlash);
    url.setScheme(QStringLiteral("smb"));

    return (url.isValid() && !url.host().isEmpty() && !url.path().isEmpty() && url.path().length() != 1);
}

void Smb4KMountDialog::adjustDialogSize()
{
    ensurePolished();
    layout()->activate();

    QSize dialogSize;
    dialogSize.setWidth(width());
    dialogSize.setHeight(height() - m_bookmarkWidget->height() - layout()->contentsMargins().bottom() - layout()->contentsMargins().top());

    resize(dialogSize);
}

void Smb4KMountDialog::slotEnableButtons(const QString &text)
{
    bool enable = isValidLocation(text);
    m_okButton->setEnabled(enable);
    m_bookmarkButton->setEnabled(enable);
}

void Smb4KMountDialog::slotBookmarkButtonClicked()
{
    m_bookmarkWidget->setVisible(!m_bookmarkWidget->isVisible());

    if (!m_bookmarkWidget->isVisible()) {
        adjustDialogSize();
    }
}

void Smb4KMountDialog::slotEnableBookmarkInputWidget()
{
    m_bookmarkInputWidget->setEnabled(m_bookmarkShare->isChecked());
}

void Smb4KMountDialog::slotLocationEntered()
{
    QString userInputUrl = m_locationInput->userText().trimmed();

    if (userInputUrl.startsWith(QStringLiteral("\\"))) {
        userInputUrl.replace(QStringLiteral("\\"), QStringLiteral("/"));
    }

    QUrl url = QUrl::fromUserInput(userInputUrl).adjusted(QUrl::StripTrailingSlash);
    url.setScheme(QStringLiteral("smb"));

    if (isValidLocation(userInputUrl)) {
        m_locationInput->completionObject()->addItem(userInputUrl);
    }
}

void Smb4KMountDialog::slotIpAddressEntered()
{
    QString userInputIpAddress = m_ipAddressInput->userText().trimmed();

    if (!userInputIpAddress.isEmpty()) {
        m_ipAddressInput->completionObject()->addItem(userInputIpAddress);
    }
}

void Smb4KMountDialog::slotWorkgroupEntered()
{
    QString userInputWorkgroup = m_workgroupInput->userText().trimmed();

    if (!userInputWorkgroup.isEmpty()) {
        m_workgroupInput->completionObject()->addItem(userInputWorkgroup);
    }
}

void Smb4KMountDialog::slotLabelEntered()
{
    QString userInputLabel = m_bookmarkLabelInput->userText().trimmed();

    if (!userInputLabel.isEmpty()) {
        m_bookmarkLabelInput->completionObject()->addItem(userInputLabel);
    }
}

void Smb4KMountDialog::slotCategoryEntered()
{
    QString userInputCategory = m_bookmarkCategoryInput->currentText();

    if (!userInputCategory.isEmpty()) {
        m_bookmarkCategoryInput->completionObject()->addItem(userInputCategory);
    }
}

void Smb4KMountDialog::slotAccepted()
{
    QString userInputUrl = m_locationInput->userText().trimmed();

    if (userInputUrl.startsWith(QStringLiteral("\\"))) {
        userInputUrl.replace(QStringLiteral("\\"), QStringLiteral("/"));
    }

    QUrl url = QUrl::fromUserInput(userInputUrl).adjusted(QUrl::StripTrailingSlash);
    url.setScheme(QStringLiteral("smb"));

    // This case might never happen, because the buttons are only
    // enabled when isValidLocation() returns TRUE, but we leave
    // this here for safety.
    if (!isValidLocation(userInputUrl)) {
        m_locationInput->setFocus();
        return;
    }

    SharePtr share = SharePtr(new Smb4KShare());
    share->setUrl(url);

    BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark());
    bookmark->setUrl(url);

    QHostAddress userIpAddressInput(m_ipAddressInput->userText().trimmed());

    if (userIpAddressInput.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        share->setHostIpAddress(userIpAddressInput.toString());
        bookmark->setHostIpAddress(userIpAddressInput.toString());
    }

    QString userInputWorkgroup = m_workgroupInput->userText().trimmed();

    if (!userInputWorkgroup.isEmpty()) {
        share->setWorkgroupName(userInputWorkgroup);
        bookmark->setWorkgroupName(userInputWorkgroup);
    }

    if (m_bookmarkShare->isChecked()) {
        bookmark->setLabel(m_bookmarkLabelInput->userText());
        bookmark->setCategoryName(m_bookmarkCategoryInput->currentText());

        Smb4KBookmarkHandler::self()->addBookmark(bookmark);
    }

    Smb4KMounter::self()->mountShare(share);

    share.clear();
    bookmark.clear();

    if (m_bookmarkWidget->isVisible()) {
        m_bookmarkInputWidget->setVisible(false);
        adjustDialogSize();
    }

    KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("LocationCompletion", m_locationInput->completionObject()->items());
    group.writeEntry("IPAddressCompletion", m_ipAddressInput->completionObject()->items());
    group.writeEntry("WorkgroupCompletion", m_workgroupInput->completionObject()->items());
    group.writeEntry("LabelCompletion", m_bookmarkLabelInput->completionObject()->items());
    group.writeEntry("CategoryCompletion", m_bookmarkCategoryInput->completionObject()->items());

    accept();
}

void Smb4KMountDialog::slotRejected()
{
    reject();
}
