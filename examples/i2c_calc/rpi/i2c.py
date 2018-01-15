import smbus
import time
import random

ADDR = 10
BUS = 1


bus = smbus.SMBus(BUS)


while True:
    for i in range(0, 255):
        try:
            bus.write_byte_data(ADDR, i, i + 1)

            rd = random.randint(0, 255)
            print("val {} {}".format(rd, bus.read_byte_data(ADDR, rd)))
        except Exception as e:
            print(e)
        finally:
            time.sleep(0.1)

