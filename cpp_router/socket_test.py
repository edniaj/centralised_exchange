import socket
import time
import sys

def send_messages(host, port, retries=5):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            for _ in range(retries):
            
                fix_message = (
                    "8=FIX.4.2\x01"  # BeginString
                    "9=145\x01"  # BodyLength
                    "35=D\x01"  # MsgType (D = New Order - Single)
                    "49=SENDER\x01"  # SenderCompID
                    "56=TARGET\x01"  # TargetCompID
                    "34=1\x01"  # MsgSeqNum
                    "52=20230615-12:00:00\x01"  # SendingTime
                    "11=OrderID123\x01"  # ClOrdID
                    "21=1\x01"  # HandlInst
                    "55=AAPL\x01"  # Symbol
                    "54=1\x01"  # Side (1 = Buy)
                    "40=2\x01"  # OrdType (2 = Limit)
                    "44=150.00\x01"  # Price
                    "38=100\x01"  # OrderQty
                    "59=0\x01"  # TimeInForce (0 = Day)
                    "10=128\x01"  # CheckSum
                )        
                fix_message = (
                    "x\x01"
                )
                
                s.sendall(fix_message.encode())
                print(f"Sent: {fix_message}")
                time.sleep(0.5)   
            s.close()         
    except socket.error as e:
        print(e)
# Usage
if __name__ == "__main__":
    send_messages('127.0.0.1', 8888)
    