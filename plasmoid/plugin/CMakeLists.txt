include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core)

set(smb4kqmlplugin_LIB_SRC smb4kqmlplugin.cpp)

add_library(smb4kqml SHARED ${smb4kqmlplugin_LIB_SRC})

target_link_libraries(smb4kqml
  Qt5::Core
  Qt5::Qml
  smb4kcore)

install(TARGETS smb4kqml DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/smb4k-qml/contents/plugin/)
install(FILES qmldir DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/smb4k-qml/contents/plugin/)
