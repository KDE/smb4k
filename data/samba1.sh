#!/bin/sh

while read line ; do
  if echo "$line" | grep 'RemotePort=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Remote/RemoteSMB/'
  else
    echo "$line"
  fi
done