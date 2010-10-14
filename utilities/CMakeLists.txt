
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set( smb4k_mount_SRCS smb4k_mount.cpp )

kde4_add_executable( smb4k_mount ${smb4k_mount_SRCS} )

target_link_libraries( smb4k_mount  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} )

install( TARGETS smb4k_mount DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set( smb4k_umount_SRCS smb4k_umount.cpp )

kde4_add_executable( smb4k_umount ${smb4k_umount_SRCS} )

target_link_libraries( smb4k_umount  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install( TARGETS smb4k_umount DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set( smb4k_kill_SRCS smb4k_kill.cpp )

kde4_add_executable( smb4k_kill ${smb4k_kill_SRCS} )

target_link_libraries( smb4k_kill  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} )

install( TARGETS smb4k_kill DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set( smb4k_sudowriter_SRCS smb4k_sudowriter.cpp )

kde4_add_executable( smb4k_sudowriter ${smb4k_sudowriter_SRCS} )

target_link_libraries( smb4k_sudowriter  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} )

install( TARGETS smb4k_sudowriter DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############
