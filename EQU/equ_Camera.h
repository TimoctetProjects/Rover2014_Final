/***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	23/01/2015
 * @file	equ_Camera.h
 * @brief	Driver camera
 *
 ***************************************************************************************************/

#ifndef EQU_CAMERA_H
#define EQU_CAMERA_H

/***************************************************************************************************
 * Includes
 */
#include "util_AffDebug.h"
#include "equ_Servo.h"
#include "equ_MCC.h"
#include "equ_Bargraph.h"

/***************************************************************************************************
 * Exported Fonction
 */
void 	Camera_init();
Bool_e 	Camera_IsConvRunning();



#endif //EQU_CAMERA_H
