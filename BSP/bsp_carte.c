/*********************************************************************
 * *******************************************************************
 *
 * @file	bsp_carte.c
 *
 * @author	Duclos Timothe
 *
 * @date	10/10/2014
 *
 * @brief	BSp
 *
 *********************************************************************/


/********************************************************************
 * Includes
 */
#include "bsp_carte.h"

/********************************************************************
 * Private defines
 */

/********************************************************************
 * Private macros
 */
#define __GetPeripheral(String, Periph) \
			__strncpy (String, Periph, 	4)


/********************************************************************
 * Private types
 */
/** @brief Enumeration des periph */
typedef enum {

	PERIPH_GPIOA = 0,
	PERIPH_GPIOB,
	PERIPH_GPIOC,
	PERIPH_GPIOD,
	PERIPH_GPIOE,
	PERIPH_GPIOF,
	PERIPH_GPIOG,
	PERIPH_GPIOH,
	PERIPH_GPIOI,

	nb_PERIPHS,
}Periph_e;

/********************************************************************
 * Private Variables
 */

/********************************************************************
 * Private Function Prototypes
 */

inline void 	BSP_Init_RCCPeriph	(GPIO_TypeDef* Peripheral);
inline uint32_t GetPinSource		(uint16_t GpioPin);


/********************************************************************
 * Exported Functions
 */
/**--------------------------------------------------------------------------
 *
 * @brief	Initialisation du BSP
 *
 */
void
BSP_Init(
		void
) {

	//----------------------------------------------------------------
	//---------------- Declaration des variables
	uint8_t 			b_GPIO;
	uint32_t 			SourcePin;
	GPIO_InitTypeDef 	Init_GPIO;
	GPIO_TypeDef* 		pPeripheral;

	//----------------------------------------------------------------
	//---------------- Initialisation STM32 system (clock, PLL and Flash configuration) */
	SystemInit();
	SystemCoreClockUpdate();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4 );

	//----------------------------------------------------------------
	//---------------- Init des Pins
	for(b_GPIO=1; b_GPIO<nb_GPIO ;b_GPIO++) {

		pPeripheral			= Mapping_GPIO[b_GPIO].GpioPeripheral;
		Init_GPIO.GPIO_Pin 	= Mapping_GPIO[b_GPIO].GpioPin;
		Init_GPIO.GPIO_Mode	= Mapping_GPIO[b_GPIO].GpioMode;
		Init_GPIO.GPIO_OType= Mapping_GPIO[b_GPIO].GPIO_OType;
		Init_GPIO.GPIO_PuPd	= Mapping_GPIO[b_GPIO].GPIO_PuPd;
		Init_GPIO.GPIO_Speed= Mapping_GPIO[b_GPIO].GPIO_Speed;

		BSP_Init_RCCPeriph(pPeripheral);

		if(Init_GPIO.GPIO_Mode == GPIO_Mode_AF) {

			SourcePin = GetPinSource(Mapping_GPIO[b_GPIO].GpioPin);
			GPIO_PinAFConfig(Mapping_GPIO[b_GPIO].GpioPeripheral, SourcePin, Mapping_GPIO[b_GPIO].AlternateFunction);
		}

		GPIO_Init(pPeripheral, &Init_GPIO);
	}
}


/**------------------------------------------------------------------
 *
 * @brief	Get Peripheral in string
 *
 */
void
toString_GetPeriphral(
		GPIO_TypeDef* 	Peripheral,		/**<[in] Bus GPIOx */
		uint8_t*	String				/**<[in] Buffer */
) {
	switch((uint32_t) Peripheral) {

		case (uint32_t) GPIOA:	__GetPeripheral(String, "PTA");		break;
		case (uint32_t) GPIOB:	__GetPeripheral(String, "PTB");		break;
		case (uint32_t) GPIOC:	__GetPeripheral(String, "PTC");		break;
		case (uint32_t) GPIOD:	__GetPeripheral(String, "PTD");		break;
		case (uint32_t) GPIOE:	__GetPeripheral(String, "PTE");		break;
		case (uint32_t) GPIOF:	__GetPeripheral(String, "PTF");		break;
		case (uint32_t) GPIOG:	__GetPeripheral(String, "PTG");		break;
		case (uint32_t) GPIOH:	__GetPeripheral(String, "PTH");		break;
		case (uint32_t) GPIOI:	__GetPeripheral(String, "PTI");		break;
	}
}

/**------------------------------------------------------------------
 *
 * @brief	Get Pin number in string
 *
 */
void
toString_GetPin(
		uint16_t 		GpioPin,		/**<[in] Pin dont on veut la source */
		uint8_t*		String			/**<[in] Buffer */
) {

	switch(GpioPin) {

		case (uint16_t) GPIO_Pin_0:		__strncat( String, "0", 1 );		break;
		case (uint16_t) GPIO_Pin_1:		__strncat( String, "1", 1 );		break;
		case (uint16_t) GPIO_Pin_2:		__strncat( String, "2", 1 );		break;
		case (uint16_t) GPIO_Pin_3:		__strncat( String, "3", 1 );		break;
		case (uint16_t) GPIO_Pin_4:		__strncat( String, "4", 1 );		break;
		case (uint16_t) GPIO_Pin_5:		__strncat( String, "5", 1 );		break;
		case (uint16_t) GPIO_Pin_6:		__strncat( String, "6", 1 );		break;
		case (uint16_t) GPIO_Pin_7:		__strncat( String, "7", 1 );		break;
		case (uint16_t) GPIO_Pin_8:		__strncat( String, "8", 1 );		break;
		case (uint16_t) GPIO_Pin_9:		__strncat( String, "9", 1 );		break;
		case (uint16_t) GPIO_Pin_10:	__strncat( String, "10", 2 );		break;
		case (uint16_t) GPIO_Pin_11:	__strncat( String, "11", 2 );		break;
		case (uint16_t) GPIO_Pin_12:	__strncat( String, "12", 2 );		break;
		case (uint16_t) GPIO_Pin_13:	__strncat( String, "13", 2 );		break;
		case (uint16_t) GPIO_Pin_14:	__strncat( String, "14", 2 );		break;
		case (uint16_t) GPIO_Pin_15:	__strncat( String, "15", 2 );		break;
	}
}


/********************************************************************
 * Private Function Definition
 */

/**------------------------------------------------------------------
 *
 * @brief	Initialisation des horloge des bus
 *
 */
inline void
BSP_Init_RCCPeriph(
		GPIO_TypeDef* Peripheral		/**<[in] Bus GPIOx */
) {
	static Bool_e Is_Peripheral_Initialized[nb_PERIPHS];

	switch((uint32_t) Peripheral) {

		case (uint32_t) GPIOA:	if(Is_Peripheral_Initialized[PERIPH_GPIOA] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOA] = TRUE;
					} break;


		case (uint32_t) GPIOB:	if(Is_Peripheral_Initialized[PERIPH_GPIOB] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOB] = TRUE;
					} break;

		case (uint32_t) GPIOC:	if(Is_Peripheral_Initialized[PERIPH_GPIOC] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOC] = TRUE;
					} break;

		case (uint32_t) GPIOD:	if(Is_Peripheral_Initialized[PERIPH_GPIOD] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOD] = TRUE;
					} break;

		case (uint32_t) GPIOE:	if(Is_Peripheral_Initialized[PERIPH_GPIOE] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOE] = TRUE;
					} break;

		case (uint32_t) GPIOF:	if(Is_Peripheral_Initialized[PERIPH_GPIOF] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOF] = TRUE;
					} break;

		case (uint32_t) GPIOG:	if(Is_Peripheral_Initialized[PERIPH_GPIOG] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOG] = TRUE;
					} break;

		case (uint32_t) GPIOH:	if(Is_Peripheral_Initialized[PERIPH_GPIOH] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOH] = TRUE;
					} break;

		case (uint32_t) GPIOI:	if(Is_Peripheral_Initialized[PERIPH_GPIOI] == FALSE) {
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
						Is_Peripheral_Initialized[PERIPH_GPIOI] = TRUE;
					} break;
	}
}

/**--------------------------------------------------------------------------
 *
 * @brief	Recuperation de PinSource a partir de Pin
 *
 */
inline uint32_t
GetPinSource(
		uint16_t GpioPin	/**<[in] Pin dont on veut la source */
) {

	uint32_t SourcePin;

	switch(GpioPin) {

		case GPIO_Pin_0:		SourcePin = GPIO_PinSource0;			break;
		case GPIO_Pin_1:		SourcePin = GPIO_PinSource1;			break;
		case GPIO_Pin_2:		SourcePin = GPIO_PinSource2;			break;
		case GPIO_Pin_3:		SourcePin = GPIO_PinSource3;			break;
		case GPIO_Pin_4:		SourcePin = GPIO_PinSource4;			break;
		case GPIO_Pin_5:		SourcePin = GPIO_PinSource5;			break;
		case GPIO_Pin_6:		SourcePin = GPIO_PinSource6;			break;
		case GPIO_Pin_7:		SourcePin = GPIO_PinSource7;			break;
		case GPIO_Pin_8:		SourcePin = GPIO_PinSource8;			break;
		case GPIO_Pin_9:		SourcePin = GPIO_PinSource9;			break;
		case GPIO_Pin_10:		SourcePin = GPIO_PinSource10;			break;
		case GPIO_Pin_11:		SourcePin = GPIO_PinSource11;			break;
		case GPIO_Pin_12:		SourcePin = GPIO_PinSource12;			break;
		case GPIO_Pin_13:		SourcePin = GPIO_PinSource13;			break;
		case GPIO_Pin_14:		SourcePin = GPIO_PinSource14;			break;
		case GPIO_Pin_15:		SourcePin = GPIO_PinSource15;			break;
	}

	return SourcePin;
}
