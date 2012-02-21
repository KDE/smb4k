set ( smb4kqmlplugin_LIB_SRC smb4kqmlplugin.cpp )

kde4_add_library( smb4kqml SHARED ${smb4kqmlplugin_LIB_SRC} )

target_link_libraries( smb4kqml
  smb4kcore
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDE4_KDECORE_LIBS} )

install( TARGETS smb4kqml DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/smb4k-qml/contents/plugin/ )
install( FILES qmldir DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/smb4k-qml/contents/plugin/ )
