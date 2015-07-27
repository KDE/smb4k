include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR}  
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/smb4k 
  ${CMAKE_BINARY_DIR}/smb4k)

set(smb4ksharesview_PART_SRCS
   smb4kshareslistview.cpp
   smb4ksharesiconview.cpp
   smb4ksharesview_part.cpp
   smb4kshareslistviewitem.cpp
   smb4ksharesiconviewitem.cpp)

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

install(TARGETS smb4ksharesview DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES smb4ksharesview_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/smb4k)

