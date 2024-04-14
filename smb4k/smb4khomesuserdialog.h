/*
 *  Dialog to choose a user for a 'homes' share
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KHOMESUSERDIALOG_H
#define SMB4KHOMESUSERDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kdialogs_export.h"

// Qt includes
#include <QDialog>
#include <QLabel>
#include <QPushButton>

// KDE includes
#include <KComboBox>

class SMB4KDIALOGS_EXPORT Smb4KHomesUserDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KHomesUserDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KHomesUserDialog();

    /**
     * Set the 'homes' share. If the share is not a 'homes' share,
     * this function returns FALSE.
     *
     * @param homesShare         The 'homes' share
     */
    bool setShare(SharePtr homesShare);

protected Q_SLOTS:
    void slotHomesUserNameEntered();
    void slotHomesUserNameChanged(const QString &text);
    void slotOkClicked();

private:
    QLabel *m_descriptionText;
    SharePtr m_share;
    KComboBox *m_userNameInput;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif
