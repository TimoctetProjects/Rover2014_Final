/*********************************************************************
 * *******************************************************************
 *
 * @file	util_FIFO.c
 *
 * @author	Duclos Timothe
 *
 * @version BETA
 *
 * @date	16/10/2014
 *
 * @brief	Utilitaire de gestion des Fifos
 *
 *********************************************************************/

/********************************************************************
 * Includes
 */
#include "util_FIFO.h"

/********************************************************************
 * Private macro
 */
#define __FIFO_IsInitValid(FiFo) \
	( FiFo == NULL)

#define __FIFO_PushData(FIFO, data, offset)	\
	( FIFO->pBuffer[FIFO->pIndexIn ++] = (uint8_t) *( (uint8_t*)data + offset ) )

#define __FIFO_PullData(FIFO) \
	FIFO->pBuffer[FIFO->pIndexOut]

/********************************************************************
 * Exported Fonctions
 */
/**-----------------------------------------------------------------
 * @brief	Initialiser une Fifo
 * @return	Statut de l'initialisation
 */
Status_e FiFo_init(
		FiFo_s* pNew_FIFO,	/**<[in] Pointeur vers la FIFO. */
		uint32_t FiFo_Size,	/**<[in] Taille de la FIFO. */
		uint32_t Data_Size  /**<[in] Nombre de paquets de 8 bits, exprime en octet. */
) {

	if __FIFO_IsInitValid(pNew_FIFO)		 return STATUS_KO;

	pNew_FIFO->pIndexIn	= 0;
	pNew_FIFO->pIndexOut	= 0;
	pNew_FIFO->FifoSize	= FiFo_Size;
	pNew_FIFO->DataSize	= Data_Size;
	pNew_FIFO->State	= FiFoState_Empty;

	pNew_FIFO->pBuffer = calloc(FiFo_Size, Data_Size);
	memset(pNew_FIFO->pBuffer, 0, FiFo_Size);

	return STATUS_OK;
}

/**-----------------------------------------------------------------
 * @brief	Depilation d'un message dans une fifo
 * @return	Statut de la depilation
 */
FiFo_State_e
FiFo_Pull(
		FiFo_s* pFIFO, 	/**<[in] Fifo a initialiser */
		void*	pMsg	/**<[in] Message vide. */
) {

	uint32_t b_retrait_donnee;

	// On vérifie qu'il y a des donnees
	if (pFIFO->State == FiFoState_Empty)
				return FiFoState_Empty;


	// Retrait de la donnee en fifo
	for (b_retrait_donnee=0; b_retrait_donnee<pFIFO->DataSize; b_retrait_donnee++)	{

		 *((uint8_t*)pMsg + b_retrait_donnee) = __FIFO_PullData(pFIFO);
		 pFIFO->pBuffer[pFIFO->pIndexOut] = 0;
		 pFIFO->pIndexOut++;
	}

	// Incrémentation de l'index de sortie
	if (pFIFO->pIndexOut >= pFIFO->FifoSize * pFIFO->DataSize)
				pFIFO->pIndexOut = 0;


	// MAJ Statut
	if (pFIFO->pIndexIn == pFIFO->pIndexOut) {
				pFIFO->State = FiFoState_Empty;
				return FiFo_LastElement;
	}

	else		pFIFO->State = FiFo_DataDispo;

	return pFIFO->State;
}

/**-----------------------------------------------------------------
 * @brief	Empilation d'un message dans une fifo
 * @return	Statut de l'empilation
 */
Status_e
FiFo_Push(
		FiFo_s* pFIFO,		/**<[in] Fifo a vider */
		...					/**<[in] Donnee a ajouter. */
) {
	//----------------------------------------------------------------
	//---------------------------- Declaration des variables
	uint32_t b_ajout_donnee;

	//---------------------------------------------------------------
	//-------------  Récupération du pointeur vers la donnee a empiler
	// Lecture de l'argument situe apres pFiFo
	register int* varg = (int*) (&pFIFO+1);

	// On garde la meme adresse mais tronque la longueur du pointeur a 8bits
	uint8_t* pData = (uint8_t*) (varg);

	// Verification Que la FiFo peut encaisser une nouvelle donnee
	if (pFIFO->State == FiFoState_Full)
			return FiFoState_Full;


	// Ajout de la donnee en fifo
	for (b_ajout_donnee=0; b_ajout_donnee<pFIFO->DataSize; b_ajout_donnee++) {

		__FIFO_PushData(pFIFO, pData, b_ajout_donnee);
	}

	// Incrémentation de l'index d'entree
	if (pFIFO->pIndexIn >= (pFIFO->FifoSize * pFIFO->DataSize) )
			pFIFO->pIndexIn = 0;


	// MAJ Statut
	if (pFIFO->pIndexIn == pFIFO->pIndexOut) {
			pFIFO->State = FiFoState_Full;
			return FiFo_LastElement;
	}

	else	pFIFO->State = FiFo_DataDispo;


	return pFIFO->State;
}

/**-----------------------------------------------------------------
 * @brief	RaZ d'une Fifo
 * @return	void
 */
void
FiFo_Clear(
		FiFo_s* pFifo		/**<[in] Fifo a vider */
) {

	pFifo->pIndexIn		= 0;
	pFifo->pIndexOut	= 0;
	pFifo->DataSize		= 0;
	pFifo->FifoSize		= 0;
	pFifo->State		= FiFoState_Empty;

	free(pFifo->pBuffer);
}


/**-----------------------------------------------------------------
 * @brief	Lire l'etat d'une FiFo
 * @return	void
 */
FiFo_State_e
FiFo_GetState(
		FiFo_s* pFifo		/**<[in] FiFo dont on veut lire l'etat */
) {
	return pFifo->State;
}
