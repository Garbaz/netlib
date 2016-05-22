#!/bin/bash
targetfolder="/usr/include/garbaz/"
printf "Will install netlib into the following folder:\n"
printf "$targetfolder\n"
ls "$targetfolder" 2>/dev/null 1>/dev/null
if [ "$?" -ne "0" ];then
mkdir "$targetfolder" || printf "Unable to create folder. Insufficient permissions?\n\n"
fi 
cp "$(dirname $0)/netlib.h" "$targetfolder" || printf "Unable to copy file. Insufficient permissions?\n\n"
printf "DONE.\n\n"
