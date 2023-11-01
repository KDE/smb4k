# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2009-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR})

add_subdirectory(icons)

########### Configuration dialog #####################

set(smb4kconfigdialog_LIB_SRCS
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
  
add_library(smb4kconfigdialog MODULE ${smb4kconfigdialog_LIB_SRCS})

target_link_libraries(smb4kconfigdialog
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Gui
  KF${QT_MAJOR_VERSION}::WidgetsAddons
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::XmlGui
  KF${QT_MAJOR_VERSION}::IconThemes
  KF${QT_MAJOR_VERSION}::I18n
  KF${QT_MAJOR_VERSION}::ConfigCore
  KF${QT_MAJOR_VERSION}::ConfigGui
  KF${QT_MAJOR_VERSION}::KIOCore
  KF${QT_MAJOR_VERSION}::KIOWidgets
  KF${QT_MAJOR_VERSION}::WindowSystem
  smb4kcore)
  
if (SMB4K_WITH_WS_DISCOVERY)
  target_compile_definitions(smb4kconfigdialog PRIVATE USE_WS_DISCOVERY=${SMB4K_WITH_WS_DISCOVERY})
endif(SMB4K_WITH_WS_DISCOVERY)

########### Dialogs ################################

set(smb4kdialogs_LIB_SRCS
  smb4kbookmarkdialog.cpp
  smb4kbookmarkeditor.cpp
  smb4kconfigpagebookmarks.cpp
  smb4kcustomsettingseditor.cpp
  smb4kcustomsettingseditorwidget.cpp
  smb4kmountdialog.cpp
  smb4kpassworddialog.cpp
  smb4kpreviewdialog.cpp
  smb4kprintdialog.cpp
  smb4ksynchronizationdialog.cpp)

add_library(smb4kdialogs SHARED ${smb4kdialogs_LIB_SRCS})

target_link_libraries(smb4kdialogs
  smb4kcore
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Widgets
  KF${QT_MAJOR_VERSION}::Completion
  KF${QT_MAJOR_VERSION}::ConfigGui
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::I18n
  KF${QT_MAJOR_VERSION}::IconThemes)

########### Main Application #########################

set(smb4k_SRCS
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
  
add_executable(smb4k ${smb4k_SRCS})

target_link_libraries(smb4k
  smb4kcore
  smb4kdialogs
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Gui
  Qt${QT_MAJOR_VERSION}::Widgets
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::WidgetsAddons
  KF${QT_MAJOR_VERSION}::DBusAddons
  KF${QT_MAJOR_VERSION}::XmlGui
  KF${QT_MAJOR_VERSION}::I18n
  KF${QT_MAJOR_VERSION}::IconThemes
  KF${QT_MAJOR_VERSION}::KIOCore
  KF${QT_MAJOR_VERSION}::ConfigWidgets
  KF${QT_MAJOR_VERSION}::Crash
  KF${QT_MAJOR_VERSION}::WindowSystem)

if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(smb4k KF6::StatusNotifierItem)
endif()

########### install files ###############

install(TARGETS smb4kconfigdialog DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4kdialogs DESTINATION ${KDE_INSTALL_LIBDIR})
install(TARGETS smb4k DESTINATION ${KDE_INSTALL_BINDIR})

install(PROGRAMS org.kde.smb4k.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES org.kde.smb4k.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

