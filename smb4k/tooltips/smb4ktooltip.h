/***************************************************************************
    smb4ktooltip  -  Provides tooltips for Smb4K
                             -------------------
    begin                : Sa Dez 23 2010
    copyright            : (C) 2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KTOOLTIP_H
#define SMB4KTOOLTIP_H

// Qt includes
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPoint>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KBasicNetworkItem;

class KDE_EXPORT Smb4KToolTip : public QWidget
{
  Q_OBJECT
  
  public:
    enum Parent { 
      NetworkBrowser,
      SharesView,
      UnknownParent
    };
    Smb4KToolTip( QWidget *parent = 0 );
    ~Smb4KToolTip();
    void show( Smb4KBasicNetworkItem *item, 
               const QPoint &pos );
    void hide();
    Smb4KBasicNetworkItem *networkItem() { return m_item; }
    void update();
    
  signals:
    void aboutToShow( Smb4KBasicNetworkItem *item );
    void aboutToHide( Smb4KBasicNetworkItem *item );
    
  protected:
    void paintEvent( QPaintEvent *e );
    
  protected slots:
    void slotHideToolTip();
    
  private:
    Smb4KBasicNetworkItem *m_item;
    Parent m_parent;
    QHBoxLayout *m_tip_layout;
    QVBoxLayout *m_info_layout;
    QGridLayout *m_text_layout;
    void setupNetworkBrowserToolTip( Smb4KBasicNetworkItem *item );
    void updateNetworkBrowserToolTip();
};

#endif
