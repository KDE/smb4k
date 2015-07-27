include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR}  
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/smb4k 
  ${CMAKE_BINARY_DIR}/smb4k)

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

install(TARGETS smb4knetworkbrowser DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES smb4knetworkbrowser_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)
