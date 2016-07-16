#!/bin/sh

while read line ; do
  if echo "$line" | grep 'SharesIconView=true' > /dev/null 2> /dev/null; then
    echo "SharesViewMode=IconView"
  fi
  if echo "$line" | grep 'SharesListView=true' > /dev/null 2> /dev/null; then
    echo "SharesViewMode=ListView"
  fi
done

echo "# DELETE [UserInterface]SharesIconView"
echo "# DELETE [UserInterface]SharesListView"
