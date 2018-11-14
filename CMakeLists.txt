project(smb4k)

set(VERSION_MAJOR 2)
set(VERSION_MINOR 9)
set(VERSION_PATCH 70)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Minimum required CMake version
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# CMake policies
cmake_policy(SET CMP0037 NEW)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

# Minimum required versions of Qt5 and KF5
set(QT_MIN_VERSION "5.6.0")
set(KF5_MIN_VERSION "5.25.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)

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
find_package(KF5Parts ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Solid ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Wallet ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED) 

find_package(KF5Plasma ${KF5_MIN_VERSION})
set_package_properties(KF5Plasma PROPERTIES TYPE RUNTIME)

# Find libsmbclient.h
find_package(Libsmbclient REQUIRED MODULE)

# Make sure that Smb4K builds when several custom targets
# with the same name exist (happens in the po directory).
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")
  set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS ON)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Make sure that all libraries, plugins, etc. are installed
# into the right place.
set(KDE_INSTALL_USE_QT_SYS_PATHS ON CACHE BOOLEAN "Use Qt system paths for installation" FORCE)

# Add subdirectories
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

# Install the header files if desired
option(INSTALL_HEADER_FILES "Install header files" OFF)

if (INSTALL_HEADER_FILES)
  message(STATUS "Installing core header files (-DINSTALL_HEADER_FILES=false to disable)")
elseif (NOT INSTALL_HEADER_FILES)
  message(STATUS "Not installing core header files (-DINSTALL_HEADER_FILES=true to enable)")
endif(INSTALL_HEADER_FILES)

# Do not install the plasmoid if not desired
option(INSTALL_PLASMOID "Install the plasmoid" ON)

if (INSTALL_PLASMOID)
  message(STATUS "Installing plasmoid (-DINSTALL_PLASMOID=false to disable)")
  add_subdirectory(plasmoid)
elseif(NOT INSTALL_PLASMOID)
  message(STATUS "Not installing plasmoid (-DINSTALL_PLASMOID=true to enable)")
endif(INSTALL_PLASMOID)

########### install files ###############

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
