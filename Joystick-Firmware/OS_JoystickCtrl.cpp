/*
 * OS_JoystickCtrl - Library for reading buttons and joystick from the OPEN-SMART
 * Wireless Koystick Remote.
 * 
 * I started with the original "WirelessTransmitterFirmware" apparently made by Fred Chu.
 * 
 * Created by Daniel Della Noce, March, 2019
 * Released into the public domain.
 * Licence: GNU GENERAL PUBLIC LICENSE V2.0
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * There's only two different types of frame sent by the remote.
 * 
 * 1. Wireless Joystick status frame. It contains the status of all the buttons,
 *    x-axis and y-axis value.
 *
 * 2. Keepalive frame. Sent to the remote to make sure it stills sees the joystick.
 * 
 * Frame from remote wireless joystick. Frames are only sent when there's a change.
 * +------------+------------+------------+------------+------------+------------+------------+
 * | Preamble   |    Keys    |   X-Axis H |   X-Axis L |   Y-Axis H |   Y-Axis L |    FCS     |
 * +------------+------------+------------+------------+------------+------------+------------+
 *     byte 0       byte 1       byte 2       byte 3       byte 4       byte 5       byte 6
 * 
 *      Preamble = 0xAA
 *      
 *      Keys = (see below)
 *           K1 pressed = 0b00000001 (0x01)
 *           K2 pressed = 0b00000010 (0x02)
 *           K3 pressed = 0b00000100 (0x04)
 *           K4 pressed = 0b00001000 (0x08)
 *           K5 pressed = 0b00010000 (0x10) (Joystick button)
 *           
 *           +------+------+------+------+------+------+------+------+
 *           |  0   |  0   |  0   |  K5  |  K4  |  K3  |  K2  |  K1  |
 *           +------+------+------+------+------+------+------+------+
 *              b7     b6     b5     b4     b3     b2     b1     b0
 *      
 *      X-Axis [H & L] => see example below
 *           if the X-Axis value is 0x03FF, then the 2-byte will be
 *           X-Axis Low  byte = 0xFF
 *           X-Axis High byte = 0x03
 *      
 *      Y-Axis [H & L] => see example below
 *           if the Y-Axis value is 0x03FF, then the 2-byte will be
 *           Y-Axis Low  byte = 0xFF
 *           Y-Axis High byte = 0x03
 *      
 *      FCS = Cheksum off all the bytes, including the preamble byte.
 * 
 * 
 * Keepalive sent every 3 seconds, by default.
 * 
 * +------------+------------+------------+------------+------------+------------+------------+
 * | Preamble   |    0xAA    |    0x55    |    0xAA    |    0x55    |    0xAA    |    FCS     |
 * +------------+------------+------------+------------+------------+------------+------------+
 *     byte 0       byte 1       byte 2       byte 3       byte 4       byte 5       byte 6
 * 
 * Preamble = 0xE0
 * FCS = 0x07
 *
*/

#include "OS_JoystickCtrl.h"

/*
 * Constructor
 */
JoystickCtrl::JoystickCtrl() {
  uint8_t keys_idx;

  /* Initialise digital I/O for all the buttons */
  _Key_Pin[k1] = DIO_KEY1;
  _Key_Pin[k2] = DIO_KEY2;
  _Key_Pin[k3] = DIO_KEY3;
  _Key_Pin[k4] = DIO_KEY4;
  _Key_Pin[kz] = DIO_KEYZ;
  for (keys_idx = k1; keys_idx <= kz; keys_idx++) {
    pinMode(_Key_Pin[keys_idx], INPUT_PULLUP);
  }

  _X_Axis_Zero = readX_Axis();  // get the initial values for the X & Y axis. Should read ~512
  _Y_Axis_Zero = readY_Axis();  // Joystick MUST be in neutral position when reading.

}//JoystickCtrl

/*
 * Initialize main variables
 */
void JoystickCtrl::init() {
  _Keys     = 0x00;
  _Old_Keys = 0x00;

  _X_Axis = readX_Axis();
  _Y_Axis = readY_Axis();
  _X_AxisOld = _X_Axis; 
  _Y_AxisOld = _Y_Axis;
}//init

/*
 * Returns the encoded key pressed in a single byte. Bit 5-7 are always 0.
 *   K1 pressed = 0b00000001 (0x01)
 *   K2 pressed = 0b00000010 (0x02)
 *   K3 pressed = 0b00000100 (0x04)
 *   K4 pressed = 0b00001000 (0x08)
 *   K5 pressed = 0b00010000 (0x10) (Joystick button)
 * 
 * +------+------+------+------+------+------+------+------+
 * |  0   |  0   |  0   |  K5  |  K4  |  K3  |  K2  |  K1  |
 * +------+------+------+------+------+------+------+------+
 *    b7     b6     b5     b4     b3     b2     b1     b0
 * 
 * This is the second byte sent when there's a change in the status of the remote.
 */
uint8_t JoystickCtrl::readKeys() {
  uint8_t keys_idx;
  uint8_t keys = 0x00;
  uint8_t Pin_Value; //Value of a pin read by digitalRead

  for (keys_idx = k1; keys_idx <= kz; keys_idx++) {
    if(!digitalRead(_Key_Pin[keys_idx])){
      delay(DEBOUNCE_DELAY_KEYS); //debounce

/*
 * Since we're using a pull-up for the buttons, we need to negate
 * the digital read to have the following values.
 *    0 = Not pressed
 *    1 = Pressed
 */
      Pin_Value = !digitalRead(_Key_Pin[keys_idx]);
      //Sets/Clear a specific bit in a byte
      keys ^= _CHANGE_BIT(keys, Pin_Value, keys_idx);
    }
  }
  return keys;
}//readKeys()

/*
 * Return the value a the x-axis as a number between 0-1023
 * This is the 3rd & 4th bytes sent when there's a change in x axis.
 */
uint16_t JoystickCtrl::readX_Axis() {
  return analogRead (AIO_XAXIS);
}//ReadX_Axis

uint16_t JoystickCtrl::getX_Axis() {
  return _X_Axis;
}//getX_Axis

/*
 * Return the value a the y-axis as a number between 0-1023
 * This is the 5th & 6th bytes sent when there's a change in the y axis.
 */
uint16_t JoystickCtrl::readY_Axis() {
  return analogRead (AIO_YAXIS);
}//ReadY_Axis

uint16_t JoystickCtrl::getY_Axis() {
  return _Y_Axis;
}//getY_Axis

uint8_t JoystickCtrl::getKeys () {
  return _Keys;
}//getKeys

uint8_t JoystickCtrl::asChange() {
  uint8_t  flag_change = 0;
  int x,y; //Diffrence between new & old x or y axis. Needs to be a 16-bit signed variable

/*
 * Check if a button has changed state. Either pressed or released.
 */
  _Keys = readKeys();
  if(_Keys != _Old_Keys) {
    _Old_Keys = _Keys;
    flag_change = 1;
  }

  _X_Axis = readX_Axis();
  x = _X_Axis - _X_AxisOld;
  if (abs (x) > X_SMOOTHING) {
    _X_AxisOld = _X_Axis;
    flag_change = 1;
  }

  _Y_Axis = readY_Axis();
  y = _Y_Axis - _Y_AxisOld;
  if (abs (y) > Y_SMOOTHING) {
    _Y_AxisOld = _Y_Axis;
    flag_change = 1;
  }
  
  return flag_change;
}//asChange

/*
 * Simple checksum function. Returns the checksum of an array of bytes.
 * Checksum is calculated of the whole packet, including the premabule.
 */
uint8_t JoystickCtrl::Checksum (const uint8_t *data, uint8_t len) {
  uint8_t c = 0;

  while (len--)
    c ^= *data++;
  return c;
}// Checksum()
