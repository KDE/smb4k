#!/bin/bash

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

$XGETTEXT `find . -name \*.qml` -L Java -o $podir/plasma_applet_org.kde.smb4kqml.pot
