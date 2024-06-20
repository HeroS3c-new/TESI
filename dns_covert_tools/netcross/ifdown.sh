#!/bin/sh
#This script is invoked when a client connects using a tuntap endpoint
#ifdown <cli|srv> <tunN|tapN> <ClientID>
if [ $# -lt 3 ]; then
 exit 1
fi

ifconfig $2 down
