import socket
import time
import sys

def send_messages(host, port, retries=5):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            for _ in range(retries):
            
                msg1 = "meow1"                        
                s.sendall(msg1.encode())
                print(f"Sent: {msg1}")
                time.sleep(1)            
    except socket.error as e:
        print(e)
# Usage
if __name__ == "__main__":
    send_messages('127.0.0.1', 8888)
    