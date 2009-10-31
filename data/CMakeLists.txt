install( FILES smb4ksettings.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS
  authentication1.sh
  network1.sh
  userinterface1.sh
  samba1.sh
  network2.sh
  samba2.sh
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
