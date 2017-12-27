import smbus
import time

ADDR = 0x20

# dirs
IODIRA = 0x00
IODIRB = 0x01
# outputs
OLATA = 0x14
OLATB = 0x15
# inputs
GPIOA = 0x12
GPIOB = 0x13
# pull ups
GPPUA = 0x0c
GPPUB = 0x0d


class MCP23017:
    def __init__(self, bus, addr = ADDR):
        self.addr = addr
        self.bus = smbus.SMBus(bus)

    # 0 - output
    # 1 - input
    def setup(self, bank, state):
        self.bus.write_byte_data(self.addr, IODIRA + bank, state)
        time.sleep(0.1)

    def write(self, bank, state):
        self.bus.write_byte_data(self.addr, GPIOA + bank, state)


mcp = MCP23017(1)
mcp.setup(0, 0x00)
mcp.setup(1, 0x00)

MAX = 8
bank = 0
while True:
    for i in range(0, MAX + 1):
        state = 2**i - 1
        mcp.write(bank, state)
        time.sleep(0.05)
        print(state)

    bank = not bank

