#!/bin/sh

while read line ; do
  if echo "$line" | grep 'ShowLogin=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Login/LoginName/'
  else
    echo "$line"
  fi
done