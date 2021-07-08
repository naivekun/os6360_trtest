/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <private/simWin32/simDragonite/Globals.h>

#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsTask.h>
#include <os/simOsIniFile.h>

#ifndef DRAGONITE_TYPE_A1

GT_STATUS bspDragoniteSWDownload
(
    IN  const GT_VOID *sourcePtr,
    IN  GT_U32         size
)
{
    /* to avoid warnings */
    sourcePtr;
    size;

    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteEnableSet
(
    IN  GT_BOOL enable
)
{
    /* to avoid warnings */
    enable;

    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteInit
(
    GT_VOID
)
{
    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteSharedMemWrite
(
    IN  GT_U32         offset,
    IN  const GT_VOID *buffer,
    IN  GT_U32         length
)
{
    /* to avoid warnings */
    offset;
    buffer;
    length;

    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteSharedMemRead
(
    IN  GT_U32   offset,
    OUT GT_VOID *buffer,
    IN  GT_U32   length
)
{
    /* to avoid warnings */
    offset;
    buffer;
    length;

    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteSharedMemoryBaseAddrGet
(
    OUT GT_U32 *dtcmPtr
)
{
    /* to avoid warnings */
    dtcmPtr;

    return GT_NOT_SUPPORTED;
}

GT_STATUS bspDragoniteGetIntVec
(
    OUT GT_U32 *intVec
)
{
    /* to avoid warnings */
    intVec;

    return GT_NOT_SUPPORTED;
}

#else /* if DRAGONITE supported */

extern U32 Checksum(U8 *Ptr);

extern U32 intLine;

static GT_BOOL dragoniteSupported = GT_FALSE;
/**
* @internal bspDragoniteSWDownload function
* @endinternal
*
* @brief   Download new version of Dragonite firmware to Dragonite MCU
*
* @param[in] sourcePtr                - Pointer to memory where new version of Dragonite firmware resides.
* @param[in] size                     -  of firmware to download to ITCM.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteSWDownload
(
    IN  const GT_VOID *sourcePtr,
    IN  GT_U32         size
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;
    else
        return GT_OK;
}

/**
* @internal bspDragoniteEnableSet function
* @endinternal
*
* @brief   Enable/Disable DRAGONITE module
*
* @param[in] enable                   – GT_TRUE  – DRAGONITE MCU starts work with parameters set by application
*                                      GT_FALSE – DRAGONITE MCU stops function
*                                       GT_OK if successful, or
*
* @retval GT_INIT_ERROR            - Failed to create packetRxThread task
*
* @note call after SW download
*
*/
GT_STATUS bspDragoniteEnableSet
(
    IN  GT_BOOL enable
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;

    /* if irq flag is 1, then interrupt set - otherwise no interrupt */
    INT_POLARITY = 1;

    /* run communication protocol simulation task */
    if (simOsTaskCreate(6/*GT_TASK_PRIORITY_NORMAL*/,
                        Task_HostComm,
                        NULL) <= 0)
    {
        printf("Failed to create packetRxThread task!\r\n");
        return GT_INIT_ERROR;
    }

    return GT_OK;
}


/**
* @internal bspDragoniteInit function
* @endinternal
*
* @brief   Initialize DRAGONITE module
*
* @retval GT_BAD_PARAM             - DRAGONITE interrupt line not defined in simulation ini-file
*
* @note Application will call this before firmware download
*
*/
GT_STATUS bspDragoniteInit
(
    GT_VOID
)
{
    char param_str[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];

    /* appDemo uses appDemoDb to see if dragonite supported */
    dragoniteSupported = GT_TRUE;

    /* read DRAGONITE interrupt line number */
    if(simOsGetCnfValue("dragonite",  "dragonite_int_line",
                        SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &intLine);
    }
    else
    {
        return GT_BAD_PARAM;
    }

    /* set comm. protocol shared memory (not whole 8K) to 0xFF */
    memset(HOST_MSG_PTR, 0xFF, sizeof(t_HostMsg)); 

    return GT_OK;
}

/**
* @internal bspDragoniteSharedMemWrite function
* @endinternal
*
* @brief   Write a given buffer to the given offset in shared memory of DRAGONITE
*         microcontroller.
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in bytes.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*
* @note Only DTCM is reachable
*
*/
GT_STATUS bspDragoniteSharedMemWrite
(
    IN  GT_U32         offset,
    IN  const GT_VOID *buffer,
    IN  GT_U32         length
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;

    /* pay attention MAX_DATA_SIZE is not 8K it's 2304 */
    if (length > MAX_DATA_SIZE - offset) 
        return GT_BAD_PARAM;

    memcpy(HOST_MSG_PTR + offset, buffer, length);

    return GT_OK;
}

/**
* @internal bspDragoniteSharedMemRead function
* @endinternal
*
* @brief   Read a memory block from a given offset in shared memory of DRAGONITE
*         microcontroller.
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] length                   - Length of the memory block to read (in bytes).
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*
* @note Only DTCM is reachanble
*
*/
GT_STATUS bspDragoniteSharedMemRead
(
    IN  GT_U32   offset,
    OUT GT_VOID *buffer,
    IN  GT_U32   length
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;

    /* pay attention MAX_DATA_SIZE is not 8K it's 2304 */
    if (length > MAX_DATA_SIZE - offset) 
        return GT_BAD_PARAM;

    memcpy(buffer, HOST_MSG_PTR + offset, length);

    return GT_OK;
}

/**
* @internal bspDragoniteSharedMemoryBaseAddrGet function
* @endinternal
*
* @brief   Get start address of DTCM
*
* @param[out] dtcmPtr                  - Pointer to beginning of DTCM where communication structures
*                                      must be placed
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteSharedMemoryBaseAddrGet
(
    OUT GT_U32 *dtcmPtr
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;

    *dtcmPtr = (GT_U32)&IrqData;

    return GT_OK;
}

/**
* @internal bspDragoniteGetIntVec function
* @endinternal
*
* @brief   This routine return the DRAGONITE interrupt vector.
*
* @param[out] intVec                   - DRAGONITE interrupt vector.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteGetIntVec
(
    OUT GT_U32 *intVec
)
{
    if(!dragoniteSupported)
        return GT_NOT_SUPPORTED;

    *intVec = intLine;

    return GT_OK;
}

#endif 


