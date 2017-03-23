include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core)
  
set(smb4kqmlplugin_LIB_SRC
  smb4kbookmarkobject.cpp
  smb4kdeclarative.cpp
  smb4kdeclarative_p.cpp
  smb4knetworkobject.cpp
  smb4kprofileobject.cpp
  smb4kqmlplugin.cpp)

add_library(smb4kqmlplugin SHARED ${smb4kqmlplugin_LIB_SRC})

target_link_libraries(smb4kqmlplugin
  Qt5::Core
  Qt5::Qml
  smb4kcore)

install(TARGETS smb4kqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)

