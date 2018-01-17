import spidev
import time
import itertools

BUS = 0
DEVICE = 1

class SPIProtocol:
    def __init__(self, spi):
        self.spi = spi
        self.retry_read = 50000

    def send(self, cmd, *args):
        data = list(itertools.chain(*[self._encode(i) for i in args]))
        self.spi.xfer([cmd, len(args)] + data)

    def recv(self, cmd):
        for i in range(0, self.retry_read):
            rcv = self.spi.xfer([cmd, 0x00, 0x00, 0x00, 0x00])

            if rcv[0] == 0:
                return self._decode(rcv[1:])

            time.sleep(0.001)

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

calc = Calculator(spi)

value = 100
add = [1, 2, 3]

calc.set(value)
time.sleep(0.1)
while True:
    calc.add(*add)
    value += sum(add)

    val = calc.read()
    print(val)

    if val != value:
        print("Expected {}, got {}".format(value, val))

    time.sleep(0.1)

