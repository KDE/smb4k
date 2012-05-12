/***************************************************************************
    smb4kcustomoptionsmanager_p - Private helper classes for 
    Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONSMANAGER_P_H
#define SMB4KCUSTOMOPTIONSMANAGER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QList>
#include <QtGui/QCheckBox>

// KDE includes
#include <kdialog.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>

class Smb4KCustomOptionsDialog : public KDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsDialog( Smb4KCustomOptions *options,
                                       QWidget *parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptionsDialog();
    
  protected slots:
    void slotSetDefaultValues();
    void slotCheckValues();
    void slotOKClicked();
    
  private:
    void setupView();
    bool defaultValues();
    Smb4KCustomOptions *m_options;
    KIntNumInput *m_smb_port;
#ifndef Q_OS_FREEBSD
    KIntNumInput *m_fs_port;
    KComboBox *m_write_access;
#endif
    KComboBox *m_protocol_hint;
    KComboBox *m_user_id;
    KComboBox *m_group_id;
    QCheckBox *m_kerberos;
};


class Smb4KCustomOptionsManagerPrivate
{
  public:
    QList<Smb4KCustomOptions *> options;
};


class Smb4KCustomOptionsManagerStatic
{
  public:
    Smb4KCustomOptionsManager instance;
};

#endif
