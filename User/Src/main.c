/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * Description of project
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "ts_calibration.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MAX_COLOURS 3

/* Private variables ---------------------------------------------------------*/
static int currentTimer = 0;
static int currentColour = 0;
static int cnt[2] = {0,0};

/* Private function prototypes -----------------------------------------------*/
static int GetUserButtonPressed(void);
static int GetTouchState (int *xCoord, int *yCoord);

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
	HAL_IncTick();

	// display timer
	cnt[currentTimer]++;
}


void EXTI0_IRQHandler(void) {
	currentTimer = !currentTimer;

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
}

void EXTI3_IRQHandler(void) {
	currentColour++;
	if(currentColour >= MAX_COLOURS) currentColour = 0;

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
}






/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize LCD and touch screen */
	LCD_Init();
	TS_Init(LCD_GetXSize(), LCD_GetYSize());
	/* touch screen calibration */
	//	TS_Calibration();

	/* Clear the LCD and display basic starter text */
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_YELLOW);
	LCD_SetBackColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font20);
	LCD_DisplayStringAtLineMode(1, "EXTI Interrupt", CENTER_MODE);

	LCD_SetFont(&Font8);
	LCD_SetColors(LCD_COLOR_MAGENTA, LCD_COLOR_BLACK); // TextColor, BackColor
	LCD_DisplayStringAtLineMode(39, "copyright silb", CENTER_MODE);



	GPIO_InitTypeDef userButton;
	userButton.Alternate = 0;
	userButton.Mode = GPIO_MODE_IT_RISING;
	userButton.Pin = GPIO_PIN_0;
	userButton.Pull = GPIO_NOPULL;
	userButton.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(GPIOA, &userButton);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);



	GPIO_InitTypeDef PG3;
	PG3.Alternate = 0;
	PG3.Mode = GPIO_MODE_IT_RISING;
	PG3.Pin = GPIO_PIN_3;
	PG3.Pull = GPIO_PULLUP;
	PG3.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(GPIOG, &PG3);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);



	int colour[3] = {LCD_COLOR_RED,
					LCD_COLOR_GREEN,
					LCD_COLOR_BLUE};

	/* Infinite loop */
	while (1)
	{
		LCD_SetFont(&Font20);
		LCD_SetTextColor(colour[currentColour]);

		LCD_SetPrintPosition(5, 0);
		printf("   Timer: %.1f", cnt[0]/1000.0);

		LCD_SetPrintPosition(7, 0);
		printf("   Timer: %.1f", cnt[1]/1000.0);
	}
}

/**
 * Check if User Button has been pressed
 * @param none
 * @return 1 if user button input (PA0) is high
 */
static int GetUserButtonPressed(void) {
	return (GPIOA->IDR & 0x0001);
}

/**
 * Check if touch interface has been used
 * @param xCoord x coordinate of touch event in pixels
 * @param yCoord y coordinate of touch event in pixels
 * @return 1 if touch event has been detected
 */
static int GetTouchState (int* xCoord, int* yCoord) {
	void    BSP_TS_GetState(TS_StateTypeDef *TsState);
	TS_StateTypeDef TsState;
	int touchclick = 0;

	TS_GetState(&TsState);
	if (TsState.TouchDetected) {
		*xCoord = TsState.X;
		*yCoord = TsState.Y;
		touchclick = 1;
		if (TS_IsCalibrationDone()) {
			*xCoord = TS_Calibration_GetX(*xCoord);
			*yCoord = TS_Calibration_GetY(*yCoord);
		}
	}

	return touchclick;
}


