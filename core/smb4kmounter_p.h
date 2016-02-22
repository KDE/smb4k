/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KMOUNTER_P_H
#define SMB4KMOUNTER_P_H

// application specific includes
#include "smb4kmounter.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtNetwork/QNetworkConfiguration>
#include <QtNetwork/QNetworkConfigurationManager>

// KDE includes
#include <KCompletion/KLineEdit>

class Smb4KShare;


class Smb4KMountDialog : public QDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent      The parent widget
     */
    explicit Smb4KMountDialog(Smb4KShare *share,
                              QWidget *parent = 0);
    /**
     * The destructor.
     */
    ~Smb4KMountDialog();

    /**
     * Returns if the share should be bookmarked or not.
     *
     * @returns TRUE if the share should be bookmarked
     */
    bool bookmarkShare() { return m_bookmark->isChecked(); }
    
    /**
     * Returns if the user input is valid or not.
     * 
     * @returns TRUE if the user input is valid.
     */
    bool validUserInput() { return m_valid; }

  protected Q_SLOTS:
    /**
     * This slot is activated if the OK button has been clicked.
     */
    void slotOkClicked();

    /**
     * This slot is activated if the Cancel button has been clicked.
     */
    void slotCancelClicked();

    /**
     * This slot is being enabled if there is input text.
     *
     * @param text        The input text.
     */
    void slotChangeInputValue(const QString &text);

    /**
     * This slot is used for making text completion for the share edit line
     * work.
     */
    void slotShareNameEntered();

    /**
     * This slot is used for making text completion for the IP edit line
     * work.
     */
    void slotIPEntered();

    /**
     * This slot is used for making text completion for the workgroup edit
     * line work.
     */
    void slotWorkgroupEntered();

  private:
    /**
     * This function sets up the view.
     */
    void setupView();
    
    /**
     * The Ok button
     */
    QPushButton *m_ok_button;
    
    /**
     * The Cancel button
     */
    QPushButton *m_cancel_button;

    /**
     * The line edit where the user has to enter the share.
     */
    KLineEdit *m_share_input;

    /**
     * The line edit where the user can enter the IP address.
     */
    KLineEdit *m_ip_input;

    /**
     * The line edit where the user can enter the workgroup.
     */
    KLineEdit *m_workgroup_input;

    /**
     * This checkbox determines whether the share should be added to the
     * bookmarks.
     */
    QCheckBox *m_bookmark;

    /**
     * The share that is passed to the mounter.
     */
    Smb4KShare *m_share;
    
    /**
     * Valid user input?
     */
    bool m_valid;
};


class Smb4KMounterPrivate
{
  public:
    int remountTimeout;
    int remountAttempts;
    int timerId;
    Smb4KMountDialog *dialog;
    QList<Smb4KShare *> importedShares;
    QList<Smb4KShare *> retries;
    QList<Smb4KShare *> remounts;
    bool firstImportDone;
//     bool internalReason;
    QString activeProfile;
};


class Smb4KMounterStatic
{
  public:
    Smb4KMounter instance;
};

#endif
