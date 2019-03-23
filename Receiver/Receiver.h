/*
 * Receiver.h
 *
 * Header file for "Receiver.ino". 
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
void Preamble (const uint8_t *);

#endif /* RECEIVER_H */
