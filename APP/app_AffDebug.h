/*********************************************************************
 * *******************************************************************
 *
 * @file	app_AffDebug.h
 *
 * @author	Duclos Timothe
 *
 * @date	30/09/2014
 *
 * @brief	Gestion de l'affichage console
 *
 *********************************************************************/

#ifndef APP_AFFDEBUG_H
#define APP_AFFDEBUG_H

/********************************************************************
 * Includes
 */
#include "util_TSW.h"
#include "drv_GPIO.h"
#include "equ_Camera.h"

/********************************************************************
 * Exported defines
 */
#define	NB_TOURBOUCLE_ECHANTILLON	2000

/********************************************************************
 * Exported variables
 */

/********************************************************************
 * Exported types
 */
typedef enum {

	Affichage_Initialisation = 0,
	Affichage_DataPinAndValue,

}Etape_Affichage_e;

/********************************************************************
 * Exported Function
 */

/**------------------------------------------------------------------
 *
 * @brief	Gestion de l'affichage Debug
 *
 */
void
Affichage_Main (
		void
);

/**------------------------------------------------------------------
 *
 * @brief	Setter la duree du tour de boucle
 *
 */
void
setDureeTourBoucle(
	void
);

#endif
