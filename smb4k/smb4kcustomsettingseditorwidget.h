/*
 *  Editor widget for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KCUSTOMSETTINGSEDITORWIDGET_H
#define SMB4KCUSTOMSETTINGSEDITORWIDGET_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>

// KDE includes
#include <KComboBox>
#include <KLineEdit>

/**
 * This widget is used to edit custom settings
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 3.3.0
 */

class Smb4KCustomSettingsEditorWidget : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KCustomSettingsEditorWidget(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KCustomSettingsEditorWidget();

    /**
     * Set the custom settings object that needs to be edited
     */
    void setCustomSettings(const Smb4KCustomSettings &settings);

    /**
     * Get the custom settings object that has been edited
     */
    Smb4KCustomSettings getCustomSettings() const;

    /**
     * Clears the input widgets
     */
    void clear();

Q_SIGNALS:
    void edited(bool changed);

protected Q_SLOTS:
    void slotAlwaysRemoutShareToggled(bool checked);
#ifdef Q_OS_LINUX
    void slotUseWriteAccessToggled(bool checked);
    void slotWriteAccessChanged(int index);
    void slotUseFileSystemPortToggled(bool checked);
    void slotFileSystemPortChanged(int port);
    void slotCifsUnixExtensionSupportToggled(bool checked);
#endif
    void slotUseUserIdToggled(bool checked);
    void slotUserIdChanged(int index);
    void slotUseGroupIdToggled(bool checked);
    void slotGroupIdChanged(int index);
    void slotUseFileModeToggled(bool checked);
    void slotFileModeChanged(const QString &text);
    void slotUseDirectoryModeToggled(bool checked);
    void slotDirectoryModeChanged(const QString &text);
#ifdef Q_OS_LINUX
    void slotUseSmbMountProtocolVersionToggled(bool checked);
    void slotSmbMountProtocolVersionChanged(int index);
    void slotUseSecurityModeToggled(bool checked);
    void slotSecurityModeChanged(int index);
#endif
    void slotUseClientProtocolVersionsToggled(bool checked);
    void slotMinimalClientProtocolVersionChanged(int index);
    void slotMaximalClientProtocolVersionChanged(int index);
    void slotUseRemoteSmbPortToggled(bool checked);
    void slotRemoteSmbPortChanged(int port);
    void slotUseKerberosToggled(bool checked);
    void slotMacAddressChanged(const QString &text);
    void slotSendPacketBeforeScanToggled(bool checked);
    void slotSendPacketBeforeMountToggled(bool checked);

private:
    void setupView();
    void checkValues();
    bool m_haveCustomSettings;
    Smb4KCustomSettings m_customSettings;
    QCheckBox *m_alwaysRemountShare;
#ifdef Q_OS_LINUX
    QCheckBox *m_useWriteAccess;
    KComboBox *m_writeAccess;
    QCheckBox *m_useFileSystemPort;
    QSpinBox *m_fileSystemPort;
    QCheckBox *m_cifsUnixExtensionSupport;
#endif
    QCheckBox *m_useUserId;
    KComboBox *m_userId;
    QCheckBox *m_useGroupId;
    KComboBox *m_groupId;
    QCheckBox *m_useFileMode;
    KLineEdit *m_fileMode;
    QCheckBox *m_useDirectoryMode;
    KLineEdit *m_directoryMode;
#ifdef Q_OS_LINUX
    QCheckBox *m_useSmbMountProtocolVersion;
    KComboBox *m_smbMountProtocolVersion;
    QCheckBox *m_useSecurityMode;
    KComboBox *m_securityMode;
#endif
    QCheckBox *m_useClientProtocolVersions;
    QLabel *m_minimalClientProtocolVersionLabel;
    KComboBox *m_minimalClientProtocolVersion;
    QLabel *m_maximalClientProtocolVersionLabel;
    KComboBox *m_maximalClientProtocolVersion;
    QCheckBox *m_useRemoteSmbPort;
    QSpinBox *m_remoteSmbPort;
    QCheckBox *m_useKerberos;
    QLabel *m_macAddressLabel;
    KLineEdit *m_macAddress;
    QCheckBox *m_sendPacketBeforeScan;
    QCheckBox *m_sendPacketBeforeMount;
};

#endif
