# open-smart-joystick
Firmware for OPEN-SMART wireless joystick remote

This is a new firmware for the OPEN-SMART wireless joystick. Following is what I added from the original version.

* Keepalive sent from the remote at specific interval
* Two types of packets sent, with fixed length
* Checksum at the end of each packets

To install this firmware into the wireless joystcik remote you need to have a bootloader.<br>

I installed the optiLoader. Follow the instruction at https://github.com/WestfW/OptiLoader<br>

## Frame sent from remote, only when there's a change

+------------+------------+------------+------------+------------+------------+------------+
| Preamble   |    Key     |   X-Axis H |   X-Axis L |   Y-Axis H |   Y-Axis L |    FCS     |
+------------+------------+------------+------------+------------+------------+------------+

Preamble = 0xAA

The following byte represents any of the button. Bits [5-7] are always 0.

+------+------+------+------+------+------+------+------+
|  0   |  0   |  0   |  K5  |  K4  |  K3  |  K2  |  K1  |
+------+------+------+------+------+------+------+------+
   b7     b6     b5     b4     b3     b2     b1     b0

K1 pressed = 0b00000001 (0x01)
K2 pressed = 0b00000010 (0x02)
K3 pressed = 0b00000100 (0x04)
K4 pressed = 0b00001000 (0x08)
KZ pressed = 0b00010000 (0x10) (Joystick button)

Values for X or Y axis are encoded on 16-Bits to keep the full resolution of the joystick.

* X-Axis H: The MSB of the X-Axis Value
* X-Axis L: The LSB of the X-Axis Value
* Y-Axis H: The MSB of the Y-Axis Value
* Y-Axis L: The LSB of the Y-Axis Value

As an example, if the axis value is 0x03ff, then

[X-Y]-Axis Low  byte = 0xFF<br>
[X-Y]-Axis High byte = 0x03<br>

The last byte is a FCS on all the bytes, including preamble.
