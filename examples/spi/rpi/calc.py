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
    CMD_ADD = 0x10
    CMD_READ = 0x11

    def add(self, num):
        self.spi.xfer([self.CMD_ADD] + self._encode(num))

    def read(self):
        return self.recv(self.CMD_READ)



spi = spidev.SpiDev()
spi.open(BUS, DEVICE)
spi.max_speed_hz = 1000

calc = Calculator(spi)
while True:
    calc.add(1)
    print(calc.read())
    time.sleep(0.1)