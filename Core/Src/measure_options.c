/**************************************************************************************
 * measure_options.c
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
#include "stdio.h"
#include "stdlib.h"
#include <math.h>



/******************************************************************************
 * Defines
 *****************************************************************************/


/******************************************************************************
 * Variables
 *****************************************************************************/
const double pi = M_PI;
const double u0 = 4*pi/(powf(10, 7));  //permeability of free space

/******************************************************************************
 * Functions
 *****************************************************************************/

/******************************************************************************
 * mean_val: Determines the mean value of an array
 *****************************************************************************/
uint32_t mean_val(uint32_t ADC_values[], uint32_t length){
	uint32_t mean[60];

		for(int i = 0; i <= (length-1); i++){
			mean[i] = (ADC_values[i*5]+
					  ADC_values[i*5+1]+
					  ADC_values[i*5+2]+
					  ADC_values[i*5+3]+
					  ADC_values[i*5+4])/5;
		}
	return mean;
}

/******************************************************************************
 * standard_deviation: With the ADC_value array and the length of it
 * 					   the standard deviation gets determined
 *****************************************************************************/
float standard_deviation(uint32_t ADC_values[], uint32_t length){
	uint32_t sum = 0;
	float mean = mean_val(ADC_values, length);

	length += 1;
	for(int i = 0; i < length; i++){
		sum	+= powf(mean - ADC_values[i],2);
	}
	return (sqrtf(sum/length));
}

/*****************************************************************************
 * angle: needs both distances of the electrostatic/magnetic
 * 		  measurements handed over
 *****************************************************************************/
float angle(float distance1, float distance2){
	float angle_val;
	const float dist_pads = 7;

	if(distance1 > distance2){
		angle_val = acosf((sqrt(2*powf(distance1,4)
								-powf(distance1,2)*powf(dist_pads,2)
								-2*powf(distance2,2)*powf(distance1,2)
								+distance1*dist_pads))/(2*powf(distance1,2)));
	}else
	{
		angle_val = acosf((sqrt(2*powf(distance2,4)
								-powf(distance2,2)*powf(dist_pads,2)
								-2*powf(distance1,2)*powf(distance2,2)
								+distance2*dist_pads))/(2*powf(distance2,2)));
	}
	return angle_val;
}

/*****************************************************************************
 * current: determines current which flows through cable
 * 	B: magnetic field strength
 * 	distance: from cable to board
 *****************************************************************************/
uint32_t current(uint32_t B, uint32_t distance)
{
	const double u0 = 4*pi/(powf(10, 7));  //permeability of free space
	return(2*pi*distance*B/u0);
}

/*****************************************************************************
 * maxsize: determines biggest value in handed over array
 *****************************************************************************/
uint32_t maxsize(uint32_t array[], uint32_t length, uint32_t offset)
{
	uint32_t max_val = array[4*0+offset];
	for(int i = 1; i <= length; i++){
		if(max_val < array[4*i+offset]){max_val = array[4*i+offset];}
	}
	return max_val;
}





