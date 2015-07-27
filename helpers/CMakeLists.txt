set(smb4kmounthelper_SRCS smb4kmounthelper.cpp)

add_executable(mounthelper ${smb4kmounthelper_SRCS})

target_link_libraries(mounthelper
  Qt5::Core
  KF5::Auth
  KF5::CoreAddons
  KF5::I18n
  KF5::KIOCore)

install(TARGETS mounthelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(mounthelper net.sourceforge.smb4k.mounthelper root)
kauth_install_actions(net.sourceforge.smb4k.mounthelper smb4kmounthelper.actions)


