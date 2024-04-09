# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>
# SPDX-FileCopyrightText: 2010-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core 
  ${CMAKE_BINARY_DIR}/core)

add_executable(mounthelper)

target_sources(mounthelper PRIVATE
  smb4kmounthelper.cpp)

target_link_libraries(mounthelper
  smb4kcore
  Qt6::Core
  KF6::AuthCore
  KF6::CoreAddons
  KF6::I18n
  KF6::KIOCore)

install(TARGETS mounthelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(mounthelper org.kde.smb4k.mounthelper root)
kauth_install_actions(org.kde.smb4k.mounthelper smb4kmounthelper.actions)


