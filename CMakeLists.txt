project( smb4k )

set( VERSION_MAJOR 1 )
set( VERSION_MINOR 2 )
set( VERSION_PATCH 0 )

set( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} )

cmake_minimum_required( VERSION 2.8.9 FATAL_ERROR )

set( QT_MIN_VERSION 4.7.0 )
set( KDE_MIN_VERSION 4.8.0 ) 
find_package( KDE4 REQUIRED )

include(KDE4Defaults)
include(MacroLibrary)
include(FindGettext)
include(ConvenienceLibs.cmake)
include(ConfigureChecks.cmake)

# Make sure that Smb4K builds when several custom targets
# with the same name exist (happens in the po directory).
if ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po" )
  set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS ON)
endif( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po" )

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories( ${KDE4_INCLUDES}
                     ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory( core )
add_subdirectory( helpers )
add_subdirectory( smb4k )
add_subdirectory( data )
add_subdirectory( doc )

########### Plasmoid ####################

option( INSTALL_PLASMOID "Install plasmoid" ON )

if ( ${KDE_VERSION} VERSION_LESS 4.10.0 )
  set( INSTALL_PLASMOID false )
endif ( ${KDE_VERSION} VERSION_LESS 4.10.0 )

if ( INSTALL_PLASMOID )
  message( STATUS "Installing plasmoid (-DINSTALL_PLASMOID=false to disable)" )
  add_subdirectory( plasmoid )
elseif ( NOT INSTALL_PLASMOID )
  if ( ${KDE_VERSION} VERSION_LESS 4.10.0 )
    message( STATUS "Not installing plasmoid (KDE version 4.10 or later required)" )
  else ( ${KDE_VERSION} VERSION_LESS 4.10.0 )
    message( STATUS "Not installing plasmoid (-DINSTALL_PLASMOID=true to enable)" )
  endif ( ${KDE_VERSION} VERSION_LESS 4.10.0 )
endif( INSTALL_PLASMOID )

########### Header files ################

option( INSTALL_HEADER_FILES "Install header files" OFF )

if ( INSTALL_HEADER_FILES )
  message( STATUS "Installing core header files (-DINSTALL_HEADER_FILES=false to disable)" )
elseif ( NOT INSTALL_HEADER_FILES )
  message( STATUS "Not installing core header files (-DINSTALL_HEADER_FILES=true to enable)" )
endif( INSTALL_HEADER_FILES )

########### install files ###############



########### CPack part ##################

set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} )

set( CPACK_SOURCE_PACKAGE_FILE_NAME smb4k-${VERSION}
     CACHE INTERNAL "source package base name" )
set( CPACK_SOURCE_GENERATOR TBZ2 )
set( CPACK_SOURCE_IGNORE_FILES
     "~$"
     "^${PROJECT_SOURCE_DIR}/Messages.sh"
     "^${PROJECT_SOURCE_DIR}/po/scripts/"
     "^${PROJECT_SOURCE_DIR}/*.kdev4" 
     "^${PROJECT_SOURCE_DIR}.*/.svn/" 
     "^${PROJECT_SOURCE_DIR}/build/" )

include( CPack )
