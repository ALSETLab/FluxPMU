//////////////////////////////////////////////////////////////
//	more_FNC - 03/01/2007									//
//	Helper Functions for use of PIC Starter Board			//
//////////////////////////////////////////////////////////////

#include <p18cxxx.h>  
#include <stdlib.h>
#include <delays.h>

// Global variables
//
unsigned int Timer1 	= 0;
int Freq1	= 50;
int Clock1	= 0;
int Tick1   = 0;
float Multiplier = 1.0;

// Function prototypes
//
void UpdateTime(void);
void high_ISR2(void);
void low_ISR2(void);

//-------------------------------------------------------------
// -- Interrupt stuff - RB0 / RB1
#pragma code HIGH_INTERRUPT_VECTOR = 0x8
void high_ISR(void)
{
	_asm
	goto high_ISR2
	_endasm		
}
#pragma code

#pragma interrupt high_ISR2
void high_ISR2(void){

	if(INTCONbits.INT0IF == 1){							// Time Stamp Interrupt (RB0)
		
		RB0_time = ReadTimer0();
		RB0_flag = 1;
	
		INTCONbits.INT0IF = 0;							// Clear interrupt
	}
	else if(INTCON3bits.INT1IF == 1){					// 1PSS Interrupt (RB1)
		
		PORTDbits.RD5 = 0;
		WriteTimer1( 65536 - Timer1 );

		PPS_total_time = ReadTimer0();
		WriteTimer0(0);
		PIE1bits.TMR1IE = 1;
		PIR1bits.TMR1IF = 0;

		Clock1 = 0;
	
		UpdateTime();
	
		INTCON3bits.INT1IF = 0;							// Clear interrupt
	}
	else if(PIR1bits.TMR1IF == 1){						// TIMER1 Interrupt

		PORTDbits.RD5 = !PORTDbits.RD5;
		WriteTimer1( 65536 - Timer1 );
	
		if(Clock1 >= ((Freq1*2)-2) ){
			PIE1bits.TMR1IE = 0;
		}
		Clock1++;

		PIR1bits.TMR1IF = 0; 							// Clear interrupt
	}
}

//--------------------------------------------------------
// -- Low priority interrupt vector
#pragma code LOW_INTERRUPT_VECTOR = 0x18
void low_ISR(void){
 	_asm 
	goto low_ISR2 
	_endasm
}
#pragma code

#pragma interruptlow low_ISR2
void low_ISR2(void){

}

//-------------------------------------------------------------
// -- Updates system time
void UpdateTime(void){

	seconds++;
	if(seconds >= 60){
		minutes++;
		seconds = 0;}
	if(minutes >= 60){
		hours++;
		minutes = 0;}
	if(hours >= 24){
		days++;
		hours = 0;}

	if		(months == 0 && days >= 31){	// Jan
		months++;
		days = 0;
	}
	else if	(months == 1 && days >= 28 && years%4 != 0){	// Feb (Normal)
		months++;
		days = 0;
	}
	else if	(months == 1 && days >= 29 && years%4 == 0){	// Feb (Leap)
		months++;
		days = 0;
	}
	else if	(months == 2 && days >= 31){	// March
		months++;
		days = 0;
	}
	else if	(months == 3 && days >= 30){	// April
		months++;
		days = 0;
	}
	else if	(months == 4 && days >= 31){	// May
		months++;
		days = 0;
	}
	else if	(months == 5 && days >= 30){	// June
		months++;
		days = 0;
	}
	else if	(months == 6 && days >= 31){	// July
		months++;
		days = 0;
	}
	else if	(months == 7 && days >= 31){	// Aug
		months++;
		days = 0;
	}
	else if	(months == 8 && days >= 30){	// Sept
		months++;
		days = 0;
	}
	else if	(months == 9 && days >= 31){	// Oct
		months++;
		days = 0;
	}	
	else if	(months == 10 && days >= 30){	// Nov
		months++;
		days = 0;
	}
	else if	(months >= 11 && days >= 31){	// Dec
		months++;
		days = 0;
	}

	if(months >= 12){
		years++;
		months = 0;
	}
}

/* END OF FILE */