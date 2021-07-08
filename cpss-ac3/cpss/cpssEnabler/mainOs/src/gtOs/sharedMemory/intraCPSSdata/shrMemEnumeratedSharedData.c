/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSem.h>



#ifdef SHARED_MEMORY

static uintptr_t sharedDataArray[CPSS_SHARED_DATA_MAX_E];
GT_SEM semMtx = 0;

/**
* @internal cpssMultiProcGetSharedData function
* @endinternal
*
* @brief   return enumerated shared data
*
* @param[in] id                       - data identifier
*                                       data (integer or pointer)
*/
uintptr_t cpssMultiProcGetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id
)
{
    if (id < 0 || id >= CPSS_SHARED_DATA_MAX_E)
    {
        return 0;
    }
    return sharedDataArray[id];
}

/**
* @internal cpssMultiProcSetSharedData function
* @endinternal
*
* @brief   Set enumerated shared data
*
* @param[in] id                       - data identifier
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cpssMultiProcSetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id,
    IN uintptr_t data
)
{
    if (id < 0 || id >= CPSS_SHARED_DATA_MAX_E)
    {
        return GT_BAD_PARAM;
    }
    sharedDataArray[id] = data;
    return GT_OK;
}


/**
* @internal cpssMultiProcSharedDataLock function
* @endinternal
*
* @brief   Lock shared data for critical operations (data alloc)
*/
GT_STATUS cpssMultiProcSharedDataLock(void)
{
    if (!semMtx)
    {
        /* not initialized yet */
        if (osSemBinCreate("MP_EnumeratedMtx", OS_SEMB_FULL, &semMtx) != GT_OK)
        {
            return GT_FAIL;
        }
    }
    return osSemWait(semMtx, OS_WAIT_FOREVER);
}

/**
* @internal cpssMultiProcSharedDataUnlock function
* @endinternal
*
* @brief   Unlock shared data
*/
GT_STATUS cpssMultiProcSharedDataUnlock(void)
{
    if (!semMtx)
    {
        return GT_FAIL;
    }
    return osSemSignal(semMtx);
}

#endif




