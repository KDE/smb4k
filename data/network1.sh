#!/bin/sh

while read line ; do
  if echo "$line" | grep 'BrowseList=' > /dev/null 2> /dev/null ; then
    if echo "$line" | grep 'LookupDomains' > /dev/null 2> /dev/null ; then
      echo "LookupDomains=true"
    elif echo "$line" | grep 'QueryCurrentMaster' > /dev/null 2> /dev/null ; then
      echo "QueryCurrentMaster=true"
    elif echo "$line" | grep 'QueryCustomMaster' > /dev/null 2> /dev/null ; then
      echo "QueryCustomMaster=true"
    elif echo "$line" | grep 'ScanBroadcastAreas' > /dev/null 2> /dev/null ; then
      echo "ScanBroadcastAreas=true"
    fi
  else
    echo "$line"
  fi
done