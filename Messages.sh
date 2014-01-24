#!/bin/bash
$EXTRACTRC `find . -name \*.kcfg -o -name \*.rc | grep -v 'core/' ` >> ./rc.cpp
$XGETTEXT `find . -name \*.cpp | grep -v 'core/' ` -o $podir/smb4k.pot
