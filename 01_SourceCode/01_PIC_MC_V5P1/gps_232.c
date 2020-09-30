// Header Files
//
#include <p18f2525.h>
#include <string.h>
#include <delays.h>

// Function Prototypes
//
void read_gps(void);
void gp_rmc(void);

// Global Variables
//
#pragma idata largebank			// Since this is so long
char gps_in[80] = "Abcdef";		// it is put in a special
#pragma idata					// section of memory


//-------------------------------------------------------------
// -- Read GPS Time and Active Lock
void read_gps(void){

	int	i = 0;
	int gprmc = 0;
	char in_temp = 0;

	while( gprmc == 0 ){		// NMEA sentences which MUST be read

		if (RCSTAbits.OERR){
		     RCSTAbits.CREN=0; /* disable / enable receiver   */
		     RCSTAbits.CREN=1; /* to clear over-run condition */
		   	}
		
		while(i == 0){
			while(!DataRdyUSART());
			in_temp = ReadUSART();
			if(in_temp == '$'){ i = 1; }		
		}
		
		if (RCSTAbits.OERR){
		     RCSTAbits.CREN=0; /* disable / enable receiver   */
		     RCSTAbits.CREN=1; /* to clear over-run condition */
		   	}
		
		i = 0;
		
		while(i<80){
			
			while(!DataRdyUSART());
			
			gps_in[i] = ReadUSART();

			if(i == 4){ 									// This section of code
				if(gps_in[4] == 'C'){ /* Continue */}		// checks for the 'C' in
				else{gps_in[i] = '*';}						// GPRMC.  Remove if listening
			 }												// for other strings
		
			if(gps_in[i] == '*'){
				i = 81;
			}
			
			i++;
			if(i == 80){i = 0;}
		}
		
		if( gps_in[0] == 'G' &&
			gps_in[1] == 'P' &&
			gps_in[2] == 'R' &&
			gps_in[3] == 'M' &&
			gps_in[4] == 'C' ){ 

				gprmc = 1;			
				gp_rmc();
		}
	}
}


//-------------------------------------------------------------
// -- NMEA - GPRMC
void gp_rmc(void){
	
	// This is reading commas until the 7th location, then grabs the data

	int comma_count = 0;
	int comma_service = 0;
	int gps_count = 0;
	char chr_temp = 48;

	char temp_txt[] = "          ";

	int i = 0;
	int j = 0;

	//                01234567890123456789
	//$GPRMC,113405,A,5438.22,N,00556.16,W,000.0,229.9,090706,007.5,W*75

	while(chr_temp != '*' && gps_count <= strlen(gps_in)){
		
		chr_temp = gps_in[ gps_count ];
	
		if(chr_temp == ','){ 
			comma_count++;
			comma_service = 0; 
		}	
		else if(comma_count == 1 && comma_service == 0){		// 1st Comma = TIME

			hours 	= 10 * ( gps_in[ gps_count + 0 ] - 48 ) + ( gps_in[ gps_count + 1 ] - 48 );
			minutes = 10 * ( gps_in[ gps_count + 2 ] - 48 ) + ( gps_in[ gps_count + 3 ] - 48 );
			seconds = 10 * ( gps_in[ gps_count + 4 ] - 48 ) + ( gps_in[ gps_count + 5 ] - 48 );

			comma_service = 1;
		}
		else if(comma_count == 2 && comma_service == 0){		// 2nd Comma = STATUS

			if(gps_in[ gps_count + 0 ] == 'A'){
				lock = 1; 
				lock_hour = hours;
				lock_mins = minutes;
			}
			else{ lock = 0; }
	
			comma_service = 1;
		}
		else if( comma_count == 3 && comma_service == 0){		// 3rd Comma = LATITUDE

			i = 0;
			j = 0;
			while( j < 4 ){

				if( gps_in[ gps_count + i ] == ',' ){
					j++; 
				}
				position[i] = gps_in[ gps_count + i ];
				i++;
			}

			comma_service = 1;
		}
		else if(comma_count == 9 && comma_service == 0){		// 9th Comma = DATE

			days 	= 10 * ( gps_in[ gps_count + 0 ] - 48 ) + ( gps_in[ gps_count + 1 ] - 48 );
			months  = 10 * ( gps_in[ gps_count + 2 ] - 48 ) + ( gps_in[ gps_count + 3 ] - 48 );
			years   = 10 * ( gps_in[ gps_count + 4 ] - 48 ) + ( gps_in[ gps_count + 5 ] - 48 );

			comma_service = 1;
		}

		gps_count++;
	}

}


