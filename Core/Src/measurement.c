/**************************************************************************************
 * single_measurement.c
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
#include "pushbutton.h"
#include "menu.h"
#include "measuring.h"
#include "math.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
#define ADC_NUMS		60			///< Number of samples
#define ADC_DAC_RES		12			///< Resolution
/******************************************************************************
 * Variables
 *****************************************************************************/
static uint32_t ADC_samples[5*ADC_NUMS];///< ADC values of max. 2 input channels
static uint32_t ADC_samples2[2*ADC_NUMS];///< ADC values of max. 2 input channels
bool MEAS_data_ready = false;
bool ac_meas = false;
uint32_t MEAS_input_count = 1;			///< 1 or 2 input channels?

static uint32_t ADC_sample_count = 0;	///< Index for buffer
/******************************************************************************
 * Functions
 *****************************************************************************/

/******************************************************************************
 * setup IRQHandler
 *****************************************************************************/
void DMA2_Stream1_IRQHandler(void)
{
	if (DMA2->LISR & DMA_LISR_TCIF1) {	// Stream1 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream1_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);// Clear pending DMA interrupt
		DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream1->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF1;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC3->CR2 &= ~ADC_CR2_ADON;		// Disable ADC3
		ADC3->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
	}
}

/******************************************************************************
 * init adc3 for conversion
 *****************************************************************************/
void ADC3_IN13_DMA_init(void)
{
	MEAS_input_count = 1;				// Only 1 input is converted
	__HAL_RCC_ADC2_CLK_ENABLE();		// Enable Clock for ADC3
	ADC2->SQR3 |= (13UL << ADC_SQR3_SQ1_Pos);	// Input 13 = first conversion
	ADC2->CR2 |= (1UL << ADC_CR2_EXTEN_Pos);	// En. ext. trigger on rising e.
	ADC2->CR2 |= (6UL << ADC_CR2_EXTSEL_Pos);	// Timer 2 TRGO event
	ADC2->CR2 |= ADC_CR2_DMA;			// Enable DMA mode
	__HAL_RCC_DMA2_CLK_ENABLE();		// Enable Clock for DMA2
	DMA2_Stream2->CR &= ~DMA_SxCR_EN;	// Disable the DMA stream 2
	while (DMA2_Stream2->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
	DMA2->LIFCR |= DMA_LIFCR_CTCIF1;	// Clear transfer complete interrupt fl.
	DMA2_Stream2->CR |= (1UL << DMA_SxCR_CHSEL_Pos);	// Select channel 1
	DMA2_Stream2->CR |= DMA_SxCR_PL_1;		// Priority high
	DMA2_Stream2->CR |= DMA_SxCR_MSIZE_1;	// Memory data size = 32 bit
	DMA2_Stream2->CR |= DMA_SxCR_PSIZE_1;	// Peripheral data size = 32 bit
	DMA2_Stream2->CR |= DMA_SxCR_MINC;	// Increment memory address pointer
	DMA2_Stream2->CR |= DMA_SxCR_TCIE;	// Transfer complete interrupt enable
	DMA2_Stream2->NDTR = ADC_NUMS;		// Number of data items to transfer
	DMA2_Stream2->PAR = (uint32_t)&ADC2->DR;	// Peripheral register address
	DMA2_Stream2->M0AR = (uint32_t)ADC_samples2;	// Buffer memory loc. address
}

/** ***************************************************************************
 * @brief Start DMA, ADC and timer
 *
 *****************************************************************************/
void ADC3_IN13_DMA_start(void)
{
	DMA2_Stream2->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream2_IRQn);	// Enable DMA interrupt in the NVIC
	ADC2->CR2 |= ADC_CR2_ADON;			// Enable ADC3
	TIM3->CR1 |= TIM_CR1_CEN;			// Enable timer
}

/******************************************************************************
 * setup IRQHandler
 *****************************************************************************/
void DMA2_Stream2_IRQHandler(void)
{
	if (DMA2->LISR & DMA_LISR_TCIF2) {	// Stream1 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream2_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);// Clear pending DMA interrupt
		DMA2_Stream2->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream2->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF2;// Clear transfer complete interrupt fl.
		TIM3->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC2->CR2 &= ~ADC_CR2_ADON;		// Disable ADC3
		ADC2->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
		ac_meas = true;
	}
}

/******************************************************************************
 * initialize ADC3 and DMA2 for electrostatic inputs
 *****************************************************************************/
void ADC3_IN13_IN4_IN11_IN6_scan_init(void)
{
	MEAS_input_count = 2;				// Only 1 input is converted
	__HAL_RCC_ADC3_CLK_ENABLE();		// Enable Clock for ADC2
	ADC3->SQR1 |= ADC_SQR1_L_2;			// Convert 4 inputs
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ1_Pos);	// Input 13 = first conversion
	ADC3->SQR3 |= (4UL << ADC_SQR3_SQ2_Pos);	// Input 4 = second conversion
	ADC3->SQR3 |= (11UL << ADC_SQR3_SQ3_Pos);	// Input 11 = first conversion
	ADC3->SQR3 |= (6UL << ADC_SQR3_SQ4_Pos);	// Input 6 = second conversion
	ADC3->CR1 |= ADC_CR1_SCAN;			// Enable scan mode
	ADC3->CR2 |= (1UL << ADC_CR2_EXTEN_Pos);	// En. ext. trigger on rising e. (rising edge)
	ADC3->CR2 |= (6UL << ADC_CR2_EXTSEL_Pos);	// Timer 2 TRGO event (to start)
	ADC3->CR2 |= ADC_CR2_DMA;			// Enable DMA mode
	__HAL_RCC_DMA2_CLK_ENABLE();		// Enable Clock for DMA2
	DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA stream 3
	while (DMA2_Stream1->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
	DMA2->LIFCR |= DMA_LIFCR_CTCIF3;	// Clear transfer complete interrupt fl.
	DMA2_Stream1->CR |= (2UL << DMA_SxCR_CHSEL_Pos);	// Select channel 2
	DMA2_Stream1->CR |= DMA_SxCR_PL_1;		// Priority high
	DMA2_Stream1->CR |= DMA_SxCR_MSIZE_1;	// Memory data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_PSIZE_1;	// Peripheral data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_MINC;		// Increment memory address pointer
	DMA2_Stream1->CR |= DMA_SxCR_TCIE;		// Transfer complete interrupt enable
	DMA2_Stream1->NDTR = 4*ADC_NUMS;		// Number of data items to transfer
	DMA2_Stream1->PAR = (uint32_t)&ADC3->DR;	// Peripheral register address
	DMA2_Stream1->M0AR = (uint32_t)ADC_samples;	// Buffer memory loc. address
}

/** ***************************************************************************
 * @brief Start DMA, ADC and timer
 *****************************************************************************/
void ADC3_IN13_IN4_IN11_IN6_scan_start(void)
{
	DMA2_Stream1->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);	// Enable DMA interrupt in the NVIC
	ADC3->CR2 |= ADC_CR2_ADON;			// Enable ADC3
	TIM2->CR1 |= TIM_CR1_CEN;			// Enable timer
}

/******************************************************************************
 * initialize ADC3 and DMA2 for electrostatic inputs
 *****************************************************************************/
void ADC3_IN13_4times_scan_init(void)
{
	MEAS_input_count = 2;				// Only 1 input is converted
	__HAL_RCC_ADC3_CLK_ENABLE();		// Enable Clock for ADC2
	ADC3->SQR1 |= ADC_SQR1_L_0 | ADC_SQR1_L_2;			// Convert 2 inputs
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ1_Pos);	// Input 13 = first conversion
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ2_Pos);	// Input 13 = second conversion
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ3_Pos);	// Input 13 = third conversion
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ4_Pos);	// Input 13 = fourth conversion
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ5_Pos);	// Input 13 = fifth conversion
	ADC3->CR1 |= ADC_CR1_SCAN;			// Enable scan mode
	ADC3->CR2 |= (1UL << ADC_CR2_EXTEN_Pos);	// En. ext. trigger on rising e. (rising edge)
	ADC3->CR2 |= (6UL << ADC_CR2_EXTSEL_Pos);	// Timer 2 TRGO event (to start)
	ADC3->CR2 |= ADC_CR2_DMA;			// Enable DMA mode
	__HAL_RCC_DMA2_CLK_ENABLE();		// Enable Clock for DMA2
	DMA2_Stream2->CR &= ~DMA_SxCR_EN;	// Disable the DMA stream 3
	while (DMA2_Stream2->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
	DMA2->LIFCR |= DMA_LIFCR_CTCIF3;	// Clear transfer complete interrupt fl.
	DMA2_Stream2->CR |= (2UL << DMA_SxCR_CHSEL_Pos);	// Select channel 2
	DMA2_Stream2->CR |= DMA_SxCR_PL_1;		// Priority high
	DMA2_Stream2->CR |= DMA_SxCR_MSIZE_1;	// Memory data size = 32 bit
	DMA2_Stream2->CR |= DMA_SxCR_PSIZE_1;	// Peripheral data size = 32 bit
	DMA2_Stream2->CR |= DMA_SxCR_MINC;		// Increment memory address pointer
	DMA2_Stream2->CR |= DMA_SxCR_TCIE;		// Transfer complete interrupt enable
	DMA2_Stream2->NDTR = 5*ADC_NUMS;		// Number of data items to transfer
	DMA2_Stream2->PAR = (uint32_t)&ADC3->DR;	// Peripheral register address
	DMA2_Stream2->M0AR = (uint32_t)ADC_samples;	// Buffer memory loc. address
}

/** ***************************************************************************
 * @brief Start DMA, ADC and timer
 *****************************************************************************/
void ADC3_IN13_4times_scan_start(void)
{
	DMA2_Stream2->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);	// Enable DMA interrupt in the NVIC
	ADC3->CR2 |= ADC_CR2_ADON;			// Enable ADC3
	TIM2->CR1 |= TIM_CR1_CEN;			// Enable timer
}

/*******************************************************************************
 *
 ******************************************************************************/
void display_single_measurement(void)
{
	const uint32_t Y_OFFSET = 260;
	const uint32_t X_SIZE = 240;
	float angle_d;
	const uint32_t f = (1 << ADC_DAC_RES) / Y_OFFSET + 1;	// Scaling factor

	float input1 = maxsize(ADC_samples, ADC_NUMS,0);
	float input2 = maxsize(ADC_samples, ADC_NUMS,1);
	if(input1 > input2){angle_d = (asinf(input2/input1))*180;}
	else{angle_d = (asinf(input1/input2))*180;}
	uint32_t input3 = maxsize(ADC_samples, ADC_NUMS,2);
	uint32_t input4 = maxsize(ADC_samples, ADC_NUMS,3);
	uint32_t data;
	uint32_t data_last;
	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET+1);
	/* Write first 2 samples as numbers */
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	char text[18];
	char title[25];
	/*Electrostatic Measurement*/
	snprintf(title, 24, "Electrost. Measurem.:");
	BSP_LCD_DisplayStringAt(0, 20, (uint8_t *)title, LEFT_MODE);
	snprintf(text, 17, "Left Input  %4d", (int)(input1));
	BSP_LCD_DisplayStringAt(0, 40, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 17, "Right Input %4d", (int)(input2));
	BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 17, "angle       %4d", (int)(angle_d));
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t *)text, LEFT_MODE);
	/*Magnetic Measurement*/
	snprintf(title, 24, "Magnetic Measurement:");
	BSP_LCD_DisplayStringAt(0, 110, (uint8_t *)title, LEFT_MODE);
	snprintf(text, 17, "Left Input  %4d", (int)(input3));
	BSP_LCD_DisplayStringAt(0, 130, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 17, "Right Input %4d", (int)(input4));
	BSP_LCD_DisplayStringAt(0, 150, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 17, "Current     %4d", (int)(angle_d));
	BSP_LCD_DisplayStringAt(0, 170, (uint8_t *)text, LEFT_MODE);
	/* Draw the  values of input channel 1 as a curve */
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	data = ADC_samples[MEAS_input_count*0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (ADC_samples[MEAS_input_count*i]) / f;
		if (data > Y_OFFSET) { data = Y_OFFSET; }// Limit value, prevent crash
		BSP_LCD_DrawLine(4*(i-1), Y_OFFSET-data_last, 4*i, Y_OFFSET-data);
	}
	/* Draw the  values of input channel 2 (if present) as a curve */
	if (MEAS_input_count == 2) {
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		data = ADC_samples[MEAS_input_count*0+1] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (ADC_samples[MEAS_input_count*i+1]) / f;
			if (data > Y_OFFSET) { data = Y_OFFSET; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET-data_last, 4*i, Y_OFFSET-data);
		}
	}
	/* Clear buffer and flag */
	for (uint32_t i = 0; i < ADC_NUMS; i++){
		ADC_samples[2*i] = 0;
		ADC_samples[2*i+1] = 0;
	}
	ADC_sample_count = 0;
}



