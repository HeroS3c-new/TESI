from scapy.all import *

def packet_callback(packet):
    if TCP in packet:
        chk = packet[TCP].chksum
        s = ''.join([chr(c) for c in chk])
        print(s)

sniff(filter="host 192.168.1.40", prn=packet_callback)

