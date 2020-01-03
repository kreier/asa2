# MicroPython

For those who already start with MicroPython on the ESP8266 as preparation for ASA3 - here is the place to collect files.

## Installation

Instructions are found at [micropython.org](https://docs.micropython.org/en/latest/esp8266/tutorial/intro.html). For example you can check the size of your flash with `esptool.exe --port COM5 --chip auto flash_id`. Use the firmware found at [the download page of micropython.org](http://micropython.org/download#esp8266) and flash the new firmware with

```
esptool.py --port COM5 --baud 460800 write_flash --flash_size=detect 0 esp8266-20200103-v1.12.bin
```

Now you can connect to your esp8266 using uPyCraft or Thonny. For the first one you might want to install [SourceCodePro.ttf](https://github.com/kreier/python2018/raw/master/micropython/SourceCodePro.ttf) for all users to avoid the error message.

## Scan for I2C devices

The code is rather short and simple. Here a scanner for slaves on the I2C bus with ESP8266:

``` py
from machine import Pin, I2C
i2c = I2C(scl=Pin(12), sda=Pin(14), freq=400000) # in esp32 it is 5 and 4
i2c.scan()
```

## OLED display 1306 with 128x64 pixel

You have to install the library on your esp8266.
