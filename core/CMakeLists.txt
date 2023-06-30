# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2009-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
#
# KDSoap WS Discovery client uses 'signals' and 'slots'
# 
remove_definitions(
  -DQT_NO_KEYWORDS)

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
  smb4knotification.cpp
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
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Gui
  Qt${QT_MAJOR_VERSION}::PrintSupport
  Qt${QT_MAJOR_VERSION}::Network
  KF${QT_MAJOR_VERSION}::ConfigCore
  KF${QT_MAJOR_VERSION}::ConfigGui
  KF${QT_MAJOR_VERSION}::AuthCore
  KF${QT_MAJOR_VERSION}::IconThemes
  KF${QT_MAJOR_VERSION}::I18n
  KF${QT_MAJOR_VERSION}::Completion
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::Solid
  KF${QT_MAJOR_VERSION}::KIOCore
  KF${QT_MAJOR_VERSION}::KIOWidgets
  KF${QT_MAJOR_VERSION}::Notifications
  KF${QT_MAJOR_VERSION}::JobWidgets
  KF${QT_MAJOR_VERSION}::Wallet
  KF${QT_MAJOR_VERSION}::DNSSD
  ${LIBSMBCLIENT_LIBRARIES})
    
#
# KI18N Translation Domain for library
#
add_definitions(-DTRANSLATION_DOMAIN=\"smb4k-core\")

install(TARGETS smb4kcore DESTINATION ${KDE_INSTALL_LIBDIR})

if (SMB4K_WITH_WS_DISCOVERY)
  target_compile_definitions(smb4kcore PRIVATE USE_WS_DISCOVERY=${SMB4K_WITH_WS_DISCOVERY})
  target_link_libraries(smb4kcore KDSoap::WSDiscoveryClient)
endif(SMB4K_WITH_WS_DISCOVERY)


########### install files ###############

install(FILES smb4k.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES smb4k.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

