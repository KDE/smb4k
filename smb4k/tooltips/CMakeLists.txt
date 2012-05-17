include_directories(
   ${KDE4_INCLUDES}
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}/core/
   ${CMAKE_CURRENT_BINARY_DIR}/core/ )

set( smb4ktooltips_LIB_SRCS
  smb4ktooltip.cpp )
  
kde4_add_library( smb4ktooltips SHARED ${smb4ktooltips_LIB_SRCS} )

target_link_libraries( smb4ktooltips
  ${KDE4_KDECORE_LIBS}
  smb4kcore
  ${KDE4_KDEUI_LIBS} )

install( TARGETS smb4ktooltips DESTINATION ${LIB_INSTALL_DIR} )
