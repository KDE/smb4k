#!/bin/bash
$EXTRACTRC `find . -name \*.kcfg -o -name \*.rc` >> ./rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/smb4k.pot
$XGETTEXT `find . -name \*.qml` -L Java --join -o $podir/smb4k.pot
