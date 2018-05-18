import smbus
import time
bus = smbus.SMBus(1)
address = 0x20 

print("Write byte 0xAB")
bus.write_byte_data(address, 15, 0xAB)
print("Read byte back")


r = bus.read_byte_data(address, 15)
print("Received {}: {}".format(hex(r), "OK" if r == 0xAB else "ERROR"))


data = [9, 2, 3, 4, 5]
print("Sending {} block of data".format(data))
bus.write_i2c_block_data(address, 1, data)
print("Reading block of data back")
rcv = bus.read_i2c_block_data(address, 1, len(data))
print("Received block {} {}".format(rcv, "OK" if data == rcv else "ERROR"))


for i in range(0, 16):
    bus.write_byte_data(address, i, i + 100)

for i in range(0, 16):
    r = bus.read_byte_data(address, i)
    print(r)


rcv = bus.read_i2c_block_data(address, 12, 8)
print(rcv)
