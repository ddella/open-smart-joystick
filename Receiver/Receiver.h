/*
 * Receiver.h
 *
 * Header file for "Receiver.ino". 
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

#ifndef RECEIVER_H
#define RECEIVER_H

/* Just comment the "KEEPALIVE" if you don't want to test the keepalives
 * sent from the remote at regular interval.
 */
#define KEEPALIVE

/*
 * Digital IO pin from the RF receiver. Can be any digital pin.
 */
#define RX_DIO_PIN 4
#define MAX_MESSAGE_LEN 7 //Joystick sends a max of 7 bytes
#define TX_SPEED 2000 //Transmission speed in bits/sec

/*
 * Preamble byte for each type of packets sent by the remote
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

#define WORD(MSB,LSB) (((uint16_t)MSB << 8) | LSB)  //Returns a uint16 from two uint8

/* Global variables for schedulers */
#ifdef KEEPALIVE
uint32_t previousMillis_500ms;
uint32_t LastKeepalive; //millis() timestamp of the last keepalive received
#endif

/* ============================================================================
 *                              PROTOTYPES
 * =========================================================================== */

uint8_t Checksum (const uint8_t *, uint8_t);
void PrintBuf (const uint8_t *, uint8_t);
void RXStatus (const uint8_t *);
void RXKeealive (const uint8_t *);

#endif /* RECEIVER_H */
