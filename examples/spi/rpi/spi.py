import spidev
import time

BUS = 0
DEVICE = 1

PORT = 1 # PORTB
LED = 5
MAX_STEPS = 15

spi = spidev.SpiDev()
spi.open(BUS, DEVICE)
spi.max_speed_hz = 100000

delay = 1
modif = 1
while True:
    if delay >= MAX_STEPS:
        modif = -1
    elif delay <= 1:
        modif = 1

    spi.xfer([PORT, 1 << LED, 0xFF])
    time.sleep(1.0 / delay)
    spi.xfer([PORT, 1 << LED, 0x00])
    time.sleep(1.0 / delay)
    print(1.0 / delay)

    delay += 1 * modif
