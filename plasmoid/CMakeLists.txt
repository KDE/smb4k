# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2016-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

project(smb4kqml)

add_subdirectory(plugin)

if (QT_MAJOR_VERSION STREQUAL "6")
  plasma_install_package(package6 org.kde.smb4kqml)
else()
  plasma_install_package(package5 org.kde.smb4kqml)
endif()
