# dead_hand
stm32 based relay controller with wifi

Chip: STM32F205RBT6

Wifi: esp-01 (es8266)

Relay: SRD-05VDC-Sl-C

Display: SSD1306, I2C 0.96


# build

build resources.s in resources path. (make)
Add resources.o as object into project

Make sure that TRACE flag does not passed if you not using SWD debug.