# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Pino Toscano <pino@kde.org>
# SPDX-FileCopyrightText: 2009-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

#
# Minimum required CMake version
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

#
# Version
#
set(VERSION_MAJOR 3)
set(VERSION_MINOR 2)
set(VERSION_PATCH 70)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

#
# Project
#
project(smb4k)

#
# CMake policies
#
cmake_policy(SET CMP0037 NEW)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

#
# Minimum required versions of Qt and Frameworks
#
set(QT_MIN_VERSION "5.15.8")
set(KF5_MIN_VERSION "5.103.0")

#
# Options for building Smb4K
#
# Install plasmoid
option(SMB4K_INSTALL_PLASMOID "Install the plasmoid" ON)

# Build with KDSoap WS Discovery client
option(SMB4K_WITH_WS_DISCOVERY "Build with WS-Discovery support for browsing" OFF)

# Compile definitions
# Error out when deprecated functions (in Qt <= 6.0.0) are encountered
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

#
# Required packages and includes
#
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(CheckSymbolExists)
include(KDEClangFormat)

#
# Version
# 
ecm_setup_version(${VERSION} VARIABLE_PREFIX SMB4K VERSION_HEADER smb4k_version.h)

#
# Source code formatting
# 
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

# Qt modules
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS
  Core
  Gui
  Network
  PrintSupport
  Qml
  Widgets
)

# Frameworks modules
find_package(KF${QT_MAJOR_VERSION} ${KF5_MIN_VERSION} REQUIRED COMPONENTS
  Auth
  Completion
  Config
  ConfigWidgets
  CoreAddons
  Crash
  DBusAddons
  DNSSD
  DocTools
  I18n
  IconThemes
  JobWidgets
  KIO
  Notifications
  Solid
  Wallet
  WidgetsAddons
  WindowSystem
  XmlGui
)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(KF6StatusNotifierItem ${KF_MIN_VERSION} REQUIRED)
endif()

# Install the plasmoid if desired
if (SMB4K_INSTALL_PLASMOID)
  find_package(KF${QT_MAJOR_VERSION} COMPONENTS Plasma)
  set_package_properties(KF${QT_MAJOR_VERSION}Plasma PROPERTIES TYPE RUNTIME)
endif()

# Find libsmbclient.h
find_package(Libsmbclient REQUIRED MODULE)

# Check that the required smbc_* functions are provided
set(CMAKE_REQUIRED_LIBRARIES ${LIBSMBCLIENT_LIBRARIES})
set(CMAKE_REQUIRED_INCLUDES ${LIBSMBCLIENT_INCLUDE_DIRS})
check_symbol_exists(smbc_setOptionProtocols libsmbclient.h HAVE_SMBC_PROTOCOL)

if (NOT HAVE_SMBC_PROTOCOL)
  message(FATAL_ERROR "The function smbc_setOptionProtocols() is missing in Samba's client library's header file.")
endif()

# Find KDSoap client
if (SMB4K_WITH_WS_DISCOVERY)
    if (QT_MAJOR_VERSION EQUAL "6")
      set(KDSOAP_PACKAGE KDSoap-qt6)
      set(KDSOAP_VERSION 2.0.0)
    else ()
      set(KDSOAP_PACKAGE KDSoap)
      set(KDSOAP_VERSION 1.9.0)
    endif ()
    message(STATUS "Building with WS-Discovery support (-DSMB4K_WITH_WS_DISCOVERY=OFF to disable)")
    find_package(${KDSOAP_PACKAGE} ${KDSOAP_VERSION} REQUIRED)
    find_package(KDSoapWSDiscoveryClient 0.2 REQUIRED)
elseif(NOT SMB4K_WITH_WS_DISCOVERY)
    message(STATUS "Not building with WS-Discovery support (-DSMB4K_WITH_WS_DISCOVERY=ON to enable)")
endif(SMB4K_WITH_WS_DISCOVERY)

#
# Make sure that Smb4K builds when several custom targets
# with the same name exist (happens in the po directory).
#
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")
  set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS ON)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")

add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055000)

#
# Add subdirectories
#
add_subdirectory(core)
add_subdirectory(helpers)
add_subdirectory(smb4k)
add_subdirectory(data)
add_subdirectory(doc)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")
  ki18n_install(po)
  if(KF5DocTools_FOUND)
    kdoctools_install(po)
  endif(KF5DocTools_FOUND)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")

#
# Make adjustments according to the options set by the user
#
# Plasmoid: Notify the user and add the subdirectory if desired
if (SMB4K_INSTALL_PLASMOID)
  message(STATUS "Installing plasmoid (-DSMB4K_INSTALL_PLASMOID=OFF to disable)")
  add_subdirectory(plasmoid)
elseif(NOT SMB4K_INSTALL_PLASMOID)
  message(STATUS "Not installing plasmoid (-DSMB4K_INSTALL_PLASMOID=ON to enable)")
endif(SMB4K_INSTALL_PLASMOID)

########### install files ###############

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
