set( smb4kmounthelper_SRCS smb4kmounthelper.cpp )

kde4_add_executable( smb4kmounthelper ${smb4kmounthelper_SRCS} )
target_link_libraries( smb4kmounthelper ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )
install( TARGETS smb4kmounthelper DESTINATION ${LIBEXEC_INSTALL_DIR} )

kde4_install_auth_helper_files( smb4kmounthelper de.berlios.smb4k.mounthelper root )
kde4_install_auth_actions( smb4kmounthelper smb4kmounthelper.actions )