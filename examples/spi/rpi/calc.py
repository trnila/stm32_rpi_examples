import spidev
import time

BUS = 0
DEVICE = 1

class SPIProtocol:
    def __init__(self, spi):
        self.spi = spi

    def send(self, cmd, data):
        self.spi.xfer([cmd] + self._encode(data))

    def recv(self, cmd):
        rcv = self.spi.xfer([cmd, 0x00, 0x00, 0x00, 0x00])
        return self._decode(rcv[1:])

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

    def add(self, num):
        self.send(self.CMD_ADD, num)

    def sub(self, num):
        self.send(self.CMD_SUB, num)

    def land(self, num):
        self.send(self.CMD_AND, num)

    def lor(self, num):
        self.send(self.CMD_OR, num)


spi = spidev.SpiDev()
spi.open(BUS, DEVICE)
spi.max_speed_hz = 1000

calc = Calculator(spi)

calc.set(100)
while True:
    calc.add(5)
    val = calc.read()
    print(val)
    time.sleep(0.001)

