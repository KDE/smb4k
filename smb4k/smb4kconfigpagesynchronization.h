/*
    The configuration page for the synchronization options

    SPDX-FileCopyrightText: 2005-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGESYNCHRONIZATION_H
#define SMB4KCONFIGPAGESYNCHRONIZATION_H

// Qt includes
#include <QCheckBox>
#include <QSpinBox>
#include <QTabWidget>

// KDE includes
#include <KLineEdit>
#include <KUrlRequester>

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
    explicit Smb4KConfigPageSynchronization(QWidget *parent = nullptr);

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

protected Q_SLOTS:
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

private:
    KUrlRequester *m_synchronizationPrefix;
    QCheckBox *m_makeBackups;
    QWidget *m_backupSettingsWidget;
    QCheckBox *m_useBackupSuffix;
    KLineEdit *m_backupSuffix;
    QCheckBox *m_useBackupDirectory;
    KUrlRequester *m_backupDirectory;
    QCheckBox *m_useMinimalTransferSize;
    QSpinBox *m_minimalTransferSize;
    QCheckBox *m_useMaximalTransferSize;
    QSpinBox *m_maximalTransferSize;
    QCheckBox *m_usePartialDirectory;
    KUrlRequester *m_partialDirectory;
    QCheckBox *m_useExcludePattern;
    KLineEdit *m_excludePattern;
    QCheckBox *m_useExcludeFrom;
    KUrlRequester *m_excludeFrom;
    QCheckBox *m_useIncludePattern;
    KLineEdit *m_includePattern;
    QCheckBox *m_useIncludeFrom;
    KUrlRequester *m_includeFrom;
    QCheckBox *m_useBlockSize;
    QSpinBox *m_blockSize;
    QWidget *m_compressionSettingsWidget;
    QCheckBox *m_useFFilterRule;
    QCheckBox *m_useFFFilterRule;
};

#endif
