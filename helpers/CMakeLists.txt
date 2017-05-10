include_directories(
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core )

set( smb4kmounthelper_SRCS smb4kmounthelper.cpp )

kde4_add_executable( mounthelper ${smb4kmounthelper_SRCS} )
target_link_libraries( mounthelper smb4kcore ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )
install( TARGETS mounthelper DESTINATION ${LIBEXEC_INSTALL_DIR} )

kde4_install_auth_helper_files( mounthelper net.sourceforge.smb4k.mounthelper root )
kde4_install_auth_actions( net.sourceforge.smb4k.mounthelper smb4kmounthelper.actions )


