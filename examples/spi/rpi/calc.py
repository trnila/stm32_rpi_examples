import spidev
import time

BUS = 0
DEVICE = 1

spi = spidev.SpiDev()
spi.open(BUS, DEVICE)
spi.max_speed_hz = 1000

class Calculator:
    CMD_ADD = 0x10
    CMD_READ = 0x11

    def add(self, num):
        spi.xfer([self.CMD_ADD, num & 0xFF])

    def read(self):
        return spi.xfer([self.CMD_READ, 0x00])


calc = Calculator()
while True:
    calc.add(1)
    time.sleep(0.5)
    print(calc.read())
    time.sleep(3)