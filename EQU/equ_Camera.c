/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	23/01/2015
 * @file	equ_Camera.c
 * @brief	Driver camera
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "equ_Camera.h"
#include "drv_ADC.h"
#include "drv_PWM.h"

/***************************************************************************************************
 * Private defines
 */
/** Configuration Fonctionnement camera*/
#define PERIODESIGNAL_SI_us		50000
#define RATIOSIGNAL_SI_pr1000	10
#define PERIODE_SIGNAL_SI_s		PERIODESIGNAL_SI_us/1000000

#define PERIODESIGNAL_CLK_us	100
#define RATIOSIGNAL_CLK_pr1000	500

/** Configuration Analyse camera */
#define NBPIXELTAKEN		120
#define INDICECAMERACENTRE	64

#define DELTACAMERACENTRE	10

/** Asservissement PID */
//#define PROPORTIONNEL	1
#define PID		1

#ifdef PROPORTIONNEL
	#define EQUATION_A	0.0008451227963
	#define EQUATION_B	0.7525818501
	#define EQUATION_C	30.29638456
#elif PID
	#define EQUATION_A	-0.002159197012
	#define EQUATION_B	1.10305789
	#define EQUATION_C	75


#endif

/***************************************************************************************************
 * Private macros
 */
#define __IsSemCreateOK(sEM)	sEM != NULL ?	TRUE : FALSE


/***************************************************************************************************
 * Private Types
 */
typedef enum {
	Recherche_Sommet1 = 0,
	Recherche_Minimum,
	Recherche_Sommet2,
}Etapes_Analyse_e;

enum {
	IDSommet1 = 0,
	IDMinimum,
	IDSommet2,
};

typedef enum {
	Camera_initialisation = 0,
	Camera_analyse
}Camera_Etapes_e;

typedef struct {
	uint8_t 	Indices[3];		/** Indice des sommets et minimums */
	uint32_t	ValCamADC[128];	/** Tableau de valeurs de la camera */
	uint32_t 	ValCamADCMoy;	/** Valeur moyenne de la ligne */

	ADC_s		ADC_Camera;
	TSW_s 		TSW_PeriodeSI;	/** Timer gerant le rafraichissement du signal SI */

	Bool_e  	fAnalyseNeeded;	/** Flag signifiant qu'une analyse est necessaire */
	Bool_e 		fConvRunning;	/** Flag signifiant qu'une conversion est en cours */
	//Bool_e		fLigneReady;	/** Flag signifiant qu'une ligne est prete */
}Camera_s;

/***************************************************************************************************
 * Private Functions prototypes
 */
static void Camera_SignalSI_IRQHandle();
static void Camera_SignalCLK_IRQHandle();
static void Camera_Analyse(void* pParam);
static void Camera_CalculCommande(void* pParam);
static void Camera_ADC_IRQHandler(void* value_mV);

#ifdef DEBUG_ON
static void Ligne_toString(toString_Possibilities_e Field, Mapping_GPIO_e IDPin, uint8_t* pString);
static void Commande_toString(toString_Possibilities_e	Field, Mapping_GPIO_e IDPin,uint8_t* pString);
static void Index_toString(toString_Possibilities_e Field, Mapping_GPIO_e IDPin, uint8_t* pString);
#endif /** DEBUG_ON */

/***************************************************************************************************
 * Private variables
 */
static xSemaphoreHandle semAnalyse = NULL, semCommande = NULL;

static uint8_t	nbIT;
static Camera_s Camera = {
		.ADC_Camera.ID_Pin 			= LigneCam,
		.ADC_Camera.StoreValue_mV 	= NULL,
		.ADC_Camera.ID_Mesure 		= 0,
		.ADC_Camera.IRQFunction 	= (pFunction)&Camera_ADC_IRQHandler
};

static float Commande;

#ifdef DEBUG_ON
static AffDebug_Ligne_s LigneMoy = {
		.Nom 			= "Valeur ligne moy",
		.ID 			= 0,
		.Previous_Value = "",
		.toString 		= (pFunction)&Ligne_toString,
		.toBeDisplayed 	= TRUE,
};

static AffDebug_Ligne_s IndexMin = {
		.Nom 			= "Index Min",
		.ID 			= 0,
		.Previous_Value = "",
		.toString 		= (pFunction)&Index_toString,
		.toBeDisplayed 	= TRUE,
};

static AffDebug_Ligne_s CommandeAff = {
		.Nom 			= "Commande",
		.ID 			= 0,
		.Previous_Value = "",
		.toString 		= (pFunction)&Commande_toString,
		.toBeDisplayed 	= TRUE,
};

/***************************************************************************************************
 * Constructor functions
 */
__attribute__((constructor(AffDebug_Ligne(1)))) void
AffIndex_init() {
	AffDebug_AddLine(&IndexMin);
	AffDebug_AddLine(&CommandeAff);
	AffDebug_AddLine(&LigneMoy);
}
#endif /** DEBUG_ON */

/***************************************************************************************************
 * Exported Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Initialisation de la camera
 */
void
Camera_init() {

	ADC_PeriphConfigure(&Camera.ADC_Camera);

	PWM_Init((PWM_init_s){ 	Broche_SI,
							PERIODESIGNAL_SI_us,
							RATIOSIGNAL_SI_pr1000,
							&Camera_SignalSI_IRQHandle	});

	PWM_Init((PWM_init_s){	BROCHE_CLK,
							PERIODESIGNAL_CLK_us,
							RATIOSIGNAL_CLK_pr1000,
							&Camera_SignalCLK_IRQHandle	});

	xTaskCreate( 	Camera_Analyse,
					(const char * const )"CamAnalyse",
					configMINIMAL_STACK_SIZE,
					NULL,
					3,
					NULL	);

	xTaskCreate( 	Camera_CalculCommande,
					(const char * const )"CamCmd",
					configMINIMAL_STACK_SIZE,
					NULL,
					3,
					NULL	);

	semAnalyse  = xSemaphoreCreateBinary();
	semCommande = xSemaphoreCreateBinary();

	__CheckParameters(__IsSemCreateOK(semAnalyse));
	__CheckParameters(__IsSemCreateOK(semCommande));

	PWM_Activer	(Broche_SI);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture du flag signifiant qu'une conversion est en cours
 */
Bool_e
Camera_IsConvRunning() {
	return Camera.fConvRunning;
}

/***************************************************************************************************
 * Private Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Analyse des donnees relevees de la ligne
 */
static void
Camera_Analyse(void* pParam) {

	Etapes_Analyse_e 	Etapes_Analyse 	= Recherche_Sommet1;
	uint8_t 			b_Indice 	= 0;
	uint32_t 			Valeur_moy 	= 0;

	xSemaphoreHandle* Sem = Bargraph_GetSem();

	for(;;) {
		xSemaphoreTake (semAnalyse, portMAX_DELAY);
		for(b_Indice=0; b_Indice<NBPIXELTAKEN; b_Indice = b_Indice+2) {

			Valeur_moy = Camera.ValCamADC[b_Indice]
					   + Camera.ValCamADC[b_Indice+1]
					   + Camera.ValCamADC[b_Indice+2];
			Valeur_moy /= 2;
			Camera.ValCamADCMoy += Valeur_moy;

			switch(Etapes_Analyse) {

				//----------------------------------------------------
				case Recherche_Sommet1:
					if(Valeur_moy > 3800) {
						Camera.Indices[IDSommet1] = b_Indice;
						Etapes_Analyse = Recherche_Minimum;
					} break;

				//----------------------------------------------------
				case Recherche_Minimum:
					if(Valeur_moy < 2600) {
						Camera.Indices[IDMinimum] = b_Indice;
						Etapes_Analyse = Recherche_Sommet2;
					} break;

				//----------------------------------------------------
				case Recherche_Sommet2:
					if(Valeur_moy > 3800) {
						Camera.Indices[IDSommet2] = b_Indice;
					} break;

				//----------------------------------------------------
				default:
					break;
			}
		}

		Camera.ValCamADCMoy /= (120/2);
		if(Camera.ValCamADCMoy > 3000) {
			switch(Etapes_Analyse) {
				case Recherche_Sommet1:
				case Recherche_Minimum:
					Commande = 75;
					break;

				case Recherche_Sommet2:
					xSemaphoreGive(semCommande);
					break;

				default:
					break;
			}
		} else	xSemaphoreGive (*Sem);
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Calcul de la commande servomoteur par asservissement PID
 *
 * @note
 * Indice min : 30 	-> pos voulue : 64 -> erreur = 30 - 64 = -34 : cmd servo : 35
 * Indice max : 120 	-> pos voulue : 64 -> erreur = 120- 64 = +56 : cmd servo : 130
 * Indice moy : 64	-> pos voulue : 64 -> erreur = 64 - 64 = 0   : cmd servo : 75
 *
 * Equation du type : erreur^2*A + erreur*B + erreur
 *
 * Coeff A = -37/17136
 * Coeff B = 9451/8568
 * Coeff C = 75

 * Temps d'acquisition ligne dt = 0.02s
 * Integral 	+= 	erreur*dt*ki
 * Derive 	=	(erreur - last_erreur)/dt
 *
 */

#define COEFF_KI		0.1
#define COEFF_KD		2
#define PERIODE_ACQUI_LIGNE	0.02

static void
Camera_CalculCommande(void* pParam) {
	xSemaphoreHandle* Sem = Bargraph_GetSem();

	for(;;) {
		xSemaphoreTake (semCommande, portMAX_DELAY);

#ifdef PROPORTIONNEL
		if( (Camera.Indices[IDMinimum] > (DELTACAMERACENTRE - DELTACAMERACENTRE))
		&&  (Camera.Indices[IDMinimum] < (DELTACAMERACENTRE + DELTACAMERACENTRE)) )
			Commande = 75;
		else {
			Commande = (EQUATION_A * Camera.Indices[IDMinimum] * Camera.Indices[IDMinimum] * (-1))
				 + (EQUATION_B * Camera.Indices[IDMinimum]) + EQUATION_C;

				if(Commande > 100)
					Commande = 100;
			else 	if(Commande < 50)
					Commande = 50;
		}

		Servo_setRatio_pr1000((uint32_t)Commande);

#elif PID
		int32_t erreur = 0;
		static int32_t last_erreur = 0;
		static float integral = 0;
		float derive = 0;

		if( (Camera.Indices[IDMinimum] > (DELTACAMERACENTRE - DELTACAMERACENTRE))
		&&  (Camera.Indices[IDMinimum] < (DELTACAMERACENTRE + DELTACAMERACENTRE)) ) {
				Commande = 75;
				last_erreur = erreur = 0;

		} else {
			erreur 		= Camera.Indices[IDMinimum] - INDICECAMERACENTRE;
			integral 	+= erreur*PERIODE_ACQUI_LIGNE*COEFF_KI;
			derive 		= (erreur - last_erreur)*COEFF_KD/PERIODE_ACQUI_LIGNE;
			last_erreur = erreur;

			Commande = (EQUATION_A * erreur * erreur)
					 + (EQUATION_B * erreur)
					 + EQUATION_C
					 + integral + derive;

				 if(Commande > 100)
					Commande = 100;
			else if(Commande < 50)
					Commande = 50;

			Servo_setRatio_pr1000((uint32_t)Commande);
		}
#endif
		xSemaphoreGive (*Sem);
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Fonction a lancer dans l'interruption ADC
 */
static void
Camera_ADC_IRQHandler(
	void* value_mV
) {
	Camera.ValCamADC[nbIT-1] = *(uint32_t*)value_mV;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Fonction a lancer dans l'interruption du signal SI
 */
static void
Camera_SignalSI_IRQHandle() {
	if(!Camera.fConvRunning) {
		PWM_Activer(BROCHE_CLK);
		Camera.fConvRunning = TRUE;
	}
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Fonction a lancer dans l'interruption du signal CLK
 */
static void
Camera_SignalCLK_IRQHandle() {

	if(Camera.fConvRunning) {
		ADC_StartConv( Camera.ADC_Camera );
		if(nbIT == 128){
			signed long pxHigherPriorityTaskWoken = 0;
			PWM_Desactiver(BROCHE_CLK);
			TIM_ITConfig(TIM2, TIM_IT_CC4, DISABLE);
			nbIT = 0;
			Camera.fConvRunning 	= FALSE;
			xSemaphoreGiveFromISR(semAnalyse, &pxHigherPriorityTaskWoken);
			portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
		} else	nbIT++;
	}
}

#ifdef DEBUG_ON
/**-------------------------------------------------------------------------------------------------
 * @brief	toString des lignes AffDebug pour la camera
 */
static void
Index_toString(
		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {
	switch(Field) {
		case toString_Getpin:	break;
		case toString_GetValue:	snprintf((char *)pString, 5, "%d", (int)(Camera.Indices[IDMinimum]));
								break;
	}
}

static void
Commande_toString(
		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {
	switch(Field) {
		case toString_Getpin:	break;
		case toString_GetValue:	snprintf((char *)pString, 10, "%d", (int)(Commande));
								break;
	}
}

static void
Ligne_toString(
		toString_Possibilities_e	Field,
		Mapping_GPIO_e				IDPin,
		uint8_t*					pString
) {
	switch(Field) {
		case toString_Getpin:	break;
		case toString_GetValue:	snprintf((char *)pString, 10, "%d", (int)(Camera.ValCamADCMoy));
								break;
	}
}


#endif /** DEBUG_ON */
