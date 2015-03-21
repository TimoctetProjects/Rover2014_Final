/*********************************************************************
 * *******************************************************************
 *
 * @file	bsp_Led.c
 *
 * @author	Duclos Timothe
 *
 * @date	19/07/2014
 *
 * @brief	Gestion des Leds
 *
 *********************************************************************/

/********************************************************************
 * Includes
 */
#include "bsp_Led.h"

/********************************************************************
 * Private defines
 */
#define BLINKING_STRING			"Blinking"

/********************************************************************
 * Private macros
 */
#define __Led_IsIDValid(id)	\
	( 	(id >= 0) && (id < nb_LEDS) 	)

#define __Led_IsStateOFF(id) \
	(	Led[id].State == LED_STATE_OFF	)

#define __Led_IsCalculationDone(id) \
	(	Led[id].Blinking_Config.Calculation_Done == FALSE	)

#define __Led_IsStateToBeChanged(state, id) \
	( 	(state >= 0) &&  (state < nb_STATE_LEDSTATES) &&  (Led[id].State != state) 	)

#define	__Led_SetOnString(String) \
	__strncat	( String, ON_STRING		)

#define	__Led_SetOffString(String) \
	__strncat	( String, OFF_STRING		)

#define	__Led_SetBlinkingString(String)	\
	__strncat	( String, BLINKING_STRING	)

#define __GetFrequency(str, T)	\
	__snprintf( str, 	 NB_CHAR_FREQUENCY, 	" %d", 		(int)(1000 / T)	)

#define __SetFrequency(str1, str2)	\
	__strncat	( str1,	 str2, 		NB_CHAR_FREQUENCY		)


/********************************************************************
 * Private types
 */
/** Structure de Configuration du Blinking */
typedef struct {

	uint32_t			Period_ms;		/** Periode de clignotement. */
	uint32_t			Ratio_pr100;		/** Ration du clignotement. */

	Etape_Blinking_e 	Etape;			/** Etape de blinking */

	Bool_e 				Calculation_Done;	/** Flag signifiant que le calcul est fait. */

	uint32_t			Duree_Etat_Haut_ms;	/** Durre de l'etat haut. */
	uint32_t			Duree_Etat_Bas_ms;	/** Durre de l'etat bas. */

}Led_Bliking_Conf_s;

/** Structure objet Led */
typedef struct {

	LedState_e 			State;			/** Etat de la Led. */

	Led_Bliking_Conf_s 	Blinking_Config;	/** Configuration du Blinking. */

	uint8_t				ID_PinMapping;		/** Identifiant de la Pin. */
	TSW_s				Timer;			/** Identifiant du Timer. */

}Led_s;

/********************************************************************
 * Private variables
 */

/** Objets Led*/
static Led_s Led[nb_LEDS] = {

		//---------------------------------------------------------------------------------------------------
		//----------------------------------------- Init des Leds
		// Led Life Bit
		{	.State = LED_STATE_OFF, 			.ID_PinMapping = LED_GREEN,	.Timer = {0},
			.Blinking_Config.Etape = BLINKING_ETAT_HAUT, 	.Blinking_Config.Calculation_Done = FALSE,		},

		/*// Led Error
		{	.State = LED_STATE_OFF, 	.ID_PinMapping = LED_RED,	.Timer = TIMER_Led_Eroor,
			.Blinking_Config.Etape = BLINKING_ETAT_HAUT, 	.Blinking_Config.Calculation_Done = FALSE,		},

		// Led Lambda
		{	.State = LED_STATE_OFF, 	.ID_PinMapping = LED_ORANGE,	.Timer = TIMER_Led_Lambda,
			.Blinking_Config.Etape = BLINKING_ETAT_HAUT, 	.Blinking_Config.Calculation_Done = FALSE,		},
*/};


/********************************************************************
 * Private Functions prototypes
 */
inline void Gestion_Led_Blinking(Led_e Led_ID);
inline void Gestion_Led			(Led_e Led_ID);

/********************************************************************
 * Exported Functions
 */
/**--------------------------------------------------------------------
 *
 * @brief	Led init
 *
 * @return	void
 *
 */
void
Led_Init() {

	uint8_t b_GInit_Led;

	for(b_GInit_Led=0; b_GInit_Led<nb_LEDS; b_GInit_Led++) {

		Led[b_GInit_Led].State 			= LED_STATE_OFF;
		Led_RaZ_Blinking_Conf(b_GInit_Led);
	}
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant d'ecrire l'etat d'une Led
 *
 * @return	void
 *
 */
uint8_t
Led_SetState(
		Led_e		Led_ID,	/**<[in] Identifiant de la Led */
		LedState_e 	State	/**<[in] Nouvel etat de la Led */
) {

	if __Led_IsStateToBeChanged(State, Led_ID) {

		Led[Led_ID].State = State;
		Led[Led_ID].Blinking_Config.Calculation_Done = FALSE;

		if(__Led_IsStateOFF(Led_ID))
			Led_RaZ_Blinking_Conf(Led_ID);

	} else 	return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de mettre la periode de clignotement a 1Hz
 *
 * @return	void
 *
 */
uint8_t
Led_SetBlinking_1Hz(
		Led_e		Led_ID	/**<[in] Identifiant de la Led */
)   {
	if(__Led_IsIDValid(Led_ID)) {

			Led[Led_ID].State							= LED_STATE_BLINKING;
			Led[Led_ID].Blinking_Config.Period_ms 		= 1000;
			Led[Led_ID].Blinking_Config.Ratio_pr100 	= 50;
			Led[Led_ID].Blinking_Config.Calculation_Done= FALSE;

	} else 	return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de mettre la periode de clignotement a 2Hz
 *
 * @return	void
 *
 */
uint8_t
Led_SetBlinking_2Hz(
		Led_e		Led_ID	/**<[in] Identifiant de la Led */
)  {

	if(__Led_IsIDValid(Led_ID)) {

			Led[Led_ID].State 							= LED_STATE_BLINKING;
			Led[Led_ID].Blinking_Config.Period_ms 		= 500;
			Led[Led_ID].Blinking_Config.Ratio_pr100 	= 50;
			Led[Led_ID].Blinking_Config.Calculation_Done= FALSE;

	} else 		return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	RaZ de la confguration Blinking
 *
 */
void
Led_RaZ_Blinking_Conf(
		Led_e		Led_ID	/**<[in] Identifiant de la Led */
)  {

	Led[Led_ID].Blinking_Config.Period_ms 			= 0;
	Led[Led_ID].Blinking_Config.Ratio_pr100 		= 0;
	Led[Led_ID].Blinking_Config.Etape 				= BLINKING_ETAT_HAUT;
	Led[Led_ID].Blinking_Config.Calculation_Done 	= FALSE;
	Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms 	= 0;
	Led[Led_ID].Blinking_Config.Duree_Etat_Bas_ms 	= 0;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de setter la periode de clignotement
 * 			d'une Led
 *
 * @return	void
 *
 */
uint8_t
Led_SetPeriod_ms(
		Led_e		Led_ID,		/**<[in] Identifiant de la Led */
		uint32_t 	Period_ms	/**<[in] Periode de clignotement */
) {

	if(__Led_IsIDValid(Led_ID)) {
		Led[Led_ID].State = LED_STATE_BLINKING;
		Led[Led_ID].Blinking_Config.Period_ms = Period_ms;

	}  else return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de setter le ratio de clignotement
 * 			d'une Led
 *
 * @return	void
 *
 */
uint8_t
Led_SetRatio_pr100(
		Led_e		Led_ID,		/**<[in] Identifiant de la Led */
		uint32_t 	Ratio_pr100	/**<[in] Ratio de clignotement */
)  {

	if(__Led_IsIDValid(Led_ID)) {
		Led[Led_ID].State = LED_STATE_BLINKING;
		Led[Led_ID].Blinking_Config.Ratio_pr100 = Ratio_pr100;
	}  else return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de lire l'etat d'une Led
 *
 * @return	etat de la Led
 *
 */
LedState_e
Led_GetState(
		Led_e		Led_ID	/**<[in] Identifiant de la Led */
)  {

	return Led[Led_ID].State;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de lire la conf d'une Led
 *
 * @return	ID de la Led si OK
 *
 */
uint8_t
Led_GetConf(
		Led_e		Led_ID,		/**<[in] Identifiant de la Led */
		Led_s*		LedConf		/**<[out]Conf de la Led*/
) {

	if(__Led_IsIDValid(Led_ID)) {
		*LedConf = Led[Led_ID];
	}  else return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de lire la conf de clignotement
 *
 * @return	ID de la Led si OK
 *
 */
uint8_t
Led_GetBlinkingConf(
		Led_e				Led_ID,			/**<[in] Identifiant de la Led */
		Led_Bliking_Conf_s*	BlinlingConf	/**<[out]Conf blink */
) {

	if(__Led_IsIDValid(Led_ID)) {
		*BlinlingConf = Led[Led_ID].Blinking_Config;
	}  else return Led_err_Innexistant;
	return Led_ID;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction permettant de piloter les Leds
 *
 * @return	void
 *
 */
void
Led_Main() {

	uint8_t b_Gestion_Led;

	for(b_Gestion_Led=0; b_Gestion_Led<nb_LEDS; b_Gestion_Led++) {
		Gestion_Led(b_Gestion_Led);
	}
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction de lire l'ID du mapping de l'une LED
 *
 * @return	void
 *
 */
Mapping_GPIO_e
Led_Get_IDPin(
		Led_e ID_Led	/**<[in] ID de la Led */
) {

	if(__Led_IsIDValid(ID_Led)) {
		return Led[ID_Led].ID_PinMapping;
	}	return 0;
}

/**--------------------------------------------------------------------
 *
 * @brief	Fonction transforman l'etat en string
 *
 * @return	void
 *
 */
/*void
Led_Value_toString(

		toString_Possibilities_e	Field,
		Led_e 				ID_Led,
		uint8_t*			pString
) {
	Led_Bliking_Conf_s 	Led_BlinkingConf;
	uint8_t			Frequency[NB_CHAR_FREQUENCY];

	memset(Frequency, 0, NB_CHAR_FREQUENCY);

	switch(Field) {

		//----------------------------------------------------------
		case toString_Getpin:	toString_GetPeriphral	(Mapping_GPIO[Led[ID_Led].ID_PinMapping].GpioPeripheral,pString);
					toString_GetPin		(Mapping_GPIO[Led[ID_Led].ID_PinMapping].GpioPin, 	pString);
					break;

		//----------------------------------------------------------
		case toString_GetValue:

					switch(Led_GetState(ID_Led)) {

						case    LED_STATE_ON:
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_verte);
										__Led_SetOnString(pString);
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_blanc);
										break;
						case    LED_STATE_OFF:
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_rouge);
										__Led_SetOffString(pString);
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_blanc);
										break;

						case    LED_STATE_BLINKING:
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_verte);
										__Led_SetBlinkingString	(pString);
										Led_GetBlinkingConf(ID_Led, &Led_BlinkingConf);
										__GetFrequency(Frequency, Led_BlinkingConf.Period_ms);
										__SetFrequency(pString, Frequency);
										__SetFrequencyUnit_Hz(pString);
										__VT100STRING_SET_FOREGROUND_COLOR(pString, Couleur_blanc);
										break;

						default:			break;


					} break;


		//----------------------------------------------------------
		default:
					break;
	}
}*/

/********************************************************************
 * Private Functions definition
 */

/**------------------------------------------------------------------
 *
 * @brief Gestion du blinkage !
 *
 */
inline void
Gestion_Led_Blinking(
		Led_e Led_ID	/**<[in] ID de la Led */
) {
	//---------------------------------------------------------------
	//--------- Declaration des variables
	Etat_e 		Etat_Timer;

	//---------------------------------------------------------------
	//--------- Lecture de l'etat du Timer
	Etat_Timer = Led[Led_ID].Timer.Etat;

	//--------- Gestion du bliking
	switch(Led[Led_ID].Blinking_Config.Etape) {

		case BLINKING_ETAT_HAUT:

			//---------- Calcul de la duree de l'etat haut
			if(__Led_IsCalculationDone(Led_ID)) {

				Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms = Led[Led_ID].Blinking_Config.Period_ms * Led[Led_ID].Blinking_Config.Ratio_pr100;
				Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms /= 100;
				Led[Led_ID].Blinking_Config.Calculation_Done = TRUE;

				if(Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms == 0) {
					Led[Led_ID].State = LED_STATE_OFF;
					break;
				}
			}


			//---------- Gestion du Timer et de l'etat de la Pin
			if(Etat_Timer == ETAT_INACTIF) {

				TSW_Start(&Led[Led_ID].Timer, Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms);
				GPIO_Set(Led[Led_ID].ID_PinMapping, ETAT_ACTIF);
			}

			if(__TSW_isFinished(Led[Led_ID].Timer)) {

				TSW_Reset(&Led[Led_ID].Timer);
				Led[Led_ID].Blinking_Config.Calculation_Done = FALSE;
				GPIO_Set(Led[Led_ID].ID_PinMapping, ETAT_INACTIF);
				Led[Led_ID].Blinking_Config.Etape = BLINKING_ETAT_BAS;

			} break;



		case BLINKING_ETAT_BAS:

			//---------- Calcul de la duree de l'etat bas
			if(__Led_IsCalculationDone(Led_ID)) {

				Led[Led_ID].Blinking_Config.Duree_Etat_Bas_ms = Led[Led_ID].Blinking_Config.Period_ms - Led[Led_ID].Blinking_Config.Duree_Etat_Haut_ms;
				Led[Led_ID].Blinking_Config.Calculation_Done = TRUE;
			}

			//---------- Gestion du Timer
			if(Etat_Timer == ETAT_INACTIF)
				TSW_Start(&Led[Led_ID].Timer, Led[Led_ID].Blinking_Config.Duree_Etat_Bas_ms);

			if(__TSW_isFinished(Led[Led_ID].Timer)) {

				TSW_Reset(&Led[Led_ID].Timer);
				Led[Led_ID].Blinking_Config.Calculation_Done = FALSE;
				Led[Led_ID].Blinking_Config.Etape = BLINKING_ETAT_HAUT;

			} break;


		default:
			break;
	}
}

/**------------------------------------------------------------------------------
 *
 * @brief	Gestion etats Leds
 *
 */
inline void
Gestion_Led(
	Led_e Led_ID	/**<[in] ID de la Led */
) {
	switch(Led[Led_ID].State) {

		case LED_STATE_ON:
					if(Led[Led_ID].Blinking_Config.Calculation_Done == FALSE) {
						GPIO_Set(Led[Led_ID].ID_PinMapping, ETAT_ACTIF);
						Led[Led_ID].Blinking_Config.Calculation_Done = TRUE;
					} break;

		case LED_STATE_OFF:
					if(Led[Led_ID].Blinking_Config.Calculation_Done == FALSE) {
						GPIO_Set(Led[Led_ID].ID_PinMapping, ETAT_INACTIF);
						Led[Led_ID].Blinking_Config.Calculation_Done = TRUE;
					} break;

		case LED_STATE_BLINKING:
					Gestion_Led_Blinking(Led_ID);
					break;

		default:
					break;
	}
}
