#!/bin/sh

while read line ; do
  if echo "$line" | grep 'InodeDataCaching=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Inode/NoInode/'
  else
    echo "$line"
  fi
done