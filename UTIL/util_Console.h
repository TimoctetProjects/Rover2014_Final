/***************************************************************************************************
 *
 * @file	util_Console.h
 * @author	Duclos Timothe
 * @date	17/09/2014
 * @brief	Utilitaire d'affichage console
 *
 ***************************************************************************************************/

#ifndef UTIL_CONSOLE_H
#define UTIL_CONSOLE_H

/***************************************************************************************************
 * Includes
 */
#include "bsp_carte.h"



/***************************************************************************************************
 * Exported defines
 */

/** Champ d'affichage */
#define FREQUENCY_UNIT_Hz		" Hz"
#define FREQUENCY_UNIT_KHz		" KHz"
#define ON_STRING				"On"
#define OFF_STRING				"Off"

#define NB_CHAR_FREQUENCY		15
#define NB_CHAR_POURCENTAGE		12

/** Code ASCII de Caractères*/
#define ASCII_ESC 				27

/** Taille de chaines */
#define COLOR_STRING_LENGTH		9

/** Code VT100 */
#define VT100STRING_COLOR					"\033[3%dm"
#define VT100STRING_MOVECURSOR_TOPOSITION	"\033[%d;%df"
#define VT100STRING_MOVECURSOR_TOPLEFTUP	"\033[H"
#define VT100STRING_CLEARSCREEN				"\033[2J"
#define VT100STRING_CLEARSCREENFROMPOS_UP	"\033[1J"
#define VT100STRING_CLEARSCREENFROMPOS_DOWN	"\033[0J"
#define VT100STRING_CLEAR_LINE				"\033[2K"
#define VT100STRING_CLEAR_LINE_LEFT			"\033[1K"
#define VT100STRING_CLEAR_LINE_RIGHT		"\033[0K"
#define VT100STRING_SWUTCH_CURSOR_INVISIBLE	"\033[?25l"

#define VT100STRING_IDENTIFY_TERMINAL		"\033Z"

/***************************************************************************************************
 * Exported types
 */
typedef enum {

	Couleur_noir = 0,
	Couleur_rouge,
	Couleur_verte,
	Couleur_jaune,
	Couleur_bleu,
	Couleur_magenta,
	Couleur_cyan,
	Couleur_blanc,
	Couleur_default,

	nb_Couleurs

}Liste_couleursVT100_e;

/***************************************************************************************************
 * Exported variables
 */
static const uint8_t ColorString[nb_Couleurs][COLOR_STRING_LENGTH] = {

		{"\033[30m"},		/** Noir */
		{"\033[31m"},		/** Rouge */
		{"\033[32m"},		/** Vert */
		{"\033[33m"},		/** Jaune */
		{"\033[34m"},		/** Bleu */
		{"\033[35m"},		/** Magenta */
		{"\033[36m"},		/** Cyan */
		{"\033[37m"},		/** Blanc */
		{"\033[39m"},		/** Default */
};


/***************************************************************************************************
 * Exported Macros
 */
#define __Console_Send			printf

/**-------------------------------------------------------------------------------------------------
 *  Commande VT100
 *
 */
/** Deplacer le curseur */
#define __VT100CMD_MOVECURSOR_TOPOSITION(l, c)		__Console_Send(VT100STRING_MOVECURSOR_TOPOSITION,(int)(l), (int)(c))
#define __VT100CMD_MOVECURSOR_TOPLEFTUP				__Console_Send(VT100STRING_MOVECURSOR_TOPLEFTUP)

/** Effacement ecran */
#define __VT100CMD_CLEAR_SCREEN						__Console_Send(VT100STRING_CLEARSCREEN)
#define __VT100CMD_CLEAR_SCREEN_FROM_CURSOR_UP		__Console_Send(VT100STRING_CLEARSCREENFROMPOS_UP)
#define __VT100CMD_CLEAR_SCREEN_FROM_CURSOR_DOWN	__Console_Send(VT100STRING_CLEARSCREENFROMPOS_DOWN)

/** Effacement Ligne */
#define __VT100CMD_CLEAR_LINE						__Console_Send(VT100STRING_CLEAR_LINE)
#define __VT100CMD_CLEAR_LINE_FROM_CURSOR_LEFT		__Console_Send(VT100STRING_CLEAR_LINE_LEFT)
#define __VT100CMD_CLEAR_LINE_FROM_CURSOR_RIGHT		__Console_Send(VT100STRING_CLEAR_LINE_RIGHT)

/** Rendre le curseur invisible */
#define __VT100CMD_SWITCH_CURSOR_INVISIBLE			__Console_Send(VT100STRING_SWUTCH_CURSOR_INVISIBLE)


/** Couleur avant plan */
#define __VT100CMD_SET_FOREGROUND_COLOR(couleur)	__Console_Send(VT100STRING_COLOR, couleur)


/** Identifier le terminal */
#define __VT100CMD_IDENTIDY_TERMINAL				__Console_Send(VT100STRING_IDENTIFY_TERMINAL)


/**-------------------------------------------------------------------------------------------------
 *  Generation de strings
 *
 */
#define __VT100STRING_SET_FOREGROUND_COLOR(str, clr) \
	__strncat(str, ColorString[clr], COLOR_STRING_LENGTH)

#define __SetFrequencyString(str, freq) \
	__strncat(str, freq, NB_CHAR_FREQUENCY	)

#define __InsertPinSeparator(str) \
	__strncat(str, 	"-", 1)

#define __SetFrequencyUnit_Hz(str) \
	__strncat(str, 	 FREQUENCY_UNIT_Hz, __strlen(FREQUENCY_UNIT_KHz))

#define __SetFrequencyUnit_KHz(str) \
	__strncat(str, 	 FREQUENCY_UNIT_KHz, __strlen(FREQUENCY_UNIT_KHz))



/***************************************************************************************************
 * Exported Function
 */
Bool_e 	Console_GetPresence	(void);
void Console_Init(void);

#endif /** UTIL_CONSOLE_H */
