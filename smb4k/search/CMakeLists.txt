include_directories( 
  ${KDE4_INCLUDES} 
  ${KDE4_INCLUDE_DIR} 
  ${QT_INCLUDES} )

########### next target ###############

set( smb4knetworksearch_PART_SRCS 
  smb4knetworksearch.cpp 
  smb4knetworksearch_part.cpp 
  smb4knetworksearchitem.cpp )

kde4_add_plugin( smb4knetworksearch ${smb4knetworksearch_PART_SRCS} )

target_link_libraries( smb4knetworksearch 
  ${KDE4_KDECORE_LIBS} 
  smb4kcore 
  ${KDE4_KPARTS_LIBS} )

install( TARGETS smb4knetworksearch DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  smb4knetworksearch_part.rc DESTINATION ${DATA_INSTALL_DIR}/smb4k )
