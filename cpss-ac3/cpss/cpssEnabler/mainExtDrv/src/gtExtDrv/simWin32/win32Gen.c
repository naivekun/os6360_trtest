/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file win32Gen.c
*
* @brief Generic extDrv functions
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/drivers/gtGenDrv.h>

/**
* @internal extDrvSoCInit function
* @endinternal
*
* @brief   Initialize SoC related drivers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note This API is called by after cpssExtServicesBind()
*
*/
GT_STATUS extDrvSoCInit(void)
{
    return GT_OK;
}

/**
* @internal extDrvBoardIdGet function
* @endinternal
*
* @brief   Get BoardId value
*
* @param[out] boardIdPtr               - boardId
*                                      None.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvBoardIdGet
(
    OUT GT_U32  *boardIdPtr
)
{
    /* This is a temporary solution in order to
     * enable CPSS software to get the Cetus
     * DB's board-id - '0x50'. (Please check
     * the board-id definitions in uBoot.
     * In the future, it is supposed to support
     * all board-ids of different switch devices.
     */
    *boardIdPtr = 0x50;
    return GT_OK;
}


