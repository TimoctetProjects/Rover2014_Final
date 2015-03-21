/***************************************************************************************************
 *
 * @file	util_AffDebug.c
 * @author	Duclos Timothe
 * @date	05/11/2014
 * @brief	Gestion de l'affichage console
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "util_AffDebug.h"

/***************************************************************************************************
 * Private defines
 */
#define NULL_STRING		"NULL"
#define PERIODEREFRESHAFF_ms	200

/***************************************************************************************************
 * Private Macros
 */
#define __Offset_X_Colonne1			(	(8 * TAILLE_TABULATION)					)
#define __Offset_X_Colonne2			(	__Offset_X_Colonne1 	+ (4 * TAILLE_TABULATION)	)
#define __Offset_X_Colonne3			(	__Offset_X_Colonne2 	+ (4 * TAILLE_TABULATION)	)

#define __Taile_Ligne_Separatrice 		(	(NB_COLONNES		* NB_CHAR_NOM)	/	2	)

#define	__SetNullString(str)			( 	__strncpy (str, NULL_STRING	) 	)

#define __IsDataChanged(id, buf)		(	__strncmp( DonneAffiche[id]->Previous_Value, buf, __strlen(buf) ) != 0	)
#define __CopyNewData(id, buf)			__strncpy( DonneAffiche[b_Affichage]->Previous_Value, (char *)buf , strlen((char *)buf))
#define __GetDeltaStringLength(id, buf)		__strlen( buf)  - __strlen( DonneAffiche[id]->Previous_Value )
#define __IsNewDataShorter(diff)		( 	diff < 0			)

#define __TrBClMoy_SetValue_ms(str, val)	sprintf((char *)str , "%d ms", (int)(val/NB_TOURBOUCLE_ECHANTILLON))
#define __TrBClMoy_SetValue_us(str, val)	sprintf((char *)str , "%d us", (int)((val*1000)/NB_TOURBOUCLE_ECHANTILLON))

#define __GetValue_string(id, type, str)	DonneAffiche[id]->toString((void *)type, DonneAffiche[id]->ID, str)


/***************************************************************************************************
 * Private Types
 */
typedef enum {
	Ligne_Titre_Tableau = 3,
	Ligne_Separateur,
	Premiere_Ligne_Tableau,
}Offset_Y_Lignes;

typedef enum {
	Colonne_Nom = 0,
	Colonne_Pin,
	Colonne_Valeur,
}Nom_Colonnes_e;

/***************************************************************************************************
 * Private Fonctions prototypes
 */
static 	void Affichage_Data (TimerHandle_t xTimer);


/***************************************************************************************************
 * Private variables
 */
static TimerHandle_t TimerAffData;

static const	uint32_t	offsetColonne	[NB_COLONNES] 	= 	{

		__Offset_X_Colonne1-10,	__Offset_X_Colonne2,	__Offset_X_Colonne3
};

static AffDebug_Ligne_s* 	DonneAffiche[AFFDEBUG_NBLIGNES];
static uint8_t 				NbLigneUsed;

/***************************************************************************************************
 * Exported Fonctions
 */
void
AffDebug_init() {
	uint32_t 	b_Affichage = 0;
	uint8_t		TitreColonne	[NB_COLONNES]	 [NB_CHAR_NOM] = {	Nom_Colonne1,	Nom_Colonne2,	Nom_Colonne3 };
	uint8_t*	Buffer			[NB_CHAR_PERIPH];

	//------------------------ Affichage Ligne Titre Tableau
	for(b_Affichage=0; b_Affichage<NB_COLONNES; b_Affichage++) {

		__VT100CMD_MOVECURSOR_TOPOSITION(Ligne_Titre_Tableau, offsetColonne[b_Affichage]);
		__Console_Send((char *) TitreColonne[b_Affichage]);
	}

	//------------------------ Affichage ligne separatrice
	__VT100CMD_MOVECURSOR_TOPOSITION(Ligne_Separateur, OFFSET_LIGNE_SEPARATRICE);

	for(b_Affichage=0; b_Affichage<__Taile_Ligne_Separatrice; b_Affichage++) {
		__Console_Send(Char_Separateur_Y);
	}

	//------------------------ Affichage nom et pin colonnes
	for(b_Affichage=0; b_Affichage<NbLigneUsed; b_Affichage++) {

		if (DonneAffiche[b_Affichage]->toBeDisplayed != FALSE) {

			memset(Buffer, 0, NB_CHAR_PERIPH);

			//--------- Placement curseur et Affichage Nom de la ligne
			__VT100CMD_MOVECURSOR_TOPOSITION(Premiere_Ligne_Tableau+b_Affichage, offsetColonne[Colonne_Nom]);
			__Console_Send((char *)(DonneAffiche[b_Affichage]->Nom));

			//--------- Placement curseur et Affichage Pin utilise
			__VT100CMD_MOVECURSOR_TOPOSITION(Premiere_Ligne_Tableau+b_Affichage, offsetColonne[Colonne_Pin]);
			__GetValue_string(b_Affichage, toString_Getpin, Buffer);
			__Console_Send((char *)Buffer);
		}
	}

	TimerAffData = xTimerCreate(    "AffData",
									PERIODEREFRESHAFF_ms,
									 pdTRUE,
									 NULL,
									 Affichage_Data );

	xTimerStart(TimerAffData, portMAX_DELAY);
}
/**-------------------------------------------------------------------------------------------------
 * @brief	Insertion d'une ligne d'affichage
 */
inline void
AffDebug_AddLine(
	AffDebug_Ligne_s* NewLine		/**<[in] Structure de la nouvelle ligne */
) {
	DonneAffiche[NbLigneUsed++] = NewLine;
}


/***************************************************************************************************
 * Private Functions definition
 */
/**-------------------------------------------------------------------------------------------------
 *
 * @brief	Affichage tableau valeurs quand diffÈrentes de la prÈcÈdente
 *
 */
static void
Affichage_Data(TimerHandle_t xTimer) {

	uint8_t		b_Effacer = 0;
	uint8_t		New_Data	[NB_CHAR_VALUE];
	int32_t		Difference;
	uint32_t 	b_Affichage = 0;

	if(Console_GetPresence()) {
		for(b_Affichage=0; b_Affichage<NbLigneUsed; b_Affichage++) {

			if (DonneAffiche[b_Affichage]->toBeDisplayed != FALSE) {

				//---------- R‡Z Tableau et recuperation de la nouvelle donne sous forme de chaine
				memset(New_Data, 0, NB_CHAR_VALUE);
				__GetValue_string(b_Affichage, toString_GetValue, New_Data);

				//---------- Test si la donne a ete change
				if __IsDataChanged(b_Affichage, New_Data) {

					//-------------- Copie de la nouvelle donnee pour la prochaine comparaison
					__CopyNewData(b_Affichage, New_Data);

					//-------------- Calcul de la difference en caracter (afin que ca recouvre totalement l'ancienne chaine)
					Difference = __GetDeltaStringLength(b_Affichage, New_Data);

					if __IsNewDataShorter(Difference) {

						for(b_Effacer = 0; b_Effacer < -Difference; b_Effacer++)	{

							if(strlen((char *)New_Data) != NB_CHAR_VALUE)
								strncat((char *)New_Data, " ", 1);
							else
								b_Effacer = -Difference;
						}
					}

					//-------------- Placement curseur et Affichage
					__VT100CMD_MOVECURSOR_TOPOSITION(Premiere_Ligne_Tableau+b_Affichage, offsetColonne[Colonne_Valeur]);
					__Console_Send((char *)New_Data);
				}
			}
		}
	}
}

