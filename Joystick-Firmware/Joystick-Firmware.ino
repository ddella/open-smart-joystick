/*
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
#include <VirtualWire.h>
#include "OS_JoystickCtrl.h"

/*
 * Different ifdef for codes
 */
//#define DEBUG

/* Just comment the "KEEPALIVE" if you don't want the remote to send keepalives
 * at regular interval. This feature adds 190 bytes to the code.
 */
#define KEEPALIVE

#define MAX_TX_BUFFER 7 //Remote is sending a fix amount of bytes
#define TX_SPEED 2000 //Transmission speed in bits/sec

// TX buffer transmitted to the receiver
uint8_t TXBuffer[MAX_TX_BUFFER]= {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}; 

//Wireless RF 433 Mhz module on the remote
#define RF_TX_PIN 6 //connected the RF transmitter module on pin 6.

#ifdef KEEPALIVE
// Calculate the counter overflow count based on the required bit speed
// and CPU clock rate
#define NBR_SECONDS 2 //Number of seconds timer #2 overflows
const uint16_t TIMER2_OVERFLOW_CNT_VALUE = ((F_CPU / 1024UL) / 255) * NBR_SECONDS;

// global variable to count the number of overflows for Timer2
volatile uint16_t Timer2_Overflow_Cnt; //Can't count more than ~4 sec with an 8-bit var
#endif

/*
 * Global variables
 */
JoystickCtrl remote;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial) {
    ;// do nothing
  };
  Serial.println(F("OPEN-SMART Wireless Joystick Remote"));
#endif

/*
 * Initialize timer #2 to send keepalive
 */
#ifdef KEEPALIVE
  Timer2_Init();
#endif

/*
 * Initialize the wireless remote
 */
  remote.init ();

/*
 * Initialize the 433Mhz transmitter
 */
  vw_set_tx_pin (RF_TX_PIN); // Setup transmit pin
  vw_setup (TX_SPEED); // Transmission speed in bits per second.

}//setup

void loop() {

/*
 * If the status of the remote as changed, send the status of all button/axis
 */
  if (remote.asChange()) {
    /*
     * If the status of the remote as changed, send a status packet with
     * the value of all sensors (button and joystick).
     */
    //TXBuffer[0] = 0xAA;//Already filled
    TXBuffer[1] = remote.getButtons ();
    TXBuffer[2] = MSB(remote.getX_Axis());
    TXBuffer[3] = LSB(remote.getX_Axis());;
    TXBuffer[4] = MSB(remote.getY_Axis());
    TXBuffer[5] = LSB(remote.getY_Axis());;
    TXBuffer[MAX_TX_BUFFER-1] = remote.Checksum (TXBuffer, MAX_TX_BUFFER-1);
  
    vw_send (TXBuffer, MAX_TX_BUFFER);
    }//if remote.asChange()
  
    #ifdef KEEPALIVE
    /*
     * Since we're using timer #2 and the counter is 8-bit, we have to overflow
     * the counter so many times before sending a keepalive.
     */
    if (Timer2_Overflow_Cnt >= TIMER2_OVERFLOW_CNT_VALUE) {
      //send keepalive packet
      Send_KeepAlive ();
      Timer2_Overflow_Cnt = 0; //Re initialize the overflow counter
    }
    #endif
}//loop

/*
 * Timer2 Interrupt Service Routine is called when counter for timer2 overflows.
 * 
 * If we use the highest pre-scalar of 1024, calculation shows that ISR will be called every 16ms.
 * 
 * OverFlowCount = 1000ms / 16ms = 62,5 ≈ 63 overflows per second
 * 
 */
#ifdef KEEPALIVE
ISR (TIMER2_OVF_vect) {
    // keep a track of number of overflows
    Timer2_Overflow_Cnt++;
}//ISR


/*
 * F_CPU = 16 MHz: Clock Time Period T = 1 / 16M = 0.0000625 ms.
 * 
 * Timer Count = Required Delay / (Clock Time Period / prescaler)
 *
 * If we want to have a Delay of 1000 ms (1sec) and a prescaler of 1
 *
 * PRESCALER OF 1
 * Timer Count = 1000 / (0.0000625 * 1) = 16000000
 *    The timer will count to 16000000 every second
 *    The 8-bit timer will overflow 16000000/255 = 62745 times per second
 *
 * PRESCALER OF 1024
 * Timer Count = 1000 / (0.0000625 * 1024) = 15625
 *    The timer will count to 15625 every second
 *    The 8-bit timer will overflow 15625/255 = 61 times per second
 *
 * timer speed (Hz) = Arduino clock speed (16MHz) / prescaler
 * 16000000 / 1024 = 15625 This is the counter value every seconds. The TCNT2 is only 8-bit
 * 15625 / 255 = 61Hz The timer2 will overflow 61 times each second
 *
 * Timer2 will interrupt at 16KHz
 */

// initialize timer2, interrupt and variable
void Timer2_Init() {

  cli(); // stop interrupts
  // initialize Timer/Counter2 Control Register
  TCCR2 = 0x00;
  // set up timer2 with prescaler = 1024
  TCCR2 |= (1 << CS22) | (1 << CS21) | (1 << CS20);
  
  // initialize Timer/Counter2 Register
  TCNT2 = 0x00;
  
  // enable overflow interrupt
  // Timer/Counter Interrupt Mask Register – TIMSK is common to all timers!
  TIMSK |= (1 << TOIE2); // When the TOIE2 bit is set, Timer/Counter2 Overflow interrupt is enabled
  
  // enable global interrupts
  sei();
  
  // initialize overflow counter variable for timer 2
  Timer2_Overflow_Cnt = 0;
}//Timer2_Init()

/*
 * Sends keepalive packets to the receiver.
 * 
 * Keepalive packets are 7-bytes long with fixed pattern.
 *
 * +------------+------------+------------+------------+------------+------------+------------+
 * | Preamble   |    0x55    |    0xAA    |    0x55    |    0xAA    |    0x55    |    FCS     |
 * +------------+------------+------------+------------+------------+------------+------------+
 *
 * Preamble = 0xE0
 * FCS = 0xB5
 * 
 */
void Send_KeepAlive () {
  static uint8_t KeepAlive [MAX_TX_BUFFER] = {0xE0, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xB5};
  /*
   * Don't need to calculate the checksum each time we send a keepalive.
   * It's a fixed packet :-)
   */
  //TXBuffer[MAX_TX_BUFFER-1] = remote.Checksum (TXBuffer, MAX_TX_BUFFER-1);
  vw_send (KeepAlive, MAX_TX_BUFFER);
  Serial.println (F("Keepalive sent"));
}
#endif
