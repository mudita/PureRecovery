#!/bin/bash
FILE="build/tests/PureRecovery-test.bin"
DEST="/media/phone"

if [ ! -f $FILE ]; then
    echo "File not found $FILE"
    exit -1
fi

if [ ! -b "/dev/sda1" ]; then 
    echo "Blk dev not found"
    exit -1
fi
pmount -w -u 0000 /dev/sda1 phone 
sudo cp -v $FILE "$DEST/current/update.bin"
pumount "/dev/sda1"
