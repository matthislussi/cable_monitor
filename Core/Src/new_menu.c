/**************************************************************************************
 * single_measurement.c
 *
 *
 * Function: Frontend initialization
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

#include "menu.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
#define	MENU_OPTIONS			2
#define MENU_FONT				&Font12	///< Possible font sizes: 8 12 16 20 24
#define MENU_HEIGHT				40		///< Height of menu bar
#define MENU_MARGIN				2		///< Margin around a menu entry
/** Position of menu bar: 0 = top, (BSP_LCD_GetYSize()-MENU_HEIGHT) = bottom */
#define MENU_Y					(BSP_LCD_GetYSize()-MENU_HEIGHT)

/******************************************************************************
 * Variables
 *****************************************************************************/
static MENU_item_t MENU_selected = MENU_NONE;	///< Transition to this menu
static MENU_entry_t MENU_entry[MENU_OPTIONS] = {
		{"single",	"measurement",		LCD_COLOR_BLACK,	LCD_COLOR_LIGHTBLUE},
		{"accurate", "measurement",		LCD_COLOR_BLACK,	LCD_COLOR_LIGHTGREEN},
};

/******************************************************************************
 * Functions
 *****************************************************************************/

void menu_setup(void){
	BSP_LCD_SetFont(MENU_FONT);
	uint32_t x, y, m, w, h;
	y = MENU_Y;
	m = MENU_MARGIN;
	w = BSP_LCD_GetXSize()/MENU_OPTIONS;
	h = MENU_HEIGHT;
	for (uint32_t i = 0; i < MENU_OPTIONS; i++) {
		x = i*w;
		BSP_LCD_SetTextColor(MENU_entry[i].back_color);
		BSP_LCD_FillRect(x+m, y+m, w-2*m, h-2*m);
		BSP_LCD_SetBackColor(MENU_entry[i].back_color);
		BSP_LCD_SetTextColor(MENU_entry[i].text_color);
		BSP_LCD_DisplayStringAt(x+3*m, y+3*m,
				(uint8_t *)MENU_entry[i].line1, LEFT_MODE);
		BSP_LCD_DisplayStringAt(x+3*m, y+h/2,
				(uint8_t *)MENU_entry[i].line2, LEFT_MODE);
	}
}

void MENU_check_touch(void)
{
	static MENU_item_t item_old = MENU_NONE;
	static MENU_item_t item_new = MENU_NONE;
	static TS_StateTypeDef  TS_State;	// State of the touch controller
	BSP_TS_GetState(&TS_State);			// Get the state
#ifdef EVAL_REV_E
// Evalboard revision E (blue) has an inverted y-axis in the touch controller
	TS_State.Y = BSP_LCD_GetYSize() - TS_State.Y;	// Invert the y-axis
#endif
	if (TS_State.TouchDetected) {		// If a touch was detected
		/* Do only if last transition not pending anymore */
		if (MENU_NONE == MENU_selected) {
			item_old = item_new;		// Store old item
			/* If touched within the menu bar? */
			if ((MENU_Y < TS_State.Y) && (MENU_Y+MENU_HEIGHT > TS_State.Y)) {
				item_new = TS_State.X	// Calculate new item
						/ (BSP_LCD_GetXSize()/MENU_OPTIONS);
				if ((0 > item_new) || (MENU_OPTIONS <= item_new)) {
					item_new = MENU_NONE;	// Out of bounds
				}
				if (item_new == item_old) {	// 2 times the same menu item
					item_new = MENU_NONE;
					MENU_selected = item_old;
				}
			}
		}
	}
}

MENU_item_t MENU_get_selection(void)
{
	MENU_item_t item = MENU_selected;
	MENU_selected = MENU_NONE;
	return item;
}


