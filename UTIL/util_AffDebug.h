/***************************************************************************************************
 *
 * @file	util_AffDebug.h
 * @author	Duclos Timothe
 * @date	05/11/2014
 * @brief	Gestion de l'affichage console
 *
 ***************************************************************************************************/

#ifndef UTIL_AFFDEBUG_H
#define UTIL_AFFDEBUG_H

/***************************************************************************************************
 * Includes
 */
#include "util_TSW.h"
#include "util_Console.h"

/***************************************************************************************************
 * Exported defines
 */
/** @brief Configuration affichage */
/** Refresh de l'affichage */
#define PERIODE_RAFRAICHISSEMENT_AFFICHAGE_ms	50

/** Definition du tableau */
#define TAILLE_TABULATION			5

#define NB_CHAR_NOM					30
#define NB_CHAR_VALUE				50
#define NB_CHAR_PERIPH				17

#define NB_COLONNES					3
#define Nom_Colonne1				"Nom"
#define Nom_Colonne2				"Pin"
#define Nom_Colonne3				"Valeur"

#define	Char_Separateur_Y			".."

#define OFFSET_LIGNE_SEPARATRICE	20

#define AFFDEBUG_NBLIGNES			20

/** Mesure duree tour de boucle */
#define	NB_TOURBOUCLE_ECHANTILLON	2000

/***************************************************************************************************
 * Exported variables
 */

/***************************************************************************************************
 * Exported Macros
 */
#define AffDebug_Ligne(lIGNE)	( lIGNE < 20 && lIGNE >= 1 ? 110 + lIGNE : 119 )

/***************************************************************************************************
 * Exported types
 */
typedef enum {

	Affichage_Initialisation = 0,
	Affichage_DataPinAndValue,

}Etape_Affichage_e;

/** @brief Structure d'une ligne d'affichage */
typedef struct {

	uint8_t		Nom				[NB_CHAR_NOM];		/** Nom de la ligne */
	uint8_t		ID;									/** Identifiant qui sera passe en argument a la fonction d'affichage */
	uint8_t		Previous_Value	[NB_CHAR_VALUE];	/** Champ contenant la valeur affichee precedente */
	pFunction	toString;							/** Fonction d'affichage permettant d'afficher la pin et la data */
	Bool_e		toBeDisplayed;						/** Permet d'autoriser ou d'interdire l'affichage de la ligne */

}AffDebug_Ligne_s;

/***************************************************************************************************
 * Exported Function
 */
inline  void 	AffDebug_AddLine		(AffDebug_Ligne_s* NewLine);
		void 	AffDebug_Main 			(void);
		void 	AffDebug_init 			(void);
		void 	AffDebug_setDureeTourBoucle	(void);


#endif /* UTIL_AFFDEBUG_H */
