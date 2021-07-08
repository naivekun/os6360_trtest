/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/

#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#else
#include <common.h>
#endif

#ifndef MV_HWS_FREE_RTOS
/* Note: regarding the return code in case of an error of cpssSystemRecoveryStateGet,
   if the calling function is the semaphore functions that their return code is
   GT_BOOL, we cannot return the tmpRc in case of an error, as it does not match the
   GT_BOOL type. So, macro is supplied with _rcError. If we do use it in GT_STATUS
   return code type function, we can supply tmpRc as _rcError, then the function
   cpssSystemRecoveryStateGet error code will be returned as is */
#define SYSTEM_RECOVERY_STATE_CHECK_AND_SKIP_MAC(_rcSkip, _rcError)   \
    do{                                                               \
        CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;                \
        GT_STATUS tmpRc;                                              \
        /* holds system recovery information */                       \
        tmpRc = cpssSystemRecoveryStateGet(&system_recovery);         \
        if (tmpRc != GT_OK)                                           \
        {                                                             \
            HWS_LOG_ERROR_AND_RETURN_MAC(_rcError,                    \
                LOG_ARG_STRING_MAC("hw sema system recovery failed"), \
                LOG_ARG_GEN_PARAM_MAC(tmpRc));                        \
        }                                                             \
        SYSTEM_STATE_CAUSED_SKIP_AND_RETURN_MAC(_rcSkip);             \
    } while (0)
#else
#define SYSTEM_RECOVERY_STATE_CHECK_AND_SKIP_MAC(_rcSkip, _rcError)
#endif /* MV_HWS_FREE_RTOS */

/**
* @internal mvPexRead function
* @endinternal
*
* @brief   Read internal PCI register.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPexRead(GT_U8 devNum, GT_U32 addr, GT_U32 *dataPtr)
{
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    CHECK_STATUS(cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, addr, dataPtr));
    return GT_OK;
#else
    *dataPtr = MV_REG_BYTE_READ(addr);
    return GT_OK;
#endif
}

/**
* @internal mvPexWrite function
* @endinternal
*
* @brief   Write data into internal PCI register.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPexWrite(GT_U8 devNum, GT_U32 addr, GT_U32 data)
{
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    return cpssDrvPpHwInternalPciRegWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, addr, data);
#else
    MV_REG_BYTE_WRITE(addr, data);
    return GT_OK;
#endif
}

/**
* @internal mvMGRead function
* @endinternal
*
* @brief   Read register from the switching core MG unit.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvMGRead(GT_U8 devNum, GT_U32 addr, GT_U32 *dataPtr)
{
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    CHECK_STATUS(cpssDrvPpHwRegisterRead(devNum, 0 , addr, dataPtr));
#else
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, addr, dataPtr, 0xFFFFFFFF/*mask*/));
#endif
    return GT_OK;
}

/**
* @internal mvMGWrite function
* @endinternal
*
* @brief   Write data into the switching core MG unit.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvMGWrite(GT_U8 devNum, GT_U32 addr, GT_U32 data)
{
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    CHECK_STATUS(cpssDrvPpHwRegisterWrite(devNum, 0, addr, data));
#else
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, addr, data, 0xFFFFFFFF/*mask*/));
#endif
    return GT_OK;
}

/**
* @internal mvSemaLock function
* @endinternal
*
* @brief   Aquiring the HW semaphore lock.
*         Locking operation:
*         In Aldrin and above- we need to write the CPU-ID explicitly and then read the value, if the writting succeeded and
*         the value we read is the value we wrote, it means that the lock operation succeeded. Else, the lock is
*         allready locked by someone else and we will keep performing the locking operation until success.
*         In BobK and below- the read operation will return the CPU Id of the CPU who hold the lock without explicitly writing
*         the value to the semaphore. If the value is our CPU Id, the lock operation succeeded. Else, the lock is
*         allready locked by someone else and we will keep performing the locking operation until success.
*         In Bobk, A semphor can be locked by the MSYS CPU or an external IO device. For the MSYS CPU, the ID is
*         the CPU\Core Id, and 0x4 is for any other external IO device.
*         In case of a read\write error we will keep performing the operation and not return as the lock request
*         should return only on successfull lock state.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvSemaLock(GT_U8 devNum, GT_U32 num)
{
#ifndef BV_DEV_SUPPORT
    GT_U32 tmp=0;
    GT_STATUS rc;
    GT_U32 cpuId;

    if (num > MV_MAX_SEMA)
    {
        hwsOsPrintf("Error: Invalid semaphore number (%d)\n", num);
        return GT_FALSE;
    }


#ifdef MV_HWS_FREE_RTOS
    #ifdef MV_MSYS_BOBK
        cpuId = 0;
    #else
        cpuId = 4;
    #endif
#else
            /* CPSS */
    if (hwsDeviceSpecInfo[devNum].devType < Aldrin){
        cpuId = (prvCpssDrvPpConfig[devNum]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E)? 0 : 4;
    }
    else
        cpuId = 0;
#endif

    /* in system recovery state, register write operation not performed so
       HW semaphore mecanism shuold be skipped*/
    SYSTEM_RECOVERY_STATE_CHECK_AND_SKIP_MAC(GT_TRUE/*return code in case of skip*/,
                                             GT_FALSE/*return code in case of error*/);

    do
    {
        /* In Aldrin and above the semaphore is in the switching core MG unit, In Bobk is in the internal PCI / CnM unit. */
        if (hwsDeviceSpecInfo[devNum].devType >= Aldrin)
        {
            /* Manually IDs assignment for non MSYS semaphore.
               For CM3 the value will be 4, for non CM3 the values is 0. */
            rc = mvMGWrite(devNum, MV_SEMA_REG_BASE_SWITCH_MG + num*4, cpuId);
            if (rc!=GT_OK)
            {
                hwsOsPrintf("Error: Failed to write semaphore value. (%d)\n", rc);
            }
            rc = mvMGRead(devNum, MV_SEMA_REG_BASE_SWITCH_MG + num*4, &tmp);
            if (rc!=GT_OK)
            {
                hwsOsPrintf("Error: Failed to read semaphore value. (%d)\n", rc);
            }
        }
        else
        {
            rc = mvPexRead(devNum, MV_SEMA_REG_BASE_CNM_PCI + num*4, &tmp);
            if (rc!=GT_OK)
            {
                hwsOsPrintf("Error: Failed to read semaphore value. (%d)\n", rc);
            }
        }
    } while (((tmp & 0xFF) != cpuId) && ((tmp & 0xFF) != ((cpuId == 0) ? 1 : cpuId)));
#endif /* !defined(BV_DEV_SUPPORT) */
    return GT_TRUE;
}

/**
* @internal mvSemaTryLock function
* @endinternal
*
* @brief   One shot try for locking the HW semaphore lock.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvSemaTryLock(GT_U8 devNum, GT_U32 num)
{
#ifndef BV_DEV_SUPPORT
    GT_U32 tmp;
    GT_STATUS rc;
    GT_U32 cpuId;

    if (num > MV_MAX_SEMA)
    {
        hwsOsPrintf("Error: Invalid semaphore number (%d)\n", num);
        return GT_FALSE;
    }

#ifdef MV_HWS_FREE_RTOS
    #ifdef MV_MSYS_BOBK
        cpuId = 0;
    #else
        cpuId = 4;
    #endif
#else
            /* CPSS */
    if (hwsDeviceSpecInfo[devNum].devType < Aldrin){
        cpuId = (prvCpssDrvPpConfig[devNum]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E)? 0 : 4;
    }
    else
        cpuId = 0;
#endif

    /* in system recovery state, register write operation not performed so
       HW semaphore mecanism shuold be skipped*/
    SYSTEM_RECOVERY_STATE_CHECK_AND_SKIP_MAC(GT_TRUE/*return code in case of skip*/,
                                             GT_FALSE/*return code in case of error*/);

    /* In Aldrin and above the semaphore is in the switching core MG unit, In Bobk is in the internal PCI / CnM unit. */
    if (hwsDeviceSpecInfo[devNum].devType >= Aldrin)
    {
        /* Manually IDs assignment for non MSYS semaphore.
           For CM3 the value will be 4, for non CM3 the values is 0. */
        rc = mvMGWrite(devNum, MV_SEMA_REG_BASE_SWITCH_MG + num*4, 1);
        if (rc!=GT_OK)
        {
            hwsOsPrintf("Error: Failed to write semaphore value. (%d)\n", rc);
            return GT_FALSE;
        }
        rc = mvMGRead(devNum, MV_SEMA_REG_BASE_SWITCH_MG + num*4, &tmp);
        if (rc!=GT_OK)
        {
            hwsOsPrintf("Error: Failed to read semaphore value. (%d)\n", rc);
            return GT_FALSE;
        }
    }
    else
    {
        rc = mvPexRead(devNum, MV_SEMA_REG_BASE_CNM_PCI + num*4, &tmp);
        if (rc!=GT_OK)
        {
            hwsOsPrintf("Error: Failed to read semaphore value. (%d)\n", rc);
            return GT_FALSE;
        }
    }
    if (((tmp & 0xFF) != cpuId) && ((tmp & 0xFF) != ((cpuId == 0) ? 1 : cpuId)))
    {
        return GT_FALSE;
    }
    else
    {
        return GT_TRUE;
    }
#else /* defined(BV_DEV_SUPPORT) */
    return GT_TRUE;
#endif
}

/**
* @internal mvSemaUnlock function
* @endinternal
*
* @brief   Releasing the HW semaphore lock.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvSemaUnlock(GT_U8 devNum, GT_U32 num)
{
    if (num > MV_MAX_SEMA)
    {
        hwsOsPrintf("Error: Invalid semaphore number (%d)\n", num);
        return GT_FALSE;
    }

    /* in system recovery state, register write operation not performed so
       HW semaphore mecanism shuold be skipped*/
    SYSTEM_RECOVERY_STATE_CHECK_AND_SKIP_MAC(GT_TRUE/*return code in case of skip*/,
                                             GT_FALSE/*return code in case of error*/);

#ifndef BV_DEV_SUPPORT
    /* In Aldrin and above the semaphore is in the switching core MG unit, In Bobk is in the internal PCI / CnM unit. */
    if (hwsDeviceSpecInfo[devNum].devType >= Aldrin)
    {
        CHECK_STATUS(mvMGWrite(devNum, MV_SEMA_REG_BASE_SWITCH_MG + num*4, 0xFF));
    }
    else
    {
        CHECK_STATUS(mvPexWrite(devNum, MV_SEMA_REG_BASE_CNM_PCI + num*4, 0xFF));
    }
#endif
    return GT_TRUE;
}

/**
* @internal mvSemaOwnerGet function
* @endinternal
*
* @brief   Get the semaphore onwer.
*
* @param[out] semaOwner            - pointer to semaphore owner
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvSemaOwnerGet(GT_U8 devNum, GT_U32 *semaOwner)
{
    GT_STATUS rc;

    /* read semaphore register */
    rc = mvMGRead(devNum, MV_SEMA_REG_BASE_SWITCH_MG + MV_SEMA_AVAGO*4, semaOwner);
    if (rc!=GT_OK)
    {
        hwsOsPrintf("Failed to read semaphore owner - return code (%d)\n", rc);
        return GT_FAIL;
    }

    return GT_OK;
}



