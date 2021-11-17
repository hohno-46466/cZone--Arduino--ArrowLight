#!/bin/sh

# 00doit.sh

# Last update: Wed Oct 27 06:38:35 JST 2021 by @hohno_at_kuimc

# ----------------------------------------------------------

PNAME=$(basename $0)
# echo "$PNAME"

# Never end with "/"
TARGET="sketch_20210906a_74HC595"

# Never end with "/"
DIR1="$HOME/Dropbox/Code/Arduino/CommonWorkArea/hohno-ACS-2021R.Linux"
DIR2="$HOME/GitHub/cZone--Arduino--ArrowLight/src"

# ----------------------------------------------------------

mesg_exit() {
  echo "$1"
  exit $2
}

usage_exit() {
  mesg_exit "usage: $PNAME [up|upload|down|download|unison|diff]" $1
}

[ "x$(which rsync)" = "x" ] && mesg_exit "${PNAME}: rsync is not installed. aborted..." 99
[ "x$(which unison)" = "x" ] && mesg_exit "${PNAME}: unison is not installed. aborted..." 99

# ----------------------------------------------------------

syncType="none"

if [ "x$1" = "x-h" -o "x$1" = "x--help" ]; then
  usage_exit 9
 :
elif [ "x$1" = "xup" -o "x$1" = "xupload" ]; then
  syncType="up"
  shift
  :
elif [ "x$1" = "xdown" -o "x$1" = "xdownload" ]; then
  syncType="down"
  shift
  :
elif [ "x$1" = "xunison" ]; then
  syncType="unison"
  shift
  :
elif [ "x$1" = "xdiff" ]; then
  syncType="diff"
  shift
  :
fi

if [ "x$syncType" = "xnone" ]; then
  usage_exit 9
  :
fi

if [ ! -d "$DIR1/$TARGET" ]; then
  mesg_exit "${PNAME}: Can't find directory ($DIR1/$TARGET)" 2
  :
fi

if [ ! -d "$DIR2/$TARGET" ]; then
  mesg_exit "${PNAME}: Can't find directory ($DIR2/$TARGET)" 3
  :
fi

CDIR=$(pwd)

if [ "x$CDIR" = "x$DIR1" ]; then
  SRCDIR="$DIR1/$TARGET"
  DSTDIR="$DIR2/$TARGET"

elif [ "x$CDIR" = "x$DIR2" ]; then
  SRCDIR="$DIR2/$TARGET"
  DSTDIR="$DIR1/$TARGET"

else
  mesg_exit "${PNAME}: You must change directory to one of $DIR1 or $DIR2" 5
    :
fi

# echo "DEBUG: ($SRCDIR) ($DSTDIR)"
# exit 90

# ----------------------------------------------------------

opts="$opts"
opts="$opts --exclude=${PNAME}"
opts="$opts --exclude=.Spotlight-V100"
opts="$opts --exclude=.TemporaryItems"
opts="$opts --exclude=.fseventsd"
opts="$opts --exclude=.git*"
opts="$opts --exclude='*.swp'"
opts="$opts --exclude='*~'"
opts="$opts --exclude='*.bak'"

# opts="$opts --exclude='*.md'"
# opts="$opts --exclude='*.txt'"

optx=$(echo $opts | sed -e 's/--exclude=/-ignore "Regex /g' -e 's/\(Regex [^ ]*\) /\1" /g' -e 's/$/"/')

# echo "($opts)"
# echo "($optx)"
# exit

# ----------------------------------------------------------

if [ "x$syncType" = "xup" ]; then
  echo "${PNAME}: ${SRCDIR} -> ${DSTDIR}"
  # echo $(echo rsync -avE $@ $opts ${SRCDIR}/ ${DSTDIR})
  eval $(echo rsync -avE $@ $opts ${SRCDIR}/ ${DSTDIR})
  :
elif [ "x$syncType" = "xdown" ]; then
  echo "${PNAME}: ${DSTDIR} -> ${SRCDIR}"
  # echo $(echo rsync -avE $@ $opts ${DSTDIR})/ ${SRCDIR}
  eval $(echo rsync -avE $@ $opts ${DSTDIR})/ ${SRCDIR}
  :
elif [ "x$syncType" = "xunison" ]; then
  echo "${PNAME}: ${DSTDIR} <-> ${SRCDIR}"
  # echo $(echo "unison -batch $SRCDIR $DSTDIR $optx")
  eval $(echo "unison -batch $SRCDIR $DSTDIR $optx")
  :
elif [ "x$syncType" = "xdiff" ]; then
  echo "${PNAME}: Compare ${SRCDIR} and ${DSTDIR}"
  echo "(diff)"
  echo $(echo diff -r -q ${SRCDIR} ${DSTDIR})
  eval $(echo diff -r -q ${SRCDIR} ${DSTDIR})
  echo "(unison)"
  unison -batch -noupdate "$SRCDIR" -noupdate "$DSTDIR" "$SRCDIR" "$DSTDIR"
  :
else
  usage_exit 9
  :
fi

exit 0
