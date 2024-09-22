import socket
import time
import sys

def send_logon(host, port, username, password, retries=5):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            
            # Send Logon message
            logon_message = (
                "8=FIX.4.2\x01"  # BeginString
                "9=0\x01"  # BodyLength (placeholder)
                "35=A\x01"  # MsgType (A = Logon)
                "49=SENDER\x01"  # SenderCompID
                "56=TARGET\x01"  # TargetCompID
                "34=1\x01"  # MsgSeqNum
                "52=" + time.strftime("%Y%m%d-%H:%M:%S") + "\x01"  # SendingTime
                "98=0\x01"  # EncryptMethod (0 = None)
                "108=30\x01"  # HeartBtInt
                "553=" + username + "\x01"  # Username
                "554=" + password + "\x01"  # Password
            )
            
            # Calculate and append the checksum
            checksum = sum(ord(c) for c in logon_message) % 256
            logon_message += f"10={checksum:03d}\x01"
            
            # Send the logon message
            s.sendall(logon_message.encode())
            print(f"Sent Logon: {logon_message}")
            
            # Wait for logon response
            response = s.recv(1024).decode()
            print(f"Received: {response}")
            
            # Check if logon was successful (you may need to parse the response)
            if "35=A" not in response:
                print("Authentication failed")
                return
            
            # If authentication successful, proceed with sending other messages
            for _ in range(retries):
                # Your existing FIX message code here
                # ...

# Usage
if __name__ == "__main__":
    send_messages('127.0.0.1', 8888, 'admin', 'password')
