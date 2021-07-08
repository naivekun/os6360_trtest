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
* @file win32DragoniteDrv.c
*
* @brief Includes DRAGONITE functions wrappers
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <gtExtDrv/drivers/pssBspApis.h>

/**
* @internal extDrvDragoniteSharedMemWrite function
* @endinternal
*
* @brief   Write a given buffer to the given address in shared memory of DRAGONITE
*         microcontroller.
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in words.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvDragoniteSharedMemWrite
(
    IN  GT_U32  offset,
    IN  GT_VOID *buffer,
    IN  GT_U32  length
)
{
    return bspDragoniteSharedMemWrite(offset, buffer, length);
}

/**
* @internal extDrvDragoniteSharedMemRead function
* @endinternal
*
* @brief   Read a data from the given address in shared memory of DRAGONITE microcontroller
*
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] length                   - Length of the memory block to read (in
*                                      words).
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvDragoniteSharedMemRead
(
    IN  GT_U32  offset,
    IN  GT_U32  length,
    OUT GT_VOID *buffer
)
{
    return bspDragoniteSharedMemRead(offset, buffer, length);
}

/**
* @internal extDrvDragoniteShMemBaseAddrGet function
* @endinternal
*
* @brief   Get start address of communication structure in DTCM
*
* @param[out] dtcmPtr                  - Pointer to beginning of DTCM, where
*                                      communication structures must be placed
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteShMemBaseAddrGet
(
    OUT  GT_U32  *dtcmPtr
)
{
    return bspDragoniteSharedMemoryBaseAddrGet(dtcmPtr);
}




