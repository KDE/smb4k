project( smb4k-qml )

install( DIRECTORY package/
         DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/smb4k-qml 
         PATTERN .svn EXCLUDE )
 
install( FILES package/metadata.desktop
         DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-smb4k-qml.desktop)

add_subdirectory( plugin )