
include_directories( 
  ${KDE4_INCLUDES} 
  ${KDE4_INCLUDE_DIR} 
  ${QT_INCLUDES} 
  ${CMAKE_CURRENT_SOURCE_DIR}/smb4k 
  ${CMAKE_CURRENT_BINARY_DIR}/smb4k )


########### next target ###############

set(smb4knetworkbrowser_PART_SRCS
  smb4knetworkbrowser_part.cpp
  smb4knetworkbrowser.cpp
  smb4knetworkbrowseritem.cpp
  )

kde4_add_plugin( smb4knetworkbrowser ${smb4knetworkbrowser_PART_SRCS} )

target_link_libraries( smb4knetworkbrowser  
  ${KDE4_KDECORE_LIBS} 
  smb4kcore 
  smb4ktooltips
  ${KDE4_KPARTS_LIBS} )

install(TARGETS smb4knetworkbrowser DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  smb4knetworkbrowser_part.rc
         DESTINATION ${DATA_INSTALL_DIR}/smb4k )
