      
/*******************************************************************************
*
*                   Copyright 2002, GALILEO TECHNOLOGY, LTD.
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
*
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES,
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.
* (MJKK), GALILEO TECHNOLOGY LTD. (GTL) AND GALILEO TECHNOLOGY, INC. (GTI).
*
*******************************************************************************
*/
/**
********************************************************************************
* @file gtDpssGlue.h
*
* @brief Initialize the DPSS and get information from it.
*
* @version   1.1.2.2
********************************************************************************
*/

#ifndef __gtDpssGlueh
#define __gtDpssGlueh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/********* include ***********************************************************/
#include <gtOs/gtGenTypes.h>

 
/******** typedefs ***********************************************************/

/**
* @internal extDpssIsSysDistributed function
* @endinternal
*
* @brief   If the system is distrebuted then return GT_TRUE other wise
*         return GT_FALSE.
*
* @retval GT_TRUE                  - Disterbuted system
* @retval GT_FALSE                 - Non disterbuted system.
*/
GT_BOOL extDpssIsSysDistributed
(
    GT_VOID
);

/**
* @internal extDpssInit function
* @endinternal
*
* @brief   Initialize all the modules in the DPSS.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS extDpssInit
(
    GT_VOID
);

 
/**
* @internal extDpssIsUnitMaster function
* @endinternal
*
* @brief   Returns the state of the unit master or slave
*
* @retval GT_TRUE                  - The unit is master.
* @retval GT_FALSE                 - The unit is slave.
*
* @note If this function is not implemented it should return GT_TRUE.
*
*/
GT_BOOL extDpssIsUnitMaster
(
    GT_VOID
);

 
/**
* @internal extDpssWaitForCfg function
* @endinternal
*
* @brief   Wait for the configuration to come from the master. After the
*         configuration came from the master then we can move to core
*         phase 2 init. The function is blocking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note The configuration from the master contains several parameters. One
*       of the parameters is the new PP dev num, this information is needed
*       for the core phase 2 init.
*       If this function is not implemented it should return GT_OK.
*
*/
GT_STATUS extDpssWaitForCfg
(
    GT_VOID
);

/**
* @internal extDpssWaitForMasterCfg function
* @endinternal
*
* @brief   Wait for all the slaves to finish there configuration. After all
*         the slaves finished ther configuration we can move to logical
*         configuration
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS  extDpssWaitForMasterCfg
(
    GT_VOID
);

/**
* @internal extDpssConvTblLock function
* @endinternal
*
* @brief   Locks the conversion table for reading.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note If this function is not implemented it should return GT_OK.
*
*/
GT_STATUS extDpssConvTblLock
(
    GT_VOID
);

/**
* @internal extDpssConvTblUnLock function
* @endinternal
*
* @brief   Unlocks the conversion table for reading.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note If this function is not implemented it should return GT_OK.
*
*/
GT_STATUS extDpssConvTblUnLock
(
    GT_VOID
);


/**
* @internal extDpssUpdateConvTbl function
* @endinternal
*
* @brief   Update the conversion table with the new values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note The data of the new values is stored in the USS and this function
*       will only trigger the USS to save the new data in the conversion
*       table.
*       If this function is not implemented it should return GT_OK.
*
*/
GT_STATUS extDpssUpdateConvTbl
(
    GT_VOID
);

 
/**
* @internal extDpssGetPpConvTbl function
* @endinternal
*
* @brief   Get the dev num conversion table from physical to logical.
*         This data is returned from the USS. The USS received the data
*         from the configuration message that came from the master.
*
* @param[out] devConvTbl               : Pointer to an array. The index of the array is the
*                                      physical dev number of the PP, the value in each cell is
*                                      the logical dev number of the PP.
* @param[out] devConvTblLen            : The number of elements in the array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note If this function is not implemented then it should return an array.
*       of the new PP dev nums. The array should be allocated and not a
*       local function array.
*
*/
GT_STATUS extDpssGetPpConvTbl
(
    OUT GT_U8 **devConvTbl,
    OUT GT_U8 *devConvTblLen
);

/**
* @internal extDpssIsIpcEnabled function
* @endinternal
*
* @brief   If the IPC is enabled return GT_TRUE other wise
*         return GT_FALSE.
*
* @retval GT_TRUE                  - IPC enabled
* @retval GT_FALSE                 - IPC disabled.
*/
GT_BOOL extDpssIsIpcEnabled
(
    GT_VOID
);

/**
* @internal extDpssIpcSlaveInit function
* @endinternal
*
* @brief   Initialize the IPC slave
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - Operation failed.
*/
GT_STATUS extDpssIpcSlaveInit
(
    GT_VOID
);

/**
* @internal extDpssIpcMasterInit function
* @endinternal
*
* @brief   Initialize the IPC master
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - Operation failed.
*/
GT_STATUS extDpssIpcMasterInit
(
    GT_VOID
);

/**
* @internal extDpssInitSysPpParams function
* @endinternal
*
* @brief   Initializes all the PPs logical device numbers and unit IDs.
*         This is needes before TAPI init in the master.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - Operation failed.
*/
GT_STATUS extDpssInitSysPpParams
(
	GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtDpssGlueh */


