/**************************************************************************************
 * analytics.c
 *
 *
 * Function: the interface of analytics.c
 *
 *
 *  Created on: 04.12.2021
 *      Author: matth
 *************************************************************************************/

#ifndef SRC_ANALYTICS_H_
#define SRC_ANALYTICS_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "stdint.h"


/******************************************************************************
 * Defines
 *****************************************************************************/



/******************************************************************************
 * Types
 *****************************************************************************/
typedef struct output {
	uint32_t distance;
	uint32_t mean_distance;
	uint32_t strd_deviation;
	uint32_t current;
	float angle;
} RESULT_item_t;

typedef struct max_values {
	uint32_t pad_l;
	uint32_t pad_r;
	uint32_t halsens_l;
	uint32_t halsens_r;
} MAXVAL_item_t;

/******************************************************************************
 * Functions
 *****************************************************************************/
extern void samples_add(uint32_t ADC_measurement[]);
extern uint32_t* samples_mean(void);
extern RESULT_item_t get_Values(void);
static void get_max_values(void);

#endif /* SRC_ANALYTICS_H_ */
