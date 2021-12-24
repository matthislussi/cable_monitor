/**************************************************************************************
 * lcd_gui.c
 *
 *
 * Function: Outputs the distance of one Phase
 *
 *
 *  Created on: 04.12.2021
 *      Author: matth
 *************************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "main.h"
#include "analytics.h"
#include "menu.h"




/******************************************************************************
 * Defines
 *****************************************************************************/
#define ADC_DAC_RES		12			///< Resolution

/******************************************************************************
 * Variables
 *****************************************************************************/
const uint32_t Y_OFFSET = 260;
const uint32_t X_SIZE = 240;
const uint32_t f = (1 << ADC_DAC_RES) / Y_OFFSET + 1;	// Scaling factor
const uint32_t max_dist = 200;

static uint32_t actual_dist = 201;
static RESULT_item_t result;

/******************************************************************************
* @brief determines if cable monitor is within maximum range
*
* @n function writes "OUT OF RANGE!" when out of range
 *****************************************************************************/
static void gui_within_range(){
	if(actual_dist > max_dist){
		char text[23];
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		BSP_LCD_SetFont(&Font24);
		snprintf(text, 22, "OUT OF RANGE!");
		BSP_LCD_DisplayStringAt(0, 175, (uint8_t *)text, CENTER_MODE);
	}
}

/******************************************************************************
* @brief sets up the lcd screen for single measurement
 *****************************************************************************/
void gui_sing_measurement(){

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET+1);
	/* Write first 2 samples as numbers */

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	char text[23];
	BSP_LCD_SetFont(&Font20);
	snprintf(text, 22, "SING. MEASURM.:");
	BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 22, "Distance:");
	BSP_LCD_DisplayStringAt(0, 45, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Angle:               ");
	BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:             ");
	BSP_LCD_DisplayStringAt(0, 75, (uint8_t *)text, LEFT_MODE);
	GUI_WithinRange();
}

/******************************************************************************
* @brief sets up the lcd screen for accurate measurement
 *****************************************************************************/
void gui_acc_measurement(){


	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET+1);
	/* Write first 2 samples as numbers */

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	char text[23];
	BSP_LCD_SetFont(&Font20);
	snprintf(text, 22, "ACCUR. MEASURM.:");
	BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 22, "Distance:" );
	BSP_LCD_DisplayStringAt(0, 45, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 22, "  Mean Value:        %4d");
	BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 22, "  Strd Deviat:       ");
	BSP_LCD_DisplayStringAt(0, 75, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Angle:               ");
	BSP_LCD_DisplayStringAt(0, 93, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:             ");
	BSP_LCD_DisplayStringAt(0, 108, (uint8_t *)text, LEFT_MODE);
	GUI_WithinRange();
}

/******************************************************************************
* @brief draws curve of samples
 *****************************************************************************/
void gui_draw_samples(void){

	/*BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	data = ADC_samples[MEAS_input_count*0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (ADC_samples[MEAS_input_count*i]) / f;
		if (data > Y_OFFSET) { data = Y_OFFSET; }// Limit value, prevent crash
		BSP_LCD_DrawLine(4*(i-1), Y_OFFSET-data_last, 4*i, Y_OFFSET-data);
	}*/
/* Draw the  values of input channel 2 (if present) as a curve */
	/*if (MEAS_input_count >= 2) {
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		data = ADC_samples[MEAS_input_count*0+1] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (ADC_samples[MEAS_input_count*i+1]) / f;
			if (data > Y_OFFSET) { data = Y_OFFSET; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET-data_last, 4*i, Y_OFFSET-data);
		}
	}*/
/* Clear buffer and flag */
	/*for (uint32_t i = 0; i < ADC_NUMS; i++){
		ADC_samples[2*i] = 0;
		ADC_samples[2*i+1] = 0;
		ADC_samples[2*i+2] = 0;
		ADC_samples[2*i+3] = 0;
		}
	ADC_sample_count = 0;*/
}























