/*
 *  Mount dialog
 *
 *  SPDX-FileCopyrightText: 2023-2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kmountdialog.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kcustomsettingsmanager.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4knotification.h"
#include "core/smb4kprofilemanager.h"
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
    // FIXME: Implement reload of custom settings if they were
    // changed externally (custon settings dialog or config dialog).

    setWindowTitle(i18n("Mount Dialog"));
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::South);

    //
    // 'Mounting' tab
    //
    QWidget *mountingWidget = new QWidget(m_tabWidget);
    QVBoxLayout *mountingWidgetLayout = new QVBoxLayout(mountingWidget);

    QWidget *descriptionWidget = new QWidget(mountingWidget);

    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("media-mount")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignVCenter);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    QLabel *descriptionText = new QLabel(mountingWidget);
    descriptionText->setText(i18n("Enter the location and optionally the IP address and workgroup to mount a share."));
    descriptionText->setWordWrap(true);
    descriptionText->setAlignment(Qt::AlignVCenter);
    descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionText);

    mountingWidgetLayout->addWidget(descriptionWidget);
    mountingWidgetLayout->addSpacing(mountingWidgetLayout->spacing());

    QWidget *inputWidget = new QWidget(mountingWidget);

    QGridLayout *inputWidgetLayout = new QGridLayout(inputWidget);
    inputWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *locationLabel = new QLabel(i18n("Location:"), inputWidget);
    m_locationInput = new KLineEdit(inputWidget);
    m_locationInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_locationInput->setClearButtonEnabled(true);

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

    mountingWidgetLayout->addWidget(inputWidget);
    mountingWidgetLayout->addStretch(100);

    m_tabWidget->addTab(mountingWidget, i18n("Mounting"));

    //
    // 'Bookmark' tab
    //
    m_bookmarkWidget = new QWidget(m_tabWidget);
    m_bookmarkWidget->setVisible(false);

    QVBoxLayout *bookmarkWidgetLayout = new QVBoxLayout(m_bookmarkWidget);

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

    bookmarkWidgetLayout->addWidget(m_bookmarkShare);
    bookmarkWidgetLayout->addWidget(m_bookmarkInputWidget);
    bookmarkWidgetLayout->addStretch(100);

    int tabIndex = m_tabWidget->addTab(m_bookmarkWidget, i18n("Bookmark"));
    m_tabWidget->setTabEnabled(tabIndex, false);

    //
    // 'Custom Settings' tab
    //
    m_customSettingsWidget = new Smb4KCustomSettingsEditorWidget(m_tabWidget);

    tabIndex = m_tabWidget->addTab(m_customSettingsWidget, i18n("Custom Settings"));
    m_tabWidget->setTabEnabled(tabIndex, false);

    connect(m_customSettingsWidget, &Smb4KCustomSettingsEditorWidget::edited, this, &Smb4KMountDialog::slotCustomSettingsEdited);

    layout->addWidget(m_tabWidget);

    //
    // Button box
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    m_okButton->setEnabled(false);

    connect(m_okButton, &QPushButton::clicked, this, &Smb4KMountDialog::slotAccepted);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KMountDialog::slotRejected);

    layout->addWidget(buttonBox);

    connect(Smb4KCustomSettingsManager::self(), &Smb4KCustomSettingsManager::updated, this, &Smb4KMountDialog::slotCustomSettingsUpdated);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    m_locationInput->setFocus();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("MountDialog"));
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), QStringLiteral("CompletionItems"));

    if (completionGroup.exists()) {
        m_locationInput->completionObject()->setItems(completionGroup.readEntry("LocationCompletion", QStringList()));
        m_ipAddressInput->completionObject()->setItems(completionGroup.readEntry("IpAddressCompletion", QStringList()));
        m_workgroupInput->completionObject()->setItems(completionGroup.readEntry("WorkgroupCompletion", QStringList()));
        m_bookmarkLabelInput->completionObject()->setItems(completionGroup.readEntry("LabelCompletion", QStringList()));
        m_bookmarkCategoryInput->completionObject()->setItems(completionGroup.readEntry("CategoryCompletion", Smb4KBookmarkHandler::self()->categoryList()));

        m_customSettingsWidget->loadCompletionItems();
    }
}

Smb4KMountDialog::~Smb4KMountDialog()
{
}

QUrl Smb4KMountDialog::createUrl(const QString &text) const
{
    QUrl url;
    QString userInfo, userInput = text;

    if (userInput.startsWith(QStringLiteral("\\"))) {
        userInput.replace(QStringLiteral("\\"), QStringLiteral("/"));
    }

    // If a UNC with user info is passed, QUrl seems to make wrong assuptions,
    // so just cut out the user info and set it later.
    if (userInput.startsWith(QStringLiteral("//")) && userInput.contains(QStringLiteral("@"))) {
        userInfo = userInput.section(QStringLiteral("@"), 0, -2).section(QStringLiteral("/"), 2, -1);
        userInput.remove(userInfo + QStringLiteral("@"));
    }

    url = QUrl::fromUserInput(userInput).adjusted(QUrl::StripTrailingSlash);
    url.setScheme(QStringLiteral("smb"));

    if (!userInfo.isEmpty()) {
        url.setUserInfo(userInfo);
    }

    return url;
}

bool Smb4KMountDialog::isValidLocation(const QString &text)
{
    QUrl url = createUrl(text);
    return (url.isValid() && !url.host().isEmpty() && !url.path().isEmpty() && url.path().length() != 1);
}

void Smb4KMountDialog::slotEnableButtons(const QString &text)
{
    Q_UNUSED(text);

    bool enable = isValidLocation(m_locationInput->userText());
    m_okButton->setEnabled(enable);

    int tabIndex = m_tabWidget->indexOf(m_bookmarkWidget);
    m_tabWidget->setTabEnabled(tabIndex, enable);

    tabIndex = m_tabWidget->indexOf(m_customSettingsWidget);
    m_tabWidget->setTabEnabled(tabIndex, enable);
}

void Smb4KMountDialog::slotEnableBookmarkInputWidget()
{
    m_bookmarkInputWidget->setEnabled(m_bookmarkShare->isChecked());
}

void Smb4KMountDialog::slotLocationEntered()
{
    if (!isValidLocation(m_locationInput->userText().trimmed())) {
        m_customSettingsWidget->clear();
        return;
    }

    m_locationInput->completionObject()->addItem(m_locationInput->userText().trimmed());

    QUrl url = createUrl(m_locationInput->userText());

    CustomSettingsPtr savedCustomSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(url);
    Smb4KCustomSettings customSettings;

    if (!savedCustomSettings) {
        Smb4KShare share;
        share.setUrl(url);
        customSettings = Smb4KCustomSettings(&share);
        customSettings.setProfile(Smb4KProfileManager::self()->activeProfile());
    } else {
        customSettings = *savedCustomSettings.data();

        m_ipAddressInput->setText(customSettings.ipAddress());
        m_ipAddressInput->completionObject()->addItem(customSettings.ipAddress());

        m_workgroupInput->setText(customSettings.workgroupName());
        m_workgroupInput->completionObject()->addItem(customSettings.workgroupName());
    }

    m_customSettingsWidget->setCustomSettings(customSettings);
}

void Smb4KMountDialog::slotIpAddressEntered()
{
    QHostAddress userInputIpAddress(m_ipAddressInput->userText().trimmed());

    if (userInputIpAddress.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol) {
        return;
    }

    m_ipAddressInput->completionObject()->addItem(userInputIpAddress.toString());

    Smb4KCustomSettings customSettings = m_customSettingsWidget->getCustomSettings();
    customSettings.setIpAddress(userInputIpAddress.toString());
    m_customSettingsWidget->setCustomSettings(customSettings);
}

void Smb4KMountDialog::slotWorkgroupEntered()
{
    QString userInputWorkgroupName = m_workgroupInput->userText().trimmed();

    if (userInputWorkgroupName.isEmpty()) {
        return;
    }

    m_workgroupInput->completionObject()->addItem(userInputWorkgroupName);

    Smb4KCustomSettings customSettings = m_customSettingsWidget->getCustomSettings();
    customSettings.setWorkgroupName(userInputWorkgroupName);
    m_customSettingsWidget->setCustomSettings(customSettings);
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

void Smb4KMountDialog::slotCustomSettingsEdited(bool changed)
{
    Q_UNUSED(changed);

    Smb4KCustomSettings customSettings = m_customSettingsWidget->getCustomSettings();

    // We do not need to take special care here for unwanted infinite
    // looping, because the QLineEdit::editingFinished signal is only
    // emitted after user input and not if the IP address or workgroup
    // name are changed programmatically.

    if (customSettings.ipAddress() != m_ipAddressInput->userText().trimmed()) {
        m_ipAddressInput->setText(customSettings.ipAddress());
    }

    if (customSettings.workgroupName() != m_workgroupInput->userText().trimmed()) {
        m_workgroupInput->setText(customSettings.workgroupName());
    }
}

void Smb4KMountDialog::slotCustomSettingsUpdated()
{
    if (!isValidLocation(m_locationInput->userText())) {
        return;
    }

    QUrl url = createUrl(m_locationInput->userText());
    CustomSettingsPtr savedCustomSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(url);

    if (savedCustomSettings) {
        Smb4KCustomSettings customSettings = m_customSettingsWidget->getCustomSettings();
        customSettings.update(savedCustomSettings.data());
        m_customSettingsWidget->setCustomSettings(customSettings);
    }
}

void Smb4KMountDialog::slotAccepted()
{
    // First, save the custom settings so they can be used for mounting.
    if (!m_customSettingsWidget->hasDefaultCustomSettings()) {
        Smb4KCustomSettings tempCustomSettings = m_customSettingsWidget->getCustomSettings();
        CustomSettingsPtr customSettings = CustomSettingsPtr(new Smb4KCustomSettings(tempCustomSettings));
        Smb4KCustomSettingsManager::self()->addCustomSettings(customSettings);
    }

    QUrl url = createUrl(m_locationInput->userText().trimmed());

    SharePtr share = SharePtr(new Smb4KShare());
    share->setUrl(url);

    BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark());
    bookmark->setUrl(url);

    QHostAddress userInputIpAddress(m_ipAddressInput->userText().trimmed());

    if (userInputIpAddress.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        share->setHostIpAddress(userInputIpAddress.toString());
        bookmark->setHostIpAddress(userInputIpAddress.toString());
    }

    QString userInputWorkgroupName = m_workgroupInput->userText().trimmed();

    if (!userInputWorkgroupName.isEmpty()) {
        share->setWorkgroupName(userInputWorkgroupName);
        bookmark->setWorkgroupName(userInputWorkgroupName);
    }

    if (m_bookmarkShare->isChecked()) {
        bookmark->setLabel(m_bookmarkLabelInput->userText());
        bookmark->setCategoryName(m_bookmarkCategoryInput->currentText());

        Smb4KBookmarkHandler::self()->addBookmark(bookmark);
    }

    Smb4KMounter::self()->mountShare(share);

    share.clear();
    bookmark.clear();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("MountDialog"));
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), QStringLiteral("CompletionItems"));
    completionGroup.writeEntry("LocationCompletion", m_locationInput->completionObject()->items());
    completionGroup.writeEntry("IpAddressCompletion", m_ipAddressInput->completionObject()->items());
    completionGroup.writeEntry("WorkgroupCompletion", m_workgroupInput->completionObject()->items());
    completionGroup.writeEntry("LabelCompletion", m_bookmarkLabelInput->completionObject()->items());
    completionGroup.writeEntry("CategoryCompletion", m_bookmarkCategoryInput->completionObject()->items());

    m_customSettingsWidget->saveCompletionItems();

    accept();
}

void Smb4KMountDialog::slotRejected()
{
    reject();
}
