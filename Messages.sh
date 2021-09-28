#!/bin/bash

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

$EXTRACTRC `find . -name \*.kcfg -o -name \*.rc | grep -v 'core/' ` >> ./rc.cpp
$XGETTEXT `find . -name \*.cpp | grep -v 'core/' ` -o $podir/smb4k.pot
