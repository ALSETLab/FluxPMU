//////////////////////////////////////////////////////////////
//	Source for Nova Robotics GPS Clock (with PPL)			//
//	Version 5.0 - For 18F252 Device							//
//	30/06/2010												//
//////////////////////////////////////////////////////////////
//
// Global Variables
//
unsigned int PPS_initial_time = 0;
unsigned int PPS_total_time = 0;
unsigned int PPS_high_time = 0;
int years = 0;
int months = 0;
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;
int lock = 0;
int lock_hour = 0;
int lock_mins = 0;
int timecheck = 0;

//$GPRMC,040302.663,A,3939.7123,N,10506.6123,W,0.27,358.86,200804,,*1A
char position[] =    "                        ";

// Header Files
//
#include <p18f2525.h>
#include <timers.h>
#include <usart.h>
#include <stdlib.h>
#include <delays.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <portb.h>
#include "dml_common.c"	// David's useful functions
#include "gps_232.c"	// Reads the GPRMC line of NMEA
#include "pps_time.c"	// Accurate 1PPS time keeping

// Pre-processor Instructions
//
#pragma config OSC = HSPLL				// - set HS oscillator
#pragma config WDT = OFF, WDTPS = 128	// - enable watchdog timer
#pragma config LVP = OFF				// - disable low voltage programming
#pragma config DEBUG = ON				// - enable background debugging

// Function Prototypes
//
void setup(void);
void bootup(void);
void time_stamp(void);
void read_time(void);
void num2txt(long, long);

//-------------------------------------------------------------
// -- Setup
void setup(void){

	ADCON1 = 0b00001110;			// Set PORTA 1.2.3 for LCD IO

	Delay10TCYx( 5 );				// Wait a moment
		    //76543210
	TRISA = 0b11110011;				// Port A IO
	TRISB = 0b11110011;				// Port B IO
	TRISC = 0b11001111;				// Port C IO
	
	OpenTimer0(TIMER_INT_OFF &		// Open Timer0
		T0_16BIT & 
		T0_SOURCE_INT & 
		T0_PS_1_128);

	OpenTimer1(TIMER_INT_ON & 		// Open Timer1
		T1_16BIT_RW & 
		T1_SOURCE_INT & 
		T1_PS_1_4);
	
	OpenUSART (USART_TX_INT_OFF &	// Open the USART
		USART_RX_INT_OFF &
		USART_ASYNCH_MODE &	
		USART_EIGHT_BIT &			// Configured as 8N1, in polled mode
		USART_CONT_RX &
		USART_BRGH_HIGH, 207);		// 4800 Baud (using clock multiplier)*/

	OpenRB0INT (PORTB_CHANGE_INT_ON & 	// Enable the RB0/INT0 interrupt
		PORTB_PULLUPS_ON & 				// Configure the RB0 pin for input
		RISING_EDGE_INT); 				// Trigger interrupt upon falling edge
	
	OpenRB1INT (PORTB_CHANGE_INT_ON & 	// Enable the RB0/INT0 interrupt
		PORTB_PULLUPS_ON & 				// Configure the RB0 pin for input
		RISING_EDGE_INT); 				// Trigger interrupt upon falling edge

	RCONbits.IPEN = 0;				// Enable interrupt priority
	IPR1bits.RCIP = 0;				// Make receive interrupt low priority
	IPR1bits.TMR1IP = 1;			// Set to low priority Interrupt
	INTCONbits.GIEH = 1;			// Enable all high priority interrupts
	INTCONbits.GIEL = 1;			// Enable all low priority interrupts
}

// ************* MAIN *************
//
void main(void){

	int i = 0;

	Delay10KTCYx(100);
	setup();

	INTCONbits.GIEH = 0;		// INTERRUPTS
	INTCONbits.GIEL = 0;		// OFF

	PORTAbits.RA2 = 0;
	PORTAbits.RA3 = 1;
	PORTAbits.RA4 = 1;

	bootup();

	PLL_speed();									// Check required PLL speed	
			
	WriteTimer1(0);

	INTCONbits.GIEH = 1;		// INTERRUPTS
	INTCONbits.GIEL = 1;		// ON

	while(1){
		
		if( seconds == 0 && timecheck == 0 ){		// Check the GPS receiver's time once a minute
			read_time();
			timecheck = 1;	
		}
		else if( seconds >= 1 && timecheck == 1 ){
			timecheck = 0;
		}			

		if( Clock1 == 0 && Tick1 == 0){				// Output a 'tick' to the RS232 port
			if(lock == 1){ putrsUSART((const far rom char *)"*"); }
			else{ putrsUSART((const far rom char *)"-"); }
			PORTAbits.RA3 = seconds%2;				// Toggle RD6 to indicate 1PPS transition
			Tick1 = 1;

			PLL_speed();							// Check required PLL speed
		}
		else if( Clock1 != 0 ){
			Tick1 = 0;
		}

		if( RB0_flag == 1 ){						// Service the request for a time stamp
			time_stamp();
		}
	
	}
}

//-----------------------------------------------------
// TIME STAMP
//
void time_stamp(void){

	char temp_txt[] = "          ";
	int i = 0;
	int j = 0;

	// $CLOCK,<hh:mm:ss.mmm>,<dd/mm/yy>,<lock (A/V)>,<PLL Freq>,<lat>,<log>,<lock - hh:mm>*
	
	RB0_time = (int) (1000 * (float) RB0_time / (float) PPS_total_time);

	putrsUSART((const far rom char *)"\n\r$CLOCK,");
	num2txt( hours,2 );
	putrsUSART((const far rom char *)":");
	num2txt( minutes,2 );
	putrsUSART((const far rom char *)":");
	num2txt( RB0_secs,2 );
	putrsUSART((const far rom char *)".");
	num2txt( RB0_time,3 );
	putrsUSART((const far rom char *)",");
	num2txt( days,2 );
	putrsUSART((const far rom char *)"/");
	num2txt( months,2 );
	putrsUSART((const far rom char *)"/");
	num2txt( years,2 );
	putrsUSART((const far rom char *)",");

	if(lock == 1){
		putrsUSART((const far rom char *)"A");
	}
	else{
		putrsUSART((const far rom char *)"V");
	}
	putrsUSART((const far rom char *)",");
	num2txt( Freq1,2 );
	putrsUSART((const far rom char *)",");
	
	i = 0;
	j = 0;
	while( j == 0 ){
		
		if(position[i] == 'E' || position[i] == 'W'){ 
			j = 1;
		}
		putcUSART( position[i] );
		while( BusyUSART() );
		i++;	
	}
	putrsUSART((const far rom char *)",");
	num2txt( lock_hour,2 );
	putrsUSART((const far rom char *)":");
	num2txt( lock_mins,2 );
	putrsUSART((const far rom char *)"*");

	RB0_flag = 0;
}

//-----------------------------------------------------
// READ TIME
//
void read_time(void){

	read_gps();							// is received, and time can be assumed valid.
	seconds = seconds - 1;					// Time seems to be 1 second ahead of UTC without this correction
	PORTAbits.RA2 = lock;					// Indicate lock on RD7
}

//-------------------------------------------------------------
// -- BOOT UP
void bootup(void){
										
	putrsUSART((const far rom char *)"\n\r");
										//0123456789012345678901234567890123456789
	putrsUSART((const far rom char *)"\n\r****************************************");
	putrsUSART((const far rom char *)"\n\r*** NOVA ROBOTICS - GPS CLOCK - V5.0 ***");
	putrsUSART((const far rom char *)"\n\r****************************************");
	putrsUSART((const far rom char *)"\n\r");
	putrsUSART((const far rom char *)"\n\rSystem Has Booted");
	putrsUSART((const far rom char *)"\n\r");
	putrsUSART((const far rom char *)"\n\r> Waiting for satellite lock");

	lock = 0;
	while(lock == 0){					// Run the Read_GPSlock fucntion until a satellite fix
		read_gps();						// is received, and time can be assumed valid.
	}

	putrsUSART((const far rom char *)"\n\r> Satellite lock established");
	putrsUSART((const far rom char *)"\n\r> Calibrating to 1PPS reference");
	
	while( PORTBbits.RB0 == 1 );
	while( PORTBbits.RB0 == 0 );
	WriteTimer0(0);
	while( PORTBbits.RB0 == 1 );
	PPS_high_time = ReadTimer0();			// Duration of the '1' in the 1PPS
	while( PORTBbits.RB0 == 0 );
	PPS_total_time = ReadTimer0();			// Total period of the 1PPS
	PPS_initial_time = ReadTimer0();		// Total period of the 1PPS

	putrsUSART((const far rom char *)"\n\r>> PPS_total_time ");
	num2txt(PPS_total_time,5);
	putrsUSART((const far rom char *)"\n\r>> PPS_high_time  ");
	num2txt(PPS_high_time,5);

	putrsUSART((const far rom char *)"\n\r> Reading NMEA Format Time  ");

	read_time();

	putrsUSART((const far rom char *)"\n\r");
	putrsUSART((const far rom char *)"\n\r*****************************");
	putrsUSART((const far rom char *)"\n\r*** ");
	num2txt(hours,2);
	putrsUSART((const far rom char *)":");
	num2txt(minutes,2);
	putrsUSART((const far rom char *)":");
	num2txt(seconds,2);
	putrsUSART((const far rom char *)" - ");
	num2txt(days,2);
	putrsUSART((const far rom char *)"/");
	num2txt(months,2);
	putrsUSART((const far rom char *)"/20");
	num2txt(years,2);
	putrsUSART((const far rom char *)" ***");
	putrsUSART((const far rom char *)"\n\r*****************************");
	putrsUSART((const far rom char *)"\n\r");

}

// ********* END OF FILE **********




