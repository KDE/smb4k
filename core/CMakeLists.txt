
include_directories(
  ${KDE4_INCLUDES}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES} )


########### next target ###############

set( smb4kcore_LIB_SRCS
  smb4kauthinfo.cpp
  smb4kbasicnetworkitem.cpp
  smb4kbookmark.cpp
  smb4kbookmarkhandler.cpp
  smb4kcore.cpp
  smb4kcoremessage.cpp
  smb4kglobal.cpp
  smb4kglobal_p.cpp
  smb4khomesshareshandler.cpp
  smb4khost.cpp
  smb4kipaddressscanner.cpp
  smb4kipaddressscanner_p.cpp
  smb4kmounter.cpp
  smb4kmounter_p.cpp
  smb4kpreviewer.cpp
  smb4kpreviewer_p.cpp
  smb4kpreviewitem.cpp
  smb4kprint.cpp
  smb4kprint_p.cpp
  smb4kprintinfo.cpp
  smb4kprocess.cpp
  smb4ksambaoptionshandler.cpp
  smb4ksambaoptionsinfo.cpp
  smb4kscanner.cpp
  smb4kscanner_p.cpp
  smb4ksearch.cpp
  smb4ksearch_p.cpp
  smb4kshare.cpp
  smb4ksolidinterface.cpp
  smb4ksudowriterinterface.cpp
  smb4ksynchronizationinfo.cpp
  smb4ksynchronizer.cpp
  smb4ksynchronizer_p.cpp
  smb4kwalletmanager.cpp
  smb4kworkgroup.cpp )

kde4_add_kcfg_files( smb4kcore_LIB_SRCS smb4ksettings.kcfgc )

kde4_add_library( smb4kcore SHARED ${smb4kcore_LIB_SRCS} )

target_link_libraries( smb4kcore
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_SOLID_LIBS} )

set_target_properties( smb4kcore PROPERTIES VERSION 4.0.0 SOVERSION 4 )
install( TARGETS smb4kcore DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES smb4k.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES
  smb4kauthinfo.h
  smb4kbasicnetworkitem.h
  smb4kbookmark.h
  smb4kbookmarkhandler.h
  smb4kcore.h
  smb4kcoremessage.h
  smb4kdefs.h
  smb4kglobal.h
  smb4khomesshareshandler.h
  smb4khost.h
  smb4kipaddressscanner.h
  smb4kmounter.h
  smb4kpreviewer.h
  smb4kpreviewitem.h
  smb4kprint.h
  smb4kprintinfo.h
  smb4kprocess.h
  smb4ksambaoptionshandler.h
  smb4ksambaoptionsinfo.h
  smb4kscanner.h
  smb4ksearch.h
  smb4kshare.h
  smb4ksolidinterface.h
  smb4ksudowriterinterface.h
  smb4ksynchronizationinfo.h
  smb4ksynchronizer.h
  smb4kwalletmanager.h
  smb4kworkgroup.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )
