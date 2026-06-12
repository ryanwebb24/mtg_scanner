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
    msg = f"{addr}|{cmd}|{data}\n"
    print(f"ATTEMPTING SEND: {msg.strip()}")
    GPIO.output(DE_PIN, GPIO.HIGH)
    time.sleep(0.1)
    ser.reset_output_buffer()
    encoded = msg.encode()
    print(f"WRITING BYTES: {encoded}")
    ser.write(encoded)
    ser.flush()
    time.sleep(0.1)
    GPIO.output(DE_PIN, GPIO.LOW)
    print(f"SENT: {msg.strip()}")

def receive():
    raw = ser.readline()
    print(f"RAW BYTES: {raw}")
    try:
        line = raw.decode('utf-8').strip()
    except UnicodeDecodeError:
        print("  ** decode error, skipping")
        return None
    print(line)
    if not line:
        return None
    parts = line.split('|', 2) # max 2 splits
    if len(parts) != 3:
        print("  ** malformed message")
        return None
    addr, cmd, data = parts
    print(f"  addr: {addr}  cmd: {cmd}  data: {data}")
    return addr, cmd, data

def handle(addr, cmd, data):
    global addr_counter
    print(f"{addr} {cmd} {data}")
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
    print("Listening on /dev/ttyS0...")
    while True:
        result = receive()
        if result:
            handle(*result)
except KeyboardInterrupt:
    print("exiting")
finally:
    GPIO.cleanup()
    ser.close()