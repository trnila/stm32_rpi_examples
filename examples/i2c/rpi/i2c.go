package main

import (
    "golang.org/x/exp/io/i2c"
    "time"
)

const (
    I2C_ADDR = 0x20
    CMD_IODIRA = 0x00
    CMD_GPIOA = 0x12
)

func i2c_cmd(dev *i2c.Device, cmd, param byte) {
    var req = []byte{cmd, param}

    err := dev.Write(req); if err != nil {
        panic(err)
    }
    time.Sleep(time.Millisecond)
}

func gpio_setup(dev *i2c.Device, bank, setup byte) {
    i2c_cmd(dev, CMD_IODIRA + bank, setup)
}

func gpio_set(dev *i2c.Device, bank, state byte) {
    i2c_cmd(dev, CMD_GPIOA + bank, state)
}

func main() {
	dev, err := i2c.Open(&i2c.Devfs{Dev: "/dev/i2c-1"}, I2C_ADDR); if err != nil {
		panic(err)
	}

    gpio_setup(dev, 0, 0x00)


    const delay = 100 * time.Millisecond
    for {
        gpio_set(dev, 1, 0x00)
        time.Sleep(delay)
        gpio_set(dev, 0, 0xFF)
        time.Sleep(delay)
        gpio_set(dev, 1, 0xFF)
        time.Sleep(delay)
        gpio_set(dev, 0, 0x00)
        time.Sleep(delay)
    }
}