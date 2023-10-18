/*
    The configuration page for the mount options

    SPDX-FileCopyrightText: 2015-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEMOUNTING_H
#define SMB4KCONFIGPAGEMOUNTING_H

// Qt includes
#include <QTabWidget>
#include <QCheckBox>

// KDE includes
#include <KUrlRequester>
#include <KLineEdit>

/**
 * This configuration page contains the mount options
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 2.0.0
 */

class Smb4KConfigPageMounting : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * The constructor
     */
    explicit Smb4KConfigPageMounting(QWidget *parent = nullptr);

    /**
     * The destructor
     */
    virtual ~Smb4KConfigPageMounting();

    /**
     * Check the settings for problems. Returns TRUE if none were
     * found and FALSE otherwise.
     *
     * @returns TRUE if all settings are okay.
     */
    bool checkSettings();

protected Q_SLOTS:
    /**
     * Sets the new general user ID.
     *
     * @param action              The action that represents the new user.
     */
    void slotNewUserTriggered(QAction *action);

    /**
     * Sets the new general group ID.
     *
     * @param action              The action that represents the new group.
     */
    void slotNewGroupTriggered(QAction *action);

#if defined(Q_OS_LINUX)
    /**
     * Enable / disable the options that are only necessary when the servers
     * do not support the CIFS Unix extensions.
     *
     * @param checked             TRUE if the button is checked
     */
    void slotCIFSUnixExtensionsSupport(bool checked);

    /**
     * This slot is activated when the additional CIFS options are to be
     * edited (Linux only).
     */
    void slotAdditionalCIFSOptions();
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    /**
     * This slot is activated when the setting of the character set usage is changed
     * (BSD only).
     */
    void slotCharacterSets(bool on);
#endif

    /**
     * This slot is activated when the "Remount shares" check box is toggled
     */
    void slotRemountSharesToggled(bool on);

private:
    void setupWidget();
    KUrlRequester *m_mountPrefix;
    QWidget *m_remountSettingsWidget;
    QCheckBox *m_useFileMode;
    KLineEdit *m_fileMode;
    QCheckBox *m_useDirectoryMode;
    KLineEdit *m_directoryMode;
    KLineEdit *m_userId;
    KLineEdit *m_groupId;
#if defined(Q_OS_LINUX)
    QWidget *m_singleCifsExtensionsSettingsWidget;
    KLineEdit *m_additionalCifsOptions;
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QWidget *m_characterSetSettingsWidget;
#endif
};

#endif
