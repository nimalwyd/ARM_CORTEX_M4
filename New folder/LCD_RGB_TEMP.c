/*
  *FILE   		  :  prog8125_lcd_adc_rgb
  *PROJECT		  :  PROG8125 - Assignment # 3
  *PROGRAMMER     :  Nimal krishna
  *FIRST VERSION  :  30/10/2016
  *DESCRIPTION    :  This project is  for displaying the current temperature using temperature sensor MCP7000A.
  *					 The temperature is displayed on the top right corner of the LCD with one decimal place.
  *					 The RGB led is   illuminated with specific color for given temperature ranges.
  *
  *					  less than -15 C   white
  *					 -15 C  to 5.0 C    blue
  *				      5.1 C to 15 C     yellow
  *                   15.1 C to 25 C    orange
  *                   25.1 C to 50 C    red
  *                   above 50 C        white
  *
  *                   The DAC is used for illuminating the tri color LED.
  *                   The ADC is used for reading values from the temperature sensor.
 */


/* Includes ------------------------------------------------------------------*/

#include "stm32f3xx_hal.h"
#include "usb_device.h"
#include <vcp_prog8125.h>
#include <string.h>
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/

ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim15;
static GPIO_InitTypeDef GPIO_InitStruct;
circularQueue_t VcpQueue;    									 // queue used for input from VCP
double  analogueVoltage[ 20 ] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };
										   // This array is used to store the  analog voltage  reading from the temperature sensor ,
										   // so as to find its  average after 20 such readings
double averageAnaloguevoltage = 0.0;       // Used to store the average of 20 analog readings
double tempDifference = 0.0;               // Difference of averageAnaloguevoltage from the lowest value of analog voltage for
										   // the lower limit of temperature band
double absoluteTemperature = 0.0;		   // Temperature calculated in degree_celcius from the analog reading
int   degreeSymbolValue = 0xDF;     				  // LCD code for degree ( ° ) symbol
char degreeSymbol[ sizeof ( degreeSymbolValue ) ];    // Array used to store the string corresponding  to the degree symbol



/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config( void );
void Error_Handler( void );
static void MX_GPIO_Init( void );
static void MX_ADC1_Init( void );
static void MX_TIM2_Init( void );
static void MX_TIM8_Init( void );
static void MX_TIM15_Init( void );
void HAL_TIM_MspPostInit( TIM_HandleTypeDef *htim );
void HD44780_Init( void );
void HD44780_PutChar( unsigned char c );
void HD44780_GotoXY( unsigned char x , unsigned char y );
void HD44780_PutStr( char *str );
void HD44780_ClrScr( void );
void RGBColorMixer( uint8_t redIntensity, uint8_t greenIntensity,uint8_t blueIntensity );
void initEport( void );

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

 /* Reset of all peripherals, Initializes the Flash interface and the SysTick. */
  HAL_Init( );

 /* Configure the system clock to 72 Mhz */
  SystemClock_Config( );

 /* Initialize all configured peripherals */

  initializeVcpQueue( &VcpQueue );
  MX_GPIO_Init( );
  MX_ADC1_Init( );
  MX_TIM2_Init( );
  MX_TIM8_Init( );
  MX_TIM15_Init( );
  MX_USB_DEVICE_Init( );
  initEport( );

  // start of LCD setup code
  HD44780_Init( );				   						 // Initialize the LCD
  HD44780_ClrScr( );						    		 // Clear the display
  HAL_Delay( 1000 );									 // Delay of 1000 ms
  HD44780_GotoXY( 0 , 0 );			     				 //set the character cursor to col=0, row=0
  HD44780_PutStr( "Temperature"  );						 //display text
  HD44780_GotoXY( 0 , 1 );			     				 //set the character cursor to col=0, row=0
  HD44780_PutStr( "Sensor"  );							 //display text
  HAL_Delay( 500 );										 // Delay of 500ms

  // start ADC used for Analog to Digital input on PA0
  HAL_ADC_Start( &hadc1 );

  // start PWM used for RGB LED
  HAL_TIM_PWM_Start( &htim2, TIM_CHANNEL_4 );
  HAL_TIM_PWM_Start( &htim8, TIM_CHANNEL_4 );
  HAL_TIM_PWM_Start( &htim15, TIM_CHANNEL_2 );



  sprintf( degreeSymbol , "%c" , degreeSymbolValue );  	 //  Convert the LCD code for degree symbol( 0xDF) into a string


	while ( 1 )

	{

		for ( int  i = 0 ; i < 20 ; i++ )
		{

			HAL_ADC_Start( &hadc1 );   									  // Start the ADC conversion
			if ( HAL_ADC_PollForConversion( &hadc1 , 1000000) == HAL_OK)  // Check for the completion of the ADC conversion
			{
				analogueVoltage[ i ] = HAL_ADC_GetValue(&hadc1);          // The converted value is  stored in an array

			}
			if( i == 0 )
			{
				averageAnaloguevoltage = analogueVoltage[ i ] ;  // The first read value is stored in the 0th position of the array
			}
			else
			{
				averageAnaloguevoltage = ( analogueVoltage[ i ] + averageAnaloguevoltage )/2 ;
															  // average value is updated on every new measurements

			}

			HAL_ADC_Stop( &hadc1 ); 	// stop the ADC conversion
			HAL_Delay( 100 );			// Delay of 100ms

		}


		 if ( averageAnaloguevoltage >= 415 &&   averageAnaloguevoltage <= 704 )
			          // The case in which temperature is between -15°C and 5°C
		 {
			HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , GPIO_PIN_SET );   // turn on the blue led
			HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , GPIO_PIN_RESET );
			HAL_GPIO_WritePin( GPIOE , GPIO_PIN_10 , GPIO_PIN_RESET );
			HAL_GPIO_WritePin( GPIOE , GPIO_PIN_11 , GPIO_PIN_RESET );
			RGBColorMixer( 0 , 0 , 255 );  // call to set RGB led to blue, with R,G,B value of 0,0,255
			HAL_Delay( 100 );              // Delay of 100ms
			tempDifference =  averageAnaloguevoltage - 415.0 ; // Difference with the averageAnalog value for -15°C
			absoluteTemperature =  -15.0 + ( tempDifference *  20.0/289.0 ) ;  // The exact temperature for the analog value read
			char arr[ sizeof( absoluteTemperature ) ];  // Array used to store the absolute temperature as a string

	  		  	if (absoluteTemperature > -0.05   &&  absoluteTemperature < 0.05   )  // when temperature is 0°C
				{

	  		  			HD44780_Init( );		    		// Initialize the LCD
						HD44780_ClrScr( );	    			// clear the display
						HAL_Delay( 1000 );	   				// Delay of 1s
						HD44780_GotoXY( 13, 0 );  			// set the character cursor to col=13, row=0
						HD44780_PutStr( "0" );				// Display 0 ( don't display temperature in float form like 0.0 )
						HD44780_PutStr( degreeSymbol );		// Display  ° symbol
						HD44780_PutStr( "C" );				// Display 0
						HAL_Delay( 500 );					// Delay of 1s


				}
				else      // when temperature is in the range -15°C to 5°C , but is no t zero
				{
						sprintf( arr,  "%2.1f", absoluteTemperature );   //  Convert the temperature into a string
						HD44780_Init( );								 // Initialize the LCD
						HD44780_ClrScr( );							     //clear the display
						HAL_Delay( 1000 );							     // Delay of 1s
						  		if (absoluteTemperature < 0  &&  absoluteTemperature > -10) // when the temperature has only one digit
						  		{
						  		  	HD44780_GotoXY( 10, 0 );             //set the character cursor to col=10, row=0
						  		}

						  		else if ( absoluteTemperature <=  -10 )  // the temperature has two  digits

						  		{
						  			HD44780_GotoXY( 9, 0 );              //set the character cursor to col=9, row=0

						  		}

						  		else if ( absoluteTemperature > 0 )      // when the temperature has only one digit as it is in 0 to 5°C

						  		{
						  		  	HD44780_GotoXY( 11 , 0 );            //set the character cursor to col=11, row=0

						  		}

						  		  			HD44780_PutStr( arr );				// display the temperature stored as a string
						  		  			HD44780_PutStr( degreeSymbol ); 	// display the ° symbol
						  		  			HD44780_PutStr( "C" );				// display C
						  		  			HAL_Delay( 500 );					// Delay of 500ms
				}


		 }

		else if ( averageAnaloguevoltage >= 715  &&  averageAnaloguevoltage <= 841 )
			 // The case in which temperature is between 5°C and 15°C


		{
				HAL_GPIO_WritePin( GPIOE , GPIO_PIN_11 , GPIO_PIN_SET );   // turn on the yellow led
				HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , GPIO_PIN_RESET );
				HAL_GPIO_WritePin( GPIOE , GPIO_PIN_10 , GPIO_PIN_RESET );
				HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , GPIO_PIN_RESET );
				RGBColorMixer( 255 , 255 , 0); // call to set RGB led to yellow,  with R,G,B value of 255,255,0
				HAL_Delay( 100 ); 			   // Delay of 100ms
		  		tempDifference =  averageAnaloguevoltage - 715.0 ;  // Difference with the averageAnalog value for 5°C
				absoluteTemperature = ( tempDifference * 10.0/126.0 )+ 5.0 ;  // The exact temperature for the analog value read
				char arr[ sizeof( absoluteTemperature ) ];  // Array used to store the absolute temperature as a string
				sprintf( arr ,  "%2.1f" , absoluteTemperature );	//  Convert the temperature into a string
				HD44780_Init( );		// Initialize the LCD
				HD44780_ClrScr( );	    //clear the display
				HAL_Delay( 1000 );	    // Delay of 1000ms

				if ( absoluteTemperature < 10 )     // when temperature is in between 5°C and 10°C , ie with one digit
				{
					HD44780_GotoXY( 11 , 0);         //set the character cursor to col=11, row=0
				}

				else if ( absoluteTemperature >= 10) // when temperature is in between 10°C and 15°C , ie with two digit

				{
					HD44780_GotoXY( 10 , 0 );   //set the character cursor to col=10, row=0

				}


				HD44780_PutStr( arr );			// display the temperature stored as a string
				HD44780_PutStr( degreeSymbol );	// display the ° symbol
				HD44780_PutStr( "C" );			// display C
				HAL_Delay( 500 );					// Delay of 500ms


						  		  		 	 }
				else if ( averageAnaloguevoltage >= 856 &&   averageAnaloguevoltage <= 985 )

					// The case in which temperature is between 15°C and 25°C
				{

					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_10 , GPIO_PIN_SET );		// turn on the orange led
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , GPIO_PIN_RESET );
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , GPIO_PIN_RESET );
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_11 , GPIO_PIN_RESET );
  		  			RGBColorMixer( 255 , 128 , 0 );		// call to set RGB led to orange,  with R,G,B value of 255,128,0
  		  			HAL_Delay( 100 );				    // Delay of 100ms
  		  			tempDifference =  averageAnaloguevoltage - 856.0 ;		// Difference with the averageAnalog value for 15°C
					absoluteTemperature =  ( tempDifference *  10.0/129.0 ) + 15.0;	// The exact temperature for the analog value read
					char arr[sizeof( absoluteTemperature )];			// Array used to store the absolute temperature as a string
					sprintf( arr ,  "%2.1f" , absoluteTemperature );	//  Convert the temperature into a string

					HD44780_Init( );				// Initialize the LCD
					HD44780_ClrScr( );				//clear the display
					HAL_Delay( 1000 );				// Delay of 1s
					HD44780_GotoXY( 10 , 0 );		//set the character cursor to col=10, row=0
					HD44780_PutStr( arr );			// display the temperature stored as a string
					HD44780_PutStr( degreeSymbol );	// display the ° symbol
					HD44780_PutStr( "C" );			// display C
					HAL_Delay( 500 );				// Delay of 500ms


				}
				else if ( averageAnaloguevoltage > 997  &&   averageAnaloguevoltage <= 1320 )
					// The case in which temperature is between 25°C and 50°C
				{

					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , GPIO_PIN_SET );		// turn on the red led
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , GPIO_PIN_RESET );
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_10 , GPIO_PIN_RESET );
					HAL_GPIO_WritePin( GPIOE , GPIO_PIN_11 , GPIO_PIN_RESET );
					RGBColorMixer( 255 , 0 , 0 );		// call to set RGB led to red,  with R,G,B value of 255,0,0
					HAL_Delay( 100 );				    // Delay of 100ms
					tempDifference =  averageAnaloguevoltage - 997.0 ;		// Difference with the averageAnalog value for 25°C
					absoluteTemperature = ( tempDifference *  25.0/323.0 ) +25.0 ;		// The exact temperature for the analog value read
					char arr[sizeof( absoluteTemperature )];		  // Array used to store the absolute temperature as a string
					sprintf( arr ,  "%2.1f" , absoluteTemperature );  //  Convert the temperature into a string

					HD44780_Init( );					// Initialize the LCD
					HD44780_ClrScr( );			    	//clear the display
					HAL_Delay( 1000 );				    // Delay of 1s
					HD44780_GotoXY( 10 , 0 );			//set the character cursor to col=10, row=0
					HD44780_PutStr( arr );			    // display the temperature stored as a string
					HD44780_PutStr( degreeSymbol );	    // display the ° symbol
					HD44780_PutStr( "C" );			    // display C
					HAL_Delay( 500 );					// Delay of 500ms


				}

				else if( averageAnaloguevoltage > 1320  || averageAnaloguevoltage <  415 )
					// The case in which temperature is less than -15°c or greater than  50°C

				{
					HD44780_Init( );					 // Initialize the LCD
					HD44780_ClrScr( );				     //clear the display
					HAL_Delay( 1000 );				     // Delay of 1s
					HD44780_GotoXY( 0 , 0 );			 //set the character cursor to col=0, row=0
					HD44780_PutStr( "out of range " );   //display text
					HAL_Delay(500);					     // Delay of 500ms
					RGBColorMixer( 255 , 255 , 255 );	 // call to set RGB led to white ,  with R,G,B value of 255,255,255
					HAL_Delay( 100 );				 	 // Delay of 100ms


				}

	}


}

/*
       * FUNCTION		: initEport
       *
       * DESCRIPTION 	: To initialize the port e with input mode,speed,configuration ...etc
       *
       * PARAMETERS		: void
       *
       * RETURNS		: void

       */

void initEport( )
{

		__GPIOE_CLK_ENABLE( );

		GPIO_InitStruct.Pin = ( GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12
				| GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8 );
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

		HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

}

/*
       * FUNCTION		: SystemClock_Config
       *
       * DESCRIPTION 	: To configure the system clock as 72 mhz
       *
       * PARAMETERS		: void
       *
       * RETURNS		: void

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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_TIM8
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/*
       * FUNCTION		: MX_ADC1_Init
       *
       * DESCRIPTION 	: To initialize the adc used for the temperature sensor
       *
       * PARAMETERS		: void
       *
       * RETURNS		: void

       */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
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
  sConfig.Channel = ADC_CHANNEL_1;
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

/* TIM2 init function */
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
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);

}

/* TIM8 init function */
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
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
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
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim8);

}

/* TIM15 init function */
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
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
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
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) != HAL_OK)
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
  if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim15);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

}


/* FUNCTION      : RGBColorMixer
 * DESCRIPTION   : This function mixes the colors of red, green, and blue using PWM generator as follows:
 * 					Red pin   ---> 150ohms --->PA3
 * 					Ground	  ---------------->GND
 *					Green pin ---> 100ohms --->PC9
 *					Blue pin  ---> 100ohms --->PF10
 * PARAMETERS    : redIntensity   : Unsigned 8bit integer (0-255)
 greenIntensity : Unsigned 8bit integer (0-255)
 blueIntensity  : Unsigned 8bit integer (0-255)
 * RETURNS       : NULL
 */
void RGBColorMixer(uint8_t redIntensity, uint8_t greenIntensity,
		uint8_t blueIntensity) {
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, redIntensity);
	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, greenIntensity);
	__HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_2, blueIntensity);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
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

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
