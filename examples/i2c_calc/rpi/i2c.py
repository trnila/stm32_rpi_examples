import smbus
import time
import random

ADDR = 10
BUS = 1

class Protocol:
    TRIES = 3

    def __init__(self, bus, addr):
        self.bus = bus
        self.addr = addr

    def send(self, cmd, num):
        for i in range(0, self.TRIES):
            try:
                return self.bus.write_i2c_block_data(self.addr, cmd, self._encode(num))
            except IOError as e:
                time.sleep(0.1)

        raise e

    def recv(self, cmd):
        return self._decode(self.bus.read_i2c_block_data(self.addr, cmd, 4))

    def _encode(self, num):
        return [
            num & 0xFF,
            (num >> 8) & 0xFF,
            (num >> 16) & 0xFF,
            (num >> 24) & 0xFF
        ]

    def _decode(self, num):
        return (num[3] << 24) | (num[2] << 16) | (num[1] << 8) | num[0]

class App(Protocol):
    CMD_READ = 0x10
    CMD_SET = 0x11
    CMD_ADD = 0x12
    CMD_SUB = 0x13
    CMD_AND = 0x14
    CMD_OR = 0x15

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

    def read(self):
        return self.recv(self.CMD_READ)


bus = smbus.SMBus(BUS)
calc = App(bus, ADDR)
calc.set(0)

while True:
    calc.add(1)
    time.sleep(0.1)
    print(calc.read())

    time.sleep(0.1)