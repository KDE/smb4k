/***************************************************************************
    smb4kuserinterfaceoptions  -  This configuration page takes care
    of all settings concerning the user interface of Smb4K
                             -------------------
    begin                : Mi Aug 30 2006
    copyright            : (C) 2006-2013 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kuserinterfaceoptionspage.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>

// KDE includes
#include <klocale.h>


Smb4KUserInterfaceOptionsPage::Smb4KUserInterfaceOptionsPage( QWidget *parent )
: KTabWidget( parent )
{
  //
  // Main window and system tray widget
  //
  QWidget *mainwindow_tab         = new QWidget( this );

  QVBoxLayout *main_window_layout = new QVBoxLayout( mainwindow_tab );
  main_window_layout->setSpacing( 5 );
  main_window_layout->setMargin( 0 );
  
  // Bookmarks
  QGroupBox *bookmarks_box        = new QGroupBox( i18n( "Bookmarks" ), mainwindow_tab );

  QGridLayout *bookmarks_layout   = new QGridLayout( bookmarks_box );
  bookmarks_layout->setSpacing( 5 );

  QCheckBox *show_bookmark_label  = new QCheckBox( Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(),
                                    bookmarks_box );
  show_bookmark_label->setObjectName( "kcfg_ShowCustomBookmarkLabel" );

  bookmarks_layout->addWidget( show_bookmark_label, 0, 0, 0 );

  main_window_layout->addWidget( bookmarks_box );
  main_window_layout->addStretch( 100 );

  insertTab( MainWindowSystemTrayTab, mainwindow_tab, i18n( "General Settings" ) );

  //
  // Network browser
  //
  QWidget *network_browser_tab    = new QWidget( this );

  QVBoxLayout *net_browser_layout = new QVBoxLayout( network_browser_tab );
  net_browser_layout->setSpacing( 5 );
  net_browser_layout->setMargin( 0 );
  
  // Behavior
  QGroupBox *behavior_box         = new QGroupBox( i18n( "Behavior" ), network_browser_tab );
  
  QGridLayout *behavior_layout    = new QGridLayout( behavior_box );
  behavior_layout->setSpacing( 5 );
  
  QCheckBox *auto_open            = new QCheckBox( Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(),
                                    behavior_box );
  auto_open->setObjectName( "kcfg_AutoExpandNetworkItems" );
  
  behavior_layout->addWidget( auto_open, 0, 0, 0 );

  // Remote shares
  QGroupBox *remote_shares_box    = new QGroupBox( i18n( "Remote Shares" ), network_browser_tab );

  QGridLayout *shares_layout      = new QGridLayout( remote_shares_box );
  shares_layout->setSpacing( 5 );

  QCheckBox *show_printers        = new QCheckBox( Smb4KSettings::self()->showPrinterSharesItem()->label(),
                                    remote_shares_box );
  show_printers->setObjectName( "kcfg_ShowPrinterShares" );

  QCheckBox *show_hidden          = new QCheckBox( Smb4KSettings::self()->showHiddenSharesItem()->label(),
                                    remote_shares_box );
  show_hidden->setObjectName( "kcfg_ShowHiddenShares" );

  shares_layout->addWidget( show_printers, 0, 0, 0 );
  shares_layout->addWidget( show_hidden, 0, 1, 0 );

  // Columns
  QGroupBox *columns_box          = new QGroupBox( i18n( "Columns" ), network_browser_tab );

  QGridLayout *columns_layout     = new QGridLayout( columns_box );
  columns_layout->setSpacing( 5 );

  QCheckBox *show_type            = new QCheckBox( Smb4KSettings::self()->showTypeItem()->label(),
                                    columns_box );
  show_type->setObjectName( "kcfg_ShowType" );

  QCheckBox *show_ip_address      = new QCheckBox( Smb4KSettings::self()->showIPAddressItem()->label(),
                                    columns_box );
  show_ip_address->setObjectName( "kcfg_ShowIPAddress" );

  QCheckBox *show_comment         = new QCheckBox( Smb4KSettings::self()->showCommentItem()->label(),
                                    columns_box );
  show_comment->setObjectName( "kcfg_ShowComment" );

  columns_layout->addWidget( show_type, 0, 0, 0 );
  columns_layout->addWidget( show_ip_address, 0, 1, 0 );
  columns_layout->addWidget( show_comment, 1, 0, 0 );

  // Network item tooltips
  QGroupBox *network_tooltips_box = new QGroupBox( i18n( "Tooltips" ), network_browser_tab );

  QGridLayout *n_tooltips_layout  = new QGridLayout( network_tooltips_box );
  n_tooltips_layout->setSpacing( 5 );

  QCheckBox *network_tooltip      = new QCheckBox( Smb4KSettings::self()->showNetworkItemToolTipItem()->label(),
                                    network_tooltips_box );
  network_tooltip->setObjectName( "kcfg_ShowNetworkItemToolTip" );

  n_tooltips_layout->addWidget( network_tooltip, 0, 0, 0 );

  net_browser_layout->addWidget( behavior_box );
  net_browser_layout->addWidget( remote_shares_box );
  net_browser_layout->addWidget( columns_box );
  net_browser_layout->addWidget( network_tooltips_box );
  net_browser_layout->addStretch( 100 );

  insertTab( NetworkNeighborhoodTab, network_browser_tab, i18n( "Network Neighborhood" ) );

  //
  // Shares view
  //
  QWidget *shares_view_tab        = new QWidget( this );

  QGridLayout *shares_view_layout = new QGridLayout( shares_view_tab );
  shares_view_layout->setSpacing( 5 );
  shares_view_layout->setMargin( 0 );

  // Shares view
  QGroupBox *view_box             = new QGroupBox( i18n( "View" ), shares_view_tab );

  QGridLayout *view_layout        = new QGridLayout( view_box );
  view_layout->setSpacing( 5 );

  QButtonGroup *shares_buttons    = new QButtonGroup( view_box );

  QRadioButton *shares_icon_view  = new QRadioButton( Smb4KSettings::self()->sharesIconViewItem()->label(),
                                    view_box );
  shares_icon_view->setObjectName( "kcfg_SharesIconView" );

  QRadioButton *shares_list_view  = new QRadioButton( Smb4KSettings::self()->sharesListViewItem()->label(),
                                    view_box );
  shares_list_view->setObjectName( "kcfg_SharesListView" );

  shares_buttons->addButton( shares_icon_view );
  shares_buttons->addButton( shares_list_view );

  QSpacerItem *spacer3 = new QSpacerItem( 7, 7, QSizePolicy::Preferred, QSizePolicy::Fixed );

  QLabel *list_view_label         = new QLabel( i18n( "Settings for the list view:" ), view_box );

  QCheckBox *show_owner           = new QCheckBox( Smb4KSettings::self()->showOwnerItem()->label(),
                                    view_box );
  show_owner->setObjectName( "kcfg_ShowOwner" );

#ifndef Q_OS_FREEBSD
  QCheckBox *show_login           = new QCheckBox( Smb4KSettings::self()->showLoginNameItem()->label(),
                                    view_box );
  show_login->setObjectName( "kcfg_ShowLoginName" );
#endif

  QCheckBox *show_filesystem      = new QCheckBox( Smb4KSettings::self()->showFileSystemItem()->label(),
                                    view_box );
  show_filesystem->setObjectName( "kcfg_ShowFileSystem" );

  QCheckBox *show_free_space      = new QCheckBox( Smb4KSettings::self()->showFreeDiskSpaceItem()->label(),
                                    view_box );
  show_free_space->setObjectName( "kcfg_ShowFreeDiskSpace" );

  QCheckBox *show_used_space      = new QCheckBox( Smb4KSettings::self()->showUsedDiskSpaceItem()->label(),
                                    view_box );
  show_used_space->setObjectName( "kcfg_ShowUsedDiskSpace" );

  QCheckBox *show_total_space     = new QCheckBox( Smb4KSettings::self()->showTotalDiskSpaceItem()->label(),
                                    view_box );
  show_total_space->setObjectName( "kcfg_ShowTotalDiskSpace" );

  QCheckBox *show_usage           = new QCheckBox( Smb4KSettings::self()->showDiskUsageItem()->label(),
                                    view_box );
  show_usage->setObjectName( "kcfg_ShowDiskUsage" );

  view_layout->addWidget( shares_icon_view, 0, 0, 1, 2, 0 );
  view_layout->addWidget( shares_list_view, 1, 0, 1, 2, 0 );
  view_layout->addItem( spacer3, 2, 0, 1, 2 );
  view_layout->addWidget( list_view_label, 3, 0, 1, 2, 0 );
#ifndef Q_OS_FREEBSD
  view_layout->addWidget( show_owner, 4, 0, 0 );
  view_layout->addWidget( show_login, 4, 1, 0 );
  view_layout->addWidget( show_filesystem, 5, 0, 0 );
  view_layout->addWidget( show_free_space, 5, 1, 0 );
  view_layout->addWidget( show_used_space, 6, 0, 0 );
  view_layout->addWidget( show_total_space, 6, 1, 0 );
  view_layout->addWidget( show_usage, 7, 0, 0 );
#else
  view_layout->addWidget( show_owner, 4, 0, 0 );
  view_layout->addWidget( show_filesystem, 4, 1, 0 );
  view_layout->addWidget( show_free_space, 5, 0, 0 );
  view_layout->addWidget( show_used_space, 5, 1, 0 );
  view_layout->addWidget( show_total_space, 6, 0, 0 );
  view_layout->addWidget( show_usage, 6, 1, 0 );
#endif

  // Mounted shares
  QGroupBox *mounted_shares_box   = new QGroupBox( i18n( "Mounted Shares" ), shares_view_tab );

  QGridLayout *mounted_layout     = new QGridLayout( mounted_shares_box );
  mounted_layout->setSpacing( 5 );

  QCheckBox *show_mountpoint      = new QCheckBox( Smb4KSettings::self()->showMountPointItem()->label(),
                                    mounted_shares_box );
  show_mountpoint->setObjectName( "kcfg_ShowMountPoint" );

  QCheckBox *show_all_shares      = new QCheckBox( Smb4KSettings::self()->showAllSharesItem()->label(),
                                    mounted_shares_box );
  show_all_shares->setObjectName( "kcfg_ShowAllShares" );

  mounted_layout->addWidget( show_mountpoint, 0, 0, 0 );
  mounted_layout->addWidget( show_all_shares, 1, 0, 0 );

  // Share tooltips
  QGroupBox *share_tooltips_box   = new QGroupBox( i18n( "Tooltips" ), shares_view_tab );

  QGridLayout *s_tooltips_layout  = new QGridLayout( share_tooltips_box );
  s_tooltips_layout->setSpacing( 5 );

  QCheckBox *show_share_tooltip   = new QCheckBox( Smb4KSettings::self()->showShareToolTipItem()->label(),
                                    share_tooltips_box );
  show_share_tooltip->setObjectName( "kcfg_ShowShareToolTip" );

  s_tooltips_layout->addWidget( show_share_tooltip, 0, 0, 0 );

  QSpacerItem *spacer4 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  shares_view_layout->addWidget( view_box, 0, 0, 0 );
  shares_view_layout->addWidget( mounted_shares_box, 1, 0, 0 );
  shares_view_layout->addWidget( share_tooltips_box, 2, 0, 0 );
  shares_view_layout->addItem( spacer4, 3, 0 );

  insertTab( MountedSharesTab, shares_view_tab, i18n( "Mounted Shares" ) );

  //
  // Preview dialog
  //
  QWidget *preview_tab            = new QWidget( this );

  QGridLayout *preview_layout     = new QGridLayout( preview_tab );
  preview_layout->setSpacing( 5 );
  preview_layout->setMargin( 0 );


  QGroupBox *preview_files_box    = new QGroupBox( i18n( "Hidden Files && Directories" ), preview_tab );

  QGridLayout *prev_files_layout  = new QGridLayout( preview_files_box );
  prev_files_layout->setSpacing( 5 );

  QCheckBox *preview_hidden       = new QCheckBox( Smb4KSettings::self()->previewHiddenItemsItem()->label(),
                                    preview_files_box );
  preview_hidden->setObjectName( "kcfg_PreviewHiddenItems" );

  prev_files_layout->addWidget( preview_hidden, 0, 0, 0 );

  QSpacerItem *spacer5 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  preview_layout->addWidget( preview_files_box, 0, 0, 0 );
  preview_layout->addItem( spacer5, 1, 0, 1, 1, 0 );

  insertTab( PreviewDialogTab, preview_tab, i18n( "Preview Dialog" ) );
}


Smb4KUserInterfaceOptionsPage::~Smb4KUserInterfaceOptionsPage()
{
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


#include "smb4kuserinterfaceoptionspage.moc"
