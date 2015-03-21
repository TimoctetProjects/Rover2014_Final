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
