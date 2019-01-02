install(FILES 
  smb4ksettings.upd 
  smb4k_3_0_0.upd 
  DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(PROGRAMS
  authentication1.sh
  network1.sh
  network2.sh
  samba1.sh
  samba2.sh
  samba3.sh
  userinterface1.sh
  userinterface2.sh
  userinterface3.sh
  userinterface4.sh
  userinterface5.sh
  DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
