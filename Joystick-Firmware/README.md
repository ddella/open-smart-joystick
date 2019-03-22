# open-smart-joystick

Files to be uploaded the the OPEN-SMART Wireless Remote Control.

1. Open the ".ino" in Arduino IDE.
2. Have a USB-to-serial from your PC to the OPEN-SMART Wireless Remote Control.
3. Select the correct board type (ATmega8) and serial port.
4. Hit the "upload" key at the top-left corner of the IDE.

### Prerequisites

To upload the code, you'll need a USB-to-serial. I used a Sparkfun FTDI basic with the
following connection.

```
----------------------
Sparkfun | OPEN-SMART
----------------------
GND      |   GND     |
5V       |   VIN     |
TXD      |   RX      |
RXI      |   TX      |
DTR      |   DTR     |
----------------------
```

