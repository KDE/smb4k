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
  smb4kclient.cpp
  smb4kclient_p.cpp
  smb4kcustomoptions.cpp
  smb4kcustomoptionsmanager.cpp
  smb4kcustomoptionsmanager_p.cpp
  smb4kfile.cpp
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
  smb4kprofilemanager.cpp
  smb4kprofilemanager_p.cpp
  smb4kshare.cpp
  smb4ksynchronizer.cpp
  smb4ksynchronizer_p.cpp
  smb4kwalletmanager.cpp
  smb4kwalletmanager_p.cpp
  smb4kworkgroup.cpp)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS 
  smb4ksettings.kcfgc 
  smb4kmountsettings_linux.kcfgc)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "FreeBSD" OR
       ${CMAKE_HOST_SYSTEM_NAME} MATCHES "NetBSD" OR
       ${CMAKE_HOST_SYSTEM_NAME} MATCHES "DragonFly")
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS 
  smb4ksettings.kcfgc 
  smb4kmountsettings_bsd.kcfgc)
else ()
  kconfig_add_kcfg_files(smb4kcore_LIB_SRCS 
  smb4ksettings.kcfgc)
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
  ${AUTHLIB}
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
  KF5::DNSSD
  ${LIBSMBCLIENT_LIBRARIES})

set_target_properties(smb4kcore PROPERTIES VERSION 6.1.0 SOVERSION 6)
install(TARGETS smb4kcore DESTINATION ${KDE_INSTALL_LIBDIR})

if (SMB4K_WITH_WS_DISCOVERY)
  target_compile_definitions(smb4kcore PRIVATE USE_WS_DISCOVERY=${SMB4K_WITH_WS_DISCOVERY})
  target_link_libraries(smb4kcore KDSoap::WSDiscoveryClient)
endif(SMB4K_WITH_WS_DISCOVERY)


########### install files ###############

install(FILES smb4k.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES smb4k.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

