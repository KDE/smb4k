#!/bin/bash

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

$EXTRACTRC `find . -name \*.kcfg -o -name \*.rc` >> ./rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/smb4k-core.pot
