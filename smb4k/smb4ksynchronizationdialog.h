/*
 *  Synchronization dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KSYNCHRONIZATIONDIALOG_H
#define SMB4KSYNCHRONIZATIONDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QDialog>
#include <QLabel>
#include <QPushButton>

// KDE includes
#include <KUrlRequester>

class Q_DECL_EXPORT Smb4KSynchronizationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KSynchronizationDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KSynchronizationDialog();

    /**
     * Set the share. Ideally, this function should be used before
     * the dialog is shown.
     *
     * @param share       The share for which the preview should be
     *                    acquired
     *
     * @returns TRUE if a share was set and false otherwise
     */
    bool setShare(const SharePtr &share);

protected Q_SLOTS:
    void slotSourcePathChanged(const QString &path);
    void slotDestinationPathChanged(const QString &path);
    void slotSwapPaths();
    void slotSynchronize();

private:
    QPushButton *m_synchronizeButton;
    QPushButton *m_swapButton;
    QPushButton *m_cancelButton;
    QLabel *m_descriptionText;
    KUrlRequester *m_sourceInput;
    KUrlRequester *m_destinationInput;
};

#endif
