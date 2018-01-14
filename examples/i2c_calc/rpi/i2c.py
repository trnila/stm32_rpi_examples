import smbus
import time

ADDR = 10
BUS = 1


bus = smbus.SMBus(BUS)
while True:
    try:
#        bus.write_byte_data(ADDR, 15, 10)

        print(bus.read_byte_data(ADDR, 15))
    except Exception as e:
        print(e)
    finally:
        time.sleep(1)

