#!/bin/bash
ls /usr/include/garbaz/ 2>/dev/null 1>/dev/null
if [ "$?" -ne "0" ];then
sudo mkdir /usr/include/garbaz/
fi 
sudo cp "$(dirname $0)/netlib.h" "/usr/include/garbaz/"
echo "done."
