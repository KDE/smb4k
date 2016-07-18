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
add_subdirectory(browser)
add_subdirectory(sharesview)
add_subdirectory(search)

########### next target ###############

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
  smb4kcore)

########### next target ###############

set(smb4k_SRCS
  main.cpp
  smb4kbookmarkmenu.cpp
  smb4kprofilesmenu.cpp
  smb4ksharesmenu.cpp
  smb4kmainwindow.cpp
  smb4ksystemtray.cpp
  smb4ktooltip.cpp)

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

install(TARGETS smb4k DESTINATION ${KDE_INSTALL_BINDIR})

########### install files ###############

install(TARGETS smb4ktooltips DESTINATION ${KDE_INSTALL_LIBDIR})

install(PROGRAMS smb4k.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES smb4k_shell.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
install(FILES smb4k.appdata.xml DESTINATION share/appdata)

# kde4_install_icons(${ICON_INSTALL_DIR})
