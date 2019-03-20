# open-smart-joystick
Firmware for OPEN-SMART wireless joystick remote

This is a new firmware for the OPEN-SMART wireless joystick. Following is what I added from the original version.

* Keepalive sent from the remote at specific interval
* Two types of packets sent, with fixed length
* Checksum at the end of each packets

To install this firmware into the wireless joystcik remote you need to have a bootloader.<br>

I installed the optiLoader. Follow the instruction at https://github.com/WestfW/OptiLoader<br>
