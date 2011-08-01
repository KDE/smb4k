project( smb4k )

set( VERSION 0.10.72 )
set( VERSION_MAJOR 0 )
set( VERSION_MINOR 10 )
set( VERSION_PATCH 72 )

cmake_minimum_required( VERSION 2.6 )

set( QT_MIN_VERSION 4.7.0 )
set( KDE_MIN_VERSION 4.4.0 ) 
find_package( KDE4 REQUIRED )

include(KDE4Defaults)
include(MacroLibrary)
include(FindGettext)
include(ConvenienceLibs.cmake)
include(ConfigureChecks.cmake)

# Disallow in-source builds
# macro_ensure_out_of_source_build( "Smb4K requires an out of source build. Please create a separate build directory and
# run 'cmake [options] <path_to_source_directory>' there." )

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

########### Header files ################

option( INSTALL_HEADER_FILES "Install header files" OFF )

if ( INSTALL_HEADER_FILES )
  message( STATUS "Installing Smb4K's core header files (-DINSTALL_HEADER_FILES=false to disable)" )
elseif ( NOT INSTALL_HEADER_FILES )
  message( STATUS "Not installing Smb4K's core header files (-DINSTALL_HEADER_FILES=true to enable)" )
endif( INSTALL_HEADER_FILES )

########### KIO slave ###################

#option( COMPILE_KIOSLAVE "Compile Smb4K with KIO slave" OFF )
#
#if ( COMPILE_KIOSLAVE )
#  message( STATUS "Building with KIO slave (-DCOMPILE_KIOSLAVE=false to disable)" )
#  add_subdirectory( kioslave )
#elseif( NOT COMPILE_KIOSLAVE )
#  message( STATUS "Building without KIO slave (-DCOMPILE_KIOSLAVE=true to enable)" )
#endif( COMPILE_KIOSLAVE )

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
