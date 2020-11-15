/***************************************************************************
    Private helper classes for Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2020 by Alexander Reinholdt
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

// application specific includes
#include "smb4kcustomoptionsmanager.h"
#include "smb4kglobal.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QDialog>

class Smb4KCustomOptionsDialog : public QDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsDialog(const OptionsPtr &options, QWidget *parent = 0);
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptionsDialog();
    
  protected Q_SLOTS:
    void slotSetDefaultValues();
    void slotCheckValues();
    void slotOKClicked();
    void slotEnableWOLFeatures(const QString &mac);
    void slotCifsExtensionsSupport(bool support);
    
  private:
    void setupView();
    bool checkDefaultValues();
    void setDefaultValues();
    void saveValues();
    OptionsPtr m_options;
};


class Smb4KCustomOptionsManagerPrivate
{
  public:
    QList<OptionsPtr> options;
};


class Smb4KCustomOptionsManagerStatic
{
  public:
    Smb4KCustomOptionsManager instance;
};

#endif
