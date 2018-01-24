# Touch
## Requirements
### hardware
- SPI1
- UART1
- penirq GPIO21 to PB0

### software
add `dtoverlay=ads7846,penirq=21,penirq_pull=2,speed=10000` to */boot/config.txt*


## Run
```sh
cd rpi
./run.sh uart
./run.sh input
```

