#!/bin/sh
for i in `find . -name \*.po` ; do
	msgmerge -U $i $(pwd)/po/smb4k.pot;
done
