# MicroPython

For those who already start with MicroPython on the ESP8266 as preparation for ASA3 - here is the place to collect files.

The code is rather short and simple. Here a scanner for slaves on the I2C bus with ESP8266:

```
from machine import Pin, I2C
i2c = I2C(scl=Pin(12), sda=Pin(14), freq=400000) # in esp32 it is 5 and 4
i2c.scan()
```
