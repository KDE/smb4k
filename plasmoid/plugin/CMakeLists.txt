# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2012-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/smb4k
  ${CMAKE_BINARY_DIR}/smb4k)
  
add_library(smb4kqmlplugin SHARED)

target_sources(smb4kqmlplugin PRIVATE
  smb4kbookmarkobject.cpp
  smb4kdeclarative.cpp
  smb4knetworkobject.cpp
  smb4kprofileobject.cpp
  smb4kqmlplugin.cpp)

target_link_libraries(smb4kqmlplugin
  Qt6::Core
  Qt6::Qml
  KF6::XmlGui
  smb4kcore
  smb4kdialogs)

install(TARGETS smb4kqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)

