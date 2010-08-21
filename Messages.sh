#!/bin/bash
$EXTRACTRC `find . -name \*.kcfg -o -name \*.rc` >> ./rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/smb4k.pot
