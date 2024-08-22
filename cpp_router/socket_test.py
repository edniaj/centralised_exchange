import socket
import sys

# Create a raw socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_RAW)
except socket.error as msg:
    print(f"Socket creation error: {msg}")
    sys.exit()

# Destination IP and port
dest_ip = '127.0.0.1'  # localhost
dest_port = 8888

# Data to send
data = b"Hello, this is a test message!"

# Construct the packet
packet = data

# Send the packet
try:
    s.sendto(packet, (dest_ip, dest_port))
    print(f"Packet sent successfully to {dest_ip}:{dest_port}")
except socket.error as msg:
    print(f"Failed to send packet: {msg}")
    sys.exit()

# Close the socket
s.close()