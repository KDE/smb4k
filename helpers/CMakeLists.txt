# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>
# SPDX-FileCopyrightText: 2010-2013 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core)

set(smb4kmounthelper_SRCS smb4kmounthelper.cpp)

add_executable(mounthelper ${smb4kmounthelper_SRCS})

target_link_libraries(mounthelper
  smb4kcore
  Qt${QT_MAJOR_VERSION}::Core
  ${AUTHLIB}
  KF5::CoreAddons
  KF5::I18n
  KF5::KIOCore)

install(TARGETS mounthelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(mounthelper org.kde.smb4k.mounthelper root)
kauth_install_actions(org.kde.smb4k.mounthelper smb4kmounthelper.actions)


