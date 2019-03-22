#ifndef _OPENSMART_JOYSTICKCTRL_H__
#define _OPENSMART_JOYSTICKCTRL_H__

#include <Arduino.h>

#define DEBOUNCE_DELAY_KEYS     10 // in msec
#define DEBOUNCE_DELAY_JOYSTICK 10 // in msec

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

#define MSB(b) (b >> 8)  //Returns the MSB of an uint16_t
#define LSB(b) (b & 0xff) //Returns the LSB of an uint16_t

#define X_SMOOTHING 30 //Minimum movement of X axis before we consider a change
#define Y_SMOOTHING 30 //Minimum movement of Y axis before we consider a change

class JoystickCtrl {
private:
/*
 * _Keys byte
 * K1 pressed = 0b00000001 (0x01)
 * K2 pressed = 0b00000010 (0x02)
 * K3 pressed = 0b00000100 (0x04)
 * K4 pressed = 0b00001000 (0x08)
 * KZ pressed = 0b00010000 (0x10) (Joystick button)
 */
  uint8_t  _Keys;
  uint8_t _Old_Keys;
  uint16_t _X_Axis, _Y_Axis; //Analog value for joystick pins
  uint16_t _X_Axis_Zero, _Y_Axis_Zero; //Initial value of each axis, around 1023/2
  uint16_t _X_AxisOld, _Y_AxisOld; //Values for joystick pins
  uint8_t _Key_Pin [MAX_KEY]; //Array of I/O pins for all the button

  uint8_t  readKeys();   //Digital read of all the buttons of the remote                     
  uint16_t readX_Axis(); //Analog read of pin as uint16_t: Private
  uint16_t readY_Axis(); //Analog read of pin as uint16_t: Private

public:
  JoystickCtrl ();
  void init();
  uint8_t  getKeys ();  //Returns the encoded keys pressed in a single byte.

  uint16_t getX_Axis(); //Returns variable _X_Axis as uint16_t: Public
  uint16_t getY_Axis(); //Returns variable _Y_Axis as uint16_t: Public

  uint8_t asChange(); //Read all sensors on remote and return true if something chnaged.
  uint8_t Checksum (const uint8_t *data, uint8_t len); //Returns checksum of an array of bytes.
};

#endif
