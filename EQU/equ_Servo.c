/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	03/02/2015
 * @file	equ_Servo.c
 * @brief	Driver servomoteur
 *
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 */
#include "equ_Servo.h"

/***************************************************************************************************
 * Private defines
 */
#define PERIODEPWMSERVO_us			20000
#define RATIOPWMSERVOINIT_pr1000	75

/***************************************************************************************************
 * Exported functions
 */
/**-------------------------------------------------------------------------------------------------
 * @brief	Initialisation du servomoteur
 */
void
Servo_init(void) {
	PWM_Init((PWM_init_s){	PIN_SERVO_DRIECTION,
							PERIODEPWMSERVO_us,
							RATIOPWMSERVOINIT_pr1000,
							NULL	});
	PWM_Activer(PIN_SERVO_DRIECTION);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Modification du ratio du servomoteur
 */
void
Servo_setRatio_pr1000(
	uint32_t Ratio_pr1000	/**<[in] Ratio du servomoteur en pr1000 */
) {
	PWM_Desactiver	( PIN_SERVO_DRIECTION );
	PWM_Desinit		( PIN_SERVO_DRIECTION );
	PWM_Init		( (PWM_init_s){	PIN_SERVO_DRIECTION,
									PERIODEPWMSERVO_us,
									Ratio_pr1000,
									NULL } );
	PWM_Activer(PIN_SERVO_DRIECTION);
}

