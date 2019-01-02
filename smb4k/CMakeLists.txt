include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/configdlg
  ${CMAKE_CURRENT_BINARY_DIR}/configdlg
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR})

add_subdirectory(icons)

########### Configuration dialog #####################

set(smb4kconfigdialog_LIB_SRCS
  smb4kconfigdialog.cpp
  smb4kconfigpageauthentication.cpp
  smb4kconfigpagecustomoptions.cpp
  smb4kconfigpagemounting.cpp
  smb4kconfigpagenetwork.cpp
  smb4kconfigpageprofiles.cpp
  smb4kconfigpagesynchronization.cpp
  smb4kconfigpageuserinterface.cpp)
  
add_library(smb4kconfigdialog MODULE ${smb4kconfigdialog_LIB_SRCS})

target_link_libraries(smb4kconfigdialog
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  KF5::WidgetsAddons
  KF5::CoreAddons
  KF5::XmlGui
  KF5::IconThemes
  KF5::I18n
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::KIOCore
  KF5::KIOWidgets
  smb4kcore)

########### Main Application #########################

set(smb4k_SRCS
  main.cpp
  smb4kbookmarkmenu.cpp
  smb4kprofilesmenu.cpp
  smb4ksharesmenu.cpp
  smb4ksharesview.cpp
  smb4ksharesviewitem.cpp
  smb4ksharesviewdockwidget.cpp
  smb4kmainwindow.cpp
  smb4knetworkbrowser.cpp
  smb4knetworkbrowseritem.cpp
  smb4knetworkbrowserdockwidget.cpp
  smb4knetworksearchtoolbar.cpp
  smb4ksystemtray.cpp
  smb4ktooltip.cpp)

add_executable(smb4k ${smb4k_SRCS})

target_link_libraries(smb4k
  smb4kcore
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::DBusAddons
  KF5::XmlGui
  KF5::I18n
  KF5::IconThemes
  KF5::KIOCore
  KF5::ConfigWidgets
  KF5::Crash
  KF5::WindowSystem)

########### install files ###############

install(TARGETS smb4kconfigdialog DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4k DESTINATION ${KDE_INSTALL_BINDIR})

install(PROGRAMS org.kde.smb4k.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES smb4k_shell.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
install(FILES org.kde.smb4k.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

