#!/bin/sh
#This script is invoked when a client connects using a tuntap endpoint
#Note this script has a little bug (if clientID>255)... il will be fixed soon
#
#ifup <cli|srv> <tunN|tapN> <ClientID>

if [ $# -lt 3 ]; then
 exit 1
fi

case $2 in
  tun*) if [ "$1" == "cli" ]; then
            ifconfig $2 up 10.1.2.$3 pointopoint 10.1.2.254
	else
	    ifconfig $2 up 10.1.2.254 pointopoint 10.1.2.$3
	 fi;;
  tap*) if [ "$1" == "cli" ]; then
	    ifconfig $2 up 10.1.2.$3 netmask 255.255.255.0
	else
	    ifconfig $2 up 10.1.2.1 netmask 255.255.255.0
	fi;;
esac
