/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtExtDrv/drivers/gtPciDrv.h>

#include <asicSimulation/SCIB/scib.h>
#ifndef SHARED_MEMORY
#include <extUtils/IOStream/IOStream.h>
#endif
#ifdef ASIC_SIMULATION
extern void SHOSTG_reset (unsigned int status);
#endif

#ifndef SHARED_MEMORY
extern IOStreamPTR defaultStdio;
#endif
/**
* @internal extDrvResetInit function
* @endinternal
*
* @brief   This routine calls in init to do system init config for reset.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvResetInit
(
)
{

    return GT_OK;
}

/**
* @internal extDrvReset function
* @endinternal
*
* @brief   This routine calls to reset of CPU.
*
* @retval GT_FAIL                  - always.
*/
GT_STATUS extDrvReset
(
)
{
#ifdef ASIC_SIMULATION
    GT_STATUS rc = GT_OK;
#ifndef SHARED_MEMORY
    if (defaultStdio != NULL)
    {
        defaultStdio->destroy(defaultStdio);
    }
#endif

    SCIB_SEM_TAKE;
    rc = osBindStdOut(NULL,NULL);
    SCIB_SEM_SIGNAL;
    if (rc != GT_OK)
    {
        osPrintf("bad bind\n");
    }

    SHOSTG_reset(0);
#endif
    return GT_FAIL;
}




