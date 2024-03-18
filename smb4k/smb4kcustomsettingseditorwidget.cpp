/*
 *  Editor widget for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

Smb4KCustomSettingsEditorWidget::Smb4KCustomSettingsEditorWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // FIXME: Implement mount point!?

    // FIXME: Honor disabled widgets and unchecked check boxes!?
    m_haveCustomSettings = false;
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

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : qAsConst(writeAccessChoices)) {
        m_writeAccess->addItem(wa.label);
    }

    connect(m_useWriteAccess, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseWriteAccessToggled);
    connect(m_writeAccess, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotWriteAccessChanged);

    m_useFileSystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), tab2);
    m_fileSystemPort = new QSpinBox(tab2);
    m_fileSystemPort->setMinimum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->minValue().toInt());
    m_fileSystemPort->setMaximum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->maxValue().toInt());

    connect(m_useFileSystemPort, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseFileSystemPortToggled);
    connect(m_fileSystemPort, &QSpinBox::valueChanged, this, &Smb4KCustomSettingsEditorWidget::slotFileSystemPortChanged);

    m_cifsUnixExtensionSupport = new QCheckBox(i18n("This server supports the CIFS Unix extensions"), tab2);

    connect(m_cifsUnixExtensionSupport, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotCifsUnixExtensionSupportToggled);

    m_useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), tab2);
    m_userId = new KComboBox(tab2);

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &user : qAsConst(allUsers)) {
        m_userId->addItem(user.loginName() + QStringLiteral(" (") + user.userId().toString() + QStringLiteral(")"), user.userId().toString());
    }

    connect(m_useUserId, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseUserIdToggled);
    connect(m_userId, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotUserIdChanged);

    m_useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), tab2);
    m_groupId = new KComboBox(tab2);

    QList<KUserGroup> allGroups = KUserGroup::allGroups();

    for (const KUserGroup &group : qAsConst(allGroups)) {
        m_groupId->addItem(group.name() + QStringLiteral(" (") + group.groupId().toString() + QStringLiteral(")"), group.groupId().toString());
    }

    connect(m_useGroupId, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseGroupIdToggled);
    connect(m_groupId, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotGroupIdChanged);

    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), tab2);
    m_fileMode = new KLineEdit(tab2);
    m_fileMode->setClearButtonEnabled(true);

    connect(m_useFileMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseFileModeToggled);
    connect(m_fileMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotFileModeChanged);

    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), tab2);
    m_directoryMode = new KLineEdit(tab2);
    m_directoryMode->setClearButtonEnabled(true);

    connect(m_useDirectoryMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseDirectoryModeToggled);
    connect(m_directoryMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotDirectoryModeChanged);

    tab2Layout->addWidget(m_useWriteAccess, 0, 0);
    tab2Layout->addWidget(m_writeAccess, 0, 1);
    tab2Layout->addWidget(m_useFileSystemPort, 1, 0);
    tab2Layout->addWidget(m_fileSystemPort, 1, 1);
    tab2Layout->addWidget(m_cifsUnixExtensionSupport, 2, 0, 1, 2);
    tab2Layout->addWidget(m_useUserId, 3, 0);
    tab2Layout->addWidget(m_userId, 3, 1);
    tab2Layout->addWidget(m_useGroupId, 4, 0);
    tab2Layout->addWidget(m_groupId, 4, 1);
    tab2Layout->addWidget(m_useFileMode, 5, 0);
    tab2Layout->addWidget(m_fileMode, 5, 1);
    tab2Layout->addWidget(m_useDirectoryMode, 6, 0);
    tab2Layout->addWidget(m_directoryMode, 6, 1);
    tab2Layout->setRowStretch(7, 100);

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

    m_useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), tab4);
    m_remoteSmbPort = new QSpinBox(tab4);
    m_remoteSmbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
    m_remoteSmbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());

    connect(m_useRemoteSmbPort, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseClientProtocolVersionsToggled);
    connect(m_remoteSmbPort, &QSpinBox::valueChanged, this, &Smb4KCustomSettingsEditorWidget::slotRemoteSmbPortChanged);

    m_useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), tab4);

    connect(m_useKerberos, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled);

    tab4Layout->addWidget(m_useClientProtocolVersions, 0, 0, 1, 2);
    tab4Layout->addWidget(m_minimalClientProtocolVersionLabel, 1, 0);
    tab4Layout->addWidget(m_minimalClientProtocolVersion, 1, 1);
    tab4Layout->addWidget(m_maximalClientProtocolVersionLabel, 2, 0);
    tab4Layout->addWidget(m_maximalClientProtocolVersion, 2, 1);
    tab4Layout->addWidget(m_useRemoteSmbPort, 3, 0);
    tab4Layout->addWidget(m_remoteSmbPort, 3, 1);
    tab4Layout->addWidget(m_useKerberos, 4, 0, 1, 2);
    tab4Layout->setRowStretch(5, 100);

    addTab(tab4, i18n("Browse Settings"));

    QWidget *tab5 = new QWidget(this);
    QGridLayout *tab5Layout = new QGridLayout(tab5);

    m_macAddressLabel = new QLabel(i18n("MAC Address:"), tab5);
    m_macAddressLabel->setEnabled(false);
    m_macAddress = new KLineEdit(tab5);
    m_macAddress->setClearButtonEnabled(true);
    m_macAddress->setInputMask(QStringLiteral("HH:HH:HH:HH:HH:HH;_")); // MAC address, see QLineEdit doc
    m_macAddress->setEnabled(false);
    m_macAddressLabel->setBuddy(m_macAddress);

    connect(m_macAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotMacAddressChanged);

    m_sendPacketBeforeScan = new QCheckBox(i18n("Send magic packet before scanning the network neighborhood"), tab5);
    m_sendPacketBeforeScan->setEnabled(false);
    m_sendPacketBeforeMount = new QCheckBox(i18n("Send magic packet before mounting a share"), tab5);
    m_sendPacketBeforeMount->setEnabled(false);

    connect(m_sendPacketBeforeScan, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeScanToggled);
    connect(m_sendPacketBeforeMount, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeMountToggled);

    tab5Layout->addWidget(m_macAddressLabel, 0, 0);
    tab5Layout->addWidget(m_macAddress, 0, 1);
    tab5Layout->addWidget(m_sendPacketBeforeScan, 1, 0, 1, 2);
    tab5Layout->addWidget(m_sendPacketBeforeMount, 2, 0, 1, 2);
    tab5Layout->setRowStretch(3, 100);

    addTab(tab5, i18n("Wake-On-LAN Settings"));
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

    m_useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), tab2);
    m_userId = new KComboBox(tab2);

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &user : qAsConst(allUsers)) {
        m_userId->addItem(user.loginName() + QStringLiteral(" (") + user.userId().toString() + QStringLiteral(")"), user.userId().toString());
    }

    connect(m_useUserId, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseUserIdToggled);
    connect(m_userId, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotUserIdChanged);

    m_useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), tab2);
    m_groupId = new KComboBox(tab2);

    QList<KUserGroup> allGroups = KUserGroup::allGroups();

    for (const KUserGroup &group : qAsConst(allGroups)) {
        m_groupId->addItem(group.name() + QStringLiteral(" (") + group.groupId().toString() + QStringLiteral(")"), group.groupId().toString());
    }

    connect(m_useGroupId, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseGroupIdToggled);
    connect(m_groupId, &KComboBox::currentIndexChanged, this, &Smb4KCustomSettingsEditorWidget::slotGroupIdChanged);

    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), tab2);
    m_fileMode = new KLineEdit(tab2);
    m_fileMode->setClearButtonEnabled(true);

    connect(m_useFileMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseFileModeToggled);
    connect(m_fileMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotFileModeChanged);

    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), tab2);
    m_directoryMode = new KLineEdit(tab2);
    m_directoryMode->setClearButtonEnabled(true);

    connect(m_useDirectoryMode, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseDirectoryModeToggled);
    connect(m_directoryMode, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotDirectoryModeChanged);

    tab2Layout->addWidget(m_useUserId, 0, 0);
    tab2Layout->addWidget(m_userId, 0, 1);
    tab2Layout->addWidget(m_useGroupId, 1, 0);
    tab2Layout->addWidget(m_groupId, 1, 1);
    tab2Layout->addWidget(m_useFileMode, 2, 0);
    tab2Layout->addWidget(m_fileMode, 2, 1);
    tab2Layout->addWidget(m_useDirectoryMode, 3, 0);
    tab2Layout->addWidget(m_directoryMode, 3, 1);
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

    m_useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), tab3);
    m_remoteSmbPort = new QSpinBox(tab3);
    m_remoteSmbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
    m_remoteSmbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());

    connect(m_useRemoteSmbPort, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseClientProtocolVersionsToggled);
    connect(m_remoteSmbPort, &QSpinBox::valueChanged, this, &Smb4KCustomSettingsEditorWidget::slotRemoteSmbPortChanged);

    m_useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), tab3);

    connect(m_useKerberos, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled);

    tab3Layout->addWidget(m_useClientProtocolVersions, 0, 0, 1, 2);
    tab3Layout->addWidget(m_minimalClientProtocolVersionLabel, 1, 0);
    tab3Layout->addWidget(m_minimalClientProtocolVersion, 1, 1);
    tab3Layout->addWidget(m_maximalClientProtocolVersionLabel, 2, 0);
    tab3Layout->addWidget(m_maximalClientProtocolVersion, 2, 1);
    tab3Layout->addWidget(m_useRemoteSmbPort, 3, 0);
    tab3Layout->addWidget(m_remoteSmbPort, 3, 1);
    tab3Layout->addWidget(m_useKerberos, 4, 0, 1, 2);
    tab3Layout->setRowStretch(5, 100);

    addTab(tab3, i18n("Browse Settings"));

    QWidget *tab4 = new QWidget(this);
    QGridLayout *tab4Layout = new QGridLayout(tab4);

    m_macAddressLabel = new QLabel(i18n("MAC Address:"), tab4);
    m_macAddressLabel->setEnabled(false);
    m_macAddress = new KLineEdit(tab4);
    m_macAddress->setClearButtonEnabled(true);
    m_macAddress->setInputMask(QStringLiteral("HH:HH:HH:HH:HH:HH;_")); // MAC address, see QLineEdit doc
    m_macAddress->setEnabled(false);
    m_macAddressLabel->setBuddy(m_macAddress);

    connect(m_macAddress, &KLineEdit::textChanged, this, &Smb4KCustomSettingsEditorWidget::slotMacAddressChanged);

    m_sendPacketBeforeScan = new QCheckBox(i18n("Send magic packet before scanning the network neighborhood"), tab4);
    m_sendPacketBeforeScan->setEnabled(false);
    m_sendPacketBeforeMount = new QCheckBox(i18n("Send magic packet before mounting a share"), tab4);
    m_sendPacketBeforeMount->setEnabled(false);

    connect(m_sendPacketBeforeScan, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeScanToggled);
    connect(m_sendPacketBeforeMount, &QCheckBox::toggled, this, &Smb4KCustomSettingsEditorWidget::slotSendPacketBeforeMountToggled);

    tab4Layout->addWidget(m_macAddressLabel, 0, 0);
    tab4Layout->addWidget(m_macAddress, 0, 1);
    tab4Layout->addWidget(m_sendPacketBeforeScan, 1, 0, 1, 2);
    tab4Layout->addWidget(m_sendPacketBeforeMount, 2, 0, 1, 2);
    tab4Layout->setRowStretch(3, 100);

    addTab(tab4, i18n("Wake-On-LAN Settings"));
}
#else
void Smb4KCustomSettingsEditorWidget::setupView()
{
}
#endif

void Smb4KCustomSettingsEditorWidget::setCustomSettings(const Smb4KCustomSettings &settings)
{
    m_ipAddress->setText(settings.ipAddress());
    m_workgroup->setText(settings.workgroupName());

    if (settings.type() != Host) {
        m_alwaysRemountShare->setEnabled(true);
        m_alwaysRemountShare->setChecked(settings.remount() == Smb4KCustomSettings::RemountAlways);
    }

#ifdef Q_OS_LINUX
    m_useWriteAccess->setChecked(settings.useWriteAccess());
    m_writeAccess->setCurrentIndex(settings.writeAccess());

    m_useFileSystemPort->setChecked(settings.useFileSystemPort());
    m_fileSystemPort->setValue(settings.fileSystemPort());

    m_cifsUnixExtensionSupport->setChecked(settings.cifsUnixExtensionsSupport());
#endif

    m_useUserId->setChecked(settings.useUser());
    int userIndex = m_userId->findData(settings.user().userId().toString());
    m_userId->setCurrentIndex(userIndex);

    m_useGroupId->setChecked(settings.useGroup());
    int groupIndex = m_groupId->findData(settings.group().groupId().toString());
    m_groupId->setCurrentIndex(groupIndex);

    m_useFileMode->setChecked(settings.useFileMode());
    m_fileMode->setText(settings.fileMode());

    m_useDirectoryMode->setChecked(settings.useDirectoryMode());
    m_directoryMode->setText(settings.directoryMode());

#ifdef Q_OS_LINUX
    m_useSmbMountProtocolVersion->setChecked(settings.useMountProtocolVersion());
    int mountProtocolVersionIndex = m_smbMountProtocolVersion->findData(settings.mountProtocolVersion());
    m_smbMountProtocolVersion->setCurrentIndex(mountProtocolVersionIndex);

    m_useSecurityMode->setChecked(settings.useSecurityMode());
    int securityModeIndex = m_securityMode->findData(settings.securityMode());
    m_securityMode->setCurrentIndex(securityModeIndex);
#endif

    m_useClientProtocolVersions->setChecked(settings.useClientProtocolVersions());
    int minimalClientProtocolVersionIndex = m_minimalClientProtocolVersion->findData(settings.minimalClientProtocolVersion());
    m_minimalClientProtocolVersion->setCurrentIndex(minimalClientProtocolVersionIndex);
    int maximalClientProtocolVersionIndex = m_maximalClientProtocolVersion->findData(settings.maximalClientProtocolVersion());
    m_maximalClientProtocolVersion->setCurrentIndex(maximalClientProtocolVersionIndex);

    m_useRemoteSmbPort->setChecked(settings.useSmbPort());
    m_remoteSmbPort->setValue(settings.smbPort());

    m_useKerberos->setChecked(settings.useKerberos());

    if (settings.type() == Host) {
        m_macAddressLabel->setEnabled(Smb4KSettings::enableWakeOnLAN());
        m_macAddress->setEnabled(Smb4KSettings::enableWakeOnLAN());
        m_sendPacketBeforeScan->setEnabled(Smb4KSettings::enableWakeOnLAN());
        m_sendPacketBeforeMount->setEnabled(Smb4KSettings::enableWakeOnLAN());

        m_macAddress->setText(settings.macAddress());
        m_sendPacketBeforeScan->setChecked(settings.wakeOnLanSendBeforeNetworkScan());
        m_sendPacketBeforeMount->setChecked(settings.wakeOnLanSendBeforeMount());
    }

    m_customSettings = settings;
    m_haveCustomSettings = true;
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

    m_customSettings.setUseFileSystemPort(m_useFileSystemPort->isChecked());
    m_customSettings.setFileSystemPort(m_fileSystemPort->value());

    m_customSettings.setCifsUnixExtensionsSupport(m_cifsUnixExtensionSupport->isChecked());
#endif

    m_customSettings.setUseUser(m_useUserId->isChecked());
    m_customSettings.setUser(KUser(K_UID(m_userId->currentData().toInt())));

    m_customSettings.setUseGroup(m_useGroupId->isChecked());
    m_customSettings.setGroup(KUserGroup(K_GID(m_groupId->currentData().toInt())));

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

    m_customSettings.setUseSmbPort(m_useRemoteSmbPort->isChecked());
    m_customSettings.setSmbPort(m_remoteSmbPort->value());

    m_customSettings.setUseKerberos(m_useKerberos->isChecked());

    if (m_macAddress->hasAcceptableInput()) {
        m_customSettings.setMACAddress(m_macAddress->text());
    } else {
        m_customSettings.setMACAddress(QString());
    }
    m_customSettings.setWakeOnLanSendBeforeNetworkScan(m_sendPacketBeforeScan->isChecked());
    m_customSettings.setWakeOnLanSendBeforeMount(m_sendPacketBeforeMount->isChecked());

    return m_customSettings;
}

void Smb4KCustomSettingsEditorWidget::clear()
{
    m_customSettings = Smb4KCustomSettings();
    m_haveCustomSettings = false;
    setCurrentIndex(0);

    m_alwaysRemountShare->setChecked(false);
    m_alwaysRemountShare->setEnabled(false);

#ifdef Q_OS_LINUX
    m_useWriteAccess->setChecked(false);
    m_writeAccess->setCurrentIndex(0);

    m_useFileSystemPort->setChecked(false);
    m_fileSystemPort->setValue(445);

    m_cifsUnixExtensionSupport->setChecked(false);
#endif

    m_useUserId->setChecked(false);
    m_userId->setCurrentIndex(0);

    m_useGroupId->setChecked(false);
    m_groupId->setCurrentIndex(0);

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

    m_useRemoteSmbPort->setChecked(false);
    m_remoteSmbPort->setValue(139);

    m_useKerberos->setChecked(false);

    m_macAddress->clear();
    m_macAddress->setEnabled(false);
    m_sendPacketBeforeScan->setChecked(false);
    m_sendPacketBeforeScan->setEnabled(false);
    m_sendPacketBeforeMount->setChecked(false);
    m_sendPacketBeforeMount->setChecked(false);
}

void Smb4KCustomSettingsEditorWidget::checkValues()
{
    if (!m_haveCustomSettings) {
        return;
    }

    if (m_ipAddress->text() != m_customSettings.ipAddress()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_workgroup->text().toUpper() != m_customSettings.workgroupName().toUpper()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_alwaysRemountShare->isChecked() != (m_customSettings.remount() == Smb4KCustomSettings::RemountAlways)) {
        Q_EMIT edited(true);
        return;
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

    if (m_useFileSystemPort->isChecked() != m_customSettings.useFileSystemPort()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_fileSystemPort->value() != m_customSettings.fileSystemPort()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_cifsUnixExtensionSupport->isChecked() != m_customSettings.cifsUnixExtensionsSupport()) {
        Q_EMIT edited(true);
        return;
    }
#endif

    if (m_useUserId->isChecked() != m_customSettings.useUser()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_userId->currentData().toString() != m_customSettings.user().userId().toString()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_useGroupId->isChecked() != m_customSettings.useGroup()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_groupId->currentData().toString() != m_customSettings.group().groupId().toString()) {
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

    if (m_useRemoteSmbPort->isChecked() != m_customSettings.useSmbPort()) {
        Q_EMIT edited(true);
        return;
    }

    if (m_remoteSmbPort->value() != m_customSettings.smbPort()) {
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

    // FIXME: Propagate IP address to all other shares of this host?
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

void Smb4KCustomSettingsEditorWidget::slotUseFileSystemPortToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotFileSystemPortChanged(int port)
{
    Q_UNUSED(port);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotCifsUnixExtensionSupportToggled(bool checked)
{
    Q_UNUSED(checked);

    m_useUserId->setEnabled(!checked);
    m_userId->setEnabled(!checked);
    m_useGroupId->setEnabled(!checked);
    m_groupId->setEnabled(!checked);
    m_useFileMode->setEnabled(!checked);
    m_fileMode->setEnabled(!checked);
    m_useDirectoryMode->setEnabled(!checked);
    m_directoryMode->setEnabled(!checked);

    checkValues();
}
#endif

void Smb4KCustomSettingsEditorWidget::slotUseUserIdToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUserIdChanged(int index)
{
    Q_UNUSED(index);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseGroupIdToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotGroupIdChanged(int index)
{
    Q_UNUSED(index);
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

void Smb4KCustomSettingsEditorWidget::slotUseRemoteSmbPortToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotRemoteSmbPortChanged(int port)
{
    Q_UNUSED(port);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotUseKerberosToggled(bool checked)
{
    Q_UNUSED(checked);
    checkValues();
}

void Smb4KCustomSettingsEditorWidget::slotMacAddressChanged(const QString &text)
{
    Q_UNUSED(text);

    m_sendPacketBeforeScan->setEnabled(!text.isEmpty() && m_macAddress->hasAcceptableInput());
    m_sendPacketBeforeMount->setEnabled(!text.isEmpty() && m_macAddress->hasAcceptableInput());

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
