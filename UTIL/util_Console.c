/***************************************************************************************************
 *
 * @file	util_Console.c
 * @author	Duclos Timothe
 * @date	17/09/2014
 * @brief	Utilitaire d'affichage console
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "util_Console.h"
#include "util_TSW.h"
#include "drv_USART.h"

/***************************************************************************************************
 * Private defines
 */
#define PERIODE_REFRESH_PING_TERMINAL_ms	100
#define PERIODE_PING_TIMEOUT_ms				1
#define VITESSE_USART_DEBUG_bauds			115200


/***************************************************************************************************
 * Private defines
 */
typedef struct {

	Bool_e					TerminalPresent;
	Liste_Usart_Periph_e	Periph_Usart;
	Mapping_GPIO_e			ID_Pin_TX;
	Mapping_GPIO_e			ID_Pin_RX;

}Console_s;

/***************************************************************************************************
 * Private variables
 */
static Console_s Console = {TRUE, Periph_USART6, CONSOLE_TX, CONSOLE_RX};

static TimerHandle_t TimersTimeout, TimersPing;

/***************************************************************************************************
 * Private Fonction Prototype
 */
static void Console_PingTimeout	(TimerHandle_t xTimer);
static void Console_Ping		(TimerHandle_t xTimer);

/***************************************************************************************************
 * Extern Fonction Definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Init de la console
 * @return	Etape suivante
 */
void
Console_Init(
	void
) {

	Usart_InitPeriph( Console.ID_Pin_TX,
					  Console.ID_Pin_RX,
					  VITESSE_USART_DEBUG_bauds );

	TimersTimeout = xTimerCreate(    "PongTimout",
									 PERIODE_PING_TIMEOUT_ms,
													 pdFALSE,
													 NULL,
													 Console_PingTimeout );

	TimersPing = xTimerCreate(    	"Ping",
									PERIODE_REFRESH_PING_TERMINAL_ms,
									pdTRUE,
									NULL,
									Console_Ping );

	xTimerStop	(TimersTimeout, portMAX_DELAY);
	xTimerStart	(TimersPing, 	portMAX_DELAY);

	__VT100CMD_CLEAR_SCREEN;
	__VT100CMD_MOVECURSOR_TOPLEFTUP;
	__VT100CMD_SWITCH_CURSOR_INVISIBLE;
	__VT100CMD_SET_FOREGROUND_COLOR(Couleur_blanc);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Lecture de la presence console
 * @return	TRUE (present) ou FALSE (absent)
 */
Bool_e
Console_GetPresence(
	void
) {
	return Console.TerminalPresent;
}


/***************************************************************************************************
 * Private Fonction Definition
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	On regarde si on a reçu le pong afin de "setter" l'etat de la console
 */
static void
Console_PingTimeout(TimerHandle_t xTimer) {
	uint8_t TermAnswer;
	if(USART_Read((uint32_t) USART6, &TermAnswer, 1) != 0) {

		if	(	TermAnswer == 27
			|| 	TermAnswer == 91
			|| 	TermAnswer == 63
			|| 	TermAnswer == 54
			|| 	TermAnswer == 99  )	{

			Console.TerminalPresent = TRUE;
			return;
		}
	}
	Console.TerminalPresent = FALSE;
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Ping de la console
 */
static void
Console_Ping(TimerHandle_t xTimer) {
	__VT100CMD_IDENTIDY_TERMINAL;
	xTimerStart( TimersTimeout, 0 );
}
