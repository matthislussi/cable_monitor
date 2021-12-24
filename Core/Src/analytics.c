/**************************************************************************************
 * measure_options.c
 *
 *
 * Function: Any kind of analytical function to be included in this file
 *
 *
 *  Created on: 04.12.2021
 *      Author: matth
 *************************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "measuring.h"
#include "analytics.h"
#include "math.h"



/******************************************************************************
 * Defines
 *****************************************************************************/
#define ADC_NUMS		60			///< Number of samples
#define MEAS_INPUTS		4			///< Number of inputs to be measured

/******************************************************************************
 * Variables
 *****************************************************************************/
const double pi = M_PI;
const double u0 = 4*pi/(powf(10, 7));  //permeability of free space
static RESULT_item_t an_result;
static MAXVAL_item_t peak;

uint32_t ADC_mean[MEAS_INPUTS*ADC_NUMS];

/******************************************************************************
 * Functions
 *****************************************************************************/

/******************************************************************************
 * standard_deviation: With the ADC_value array and the length of it
 * 					   the standard deviation gets determined
 *****************************************************************************/
static void standard_deviation(uint32_t ADC_values[], uint32_t length){
	uint32_t sum = 0;
	//float mean = mean_val(ADC_values, length);

	length += 1;
	for(int i = 0; i < length; i++){
		sum	+= powf(mean - ADC_values[i],2);
	}
	an_result.strd_deviation = sqrtf(sum/length);
}

/*****************************************************************************
 * angle: needs both distances of the electrostatic/magnetic
 * 		  measurements handed over
 *****************************************************************************/
static void angle(float distance1, float distance2){
	float angle_val;
	const float dist_pads = 7;

	if(distance1 > distance2){
		angle_val = acosf((sqrt(2*powf(distance1,4)
								-powf(distance1,2)*powf(dist_pads,2)
								-2*powf(distance2,2)*powf(distance1,2)
								+distance1*dist_pads))/(2*powf(distance1,2)));
	} else {
		angle_val = acosf((sqrt(2*powf(distance2,4)
								-powf(distance2,2)*powf(dist_pads,2)
								-2*powf(distance1,2)*powf(distance2,2)
								+distance2*dist_pads))/(2*powf(distance2,2)));
	}
	an_result.angle = angle_val;
}

/*****************************************************************************
 * current: determines current which flows through cable
 * 	B: magnetic field strength
 * 	distance: from cable to board
 *****************************************************************************/
static void current(uint32_t B, uint32_t distance)
{
	const double u0 = 4*pi/(powf(10, 7));  //permeability of free space
	an_result.current = (2*pi*distance*B)/u0;
}

/*****************************************************************************
 * @brief updates the struct peak with the maximal values of all 4 Inputs
 *
 * The inputs used are: ADC123_IN13 = GPIO PC3 (pad_l), ADC3_IN4 = GPIO PF6 (pad_r)
 * 	                    ADC123_IN11 = GPIO PC1 (halsens_l) and ADC3_IN6 = GPIO PF8 (halsens_r)
 *
 * Order of Inputs red by ADC: IN13 = 0, IN4 = 1, IN11 = 2, IN6 = 3
 *****************************************************************************/
static void get_max_values(void)
{
	uint32_t maxval[4];
	for(uint32_t i = 0 ; i <= MEAS_INPUTS ; i++ ){
		maxval[i] = ADC_mean[MEAS_INPUTS*0+i];
		for(uint32_t u = 0; u <= ADC_NUMS; u++){
			if(maxval[i] < ADC_mean[MEAS_INPUTS*u+i]){
				maxval[i] = ADC_mean[MEAS_INPUTS*u+i];
			}
		}
	}
	peak.pad_l = maxval[0];
	peak.pad_r = maxval[1];
	peak.halsens_l = maxval[2];
	peak.halsens_r = maxval[3];
}

/** ***************************************************************************
 * @brief add up arrays to average them in the end
 *****************************************************************************/
void samples_add(uint32_t ADC_measurement[]){
	for(uint32_t i = 0 ; i < (MEAS_INPUTS*ADC_NUMS)-1 ; i++){
		ADC_mean[i] += ADC_measurement[i];
	}
}

/** ***************************************************************************
 * @brief averages the samples
 *****************************************************************************/
uint32_t* samples_mean(void){
	for(uint32_t i = 0 ; i < (MEAS_INPUTS*ADC_NUMS)-1 ; i++){
		ADC_mean[i] = ADC_mean[i] / SEQUENZ_MAX;
	}
	return ADC_mean;
}

/** ***************************************************************************
 * @brief averages the samples
 *****************************************************************************/
RESULT_item_t get_Values(void){
	return an_result;
}



