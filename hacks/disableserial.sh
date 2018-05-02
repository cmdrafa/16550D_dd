#!/bin/bash

# Disables standard serial driver and
#  deletes all its inodes in the /dev/ directory
# 
# IMPORTANT: This is an ugly hack, but:
#    - the serial device is built into the kernel, not a kernel module
#    - to disable it, we would need to configure the kernel with 0 serial
#       ports, but 
#          * compiling the full kernel takes about 30 minutes
#          * ask CICA to upload yet another VM disk image was not an option
#
# In any case, you can go back to square 0, by rebooting the system

# set -x

# step one: disable serial port
/sbin/insmod -f ./disser.ko || exit 1
/sbin/rmmod disser


# step two: delete /dev/ttyS* nodes
device=ttyS

for file in `ls /dev/${device}*`
do
    rm -f ${file}
done

exit 0
