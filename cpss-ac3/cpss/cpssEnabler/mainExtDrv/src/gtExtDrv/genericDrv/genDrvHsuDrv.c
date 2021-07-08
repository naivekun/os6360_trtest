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
* @file genDrvHsuDrv.c
*
* @brief Includes HSU function wrappers, for HSU functionality
*
* @version   5
********************************************************************************
*/
#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/pssBspApis.h>
/**
* @internal extDrvHsuMemBaseAddrGet function
* @endinternal
*
* @brief   Get start address of HSU area
*
* @param[out] hsuPtrAddr               - Pointer address to beginning of HSU area
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvHsuMemBaseAddrGet
(
    OUT  GT_U32  **hsuPtrAddr
)
{
#if ((!defined ASIC_SIMULATION) && ((defined XCAT_DRV) || (defined GDA8548_DRV)))
  /* allocate 8Mbyte continuous memory for HSU needs */
  *hsuPtrAddr =(GT_U32*)(bspHsuMalloc(8*1024*1024));
    #endif
  return GT_OK;
}

/**
* @internal extDrvHsuWarmRestart function
* @endinternal
*
* @brief   Performs warm restart of the 8548 cpu
*/

GT_VOID extDrvHsuWarmRestart
(
    GT_VOID
)
{
#if ((!defined ASIC_SIMULATION) && ((defined XCAT_DRV) || (defined GDA8548_DRV)))
    bspWarmRestart();
    #endif
}

/**
* @internal extDrvHsuInboundSdmaEnable function
* @endinternal
*
* @brief   This routine enables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaEnable
(
    GT_VOID
)
{
#if ((!defined ASIC_SIMULATION) && (defined GDA8548_DRV))
  bspInboundSdmaEnable();
#endif
  return GT_OK;
}

/**
* @internal extDrvHsuInboundSdmaDisable function
* @endinternal
*
* @brief   This routine disables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaDisable
(
    GT_VOID
)
{
#if ((!defined ASIC_SIMULATION) && (defined GDA8548_DRV))
  bspInboundSdmaDisable();
#endif
  return GT_OK;
}

/**
* @internal extDrvHsuInboundSdmaStateGet function
* @endinternal
*
* @brief   This routine gets the state of cpu inbound sdma access.
*
* @param[out] enablePtr                - (pointer to) inbound sdma status .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaStateGet
(
    OUT GT_BOOL *enablePtr
)
{
#if ((!defined ASIC_SIMULATION) && (defined GDA8548_DRV))
    *enablePtr = bspInboundSdmaStateGet();
    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
}


