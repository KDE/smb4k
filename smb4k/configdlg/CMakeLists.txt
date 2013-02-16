
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(smb4kconfigdialog_LIB_SRCS
   smb4kauthoptionspage.cpp
   smb4kconfigdialog.cpp
   smb4kcustomoptionspage.cpp
   smb4klaptopsupportoptionspage.cpp
   smb4knetworkoptionspage.cpp
   smb4krsyncoptionspage.cpp
   smb4ksambaoptionspage.cpp
   smb4kshareoptionspage.cpp
   smb4kuserinterfaceoptionspage.cpp )

kde4_add_plugin( smb4kconfigdialog ${smb4kconfigdialog_LIB_SRCS} )

target_link_libraries( smb4kconfigdialog
  ${KDE4_KDECORE_LIBS}
  smb4kcore
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS} )

install(TARGETS smb4kconfigdialog DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############



