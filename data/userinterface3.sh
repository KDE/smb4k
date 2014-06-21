#!/bin/sh

while read line ; do
  if echo "$line" | grep 'ShowPrinterShares=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Show/Detect/'
  elif echo "$line" | grep 'ShowHiddenShares=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Show/Detect/'
  elif echo "$line" | grep 'PreviewHiddenItems=' > /dev/null 2> /dev/null ; then
    echo "$line"
  fi
done

echo "# DELETE [UserInterface]ShowPrinterShares"
echo "# DELETE [UserInterface]ShowHiddenShares"
echo "# DELETE [UserInterface]PreviewHiddenItems"