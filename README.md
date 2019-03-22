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

#### Status packet

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
| Preamble   |   Button   | MSB x-axis | LSB x-axis | MSB y-axis | LSB y-axis |    FCS     |
+------------+------------+------------+------------+------------+------------+------------+
```

Preamble of a status packet = 0xAA

The following byte represents the "button". Each button on the remote has a bit associated to it.
* A bit sets at 1 means "button pressed".
* A bit sets at 0 means "button not pressed".

Bits [5-7] are always 0.

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

Values for X and Y axis are encoded on 16-Bits to keep the full resolution of the joystick.

* MSB x-axis: The MSB of the X-Axis Value
* LSB x-axis: The LSB of the X-Axis Value
* MSB y-axis: The MSB of the Y-Axis Value
* LSB y-axis: The LSB of the Y-Axis Value

As an example, if an axis value is 0x1234, then

* MSB of axis byte  = 0x12
* LSB of axis byte  = 0x34

The last byte is FCS. It's a simple checksum on all the bytes, including preamble.

#### Keepalive packet

The keepalive packets are sent at regular interval so the receiver can detect when its out of range.
This feature can be disable when compiling the firmware. See the `#define` in the code.
The structure of the packet is very simple. It's a fixed packet with fixed value.

Keepalive sent every 3 seconds
```
+------------+------------+------------+------------+------------+------------+------------+
| Preamble   |    0xAA    |    0x55    |    0xAA    |    0x55    |    0xAA    |    FCS     |
+------------+------------+------------+------------+------------+------------+------------+
```

Preamble = 0xE0

The last byte is FCS. It's a simple checksum on all the bytes, including preamble.

## Authors

* **Daniel Della Noce** - *Initial work* - [ddella](https://github.com/ddella)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Thanks to Bill Westfield for his optiLoader
* Inspiration from intial code of OPEN-SMART. Nice schematic from Jack and Fred Chu.
