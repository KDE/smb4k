include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${LIBSMBCLIENT_INCLUDE_DIRS})

set(smb4kcore_LIB_SRCS  
  smb4kauthinfo.cpp
  smb4kbasicnetworkitem.cpp
  smb4kbookmark.cpp
  smb4kbookmarkhandler.cpp
  smb4kbookmarkhandler_p.cpp
  smb4kcustomoptions.cpp
  smb4kcustomoptionsmanager.cpp
  smb4kcustomoptionsmanager_p.cpp
  smb4kglobal.cpp
  smb4kglobal_p.cpp
  smb4khardwareinterface.cpp
  smb4khardwareinterface_p.cpp
  smb4khomesshareshandler.cpp
  smb4khomesshareshandler_p.cpp
  smb4khost.cpp
  smb4kmounter.cpp
  smb4kmounter_p.cpp
  smb4knotification.cpp
  smb4knotification_p.cpp
  smb4kpreviewer.cpp
  smb4kpreviewer_p.cpp
  smb4kprint.cpp
  smb4kprint_p.cpp
  smb4kprocess.cpp
  smb4kprofilemanager.cpp
  smb4kprofilemanager_p.cpp
  smb4kscanner.cpp
  smb4kscanner_p.cpp
  smb4ksearch.cpp
  smb4ksearch_p.cpp
  smb4kshare.cpp
  smb4ksynchronizer.cpp
  smb4ksynchronizer_p.cpp
  smb4kwalletmanager.cpp
  smb4kwalletmanager_p.cpp
  smb4kworkgroup.cpp)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS smb4ksettings.kcfgc smb4kmountsettings_linux.kcfgc)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "FreeBSD")
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS smb4ksettings.kcfgc smb4kmountsettings_freebsd.kcfgc)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "NetBSD")
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS smb4ksettings.kcfgc smb4kmountsettings_freebsd.kcfgc)
else ()
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS smb4ksettings.kcfgc)
endif ()

add_library(smb4kcore SHARED ${smb4kcore_LIB_SRCS})

target_link_libraries(smb4kcore
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  Qt5::Test
  Qt5::PrintSupport
  Qt5::Network
  Qt5::Qml
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::Auth
  KF5::IconThemes
  KF5::I18n
  KF5::Completion
  KF5::CoreAddons
  KF5::Solid
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::Notifications
  KF5::JobWidgets
  KF5::Wallet
  KF5::XmlGui
  ${LIBSMBCLIENT_LIBRARIES})

set_target_properties(smb4kcore PROPERTIES VERSION 5.1.0 SOVERSION 5)
install(TARGETS smb4kcore DESTINATION ${KDE_INSTALL_LIBDIR})


########### install files ###############

install(FILES smb4k.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES smb4k.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

if (INSTALL_HEADER_FILES)
  install(FILES
    smb4kauthinfo.h
    smb4kbasicnetworkitem.h
    smb4kbookmark.h
    smb4kbookmarkhandler.h
    smb4kcustomoptions.h
    smb4kcustomoptionsmanager.h
    smb4kdeclarative.h
    smb4kglobal.h
    smb4khardwareinterface.h
    smb4khomesshareshandler.h
    smb4khost.h
    smb4kmounter.h
    smb4knotification.h
    smb4kpreviewer.h
    smb4kprint.h
    smb4kprocess.h
    smb4kprofilemanager.h
    smb4kscanner.h
    smb4ksearch.h
    smb4kshare.h
    smb4ksynchronizer.h
    smb4kwalletmanager.h
    smb4kworkgroup.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR})
endif (INSTALL_HEADER_FILES)
