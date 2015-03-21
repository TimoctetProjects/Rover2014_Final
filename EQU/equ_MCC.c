/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	20/01/2015
 * @file	equ_MCC.c
 * @brief	Driver MCC
 *
 ***************************************************************************************************/


/***************************************************************************************************
 * Includes
 */
#include "equ_MCC.h"
#include "drv_GPIO.h"

/***************************************************************************************************
 * Private defines
 */
#define MCC_NBMCC			2
#define DELTA_VITESSE		5
#define NB_MAGNETS			4

#define MCC_PERIODE_HBRIDGE_START_us	100
#define MCC_RATIO_HBRIDGE_START_pr1000	500



/***************************************************************************************************
 * Private macros
 */
#define	__SetPeripheralString(IDpin, str) \
	( toString_GetPeriphral (Mapping_GPIO[IDpin].GpioPeripheral, 	str) )

#define	__SetPinNumberString(IDpin, str) \
	( toString_GetPin		(Mapping_GPIO[IDpin].GpioPin, 			str) )


/***************************************************************************************************
 * Private Types
 */
/** Enumeration des etapes */
typedef enum {
	MCC_initialisation = 0,
	MCC_MesureADC,
	MCC_Asservissement,
}MCC_Etapes_e;

/** Structure d'un MCC */
typedef struct {
	ADC_s 			ADC_FeedBack;

	uint32_t 		FeedBackValue_mV;
	uint32_t		VitesseRotation_us;
	uint32_t		VitesseLineaire_mm_sec;


	Mapping_GPIO_e	IDPin_HBrisge1;
	Mapping_GPIO_e	IDPin_HBrisge2;
	uint32_t		Periode_us;
	uint16_t		Ratio_pr100;
}MCC_s;

/***************************************************************************************************
 * Private Functions prototypes
 */
static void MCC_IC_IRQ1();
static void MCC_IC_IRQ2();
static void	MCC_MeasureFeedback();
static void MCC_CalculVitesse() ;

#ifdef DEBUG_ON
		static void MCC_ICValue1_toString(toString_Possibilities_e Field, Mapping_GPIO_e IDPin,	uint8_t* pString);
		static void MCC_ICValue2_toString(toString_Possibilities_e Field, Mapping_GPIO_e IDPin,	uint8_t* pString);
		static void MCC_Value_toString(toString_Possibilities_e	Field, uint8_t IDMes, uint8_t* pString);
		static void MCC_ValueVitesse_toString(toString_Possibilities_e	Field, uint8_t IDMes, uint8_t* pString);
#endif /** DEBUG_ON */

/***************************************************************************************************
 * Private variables
 */
static xSemaphoreHandle 	semLaunchFeedback, semCalculVitesse;

static MCC_s MCC[MCC_NBMCC] = {

	/** MCC A */
	{ 	.ADC_FeedBack.ID_Pin 		= FeedBack1,
		.ADC_FeedBack.StoreValue_mV = &MCC[MCC_A].FeedBackValue_mV,
		.ADC_FeedBack.ID_Mesure 	= 0,
		.ADC_FeedBack.IRQFunction	= NULL,

		.IDPin_HBrisge1 		= HA_Bridge1,
		.IDPin_HBrisge2 		= HA_Bridge2,
		.Periode_us 			= MCC_PERIODE_HBRIDGE_START_us,
		.Ratio_pr100 			= MCC_RATIO_HBRIDGE_START_pr1000	},


	/** MCC B */
	{ 	.ADC_FeedBack.ID_Pin 		= FeedBack2,
		.ADC_FeedBack.StoreValue_mV = &MCC[MCC_B].FeedBackValue_mV,
		.ADC_FeedBack.ID_Mesure 	= 0,
		.ADC_FeedBack.IRQFunction	= NULL,

		.IDPin_HBrisge1 		= HB_Bridge1,
		.IDPin_HBrisge2 		= HB_Bridge2,
		.Periode_us 			= MCC_PERIODE_HBRIDGE_START_us,
		.Ratio_pr100 			= MCC_RATIO_HBRIDGE_START_pr1000	},
};

#ifdef DEBUG_ON
static AffDebug_Ligne_s	MCC_Aff_AH1 = {
		.Nom			= "PWM HB A1 (Drv Moteur)",
		.ID  			= HA_Bridge1,
		.Previous_Value = "",
		.toString 		= (pFunction)PWM_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_AH2 = {
		.Nom			= "PWM HB A2 (Drv Moteur)",
		.ID  			= HA_Bridge2,
		.Previous_Value = "",
		.toString 		= (pFunction)PWM_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_BH1 = {
		.Nom			= "PWM HB B1 (Drv Moteur)",
		.ID  			= HB_Bridge1,
		.Previous_Value = "",
		.toString 		= (pFunction)PWM_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_BH2 = {
		.Nom			= "PWM HB B2 (Drv Moteur)",
		.ID  			= HB_Bridge2,
		.Previous_Value = "",
		.toString 		= (pFunction)PWM_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_IC1 = {
		.Nom			= "Valeur IC1 (Drv Moteur)",
		.ID  			= IC_Speed1,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_ICValue1_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_IC2 = {
		.Nom			= "Valeur IC2 (Drv Moteur)",
		.ID  			= IC_Speed2,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_ICValue2_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_FeedBack1 = {
		.Nom			= "FeedBack 1",
		.ID  			= MCC_A,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_FeedBack2 = {
		.Nom			= "FeedBack 2",
		.ID  			= MCC_B,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_Value_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_VittLin1 = {
		.Nom			= "Vit lin 1",
		.ID  			= MCC_A,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_ValueVitesse_toString,
		.toBeDisplayed 	= TRUE
};

static AffDebug_Ligne_s	MCC_Aff_VittLin2 = {
		.Nom			= "Vit lin 2",
		.ID  			= MCC_B,
		.Previous_Value = "",
		.toString 		= (pFunction)MCC_ValueVitesse_toString,
		.toBeDisplayed 	= TRUE
};

/***************************************************************************************************
 * Constructor functions
 */
__attribute__((constructor(AffDebug_Ligne(3)))) void
MCC_Affinit() {
	AffDebug_AddLine(&MCC_Aff_FeedBack1);
	AffDebug_AddLine(&MCC_Aff_FeedBack2);
	AffDebug_AddLine(&MCC_Aff_AH1);
	AffDebug_AddLine(&MCC_Aff_AH2);
	AffDebug_AddLine(&MCC_Aff_BH1);
	AffDebug_AddLine(&MCC_Aff_BH2);
	AffDebug_AddLine(&MCC_Aff_IC1);
	AffDebug_AddLine(&MCC_Aff_IC2);
	AffDebug_AddLine(&MCC_Aff_VittLin1);
	AffDebug_AddLine(&MCC_Aff_VittLin2);
}

#endif /** DEBUG_ON */

/***************************************************************************************************
 * Exported Functions definition
 */
void
MCC_init() {
	uint8_t  b_MCC;

	for(b_MCC=0; b_MCC<MCC_NBMCC; b_MCC++) {

		PWM_Init( (PWM_init_s) {	MCC[b_MCC].IDPin_HBrisge1,
									MCC[b_MCC].Periode_us,
									MCC[b_MCC].Ratio_pr100 } 	);

		PWM_Init( (PWM_init_s) {	MCC[b_MCC].IDPin_HBrisge2,
								 	MCC[b_MCC].Periode_us,
								 	MCC[b_MCC].Ratio_pr100	}	);

		ADC_PeriphConfigure(&MCC[b_MCC].ADC_FeedBack);
	}

	InputCapture_init((InputCapture_init_s){IC_Speed1, 600000, (pFunctionVide)&MCC_IC_IRQ1} );
	InputCapture_init((InputCapture_init_s){IC_Speed2, 600000, (pFunctionVide)&MCC_IC_IRQ2} );


	xTaskCreate( 	MCC_MeasureFeedback,
					(const char * const )"MesFeedBack",
					configMINIMAL_STACK_SIZE,
					NULL,
					2,
					NULL	);

	xTaskCreate( 	MCC_CalculVitesse,
					(const char * const )"CalcVitLin",
					configMINIMAL_STACK_SIZE,
					NULL,
					2,
					NULL	);

	semLaunchFeedback 	= xSemaphoreCreateBinary();
	semCalculVitesse 	= xSemaphoreCreateBinary();
}

void MCC_GoForward(void) {
	GPIO_Set(MCC_Enable, ETAT_ACTIF);
	PWM_Activer(MCC[MCC_A].IDPin_HBrisge1);
	PWM_Activer(MCC[MCC_B].IDPin_HBrisge2);
	PWM_Desactiver(MCC[MCC_A].IDPin_HBrisge2);
	PWM_Desactiver(MCC[MCC_B].IDPin_HBrisge1);
}

void MCC_GoBackward(void) {
	GPIO_Set(MCC_Enable, ETAT_ACTIF);
	PWM_Activer(MCC[MCC_A].IDPin_HBrisge2);
	PWM_Activer(MCC[MCC_B].IDPin_HBrisge1);
	PWM_Desactiver(MCC[MCC_A].IDPin_HBrisge1);
	PWM_Desactiver(MCC[MCC_B].IDPin_HBrisge2);
}

void MCC_Stop(void) {
	GPIO_Set(MCC_Enable, ETAT_INACTIF);
	PWM_Desactiver(MCC[MCC_A].IDPin_HBrisge1);
	PWM_Desactiver(MCC[MCC_B].IDPin_HBrisge2);
	PWM_Desactiver(MCC[MCC_B].IDPin_HBrisge2);
	PWM_Desactiver(MCC[MCC_B].IDPin_HBrisge1);
}

void MCC_HardStop(void) {
	GPIO_Set(MCC_Enable, ETAT_INACTIF);
	PWM_Activer(MCC[MCC_A].IDPin_HBrisge1);
	PWM_Activer(MCC[MCC_B].IDPin_HBrisge2);
	PWM_Activer(MCC[MCC_B].IDPin_HBrisge2);
	PWM_Activer(MCC[MCC_B].IDPin_HBrisge1);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture de l'adresse de la semaphore utile aux mesures de courant de feedback
 */
xSemaphoreHandle*
MCC_GetSem(void) {
	return &semLaunchFeedback;
}

/***************************************************************************************************
 * Private Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Sequencement des mesures de courant de feedback
 */
static void
MCC_MeasureFeedback(void) {

	static uint8_t l_IDmotor = 0;

	for(;;) {
		xSemaphoreTake (semLaunchFeedback, portMAX_DELAY);
		ADC_StartConv	(MCC[l_IDmotor++].ADC_FeedBack);
		if(l_IDmotor == 2) {
			l_IDmotor = 0;
			xSemaphoreGive(semCalculVitesse);
		}
	}
}
#define DIAMETREROUE_mm		40
#define PI					3.14159265359
/**-------------------------------------------------------------------------------------------------
 * @brief	Calcul de la vitesse lineaire
 */
static void
MCC_CalculVitesse() {

	for(;;) {
		xSemaphoreTake (semCalculVitesse, portMAX_DELAY);
		MCC[MCC_A].VitesseLineaire_mm_sec = (DIAMETREROUE_mm * PI * 1000000) / MCC[MCC_A].VitesseRotation_us;
		MCC[MCC_B].VitesseLineaire_mm_sec = (DIAMETREROUE_mm * PI * 1000000) / MCC[MCC_B].VitesseRotation_us;
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Fonction a lancer dans l'IRQ InputCapture
 */
static void
MCC_IC_IRQ1(){

	static 	uint8_t nb_IT = 0;
	static 	uint32_t  capture_value_us[NB_MAGNETS] 	= {0};

	switch(nb_IT) {

		case 0:
			TIM_SetCounter(TIM5, 0);
			nb_IT++;
			break;

		case 1:
			capture_value_us[0]			 = TIM_GetCounter(TIM5);
			TIM_SetCounter(TIM5, 0);
			nb_IT++;
			break;

		case 2:
			capture_value_us[1] 			 = TIM_GetCounter(TIM5);
			TIM_SetCounter(TIM5, 0);
			nb_IT++;
			break;

		case 3:
			capture_value_us[2] 			 = TIM_GetCounter(TIM5);
			TIM_SetCounter(TIM5, 0);
			nb_IT++;
			break;

		default: nb_IT = 0;
			capture_value_us[3] 			 = TIM_GetCounter(TIM5);
			TIM_SetCounter(TIM5, 0);
			MCC[0].VitesseRotation_us 	 = capture_value_us[0]
										 + capture_value_us[1]
										 + capture_value_us[2]
			 	 	 	 	 	 	 	 + capture_value_us[3];
			break;
	}
}

static void
MCC_IC_IRQ2(){

	static 	uint8_t nb_IT = 0;
	static 	uint32_t capture_value_us[NB_MAGNETS] = {0};

	switch(nb_IT) {
		case 0:
			nb_IT++;
			TIM_SetCounter(TIM4, 0);
			break;

		case 1:
			capture_value_us[0]  = TIM_GetCounter(TIM4);
			TIM_SetCounter(TIM4, 0);
			nb_IT++;
			break;

		case 2:
			capture_value_us[1]  = TIM_GetCounter(TIM4);
			TIM_SetCounter(TIM4, 0);
			nb_IT++;
			break;

		case 3:
			capture_value_us[2]  = TIM_GetCounter(TIM4);
			TIM_SetCounter(TIM4, 0);
			nb_IT++;
			break;

		default: nb_IT = 0;
			capture_value_us[3]  = TIM_GetCounter(TIM4);
			TIM_SetCounter(TIM4, 0);
			MCC[1].VitesseRotation_us 	= capture_value_us[0]
										+ capture_value_us[1]
										+ capture_value_us[2]
									    + capture_value_us[3];
			break;
	}
}

#ifdef DEBUG_ON

/**-------------------------------------------------------------------------------------------------
 * @brief	Affichage debug ligne ADC
 */
static void
MCC_ValueVitesse_toString(
		toString_Possibilities_e	Field,
		uint8_t 					IDMes,
		uint8_t*					pString
) {
	switch(Field) {

		//------------------------------------------------------------
		case toString_Getpin:
			break;

		//------------------------------------------------------------
		case toString_GetValue:
			snprintf((char *)pString, 10, "%d mm/s", (int)MCC[IDMes].VitesseLineaire_mm_sec);
			break;

		//------------------------------------------------------------
		default:
			break;
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Affichage debug ligne ADC
 */
static void
MCC_Value_toString(

		toString_Possibilities_e	Field,
		uint8_t 					IDMes,
		uint8_t*					pString
) {

	if(IDMes > 2)	return;

	switch(Field) {

		//------------------------------------------------------------
		case toString_Getpin:
			if(IDMes) {
				__SetPeripheralString	(FeedBack1, pString);
				__SetPinNumberString	(FeedBack1, pString);
			} else {
				__SetPeripheralString	(FeedBack2, pString);
				__SetPinNumberString	(FeedBack2, pString);

			} break;

		//------------------------------------------------------------
		case toString_GetValue:
			snprintf((char *)pString, 10, "%d mV", (int)MCC[IDMes].FeedBackValue_mV);
			break;

		//------------------------------------------------------------
		default:
			break;
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Affichage debug ligne ADC
 */
static void
MCC_ICValue1_toString(

		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {

	switch(Field) {

		//------------------------------------------------------------
		case toString_Getpin:
				__SetPeripheralString	(IDPin, pString);
				__SetPinNumberString	(IDPin, pString);
				break;

		//------------------------------------------------------------
		case toString_GetValue:	snprintf((char *)pString, 15, "%d uS", (int)(MCC[0].VitesseRotation_us));
					break;

		//------------------------------------------------------------
		default:		break;
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Affichage debug ligne ADC
 */
static void
MCC_ICValue2_toString(

		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {

	switch(Field) {

		//------------------------------------------------------------
		case toString_Getpin:
				__SetPeripheralString	(IDPin, pString);
				__SetPinNumberString	(IDPin, pString);
				break;

		//------------------------------------------------------------
		case toString_GetValue:	snprintf((char *)pString, 15, "%d uS", (int)(MCC[1].VitesseRotation_us));
					break;

		//------------------------------------------------------------
		default:		break;
	}
}
#endif /** DEBUG_ON */
