/*
 *  Editor widget for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023-2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kcustomsettingseditorwidget.h"
#include "smb4ksettings.h"
#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QValidator>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

class ModeValidator : public QValidator
{
public:
    ModeValidator(QObject *parent = nullptr)
        : QValidator(parent)
    {
    }
    ~ModeValidator()
    {
    }
    QValidator::State validate(QString &input, int &pos) const override
    {
        Q_UNUSED(pos);
        if (input.trimmed().size() == 4) {
            QChar ch = input.trimmed().at(0);

            if (ch.isDigit() && ch.toLatin1() != '0') {
                return QValidator::Invalid;
            } else if (ch.isDigit() && ch.toLatin1() == '0') {
                return QValidator::Acceptable;
            }
        }
        return QValidator::Intermediate;
    }
};

Smb4KCustomSettingsEditorWidget::Smb4KCustomSettingsEditorWidget(QWidget *parent)
    : QTabWidget(parent)
{
    m_hasDefaultCustomSettings = true;

    // FIXME: Honor disabled widgets and unchecked check boxes!?

    setupView();
}

Smb4KCustomSettingsEditorWidget::~Smb4KCustomSettingsEditorWidget()
{
}

#if defined(Q_OS_LINUX)
void Smb4KCustomSettingsEditorWidget::setupView()
{
    QWidget *tab1 = new QWidget(this);
    QGridLayout *tab1Layout = new QGridLayout(tab1);

    m_ipAddressLabel = new QLabel(i18n("IP Address:"), tab1);
    m_ipAddress = new KLineEdit(tab1);
    m_ipAddress->setClearButtonEnabled(true);
    // m_ipAddress->setInputMask(QStringLiteral("000.000.000.000"));
    m_ipAddressLabel->setBuddy(m_ipAddress);

    connect(m_ipAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotIpAddressChanged);

    m_workgroupLabel = new QLabel(i18n("Workgroup:"), tab1);
    m_workgroup = new KLineEdit(tab1);
    m_workgroup->setClearButtonEnabled(true);
    m_workgroupLabel->setBuddy(m_workgroup);

    connect(m_workgroup, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotWorkgroupNameChanged);

    m_alwaysRemountShare = new QCheckBox(i18n("Always remount this share"), tab1);
    m_alwaysRemountShare->setEnabled(false);

    connect(m_alwaysRemountShare, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotAlwaysRemoutShareToggled);

    tab1Layout->addWidget(m_ipAddressLabel, 0, 0);
    tab1Layout->addWidget(m_ipAddress, 0, 1);
    tab1Layout->addWidget(m_workgroupLabel, 1, 0);
    tab1Layout->addWidget(m_workgroup, 1, 1);
    tab1Layout->addWidget(m_alwaysRemountShare, 2, 0, 1, 2);
    tab1Layout->setRowStretch(3, 100);

    addTab(tab1, i18n("Basic Settings"));

    QWidget *tab2 = new QWidget(this);
    QGridLayout *tab2Layout = new QGridLayout(tab2);

    m_useWriteAccess = new QCheckBox(Smb4KMountSettings::self()->useWriteAccessItem()->label(), tab2);
    m_writeAccess = new KComboBox(tab2);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : std::as_const(writeAccessChoices)) {
        m_writeAccess->addItem(wa.label);
    }

    connect(m_useWriteAccess, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseWriteAccessToggled);
    connect(m_writeAccess, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotWriteAccessChanged);

    m_cifsUnixExtensionSupport = new QCheckBox(i18n("This server supports the CIFS Unix extensions"), tab2);

    connect(m_cifsUnixExtensionSupport, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotCifsUnixExtensionSupportToggled);

    m_useIds = new QCheckBox(Smb4KMountSettings::self()->useIdsItem()->label(), tab2);

    m_userIdLabel = new QLabel(i18n("User ID:"), tab2);
    m_userIdLabel->setIndent(25);
    m_userId = new KLineEdit(KUser(KUser::UseRealUserID).userId().toString(), tab2);
    m_userId->setAlignment(Qt::AlignRight);
    m_userId->setReadOnly(true);

    m_groupIdLabel = new QLabel(i18n("Group ID:"), tab2);
    m_groupIdLabel->setIndent(25);
    m_groupId = new KLineEdit(KUser(KUser::UseRealUserID).groupId().toString(), tab2);
    m_groupId->setAlignment(Qt::AlignRight);
    m_groupId->setReadOnly(true);

    connect(m_useIds, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseIdsToggled);

    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), tab2);
    m_fileMode = new KLineEdit(tab2);
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setAlignment(Qt::AlignRight);
    m_fileMode->setInputMask(QStringLiteral("0999"));
    m_fileMode->setValidator(new ModeValidator(m_fileMode));

    connect(m_useFileMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseFileModeToggled);
    connect(m_fileMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotFileModeChanged);

    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), tab2);
    m_directoryMode = new KLineEdit(tab2);
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setAlignment(Qt::AlignRight);
    m_directoryMode->setInputMask(QStringLiteral("0999"));
    m_directoryMode->setValidator(new ModeValidator(m_directoryMode));

    connect(m_useDirectoryMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseDirectoryModeToggled);
    connect(m_directoryMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotDirectoryModeChanged);

    tab2Layout->addWidget(m_useWriteAccess, 0, 0);
    tab2Layout->addWidget(m_writeAccess, 0, 1);
    tab2Layout->addWidget(m_cifsUnixExtensionSupport, 1, 0, 1, 2);
    tab2Layout->addWidget(m_useIds, 2, 0, 1, 2);
    tab2Layout->addWidget(m_userIdLabel, 3, 0);
    tab2Layout->addWidget(m_userId, 3, 1);
    tab2Layout->addWidget(m_groupIdLabel, 4, 0);
    tab2Layout->addWidget(m_groupId, 4, 1);
    tab2Layout->addWidget(m_useFileMode, 5, 0);
    tab2Layout->addWidget(m_fileMode, 5, 1);
    tab2Layout->addWidget(m_useDirectoryMode, 6, 0);
    tab2Layout->addWidget(m_directoryMode, 6, 1);
    tab2Layout->setRowStretch(6, 100);

    addTab(tab2, i18n("Common Mount Settings"));

    QWidget *tab3 = new QWidget(this);
    QGridLayout *tab3Layout = new QGridLayout(tab3);

    m_useSmbMountProtocolVersion = new QCheckBox(Smb4KMountSettings::self()->useSmbProtocolVersionItem()->label(), tab3);
    m_smbMountProtocolVersion = new KComboBox(tab3);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();

    for (int i = 0; i < smbProtocolChoices.size(); ++i) {
        m_smbMountProtocolVersion->addItem(smbProtocolChoices.at(i).label, i);
    }

    connect(m_useSmbMountProtocolVersion, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseSmbMountProtocolVersionToggled);
    connect(m_smbMountProtocolVersion, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotSmbMountProtocolVersionChanged);

    m_useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), tab3);
    m_securityMode = new KComboBox(tab3);

    QList<KConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();

    for (int i = 0; i < securityModeChoices.size(); ++i) {
        m_securityMode->addItem(securityModeChoices.at(i).label, i);
    }

    connect(m_useSecurityMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseSecurityModeToggled);
    connect(m_securityMode, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotSecurityModeChanged);

    tab3Layout->addWidget(m_useSmbMountProtocolVersion, 0, 0);
    tab3Layout->addWidget(m_smbMountProtocolVersion, 0, 1);
    tab3Layout->addWidget(m_useSecurityMode, 1, 0);
    tab3Layout->addWidget(m_securityMode, 1, 1);
    tab3Layout->setRowStretch(2, 100);

    addTab(tab3, i18n("Advanced Mount Settings"));

    QWidget *tab4 = new QWidget(this);
    QGridLayout *tab4Layout = new QGridLayout(tab4);

    m_useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), tab4);

    m_minimalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), tab4);
    m_minimalClientProtocolVersionLabel->setIndent(25);
    m_minimalClientProtocolVersionLabel->setEnabled(false);
    m_minimalClientProtocolVersion = new KComboBox(tab4);
    m_minimalClientProtocolVersion->setEnabled(false);
    m_minimalClientProtocolVersionLabel->setBuddy(m_minimalClientProtocolVersion);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (int i = 0; i < minimalClientProtocolVersionChoices.size(); ++i) {
        m_minimalClientProtocolVersion->addItem(minimalClientProtocolVersionChoices.at(i).label, i);
    }

    m_maximalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), tab4);
    m_maximalClientProtocolVersionLabel->setIndent(25);
    m_maximalClientProtocolVersionLabel->setEnabled(false);
    m_maximalClientProtocolVersion = new KComboBox(tab4);
    m_maximalClientProtocolVersion->setEnabled(false);
    m_maximalClientProtocolVersionLabel->setBuddy(m_maximalClientProtocolVersion);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (int i = 0; i < maximalClientProtocolVersionChoices.size(); ++i) {
        m_maximalClientProtocolVersion->addItem(maximalClientProtocolVersionChoices.at(i).label, i);
    }

    connect(m_useClientProtocolVersions, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseClientProtocolVersionsToggled);
    connect(m_minimalClientProtocolVersion, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotMinimalClientProtocolVersionChanged);
    connect(m_maximalClientProtocolVersion, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotMaximalClientProtocolVersionChanged);

    m_useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), tab4);

    connect(m_useKerberos, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled);

    tab4Layout->addWidget(m_useClientProtocolVersions, 0, 0, 1, 2);
    tab4Layout->addWidget(m_minimalClientProtocolVersionLabel, 1, 0);
    tab4Layout->addWidget(m_minimalClientProtocolVersion, 1, 1);
    tab4Layout->addWidget(m_maximalClientProtocolVersionLabel, 2, 0);
    tab4Layout->addWidget(m_maximalClientProtocolVersion, 2, 1);
    tab4Layout->addWidget(m_useKerberos, 3, 0, 1, 2);
    tab4Layout->setRowStretch(4, 100);

    addTab(tab4, i18n("Browse Settings"));

    QWidget *tab5 = new QWidget(this);
    QVBoxLayout *tab5Layout = new QVBoxLayout(tab5);

    tab5->setEnabled(Smb4KSettings::enableWakeOnLAN());

    QWidget *macAddressWidget = new QWidget(tab5);
    QHBoxLayout *macAddressWidgetLayout = new QHBoxLayout(macAddressWidget);
    macAddressWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressWidget);
    m_macAddress = new KLineEdit(macAddressWidget);
    m_macAddress->setClearButtonEnabled(true);
    m_macAddress->setInputMask(QStringLiteral("HH:HH:HH:HH:HH:HH;_")); // MAC address, see QLineEdit doc
    m_macAddressLabel->setBuddy(m_macAddress);
    m_macAddressSearchButton = new QPushButton(macAddressWidget);
    m_macAddressSearchButton->setIcon(KDE::icon(QStringLiteral("edit-find")));
    m_macAddressSearchButton->setToolTip(i18n("Find MAC address"));

    connect(m_macAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotMacAddressChanged);
    connect(m_macAddressSearchButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditorWidget::slotFindMacAddressClicked);

    macAddressWidgetLayout->addWidget(m_macAddressLabel);
    macAddressWidgetLayout->addWidget(m_macAddress);
    macAddressWidgetLayout->addWidget(m_macAddressSearchButton);

    m_sendPacketBeforeScan = new QCheckBox(i18n("Send magic packet before scanning the network neighborhood"), tab5);
    m_sendPacketBeforeScan->setEnabled(false);
    m_sendPacketBeforeMount = new QCheckBox(i18n("Send magic packet before mounting a share"), tab5);
    m_sendPacketBeforeMount->setEnabled(false);

    connect(m_sendPacketBeforeScan, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeScanToggled);
    connect(m_sendPacketBeforeMount, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeMountToggled);

    tab5Layout->addWidget(macAddressWidget);
    tab5Layout->addWidget(m_sendPacketBeforeScan);
    tab5Layout->addWidget(m_sendPacketBeforeMount);
    tab5Layout->addStretch(100);

    m_wakeOnLanTabIndex = addTab(tab5, i18n("Wake-On-LAN Settings"));
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
void Smb4KCustomSettingsEditorWidget::setupView()
{
    QWidget *tab1 = new QWidget(this);
    QGridLayout *tab1Layout = new QGridLayout(tab1);

    m_ipAddressLabel = new QLabel(i18n("IP Address:"), tab1);
    m_ipAddress = new KLineEdit(tab1);
    m_ipAddress->setClearButtonEnabled(true);
    // m_ipAddress->setInputMask(QStringLiteral("000.000.000.000"));
    m_ipAddressLabel->setBuddy(m_ipAddress);

    connect(m_ipAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotIpAddressChanged);

    m_workgroupLabel = new QLabel(i18n("Workgroup:"), tab1);
    m_workgroup = new KLineEdit(tab1);
    m_workgroup->setClearButtonEnabled(true);
    m_workgroupLabel->setBuddy(m_workgroup);

    connect(m_workgroup, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotWorkgroupNameChanged);

    m_alwaysRemountShare = new QCheckBox(i18n("Always remount this share"), tab1);
    m_alwaysRemountShare->setEnabled(false);

    connect(m_alwaysRemountShare, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotAlwaysRemoutShareToggled);

    tab1Layout->addWidget(m_ipAddressLabel, 0, 0);
    tab1Layout->addWidget(m_ipAddress, 0, 1);
    tab1Layout->addWidget(m_workgroupLabel, 1, 0);
    tab1Layout->addWidget(m_workgroup, 1, 1);
    tab1Layout->addWidget(m_alwaysRemountShare, 2, 0, 1, 2);
    tab1Layout->setRowStretch(3, 100);

    addTab(tab1, i18n("Basic Settings"));

    QWidget *tab2 = new QWidget(this);
    QGridLayout *tab2Layout = new QGridLayout(tab2);

    m_useIds = new QCheckBox(Smb4KMountSettings::self()->useIdsItem()->label(), tab2);

    m_userIdLabel = new QLabel(i18n("User ID:"), tab2);
    m_userIdLabel->setIndent(25);
    m_userId = new KLineEdit(KUser(KUser::UseRealUserID).userId().toString(), tab2);
    m_userId->setAlignment(Qt::AlignRight);
    m_userId->setReadOnly(true);

    m_groupIdLabel = new QLabel(i18n("Group ID:"), tab2);
    m_groupIdLabel->setIndent(25);
    m_groupId = new KLineEdit(KUser(KUser::UseRealUserID).groupId().toString(), tab2);
    m_groupId->setAlignment(Qt::AlignRight);
    m_groupId->setReadOnly(true);

    connect(m_useIds, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseIdsToggled);

    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), tab2);
    m_fileMode = new KLineEdit(tab2);
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setInputMask(QStringLiteral("0999"));
    m_fileMode->setValidator(new ModeValidator(m_fileMode));

    connect(m_useFileMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseFileModeToggled);
    connect(m_fileMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotFileModeChanged);

    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), tab2);
    m_directoryMode = new KLineEdit(tab2);
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setInputMask(QStringLiteral("0999"));
    m_directoryMode->setValidator(new ModeValidator(m_directoryMode));

    connect(m_useDirectoryMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseDirectoryModeToggled);
    connect(m_directoryMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotDirectoryModeChanged);

    tab2Layout->addWidget(m_useIds, 0, 0, 1, 2);
    tab2Layout->addWidget(m_userIdLabel, 1, 0);
    tab2Layout->addWidget(m_userId, 1, 1);
    tab2Layout->addWidget(m_groupIdLabel, 2, 0);
    tab2Layout->addWidget(m_groupId, 2, 1);
    tab2Layout->addWidget(m_useFileMode, 3, 0);
    tab2Layout->addWidget(m_fileMode, 3, 1);
    tab2Layout->addWidget(m_useDirectoryMode, 4, 0);
    tab2Layout->addWidget(m_directoryMode, 4, 1);
    tab2Layout->setRowStretch(4, 100);

    addTab(tab2, i18n("Mount Settings"));

    QWidget *tab3 = new QWidget(this);
    QGridLayout *tab3Layout = new QGridLayout(tab3);

    m_useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), tab3);

    m_minimalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), tab3);
    m_minimalClientProtocolVersionLabel->setIndent(25);
    m_minimalClientProtocolVersionLabel->setEnabled(false);
    m_minimalClientProtocolVersion = new KComboBox(tab3);
    m_minimalClientProtocolVersion->setEnabled(false);
    m_minimalClientProtocolVersionLabel->setBuddy(m_minimalClientProtocolVersion);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (int i = 0; i < minimalClientProtocolVersionChoices.size(); ++i) {
        m_minimalClientProtocolVersion->addItem(minimalClientProtocolVersionChoices.at(i).label, i);
    }

    m_maximalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), tab3);
    m_maximalClientProtocolVersionLabel->setIndent(25);
    m_maximalClientProtocolVersionLabel->setEnabled(false);
    m_maximalClientProtocolVersion = new KComboBox(tab3);
    m_maximalClientProtocolVersion->setEnabled(false);
    m_maximalClientProtocolVersionLabel->setBuddy(m_maximalClientProtocolVersion);

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (int i = 0; i < maximalClientProtocolVersionChoices.size(); ++i) {
        m_maximalClientProtocolVersion->addItem(maximalClientProtocolVersionChoices.at(i).label, i);
    }

    connect(m_useClientProtocolVersions, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseClientProtocolVersionsToggled);
    connect(m_minimalClientProtocolVersion, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotMinimalClientProtocolVersionChanged);
    connect(m_maximalClientProtocolVersion, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotMaximalClientProtocolVersionChanged);

    m_useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), tab3);

    connect(m_useKerberos, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled);

    tab3Layout->addWidget(m_useClientProtocolVersions, 0, 0, 1, 2);
    tab3Layout->addWidget(m_minimalClientProtocolVersionLabel, 1, 0);
    tab3Layout->addWidget(m_minimalClientProtocolVersion, 1, 1);
    tab3Layout->addWidget(m_maximalClientProtocolVersionLabel, 2, 0);
    tab3Layout->addWidget(m_maximalClientProtocolVersion, 2, 1);
    tab3Layout->addWidget(m_useKerberos, 3, 0, 1, 2);
    tab3Layout->setRowStretch(4, 100);

    addTab(tab3, i18n("Browse Settings"));

    QWidget *tab4 = new QWidget(this);
    QVBoxLayout *tab4Layout = new QVBoxLayout(tab4);

    tab4->setEnabled(Smb4KSettings::enableWakeOnLAN());

    QWidget *macAddressWidget = new QWidget(tab4);
    QHBoxLayout *macAddressWidgetLayout = new QHBoxLayout(macAddressWidget);
    macAddressWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressWidget);
    m_macAddress = new KLineEdit(macAddressWidget);
    m_macAddress->setClearButtonEnabled(true);
    m_macAddress->setInputMask(QStringLiteral("HH:HH:HH:HH:HH:HH;_")); // MAC address, see QLineEdit doc
    m_macAddressLabel->setBuddy(m_macAddress);
    m_macAddressSearchButton = new QPushButton(macAddressWidget);
    m_macAddressSearchButton->setIcon(KDE::icon(QStringLiteral("edit-find")));
    m_macAddressSearchButton->setToolTip(i18n("Find MAC address"));

    connect(m_macAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotMacAddressChanged);
    connect(m_macAddressSearchButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditorWidget::slotFindMacAddressClicked);

    macAddressWidgetLayout->addWidget(m_macAddressLabel);
    macAddressWidgetLayout->addWidget(m_macAddress);
    macAddressWidgetLayout->addWidget(m_macAddressSearchButton);

    m_sendPacketBeforeScan = new QCheckBox(i18n("Send magic packet before scanning the network neighborhood"), tab4);
    m_sendPacketBeforeScan->setEnabled(false);
    m_sendPacketBeforeMount = new QCheckBox(i18n("Send magic packet before mounting a share"), tab4);
    m_sendPacketBeforeMount->setEnabled(false);

    connect(m_sendPacketBeforeScan, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeScanToggled);
    connect(m_sendPacketBeforeMount, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeMountToggled);

    tab4Layout->addWidget(macAddressWidget);
    tab4Layout->addWidget(m_sendPacketBeforeScan);
    tab4Layout->addWidget(m_sendPacketBeforeMount);
    tab4Layout->addStretch(100);

    m_wakeOnLanTabIndex = addTab(tab4, i18n("Wake-On-LAN Settings"));
}
#else
void Smb4KCustomSettingsEditorWidget::setupView()
{
}
#endif

void Smb4KCustomSettingsEditorWidget::setCustomSettings(const Smb4KCustomSettings &settings)
{
    m_customSettings = settings;

    m_ipAddress->setText(m_customSettings.ipAddress());
    m_workgroup->setText(m_customSettings.workgroupName());

    if (m_customSettings.type() != Host) {
        m_alwaysRemountShare->setEnabled(true);
        m_alwaysRemountShare->setChecked(m_customSettings.remount() == Smb4KCustomSettings::RemountAlways);
    }

#ifdef Q_OS_LINUX
    m_useWriteAccess->setChecked(m_customSettings.useWriteAccess());
    m_writeAccess->setCurrentIndex(m_customSettings.writeAccess());

    m_cifsUnixExtensionSupport->setChecked(m_customSettings.cifsUnixExtensionsSupport());
#endif

    m_useIds->setChecked(m_customSettings.useIds());

    m_useFileMode->setChecked(m_customSettings.useFileMode());
    m_fileMode->setText(m_customSettings.fileMode());

    m_useDirectoryMode->setChecked(m_customSettings.useDirectoryMode());
    m_directoryMode->setText(m_customSettings.directoryMode());

#ifdef Q_OS_LINUX
    m_useSmbMountProtocolVersion->setChecked(m_customSettings.useMountProtocolVersion());
    int mountProtocolVersionIndex = m_smbMountProtocolVersion->findData(m_customSettings.mountProtocolVersion());
    m_smbMountProtocolVersion->setCurrentIndex(mountProtocolVersionIndex);

    m_useSecurityMode->setChecked(m_customSettings.useSecurityMode());
    int securityModeIndex = m_securityMode->findData(m_customSettings.securityMode());
    m_securityMode->setCurrentIndex(securityModeIndex);
#endif

    m_useClientProtocolVersions->setChecked(m_customSettings.useClientProtocolVersions());
    int minimalClientProtocolVersionIndex = m_minimalClientProtocolVersion->findData(m_customSettings.minimalClientProtocolVersion());
    m_minimalClientProtocolVersion->setCurrentIndex(minimalClientProtocolVersionIndex);
    int maximalClientProtocolVersionIndex = m_maximalClientProtocolVersion->findData(m_customSettings.maximalClientProtocolVersion());
    m_maximalClientProtocolVersion->setCurrentIndex(maximalClientProtocolVersionIndex);

    m_useKerberos->setChecked(m_customSettings.useKerberos());

    widget(m_wakeOnLanTabIndex)->setEnabled(Smb4KSettings::enableWakeOnLAN());

    if (m_customSettings.type() == Host) {
        // FIXME: Enable Search button if IP address is not empty
        m_macAddress->setText(m_customSettings.macAddress());
        m_sendPacketBeforeScan->setChecked(m_customSettings.wakeOnLanSendBeforeNetworkScan());
        m_sendPacketBeforeMount->setChecked(m_customSettings.wakeOnLanSendBeforeMount());
    }
}

Smb4KCustomSettings Smb4KCustomSettingsEditorWidget::getCustomSettings() const
{
    m_customSettings.setIpAddress(m_ipAddress->text());
    m_customSettings.setWorkgroupName(m_workgroup->text());

    if (m_customSettings.type() != Host) {
        m_customSettings.setRemount(m_alwaysRemountShare->isChecked() ? Smb4KCustomSettings::RemountAlways : Smb4KCustomSettings::UndefinedRemount);
    }

#ifdef Q_OS_LINUX
    m_customSettings.setUseWriteAccess(m_useWriteAccess->isChecked());
    m_customSettings.setWriteAccess(m_writeAccess->currentIndex());

    m_customSettings.setCifsUnixExtensionsSupport(m_cifsUnixExtensionSupport->isChecked());
#endif

    m_customSettings.setUseIds(m_useIds->isChecked());

    m_customSettings.setUseFileMode(m_useFileMode->isChecked());
    m_customSettings.setFileMode(m_fileMode->text());

    m_customSettings.setUseDirectoryMode(m_useDirectoryMode->isChecked());
    m_customSettings.setDirectoryMode(m_directoryMode->text());

#ifdef Q_OS_LINUX
    m_customSettings.setUseMountProtocolVersion(m_useSmbMountProtocolVersion->isChecked());
    m_customSettings.setMountProtocolVersion(m_smbMountProtocolVersion->currentData().toInt());

    m_customSettings.setUseSecurityMode(m_useSecurityMode->isChecked());
    m_customSettings.setSecurityMode(m_securityMode->currentData().toInt());
#endif

    m_customSettings.setUseClientProtocolVersions(m_useClientProtocolVersions->isChecked());
    m_customSettings.setMinimalClientProtocolVersion(m_minimalClientProtocolVersion->currentData().toInt());
    m_customSettings.setMaximalClientProtocolVersion(m_maximalClientProtocolVersion->currentData().toInt());

    m_customSettings.setUseKerberos(m_useKerberos->isChecked());

    if (m_macAddress->hasAcceptableInput()) {
        m_customSettings.setMacAddress(m_macAddress->text());
    } else {
        m_customSettings.setMacAddress(QString());
    }
    m_customSettings.setWakeOnLanSendBeforeNetworkScan(m_sendPacketBeforeScan->isChecked());
    m_customSettings.setWakeOnLanSendBeforeMount(m_sendPacketBeforeMount->isChecked());

    return m_customSettings;
}

void Smb4KCustomSettingsEditorWidget::clear()
{
    m_customSettings = Smb4KCustomSettings();
    setCurrentIndex(0);

    m_alwaysRemountShare->setChecked(false);
    m_alwaysRemountShare->setEnabled(false);

#ifdef Q_OS_LINUX
    m_useWriteAccess->setChecked(false);
    m_writeAccess->setCurrentIndex(0);

    m_cifsUnixExtensionSupport->setChecked(false);
#endif

    m_useIds->setChecked(false);

    m_useFileMode->setChecked(false);
    m_fileMode->clear();

    m_useDirectoryMode->setChecked(false);
    m_directoryMode->clear();

#ifdef Q_OS_LINUX
    m_useSmbMountProtocolVersion->setChecked(false);
    m_smbMountProtocolVersion->setCurrentIndex(0);

    m_useSecurityMode->setChecked(false);
    m_securityMode->setCurrentIndex(0);
#endif

    m_useClientProtocolVersions->setChecked(false);
    m_minimalClientProtocolVersion->setCurrentIndex(0);
    m_maximalClientProtocolVersion->setCurrentIndex(0);

    m_useKerberos->setChecked(false);

    m_macAddress->clear();
    m_sendPacketBeforeScan->setChecked(false);
    m_sendPacketBeforeMount->setChecked(false);

    widget(m_wakeOnLanTabIndex)->setEnabled(Smb4KSettings::enableWakeOnLAN());
}

bool Smb4KCustomSettingsEditorWidget::hasDefaultCustomSettings() const
{
    return m_hasDefaultCustomSettings;
}

void Smb4KCustomSettingsEditorWidget::checkValues()
{
    // Reset m_hasDefaultCustomSettings for this check.
    m_hasDefaultCustomSettings = true;

    // Check against default values. We won't check the URL, IP address
    // and workgroup name, because those settings aren't pre-defined.
    Smb4KCustomSettings defaultCustomSettings;

    switch (defaultCustomSettings.remount()) {
    case Smb4KCustomSettings::RemountAlways: {
        if (!m_alwaysRemountShare->isChecked()) {
            m_hasDefaultCustomSettings = false;
        }
        break;
    }
    default: {
        if (m_alwaysRemountShare->isChecked()) {
            m_hasDefaultCustomSettings = false;
        }
        break;
    }
    }

#ifdef Q_OS_LINUX
    if (m_useWriteAccess->isChecked() != defaultCustomSettings.useWriteAccess()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_writeAccess->currentIndex() != defaultCustomSettings.writeAccess()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_cifsUnixExtensionSupport->isChecked() != defaultCustomSettings.cifsUnixExtensionsSupport()) {
        m_hasDefaultCustomSettings = false;
    }
#endif

    if (m_useIds->isChecked() != defaultCustomSettings.useIds()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_useFileMode->isChecked() != defaultCustomSettings.useFileMode()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_fileMode->text() != defaultCustomSettings.fileMode()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_useDirectoryMode->isChecked() != defaultCustomSettings.useDirectoryMode()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_directoryMode->text() != defaultCustomSettings.directoryMode()) {
        m_hasDefaultCustomSettings = false;
    }

#ifdef Q_OS_LINUX
    if (m_useSmbMountProtocolVersion->isChecked() != defaultCustomSettings.useMountProtocolVersion()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_smbMountProtocolVersion->currentData().toInt() != defaultCustomSettings.mountProtocolVersion()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_useSecurityMode->isChecked() != defaultCustomSettings.useSecurityMode()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_securityMode->currentData().toInt() != defaultCustomSettings.securityMode()) {
        m_hasDefaultCustomSettings = false;
    }
#endif

    if (m_useClientProtocolVersions->isChecked() != defaultCustomSettings.useClientProtocolVersions()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_minimalClientProtocolVersion->currentData().toInt() != defaultCustomSettings.minimalClientProtocolVersion()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_maximalClientProtocolVersion->currentData().toInt() != defaultCustomSettings.maximalClientProtocolVersion()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_useKerberos->isChecked() != defaultCustomSettings.useKerberos()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_macAddress->hasAcceptableInput() && (m_macAddress->text() != defaultCustomSettings.macAddress())) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_sendPacketBeforeScan->isChecked() != defaultCustomSettings.wakeOnLanSendBeforeNetworkScan()) {
        m_hasDefaultCustomSettings = false;
    }

    if (m_sendPacketBeforeMount->isChecked() != defaultCustomSettings.wakeOnLanSendBeforeMount()) {
        m_hasDefaultCustomSettings = false;
    }

    // Check if the initial values were edited.
    if (m_ipAddress->text() != m_customSettings.ipAddress()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_workgroup->text().toUpper() != m_customSettings.workgroupName().toUpper()) {
        Q_EMIT edited(true);
        return;
    }

    switch (m_customSettings.remount()) {
    case Smb4KCustomSettings::RemountAlways: {
        if (!m_alwaysRemountShare->isChecked()) {
            Q_EMIT edited(true);
            return;
        }
        break;
    }
    default: {
        if (m_alwaysRemountShare->isChecked()) {
            Q_EMIT edited(true);
            return;
        }
        break;
    }
    }

#ifdef Q_OS_LINUX
    if (m_useWriteAccess->isChecked() != m_customSettings.useWriteAccess()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_writeAccess->currentIndex() != m_customSettings.writeAccess()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_cifsUnixExtensionSupport->isChecked() != m_customSettings.cifsUnixExtensionsSupport()) {
        Q_EMIT edited(true);
        return;
    }
#endif

    if (m_useIds->isChecked() != m_customSettings.useIds()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_useFileMode->isChecked() != m_customSettings.useFileMode()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_fileMode->text() != m_customSettings.fileMode()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_useDirectoryMode->isChecked() != m_customSettings.useDirectoryMode()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_directoryMode->text() != m_customSettings.directoryMode()) {
        Q_EMIT edited(true);
        return;
    }

#ifdef Q_OS_LINUX
    if (m_useSmbMountProtocolVersion->isChecked() != m_customSettings.useMountProtocolVersion()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_smbMountProtocolVersion->currentData().toInt() != m_customSettings.mountProtocolVersion()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_useSecurityMode->isChecked() != m_customSettings.useSecurityMode()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_securityMode->currentData().toInt() != m_customSettings.securityMode()) {
        Q_EMIT edited(true);
        return;
    }
#endif

    if (m_useClientProtocolVersions->isChecked() != m_customSettings.useClientProtocolVersions()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_minimalClientProtocolVersion->currentData().toInt() != m_customSettings.minimalClientProtocolVersion()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_maximalClientProtocolVersion->currentData().toInt() != m_customSettings.maximalClientProtocolVersion()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_useKerberos->isChecked() != m_customSettings.useKerberos()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_macAddress->hasAcceptableInput() && m_macAddress->text() != m_customSettings.macAddress()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_sendPacketBeforeScan->isChecked() != m_customSettings.wakeOnLanSendBeforeNetworkScan()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_sendPacketBeforeMount->isChecked() != m_customSettings.wakeOnLanSendBeforeMount()) {
        Q_EMIT edited(true);
        return;
    }

    Q_EMIT edited(false);
}

void Smb4KCustomSettingsEditorWidget::slotIpAddressChanged(const QString &text)
{
    Q_UNUSED(text);

    // FIXME: Check if this is indeed a valid IP address!?

    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotWorkgroupNameChanged(const QString &text)
{
    Q_UNUSED(text);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotAlwaysRemoutShareToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

#ifdef Q_OS_LINUX
void Smb4KCustomSettingsEditorWidget::slotUseWriteAccessToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotWriteAccessChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotCifsUnixExtensionSupportToggled(bool checked)
{
    Q_UNUSED(checked);

    m_useIds->setEnabled(!checked);
    m_userIdLabel->setEnabled(!checked);
    m_userId->setEnabled(!checked);
    m_groupIdLabel->setEnabled(!checked);
    m_groupId->setEnabled(!checked);
    m_useFileMode->setEnabled(!checked);
    m_fileMode->setEnabled(!checked);
    m_useDirectoryMode->setEnabled(!checked);
    m_directoryMode->setEnabled(!checked);

    checkValues();
}
#endif

void Smb4KCustomSettingsEditorWidget::slotUseIdsToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseFileModeToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotFileModeChanged(const QString &text)
{
    Q_UNUSED(text);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseDirectoryModeToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotDirectoryModeChanged(const QString &text)
{
    Q_UNUSED(text);
    checkValues();
}

#ifdef Q_OS_LINUX
void Smb4KCustomSettingsEditorWidget::slotUseSmbMountProtocolVersionToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotSmbMountProtocolVersionChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseSecurityModeToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotSecurityModeChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}
#endif

void Smb4KCustomSettingsEditorWidget::slotUseClientProtocolVersionsToggled(bool checked)
{
    Q_UNUSED(checked);

    m_minimalClientProtocolVersionLabel->setEnabled(checked);
    m_minimalClientProtocolVersion->setEnabled(checked);
    m_maximalClientProtocolVersionLabel->setEnabled(checked);
    m_maximalClientProtocolVersion->setEnabled(checked);

    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotMinimalClientProtocolVersionChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotMaximalClientProtocolVersionChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotFindMacAddressClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!m_customSettings.ipAddress().isEmpty()) {
        QString macAddress = findMacAddress(m_customSettings.ipAddress());

        if (!macAddress.isEmpty()) {
            m_macAddress->setText(macAddress);
        }
    }
}

void Smb4KCustomSettingsEditorWidget::slotMacAddressChanged(const QString &text)
{
    Q_UNUSED(text);

    m_sendPacketBeforeScan->setEnabled(!m_macAddress->text().isEmpty() && m_macAddress->hasAcceptableInput());
    m_sendPacketBeforeMount->setEnabled(!m_macAddress->text().isEmpty() && m_macAddress->hasAcceptableInput());

    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeScanToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeMountToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}
