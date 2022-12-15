#
# Project
#
project(smb4k)

#
# Version
#
set(VERSION_MAJOR 3)
set(VERSION_MINOR 1)
set(VERSION_PATCH 6)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

#
# Minimum required CMake version
#
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

#
# CMake policies
#
cmake_policy(SET CMP0037 NEW)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

#
# Minimum required versions of Qt5 and KF5
#
set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.44.0")

#
# Adjustments
#
if(KF5_VERSION VERSION_GREATER "5.55.0")
    set(AUTHLIB KF5::AuthCore)
else()
    set(AUTHLIB KF5::Auth)
endif()

#
# Options for building Smb4K
#
# Install plasmoid
option(SMB4K_INSTALL_PLASMOID "Install the plasmoid" ON)

# Build with KDSoap WS Discovery client
option(SMB4K_WITH_WS_DISCOVERY "Build with WS-Discovery support for browsing" OFF)

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

ecm_setup_version(${VERSION} VARIABLE_PREFIX SMB4K VERSION_HEADER smb4k_version.h)

# Qt5 modules
find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Gui ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Network ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5PrintSupport ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Qml ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Test ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Widgets ${QT_MIN_VERSION} REQUIRED)

# KF5 modules
find_package(KF5Auth ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Completion ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DocTools ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5JobWidgets ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Notifications ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Solid ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Wallet ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DNSSD ${KF5_MIN_VERSION} REQUIRED)

# Install the plasmoid if desired
if (SMB4K_INSTALL_PLASMOID)
  find_package(KF5Plasma ${KF5_MIN_VERSION})
  set_package_properties(KF5Plasma PROPERTIES TYPE RUNTIME)
endif()

# Find libsmbclient.h
find_package(Libsmbclient REQUIRED MODULE)

# Check that the required smbc_* functions are provided
set(CMAKE_REQUIRED_LIBRARIES ${LIBSMBCLIENT_LIBRARIES})
set(CMAKE_REQUIRED_INCLUDES ${LIBSMBCLIENT_INCLUDE_DIRS})
check_symbol_exists(smbc_setOptionProtocols libsmbclient.h HAVE_SMBC_PROTOCOL)

if (NOT HAVE_SMBC_PROTOCOL)
  message(WARNING "The function smbc_setOptionProtocols() is missing in Samba's client library's header file. The ability to set protocol versions for Samba's client library will be disabled.")
endif()

# Find KDSoap client
if (SMB4K_WITH_WS_DISCOVERY)
    message(STATUS "Building with WS-Discovery support (-DSMB4K_WITH_WS_DISCOVERY=OFF to disable)")
    find_package(KDSoap 1.9.0 REQUIRED)
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
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x060000)
endif()

#
# Make sure that all libraries, plugins, etc. are installed
# into the right place.
#
set(KDE_INSTALL_USE_QT_SYS_PATHS ON CACHE BOOLEAN "Use Qt system paths for installation" FORCE)

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
