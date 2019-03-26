#ifndef _OPENSMART_JOYSTICKCTRL_H__
#define _OPENSMART_JOYSTICKCTRL_H__

#include <Arduino.h>

#define DEBOUNCE_DELAY_BUTTON   4000 // in microsec

#define DIO_KEY1 5 //Button K1
#define DIO_KEY2 4 //Button K2
#define DIO_KEY3 3 //Button K3
#define DIO_KEY4 2 //Button K4
#define DIO_KEYZ 7 //Button on joystick (labeled as z-axis on remote
#define AIO_XAXIS A0 //Analog pin for X axis
#define AIO_YAXIS A1 //Analog pin for Y axis

#define MAX_KEY 5
enum keys_idx {k1, k2, k3, k4, kz};

//number ^= (-x ^ number) & (1 << n)
#define _CHANGE_BIT(_byte, _x, _bit) (-_x ^ _byte) & (1 << _bit)

#define MSB(b) (b >> 8)   //Returns the MSB of an uint16_t
#define LSB(b) (b & 0xff) //Returns the LSB of an uint16_t

#define X_SMOOTHING 8 //Minimum movement of X axis before we consider a change
#define Y_SMOOTHING 8 //Minimum movement of Y axis before we consider a change

/*
* Preamble byte for each type of packets sent by the remote
*/

#define STATUS_PREAMBULE    0xAA
#define KEEPALIVE_PREAMBULE 0xE0

class JoystickCtrl {
private:
/*
 * _Buttons byte
 * K1 pressed = 0b00000001 (0x01)
 * K2 pressed = 0b00000010 (0x02)
 * K3 pressed = 0b00000100 (0x04)
 * K4 pressed = 0b00001000 (0x08)
 * KZ pressed = 0b00010000 (0x10) (Joystick button)
 */
  uint8_t  _Buttons;
  uint8_t  _Old_Buttons;
  uint16_t _X_Axis, _Y_Axis; //Analog value for joystick pins
  uint16_t _X_AxisOld, _Y_AxisOld; //Last values for joystick pins
  uint8_t  _Key_Pin [MAX_KEY]; //Array of I/O pins for all the buttons

  uint8_t  _readButtons ();   //Digital read of all the buttons of the remote                     
  uint16_t _read_Axis (uint8_t);  //Analog read of a joystick's x or y axis

public:
  JoystickCtrl ();
  void init();
  uint8_t  getButtons (); //Returns the encoded button, pressed/released, in a single byte.

  uint16_t getX_Axis (); //Returns variable _X_Axis as uint16_t: Public
  uint16_t getY_Axis (); //Returns variable _Y_Axis as uint16_t: Public

  bool     asChange (); //Read all sensors on remote and return true if something changed.
  uint8_t  Checksum (const uint8_t *data, uint8_t len); //Returns checksum of an array of bytes.
};

#endif
