
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(smb4ksearchdialog_PART_SRCS smb4ksearchdialog.cpp smb4ksearchdialog_part.cpp smb4ksearchdialogitem.cpp )

kde4_add_plugin(smb4ksearchdialog ${smb4ksearchdialog_PART_SRCS})

target_link_libraries(smb4ksearchdialog  ${KDE4_KDECORE_LIBS} smb4kcore ${KDE4_KPARTS_LIBS} )

install(TARGETS smb4ksearchdialog DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  smb4ksearchdialog_part.rc
         DESTINATION ${DATA_INSTALL_DIR}/smb4k )
