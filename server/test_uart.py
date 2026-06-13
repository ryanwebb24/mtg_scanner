import serial
import time
import RPi.GPIO as GPIO
import sys
import select

DE_PIN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setup(DE_PIN, GPIO.OUT)
GPIO.output(DE_PIN, GPIO.LOW)

ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=1)

devices = {}
addr_counter = 1
device_ready = False

def send(addr, cmd, data=""):
    msg = f"{addr}|{cmd}|{data}\n"
    GPIO.output(DE_PIN, GPIO.HIGH)
    time.sleep(0.1)
    ser.reset_output_buffer()
    ser.write(msg.encode())
    ser.flush()
    time.sleep(0.1)
    GPIO.output(DE_PIN, GPIO.LOW)

def receive():
    raw = ser.readline()
    try:
        line = raw.decode('utf-8').strip()
    except UnicodeDecodeError:
        return None
    if not line:
        return None
    parts = line.split('|', 2)
    if len(parts) != 3:
        return None
    addr, cmd, data = parts
    return addr, cmd, data

def handle(addr, cmd, data):
    global addr_counter, device_ready
    if cmd == "REG":
        mac = data
        if mac not in devices:
            assigned = f"{addr_counter:02}"
            devices[mac] = assigned
            addr_counter += 1
            send(mac, "ADR", assigned)
            device_ready = True
            print("Device ready — press f/l/r to send commands")
        else:
            device_ready = True

def check_keyboard():
    if select.select([sys.stdin], [], [], 0)[0]:
        key = sys.stdin.read(1)
        addr = list(devices.values())[0] if devices else "01"
        if key == 'f':
            print("sending FWD")
            send(addr, "FWD")
        elif key == 'l':
            print("sending LFT")
            send(addr, "LFT")
        elif key == 'r':
            print("sending RGT")
            send(addr, "RGT")

try:
    print("Listening on /dev/ttyAMA0...")
    while True:
        result = receive()
        if result:
            handle(*result)
        if device_ready:
            check_keyboard()
except KeyboardInterrupt:
    print("exiting")
finally:
    GPIO.cleanup()
    ser.close()