/*
  *FILE   		  :  AssignmentNo5
  *PROJECT		  :  Assignment#5
  *PROGRAMMER     :  Nimal krishna
  *FIRST VERSION  :  01/12/2016
  *DESCRIPTION    :  This project is  for parsing a sample GPS data and display the parsed data on the LCD screen when a push button is pressed.
					 Further , the time, latitude, longitude, position fix, satellites used, HDOp, geoid separation, DGPS age data are converted to
					 integer and stored, and  displayed on the LCD screen.
					 altitude data is converted to a float value and displayed on the LCD
					 checksum is converted to a byte value and displayed on the LCD.
					 The whole GPS data  is stored in a single string to simulate real time scenario in which continuous flow of data is the norm.
					 If the data send is not a GPS signal, i.e it it don't start with" $GGPA" string or the total number of data sent is not equal to
					 15 an error message is displayed, and the program continues to display the error message till the next " $GGPA"  data is received.
					 The program also uses an effective state machine to switch based on the data extracted position.
					 further all data received are converted into integer(float for altitude, byte for checksum) and stored , and thus is available
					 for any calculations


 */




/* Includes ------------------------------------------------------------------*/

#include "stm32f3xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


/* Global variables  ------------------------------------------------------------------*/

enum variables{

	delayNormal  =  100,
	delayLong    =  500,
	normalLength =  20,
	longLength   =  100,
	veryLongLength =500,
	buttonPressed   =1

};

const char dataSeperator[ 2 ] = ",";		// Used as a separator to parse data from the string
uint32_t dataExtractedPosition = 0;			// Count of data which was extracted from the string
char lcdDisplayText[ normalLength ];		// Used  to display data on the LCD
char *seperatedData;						// Used to store the parsed data

/* Private typedef -----------------------------------------------------------*/
static GPIO_InitTypeDef GPIO_InitStruct;

/* function prototypes -----------------------------------------------*/

static void SystemClock_Config( void );
static void Error_Handler( void );

void HD44780_Init( void );
void HD44780_PutChar( unsigned char c );
void HD44780_GotoXY( unsigned char x , unsigned char y );
void HD44780_PutStr( char *str );
void HD44780_ClrScr( void );

void gpggaDataParsed( void );
void utcTimeDataParsed( void );
void latitudeValueDataParsed( void );
void latitudePositiondDataParsed( void );
void longitudeValueDataParsed( void );
void longitudePositionDataParsed( void );
void positionfixDataParsed( void );
void satellitesUsedDataParsed( void );
void HDOPDataParsed( void );
void DGPSAgeDataParsed( void );
void altitudeValueDataParsed( void );
void altitudeunitsDataParsed( void );
void geoidSeperationValueDataParsed( void );
void geoidSeperationUnitDataParsed( void );
void checksumDataParsed( void );
void portaInit( void );


/*
 * FUNCTION		: main function
 *
 * DESCRIPTION 	: This is the starting point for code execution for the compiler
 *
 * PARAMETERS	: void
 *
 * RETURNS		: integer

 */



int main( void )
{
	HAL_Init( );
	SystemClock_Config( );		    // Configure the system clock to 72 Mhz
	portaInit( );
	HD44780_Init( );				// Lcd initialization
	HD44780_ClrScr( );			    // clear the Lcd screen

	while ( 1 )
	{

		char gpsData[ veryLongLength ] =
						"$GPGGA,092750.10,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M, ,*76,"
						"$GPGGA,092751.000,5321.6802,N,00630.3371,W,1,8,1.03,61.7,M,55.3,M, ,*75,"
						"$GPGGA,014729.10,4303.5753,N,08019.0810,W,1,10,1.761,214.682,M,0,M,0,*5D ,";
		// The sample GPS data to be parsed , gpsData has 3gps strings in it one followed by the other

		seperatedData = strtok( gpsData , dataSeperator );	// the string which is parsed from the sample data

		while ( seperatedData != NULL )						// The case when a new data is parsed
		{

			switch ( dataExtractedPosition )   				// Switch basesd on the 14 possible data types that can be extracted from the string
			{

			case 0:													// The gpgga data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{

					dataExtractedPosition++;						// Next data will be parsed at position 1 of the string, so dataExtractedPosition
																	// made to 1
					gpggaDataParsed( );								// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 1:													// The utcTime data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 2 of the string, so dataExtractedPosition
																	// made to 2
					utcTimeDataParsed( );							// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed
				}

			case 2:													// The latitudeValue data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 3 of the string, so dataExtractedPosition
																	// made to 3

					latitudeValueDataParsed( );						// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed
				}

			case 3:													// The latitudePosition data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{

					latitudePositiondDataParsed( );					// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					dataExtractedPosition++;						// Next data will be parsed at position 4 of the string, so dataExtractedPosition
																	// made to 4

					break;											// No other cases needs to be executed until a new data is parsed
				}

			case 4:													// The longitudeValue data is parsed


				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{

					dataExtractedPosition++;						// Next data will be parsed at position 5 of the string, so dataExtractedPosition
																	// made to 5

					longitudeValueDataParsed( );					// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed
				}

			case 5:													// The longitudePosition data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 6 of the string, so dataExtractedPosition
																	// made to 6
					longitudePositionDataParsed( );					// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed
				}

			case 6:													// The position fix data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 7 of the string, so dataExtractedPosition
																	// made to 7

					positionfixDataParsed( );						// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 7:													// The satellitesUsed data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 8 of the string, so dataExtractedPosition
																	// made to 8

					satellitesUsedDataParsed( );					// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 8:													// The HDOP data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 9 of the string, so dataExtractedPosition
																	// made to 9
					HDOPDataParsed( );								// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them
					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 9:													// The altitudeValue data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 10 of the string, so dataExtractedPosition
																	// made to 10
					altitudeValueDataParsed( );						// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed

				}
			case 10:												// The altitudeUnit data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 11 of the string, so dataExtractedPosition
																	// made to 11
					altitudeunitsDataParsed( );						// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 11:												// The geiod sepration value data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 12 of the string, so dataExtractedPosition
																	// made to 12

					geoidSeperationValueDataParsed( );				// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed


				}

			case 12:												// The geiod separation unit data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 13 of the string, so dataExtractedPosition
																	// made to 13

					geoidSeperationUnitDataParsed( );				// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 13:												// The DGPSAge data is parsed

				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition++;						// Next data will be parsed at position 14 of the string, so dataExtractedPosition
																	// made to 14

					DGPSAgeDataParsed( );							// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed

				}

			case 14:												// The checksum data is parsed


				if ( HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_0 ) == buttonPressed )	// Push button is pressed

				{
					dataExtractedPosition = 0;						// Next data will be parsed at position 0 of the string, so dataExtractedPosition
																	// made to 0 , ie from the new gps string

					checksumDataParsed( );							// This function displays the data on the lcd and convert the data string into an
																	// integer and stores them

					break;											// No other cases needs to be executed until a new data is parsed

				}

			default:												// This case needs to be executed at exceptional conditions when none of the
																	// above conditions holds true


				HAL_Delay( delayLong );								// Wait for 0.5 second

				break;

			}


		}

	}
}

/*
 * FUNCTION		: gpggaDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the gpgga data is parsed, followed by a push button press.It checks if the data
				  parsed is $GPGGA , if it is not an data error message is displayed.
				  It also displays the parsed data on the lcd screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */


void gpggaDataParsed( void )
 {

	if ( strcmp( *( &seperatedData ) , "$GPGGA" ) != 0 )		// Parsed data not equals "$GPGGA"
	{
		HD44780_ClrScr( );										// Clear the lcd screen
		HD44780_GotoXY( 0 , 0 );								// Set the character cursor to col=0, row=0
		HD44780_PutStr( "Error in data" );				        // Display the text
		HD44780_GotoXY( 0 , 1 );								// Set the character cursor to col=0, row=1
		HD44780_PutStr( *( &seperatedData ) );					// Display the parsed data
		HAL_Delay( delayLong );									// Wait for 1 sec
		dataExtractedPosition = 0;
		seperatedData = strtok( NULL , dataSeperator );			// Separated data is made null to allow next data parsing

	}

	else														// The data is authentic as it starts with "$GPGGA"
	{


		HD44780_ClrScr( );										// Clear the lcd screen
		HD44780_GotoXY( 0 , 0 );								// set the character cursor to col=0, row=0
		HD44780_PutStr( "GPS data" ); 							// Display the text
		HD44780_GotoXY( 0 , 1 );								// set the character cursor to col=0, row=1
		HD44780_PutStr( *( &seperatedData ) );					// Display the parsed data
		HAL_Delay( delayNormal );								// Wait for 100 milli sec
		seperatedData = strtok( NULL , dataSeperator );			// Separated data is made null to allow next data parsing

	}
}


/*
 * FUNCTION		: utcTimeDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the UtcTime data is parsed, followed by a push button press.It stores the time data
				  parsed in a string , and separates hh  , mm , ss.s and converts them into an integer and display the value on the LCD
				  screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void utcTimeDataParsed( void )
{

	/* private variables  ------------------------------------------------------------------*/

	int timeIntegerNonFrational = 0;			// Used to store the non fractional part of time ie hhmmss of hhmmss.ss
	int timeIntegerFrational = 0;				// Used to store the fractional part of time ie the .ss of hhmmss.ss
	int hourInteger = 0;						// Used to store the hour time in  integer value ie the hh of hhmmss.ss
	int minuteInteger = 0;						// Used to store the minute time in  integer value ie the mm of hhmmss.ss
	int secondsNonfractionalInteger = 0;		// Used to store the non fractional seconds in  integer value  ie ss of hhmmss.ss
	int secondsFractionalInteger = 0;			// Used to store the non fractional seconds  in  integer value ie .ss of hhmmss.ss

	char utcTime[ longLength ] = { 0 };				// Used to store the parsed data
	char hoursTime[ normalLength ] = " ";			// To store the hour time only from the parsed data
	char minutesTime[ normalLength ] = " ";			// To store the minute time only from the parsed data
	char secondsTime[ normalLength ] = " ";			// To store the seconds time only from the parsed data

	strcpy( utcTime , *( &seperatedData ) );    // The parsed string is stored in an array

	hoursTime[ 0 ] = utcTime[ 0 ];				// Hour time ie hh of hhmmss.ss is stored separately
	hoursTime[ 1 ] = utcTime[ 1 ];
	minutesTime[ 0 ] = utcTime[ 2 ];			// Minute time ie mm of hhmmss.ss is stored separately
	minutesTime[ 1 ] = utcTime[ 3 ];
	secondsTime[ 0 ] = utcTime[ 4 ];			// Seconds  time ie ss.ss of hhmmss.ss is stored separately
	secondsTime[ 1 ] = utcTime[ 5 ];
	secondsTime[ 2 ] = utcTime[ 6 ];
	secondsTime[ 3 ] = utcTime[ 7 ];
	secondsTime[ 4 ] = utcTime[ 8 ];
	secondsTime[ 5 ] = utcTime[ 9 ];
	secondsTime[ 6 ] = utcTime[ 10 ];

	timeIntegerNonFrational = atoi( utcTime );						// Non fractional time ie hhmmss of hhmmss.ss is made an integer
	timeIntegerFrational = atoi( &utcTime[ 7 ] );					// Fractional time ie .ss of hhmmss.ss is made an integer



	HD44780_ClrScr( );												// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );										// Set the character cursor to col=0, row=0
	HD44780_PutStr( "Time " );										// Display the text
	HD44780_GotoXY( 0 , 1 );										// Set the character cursor to col=0, row=1
	hourInteger = atoi( &hoursTime[ 0 ] );							// Time in hour is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , hourInteger );					// Time in hour is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );								// Display the time in hour
	HD44780_GotoXY( 1 , 1 );										// Set the character cursor to col=1, row=1
	HD44780_PutStr( "h" );											// Display "h"
	HD44780_GotoXY( 3 , 1 );										// Set the character cursor to col=3, row=1
	minuteInteger = atoi( &minutesTime[ 0 ] );						// Time in minute is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , minuteInteger );				// Time in minute is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );								// Display the time in minute
	HD44780_GotoXY( 5 , 1 );										// Set the character cursor to col=5, row=1
	HD44780_PutStr( "m" );											// Display "m"
	HD44780_GotoXY( 7 , 1 );										// Set the character cursor to col=7, row=1
	secondsNonfractionalInteger = atoi( &secondsTime[ 0 ] );		// Time in seconds non fractional ie ss of hhmmss.ss  is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , secondsNonfractionalInteger );	// Time in seconds non fractional ie ss of hhmmss.ss is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );								// Display the seconds non fractional time ie ss of hhmmss.ss
	HD44780_GotoXY( 9 , 1 );										// Set the character cursor to col=9, row=1
	HD44780_PutStr( "." );											// Display the text "."
	HD44780_GotoXY( 10 , 1 );										// Set the character cursor to col=10, row=1
	secondsFractionalInteger = atoi( &secondsTime[ 3 ] );			// Time in seconds fractional ie .ss of hhmmss.ss is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , secondsFractionalInteger );	// Time in seconds fractional ie .ss of hhmmss.ss is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );								// Display the seconds fractional time
	HD44780_GotoXY( 12 , 1 );										// Set the character cursor to col=12, row=1
	HD44780_PutStr( "s" );											// Display the text "s"
	HAL_Delay( delayNormal );										// Wait for 100 milli sec

	seperatedData = strtok( NULL , dataSeperator );					// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: latitudeValueDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the latitudeValue data is parsed, followed by a push button press.It stores the latitudeValue data
				  parsed in a string , and separates dd  , mm , mmmm converts them into an integer and display the vlaue on the LCD
				  screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */


void latitudeValueDataParsed( void )
 {
	 /* private variables  ------------------------------------------------------------------*/

	int latitudeDegreeInteger = 0;						// Used to store the degree in  integer value ie the dd of ddmm.mmmm
	int latitudeMinuteInteger = 0;						// Used to store the minute in  integer value ie the mm of ddmm.mmmm
	int latitudeDeciDegreeInteger = 0;					// Used to store the decimal degree in  integer value ie the .mmmm of ddmm.mmmm
	uint32_t latitudeIntegerNonFractional = 0;			// Used to store the non fractional latitude value in  integer   ie ddmm of ddmm.mmmm
	uint32_t latitudeIntegerFractional = 0;				// Used to store the fractional latitude value in  integer   ie .mmmm of ddmm.mmmm
	char latitudePosition[ longLength ] = "";			// To store the  parsed data
	char latitudeDecimalDegrees[ normalLength ] = "";	// To store the decimal degree only from the parsed data
	char latitudeDegrees[ normalLength ] = " ";			// To store the degree only from the parsed data
	char latitudeMinutes[ normalLength ] = " ";			// To store the minute only from the parsed data



	strcpy( latitudePosition , *( &seperatedData ) );					// The parsed string is stored in an array

	latitudeDegrees[ 0 ] = latitudePosition[ 0 ];						// degree ie dd of ddmm.mmmm is stored separately
	latitudeDegrees[ 1 ] = latitudePosition[ 1 ];
	latitudeMinutes[ 0 ] = latitudePosition[ 2 ];						// minute ie mm of ddmm.mmmm is stored separately
	latitudeMinutes[ 1 ] = latitudePosition[ 3 ];

	latitudeDecimalDegrees[ 0 ] = latitudePosition[ 5 ];				// decimal degree ie .mmmm of ddmm.mmmm is stored separately
	latitudeDecimalDegrees[ 1 ] = latitudePosition[ 6 ];
	latitudeDecimalDegrees[ 2 ] = latitudePosition[ 7 ];
	latitudeDecimalDegrees[ 3 ] = latitudePosition[ 8 ];

	latitudeIntegerNonFractional = atoi( latitudePosition );			// Non fractional latitude ie ddmm of ddmm.mmmm is made an integer
	latitudeIntegerFractional = atoi( &latitudeDecimalDegrees[ 0 ] );	// Fractional latitude ie .mmmm of ddmm.mmmm is made an integer

	HD44780_ClrScr( );													// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );											// set the character cursor to col=0, row=0
	HD44780_PutStr( "Latitude " );										// Display the text "Latitude"
	HD44780_GotoXY( 0 , 1 );											// Set the character cursor to col=0, row=1
	latitudeDegreeInteger = atoi( &latitudeDegrees[ 0 ] );				// Latitude degree is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , latitudeDegreeInteger );			// Latitude degree in integer is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );									// Display the value of Latitude degree
	HD44780_GotoXY( 3 , 1 );											// Set the character cursor to col=3, row=1
	latitudeMinuteInteger = atoi( &latitudeMinutes[ 0 ] );				// Latitude minute is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , latitudeMinuteInteger );			// Latitude minute in integer is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );									// Display the value of Latitude minute
	HD44780_GotoXY( 6 , 1 );											// Set the character cursor to col=6, row=1
	HD44780_PutStr( "." );												// Display the text "."
	HD44780_GotoXY( 8 , 1 );											// Set the character cursor to col=8, row=1
	latitudeDeciDegreeInteger = atoi( &latitudeDecimalDegrees[ 0 ] );	// Latitude decimal degree is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , latitudeDeciDegreeInteger );		// Latitude decimal degree in integer is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );									// Display the value of Latitude dec degree
	HAL_Delay( delayNormal );											// Wait for 100 milli sec


	seperatedData = strtok( NULL , dataSeperator );						// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: latitudePositiondDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the latitudePosition data is parsed, followed by a push button press.It stores the latitude positiondata
				  parsed in a string ,  and display the value on the LCD  screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void latitudePositiondDataParsed( void )
 {

	HD44780_GotoXY( 13 , 1 );									// Set the character cursor to col=13, row=1
	HD44780_PutStr( *( &seperatedData ) );						// Display the parsed data
	HAL_Delay( delayLong );										// Wait for 0.5 sec
	seperatedData = strtok( NULL , dataSeperator );				// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: longitudeValueDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the longitudeValue data is parsed, followed by a push button press.It stores the time data
				  parsed in a string , and separates ddd  , mm , .mmmm and converts them into an integer and display the value on the LCD
				  screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void longitudeValueDataParsed( void )
{

/* private variables  ------------------------------------------------------------------*/


	uint32_t longitudeIntegerNonFractional = 0;			// Used to store the non fractional longitude value in  integer   ie dddmm of dddmm.mmmm
	uint32_t longitudeIntegerFractional = 0;			// Used to store the fractional longitude value in  integer   ie .mmmm of dddmm.mmmm
	int longitudeDegreeInteger = 0;						// Used to store the degree in  integer value ie the ddd of dddmm.mmmm
	int longitudeMinuteInteger = 0;						// Used to store the minute in  integer value ie the mm of dddmm.mmmm
	int longitudeDeciDegreeInteger = 0;					// Used to store the decimal degree in  integer value ie the .mmmm of dddmm.mmmm
	char longitudePosition[ longLength ] = "";			// To store the  parsed data
	char longitudeDegrees[ normalLength ] = " ";		// To store the  degree only from the parsed data
	char longitudeMinutes[ normalLength ] = " ";		// To store the minute only from the parsed data
	char longitudeDecimalDegrees[ normalLength ] = "";	// To store the decimal degree only from the parsed data


	strcpy( longitudePosition , *( &seperatedData ) );					// The parsed string is stored in an array

	longitudeDegrees[ 0 ] = longitudePosition[ 0 ];						// Degree ie ddd of ddmm.mmmm is stored separately
	longitudeDegrees[ 1 ] = longitudePosition[ 1 ];
	longitudeDegrees[ 2 ] = longitudePosition[ 2 ];
	longitudeMinutes[ 0 ] = longitudePosition[ 3 ];						// Minute ie mm of ddmm.mmmm is stored separately
	longitudeMinutes[ 1 ] = longitudePosition[ 4 ];

	longitudeDecimalDegrees[ 0 ] = longitudePosition[ 6 ];				// Decimal degree  ie .mmmm of ddmm.mmmm is stored separately
	longitudeDecimalDegrees[ 1 ] = longitudePosition[ 7 ];
	longitudeDecimalDegrees[ 2 ] = longitudePosition[ 8 ];
	longitudeDecimalDegrees[ 3 ] = longitudePosition[ 9 ];

	longitudeIntegerFractional = atoi( &longitudeDecimalDegrees[ 0 ] );	// Longitude fractional ie .mmmm of dddmm.mmmm is converted to an int and stored
	longitudeIntegerNonFractional = atoi( longitudePosition );			// Longitude non fractional ie dddmm of dddmm.mmmm is converted to an int and stored

	HD44780_ClrScr( );													// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );											// set the character cursor to col=0, row=0
	HD44780_PutStr( "Longitude " );										// Display the text "Longitude"
	longitudeDegreeInteger = atoi( &longitudeDegrees[ 0 ] );			// longitude degree is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , longitudeDegreeInteger );			// longitude degree in int is converted to a string and stored
	HD44780_GotoXY( 0 , 1 );											// set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );									// Display the value of longitud edegree
	longitudeMinuteInteger = atoi( &longitudeMinutes[ 0 ] );			// Longitude minute  is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , longitudeMinuteInteger );			// Longitude minute in int is converted to a string and stored
	HD44780_GotoXY( 3 , 1 );											// Set the character cursor to col=3, row=1
	HD44780_PutStr( lcdDisplayText );									// Display the value of Longitude minute
	HD44780_GotoXY( 6 , 1 );											// Set the character cursor to col=6, row=1
	HD44780_PutStr( "." );												// Display the text "."
	HD44780_GotoXY( 8 , 1 );											// Set the character cursor to col=8, row=1
	longitudeDeciDegreeInteger = atoi( &longitudeDecimalDegrees[ 0 ] );	// Longitude decimal degree  is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , longitudeDeciDegreeInteger );		// Longitude decimal degree in int is converted to a string and stored
	HD44780_PutStr( lcdDisplayText );									// Display the value of Longitude decimal degree
	HAL_Delay( delayNormal );											// Wait for 100 milli sec

	seperatedData = strtok( NULL , dataSeperator );						// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: longitudePositionDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the longitudePosition data is parsed, followed by a push button press.It stores the longitudePosition
				  data parsed in a string ,and  display it  on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void longitudePositionDataParsed( void )
{

	HD44780_GotoXY( 14 , 1 );							// Set the character cursor to col=14, row=1
	HD44780_PutStr( *( &seperatedData ) );				// Display the parsed data
	HAL_Delay( delayLong );								// Wait for 0.5 sec
	seperatedData = strtok( NULL , dataSeperator );		// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: positionfixDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the position fix data is parsed, followed by a push button press.It stores the position fix data
				  parsed in a string ,  and convert it into an integer and store it ,and display the value on the LCD screen using the lcd
				  library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void positionfixDataParsed( void )
 {

/* private variables  ------------------------------------------------------------------*/

	uint32_t positionfixInteger = 0;        				// Used to store position fix value in integer
	char positionfixString[ normalLength ] = " ";			// Used to store the parsed data
	strcpy( positionfixString , *( &seperatedData ) );		// The parsed string is stored in an array
	positionfixInteger = atoi( positionfixString );			// position fix is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , positionfixInteger );	// position fix in int is converted to a string and stored

	HD44780_ClrScr( );										// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );								// set the character cursor to col=0, row=0
	HD44780_PutStr( "Postion fix " );						// Display the text "position fix"
	HD44780_GotoXY( 0 , 1 );								// Set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );						// Display the value of position fix
	HAL_Delay( delayNormal );								// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );			// seperated data is made null to allow next data parsing

}

/*
 * FUNCTION		: satellitesUsedDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the satellitesUsed data is parsed, followed by a push button press.It stores the satellitesUsed data
				  parsed in a string ,  and converts it into an integer and display the value on the LCDn screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void satellitesUsedDataParsed( void )
{

/* private variables  ------------------------------------------------------------------*/

	char satellitesusedString[ normalLength ] = "";					// Used to store the parsed data
	uint32_t satellitesusedInteger = 0;								// Used to store satellites used value in integer


	strcpy( satellitesusedString , *( &seperatedData ) );			// The parsed string is stored in an array
	satellitesusedInteger = atoi( satellitesusedString );			// satellites used  is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , satellitesusedInteger );		// satellites used in int is converted to a string and stored

	HD44780_ClrScr( );												// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );										// set the character cursor to col=0, row=0
	HD44780_PutStr( "Satellites used" );							// Display the text "satellites used"
	HD44780_GotoXY( 0 , 1 );										// Set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );								// Display the value of satellitesUsed
	HAL_Delay( delayNormal );										// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );					// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: HDOPDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the HDOP data is parsed, followed by a push button press.It stores the HDOP data
				  parsed in a string , and separates the fractional and non fractional part of the data and convert them into integers and
				  display the value on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void HDOPDataParsed( void )
{

/* private variables  ------------------------------------------------------------------*/

	uint32_t HDOPIntegerNonfractional = 0;					// Used to store the non fractional integer value of the parsed data
	uint32_t HDOPIntegerfractional = 0;	  					// Used to store the fractional integer value of the parsed data
	char HDOPString[ normalLength ] = "";					// Used to store the parsed data

	strcpy( HDOPString , *( &seperatedData ) );				// The parsed string is stored in an array
	HDOPIntegerNonfractional = atoi(HDOPString);			// HDOP is converted to an integer and stored
	HDOPIntegerfractional =atoi(&HDOPString[2]);			// The fractional vlaue of HDOP is converted to an integer and stored
	sprintf(lcdDisplayText, "%d",HDOPIntegerNonfractional);	// HDOP non fractional in int is converted to a string and stored
	sprintf(lcdDisplayText, "%d",HDOPIntegerfractional);	// HDOP  fractional in int is converted to a string and stored
	HD44780_ClrScr( );										// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );								// set the character cursor to col=0, row=0
	HD44780_PutStr( "HDOP " );								// Display the text "HDOP "
	HD44780_GotoXY( 0 , 1 );								// Set the character cursor to col=0, row=1
	HD44780_PutStr( HDOPString );							// Display the value of HDOP
	HAL_Delay( delayNormal );								// Wait for 100 milli sec

	seperatedData = strtok( NULL , dataSeperator );	// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: altitudeValueDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the altitudeValue data is parsed, followed by a push button press.It stores the altitude data
				  parsed in a string , and  converts it into an integer and display the value on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void altitudeValueDataParsed( void )
 {

/* private variables  ------------------------------------------------------------------*/

	char altitudeString[ longLength ];						// Used to store the parsed data
	float altitudeFloat = 0.0;								// Used to store the altitude value as a float

	HD44780_ClrScr( );										// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );								// set the character cursor to col=0, row=0
	HD44780_PutStr( "Altitude " );							// Display the text "Altitude"
	HD44780_GotoXY( 0 , 1 );								// Set the character cursor to col=0, row=1

	strcpy( altitudeString, *( &seperatedData ) );			// The parsed string is stored in an array
	altitudeFloat = atof( altitudeString );					// Altitude is converted to a float value and stored
	sprintf( lcdDisplayText , "%3.3f" , altitudeFloat );	// Altitude in float is converted to a string and stored
	HD44780_PutStr( altitudeString );						// Display the value of altitude
	HAL_Delay( delayNormal );								// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );			// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: altitudeunitsDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the altitude units data is parsed, followed by a push button press.It display the altitude
				  unit on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void altitudeunitsDataParsed( void )
{

	HD44780_GotoXY( 8 , 1 );							// Set the character cursor to col=8, row=1
	HD44780_PutStr( *( &seperatedData ) );				// Display the parsed data
	HAL_Delay( delayLong );								// Wait for 500 milli sec
	seperatedData = strtok( NULL , dataSeperator );		// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: geoidSeperationValueDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the geoidSeperationValue data is parsed, followed by a push button press.It stores the geoidSeperationValue data
				  parsed in a string ,  and converts them into an integer and display the value on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void geoidSeperationValueDataParsed( void )
{

/* private variables  ------------------------------------------------------------------*/
	int geoidseperationNonFractional = 0;                 		// Used to store the geoid separation non fractional data as int
	int geoidseperationFractional = 0;							// Used to store the geoid separation  fractional data as int
	char geoidSeperationString[ normalLength ] = "";			// Used to store the parsed data
	char geoidseperationNonFractionalarr[ normalLength ] = "";	// Used to store the geoidseperationNonFractional data as a string
	char geoidseperationFractionalarr[ normalLength] = "";		// Used to store the geoidseperationFractional data as a string

	strcpy( geoidSeperationString , *( &seperatedData ) );				  			// The parsed string is stored in an array

	geoidseperationNonFractionalarr[ 0 ] = geoidSeperationString[ 0 ];				// geoidseperationNonFractional is stored separately
	geoidseperationNonFractionalarr[ 1 ] = geoidSeperationString[ 1 ];
	geoidseperationFractionalarr[ 0 ] = geoidSeperationString[ 3 ];					// geoidseperationFractional is stored separately

	geoidseperationNonFractional = atoi( &geoidseperationNonFractionalarr[ 0 ] );	// geoid separation NonFractional  is converted to an int and stored
	geoidseperationFractional = atoi(&geoidseperationFractionalarr[0]);				// geoid separation Fractional  is converted to an int and stored
	sprintf( lcdDisplayText , "%d" , geoidseperationNonFractional );				// geoid separation Fractional is converted to a string and stored
	HD44780_ClrScr( );																// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );														// set the character cursor to col=0, row=0
	HD44780_PutStr( "Geoid seperation" );											// Display the text "Geoid separation"
	HD44780_GotoXY( 0 , 1 );														// Set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );												// Display the value of geoidseperationNonFractional
	HD44780_GotoXY( 3 , 1 );														// Set the character cursor to col=3, row=1
	HD44780_PutStr( "." );															// Display the text "."
	sprintf( lcdDisplayText , "%d" , geoidseperationFractional );					// geoid separation Fractional is converted to a string and stored
	HD44780_GotoXY( 5 , 1 );														// Set the character cursor to col=5, row=1
	HD44780_PutStr( lcdDisplayText );												// Display the value of geoid separation Fractional
	HAL_Delay( delayNormal );																// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );									// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: geoidSeperationUnitDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the geoidSeperationUnit  data is parsed, followed by a push button press.It  display the data parsed
				  on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void geoidSeperationUnitDataParsed( void )

{

	HD44780_GotoXY( 7 , 1 );								// Set the character cursor to col=7, row=1
	HD44780_PutStr( *( &seperatedData ) );					// Display the parsed data
	HAL_Delay( delayNormal );										// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );			// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: DGPSAgeDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the DGPSAge data is parsed, followed by a push button press.It stores the DGPSAge data
				  parsed in a string , and converts it into an integer and display the value on the LCD screen using the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void DGPSAgeDataParsed( void )
{
	/* private variables  ------------------------------------------------------------------*/

	uint32_t DGPSAgeInteger = 0;							// Used to store DGPSAge value in integer
	char DGPSAgeString[ normalLength ] = "";				// Used to store the parsed data


	strcpy( DGPSAgeString , *( &seperatedData ) );			// The parsed string is stored in an array
	DGPSAgeInteger = atoi( DGPSAgeString );					// DGPSAge  is converted to an integer and stored
	sprintf( lcdDisplayText , "%d" , DGPSAgeInteger );		// DGPSAge in int is converted to a string and stored
	HD44780_ClrScr( );										// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );								// set the character cursor to col=0, row=0
	HD44780_PutStr( "DPGS age" );							// Display the text "DPGS age"
	HD44780_GotoXY( 0 , 1 );								// Set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );						// Display the value of DGPSAge
	HAL_Delay( delayNormal );										// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );			// Separated data is made null to allow next data parsing

}

/*
 * FUNCTION		: checksumDataParsed
 *
 * DESCRIPTION 	: This function gets executed when the checksum data is parsed, followed by a push button press.It stores the checksum data
				  parsed in a string , and separates  "*" from it and converts it into a byte (using the strtol() )and display the vlaue on
				  the LCD  screen using  the lcd library HD44780_F3.c
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */

void checksumDataParsed( void )
 {

/* private variables  ------------------------------------------------------------------*/


	char checkSumString[ normalLength ] = "";										// Used to store the parsed data

	strcpy( checkSumString, *( &seperatedData ) );									// The parsed string is stored in an array
	uint8_t checksumByte = ( uint8_t ) strtol( &checkSumString[ 1 ], NULL , 16 );	// The checksum data without * is converted to a byte
	sprintf( lcdDisplayText , "%d", checksumByte );									// checksum byte  is converted to a string and stored
	HD44780_ClrScr( );																// Clear the lcd screen
	HD44780_GotoXY( 0 , 0 );														// set the character cursor to col=0, row=0
	HD44780_PutStr( "Checksum" );													// Display the text "checksum"
	HD44780_GotoXY( 0 , 1 );														// Set the character cursor to col=0, row=1
	HD44780_PutStr( lcdDisplayText );												// Display the  value od checksumByte
	HAL_Delay( delayNormal );														// Wait for 100 milli sec
	seperatedData = strtok( NULL , dataSeperator );									// Separated data is made null to allow next data parsing

}


/*
 * FUNCTION		: portaInit
 *
 * DESCRIPTION 	: This function configure and initializes port_a as an input port with clock enabled and GPIO_PIN_0 in pull down mode
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */
void portaInit( void )
{
	__GPIOA_CLK_ENABLE( );                			 	 	// clock is turned on for the a port
	GPIO_InitStruct.Pin = GPIO_PIN_0; 						//PIN_0 of port A is selected   using the standard structure
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 				// push button in input mode,so can accept signals
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;          			//in pull down configuration
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; 				//supports high speed operation
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 72000000
 *            HCLK(Hz)                       = 72000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 2
 *            APB2 Prescaler                 = 1
 *            HSE Frequency(Hz)              = 8000000
 *            HSE PREDIV                     = 1
 *            PLLMUL                         = RCC_PLL_MUL9 (9)
 *            Flash Latency(WS)              = 2
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	/* User may add here some code to deal with this error */
	while (1) {
	}
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

