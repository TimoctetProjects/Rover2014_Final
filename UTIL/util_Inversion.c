/***************************************************************************************************
 ***************************************************************************************************
 *
 * @author	Duclos Timothe
 * @date	11/12/2014
 * @file	util_Inversion.c
 * @brief
 *
 ***************************************************************************************************/


/***************************************************************************************************
 * Includes
 */
#include "util_Inversion.h"

/***************************************************************************************************
 * Private defines
 */

/***************************************************************************************************
 * Private macros
 */
#define __InversionPrintf(args ...)	printf(## args)



/***************************************************************************************************
 * Private Types
 */

/***************************************************************************************************
 * Private variables
 */
/***************************************************************************************************
 * Private Functions prototypes
 */

/***************************************************************************************************
 * Exported Functions definition
 */

/**-------------------------------------------------------------------------------------------------
 * @brief	Validation des inversions
 */

#if	0

#define	INVERSIONTESTVALUE_8	0xF
#define INVERSIONTESTVALUE_16	0xFF
#define INVERSIONTESTVALUE_32	0xFFFF

#define __IsInversionSuccessful(iNV1, iNV2, iNV3) 	\
		(test8 	== INVERSIONTESTVALUE_8)  	\
 	&& 	(test16 == INVERSIONTESTVALUE_16) 	\
	&&	(test32 == INVERSIONTESTVALUE_32) ?	\
			STATUS_OK : STATUS_KO

/**-------------------------------------------------------------------------------------------------
 * @brief	Validation de l'inversion
 */
Status_e
Inversion_Validation(
	void
) {
	uint8_t 	test8 	= INVERSIONTESTVALUE_8		<< 4;
	uint16_t 	test16 	= INVERSIONTESTVALUE_16 	<< 8;
	uint32_t 	test32	= INVERSIONTESTVALUE_32 	<< 16;

	Inversion_LSBMSB( &test8,  sizeof(test8)  );
	Inversion_LSBMSB( &test16, sizeof(test16) );
	Inversion_LSBMSB( &test32, sizeof(test32) );

	return __IsInversionSuccessful(test8, test16, test32);
}
#endif

/**-------------------------------------------------------------------------------------------------
 * @brief	Inversion d'un tableau de char
 */
void
Inversion_TabChar(
	uint8_t* Tab,		/**<[in] Pointeur sur le tableau a inverser */
	uint32_t Size_octet	/**<{in] Nombre d'octet (=char) composant le tableau */
) {

	uint8_t* tab_end;
	uint8_t* tab_temp = (uint8_t *) malloc((Size_octet+1) * sizeof(uint8_t));

	memset(tab_temp, 0, Size_octet);

	tab_end  = Tab 		- 1;
	Tab 	+= Size_octet	- 1;

	do 	{	*(tab_temp++) = *(Tab--);	}
	while	( (uint32_t) Tab != (uint32_t) tab_end	);

	Tab++;
	tab_temp -= Size_octet;

	__strncpy(Tab, tab_temp, Size_octet);

	free(tab_temp);
}

/**-------------------------------------------------------------------------------------------------
 * @brief	Inversion d'une valeur uint8_t/uint16_t/uint32_t : LSB <-> MSB
 */
void
Inversion_LSBMSB(
		void* 		pValue,		/**<[in] Pointeur sur la valeur a inverser */
		uint32_t 	Size_octet	/**<[in] Nombre d'octets constituant la donnee */
) {

	uint8_t  b_Inversion 	= 0;
	uint32_t MASK 		= 0x1;
	uint32_t valueInv 	= 0;

	uint32_t* pValue32 = (uint32_t *) pValue;

	MASK <<= (Size_octet * 8) - 1;

	for(b_Inversion=0; b_Inversion<(Size_octet * 8); b_Inversion++) {

		valueInv |= ( *pValue32 & (MASK >> b_Inversion) ) >> (((Size_octet * 8) - 1) - b_Inversion) << b_Inversion;
	}

	switch(Size_octet) {
		case sizeof(uint8_t):	*(uint8_t  *)pValue32 = (uint8_t ) valueInv;	break;
		case sizeof(uint16_t):	*(uint16_t *)pValue32 = (uint16_t) valueInv;	break;
		case sizeof(uint32_t):	*(uint32_t *)pValue32 = (uint32_t) valueInv;	break;
		default:								break;
	}
}

/***************************************************************************************************
 * Private Functions definition
 */
