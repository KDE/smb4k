
include_directories(
  ${KDE4_INCLUDES}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/../core
  ${CMAKE_CURRENT_BINARY_DIR}/../core )

add_subdirectory( icons )
add_subdirectory( configdlg )
add_subdirectory( dialogs )
add_subdirectory( tooltips )
add_subdirectory( browser )
add_subdirectory( sharesview )
add_subdirectory( search )


########### next target ###############

set(smb4k_SRCS main.cpp smb4kmainwindow.cpp smb4ksystemtray.cpp )

kde4_add_executable( smb4k ${smb4k_SRCS} )

target_link_libraries( smb4k
  ${KDE4_KDECORE_LIBS}
  smb4kcore
  smb4kdialogs
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS} )

install(TARGETS smb4k DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  smb4k.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES  smb4k_shell.rc DESTINATION ${DATA_INSTALL_DIR}/smb4k )

kde4_install_icons( ${ICON_INSTALL_DIR} )
