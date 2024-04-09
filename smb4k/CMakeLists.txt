# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2009-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR})

add_subdirectory(icons)

########### Configuration dialog #####################

add_library(smb4kconfigdialog MODULE)

target_sources(smb4kconfigdialog PRIVATE
  smb4kconfigdialog.cpp
  smb4kconfigpageauthentication.cpp
  smb4kconfigpagebookmarks.cpp
  smb4kconfigpagecustomsettings.cpp
  smb4kconfigpagemounting.cpp
  smb4kconfigpagenetwork.cpp
  smb4kconfigpageprofiles.cpp
  smb4kconfigpagesynchronization.cpp
  smb4kconfigpageuserinterface.cpp
  smb4kcustomsettingseditorwidget.cpp)

target_link_libraries(smb4kconfigdialog
  Qt6::Core
  Qt6::Widgets
  Qt6::Gui
  KF6::WidgetsAddons
  KF6::CoreAddons
  KF6::XmlGui
  KF6::IconThemes
  KF6::I18n
  KF6::ConfigCore
  KF6::ConfigGui
  KF6::KIOCore
  KF6::KIOWidgets
  KF6::WindowSystem
  smb4kcore)
  
if (SMB4K_WITH_WS_DISCOVERY)
  target_compile_definitions(smb4kconfigdialog PRIVATE USE_WS_DISCOVERY=${SMB4K_WITH_WS_DISCOVERY})
endif(SMB4K_WITH_WS_DISCOVERY)

########### Dialogs ################################

add_library(smb4kdialogs SHARED)

target_sources(smb4kdialogs PRIVATE
  smb4kbookmarkdialog.cpp
  smb4kbookmarkeditor.cpp
  smb4kconfigpagebookmarks.cpp
  smb4kcustomsettingseditor.cpp
  smb4kcustomsettingseditorwidget.cpp
  smb4khomesuserdialog.cpp
  smb4kmountdialog.cpp
  smb4kpassworddialog.cpp
  smb4kpreviewdialog.cpp
  smb4kprintdialog.cpp
  smb4ksynchronizationdialog.cpp)

target_link_libraries(smb4kdialogs
  Qt6::Core
  Qt6::Widgets
  KF6::Completion
  KF6::ConfigGui
  KF6::CoreAddons
  KF6::I18n
  KF6::IconThemes
  smb4kcore)

target_compile_definitions(smb4kdialogs PRIVATE SMB4KDIALOGS)

########### Main Application #########################

add_executable(smb4k)

target_sources(smb4k PRIVATE
  main.cpp
  smb4k.qrc
  smb4kbookmarkmenu.cpp
  smb4kprofilesmenu.cpp
  smb4kmainwindow.cpp
  smb4knetworkbrowser.cpp
  smb4knetworkbrowseritem.cpp
  smb4knetworkbrowserdockwidget.cpp
  smb4knetworksearchtoolbar.cpp
  smb4ksharesmenu.cpp
  smb4ksharesview.cpp
  smb4ksharesviewitem.cpp
  smb4ksharesviewdockwidget.cpp
  smb4ksystemtray.cpp
  smb4ktooltip.cpp)

target_link_libraries(smb4k
  smb4kcore
  smb4kdialogs
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  KF6::CoreAddons
  KF6::WidgetsAddons
  KF6::DBusAddons
  KF6::XmlGui
  KF6::I18n
  KF6::IconThemes
  KF6::KIOCore
  KF6::ConfigWidgets
  KF6::Crash
  KF6::WindowSystem
  KF6::StatusNotifierItem)

########### install files ###############

install(TARGETS smb4kconfigdialog DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4kdialogs DESTINATION ${KDE_INSTALL_LIBDIR})
install(TARGETS smb4k DESTINATION ${KDE_INSTALL_BINDIR})

install(PROGRAMS org.kde.smb4k.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES org.kde.smb4k.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

