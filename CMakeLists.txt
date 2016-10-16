project(smb4k)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
set(VERSION_PATCH 75)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
cmake_policy(SET CMP0037 NEW)
cmake_policy(SET CMP0063 NEW)

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)

# Qt5 modules
find_package(Qt5Core 5.4.0 REQUIRED)
find_package(Qt5Gui 5.4.0 REQUIRED)
find_package(Qt5Widgets 5.4.0 REQUIRED)
find_package(Qt5Qml 5.4.0 REQUIRED)
find_package(Qt5Test 5.4.0 REQUIRED)
find_package(Qt5Network 5.4.0 REQUIRED)
find_package(Qt5PrintSupport 5.4.0 REQUIRED)

# KF5 modules
find_package(KF5Config 5.9.0 REQUIRED)
find_package(KF5Auth 5.9.0 REQUIRED)
find_package(KF5DocTools 5.9.0 REQUIRED)
find_package(KF5IconThemes 5.9.0 REQUIRED)
find_package(KF5WidgetsAddons 5.9.0 REQUIRED)
find_package(KF5I18n 5.9.0 REQUIRED)
find_package(KF5Completion 5.9.0 REQUIRED)
find_package(KF5CoreAddons 5.9.0 REQUIRED)
find_package(KF5Solid 5.9.0 REQUIRED)
find_package(KF5KIO 5.9.0 REQUIRED)
find_package(KF5Notifications 5.9.0 REQUIRED)
find_package(KF5XmlGui 5.9.0 REQUIRED)
find_package(KF5JobWidgets 5.9.0 REQUIRED)
find_package(KF5Wallet 5.9.0 REQUIRED)
find_package(KF5DBusAddons 5.9.0 REQUIRED)
find_package(KF5Parts 5.9.0 REQUIRED)
find_package(KF5ConfigWidgets 5.9.0 REQUIRED)
find_package(KF5Notifications 5.9.0 REQUIRED)
find_package(KF5WindowSystem 5.9.0 REQUIRED)
find_package(KF5Plasma 5.9.0 REQUIRED)

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

########### Plasmoid ####################

# option(INSTALL_PLASMOID "Install plasmoid" ON)
# 
# if (INSTALL_PLASMOID)
#   message(STATUS "Installing plasmoid (-DINSTALL_PLASMOID=false to disable)")
#   add_subdirectory(plasmoid)
# elseif (NOT INSTALL_PLASMOID)
#   message(STATUS "Not installing plasmoid (-DINSTALL_PLASMOID=true to enable)")
# endif(INSTALL_PLASMOID)

########### Header files ################

option(INSTALL_HEADER_FILES "Install header files" OFF)

if (INSTALL_HEADER_FILES)
  message(STATUS "Installing core header files (-DINSTALL_HEADER_FILES=false to disable)")
elseif (NOT INSTALL_HEADER_FILES)
  message(STATUS "Not installing core header files (-DINSTALL_HEADER_FILES=true to enable)")
endif(INSTALL_HEADER_FILES)

########### install files ###############

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
