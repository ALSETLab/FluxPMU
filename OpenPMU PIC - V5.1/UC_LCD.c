/***********************************************************************
 * UC_LCD --  for use with PICDEM2Plus using PIC18F452 or PIC18F458    *
 * LCD display on PICDEM2 uses Hitachi HD44780 LCD control protocol    *
 *                                                                     *
 * Copyright (c) 2004 - Ludwig J. Keck -- All rights reserved          *
 * You may use this code without restriction so long as authorship is  *
 * credited and the code is not sold as library code and that you      *
 * provide the author with any unencumbered changes or improvements    *
 *                                                                     *
 * Start 10 Apr 04 LJK                                                 *
 *                                                                     *
 **********************************************************************/

#include <p18cxxx.h>  
#include <stdlib.h>
#include <delays.h>

/*
   PICDEM2 Configuration:
   
   Oscillator: 4 MHz

   LCD DISPLAY: 2-line by 16 char
   E  - on RA1
   RW - on RA2
   RS - on RA3

   DB4 - on RD0
   DB5 - on RD1
   DB6 - on RD2
   DB7 - on RD3

   !!! NOTE: ADCON1 must be set for DIGITAL I/O to use RAx !!!
*/

// DATA_PORT defines the port to which the LCD data lines are connected
#define DATA_PORT      PORTD
#define TRIS_DATA_PORT TRISD

#define RW_PIN   PORTAbits.RA2   // PORT for RW 
#define TRIS_RW  DDRAbits.RA2    // TRIS for RW 
#define RS_PIN   PORTAbits.RA3   // PORT for RS 
#define TRIS_RS  DDRAbits.RA3    // TRIS for RS 
#define E_PIN    PORTAbits.RA1   // PORT for E 
#define TRIS_E   DDRAbits.RA1    // TRIS for E 

#define LN_INPUT  TRIS_DATA_PORT|=0x0f  // make low nibble input
#define LN_OUTPUT TRIS_DATA_PORT&=0x00  // make low nibble output

/* Display ON/OFF Control defines */
#define DISPLAY_ON  0b00001111  
#define DISPLAY_OFF 0b00001011  
#define CURSOR_ON   0b00001111  
#define CURSOR_OFF  0b00001101  
#define BLINK_ON    0b00001111  
#define BLINK_OFF   0b00001110  


/**********************************************************************/
void delay_1us (void)  
	{  // 1 Nop at 4 MHz ~ 1 microsecond
   Nop (); // replace with Delayxxxx for higher freq operation
	}
/**********************************************************************/
void delay_20ms (void) // at 4 MHz: ~ 20 ms
	{
	Delay1KTCYx (20); 
	return;
	}
/**********************************************************************/
void delay_5ms (void) // at 4 MHz: ~ 5 ms
	{
	Delay1KTCYx (5); 
	return;
	}
/**********************************************************************/
void clock_data (void)
   {
   delay_1us ();                   // delay ~ 1 microsecond
   E_PIN = 1;                      // set LCD enable pin HI
   delay_1us ();                   // delay ~ 1 microsecond
   E_PIN = 0;                      // set LCD enable pin LO
   }
/**********************************************************************/
unsigned char LCD_busy (void)
   {
   unsigned char port_data, port_busypin;
 
   LN_INPUT; // make low nibble input
   RW_PIN = 1;                     // Set the control bits for read
   RS_PIN = 0;
   delay_1us ();
   E_PIN = 1;                      // Clock in the command
   delay_1us ();
   port_data = DATA_PORT;
   port_busypin = port_data & 0b00001000; // 0x08;
   E_PIN = 0;              // Reset clock line
   delay_1us ();
   E_PIN = 1;              // Clock in other nibble
   delay_1us ();
   E_PIN = 0;
   RW_PIN = 0;             // Reset control line
   LN_OUTPUT;  // make low nibble output
   return port_busypin;              
   }
/**********************************************************************/
void clock_nibble_out (unsigned char data) // to low nibble of port
   {
   DATA_PORT &= 0xf0;  // clear low nibble of port
   data      &= 0x0f;  // zero any bits in upper nibble of data
   DATA_PORT |= data;  // set data to port  
   delay_1us ();
   E_PIN = 1;                      // Clock the data out
   delay_1us ();
   E_PIN = 0;                                
   }
/**********************************************************************/
void LCD_command (unsigned char cmd)  // 
   {
   LN_OUTPUT;  // low nibble output
   RW_PIN = 0;                     // Set control signals for command
   RS_PIN = 0;
   clock_nibble_out (cmd >> 4); // shift upper nibble to lower nibble
   clock_nibble_out (cmd);      // this outputs lower nibble
   }
/**********************************************************************/
void LCD_init (void)
   {
   // LCD will be used in 4-bit mode. 
   // clock_nibble_out uses lower nibble of data port
   // LCD defaults on power-up to 8-bit mode
   
   TRIS_RW = 0;                    // set RW pin for output
   TRIS_RS = 0;                    // set RS pin for output
   TRIS_E  = 0;                    // set  E pin for output
   RW_PIN  = 0;                    // R/W pin made low
   RS_PIN  = 0;                    // Register select pin made low
   E_PIN   = 0;                    // Clock pin made low

   delay_20ms (); // Delay 15ms min to allow for LCD power-on reset  # 1
        
   LN_OUTPUT;    // set data port low nibble for output

   clock_nibble_out (0b0011);        // "home"                       # 2
   delay_5ms ();  // Delay for at least 4.1ms
   clock_nibble_out (0b0011);        // "home"                       # 4
   delay_5ms ();       // Delay for at least 100us                   # 5

   clock_nibble_out (0b0011);        // "home"                       # 6
   delay_5ms ();       // Delay for at least 100us
   clock_nibble_out (0b0010);   // set LCD interface to 4-bit        # 7
   delay_5ms ();

   clock_nibble_out (0b0010);   // set LCD interface to 4-bit
   clock_nibble_out (0b1000);   // 1 = 2-lines, 0 = 5x7, 00 = xx

   while (LCD_busy ());              // Wait if LCD busy
   LCD_command (0x01);             // Clear display - takes ~16 ms
   delay_20ms ();
   LCD_command (DISPLAY_ON & CURSOR_OFF & BLINK_OFF);   
   }
/**********************************************************************/
void LCD_WriteChar (char data)
   {
   LN_OUTPUT;  // low nibble output
   RW_PIN = 0;                     // Set control signals for data
   RS_PIN = 1;
   clock_nibble_out (data >> 4); // shift upper nibble to lower nibble
   clock_nibble_out (data);      // this outputs lower nibble
   LN_INPUT;
   }
/**********************************************************************/
void LCD_display (unsigned char line, unsigned char position, 
      char text[16])
   {
   int i = 0;
   unsigned char address;
   
   address = position - 1;
   if (line == 2) address += 0x40;
   address += 0b10000000;
   while (LCD_busy ()); 
   LCD_command (address);
   while (text[i])                  // write data to LCD up to null
      {
	  while (LCD_busy ());      // wait while LCD is busy
      LCD_WriteChar (text[i]); // write character to LCD
      i++; // step to next character
      }
   }
/**********************************************************************/
void LCD_blink_position (unsigned char line, unsigned char position)
   {
   unsigned char address;

   address = position - 1;
   if (line == 2) address += 0x40;
   address += 0b10000000;
   while (LCD_busy ()); 
   LCD_command (address);
   while (LCD_busy ()); 
   LCD_command (DISPLAY_ON & CURSOR_ON & BLINK_ON);
   }
/**********************************************************************/
void LCD_normal (void)
   {
   while (LCD_busy ()); 
   LCD_command (DISPLAY_ON & CURSOR_OFF & BLINK_OFF);
   }


/*                                 END                                */
/**********************************************************************/
