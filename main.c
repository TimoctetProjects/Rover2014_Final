/*********************************************************************
 * *******************************************************************
 *
 * @author	Duclos Timothe
 *
 * @date	09/12/2014
 *
 * @brief	Main
 *
 *********************************************************************/

/********************************************************************
 * Includes
 */
#include "equ_Camera.h"

/********************************************************************
 * Exported functions definition
 */
/**-------------------------------------------------------------------
 * Definition de la tache Idle
 *
 */
void vApplicationIdleHook(void){
	__WFI();
}

/**-------------------------------------------------------------------
 * Main function
 *
 */
int
main(
	void
){
	uint32_t PLLL1 = 336 <<6;
	uint32_t PLLL2 = ((2>>1) - 1) << 16;
	uint32_t PLLL3 = 0x00400000;
	uint32_t PLLLQ = 7 << 24;
	uint32_t clockk = PLLL1 | PLLL2 | PLLL3 | PLLLQ;

	//--------------------------------------------------------------
	//------------ Initialisations
	BSP_Init();
	Servo_init();
	Camera_init();
	Console_Init();
	Bargraph_init();
	MCC_init();

	__NOP;

	while(!GPIO_Get(BTN_WKP));

	MCC_GoForward();

	#ifdef DEBUG_ON

		AffDebug_init();
	#endif /** DEBUG_ON */

	vTaskStartScheduler();

	return 0;
}
