/*
 * Program:  WirelessJoystickRemote.ino
 *  
 * Description:  Open-Smart wireless joystick. Frequency of RF receiver is 433MHz.
 *   Simple example of how to use VirtualWire to receive messages from the wireless joystick.
 *   
 *   This code assumes that you are using my modified firmware for the OPEN-SMART wireless
 *   remote joystick.
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
 * SIG       |   RX DATA     | D4 (see RX_DIO_PIN below)
 * -----------------------------------------------------
 * 
 *  License:
 *   Public Domain
 * 
 */

/*Include the VirtualWire library */
#include <VirtualWire.h>

/* Just comment the "KEEPALIVE" if you don't want to test the keepalives
 * sent from the remote at regular interval.
 */
#define KEEPALIVE

// Digital IO pin that will be used for receiving data from the RF receiver
#define RX_DIO_PIN 4
#define MAX_MESSAGE_LEN 7 //Joystick sends a max of 7 bytes
#define TX_SPEED 2000 //Transmission speed in bits/sec

/*
 * Preamble byte in each different frames sent by the remote
 */
#define STATUS_PREAMBULE    0xAA
#define KEEPALIVE_PREAMBULE 0xE0

#define INTERVAL_500ms       500UL
#define KEEPALIVE_INTERVAL   3000UL //Keepalive sent every 3 sec

/*
 * Keys byte
 *   K1 pressed = 0b00000001 (0x01)
 *   K2 pressed = 0b00000010 (0x02)
 *   K3 pressed = 0b00000100 (0x04)
 *   K4 pressed = 0b00001000 (0x08)
 *   KZ pressed = 0b00010000 (0x10) (Joystick's button)
 */
#define JOYSTICK_BUTTON_K1_PRESSED 0b00000001 // (0x01)
#define JOYSTICK_BUTTON_K2_PRESSED 0b00000010 // (0x02)
#define JOYSTICK_BUTTON_K3_PRESSED 0b00000100 // (0x04)
#define JOYSTICK_BUTTON_K4_PRESSED 0b00001000 // (0x08)
#define JOYSTICK_BUTTON_KZ_PRESSED 0b00010000 // (0x10)
#define JOYSTICK_NO_BUTTON_PRESSED 0b00000000 // (0x00) no button pressed

#define WORD(high,low) (((uint16_t)high << 8) | low)  //Returns a uint16 from two uint8

/* Global variables for schedulers */
#ifdef KEEPALIVE
uint32_t previousMillis_500ms;
uint32_t LastKeepalive; //millis() timestamp of the last keepalive received
#endif

/* Global variable for last button's status */
uint8_t LastButtonPressed;

/* Global variable for current & last x-axis position */
uint16_t Last_X_Axis;
uint16_t Current_X_Axis;

/* Global variable for current & last y-axis position */
uint16_t Last_Y_Axis;
uint16_t Current_Y_Axis;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;// do nothing
  };

  Serial.println(F("OPEN-SMART Wireless Joystick Remote"));
  Serial.println(F("WirelessJoystickRemote.ino"));
  Serial.println(F("by Daniel Della Noce"));
  Serial.print(F("Please connect Rx pin of module on DIO #: ")); Serial.println (RX_DIO_PIN);
  Serial.println();
  pinMode (13, OUTPUT);

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
  uint8_t FCS; //Calculated checksum from the frame received

    // If a message is received, we store it with the length of the message received
    if (vw_get_message(RxBuffer, &Buffer_Size)) {
      digitalWrite(13, HIGH); // Turn on the LED connected pin 13
     //Remote sends fixed 7-byte frames for status and keepalive
      if (Buffer_Size == MAX_MESSAGE_LEN) {
        FCS = Checksum (RxBuffer, MAX_MESSAGE_LEN - 1);//Caculate checksum from the buffer received
        if (FCS == RxBuffer [MAX_MESSAGE_LEN - 1]) {
          //RxBuffer [0] is the preamble. Either status or keepalive
          switch (RxBuffer [0]) { // Check if we received a status or a keepalive
            case STATUS_PREAMBULE:

              Current_X_Axis = WORD(RxBuffer [2], RxBuffer [3]);
              Current_Y_Axis = WORD(RxBuffer [4], RxBuffer [5]);
              if (Current_X_Axis != Last_X_Axis) {
                Serial.print (F("X axis value changed = ")); Serial.println (Current_X_Axis);
                Last_X_Axis = Current_X_Axis;
              }
              if (Current_Y_Axis != Last_Y_Axis) {
                Serial.print (F("Y axis value changed = ")); Serial.println (Current_Y_Axis);
                Last_Y_Axis = Current_Y_Axis;
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
                 break;
                 LastButtonPressed = RxBuffer [1];
               }//switch (RxBuffer [1])
             break;
#ifdef KEEPALIVE
            case KEEPALIVE_PREAMBULE:
              Serial.print ("Keepalive: ");
              LastKeepalive = millis();//Reset keepalive timer
            break;
#endif
          }//switch (RxBuffer [0])
          PrintBuf (RxBuffer, MAX_MESSAGE_LEN);
          digitalWrite(13, LOW);          
        }//if FCS
      }//if length
    }//ifvw_get_message

  /* 
   *  This block is executed every 500 ms to check if keepalive are
   *  received from remote at the correct interval.
   */
#ifdef KEEPALIVE
  if (millis() - previousMillis_500ms >= INTERVAL_500ms) {
    previousMillis_500ms = millis();   
    if (millis() - LastKeepalive > KEEPALIVE_INTERVAL) {
      //Keepalive not received!!!
      Serial.print(F("Keepalive not received. Last keepalive received: "));
      Serial.println (millis() - LastKeepalive);
    }
  }
#endif
}//loop()

/*
 * Simple checksum function. Returns the checksum of an array of bytes.
 */
uint8_t Checksum (const uint8_t *data, uint8_t len) {
  uint8_t c = 0;

  while (len--)
    c ^= *data++;
  return c;
}// Checksum()

/*
 * Print hex digit from an array of bytes.
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
}// PrintBuf()
