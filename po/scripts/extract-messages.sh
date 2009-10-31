#! /bin/bash

podir=${podir:-$PWD/po}
files=`find . -name Messages.sh`
dirs=`for i in $files; do echo \`dirname $i\`; done | sort -u`
tmpname="$PWD/messages.log"
EXTRACTRC=${EXTRACTRC:-extractrc}
EXTRACTATTR=${EXTRACTATTR:-extractattr}
PREPARETIPS=${PREPARETIPS:-preparetips}
export EXTRACTRC EXTRACTATTR PREPARETIPS

for subdir in $dirs; do
  # skip Messages.sh files of KDevelop's app templates
  grep '{APPNAMELC}[^ ]*.pot' $subdir/Messages.sh 1>/dev/null && continue

  test -z "$VERBOSE" || echo "Making messages in $subdir"
  (cd $subdir
   ls -1 *.rc *.ui *.ui3 *.ui4 *.kcfg 2> /dev/null | xargs --no-run-if-empty $EXTRACTRC > rc.cpp
   if find . -name \*.c\* -o -name \*.h\* | fgrep -v ".svn" | xargs fgrep -s -q KAboutData ; then
	echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
	echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp
   fi

   # using xgettext 0.15 or later
   ### TODO what --flags param should be used?
   XGETTEXT_FLAGS="--copyright-holder=This_file_is_part_of_KDE --from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address=http://developer.berlios.de/bugs/?group_id=769"
   export XGETTEXT_FLAGS

   if test -f Messages.sh; then
       # Note: Messages.sh is supposed to get the translators' placeholder by rc.cpp
       podir=$podir srcdir=. XGETTEXT_PROGRAM="${XGETTEXT:-xgettext}" XGETTEXT="${XGETTEXT:-xgettext} $XGETTEXT_FLAGS" bash Messages.sh
   fi
   exit_code=$?
   if test "$exit_code" -ne 0; then
       echo "Bash exit code: $exit_code"
   else
       rm -f rc.cpp
   fi
   ) >& $tmpname
   test -s $tmpname && { echo $subdir ; cat "$tmpname"; }
done
rm -f $tmpname

