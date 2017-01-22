/*
  *FILE   		  :  AssignmentNo2
  *PROJECT		  :  PROG8125 - Assignment # 2
  *PROGRAMMER     :  Nimal krishna
  *FIRST VERSION  :  07/10/2016
  *DESCRIPTION    :  This project is  for an input sequence detector, which will detect the order of inputs,
  *                  and checks if it matches the  stored values of input combinations.If the input given is one of the  stored combination of inputs,
  *                  a green led is lit along with an auditory feedback, else a red led is lit along with another auditory feed back which indicates
  *                  that the input given is wrong push buttons are used to give inputs.Inputs can be given any number of times and any button can be pressed
  *                  any number of times.The accepted combinations of inputs are  1234,1122,4334,1244,1231,2222,2434,1111,4344,1214 in order, but they can be changed
  *                  if it is required.
 */



/* Includes ------------------------------------------------------------------*/

#include "stm32f3xx_hal.h"
#include "usb_device.h"
#include<stdio.h>
#include <vcp_prog8125.h>

/* Global variables declaration ---------------------------------------------------*/

uint16_t abutPressedtimes = 0;   // Number of times buttonA is pressed
uint16_t bbutPressedtimes = 0;	 // Number of times buttonB is pressed
uint16_t cbutPressedtimes = 0;	 // Number of times buttonC is pressed
uint16_t dbutPressedtimes = 0;	 // Number of times buttonD is pressed
uint16_t countTotalpress  = 0;	 // Total number of times of button press
uint16_t toggleNumb = 0;         // This number changes value whenever a button press is detected and  corresponding function is called.
								 // It helps to ensure that a button press is counted only one time only as it helps to define buttonPress as the case of
								 // a button is pressed and then released. If  toggleNumb = 0 it is the case when all buttons are released,
								 // Whereas if toggleNumb = 1 , it is the case of one of the four button is pressed.

uint16_t orderofButtonpress[4] = { 0 , 0 , 0 , 0 };   // This array stores the sequence of button press detected

uint16_t passCodes[10][4] = {    { 1 , 2 , 3 , 4 }, { 1 , 1 , 2 , 2 }, { 4 , 3 , 3 , 4 },{ 1 , 2 , 4 , 4 },{ 1 , 2 , 3 , 1 },
                                 { 2 , 2 , 2 , 2 }, { 2 , 4 , 3 , 4 }, { 1 , 1 , 1 , 1 },{ 4 , 3 , 4 , 4 },{ 1 , 2 , 1 , 4 }      } ;
													  // The sequences of button press which are approved, change the values here if you want to change the approved patterns



/* structure declarations ---------------------------------------------------------*/

TIM_HandleTypeDef htim1;
static GPIO_InitTypeDef  GPIO_InitStruct;
circularQueue_t  VcpQueue;   // queue used for input from vcp

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config( void );
void Error_Handler( void );
static void MX_GPIO_Init( void );
static void MX_TIM1_Init( void );
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void BuzzerSet(int16_t buz, int32_t pattern, uint32_t len, uint32_t delay);
void BuzzerTask( void );
void BuzzerInit( void );
void porteInit( void );
void portaInit( void );
void butallReleased( void );
void butaPressed( void );
void butbPressed( void );
void butcPressed( void );
void butdPressed( void );
int matchCheck( void );

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
  MX_GPIO_Init( );
  MX_USB_DEVICE_Init( );
  MX_TIM1_Init( );
  porteInit( );
  portaInit( );
  BuzzerInit( );
  initializeVcpQueue( &VcpQueue );

  /* This is the infinite loop which keeps on running */

  while ( 1 )

  {
	   /*  This function is called every time to ensure that the buzzer parameters are updated, and to keep the buzzer running
	       whenever is required */
	    BuzzerTask( );

	    if (     HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_0 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_1 ) == 0 							// The case when no button is pressed
			 &&  HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_2 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_3 ) == 0 && toggleNumb == 0 )
	     {
	  	  	 	butallReleased( );      // This function updates the value of toggleNumb to 1 , the other functions can check the value of
	  	  	 						    // toggleNumb to check for the case of no button is pressed
	  	  	 	HAL_Delay( 200 );	    // This delay helps to avoid bouncing effect

	  	 }

	  	  if(     HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_0 ) == 1 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_1 ) == 0							// The case of botton_A is pressed
	  		  &&  HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_2 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_3 ) == 0 && toggleNumb == 1 )
	  	 {
	  	 	 butaPressed( );           // This function updates the value of abutPressedtimes , countTotalpress , toggleNumb
	  	  	 HAL_Delay( 200 );		   // This delay helps to avoid bouncing effect

	  	 }

	  	  if(     HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_1 ) == 1 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_0 ) == 0							// The case of botton_B is pressed
	  		  &&  HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_2 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_3 ) == 0 && toggleNumb == 1 )
	  	 {
	  		  butbPressed( );		 // This function updates the value of bbutPressedtimes , countTotalpress , toggleNumb
	  		  HAL_Delay( 200 );		 // This delay helps to avoid bouncing effect

	  	  }

	  	  if(     HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_2 ) == 1 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_0 ) == 0 							// The case of botton_C is pressed
	  		  &&  HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_1 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_3 ) == 0 && toggleNumb == 1 )
	  	  {
	  		  butcPressed( );         // This function updates the value of cbutPressedtimes , countTotalpress , toggleNumb
	  		  HAL_Delay( 200 );		  // This delay helps to avoid bouncing effect


	  	 }

	  	  if(         HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_3 ) == 1 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_0 ) == 0 						// The case of botton_Dis pressed
	  			  &&  HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_1 ) == 0 && HAL_GPIO_ReadPin( GPIOA , GPIO_PIN_2 ) == 0 && toggleNumb == 1 )
	  	 {
	  		  butdPressed( );		// This function updates the value of dbutPressedtimes , countTotalpress , toggleNumb
	  		  HAL_Delay( 200 );		// This delay helps to avoid bouncing effect

	  	 }

	  	 if( countTotalpress == 4 )  // After the completion of 4 buttonPresses
	  	 {

	  		  if(  matchCheck() == 1  )   // If the button press pattern matches the approved patterns
	  		  {

	  			  	  	BuzzerSet( 0 , 0x0000ffff , 1 , 1000 );           // Makes the buzzer to sound one time, indicating acceptance
	  			  		HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , 1);       // Turn on the green led for 1 second
	  			  		HAL_Delay( 1000 );
	  			  		HAL_GPIO_WritePin( GPIOE , GPIO_PIN_8 , 0);

	  			  		abutPressedtimes = 0;              // Since buttons are pressed for 4 times ,all variables are initialized again  to detect next button press sequence.
	  			  		bbutPressedtimes = 0;
	  			  		cbutPressedtimes = 0;
	  			  		dbutPressedtimes = 0;
	  			  		countTotalpress = 0;
	  			  		toggleNumb = 0;
	  		  }

	  		  else if( matchCheck() != 1 ) 		// If the button press don't match  the approved patterns
	  		  {

	  			  	  	BuzzerSet( 0 , 0x00001111 , 1 , 11111 );         // Make the buzzer sound to indicate wrong input sequence
	  			  	  	HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , 1 );     //Turn on the red led for 1 second
	  			  	  	HAL_Delay( 1000 );
	  			  	  	HAL_GPIO_WritePin( GPIOE , GPIO_PIN_9 , 0 );


	  					abutPressedtimes = 0;	// Since buttons are pressed for 4 times ,all variables are initialized again  to detect next button press sequence.
	  					bbutPressedtimes = 0;
	  					cbutPressedtimes = 0;
	  					dbutPressedtimes = 0;
	  					countTotalpress = 0;
	  					toggleNumb = 0;

	  		  }
	  	 }





    }



}

/*
 * FUNCTION		: butallReleased
 *
 * DESCRIPTION 	: To detect the case when no button is pressed, and  mark such case with a stamp, i.e by giving toggleNumb = 1
 *
 * PARAMETERS	: void
 *
 * RETURNS		: void

 */


void butallReleased( void )
{

 	toggleNumb = 1; // To detect the case when no button is pressed, and  mark such case with a stamp, i.e by giving toggleNumb = 1



}

/*
 * FUNCTION		: butaPressed
 *
 * DESCRIPTION 	: To detect the case when  button_A is pressed, and  update the variables abutPressedtimesbutPressedtimes , countTotabutPressedtimeslpress , toggleNumb
 *
 * PabutPressedtimesRAMETERS	: void
 *
 * RETURNS		: void

 */

 void butaPressed( void )
 {
 	if ( countTotalpress == 0)        // Case when button_A is the first button to get pressed
 	{
 		countTotalpress = 1;          // Total button press equals 1
 		abutPressedtimes = 1;         // Number of times button_A is pressed equals 1
 		toggleNumb = 0;			      // ToggleNumb equals 1 indicate the case when a button is being pressed
 		orderofButtonpress[ 0 ] = 1;  // The first button pressed is marked as A , by giving a value of 1 in the first position of total 4 positions in the array


 	}

 	if(countTotalpress == 1 && abutPressedtimes == 0 && toggleNumb == 1) // Case when button_A is the second  button to get  pressed, but first button pressed is not A
 	{
 		countTotalpress = 2;		 // Total button press equals 2
 		abutPressedtimes = 1;		 // Number of times button_A is pressed equals 1
 		toggleNumb = 0;				 // ToggleNumb equals 1 indicate the case when a button is being pressed
 		orderofButtonpress[1] = 1;	 // The second button pressed is marked as A , by giving a value of 1 in the second position of total 4 positions in the array

 	}

 	if(countTotalpress == 1 && abutPressedtimes == 1 && toggleNumb == 1)		// Case when button_A is the second  button to get  pressed and first button pressed is also  A
 	{
 			countTotalpress = 2;	   // Total button press equals 2
 			abutPressedtimes = 2;	   // Number of times button_A is pressed equals 2
 			toggleNumb = 0;			   // ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[1] = 1; // The second button pressed is marked as A , by giving a value of 1 in the second position of total 4 positions in the array

 	}

 	if(countTotalpress == 2 && abutPressedtimes == 0 && toggleNumb == 1) // Case when button_A is the third  button to get  pressed, but first and second button pressed is not A
 	{
 			countTotalpress = 3;		// Total button press equals 3
 			abutPressedtimes = 1;		// Number of times button_A is pressed equals 1
 			toggleNumb = 0;				// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[2] = 1;	// The third button pressed is marked as A , by giving a value of 1 in the third position of total 4 positions in the array

 	}

 	if(countTotalpress == 2 && abutPressedtimes == 1 && toggleNumb == 1) // Case when button_A is the third  button to get  pressed, but button_A is already pressed one time earlier
 	{
 			countTotalpress = 3;			// Total button press equals 3
 			abutPressedtimes = 2;			// Number of times button_A is pressed equals 2
 			toggleNumb = 0;         		// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[2] = 1;		// The third button pressed is marked as A , by giving a value of 1 in the third position of total 4 positions in the array

 	}

 	if(countTotalpress == 2 && abutPressedtimes == 2 && toggleNumb == 1) // Case when button_A is the third  button to get  pressed, but button_A is already pressed twice  earlier
 	{
 			countTotalpress = 3;			// Total button press equals 3
 			abutPressedtimes = 3;			// Number of times button_A is pressed equals 2
 			toggleNumb = 0;				    // ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[2] = 1;      // The third button pressed is marked as A , by giving a value of 1 in the third position of total 4 positions in the array

 	}


   if(countTotalpress == 3 && abutPressedtimes == 0 && toggleNumb == 1) // Case when button_A is the fourth button to get  pressed, but button_A is not pressed  earlier
 	{
 			countTotalpress = 4;			// Total button press equals 4
 			abutPressedtimes = 1;			// Number of times button_A is pressed equals 1
 			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[3] = 1;		// The fourth button pressed is marked as A , by giving a value of 1 in the fourth position of total 4 positions in the array

 	}

 	if(countTotalpress == 3 && abutPressedtimes == 1 && toggleNumb == 1) // Case when button_A is the fourth  button to get  pressed, but button_A is already pressed once  earlier
 	{
 			countTotalpress = 4; 			// Total button press equals 4
 			abutPressedtimes = 2;			// Number of times button_A is pressed equals 2
 			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[3] = 1;		// The fourth button pressed is marked as A , by giving a value of 1 in the fourth position of total 4 positions in the array

 	}

 	if(countTotalpress == 3 && abutPressedtimes == 2 && toggleNumb == 1) // Case when button_A is the fourth  button to get  pressed, but button_A is already pressed twice earlier
 	{
 			countTotalpress = 4;			// Total button press equals 4
 			abutPressedtimes = 3;			// Number of times button_A is pressed equals 3
 			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[3] = 1;		// The fourth button pressed is marked as A , by giving a value of 1 in the fourth position of total 4 positions in the array

 	}

 	if(countTotalpress == 3 && abutPressedtimes == 3 && toggleNumb == 1) // Case when button_A is the fourth  button to get  pressed, but button_A is already pressed thrice earlier
 	{
 			countTotalpress = 4;			// Total button press equals 4
 			abutPressedtimes = 4;			// Number of times button_A is pressed equals 4
 			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
 			orderofButtonpress[3] = 1;		// The fourth button pressed is marked as A , by giving a value of 1 in the fourth position of total 4 positions in the array

 	}


 }


 /*
  * FUNCTION		: butbPressed
  *
  * DESCRIPTION 	: To detect the case when  button_B is pressed, and  update the variables bbutPressedtimesbutPressedtimes , countTotabutPressedtimeslpress , toggleNumb
  *
  * PabutPressedtimesRAMETERS	: void
  *
  * RETURNS		: void

  */

  void butbPressed( void )
  {
  	if ( countTotalpress == 0)      // Case when button_B is the first button to get pressed
  	{
  		countTotalpress = 1;        // Total button press equals 1
  		bbutPressedtimes = 1;       // Number of times button_B is pressed equals 1
  		toggleNumb = 0;			    // ToggleNumb equals 1 indicate the case when a button is being pressed
  		orderofButtonpress[0] = 2;  // The first button pressed is marked as B , by giving a value of 1 in the first position of total 4 positions in the array


  	}

  	if(countTotalpress == 1 && bbutPressedtimes == 0 && toggleNumb == 1) // Case when button_B is the second  button to get  pressed, but first button pressed is not B
  	{
  		countTotalpress = 2;		 // Total button press equals 2
  		bbutPressedtimes = 1;		 // Number of times button_B is pressed equals 1
  		toggleNumb = 0;				 // ToggleNumb equals 1 indicate the case when a button is being pressed
  		orderofButtonpress[1] = 2;	 // The second button pressed is marked as B, by giving a value of 1 in the second position of total 4 positions in the array

  	}

  	if(countTotalpress == 1 && bbutPressedtimes == 1 && toggleNumb == 1)		// Case when button_B is the second  button to get  pressed and first button pressed is also  B
  	{
  			countTotalpress = 2;	   // Total button press equals 2
  			bbutPressedtimes = 2;	   // Number of times button_B is pressed equals 2
  			toggleNumb = 0;			   // ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[1] = 2; // The second button pressed is marked as B , by giving a value of 1 in the second position of total 4 positions in the array

  	}

  	if(countTotalpress == 2 && bbutPressedtimes == 0 && toggleNumb == 1) // Case when button_B is the third  button to get  pressed, but first and second button pressed is not B
  	{
  			countTotalpress = 3;		// Total button press equals 3
  			bbutPressedtimes = 1;		// Number of times button_B is pressed equals 1
  			toggleNumb = 0;				// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[2] = 2;	// The third button pressed is marked as B , by giving a value of 1 in the third position of total 4 positions in the array

  	}

  	if(countTotalpress == 2 && bbutPressedtimes == 1 && toggleNumb == 1) // Case when button_B is the third  button to get  pressed, but button_B is already pressed one time earlier
  	{
  			countTotalpress = 3;			// Total button press equals 3
  			bbutPressedtimes = 2;			// Number of times button_B is pressed equals 2
  			toggleNumb = 0;         		// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[2] = 2;		// The third button pressed is marked as B , by giving a value of 1 in the third position of total 4 positions in the array

  	}

  	if(countTotalpress == 2 && bbutPressedtimes == 2 && toggleNumb == 1) // Case when button_B is the third  button to get  pressed, but button_B is already pressed twice  earlier
  	{
  			countTotalpress = 3;			// Total button press equals 3
  			bbutPressedtimes = 3;			// Number of times button_B is pressed equals 2
  			toggleNumb = 0;				    // ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[2] = 2;      // The third button pressed is marked as B , by giving a value of 1 in the third position of total 4 positions in the array

  	}


    if(countTotalpress == 3 && bbutPressedtimes == 0 && toggleNumb == 1) // Case when button_B is the fourth button to get  pressed, but button_B is not pressed  earlier
  	{
  			countTotalpress = 4;			// Total button press equals 4
  			bbutPressedtimes = 1;			// Number of times button_B is pressed equals 1
  			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[3] = 2;		// The fourth button pressed is marked as B , by giving a value of 1 in the fourth position of total 4 positions in the array

  	}

  	if(countTotalpress == 3 && bbutPressedtimes == 1 && toggleNumb == 1) // Case when button_B is the fourth  button to get  pressed, but button_B is already pressed once  earlier
  	{
  			countTotalpress = 4; 			// Total button press equals 4
  			bbutPressedtimes = 2;			// Number of times button_B is pressed equals 2
  			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[3] = 2;		// The fourth button pressed is marked as B , by giving a value of 1 in the fourth position of total 4 positions in the array

  	}

  	if(countTotalpress == 3 && bbutPressedtimes == 2 && toggleNumb == 1) // Case when button_B is the fourth  button to get  pressed, but button_B is already pressed twice earlier
  	{
  			countTotalpress = 4;			// Total button press equals 4
  			bbutPressedtimes = 3;			// Number of times button_B is pressed equals 3
  			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[3] = 2;		// The fourth button pressed is marked as B , by giving a value of 1 in the fourth position of total 4 positions in the array

  	}

  	if(countTotalpress == 3 && bbutPressedtimes == 3 && toggleNumb == 1) // Case when button_B is the fourth  button to get  pressed, but button_B is already pressed thrice earlier
  	{
  			countTotalpress = 4;			// Total button press equals 4
  			bbutPressedtimes = 4;			// Number of times button_B is pressed equals 4
  			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
  			orderofButtonpress[3] = 2;		// The fourth button pressed is marked as B , by giving a value of 1 in the fourth position of total 4 positions in the array

  	}


  }


  /*
   * FUNCTION		: butcPressed
   *
   * DESCRIPTION 	: To detect the case when no button_C is pressed, and  update the variables abutPressedtimesbutPressedtimes , countTotabutPressedtimeslpress , toggleNumb
   *
   * PabutPressedtimesRAMETERS	: void
   *
   * RETURNS		: void

   */

   void butcPressed( void )
   {
   	if ( countTotalpress == 0)      // Case when button_C is the first button to get pressed
   	{
   		countTotalpress = 1;        // Total button press equals 1
   		cbutPressedtimes = 1;       // Number of times button_C is pressed equals 1
   		toggleNumb = 0;			    // ToggleNumb equals 1 indicate the case when a button is being pressed
   		orderofButtonpress[0] = 3;  // The first button pressed is marked as C, by giving a value of 1 in the first position of total 4 positions in the array


   	}

   	if(countTotalpress == 1 && cbutPressedtimes == 0 && toggleNumb == 1) // Case when button_C is the second  button to get  pressed, but first button pressed is not C
   	{
   		countTotalpress = 2;		 // Total button press equals 2
   		cbutPressedtimes = 1;		 // Number of times button_C is pressed equals 1
   		toggleNumb = 0;				 // ToggleNumb equals 1 indicate the case when a button is being pressed
   		orderofButtonpress[1] = 3;	 // The second button pressed is marked as C , by giving a value of 1 in the second position of total 4 positions in the array

   	}

   	if(countTotalpress == 1 && cbutPressedtimes == 1 && toggleNumb == 1)		// Case when button_C is the second  button to get  pressed and first button pressed is also  C
   	{
   			countTotalpress = 2;	   // Total button press equals 2
   			cbutPressedtimes = 2;	   // Number of times button_C is pressed equals 2
   			toggleNumb = 0;			   // ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[1] = 3; // The second button pressed is marked as C , by giving a value of 1 in the second position of total 4 positions in the array

   	}

   	if(countTotalpress == 2 && cbutPressedtimes == 0 && toggleNumb == 1) // Case when button_C is the third  button to get  pressed, but first and second button pressed is not C
   	{
   			countTotalpress = 3;		// Total button press equals 3
   			cbutPressedtimes = 1;		// Number of times button_C is pressed equals 1
   			toggleNumb = 0;				// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[2] = 3;	// The third button pressed is marked as C , by giving a value of 1 in the third position of total 4 positions in the array

   	}

   	if(countTotalpress == 2 && cbutPressedtimes == 1 && toggleNumb == 1) // Case when button_C is the third  button to get  pressed, but button_C is already pressed one time earlier
   	{
   			countTotalpress = 3;			// Total button press equals 3
   			cbutPressedtimes = 2;			// Number of times button_C is pressed equals 2
   			toggleNumb = 0;         		// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[2] = 3;		// The third button pressed is marked as C , by giving a value of 1 in the third position of total 4 positions in the array

   	}

   	if(countTotalpress == 2 && cbutPressedtimes == 2 && toggleNumb == 1) // Case when button_C is the third  button to get  pressed, but button_C is already pressed twice  earlier
   	{
   			countTotalpress = 3;			// Total button press equals 3
   			cbutPressedtimes = 3;			// Number of times button_C is pressed equals 2
   			toggleNumb = 0;				    // ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[2] = 3;      // The third button pressed is marked as C , by giving a value of 1 in the third position of total 4 positions in the array

   	}


     if(countTotalpress == 3 && cbutPressedtimes == 0 && toggleNumb == 1) // Case when button_C is the fourth button to get  pressed, but button_C is not pressed  earlier
   	{
   			countTotalpress = 4;			// Total button press equals 4
   			cbutPressedtimes = 1;			// Number of times button_C is pressed equals 1
   			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[3] = 3;		// The fourth button pressed is marked as C , by giving a value of 1 in the fourth position of total 4 positions in the array

   	}

   	if(countTotalpress == 3 && cbutPressedtimes == 1 && toggleNumb == 1) // Case when button_C is the fourth  button to get  pressed, but button_C is already pressed once  earlier
   	{
   			countTotalpress = 4; 			// Total button press equals 4
   			cbutPressedtimes = 2;			// Number of times button_C is pressed equals 2
   			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[3] = 3;		// The fourth button pressed is marked as C , by giving a value of 1 in the fourth position of total 4 positions in the array

   	}

   	if(countTotalpress == 3 && cbutPressedtimes == 2 && toggleNumb == 1) // Case when button_C is the fourth  button to get  pressed, but button_C is already pressed twice earlier
   	{
   			countTotalpress = 4;			// Total button press equals 4
   			cbutPressedtimes = 3;			// Number of times button_C is pressed equals 3
   			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[3] = 3;		// The fourth button pressed is marked as C , by giving a value of 1 in the fourth position of total 4 positions in the array

   	}

   	if(countTotalpress == 3 && cbutPressedtimes == 3 && toggleNumb == 1) // Case when button_C is the fourth  button to get  pressed, but button_C is already pressed thrice earlier
   	{
   			countTotalpress = 4;			// Total button press equals 4
   			cbutPressedtimes = 4;			// Number of times button_C is pressed equals 4
   			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
   			orderofButtonpress[3] = 3;		// The fourth button pressed is marked as C , by giving a value of 1 in the fourth position of total 4 positions in the array

   	}


   }


   /*
    * FUNCTION		: butdPressed
    *
    * DESCRIPTION 	: To detect the case when no button_D is pressed, and  update the variables abutPressedtimesbutPressedtimes , countTotabutPressedtimeslpress , toggleNumb
    *
    * PabutPressedtimesRAMETERS	: void
    *
    * RETURNS		: void

    */

    void butdPressed( void )
    {
    	if ( countTotalpress == 0)      // Case when button_D is the first button to get pressed
    	{
    		countTotalpress = 1;        // Total button press equals 1
    		dbutPressedtimes = 1;       // Number of times button_D is pressed equals 1
    		toggleNumb = 0;			    // ToggleNumb equals 1 indicate the case when a button is being pressed
    		orderofButtonpress[0] = 4;  // The first button pressed is marked as D , by giving a value of 1 in the first position of total 4 positions in the array


    	}

    	if(countTotalpress == 1 && dbutPressedtimes == 0 && toggleNumb == 1) // Case when button_D is the second  button to get  pressed, but first button pressed is not D
    	{
    		countTotalpress = 2;		 // Total button press equals 2
    		dbutPressedtimes = 1;		 // Number of times button_D is pressed equals 1
    		toggleNumb = 0;				 // ToggleNumb equals 1 indicate the case when a button is being pressed
    		orderofButtonpress[1] = 4;	 // The second button pressed is marked as D , by giving a value of 1 in the second position of total 4 positions in the array

    	}

    	if(countTotalpress == 1 && dbutPressedtimes == 1 && toggleNumb == 1)		// Case when button_D is the second  button to get  pressed and first button pressed is also D
    	{
    			countTotalpress = 2;	   // Total button press equals 2
    			dbutPressedtimes = 2;	   // Number of times button_D is pressed equals 2
    			toggleNumb = 0;			   // ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[1] = 4; // The second button pressed is marked as D , by giving a value of 1 in the second position of total 4 positions in the array

    	}

    	if(countTotalpress == 2 && dbutPressedtimes == 0 && toggleNumb == 1) // Case when button_D is the third  button to get  pressed, but first and second button pressed is not D
    	{
    			countTotalpress = 3;		// Total button press equals 3
    			dbutPressedtimes = 1;		// Number of times button_D is pressed equals 1
    			toggleNumb = 0;				// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[2] = 4;	// The third button pressed is marked as D , by giving a value of 1 in the third position of total 4 positions in the array

    	}

    	if(countTotalpress == 2 && dbutPressedtimes == 1 && toggleNumb == 1) // Case when button_D is the third  button to get  pressed, but button_D is already pressed one time earlier
    	{
    			countTotalpress = 3;			// Total button press equals 3
    			dbutPressedtimes = 2;			// Number of times button_D is pressed equals 2
    			toggleNumb = 0;         		// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[2] = 4;		// The third button pressed is marked as D , by giving a value of 1 in the third position of total 4 positions in the array

    	}

    	if(countTotalpress == 2 && dbutPressedtimes == 2 && toggleNumb == 1) // Case when button_D is the third  button to get  pressed, but button_D is already pressed twice  earlier
    	{
    			countTotalpress = 3;			// Total button press equals 3
    			dbutPressedtimes = 3;			// Number of times button_D is pressed equals 2
    			toggleNumb = 0;				    // ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[2] = 4;      // The third button pressed is marked as D , by giving a value of 1 in the third position of total 4 positions in the array

    	}


      if(countTotalpress == 3 && dbutPressedtimes == 0 && toggleNumb == 1) // Case when button_D is the fourth button to get  pressed, but button_D is not pressed  earlier
    	{
    			countTotalpress = 4;			// Total button press equals 4
    			dbutPressedtimes = 1;			// Number of times button_D is pressed equals 1
    			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[3] = 4;		// The fourth button pressed is marked as D , by giving a value of 1 in the fourth position of total 4 positions in the array

    	}

    	if(countTotalpress == 3 && dbutPressedtimes == 1 && toggleNumb == 1) // Case when button_D is the fourth  button to get  pressed, but button_D is already pressed once  earlier
    	{
    			countTotalpress = 4; 			// Total button press equals 4
    			dbutPressedtimes = 2;			// Number of times button_D is pressed equals 2
    			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[3] = 4;		// The fourth button pressed is marked as D , by giving a value of 1 in the fourth position of total 4 positions in the array

    	}

    	if(countTotalpress == 3 && dbutPressedtimes == 2 && toggleNumb == 1) // Case when button_D is the fourth  button to get  pressed, but button_D is already pressed twice earlier
    	{
    			countTotalpress = 4;			// Total button press equals 4
    			dbutPressedtimes = 3;			// Number of times button_D is pressed equals 3
    			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[3] = 4;		// The fourth button pressed is marked as D , by giving a value of 1 in the fourth position of total 4 positions in the array

    	}

    	if(countTotalpress == 3 && dbutPressedtimes == 3 && toggleNumb == 1) // Case when button_D is the fourth  button to get  pressed, but button_D is already pressed thrice earlier
    	{
    			countTotalpress = 4;			// Total button press equals 4
    			dbutPressedtimes = 4;			// Number of times button_D is pressed equals 4
    			toggleNumb = 0;					// ToggleNumb equals 1 indicate the case when a button is being pressed
    			orderofButtonpress[3] = 4;		// The fourth button pressed is marked as D , by giving a value of 1 in the fourth position of total 4 positions in the array

    	}


    }

    /*
        * FUNCTION		: matchCheck
        *
        * DESCRIPTION 	: To check if the button pressed pattern matches either one of the approved sequence of 10  button press patterns
        *
        * PARAMETERS	: void
        *
        * RETURNS		: integer

        */
    int matchCheck ( )
    {
    	for( int x = 0; x <= 9 ; x++ )
    	{


    			if(   (     (  *(&passCodes[x][0])) == (*(&orderofButtonpress[0]))   ) &&   (    (  *(&passCodes[x][1])) == (*(&orderofButtonpress[1])  )   ) &&
    				  (     (  *(&passCodes[x][2])) == (*(&orderofButtonpress[2]))   ) &&   (    (  *(&passCodes[x][3])) == (*(&orderofButtonpress[3])  )	)
				  )

    								// The above condition checks if the number stored in the orderofButtonpress array matches either one of the passCodes array values
    				{

    					 return 1;  // return is 1 , means match found
    				 }


    	}

    }


    /*
       * FUNCTION		: porteInit
       *
       * DESCRIPTION 	: To initialize the port e with input mode,speed,configuration ...etc
       *
       * PARAMETERS		: void
       *
       * RETURNS		: void

       */

  void porteInit( void )
  {

  	  __GPIOE_CLK_ENABLE( );                               // clock is turned on for the e port
  	  GPIO_InitStruct.Pin = ( GPIO_PIN_8 | GPIO_PIN_9 );   //led is initialized using the standard structure
  	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;          //output mode ,so can output signals
  	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;                //in pull DOWNconfiguration
  	  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;             //supports high speed operation

  	  HAL_GPIO_Init( GPIOE , & GPIO_InitStruct);


  }

/*
      * FUNCTION		: portaInit
      *
      * DESCRIPTION 	: To initialize the port a with input mode,speEd,configuration ...etc
      *
      * PARAMETERS		: void
      *
      * RETURNS			: void

      */

  void portaInit( void )
  {
  	  __GPIOA_CLK_ENABLE( );                                                     		  // clock is turned on for the a port
  	  GPIO_InitStruct.Pin = ( GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 );        //PIN_0 of port A is selected   using the standard structure
  	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                			  // push button in input mode,so can accept signals
  	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;                                    			  //in pull down configuration
  	  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; 											  //supports high speed operation
  	  HAL_GPIO_Init( GPIOA , &GPIO_InitStruct);
  }



  /*
        * FUNCTION			: portaInit
        *
        * DESCRIPTION 		: To initialize the system clock configuration
        *
        * PARAMETERS		: void
        *
        * RETURNS			: void

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

  if (HAL_RCC_OscConfig( &RCC_OscInitStruct) != HAL_OK )
  {
    Error_Handler( );
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK )
  {
    Error_Handler( );
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_TIM1;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler( );
  }

  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 );

  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );

}



/*
        * FUNCTION			: MX_TIM1_Init
        *
        * DESCRIPTION 		: To initialize the timer 1
        *
        * PARAMETERS		: void
        *
        * RETURNS			: void

        */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
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
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
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
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);

}

/*
        * FUNCTION			: MX_GPIO_Init
        *
        * DESCRIPTION 		: To initialize the gpio ports with timers
        *
        * PARAMETERS		: void
        *
        * RETURNS			: void

        */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE( );
  __HAL_RCC_GPIOA_CLK_ENABLE( );

}


/*
        * FUNCTION			: Error_Handler
        *
        * DESCRIPTION 		: The error handler
        *
        * PARAMETERS		: void
        *
        * RETURNS			: void

        */

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
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

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
