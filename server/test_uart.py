import serial
import time
import RPi.GPIO as GPIO

DE_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(DE_PIN, GPIO.OUT)
GPIO.output(DE_PIN, GPIO.LOW)

ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=1)

devices = {}
addr_counter = 1

def send(addr, cmd, data=""):
    msg = f"{addr}:{cmd}:{data}\n"
    GPIO.output(DE_PIN, GPIO.HIGH)
    ser.write(msg.encode())
    ser.flush()
    GPIO.output(DE_PIN, GPIO.LOW)
    print(f"SENT: {msg.strip()}")

def receive():
    line = ser.readline().decode().strip()
    if not line:
        return None
    print(f"RECV: {line}")
    parts = line.split(':')
    if len(parts) != 3:
        print("  ** malformed message")
        return None
    addr, cmd, data = parts
    print(f"  addr: {addr}")
    print(f"  cmd:  {cmd}")
    print(f"  data: {data}")
    return addr, cmd, data

def handle(addr, cmd, data):
    global addr_counter
    if cmd == "REGISTER":
        mac = data
        if mac not in devices:
            assigned = f"{addr_counter:02}"
            devices[mac] = assigned
            addr_counter += 1
            print(f"  ** registered {mac} as addr {assigned}")
            send(mac, "ADDR", assigned)
        else:
            print(f"  ** already registered {mac} as {devices[mac]}")

try:
    print("Listening...")
    while True:
        result = receive()
        if result:
            handle(*result)
except KeyboardInterrupt:
    print("exiting")
finally:
    GPIO.cleanup()
    ser.close()