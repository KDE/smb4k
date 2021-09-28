# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2016 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

project(smb4kqml)

add_subdirectory(plugin)

plasma_install_package(package org.kde.smb4kqml)
