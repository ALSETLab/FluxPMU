//////////////////////////////////////////////////////////////
//	more_FNC - 03/01/2007									//
//	Helper Functions for use of PIC Starter Board			//
//////////////////////////////////////////////////////////////

#include <p18cxxx.h>  
#include <stdlib.h>
#include <delays.h>

// Global variables
//
unsigned int Timer1_scale = 0;
unsigned int Timer1_ideal = 0;
unsigned int Timer1_read  = 0;
unsigned int Timer1_bound = 0;
unsigned int Timer1_corrected = 0;

unsigned int Timer1_wait[110];
unsigned int PLL_coarse_adjust = 70;  	// 16 Found to be the case on "Clock 2"
float PLL_adjust = 0; 					// total desired fine adjustment
float PLL_adjsgn = 0;   				// sign of the adjustment
float PLL_freq   = 0; 					// operating frequency
float PLL_space1 = 0; 					// increment spacing
float PLL_space0 = 0;					// previous increment spacing
int   PLL_i = 0;						// pointer in the Timer1_wait[] delay line
int   PLL_fuzzy_event = 0;				// for debugging, lets you watch what the Fuzzy logic is doing

int Freq1	= 99;
int Freq1_old = 0;
int Clock1	= 0;
int Tick1   = 0;
float Multiplier = 1.0;

int RB0_time = 0;
int RB0_flag = 0;
int RB0_secs = 0;

// Function prototypes
//
void UpdateTime(void);
void PLL_speed(void);
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

	if(INTCONbits.INT0IF == 1){							// 1PSS Interrupt (RB0)
		
		PORTBbits.RB2 = 0;
		Timer1_read = ReadTimer1();
		WriteTimer1( 65536 - Timer1_wait[0] );

		PPS_total_time = ReadTimer0();
		WriteTimer0(0);
		PIE1bits.TMR1IE = 1;
		PIR1bits.TMR1IF = 0;

		Clock1 = 0;
	
		UpdateTime();

		INTCONbits.INT0IF = 0;							// Clear interrupt
	}
	else if(INTCON3bits.INT1IF == 1){					// Time Stamp Interrupt (RB1)
		
		RB0_time = ReadTimer0();
		RB0_secs = seconds;
		RB0_flag = 1;
	
		INTCON3bits.INT1IF = 0;							// Clear interrupt
	}
	else if(PIR1bits.TMR1IF == 1){						// TIMER1 Interrupt
	
		PORTBbits.RB2 = !PORTBbits.RB2;
		WriteTimer1( 65536 - Timer1_wait[Clock1 + 1] );
	
		if(Clock1 >= ((Freq1*2)-2) ){
			PIE1bits.TMR1IE = 0;						// Disable TMR1 after correct
		}												// number of cycles	
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

	if		(months == 1 && days >= 31){	// Jan
		months++;
		days = 1;
	}
	else if	(months == 2 && days >= 28 && years%4 != 0){	// Feb (Normal)
		months++;
		days = 1;
	}
	else if	(months == 2 && days >= 29 && years%4 == 0){	// Feb (Leap)
		months++;
		days = 1;
	}
	else if	(months == 3 && days >= 31){	// March
		months++;
		days = 1;
	}
	else if	(months == 4 && days >= 30){	// April
		months++;
		days = 1;
	}
	else if	(months == 5 && days >= 31){	// May
		months++;
		days = 1;
	}
	else if	(months == 6 && days >= 30){	// June
		months++;
		days = 1;
	}
	else if	(months == 7 && days >= 31){	// July
		months++;
		days = 1;
	}
	else if	(months == 8 && days >= 31){	// Aug
		months++;
		days = 1;
	}
	else if	(months == 9 && days >= 30){	// Sept
		months++;
		days = 1;
	}
	else if	(months == 10 && days >= 31){	// Oct
		months++;
		days = 1;
	}	
	else if	(months == 11 && days >= 30){	// Nov
		months++;
		days = 1;
	}
	else if	(months >= 12 && days >= 31){	// Dec
		years++;
		months = 1;
		days = 1;
	}

}

//-----------------------------------------------------
// PLL Speed
//
void PLL_speed(void){
	
	Freq1_old = Freq1;

	if(PORTBbits.RB4 == 1 && PORTBbits.RB5 == 0){
		Freq1 = 25;
	}
	else if(PORTBbits.RB4 == 0 && PORTBbits.RB5 == 1){
		Freq1 = 50;
	}
	else if(PORTBbits.RB4 == 1 && PORTBbits.RB5 == 1){
		Freq1 = 10;
	}
	else{
		Freq1 = 10;
	}

	if(Freq1 != Freq1_old){

		Multiplier = (float) 128 / ( (float) 4 * (float) (Freq1 * 2) );
		Timer1_ideal = (unsigned int) ((float) 31250 * Multiplier ); //PPS_total_time (ideal) * Multiplier
		Timer1_scale = Timer1_ideal - PLL_coarse_adjust;
	}

	Timer1_bound = Freq1;
	Timer1_corrected = Timer1_read - 80;


	if(      Timer1_corrected > 65535 - Timer1_bound ){											// Decrease slightly
											PLL_adjust--; 
											PLL_coarse_adjust = Timer1_ideal - Timer1_scale; 
											PLL_fuzzy_event = -10;}
	else if( Timer1_corrected > 60000 ){														// Decrease greatly
											Timer1_scale = Timer1_scale - 1; 
											PLL_fuzzy_event = -100;}

	else if( Timer1_corrected <     0 + Timer1_bound ){ 										// Increase slightly
											PLL_adjust++; 
											PLL_coarse_adjust = Timer1_ideal - Timer1_scale; 
											PLL_fuzzy_event = 10;}	
	else if( Timer1_corrected < 10000 ){ 														// Increase greatly
											Timer1_scale = Timer1_scale + 1; 
											PLL_fuzzy_event = 100;}			

	PLL_adjsgn = PLL_adjust / fabs(PLL_adjust);		// Find the sign of the corrections
	PLL_freq   = (float) Freq1 * 2;					// Number of transitions
	PLL_space0 = fabs(PLL_adjust) / PLL_freq;		// Spacing of adjustments (1st Adjustment)
	PLL_space1 = 0;									// PLL_space1 (n) is to PLL_space0 (n-1)

	PLL_i = 0;

	while(PLL_i < PLL_freq){

		PLL_space1 = PLL_space0 + fabs(PLL_adjust) / PLL_freq;

		if( (int)PLL_space1 > (int)PLL_space0 ){ Timer1_wait[PLL_i] = Timer1_scale + PLL_adjsgn; }
		else{ Timer1_wait[PLL_i] = Timer1_scale; }

		PLL_space0 = PLL_space1;
		PLL_i++;
	}

}

/* END OF FILE */