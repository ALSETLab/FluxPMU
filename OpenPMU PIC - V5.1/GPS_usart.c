//////////////////////////////////////////
// Speedo Command Prompt - 08/07/2006	//
//////////////////////////////////////////

// Header Files
//
#include <delays.h>

// Function Prototypes
//
void read_gps(void);
void gps_rmc(void);
void gps_bwc(void);
void rx_handler(void);
void speed_parse(void);
void bearing_parse(void);
void wptbear_parse(void);
void wptdist_parse(void);
void desttime_calc(void);
void time_circuits(void);
void flux_dispersal(void);

// Global Variables
//
int usart_point = 0;
int i=0;
int loopcount = 0;
int first_run = 1;
int no_speed = 1;
int no_bearing = 1;
int no_wptbear = 1;
int no_wptdist = 1;
int screen_page = 1;
int dest_vel = 0;
double theta = 0;
float timetodest = 0;
int dest_hours = 0;
int dest_mins = 0;
int dest_secs = 0;

char splash[16] = "   Test Test  ";
char speed_temp[6];
char bearing_temp[6];
char wptdist_temp[6];
char wptbear_temp[6];
char dest_time[7] = "------";
char in_temp;

#pragma idata largebank			// Since this is so long
char gps_in[80] = "Abcdef";		// it is put in a special
#pragma idata					// section of memory

void read_gps(void){
	
	i=0;
	while(i < 6){
		speed_temp[i] = ',';
		bearing_temp[i] = ',';
		wptdist_temp[i] = ',';
		wptbear_temp[i] = ',';
		i++;
	}

	while(1){
		i=0;
		
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

		i=0;
		while(i<80){
			
			while(!DataRdyUSART());
			
			gps_in[i] = ReadUSART();

			if(i == 4){ 
				if(gps_in[4] == 'C'){ /* Continue */}
				else{gps_in[i] = '*';}
			 }
			
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
			
			putsUSART(splash);			// A useful test signal
			
			gps_rmc();				// Finds the location of data
		}
		
		if( gps_in[0] == 'G' &&
			gps_in[1] == 'P' &&
			gps_in[2] == 'B' &&
			gps_in[3] == 'W' &&
			gps_in[4] == 'C' ){ 
			
			gps_bwc();				// Finds the location of data
			
			if(loopcount >= 36 && loopcount < 72){ 
				screen_page = 2;
			}
			else{screen_page = 1;} 
			
			desttime_calc();
			time_circuits();
			flux_dispersal();
			LCD_page(screen_page); 
		}

		dis_speed_slew();			// Functions for Red LEDs
		if(no_speed == 1){ segs(10,0); }
		else{ red_speed( dis_speed ); }

		Delay10KTCYx( 5 );
		
		loopcount++;
		if(loopcount >= 72){ loopcount = 0; }
	}
}

//-------------------------------------------------------------
// -- GPS Parser RMC
void gps_rmc(void){
	
	// This is reading commas until the 7th location, then grabs the data

	int comma_count = 0;
	int gps_count = 0;
	int speed_count = 0;
	int bearing_count = 0;
	int finish = 0;
	int data_length = 0;
	char chr_temp = 48;

	data_length = strlen(gps_in);

	while(chr_temp != '*' && finish != 1){
		
		chr_temp = gps_in[ gps_count ];
		
		if(comma_count == 7){
			speed_temp[ speed_count ] = chr_temp;
			speed_count++;
		}

		if(comma_count == 8){
			bearing_temp[ bearing_count ] = chr_temp;
			bearing_count++;
		}
		
		if(chr_temp == ','){ comma_count++; }
		if(gps_count == data_length){ finish = 1; }
		gps_count++;
	}

	speed_parse();
	bearing_parse();
}

//-------------------------------------------------------------
// -- SPEED Parser
void speed_parse(void){
	
	// This coverts ASCII float into FLOAT
	
	int speed_count = 0;
	int finish = 0;
	int decimal = 0;
	int decimal_count = 0;
	int speed = 0;
	double speedf = 0;

	if( speed_temp[0] == ','){ no_speed = 1; }
	else{
		no_speed = 0;
		while( finish != 1){
			speed = (speed * 10) + (speed_temp[speed_count] - 48);
			speed_count++;
	
			if( decimal == 1 ){ decimal_count++; }
			if( speed_temp[speed_count] == '.' ){ 
				speed_count++;
				decimal = 1;
			}
			if( speed_temp[speed_count] == ',' ){ 
				finish = 1;
			}
		}

		speedf = (double) speed / pow(10,decimal_count);
		speedf = speedf * 1.852;
		gps_speed = (int) speedf;
	}
}

//-------------------------------------------------------------
// -- BEARING Parser
void bearing_parse(void){
	
	// This coverts ASCII float into FLOAT
	
	int bearing_count = 0;
	int finish = 0;
	int decimal = 0;
	int decimal_count = 0;
	int bearing = 0;

	if( bearing_temp[0] == ','){ no_bearing = 1; }
	else{
		no_bearing = 0;
		while( finish != 1){
			bearing = (bearing * 10) + (bearing_temp[bearing_count] - 48);
			bearing_count++;
	
			if( decimal == 1 ){ decimal_count++; }
			if( bearing_temp[bearing_count] == '.' ){ 
				bearing_count++;
				decimal = 1;
			}
			if( bearing_temp[bearing_count] == ',' ){ 
				finish = 1;
			}
		}

		bearing = bearing / pow(10,decimal_count);
		gps_bearing = bearing;
	}
}

//-------------------------------------------------------------
// -- WAYPOINT BEARING Parser
void wptdist_parse(void){
	
	// This coverts ASCII float into FLOAT
	
	int count = 0;
	int finish = 0;
	int decimal = 0;
	int decimal_count = 0;
	int data = 0;
	float dataf = 0;

	if( wptdist_temp[0] == ','){ no_wptdist = 1; }
	else{
		no_wptdist = 0;
		while( finish != 1){
			data = (data * 10) + (wptdist_temp[count] - 48);
			count++;
	
			if( decimal == 1 ){ decimal_count++; }
			if( wptdist_temp[count] == '.' ){ 
				count++;
				decimal = 1;
			}
			if( wptdist_temp[count] == ',' ){ 
				finish = 1;
			}
		}
		
		dataf = (float) data / pow(10,decimal_count);
		dataf = dataf * 1.852;
		data  = (int) dataf;
		gps_wptdist = data;
	}
}

//-------------------------------------------------------------
// -- WAYPOINT BEARING Parser
void wptbear_parse(void){
	
	// This coverts ASCII float into FLOAT
	
	int count = 0;
	int finish = 0;
	int decimal = 0;
	int decimal_count = 0;
	int data = 0;

	if( wptbear_temp[0] == ','){ no_wptbear = 1; }
	else{
		no_wptbear = 0;
		while( finish != 1){
			data = (data * 10) + (wptbear_temp[count] - 48);
			count++;
	
			if( decimal == 1 ){ decimal_count++; }
			if( wptbear_temp[count] == '.' ){ 
				count++;
				decimal = 1;
			}
			if( wptbear_temp[count] == ',' ){ 
				finish = 1;
			}
		}
		
		data = data / pow(10,decimal_count);
		gps_wptbear = data;
	}
}

//-------------------------------------------------------------
// -- Destination Time Calculator
void desttime_calc(void){
	
	int tens = 0;
	int units = 0;
	
	theta = (double) ( gps_bearing - gps_wptbear );
	theta = theta * 0.0174533;	
	dest_vel = gps_speed * cos( theta );
	if(dest_vel < 1){
		dest_time[0] = '-';
		dest_time[1] = '-';
		dest_time[2] = '-';
		dest_time[3] = '-';
		dest_time[4] = '-';
		dest_time[5] = '-';
	}
	else{
		timetodest = (float) gps_wptdist / (float) dest_vel;
		dest_hours = (int) timetodest;
		timetodest = ( timetodest - (float) dest_hours ) * 60;
		dest_mins  = (int) timetodest;
		timetodest = ( timetodest - (float) dest_mins ) * 60;
		dest_secs  = (int) timetodest;

		dest_secs = dest_secs + (gps_in[10] - 48)*10 + (gps_in[11] - 48);
		if(dest_secs > 59){ 
			dest_secs = dest_secs - 60;
			dest_mins = dest_mins + 1;
		}
		dest_mins = dest_mins + (gps_in[8] - 48)*10 + (gps_in[9] - 48);
		if(dest_mins > 59){ 
			dest_mins = dest_mins - 60;
			dest_hours = dest_hours + 1;
		}
		dest_hours = dest_hours + (gps_in[6] - 48)*10 + (gps_in[7] - 48);
		if(dest_hours > 23){ 
			dest_hours = dest_hours - 24;
		}
		
		units = dest_hours%10;
		dest_hours = dest_hours/10;
		tens  = dest_hours%10;
		dest_time[0] = tens + 48;
		dest_time[1] = units + 48;

		units = dest_mins%10;
		dest_mins = dest_mins/10;
		tens  = dest_mins%10;
		dest_time[2] = tens + 48;
		dest_time[3] = units + 48;

		units = dest_secs%10;
		dest_secs = dest_secs/10;
		tens  = dest_secs%10;
		dest_time[4] = tens + 48;
		dest_time[5] = units + 48;
	}

}

//-------------------------------------------------------------
// -- TIME CIRCUITS - LCD Display Module
void time_circuits(void){

	page1_line2[11] = gps_in[6];
	page1_line2[12] = gps_in[7];
	page1_line2[13] = ':';
	page1_line2[14] = gps_in[8];
	page1_line2[15] = gps_in[9];
	page1_line2[16] = ':';
	page1_line2[17] = gps_in[10];
	page1_line2[18] = gps_in[11];

	page1_line3[11] = dest_time[0];
	page1_line3[12] = dest_time[1];
	page1_line3[13] = ':';
	page1_line3[14] = dest_time[2];
	page1_line3[15] = dest_time[3];
	page1_line3[16] = ':';
	page1_line3[17] = dest_time[4];
	page1_line3[18] = dest_time[5];

	if(first_run == 1){
		page1_line4[11] = gps_in[6];
		page1_line4[12] = gps_in[7];
		page1_line4[13] = ':';
		page1_line4[14] = gps_in[8];
		page1_line4[15] = gps_in[9];
		page1_line4[16] = ':';
		page1_line4[17] = gps_in[10];
		page1_line4[18] = gps_in[11];
		first_run = 0;
	}
}

//-------------------------------------------------------------
// -- TIME CIRCUITS - LCD Display Module
void flux_dispersal(void){
	
	int hund,tens,units;
	int temp_var;

	temp_var = gps_speed;
	units = 48 + temp_var%10;
	temp_var = temp_var/10;
	tens  = 48 + temp_var%10;
	temp_var = temp_var/10;
	hund  = 48 + temp_var%10;

	page2_line2[11] = hund;
	page2_line2[12] = tens;
	page2_line2[13] = units;
	page2_line2[14] = ' ';
	page2_line2[15] = 'k';
	page2_line2[16] = 'm';
	page2_line2[17] = '/';
	page2_line2[18] = 'h';

	temp_var = gps_bearing;
	units = 48 + temp_var%10;
	temp_var = temp_var/10;
	tens  = 48 + temp_var%10;
	temp_var = temp_var/10;
	hund  = 48 + temp_var%10;
	
	page2_line3[11] = hund;
	page2_line3[12] = tens;
	page2_line3[13] = units;
	page2_line3[14] = ' ';

	if(gps_bearing >= 337 || gps_bearing <  22 ){ 		// North - special case uses OR
		page2_line3[15] = 'N'; page2_line3[16] = ' ';}	//       - all others use AND
	else if(gps_bearing >= 22 && gps_bearing <  67 ){ 	// NE
		page2_line3[15] = 'N'; page2_line3[16] = 'E';}
	else if(gps_bearing >= 67 && gps_bearing < 112 ){ 	// E
		page2_line3[15] = 'E'; page2_line3[16] = ' ';}
	else if(gps_bearing >= 112 && gps_bearing < 157 ){  // SE
		page2_line3[15] = 'S'; page2_line3[16] = 'E';}
	else if(gps_bearing >= 157 && gps_bearing < 202 ){  // S
		page2_line3[15] = 'S'; page2_line3[16] = ' ';}
	else if(gps_bearing >= 202 && gps_bearing < 247 ){  // SW
		page2_line3[15] = 'S'; page2_line3[16] = 'W';}
	else if(gps_bearing >= 247 && gps_bearing < 292 ){  // W
		page2_line3[15] = 'W'; page2_line3[16] = ' ';}
	else if(gps_bearing >= 292 && gps_bearing < 337 ){  // NW
		page2_line3[15] = 'N'; page2_line3[16] = 'W';}

	temp_var = gps_wptdist;
	units = 48 + temp_var%10;
	temp_var = temp_var/10;
	tens  = 48 + temp_var%10;
	temp_var = temp_var/10;
	hund  = 48 + temp_var%10;

	page2_line4[11] = hund;
	page2_line4[12] = tens;
	page2_line4[13] = units;
	page2_line4[14] = ' ';
	page2_line4[15] = 'k';
	page2_line4[16] = 'm';
}
// END OF FILE
