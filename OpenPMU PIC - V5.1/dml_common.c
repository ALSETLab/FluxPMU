//////////////////////////////////////////////////////////////
//	David's commonly used useful source code				//
//	Version 1.0 - (c) DML 2008								//
//	18/04/2008												//
//////////////////////////////////////////////////////////////
//
// Header Files
//
#include <p18f2525.h>
#include <delays.h>

// Function Prototypes
//

// Global Variables
//
rom char STR_DECIMALPOINT[]=".";

//-------------------------------------------------------------
// -- NUMBER TO TEXT - V2
void num2txt(long x, long n){

int digit[10],i;

	if(x>9999999){x = 9999999;}
	if(x<0){x = 0;}	

	i = 0;
	while(i < n){
		digit[i] = x%10;
		x = x/10;
		i++;
	}

	i = 0;
	while(i < n){
		while(BusyUSART());
		WriteUSART(digit[n-1-i] + 48);
		i++;
	}
}
// ********* END OF FILE **********