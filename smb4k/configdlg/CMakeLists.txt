include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR}  
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/smb4k 
  ${CMAKE_BINARY_DIR}/smb4k)

set(smb4kconfigdialog_LIB_SRCS
   smb4kauthoptionspage.cpp
   smb4kconfigdialog.cpp
   smb4kcustomoptionspage.cpp
   smb4kmountoptionspage.cpp
   smb4knetworkoptionspage.cpp
   smb4kprofilespage.cpp
   smb4krsyncoptionspage.cpp
   smb4ksambaoptionspage.cpp
   smb4kshareoptionspage.cpp
   smb4kuserinterfaceoptionspage.cpp)

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

install(TARGETS smb4kconfigdialog DESTINATION ${KDE_INSTALL_PLUGINDIR})



