/***************************************************************************
    smb4ktooltip  -  Provides tooltips for Smb4K
                             -------------------
    begin                : Sa Dez 23 2010
    copyright            : (C) 2010-2016 by Alexander Reinholdt
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

#ifndef SMB4KTOOLTIP_H
#define SMB4KTOOLTIP_H

// Qt includes
#include <QtCore/QPoint>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>


// forward declarations
class Smb4KBasicNetworkItem;

class Q_DECL_EXPORT Smb4KToolTip : public QWidget
{
  Q_OBJECT
  
  public:
    enum Parent { 
      NetworkBrowser,
      SharesView,
      UnknownParent
    };
    explicit Smb4KToolTip(QWidget *parent = 0);
    ~Smb4KToolTip();
    void setup(Parent parent, Smb4KBasicNetworkItem *item);
    void update(Parent parent, Smb4KBasicNetworkItem *item);
    void show(const QPoint &pos);
    void hide();
    Smb4KBasicNetworkItem *networkItem() { return m_item; }
    
  protected:
    void paintEvent(QPaintEvent *e);
    
  protected slots:
    void slotHideToolTip();
    
  private:
    Smb4KBasicNetworkItem *m_item;
    QHBoxLayout *m_tip_layout;
    QVBoxLayout *m_info_layout;
    QGridLayout *m_text_layout;
    void setupNetworkBrowserToolTip();
    void updateNetworkBrowserToolTip();
    void setupSharesViewToolTip();
    void updateSharesViewToolTip();
    static void arc(QPainterPath& path,
                    qreal cx, qreal cy,
                    qreal radius, qreal angle,
                    qreal sweepLength);
    QLabel *m_master_browser_label;
    QLabel *m_comment_label;
    QLabel *m_ip_label;
    QLabel *m_mounted_label;
    QLabel *m_size_label;
};

#endif
