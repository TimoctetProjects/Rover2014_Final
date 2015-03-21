/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	09/12/2014
 * @file	equ_Bargraph.c
 * @brief	Gestion du bargraph refletant du niveau de charge de la batterie
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "equ_Bargraph.h"
#include "equ_MCC.h"

/***************************************************************************************************
 * Private defines
 */
#define VBATT_SEUIL_4		1724
#define VBATT_SEUIL_3		1699
#define VBATT_SEUIL_2		1674
#define VBATT_SEUIL_1		1649

/***************************************************************************************************
 * Private types
 */
typedef struct {
	uint32_t			VBatt_mV;
	xSemaphoreHandle 	semGestionLed;
	xSemaphoreHandle 	semLaunchMesBatt;
	ADC_s				Bargraph_ADC_VBatt;
}Bargraph_s;

/***************************************************************************************************
 * Private function prototypes
 */
static void Bargraph_IRQADCVBatt	(void* pValue_mV);
static void Bargraph_LaunchMesBatt	(void* pParam);
static void Bargraph_GestionLed		(void* pParam);

#ifdef DEBUG_ON
void VBatt_toString	(toString_Possibilities_e Field, Mapping_GPIO_e IDPin, uint8_t* pString);
#endif /** DEBUG_ON */

/***************************************************************************************************
 * Private variables
 */
static Bargraph_s Bargraph = {

		.semGestionLed		= NULL,

		.Bargraph_ADC_VBatt = {

				.ID_Pin			= ADCVBatt,
				.StoreValue_mV	= NULL,
				.IRQFunction	= (pFunction)&Bargraph_IRQADCVBatt
		}
};

#ifdef DEBUG_ON
static AffDebug_Ligne_s CommandeAff = {
		.Nom 			= "Niveau VBatt",
		.ID 			= ADCVBatt,
		.Previous_Value = "",
		.toString 		= (pFunction)&VBatt_toString,
		.toBeDisplayed 	= TRUE,
};

/***************************************************************************************************
 * Constructor functions
 */
__attribute__((constructor(AffDebug_Ligne(3)))) void
AffVBatt_init() {
	AffDebug_AddLine(&CommandeAff);
}
#endif /** DEBUG_ON */

/***************************************************************************************************
 * Exported functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Initialisation du bargraph
 */
void
Bargraph_init() {

	ADC_PeriphConfigure(&Bargraph.Bargraph_ADC_VBatt);

	xTaskCreate( 	Bargraph_LaunchMesBatt,
					(const char * const )"LaunchMesBatt",
					configMINIMAL_STACK_SIZE,
					NULL,
					1,
					NULL	);

	xTaskCreate( 	Bargraph_GestionLed,
					(const char * const )"GestionLed",
					configMINIMAL_STACK_SIZE,
					NULL,
					1,
					NULL	);

	Bargraph.semGestionLed 		= xSemaphoreCreateBinary();
	Bargraph.semLaunchMesBatt 	= xSemaphoreCreateBinary();
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Autorisation de la mesure de la batterie
 */
void
Bargraph_EnableMesBatt() {

}

xSemaphoreHandle*
Bargraph_GetSem(void) {
	return &Bargraph.semLaunchMesBatt;
}
/***************************************************************************************************
 * Private functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Lancement de la mesure batterie
 */
static void
Bargraph_LaunchMesBatt(void* pParam) {

	for(;;) {
		xSemaphoreTake  (Bargraph.semLaunchMesBatt, portMAX_DELAY);
		ADC_StartConv	(Bargraph.Bargraph_ADC_VBatt);
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	IRQ ADC
 */
static void
Bargraph_IRQADCVBatt(
		void* pValue_mV		/**<[in] Valeur convertie */
) {

	signed long pxHigherPriorityTaskWoken = 0;
	Bargraph.VBatt_mV = *(uint32_t*)pValue_mV;
	xSemaphoreGiveFromISR(Bargraph.semGestionLed, &pxHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Gestion des Leds
 */
static void Bargraph_GestionLed	(void* pParam) {
	SemaphoreHandle_t* semMCC = MCC_GetSem();

	for(;;) {
		xSemaphoreTake (Bargraph.semGestionLed, portMAX_DELAY);
		if(Bargraph.VBatt_mV > VBATT_SEUIL_3 && Bargraph.VBatt_mV < VBATT_SEUIL_4) {
				GPIO_Set(LED_75_100, 	ETAT_ACTIF);
				GPIO_Set(LED_50_75, 	ETAT_ACTIF);
				GPIO_Set(LED_25_50, 	ETAT_ACTIF);
				GPIO_Set(LED_0_25, 		ETAT_ACTIF);

		} else 	if(Bargraph.VBatt_mV > VBATT_SEUIL_2 && Bargraph.VBatt_mV < VBATT_SEUIL_3) {
				GPIO_Set(LED_75_100, 	ETAT_INACTIF);
				GPIO_Set(LED_50_75, 	ETAT_ACTIF);
				GPIO_Set(LED_25_50, 	ETAT_ACTIF);
				GPIO_Set(LED_0_25, 		ETAT_ACTIF);

		} else 	if(Bargraph.VBatt_mV > VBATT_SEUIL_1 && Bargraph.VBatt_mV < VBATT_SEUIL_2) {
				GPIO_Set(LED_75_100, 	ETAT_INACTIF);
				GPIO_Set(LED_50_75, 	ETAT_INACTIF);
				GPIO_Set(LED_25_50, 	ETAT_ACTIF);
				GPIO_Set(LED_0_25, 		ETAT_ACTIF);

		} else {
				GPIO_Set(LED_75_100, 	ETAT_INACTIF);
				GPIO_Set(LED_50_75, 	ETAT_INACTIF);
				GPIO_Set(LED_25_50, 	ETAT_INACTIF);
				GPIO_Set(LED_0_25, 		ETAT_ACTIF);
		}

		xSemaphoreGive (*semMCC);
	}
}

#ifdef DEBUG_ON
void
VBatt_toString	(
		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {
	switch(Field) {
		case toString_Getpin:	toString_GetPeriphral	(Mapping_GPIO[IDPin].GpioPeripheral,pString);
								toString_GetPin			(Mapping_GPIO[IDPin].GpioPin, 		pString);
								break;
		case toString_GetValue:	snprintf((char *)pString, 10, "%d", (int)(Bargraph.VBatt_mV));
								break;
	}
}
#endif /** DEBUG_ON */
