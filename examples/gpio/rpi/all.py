import RPi.GPIO as GPIO
import time
from threading import Thread
from serial import Serial

RECV_PIN = 19
SEND_PIN = 13

count = 0
def handle(arg):
    global count
    count += 1
    print("irq on", arg, count)

def read_serial():
    serial = Serial("/dev/ttyAMA0", baudrate=115200)

    while True:
        c = serial.read().decode('utf-8')
        print("Received from serial", c)

t = Thread(target=read_serial)
t.deamon = True
t.start()

GPIO.setmode(GPIO.BCM)
GPIO.setup(RECV_PIN, GPIO.IN)
GPIO.setup(SEND_PIN, GPIO.OUT)

GPIO.add_event_detect(RECV_PIN, GPIO.RISING, handle)

val = True
while True:
    GPIO.output(SEND_PIN, val)
    val = not val
    time.sleep(0.5)
