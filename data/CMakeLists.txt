install( FILES smb4ksettings.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS
  authentication1.sh
  network1.sh
  userinterface1.sh
  samba1.sh
  network2.sh
  samba2.sh
  userinterface2.sh
  userinterface3.sh
  userinterface4.sh
  samba3.sh
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
