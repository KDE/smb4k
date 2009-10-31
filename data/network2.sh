#!/bin/sh

while read line ; do
  if echo "$line" | grep 'SearchMethod=' > /dev/null 2> /dev/null ; then
    continue;
  else
    echo "$line"
  fi
done