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
* @file gtDbDxBobcat2UtilsSimClock.h
*
* @brief Simulation CoreClock for BC2/BobK/Aldrin etc
*
* @version   3
********************************************************************************
*/
#ifndef __gtDbDxBobcat2UtilsSimClock_H
#define __gtDbDxBobcat2UtilsSimClock_H

#include <cpss/extServices/os/gtOs/gtEnvDep.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    GT_U32 coreClock;
    GT_U32 pll0Value;
}CORECOLOCK_PLL0_VALUE_STC;



GT_STATUS appDemoBcat2B0SimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
);


GT_STATUS appDemoBobKSimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
);

GT_STATUS appDemoAldrinSimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
);

GT_STATUS appDemoBc3SimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif



