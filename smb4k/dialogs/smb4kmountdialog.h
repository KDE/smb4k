/***************************************************************************
    smb4kmountdialog  -  This class provides a dialog for mounting shares
    manually.
                             -------------------
    begin                : Mo Nov 29 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KMOUNTDIALOG_H
#define SMB4KMOUNTDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QCheckBox>

// KDE includes
#include <kdialog.h>
#include <klineedit.h>
#include <kdemacros.h>

// application specific includes
#include <core/smb4kshare.h>

/**
 * This class provides a dialog with which you can mount shares "manually".
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KMountDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent      The parent widget
     */
    Smb4KMountDialog( QWidget *parent = 0 );
    /**
     * The destructor.
     */
    ~Smb4KMountDialog();

  protected slots:
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
    void slotChangeInputValue( const QString &text );

    /**
     * This slot is connected to the Smb4KMounter::mounted() signal. It is
     * used to close the dialog.
     *
     * @param share       The share that was mounted.
     */
    void slotShareMounted( Smb4KShare *share );
    
    /**
     * This slot is connected to the Smb4KMounter::aboutToStart() signal.
     * 
     * @param share               The Smb4KShare object
     * 
     * @param process             The process that is about to start
     */
    void slotMounterAboutToStart( Smb4KShare *share,
                                  int process );
    
    /**
     * This slot is connected to the Smb4KMounter::finished() signal.
     * 
     * @param share               The Smb4KShare object
     * 
     * @param process             The process that finished
     */
    void slotMounterFinished( Smb4KShare *share,
                              int process );
                              
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
    Smb4KShare m_share;
};

#endif
