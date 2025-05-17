import sys
import socket
import numpy as np
import cv2
import keyboard
import time
import struct

LOCAL_UDP_IP = '192.168.1.2'
SHARED_UDP_PORT = 50000
# ESP32-S3のIPとポート
ESP32_UDP_IP = "192.168.1.1"
ESP32_UDP_PORT = 55555
reverese_flag = False

# ソケットの設定
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((LOCAL_UDP_IP, SHARED_UDP_PORT))
sock.settimeout(0.5)  # 受信待機時間（秒）

def send_command(command):
    """ESP32-S3にコマンドを送信"""
    # コマンドを送信
    sock.sendto(command.encode(), (ESP32_UDP_IP, ESP32_UDP_PORT))
    print(f"Sent: {command}")
    
def receive_telemetry():
    try:
        data, addr = sock.recvfrom(2048)  # 最大1024バイトのデータを受信
    except socket.timeout:
        # print("No data received")
        return None, None, None
    packet = np.frombuffer(data, dtype=np.uint8)
    packet_number = packet[0]
    check_sum = packet[-1]
    packet = packet[1:-1]
    return packet_number, packet, check_sum
def telemetry_reader(data):
    if len(data) != 13:
        print("Invalid header or footer")
        return
    pressure, temperature, humidity = struct.unpack('<fff', data[1:13])
    print("Received from ESP32-S3:")
    print(f"pressure: {pressure:.2f}")
    print(f"temperature: {temperature:.2f}")
    print(f"humidity: {humidity:.2f}")
def image_decode(packet_number,data,image):
    index = (packet_number-1)//3*12
    rgb = 2-(packet_number-1)%3
    image[index:index+12,:,rgb] = np.reshape(np.ravel(np.array([data//16, data%16]).T),(12,240))*16
    return image
def camera_show(image_rgb):
    cv2.imshow('ESP32S3_Sense',image_rgb)
    cv2.waitKey(1)

def main():
    print("Control the ESP32-S3")
    print("W: Forward, S: Backward, A: Left, D: Right, G: Green LED, R: Red LED, T: BME Sensor, B: Stop, Q: Quit")
    image = np.zeros((240,240,3)).astype(np.uint8)
    while True:
        packet_number, data, check_sum = receive_telemetry()
        try:
            if keyboard.is_pressed('w'):
                send_command("W")
                time.sleep(0.1)
            elif keyboard.is_pressed('s'):
                send_command("S")
                time.sleep(0.1)
            elif keyboard.is_pressed('a'):
                send_command("A")
                time.sleep(0.1)
            elif keyboard.is_pressed('d'):
                send_command("D")
                time.sleep(0.1)
            elif keyboard.is_pressed('r'):
                send_command("R")
                time.sleep(0.1)
            elif keyboard.is_pressed('g'):
                send_command("G")
                time.sleep(0.1)
            elif keyboard.is_pressed('t'):
                send_command("T")
                time.sleep(0.1)
            elif keyboard.is_pressed('b'):
                send_command("B")
                time.sleep(0.1)
            elif keyboard.is_pressed('q'):
                print("Exiting...")
                time.sleep(0.1)
                break
            else:
                pass
        except KeyboardInterrupt:
            print("Exiting...")
            break
        sys.stdout.flush()
        if packet_number is None:
            continue
        elif packet_number == 0x5C:
            telemetry_reader(data)
        elif packet_number == 0xFF:
            camera_show(cv2.rotate(image,cv2.ROTATE_180) if reverese_flag else image)
        else:
            image_decode(packet_number,data,image)

if __name__ == "__main__":
    main()