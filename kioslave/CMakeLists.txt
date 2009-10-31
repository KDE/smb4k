include_directories(
  ${KDE4_INCLUDES}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core )

set( kio_smb4k_PART_SRCS kio_smb4k.cpp )

kde4_add_plugin( kio_smb4k ${kio_smb4k_PART_SRCS} )

target_link_libraries( kio_smb4k smb4kcore ${KDE4_KIO_LIBS} )

install( TARGETS kio_smb4k DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES smb4k-network.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/dirtree/remote )
install( FILES smb4k.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES smb4k-network.desktop  DESTINATION  ${DATA_INSTALL_DIR}/remoteview )
