# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2012-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/smb4k
  ${CMAKE_BINARY_DIR}/smb4k)
  
set(smb4kqmlplugin_LIB_SRC
  smb4kbookmarkobject.cpp
  smb4kdeclarative.cpp
  smb4kdeclarative_p.cpp
  smb4knetworkobject.cpp
  smb4kprofileobject.cpp
  smb4kqmlplugin.cpp)

add_library(smb4kqmlplugin SHARED ${smb4kqmlplugin_LIB_SRC})

target_link_libraries(smb4kqmlplugin
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Qml
  smb4kcore
  smb4kdialogs)

install(TARGETS smb4kqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/smb4k/smb4kqmlplugin)

