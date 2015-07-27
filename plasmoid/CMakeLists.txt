project(smb4k-qml)

install(DIRECTORY package/
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/smb4k-qml 
        PATTERN .svn EXCLUDE)
 
install(FILES package/metadata.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-applet-smb4k-qml.desktop)

add_subdirectory(plugin)
