# open-smart-joystick

Firmware for OPEN-SMART wireless joystick remote. This is a modified firmware for the
OPEN-SMART wireless joystick. See the picture of the remote [open-smart.png](open-smart.png).

Following is what I added from the original version by OPEN-SMART.

* Keepalive sent from the remote at specific interval.
* Two types of packets sent, with fixed length.
* Simple checksum at the end of each packet.

## Getting Started

These instructions will get you a copy of the new firmware into the OPEN-SMART remote.

### Prerequisites

To install this firmware into the wireless joystcik remote you need to have a bootloader.
The remote uses an ATmega8 MCU. The easiest bootloader to install is the [optiLoader](https://github.com/WestfW/OptiLoader)
To install the optiLoader follow the instruction [here](https://www.electronoobs.com/eng_arduino_OptiLoader.php).

1. Download the .zip file [here](https://github.com/WestfW/OptiLoader).
2. Extract the zip file and open the ".ino" file in Arduino IDE.
3. Make sure you have the connections between your Arduino UNO and the OPEN-SMART remote. See this [file](bootloader/optiLoader.png).
4. Upload the code to the Arduino UNO, open serial monitor and set the baud rate at 19200
5. Upload the code.

You should receive the following message if the bootloader has been successfully burned.

```
OptiLoader Bootstrap programmer.
2011 by Bill Westfield (WestfW)

Target power on! ...
Starting Program Mode [OK]

Reading signature:9307
Searching for image...
  Found "optiboot_atmega8.hex" for atmega8
  Start address at 1E00
  Total bytes read: 482

Setting fuses for programming
  Lock: 3F ABE000  Low: BF ABA000  High: CC ABA800

Programming bootloader: 512 bytes at 0xF00
  Commit Page: F00:4C0F00
  Commit Page: F20:4C0F20
  Commit Page: F40:4C0F40
  Commit Page: F60:4C0F60
  Commit Page: F80:4C0F80
  Commit Page: FA0:4C0FA0
  Commit Page: FC0:4C0FC0
  Commit Page: FE0:4C0FE0

Restoring normal fuses
  Lock: 2F ABE000  Low: BF ABA000  High: CC ABA800

Target power OFF!

Type 'G' or hit RESET for next chip
```

## Frames sent from remote

The remote sends only two types of packets.
1. Packet with preamble of 0xAA are sent only when there's a change on the remote.
Either a button pressed/released or a movement with the joystick.
2. Packet with preamble of 0xE0 are Keepalive.

##### Status packet

The status packet is sent only when there's a change on the remote. It's seven (7) bytes long.

1. Preamble is 0xAA
2. Status byte for the buttons. Each button has a bit associate to it. See below the representation
of each bits.
3. MSB of the x-axis
4. LSB of the x-axis
5. MSB of the y-axis
6. LSB of the y-axis
7. Simple checksum of the whole packet, including the preambule

```
+------------+------------+------------+------------+------------+------------+------------+
| Preamble   |    Key     |   X-Axis H |   X-Axis L |   Y-Axis H |   Y-Axis L |    FCS     |
+------------+------------+------------+------------+------------+------------+------------+
```

Preamble = 0xAA

The following byte represents any of the button. Bits [5-7] are always 0.
```
+------+------+------+------+------+------+------+------+
|  0   |  0   |  0   |  K5  |  K4  |  K3  |  K2  |  K1  |
+------+------+------+------+------+------+------+------+
   b7     b6     b5     b4     b3     b2     b1     b0
```

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
