/*
  *FILE   		  :  prog8125-statemachine
  *PROJECT		  :  PROG8125 - Assignment # 4
  *PROGRAMMER     :  Nimal krishna
  *FIRST VERSION  :  00/11/2016
  *DESCRIPTION    :  This project is  for creating a RouletteWheel, in which LEDs rotates on normal speed,but when the pushbutton is pressed
  *					 a band of LEDs rotates at a higher speed .When the pushbutton is released the LEDs begin to slow down eventually coming
  *					 to halt after blinking the last led for five times.Then after 5 seconds the LEDs again begin to rotate in the normal mode.
  *
  *					 When the wheel is in the normal mode, the LCD displays normal mode , and when the LEDs begin to rotate at high speed the
  *					 LCD shows the operation mode as high speed, and it display slowing down when the LEDs begin to slow down after the push
  *					 button release.Lcd displays blinking when the last led blinks and it displays waiting foe 5 second when the wheel enters
  *					 the waiting mode.
  *
  *					 This project also uses a DAC to illuminate a three terminal led based on the operation mode of the Roulette wheel.
  *					 so when the LEDs are in the normal mode a white light is displayed,  but when the push button is pressed the light
  *					 gradually changes to red, and when the pushbutton is released the light gradually changes back to  white again, and
  *					 blinks 5 times when the last led of the wheel starts to blink, after that it enters a 5second off state before turning
  *					 white again when the wheel is in the normal mode.
  *
  *
 */
/* Includes ------------------------------------------------------------------*/

#include "stm32f3xx_hal.h"
#include "usb_device.h"

/* Private variables ---------------------------------------------------------*/

#define WAITING_FOR_PUSHBUTTON_PRESS_STATE		0    // If stateMachine = WAITING_FOR_PUSHBUTTON_PRESS_STATE ,implies push button not pressed
#define WAITING_FOR_PUSHBUTTON_RELEASE_STATE	1	 // If stateMachine = WAITING_FOR_PUSHBUTTON_RELEASE_STATE ,implies push button  pressed


// led flash pattern control constants
#define INITIAL_LED_DELAY						200		// LED ledturnOn duration when the push button is not pressed
#define BUTTON_PRESSED_LED_DELAY				50		// LED ledturnOn duration when the push button is  pressed
#define SLOW_DOWN_CONSTANT						650		// Time by which the ledturnOn time increases when pushbutton is released
#define WAIT_FOR_NEXT_ROUND_DELAY				5000	// Waiting time before the RouletteWheel rotates normal again after it stops
#define DONE_BLINK_RATE_DELAY					500		// LED on time when it blinks just before coming to stop
#define NUMBER_OF_FLASHES_WHEN_DONE				4		// Number of blinks by the last led before it enters the waiting period to start next normal cycle.

// delayAfterTurnOn values
#define DO_NOT_PUT_A_DELAY_AFTER_TURN_ON		0		// Used to indicate the way the LEDs should flash when the pushbutton is not pressed.
#define PUT_A_DELAY_AFTER_TURN_ON				1		// Used to indicate the way the LEDs should flash when the pushbutton is pressed.

// checkPushButtonDebounced return values
#define BUTTON_PRESSED							1		// Used as ,when  checkPushButtonDebounced() returns 1 it means push button is pressed
#define BUTTON_NOT_PRESSED						0		// Used as ,when  checkPushButtonDebounced() returns 0 it means push button is not  pressed

// debounce delay time in ms
#define DEBOUNCE_DELAY							2		// DeBounce delay time in ms, used to correct bouncing using the delay method

// an array holding the pattern of LEDs
#define LED_PATTERN_SIZE	8							// Total number of LEDs which blinks rotationally

static uint16_t ledPattern[ ] = { LD4_Pin , LD3_Pin , LD5_Pin , LD7_Pin , LD9_Pin , LD10_Pin , LD8_Pin , LD6_Pin };
														// an array holding the pattern of LEDs
uint16_t LcdSetvarNormalspped = 0;				// Used to display  text on the LCD only once when the wheel is in the normal mode
uint16_t LcdSetvarHighspped = 0;				// Used to display  text on the LCD only once when the wheel is in the high speed
uint16_t LcdSetvarSlowDown = 0;					// Used to display  text on the LCD only once when the wheel is slowing down
uint16_t LcdSetvarIdle = 0;						// Used to display  text on the LCD only once when the wheel is in the idle mode
uint16_t LcdSetvarBlink = 0;					// Used to display  text on the LCD only once when the wheel is in the blinking mode
uint16_t delayBlink = 100;						// LED turn on time for the next LED after the push button is released.
uint16_t ledStopPoition = 0;					// The position of the last LED which was turned on just before the  push button was released


ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim15;

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config( void );
void Error_Handler( void );
static void MX_GPIO_Init( void );
static void MX_ADC1_Init( void );
static void MX_TIM2_Init( void );
static void MX_TIM8_Init( void );
static void MX_TIM15_Init( void );
void HAL_TIM_MspPostInit( TIM_HandleTypeDef *htim );
static GPIO_InitTypeDef GPIO_InitStruct;
void turnOnLed( uint16_t ledNumber , int delayAfterTurnOnFlag );
int checkPushButtonDebounced( void );
void HD44780_Init( void );
void HD44780_PutChar( unsigned char c );
void HD44780_GotoXY( unsigned char x , unsigned char y );
void HD44780_PutStr( char *str );
void HD44780_ClrScr( void );
void RGBColorMixer( uint8_t redIntensity , uint8_t greenIntensity ,uint8_t blueIntensity );

/*
 * FUNCTION		: turnOnLed
 *
 * DESCRIPTION 	: This function will turn on a given LED using the passed GPIO pin number and turn off the other LEDs , with delays
				  depending on the status of pushbutton.
				  when the pushbutton is  not pressed , when one LED is turned on , all other LEDs are turned off , and there is rotation of LEDs
				  based on only one turned on at a time
				  when the pushbutton is   pressed , a number of LEDs are turned on at a time, and  all other LEDs are turned off , and there is
				  rotation of a band of LEDs. and when the push button is released the LEDs speed of rotation slows down and finally comes to a halt
				  after blinking the last led for 4 times.Then it waits for another 5 seconds, after which the normal rotation of the LEDs begin again.

				  The LCD and the three terminal led also indicates the status of the RouletteWheel
 *
 * PARAMETERS	:
					ledNumber   		 : LED pin to turn on
    				delayAfterTurnOnFlag : if set put a delay after the led is turn on in
 *
 * RETURNS		: void

 */

void turnOnLed( uint16_t ledNumber , int delayAfterTurnOnFlag )
{

	if ( delayAfterTurnOnFlag == DO_NOT_PUT_A_DELAY_AFTER_TURN_ON )
	{
		static uint16_t lastLed = 0xffff; // Not set yet so set the value to invalid one
		RGBColorMixer( 255 , 255 , 255 ); // Display white light

		if ( lastLed != 0xffff )		  // This loop will be entered after the first LED is turned on and is used to turn off the previous LED
		{
			HAL_GPIO_WritePin( GPIOE , lastLed , GPIO_PIN_RESET );	// clear the last LED if needed
		}

		if ( LcdSetvarNormalspped == 0 )   // Used to display  Normal speed only once in the LCD when the wheel is in the normal mode
		{
			LcdSetvarNormalspped = 1;	   // The value of LcdSetvarNormalspped changed to ensure that the text Normal speed is displayed
										   // only once thus avoiding the unwanted delay which LCD functions can create
			HD44780_GotoXY( 0 , 1 );	   // Go to column = 0 , row =1
			HD44780_PutStr( "Normal speed      " );	 // Display the text

		}

			lastLed = ledNumber;			// keep track of last led that we turned on so we can turn it off next time
			HAL_GPIO_WritePin( GPIOE , lastLed , GPIO_PIN_SET );	// turn on the LED pin on GPIO



	}

	if ( delayAfterTurnOnFlag == PUT_A_DELAY_AFTER_TURN_ON )
	{

		if (LcdSetvarHighspped == 0)		// Used to display  High speed only once in the LCD when the wheel is in the high speed
		{
			RGBColorMixer( 255 , 255 , 0 );	// Display yellow light
			LcdSetvarHighspped = 1;			// The value of LcdSetvarHighspped changed to ensure that the text high speed is displayed
											// only once thus avoiding the unwanted delay which LCD functions can create
			HD44780_GotoXY( 0 , 1 );		// Go to column = 0 , row =1
			HD44780_PutStr( "High speed        " );	// Display the text

		}

		RGBColorMixer( 255 , 0 , 0 );			// Display red light

		HAL_GPIO_WritePin( GPIOE , ledNumber , GPIO_PIN_SET );	 // Turn on the ledNumber pin on GPIO
		HAL_Delay( BUTTON_PRESSED_LED_DELAY );					 // Turn on the LED for a specific time

		if ( ledNumber == ledPattern[ 0 ] )		// If the last turned on LED is the first of the LED pattern array, then turn off the previous one,
												// i.e the last one of the LED pattern array.
		{

			HAL_GPIO_WritePin( GPIOE , LD6_Pin , GPIO_PIN_RESET );	//turn off the last LED  of the LED pattern array.
			HAL_Delay( BUTTON_PRESSED_LED_DELAY );
		}

		else	// If the last turned on LED is not  first one of the LED pattern array, then turn off the previous one
		{
			HAL_GPIO_WritePin( GPIOE, ledNumber - 1 , GPIO_PIN_RESET );		//	Turn off the previous LED

			HAL_Delay( BUTTON_PRESSED_LED_DELAY );
		}

		if ( checkPushButtonDebounced( ) == BUTTON_NOT_PRESSED ) 	// If pushbutton was released after it was pressed
		{

			if ( LcdSetvarSlowDown == 0 )		// Used to display  slowing down  only once in the LCD when the wheel is slowing down
			{
				RGBColorMixer( 255 , 255 , 0);	// Display yellow light
				LcdSetvarSlowDown = 1;			// The value of LcdSetvarSlowDown changed to ensure that the text slowing down is displayed
												// only once thus avoiding the unwanted delay which LCD functions can create
				HD44780_GotoXY( 0 , 1 );		// Go to column = 0 , row =1
				HD44780_PutStr( "Slowing down      " );	// Display the text

			}

			RGBColorMixer( 255 , 255 , 255 );	// Display Red light

			for ( int i = 0 ; i < LED_PATTERN_SIZE ; i++ ) 	// Find out the last LED which was turned on just before the push button was released
			{

				if (*(&ledNumber) == ledPattern[i])

				{

					ledStopPoition = i;		// If the last turned on led position matches the ledPattern array position , then
											// store the ledPattern array position in the variable  ledStopPoition
				}

			}

			for ( int k = 0 ; k < 6 ; k++ )		 // This loop is used to turn on the next 5 LEDs in a slowing down manner
			{

				if ( ledStopPoition == 7 ) 		// If the last turned on led was the last one of the array then turn on and then turn off that
				      	  	  	  	  	  	  	// LED for the specific time and then update the ledStopPoition to the next member i.e
				      	  	  	  	  	  	  	// the first led of the ledPattern array
				{
					HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ] , GPIO_PIN_SET );
					HAL_Delay( delayBlink );	// Turn on the last LED for the specific time
					delayBlink = delayBlink + SLOW_DOWN_CONSTANT;	// Increase the turn on time for the next LED
					HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ], GPIO_PIN_RESET );	 // Turn off the last LED
					ledStopPoition = 0;			 // Update the ledStopPoition to the next member i.e the first led of the ledpattern array

				}

				else if ( ledStopPoition != 7 )	// If the last turned on led was not the  last one of the array then turn on and then turn off that
												// LED for the specific time and then update the ledStopPoition to the next member
				{

					HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ] , GPIO_PIN_SET );
					HAL_Delay( delayBlink );	// Turn on the last LED for the specific time
					HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ] , GPIO_PIN_RESET );	// Turn off the last LED
					ledStopPoition++;	// Update the ledStopPoition to the next member
					delayBlink = delayBlink + SLOW_DOWN_CONSTANT;	// Increase the turn on time for the next LED
				}

				if ( k == 5 )	// If the number of LEDs turned on after the pushbutton was released is 5 ,then blink the  next led for 4 times
								// and also blink the three  terminal led, and display the text Blinking in the LCD
				{

					for ( int m = 0 ; m <= NUMBER_OF_FLASHES_WHEN_DONE ; m++ ) 	 // Used to blink the next LED 4 times
					{

						if ( LcdSetvarBlink == 0 ) 		// Used to display  blinking  only once in the LCD when the wheel is blinking
						{
							LcdSetvarBlink = 1;			// The value of LcdSetvarBlink changed to ensure that the text blinking is displayed
														// only once thus avoiding the unwanted delay which LCD functions can create
							HD44780_GotoXY(0, 1);		// Go to column = 0 , row =1
							HD44780_PutStr("Blinking        ");		// Display the text

						}

						RGBColorMixer( 255 , 255 , 255 );		// Display White light
						HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ], GPIO_PIN_SET );
						HAL_Delay( DONE_BLINK_RATE_DELAY );		// Turn on the LED for a specific time
						HAL_GPIO_WritePin( GPIOE , ledPattern[ ledStopPoition ] , GPIO_PIN_RESET );
						RGBColorMixer( 0 , 0 , 0 );  // Turn off the three terminal led, thus create the blink effect
						HAL_Delay( 300 );	// Turn off the LED for a specific time

						if ( m == NUMBER_OF_FLASHES_WHEN_DONE )	// If the last LED is blinked for 4 times , then enter the 5 seconds waiting period
																// and display the text waiting  for 5 second in a scrolling manner in the LCD
																// and turn of the three terminal LED
						{

							if ( LcdSetvarIdle == 0 ) // Used to display waiting for 5 seconds  only once in the LCD when the wheel is in the waiting mode
							{
								LcdSetvarIdle = 1;	 // The value of LcdSetvarIdle changed to ensure that the text waiting for 5 seconds  is displayed
													 // only once thus avoiding the unwanted delay which LCD functions can create
								RGBColorMixer( 0 , 0 , 0 );		// Turn off the three terminal led
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "waiting for 5 seconds " );			// Display text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "waiting for 5 seconds " );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "aiting for 5 seconds " );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "iting for 5 seconds" );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "ting for 5 seconds   " );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "ing for 5 seconds   " );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "ng for 5 seconds   " );			// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "g for 5 seconds   " );				// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "for 5 seconds    " );				// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "or 5 seconds   " );				// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "r 5 seconds       " );				// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "5 seconds     " );					// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "seconds     " );					// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "econds     " );					// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "conds     " );						// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "onds     " );						// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "nds     " );						// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "ds     " );						// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "s     " );							// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "      " );							// Display the text
								HD44780_GotoXY( 0 , 1 );							//set the character cursor to col = 0, row = 1
								HD44780_PutStr( "                   " );			// Display the text


							}


							delayBlink = 100;			// Update the delay blink again to its original value so that LEDs slowdown correctly after the
														// next push button release
							LcdSetvarNormalspped = 0;	// To ensure that LCD displays the text only once
							LcdSetvarHighspped = 0;		// To ensure that LCD displays the text only once
							LcdSetvarIdle = 0;			// To ensure that LCD displays the text only once
							LcdSetvarSlowDown = 0;		// To ensure that LCD displays the text only once
							LcdSetvarBlink = 0;			// To ensure that LCD displays the text only once

						}
					}

				}

			}

		}

	}
}

/*
 * FUNCTION		: checkPushButtonDebounced
 *
 * DESCRIPTION 	: This function willcheck for the pushbutton press state or release state, it also have safeguard against bouncing off the push
 * 				  butotn
 *
 * PARAMETERS	: Void

 *
 * RETURNS		: integer

 */
int checkPushButtonDebounced( void )
{
	int buttonState = BUTTON_NOT_PRESSED;	// pushbutton not pressed

	// get the initial button state
	if (HAL_GPIO_ReadPin( B1_GPIO_Port , B1_Pin ) != 0 )
		buttonState = BUTTON_PRESSED;    // if the push button is pressed then update the button state to 1

	HAL_Delay( DEBOUNCE_DELAY);			// small delay to bypass bounce on push button

	// check if the button state does not agree with first sample
	// if it does not agree ignore it
	if ( buttonState == BUTTON_PRESSED )
	{
		if ( HAL_GPIO_ReadPin( B1_GPIO_Port , B1_Pin ) == 0 ) // pushbutton got released after it was pressed
			buttonState = BUTTON_NOT_PRESSED;
	}
	else 		// push button is still in the pressed state
	{
		if ( HAL_GPIO_ReadPin( B1_GPIO_Port , B1_Pin ) != 0 )
			buttonState = BUTTON_PRESSED;
	}

	return buttonState;   // buttonState = 0 means pushbutton not pressed  and the buttonState is 1 means the push button is pressed
}

/*
 * FUNCTION		: RGBColorMixer
 *
 * DESCRIPTION 	: This function allows the creation of  proper colour to the three terminal led based on RGB value passed
 *
 * PARAMETERS	: redIntensity, greenIntensity, blueIntensity

 *
 * RETURNS		: Void

 */

void RGBColorMixer( uint8_t redIntensity , uint8_t greenIntensity , uint8_t blueIntensity )

{
	__HAL_TIM_SetCompare( &htim2 , TIM_CHANNEL_4 , redIntensity );
	__HAL_TIM_SetCompare( &htim8 , TIM_CHANNEL_4 , greenIntensity );
	__HAL_TIM_SetCompare( &htim15 , TIM_CHANNEL_2 , blueIntensity );
}

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
	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init( );

	/* Configure the system clock */
	SystemClock_Config( );

	/* Initialize all configured peripherals */
	MX_GPIO_Init( );
	MX_ADC1_Init( );
	MX_TIM2_Init( );
	MX_TIM8_Init( );
	MX_TIM15_Init( );
	MX_USB_DEVICE_Init( );
	HD44780_Init( );
	HD44780_ClrScr( );
													// Display wheel opertaion mode in a scrolling manner
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "wheel operation mode:" );		// Display text
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "heel operation mode:" );		// Display text
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "eel operation mode:" );		// Display text
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "el operation mode:" );			// Display text
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "l operation mode:" );			// Display text
	HD44780_GotoXY( 0 , 0 );						// set the character cursor to col=0, row=0
	HD44780_PutStr( "operation mode: " );			// Display text

	HAL_TIM_PWM_Start( &htim2 , TIM_CHANNEL_4 );
	HAL_TIM_PWM_Start( &htim8 , TIM_CHANNEL_4 );
	HAL_TIM_PWM_Start( &htim15 , TIM_CHANNEL_2 );

	while ( 1 )
	{

		static int ledIndex = 0;						 // used to select LED to turn on

		int stateMachine = checkPushButtonDebounced( );  // push button status is stored in the stateMachine

		switch ( stateMachine )
		{
		case WAITING_FOR_PUSHBUTTON_PRESS_STATE:		 // pushbutton not pressed

			HAL_Delay( INITIAL_LED_DELAY );
			turnOnLed( ledPattern[ ledIndex++ ] , DO_NOT_PUT_A_DELAY_AFTER_TURN_ON ); // turn on the LED in normal mode
			if (ledIndex >= LED_PATTERN_SIZE)   		 // is the pattern ready to repeat
				ledIndex = 0;
			break;

		case WAITING_FOR_PUSHBUTTON_RELEASE_STATE:		// pushbutton pressed

			turnOnLed(ledPattern[ledIndex++], PUT_A_DELAY_AFTER_TURN_ON);  // turn on the LED in high speed  mode, but when the push button is released
																		   // begin to slow down and finally comes to halt after blinking
			if (ledIndex >= LED_PATTERN_SIZE)							   // is the pattern ready to repeat
				ledIndex = 0;

		}

	}


}

/*
 * FUNCTION		:SystemClock_Config
 *
 * DESCRIPTION 	: to configure the system clock
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */

void SystemClock_Config( void )
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_TIM8
			| RCC_PERIPHCLK_ADC12;
	PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
	PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_HCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/*
 * FUNCTION		: MX_ADC1_Init
 *
 * DESCRIPTION 	: ADC1 initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
static void MX_ADC1_Init(void)
{

	ADC_ChannelConfTypeDef sConfig;
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/**Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = 1;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

}

/*
 * FUNCTION		:  MX_TIM2_Init
 *
 * DESCRIPTION 	:  timer 2 initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
static void MX_TIM2_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 72;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 254;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK)
	{
		Error_Handler();
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK)
	{
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim2);

}

/*
 * FUNCTION		: MX_TIM8_Init
 *
 * DESCRIPTION 	: ADC1 initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
static void MX_TIM8_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 72;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 254;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig)
			!= HAL_OK)
	{
		Error_Handler();
	}

	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.BreakFilter = 0;
	sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
	sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
	sBreakDeadTimeConfig.Break2Filter = 0;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig)
			!= HAL_OK)
	{
		Error_Handler();
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK)
	{
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim8);

}

/*
 * FUNCTION		: MX_TIM15_Init
 *
 * DESCRIPTION 	: ADC1 initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
static void MX_TIM15_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim15.Instance = TIM15;
	htim15.Init.Prescaler = 72;
	htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim15.Init.Period = 254;
	htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim15.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&htim15) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig)
			!= HAL_OK)
	{
		Error_Handler();
	}

	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.BreakFilter = 0;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig)
			!= HAL_OK)
	{
		Error_Handler();
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK)
	{
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim15);

}

/*
 * FUNCTION		: MX_GPIO_Init
 *
 * DESCRIPTION 	: GPIO initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
static void MX_GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE()
	;
	__HAL_RCC_GPIOC_CLK_ENABLE()
	;
	__HAL_RCC_GPIOF_CLK_ENABLE()
	;
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;

	/*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
	 MEMS_INT2_Pin */
	GPIO_InitStruct.Pin = DRDY_Pin | MEMS_INT3_Pin | MEMS_INT4_Pin
			| MEMS_INT1_Pin | MEMS_INT2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
	 LD7_Pin LD9_Pin LD10_Pin LD8_Pin
	 LD6_Pin */
	GPIO_InitStruct.Pin = CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin
			| LD9_Pin | LD10_Pin | LD8_Pin | LD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PA5 PA6 SPI1_MISO_Pin */
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | SPI1_MISO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins :  DP_Pin */
	GPIO_InitStruct.Pin = DM_Pin | DP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF14_USB;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB6 PB7 */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin | LD9_Pin
					| LD10_Pin | LD8_Pin | LD6_Pin, GPIO_PIN_RESET);

}

/*
 * FUNCTION		: Error_Handler
 *
 * DESCRIPTION 	: ADC1 initialization
 *
 * PARAMETERS	: void
 *
 * RETURNS		: Void

 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	while (1)
	{
	}
	/* USER CODE END Error_Handler */
}

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */

}

#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
