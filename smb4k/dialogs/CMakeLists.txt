include_directories(
   ${KDE4_INCLUDES}
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}/core/
   ${CMAKE_CURRENT_BINARY_DIR}/core/ )

########### next target ###############

set( smb4kdialogs_LIB_SRCS
  smb4kbookmarkdialog.cpp
  smb4kbookmarkeditor.cpp
  smb4kcustomoptionsdialog.cpp
  smb4kmountdialog.cpp
  smb4kpreviewdialog.cpp
  smb4kprintdialog.cpp
  smb4ksynchronizationdialog.cpp )

kde4_add_library( smb4kdialogs SHARED ${smb4kdialogs_LIB_SRCS} )

target_link_libraries( smb4kdialogs
  ${KDE4_KDECORE_LIBS}
  smb4kcore
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS} )

install(TARGETS smb4kdialogs DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
