#!/bin/sh

while read line ; do
  if echo "$line" | grep 'SharesView=' > /dev/null 2> /dev/null ; then
    if echo "$line" | grep 'IconView' > /dev/null 2> /dev/null ; then
      echo "SharesIconView=true"
    elif echo "$line" | grep 'ListView' > /dev/null 2> /dev/null ; then
      echo "SharesListView=true"
    else
      echo "$line"
    fi
  fi
done