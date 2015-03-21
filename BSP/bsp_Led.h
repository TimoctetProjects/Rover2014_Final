/*********************************************************************
 * *******************************************************************
 *
 * @file	bsp_Led.h
 *
 * @author	Duclos Timothe
 *
 * @date	19/07/2014
 *
 * @brief	Gestion des Leds
 *
 *********************************************************************/

#ifndef BSP_LED_H
#define BSP_LED_H


/********************************************************************
 * Includes
 */
#include "drv_GPIO.h"
#include "util_TSW.h"
#include "util_Console.h"

/********************************************************************
 * Exported defines
 */

/********************************************************************
 * Exported variables
 */

/********************************************************************
 * Exported types
 */

/** Enumeration des leds */
typedef enum {

	LED_LifeBit = 0,

	nb_LEDS,
	Led_err_Innexistant,

}Led_e;

/** Enumeration des etats des Leds */
typedef enum {

	LED_STATE_OFF = 0,
	LED_STATE_ON,
	LED_STATE_BLINKING,
	LED_STATE_UNDEFINED,
	nb_STATE_LEDSTATES,
}LedState_e;

/** Enumeration des etapes des Leds */
typedef enum {

	BLINKING_ETAT_HAUT = 0,
	BLINKING_ETAT_BAS,
}Etape_Blinking_e;


/********************************************************************
 * Exported Functions
 */
void 		Led_Init				(void);
void 		Led_Main				(void);
uint8_t 	Led_SetState			(Led_e Led_ID, LedState_e State);
uint8_t 	Led_SetBlinking_1Hz		(Led_e Led_ID);
uint8_t 	Led_SetBlinking_2Hz		(Led_e Led_ID);
void 		Led_RaZ_Blinking_Conf	(Led_e Led_ID);
uint8_t 	Led_SetPeriod_ms		(Led_e Led_ID, uint32_t Period_ms);
uint8_t 	Led_SetRatio_pr100		(Led_e Led_ID, uint32_t Ratio_pr100);
LedState_e 	Led_GetState			(Led_e Led_ID);
Mapping_GPIO_e Led_Get_IDPin		(Led_e ID_Led);

void Led_Value_toString(toString_Possibilities_e Field,	Led_e ID_Led, uint8_t* pString);


#endif // BSP_LED_H
