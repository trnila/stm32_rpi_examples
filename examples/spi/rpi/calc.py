import spidev
import time
import itertools
import RPi.GPIO as GPIO

BUS = 0
DEVICE = 1
GPIO_IRQ = 21

class SPIProtocol:
    def __init__(self, spi, gpio_irq):
        self.spi = spi
        self.retry_read = 50000
        self.gpio_irq = gpio_irq

        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.gpio_irq, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    def send(self, cmd, *args):
        data = list(itertools.chain(*[self._encode(i) for i in args]))
        self.spi.xfer([cmd, len(args)] + data)

    def recv(self, cmd):
        for i in range(0, self.retry_read):
            rcv = self.spi.xfer([cmd, 0x00, 0x00, 0x00, 0x00])
            # TODO: at first transfer, there is not updated queue size?
            rcv = self.spi.xfer([cmd, 0x00, 0x00, 0x00, 0x00])

            if rcv[0] == 0:
                return self._decode(rcv[1:])

            GPIO.wait_for_edge(self.gpio_irq, GPIO.RISING)

    def _encode(self, num):
        return [
            num & 0xFF,
            (num >> 8) & 0xFF,
            (num >> 16) & 0xFF,
            (num >> 24) & 0xFF
        ]

    def _decode(self, num):
        return (num[3] << 24) | (num[2] << 16) | (num[1] << 8) | num[0]


class Calculator(SPIProtocol):
    CMD_READ = 0x10
    CMD_SET = 0x11
    CMD_ADD = 0x12
    CMD_SUB = 0x13
    CMD_AND = 0x14
    CMD_OR = 0x15

    def read(self):
        return self.recv(self.CMD_READ)

    def set(self, num):
        self.send(self.CMD_SET, num)

    def add(self, *args):
        self.send(self.CMD_ADD, *args)

    def sub(self, *args):
        self.send(self.CMD_SUB, *args)

    def land(self, *args):
        self.send(self.CMD_AND, *args)

    def lor(self, *args):
        self.send(self.CMD_OR, *args)


spi = spidev.SpiDev()
spi.open(BUS, DEVICE)
spi.max_speed_hz = 1000

calc = Calculator(spi, GPIO_IRQ)

value = 100
add = [1, 2, 3]

calc.set(value)
while True:
    calc.add(*add)
    value += sum(add)

    val = calc.read()
    print(val)

    if val != value:
        print("Expected {}, got {}".format(value, val))

    time.sleep(0.1)

