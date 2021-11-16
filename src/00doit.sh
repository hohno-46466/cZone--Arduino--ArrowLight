#!/bin/sh

TARGET=sketch_20210906a_74HC595/

DIR1=./${TARGET}
DIR2=$HOME/Dropbox/Code/Arduino/CommonWorkArea/hohno-ACS-2021R.Linux/${TARGET}

unison "$@" ${DIR1} ${DIR2}
