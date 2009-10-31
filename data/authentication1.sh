#!/bin/sh

while read line ; do
  if echo "$line" | grep 'RememberPasswords=' > /dev/null 2> /dev/null ; then
    echo "$line" | sed -e 's/Passwords/Logins/'
  else
    echo "$line"
  fi
done