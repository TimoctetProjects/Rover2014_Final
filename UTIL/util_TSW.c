/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	09/12/2014
 * @file	util_TSW.c
 * @brief	Timer software
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "util_TSW.h"

/***************************************************************************************************
 * Private defines
 */
#define TAILLE_TSW_32_bits	0xFFFFFFFF

/***************************************************************************************************
 * Private macros
 */
#define __TSW_IsIDValid(id)	( (id >= 0) && (id < nb_TIMERS) )
#define __TSW_Printf		printf

/***************************************************************************************************
 * Private Types
 */

/***************************************************************************************************
 * Private variables
 */
volatile 	uint32_t 	msTicks 	= 0;
static  	TSW_s		FirstTSW	= {.NextTimer = NULL};
static		TSW_s*		CurrentTSW 	= &FirstTSW;

/***************************************************************************************************
 * Private Functions prototypes
 */
static inline void DeleteTSW(TSW_s* DoomedTimer);
static inline void InsertTSW(TSW_s* NewTimer);

/***************************************************************************************************
 * Exported Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Demarrer le timer
 */
void
TSW_Start(
	TSW_s*	 Timer,		/**<[in] Adresse du timer */
	uint32_t Value_ms	/**<[in] Valeur du timer en millisecondes */
) {

	if(Timer->Status != STATUS_ENCOURS)	InsertTSW(Timer);

	Timer->Etat 			= ETAT_ACTIF;
	Timer->Status 			= STATUS_ENCOURS;
	Timer->Stop_Value_ms 	= msTicks + Value_ms;
	Timer->Start_Value_ms 	= msTicks;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Demarrer le timer a partir d'une valeur (permet reglage plus precis)
 */
uint32_t
TSW_StartUntil(
	TSW_s*	 Timer,		/**<[in] Adresse du timer */
	uint32_t Until_ms,	/**<[in] Valeur a partir de laquelle compter */
	uint32_t Value_ms	/**<[in] Valeur du timer en millisecondes */
) {

	if(Timer->Status != STATUS_ENCOURS)	InsertTSW(Timer);

	Timer->Etat 		= ETAT_ACTIF;
	Timer->Status 		= STATUS_ENCOURS;

	if(Until_ms != 0)
		Timer->Stop_Value_ms 	= Until_ms + Value_ms;
	else
		Timer->Stop_Value_ms 	= msTicks + Value_ms;

	Timer->Start_Value_ms 	= msTicks;

	return Timer->Stop_Value_ms;
}

/**-------------------------------------------------------------------------------------------------
 * @brief
 */
void
TSW_StructInit(
	TSW_s*	 Timer	/**<[in]  Adresse du timer */
) {

	Timer->Etat				= ETAT_INACTIF;
	Timer->NextTimer		= NULL;
	Timer->Pause_Value_ms	= 0;
	Timer->PreviousTimer	= NULL;
	Timer->Start_Value_ms	= 0;
	Timer->Status			= STATUS_FINIS;
	Timer->Stop_Value_ms	= 0;
}

/**-------------------------------------------------------------------------------------------------
 * @brief
 */
void
TSW_Reset(
	TSW_s*	 Timer	/**<[in]  Adresse du timer */
) {

	DeleteTSW(Timer);
	Timer->Etat 		= ETAT_INACTIF;
	Timer->Status		= STATUS_FINIS;
	Timer->Stop_Value_ms= 0;
	Timer->NextTimer	= NULL;
	Timer->PreviousTimer= NULL;
}


/**-------------------------------------------------------------------------------------------------
 * @brief	Suspension du timer
 * @return	Valeur restante ou ecoulee
 */
uint32_t
TSW_Suspend(
	TSW_s*	 				Timer,	/**<[in]  Adresse du timer */
	TSW_ListeChoixValeurs	Choix	/**<[in]  Retourne le temps restant ou le temps ecouler en
											  fontion de ce choix :
											  TSW_ElapsedTime | TSW_RemainingTime */
) {
	if(Timer->Status == STATUS_FINIS)	return 0;

	DeleteTSW(Timer);
	Timer->Etat 			= ETAT_INACTIF;
	Timer->Status			= STATUS_OK;
	Timer->NextTimer		= NULL;
	Timer->PreviousTimer	= NULL;
	Timer->Pause_Value_ms	= Timer->Stop_Value_ms - msTicks;

	switch(Choix) {
		case TSW_ElapsedTime:		return msTicks - Timer->Start_Value_ms;
		case TSW_RemainingTime:		return Timer->Stop_Value_ms - msTicks;
	}

	return 0;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Reprise du timer
 */
void
TSW_Resume(
	TSW_s*	 	Timer	/**<[in]  Adresse du timer */
) {
	TSW_Start(Timer, Timer->Pause_Value_ms);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Arret du timer
 */
void
TSW_Stop(
	TSW_s*	 	Timer	/**<[in]  Adresse du timer */
) {
	if(Timer->Status == STATUS_FINIS)	return;

	DeleteTSW(Timer);
	Timer->Etat 			= ETAT_INACTIF;
	Timer->Status			= STATUS_FINIS;
	Timer->NextTimer		= NULL;
	Timer->PreviousTimer	= NULL;
	Timer->Pause_Value_ms	= 0;
	Timer->Start_Value_ms	= 0;
	Timer->Stop_Value_ms	= 0;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture du Timestamp
 * @return	Valeur du timestamp en millisecondes
 */
uint32_t
TSW_GetTimestamp(
		void
) {
	return msTicks;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture du temps restant
 * @return	Valeur du temps restant en millisecondes
 */
uint32_t
TSW_GetRemainingTime(
		TSW_s* Timer	/**<[in]  Adresse du timer */
) {

	if(Timer->Status == STATUS_FINIS)	return 0;
	return (Timer->Stop_Value_ms - msTicks);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture du Temps �coul�
 * @return	Valeur du temps �coule en millisecondes
 */
uint32_t
TSW_GetElapsedTime(
		TSW_s* Timer	/**<[in]  Adresse du timer */
) {
	return (msTicks - Timer->Start_Value_ms);
}


void vApplicationTickHook(void){
	msTicks++;
	if(msTicks == TAILLE_TSW_32_bits) {
		msTicks=0;
	}

	while(CurrentTSW->NextTimer != NULL) {

		CurrentTSW = CurrentTSW->NextTimer;

		if(CurrentTSW->Stop_Value_ms <= msTicks) {

			CurrentTSW->Status = STATUS_FINIS;

			if((uint32_t)CurrentTSW != (uint32_t) &FirstTSW)
				DeleteTSW(CurrentTSW);
		}


	}	CurrentTSW = &FirstTSW;
}


/***************************************************************************************************
 * Private Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Suppression d'un Timer dans la liste
 */
static inline void
DeleteTSW(
	TSW_s* DoomedTimer	/**<[in] Timer a supprimer de la liste */
) {

	((TSW_s*)DoomedTimer->NextTimer)->PreviousTimer	=	DoomedTimer->PreviousTimer;
	((TSW_s*)DoomedTimer->PreviousTimer)->NextTimer	=	DoomedTimer->NextTimer;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Ajout d'un Timer dans la liste
 */
static inline void
InsertTSW(
	TSW_s*	NewTimer	/**<[in] Timer a inserer dans la liste */
) {

	if(FirstTSW.NextTimer == NULL) {
		FirstTSW.NextTimer 		= NewTimer;
		NewTimer->NextTimer 	= NULL;
		NewTimer->PreviousTimer = &FirstTSW;
		return;
	}

	((TSW_s*)(FirstTSW.NextTimer))->PreviousTimer	= (void *)NewTimer;

	NewTimer->NextTimer 	= FirstTSW.NextTimer;
	FirstTSW.NextTimer		= NewTimer;
	NewTimer->PreviousTimer	= &FirstTSW;
}
