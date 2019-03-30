/*
 * Program:  Receiver.ino
 *  
 * Description:  This program will receive the packets from Open-Smart wireless joystick.
 *   Frequency of RF receiver is 433MHz. This code assumes that you are using my modified
 *   firmware for the OPEN-SMART wireless remote joystick.
 *   
 * There's only two different types of frame sent by the remote.
 * 
 * 1. Wireless Joystick status frame. It contains the status of all the buttons,
 *    x-axis and y-axis values.
 *
 * 2. Keepalive frame. Sent to the remote so it can detect if it's out of range.
 * 
 * Status packets from remote wireless joystick. Only sent when there's a change.
 * +------------+------------+------------+------------+------------+------------+------------+
 * | Preamble   |   Button   | MSB x-axis | LSB x-axis | MSB y-axis | LSB y-axis |    FCS     |
 * +------------+------------+------------+------------+------------+------------+------------+ 
 *    byte 0       byte 1       byte 2       byte 3       byte 4       byte 5       byte 6
 *
 *      Preamble = 0xAA
 *      
 *      Button = (see below)
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
 * Keepalive sent every 2 seconds, by default.
 * 
 * +------------+------------+------------+------------+------------+------------+------------+
 * | Preamble   |    0xAA    |    0x55    |    0xAA    |    0x55    |    0xAA    |    FCS     |
 * +------------+------------+------------+------------+------------+------------+------------+
 *     byte 0       byte 1       byte 2       byte 3       byte 4       byte 5       byte 6
 * 
 * Preamble = 0xE0
 * FCS = 0xB5
 *
 * The following values are approximate. They vary a bit. Test your remote :-)
 * 
 *            [Y-Value]
 *              1023
 *               |
 *               |
 *               |
 *               |
 *               |
 * 0------------512------------1023 [X-Value]
 *               |
 *               |
 *               |
 *               |
 *               |
 *               |
 *               0
 * 
 * Hardware:     Standalone ATmega328P-PU/Arduino Uno R3, Open-Smart joystick wireless (433 MHz).
 *               Should work with other Arduinos 
 *  
 * Software:     Developed using Arduino 1.8.8 IDE
 *  
 * Libraries:    
 *               - VirtualWire 1.9:
 *                 https://github.com/adgzlanl/433mhzRF-VirtualWire-TX-RX
 *  
 * References: 
 *  
 * Date:         March, 2019
 *  
 * Author:       Daniel Della Noce
 * 
 * ----------------------------------------------------
 *          Rx OPEN-SMART RF 433 CONNECTIONS:
 * ----------------------------------------------------
 * Rx Module | DESCRIPTION   |  ARDUINO PIN
 * ----------------------------------------------------
 * GND       |   GND         | GND
 * VCC       |   VCC (5V)    | VCC (5V)
 * SIG       |   RX DATA     | D4 (see RX_DIO_PIN)
 * -----------------------------------------------------
 * 
 * MIT License
 * 
 * Copyright (c) 2019 ddella
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 */

/*Include the VirtualWire library */
#include <VirtualWire.h>
#include "Receiver.h"

/* Global variable for last button's status */
uint8_t LastButtonPressed = JOYSTICK_NO_BUTTON_PRESSED;

/* Global variable for current & last x-axis position */
uint16_t Last_X_Axis = 505;
uint16_t Current_X_Axis = 505;

/* Global variable for current & last y-axis position */
uint16_t Last_Y_Axis = 505;
uint16_t Current_Y_Axis = 505;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;// do nothing
  };

  Serial.println(F("OPEN-SMART Wireless Joystick Remote"));
  Serial.println(F("Receiver.ino"));
  Serial.println(F("by Daniel Della Noce"));
  Serial.print(F("Please connect Rx pin of module on DIO #: ")); Serial.println (RX_DIO_PIN);
  Serial.println();

  // Initialises the DIO pin used to receive data from the Rx module
  vw_set_rx_pin(RX_DIO_PIN);

  // Received speed in bits per second
  vw_setup(TX_SPEED);

  // Enable the receiver
  vw_rx_start();

#ifdef KEEPALIVE
  previousMillis_500ms = millis();
#endif
}

void loop() {
  uint8_t Buffer_Size = MAX_MESSAGE_LEN; //Number of bytes received
  uint8_t RxBuffer[MAX_MESSAGE_LEN];     //Buffer that contains the received data
  uint8_t FCS; //Calculated checksum from the received packet

  while (1) {
  /*
   * Check to see if we received a packet. I used a while(1) loop inside loop()
   * to be able to use "continue".
   */
    if (vw_get_message(RxBuffer, &Buffer_Size)) {
      //if packet is not the correct length, then just skip. Not sent from our remote.
      if (Buffer_Size != MAX_MESSAGE_LEN) {
        Serial.print (F("Error: Received message of length "));
        Serial.print (Buffer_Size);
        Serial.print (F(" was expecting length of "));
        Serial.println (MAX_MESSAGE_LEN);
        continue;
      }
      //Caculate checksum of the received packet.
      FCS = Checksum (RxBuffer, MAX_MESSAGE_LEN - 1);
      //if checksum is wrong, then just skip. Not sent from our remote or bad packet.
      if (FCS != RxBuffer [MAX_MESSAGE_LEN - 1]) {
        Serial.println (F("Error: Received message with bad checksum: "));
        continue;
      }

      /*
       * At this point, we received a packet of the correct length and with a valid checksum.
       * We need to verify the type of packet by looking at the preambule byte.
       * RxBuffer [0] is the preamble. It can be either status or keepalive
       */
      switch (RxBuffer [0]) {
        case STATUS_PREAMBULE:
          RXStatus (RxBuffer);
        break;
        #ifdef KEEPALIVE
        case KEEPALIVE_PREAMBULE:
          RXKeealive (RxBuffer);
          //Serial.println (F("Keepalive received"));
        break;
        #endif
      }//switch (RxBuffer [0])
    } // if packet received

    /* 
     *  This block is executed every 500 ms to check if keepalive packets are
     *  received from remote at the correct interval. This is non-blocking.
     */
    #ifdef KEEPALIVE
    if (millis() - previousMillis_500ms >= INTERVAL_500ms) {
      previousMillis_500ms = millis();
    /*
     * This code is "overflow" safe. Even if millis() overflow after 49 days, the test
     * will be valid.
     * 
     * !!! The following will NOT work !!!
     * if (millis()  > LastKeepalive + KEEPALIVE_INTERVAL)
     * 
     */
      if (millis() - LastKeepalive > KEEPALIVE_INTERVAL) {
        //Keepalive not received!!!
        Serial.print(F("Keepalive not received. Last keepalive received: "));
        Serial.println (millis() - LastKeepalive);
      }
    } //if >= 500 msec
    #endif
  } //while (1)

}//loop()

/*
 * Function: RXStatus
 * ------------------
 *
 * Treat the reception of a status packet. This type of packet is sent whenever a
 * change is detected on the wireless remote. Either a button pressed/released or
 * a joystick movement.
 * 
 * @param  Array of byte
 * 
 * @return Nothing
 */
void RXStatus (const uint8_t *RxBuffer) {

  Current_X_Axis = WORD(RxBuffer [2], RxBuffer [3]);
  Current_Y_Axis = WORD(RxBuffer [4], RxBuffer [5]);
  if (Current_X_Axis != Last_X_Axis) {
    Serial.print (F("X axis value changed = ")); Serial.print (Current_X_Axis);
    Serial.print (F(" :"));
    Last_X_Axis = Current_X_Axis;
  }
  if (Current_Y_Axis != Last_Y_Axis) {
    Serial.print (F("Y axis value changed = ")); Serial.print (Current_Y_Axis);
    Last_Y_Axis = Current_Y_Axis;
    Serial.print (F(" :"));
  }

   //RxBuffer [1] is the button byte. (which button has been pressed)
   switch (RxBuffer [1]) {
     case JOYSTICK_BUTTON_K1_PRESSED:
       Serial.print (F("Button K1 pressed: "));
       LastButtonPressed = JOYSTICK_BUTTON_K1_PRESSED;
     break;
     case JOYSTICK_BUTTON_K2_PRESSED:
       Serial.print (F("Button K2 pressed: "));
       LastButtonPressed = JOYSTICK_BUTTON_K2_PRESSED;
     break;
     case JOYSTICK_BUTTON_K3_PRESSED:
       Serial.print (F("Button K3 pressed: "));
       LastButtonPressed = JOYSTICK_BUTTON_K3_PRESSED;
     break;
     case JOYSTICK_BUTTON_K4_PRESSED:
       Serial.print (F("Button K4 pressed: "));
        LastButtonPressed = JOYSTICK_BUTTON_K4_PRESSED;
     break;
     case JOYSTICK_BUTTON_KZ_PRESSED:
       Serial.print (F("Button KZ pressed: "));
       LastButtonPressed = JOYSTICK_BUTTON_KZ_PRESSED;
     break;
     case JOYSTICK_NO_BUTTON_PRESSED:
       if (LastButtonPressed == JOYSTICK_BUTTON_K1_PRESSED)
         Serial.print (F("Button K1 released: "));
       else if (LastButtonPressed == JOYSTICK_BUTTON_K2_PRESSED)
         Serial.print (F("Button K2 released: "));
       else if (LastButtonPressed == JOYSTICK_BUTTON_K3_PRESSED)
         Serial.print (F("Button K3 released: "));
       else if (LastButtonPressed == JOYSTICK_BUTTON_K4_PRESSED)
         Serial.print (F("Button K4 released: "));
       else if (LastButtonPressed == JOYSTICK_BUTTON_KZ_PRESSED)
         Serial.print (F("Button KZ released: "));
       LastButtonPressed = RxBuffer [1];  
     break;
   }//switch (RxBuffer [1])
   PrintBuf (RxBuffer, MAX_MESSAGE_LEN);
} //RXStatus (const uint8_t *RxBuffer)

/*
 * Function: RXKeealive
 * --------------------
 *
 * Treat the reception of a keepalive packet.
 * 
 * @param  Array of byte
 * 
 * @return Nothing
 */
void RXKeealive (const uint8_t *RxBuffer) {
   Serial.print ("Keepalive: ");
   LastKeepalive = millis(); //Reset keepalive timer
   PrintBuf (RxBuffer, MAX_MESSAGE_LEN);
} //RXKeealive (const uint8_t *)

/*
 * Function: Checksum
 * ------------------
 *
 * Simple checksum function. Returns the checksum of an array of bytes. The checksum is
 * a single byte.
 * 
 * @param  Array of byte
 *         Length of array
 * 
 * @return checksum
 */
uint8_t Checksum (const uint8_t *data, uint8_t len) {
  uint8_t c = 0;

  while (len--)
    c ^= *data++;
  return c;
}// Checksum (const uint8_t *, uint8_t)

/*
 * Function: PrintBuf
 * ------------------
 *
 * Print hex digit from an array of bytes. HEX numbers are printed in the following
 * format: 0x00, 0x00, 0x00, ...
 * 
 * @param  Array of byte
 *         Length of array
 * 
 * @return Nothing
 */
void PrintBuf (const uint8_t *data, uint8_t len) {
  while (len--) {
    if (*data < 10)
      Serial.print (F("0x0"));
    else
        Serial.print (F("0x"));
    Serial.print (*data++, HEX);
    if (len > 0) //Don't print the last ","
      Serial.print(",");
  }//for
  Serial.println ();
}// PrintBuf (const uint8_t *, uint8_t)
