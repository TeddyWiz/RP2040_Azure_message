import serial
import threading
import time
import binascii
import sys
#import crc16
import time
import subprocess

line = ''  # 라인 단위로 데이터 가져올 변수
port = '/dev/ttyS0' # 시리얼 포트
baud = 115200  # 시리얼 보드레이트(통신속도)

ser = serial.Serial(port, baud, timeout=3)

alivethread = True

# 쓰레드
def readthread(ser):
    global line
    
    # 쓰레드 종료될때까지 계속 돌림
    while alivethread:
        # 데이터가 있있다면
        for c in ser.read():
            # line 변수에 차곡차곡 추가하여 넣는다.
            line += (chr(c))
            if line.startswith('[') and line.endswith(']'):  # 라인의 끝을 만나면..
                # 데이터 처리 함수로 호출
                print('receive data=' + line)
                # line 변수 초기화
                line = ''

    ser.close()

def crc16arc(msg):
    crc = 0
    for b in msg:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xa001 if crc & 1 else crc >> 1
    return f"{crc:04x}"

def main():

    # 시리얼 읽을 쓰레드 생성
    #thread = threading.Thread(target=readthread, args=(ser,))
    #thread.start()

    #count = 10
    #while count > 0:
    #    strcmd = '[test' + str(count) + ']'
    #    print('send data=' + strcmd)
    #    ser.write(strcmd.encode())
    #    time.sleep(1)
    #    count -= 1
        
    #alivethread = False

    # 1. CMD , 2 file path or message , 3 mode
    print(len(sys.argv))
    if (len(sys.argv) < 2)|(len(sys.argv) == 3) :
        print("cmd Error")
        sys.exit()
    CMD = sys.argv[1]
    
    if CMD == "HH" :
        print("<CMD> <data> <MODE>")
        print("MODE 0 : file path")
        print("MODE 1 : message")
        print("Request python serial_test2.py DE ")
        print("MODE 0 ex) python serial_test2.py DE 10000000e7e061af 1")
        print("MODE 1 ex) python serial_test2.py HA hostaddr.txt 0")
        sys.exit()

    print("CMD : " + CMD)
    if len(sys.argv) == 4:
        CMDMODE = int(sys.argv[3])
        if CMDMODE == 0:
            filepath = sys.argv[2]
            f = open(filepath, 'r')
            DATA = f.read()
            print(DATA)
            print("MODE : File Send")
        else :
            DATA = sys.argv[2] 
            print("MODE : Message Send")   
            print("message : " + DATA)
    else :
        CMDMODE = 3
    
    filename = time.strftime('%Y-%m-%d %H_%M_%S', time.localtime(time.time())) + "_log.txt"
# print("filename : " + filename)
# f = open(filename, 'w')
    #CMD = "ST"
    #DATA = "SUCCESS"

    if CMDMODE !=3 :
        buffer = CMD + DATA
    else :
        buffer = CMD
    lenT = len(buffer)
    #len = 9
    len1 = (lenT>>8)
    len2 = (lenT & 0x00ff)
    
    print(f"type len1 {type(len1)}")
    print(f"type len2 {type(len2)}")
    

    crc = crc16arc(buffer.encode())
    payload_hex = binascii.hexlify(buffer.encode())
    req_cmd = f'{payload_hex.decode()}{crc}03'
    print("req_cmd:", req_cmd)
    ser_data = bytes.fromhex(req_cmd)
    ser.write(bytes(bytearray([0x02])))
    ser.write(bytes(bytearray([len1, len2])))
    ser.write(ser_data)
    #payload_hex = binascii.hexlify(cmd.encode())
    #ser.write(bytes(bytearray([0x02])))
    #ser.write(bytes(bytearray([len1, len2])))
    #ser.write(buffer.encode())
    #ser.write(crc)
    #ser.write(bytes(bytearray([crc1, crc2])))
    #ser.write(bytes(bytearray([0x03])))
    #time.sleep(1)
    recv_data = ser.readline()
    print('recv_data:', recv_data)
#    while(True) :
#        local_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
#        local_time1 = time.time()
#        print(local_time)
#       print(local_time  , file = f)
#        time.sleep(10)
    ser.close()
    if CMDMODE == 0 :
        f.close()
main()
