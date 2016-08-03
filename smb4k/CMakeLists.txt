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
add_subdirectory(configdlg)
add_subdirectory(search)

########### Tooltips #################################

set(smb4ktooltips_LIB_SRCS
  smb4ktooltip.cpp)

add_library(smb4ktooltips SHARED ${smb4ktooltips_LIB_SRCS})

target_link_libraries(smb4ktooltips
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::ConfigWidgets
  KF5::WidgetsAddons
  KF5::IconThemes
  KF5::I18n
  KF5::WindowSystem
  smb4kcore)
  
########### Browser dock widget ######################

set(smb4knetworkbrowser_PART_SRCS
  smb4knetworkbrowser_part.cpp
  smb4knetworkbrowser.cpp
  smb4knetworkbrowseritem.cpp)
  
add_library(smb4knetworkbrowser MODULE ${smb4knetworkbrowser_PART_SRCS})

target_link_libraries(smb4knetworkbrowser
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::I18n
  KF5::IconThemes
  KF5::Parts
  smb4kcore 
  smb4ktooltips)
  
########### Search dock widget #######################

set(smb4knetworksearch_PART_SRCS 
  smb4knetworksearch.cpp 
  smb4knetworksearch_part.cpp 
  smb4knetworksearchitem.cpp)
  
add_library(smb4knetworksearch MODULE ${smb4knetworksearch_PART_SRCS})

target_link_libraries(smb4knetworksearch 
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::I18n
  KF5::IconThemes
  KF5::Parts
  smb4kcore)
  
########### Shares view dock widget ##################

set(smb4ksharesview_PART_SRCS
   smb4ksharesview.cpp
   smb4ksharesview_part.cpp
   smb4ksharesviewitem.cpp)
   
add_library(smb4ksharesview MODULE ${smb4ksharesview_PART_SRCS})

target_link_libraries(smb4ksharesview
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::I18n
  KF5::IconThemes
  KF5::Parts
  KF5::KIOCore
  smb4kcore 
  smb4ktooltips)

########### Main Application #########################

set(smb4k_SRCS
  main.cpp
  smb4kbookmarkmenu.cpp
  smb4kprofilesmenu.cpp
  smb4ksharesmenu.cpp
  smb4kmainwindow.cpp
  smb4ksystemtray.cpp)

add_executable(smb4k ${smb4k_SRCS})

target_link_libraries(smb4k
  Qt5::Core
  Qt5::Widgets
  KF5::CoreAddons
  KF5::DBusAddons
  KF5::XmlGui
  KF5::Parts
  KF5::ConfigWidgets
  smb4kcore)

########### install files ###############

install(TARGETS smb4ktooltips DESTINATION ${KDE_INSTALL_LIBDIR})
install(TARGETS smb4knetworkbrowser DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4knetworksearch DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4ksharesview DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(TARGETS smb4k DESTINATION ${KDE_INSTALL_BINDIR})

install(PROGRAMS smb4k.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES smb4knetworkbrowser_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
install(FILES smb4knetworksearch_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
install(FILES smb4ksharesview_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
install(FILES smb4k_shell.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)

install(FILES smb4k.appdata.xml DESTINATION share/appdata)

