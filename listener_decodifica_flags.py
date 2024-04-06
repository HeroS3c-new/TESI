from scapy.all import *

message = []

def packet_callback(packet):
    if TCP in packet:
        chk = packet[TCP].flags
        c = chr(int(chk))
        message.append(c)

while True:
    sniff(filter="host 192.168.1.40", prn=packet_callback, timeout=5)

    try:
        print(''.join(message))
    except:
        print('A character was received, but it cannot be decoded')

    message = []
