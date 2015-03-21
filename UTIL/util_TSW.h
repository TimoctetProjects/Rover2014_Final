/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	09/12/2014
 * @file	util_TSW.h
 * @brief	Timer software
 *
 ***************************************************************************************************/

#ifndef UTIL_TSW_H
#define UTIL_TSW_H

/***************************************************************************************************
 * Includes
 */
#include "bsp_carte.h"


/***************************************************************************************************
 * Exported defines
 */

/***************************************************************************************************
 * Exported types
 */
typedef enum {
	TSW_ElapsedTime = 0,
	TSW_RemainingTime
}TSW_ListeChoixValeurs;

typedef struct {

	Etat_e		Etat;			/** Etat du Timer */
	Status_e	Status;			/** Status du Timer */

	uint32_t	Stop_Value_ms;		/** Valeur de depart du timer */
	uint32_t	Pause_Value_ms;		/** Valeur de comptage du timer lors de l'arret */
	uint32_t	Start_Value_ms;		/** Valeur de fin du timer */

	void*		PreviousTimer;
	void*		NextTimer;

}TSW_s;

/***************************************************************************************************
 * Exported variables
 */

/***************************************************************************************************
 * Exported Macros
 */
#define	TSW_New(nAME)	 		TSW_s nAME = {ETAT_INACTIF, STATUS_FINIS, 0, 0, 0, NULL, NULL}

#define __TSW_IsActive(tSW)		( (tSW.Etat   == ETAT_ACTIF) 		? TRUE : FALSE )
#define __TSW_isFinished(tSW)	( (tSW.Status == STATUS_FINIS) 		? TRUE : FALSE )
#define __TSW_isRunning(tSW)	( (tSW.Status == STATUS_ENCOURS) 	? TRUE : FALSE )

/***************************************************************************************************
 * Exported Fonction
 */
void 	 TSW_Start		(TSW_s* Timer, uint32_t  Value_ms);
void	 TSW_StructInit	(TSW_s* Timer);
void 	 TSW_Reset		(TSW_s* Timer);
void 	 TSW_Stop		(TSW_s* Timer);
void	 TSW_Resume		(TSW_s*	Timer);
uint32_t TSW_Suspend		(TSW_s* Timer, TSW_ListeChoixValeurs Choix);
uint32_t TSW_StartUntil		(TSW_s*	Timer, uint32_t Until_ms, uint32_t Value_ms);
uint32_t TSW_GetElapsedTime	(TSW_s* Timer);
uint32_t TSW_GetRemainingTime(TSW_s* Timer);
uint32_t TSW_GetElapsedTime	(TSW_s* Timer);
uint32_t TSW_GetTimestamp	(void);

void TSW_VALIDATION(void);


#endif //UTIL_TSW_H
