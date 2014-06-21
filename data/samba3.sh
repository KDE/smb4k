#!/bin/sh

while read line ; do
  if echo "$line" | grep 'NoInodeDataCaching=true' > /dev/null 2> /dev/null ; then
    echo "CacheMode=None"
  else
    echo "$line"
  fi
done

echo "# DELETE [Samba]NoInodeDataCaching