/*
    The configuration page for the synchronization options

    SPDX-FileCopyrightText: 2005-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGESYNCHRONIZATION_H
#define SMB4KCONFIGPAGESYNCHRONIZATION_H

// Qt includes
#include <QTabWidget>

/**
 * This class belongs to the configuration dialog and takes
 * care of the options that can be defined for rsync.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KConfigPageSynchronization : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * The constructor.
     *
     * @param parent        The parent widget
     */
    explicit Smb4KConfigPageSynchronization(QWidget *parent = 0);

    /**
     * The destructor
     */
    ~Smb4KConfigPageSynchronization();
    
    /**
     * Check the settings for problems. Returns TRUE if none were 
     * found and FALSE otherwise.
     *
     * @returns TRUE if all settings are okay.
     */
    bool checkSettings();

protected slots:
    /**
     * This slot is invoked if the "Archive mode" checkbox has been
     * toggled.
     *
     * @param checked       Is TRUE if the checkbox is checked and FALSE otherwise.
     */
    void slotArchiveToggled(bool checked);

    /**
     * This slot is invoked if the --archive option has to be switched
     * off.
     *
     * @param checked       Is FALSE if one of the connected checkboxes is unchecked
     *                      and TRUE otherwise.
     */
    void slotUncheckArchive(bool checked);

    /**
     * This slot is called, when the backup checkbox has been toggled.
     * It enables/disables all other backup options according to the
     * state the backup button is in.
     *
     * @param checked       Is TRUE if the m_backup check box has been
     *                      checked and FALSE otherwise.
     */
    void slotBackupToggled(bool checked);

    /**
     * This slot is called when the compression checkbox has been toggled.
     * It enables/disables all other compression settings according to the
     * state of the compression button.
     *
     * @param checked       TRUE if checked and FALSE otherwise
     */
    void slotCompressToggled(bool checked);

    /**
     * This slot is called when the 'keep partially transferred files' checkbox
     * has been toggled. It enables/disables the dependent settings according
     * to the state of the checkbox.
     *
     * @param checked       TRUE if checked and FALSE otherwise
     */
    void slotKeepPartialToggled(bool checked);

    /**
     * This slot is called if the '-F' shortcut has been toggled.
     * It unchecks the '-F -F' shortcut.
     *
     * @param checked       Is TRUE is m_f_filter is checked and FALSE otherwise.
     */
    void slotFFilterRuleToggled(bool checked);

    /**
     * This slot is called if the '-F -F' shortcut has been toggled.
     * It unchecks the '-F' shortcut.
     *
     * @param checked       Is TRUE is m_ff_filter is checked and FALSE otherwise.
     */
    void slotFFFilterRuleToggled(bool checked);
};

#endif
