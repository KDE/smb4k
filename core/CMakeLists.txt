# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2009-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${LIBSMBCLIENT_INCLUDE_DIRS})

add_library(smb4kcore SHARED)

target_sources(smb4kcore PRIVATE
  smb4kbasicnetworkitem.cpp
  smb4kbookmark.cpp
  smb4kbookmarkhandler.cpp
  smb4kclient.cpp
  smb4kclient_p.cpp
  smb4kcustomsettings.cpp
  smb4kcustomsettingsmanager.cpp
  smb4kcredentialsmanager.cpp
  smb4kfile.cpp
  smb4kglobal.cpp
  smb4kglobal_p.cpp
  smb4khardwareinterface.cpp
  smb4khomesshareshandler.cpp
  smb4khost.cpp
  smb4kmounter.cpp 
  smb4knotification.cpp
  smb4kprofilemanager.cpp
  smb4kshare.cpp
  smb4ksynchronizer.cpp
  smb4ksynchronizer_p.cpp
  smb4kworkgroup.cpp)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
  kconfig_add_kcfg_files(smb4kcore
  smb4ksettings.kcfgc 
  smb4kmountsettings_linux.kcfgc)
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "FreeBSD" OR
       ${CMAKE_HOST_SYSTEM_NAME} MATCHES "NetBSD" OR
       ${CMAKE_HOST_SYSTEM_NAME} MATCHES "DragonFly")
  kconfig_add_kcfg_files(smb4kcore
  smb4ksettings.kcfgc 
  smb4kmountsettings_bsd.kcfgc)
else ()
  kconfig_add_kcfg_files(smb4kcore
  smb4ksettings.kcfgc)
endif ()

target_link_libraries(smb4kcore
  Qt6::Core
  Qt6::Widgets
  Qt6::PrintSupport
  Qt6::Network
  KF6::ConfigCore
  KF6::ConfigGui
  KF6::AuthCore
  KF6::IconThemes
  KF6::I18n
  KF6::CoreAddons
  KF6::Solid
  KF6::KIOCore
  KF6::KIOWidgets
  KF6::Notifications
  KF6::JobWidgets
  KF6::Wallet
  KF6::DNSSD
  qt6keychain
  ${LIBSMBCLIENT_LIBRARIES})
    
if (SMB4K_WITH_WS_DISCOVERY)
  target_compile_definitions(smb4kcore PRIVATE USE_WS_DISCOVERY=${SMB4K_WITH_WS_DISCOVERY})
  target_link_libraries(smb4kcore KDSoap::WSDiscoveryClient)
endif(SMB4K_WITH_WS_DISCOVERY)

target_compile_definitions(smb4kcore PRIVATE SMB4KCORE)

add_definitions(-DTRANSLATION_DOMAIN=\"smb4k-core\")

install(TARGETS smb4kcore DESTINATION ${KDE_INSTALL_LIBDIR})

########### install files ###############

install(FILES smb4k.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES smb4k.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

