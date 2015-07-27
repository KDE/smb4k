include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR}  
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core)

set(smb4ktooltips_LIB_SRCS smb4ktooltip.cpp)
  
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

install(TARGETS smb4ktooltips DESTINATION ${KDE_INSTALL_LIBDIR})
