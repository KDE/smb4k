#!/bin/sh

while read line ; do
  if echo "$line" | grep 'ShowAllShares=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Show/Detect/'
  fi
done

echo "# DELETE [UserInterface]ShowAllShares"