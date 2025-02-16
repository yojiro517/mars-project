import socket
import keyboard
import time

# ESP32-S3のIPとポート
ESP32_UDP_IP = "192.168.1.1"
ESP32_UDP_PORT = 55555

# ソケットの設定
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.5)  # 受信待機時間（秒）

def send_command(command):
    """ESP32-S3にコマンドを送信"""
    # コマンドを送信
    sock.sendto(command.encode(), (ESP32_UDP_IP, ESP32_UDP_PORT))
    print(f"Sent: {command}")

    # TコマンドとEコマンドの場合、センサーのデータを受信
    if command == "T":
        try:
            data, addr = sock.recvfrom(1024)  # 最大1024バイトのデータを受信
            print("Received from ESP32-S3:\n" + data.decode())
        except socket.timeout:
            print("No response from ESP32-S3.")
    elif command == "E":
        try:
            data, addr = sock.recvfrom(1024)
            print("Received from ESP32-S3:\n" + data.decode())
        except socket.timeout:
            print("No response from ESP32-S3.")

def main():
    print("Control the ESP32-S3")
    print("W: Forward, S: Backward, A: Left, D: Right, G: Green LED, R: Red LED, T: BME Sensor, E: BNO Sensor, B: Stop, Q: Quit")

    while True:
        try:
            time.sleep(0.1)
            if keyboard.is_pressed('w'):
                send_command("W")
            elif keyboard.is_pressed('s'):
                send_command("S")
            elif keyboard.is_pressed('a'):
                send_command("A")
            elif keyboard.is_pressed('d'):
                send_command("D")
            elif keyboard.is_pressed('r'):
                send_command("R")
            elif keyboard.is_pressed('g'):
                send_command("G")
            elif keyboard.is_pressed('t'):
                send_command("T")
            elif keyboard.is_pressed('e'):
                send_command("E")
            elif keyboard.is_pressed('b'):
                send_command("B")
            elif keyboard.is_pressed('q'):
                print("Exiting...")
                break
            else:
                pass
        except KeyboardInterrupt:
            print("Exiting...")
            break

if __name__ == "__main__":
    main()