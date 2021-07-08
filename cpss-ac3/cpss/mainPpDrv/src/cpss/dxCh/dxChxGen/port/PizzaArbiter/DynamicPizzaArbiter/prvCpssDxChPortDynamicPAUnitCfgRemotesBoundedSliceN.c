/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChPortDynamicPAUnitCfgRemotesBoundedSliceN.c
*
* @brief AC3X TXQ specific pizza algorithm (with remote ports)
* If we under TXQ BW,
* configure as is using dummy slices addition to pizza algorithm.
* If we above TXQ BW,
* reduce BW on all ports with BW 10G and above in relative fashion.
* Configure pizza using dummy slices addition to pizza algorithm.
*
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPATypeDef.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>


GT_STATUS BuildPizzaDistributionTxQRemotesBoundedSliceN
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr,
    OUT  GT_U32      *highSpeedPortNumPtr,
    OUT  GT_U32       highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      totalBandwidth = 0;
    GT_U32      total_belowBwTH_BW;
    GT_U32      port_BW_Thr_Mbps = 10000;
    GT_U32      portBW, adjustedPortBW;
    GT_U32      requiredBW, freeBW;
    GT_U32      sliceN;

    /*-----------------------------------------*
     *  calculate bw to be allocated           *
     *-----------------------------------------*/
    totalBandwidth = 0;
    total_belowBwTH_BW = 0;
    for (i = 0 ; i < size ; i++)
    {
        portBW = (GT_U32)(portsConfigArrayPtr[i]);

        totalBandwidth += portBW;
        if (portBW < port_BW_Thr_Mbps)
        {
            total_belowBwTH_BW += portBW;
        }
    }
    /*--------------------------------------------------------*
     *  if the BW does not reach maximum,                     *
     *     just configure with dummy slice                    *
     *   else                                                 *
     *      lower ports with BW >= 10G on relative difference *
     *      and adjust to resolution                          *
     *--------------------------------------------------------*/
    if (totalBandwidth > maxPipeSize)  /* we have don't have enough slices, adjust BW for big ports */
    {
        if (total_belowBwTH_BW > maxPipeSize)
        {
            cpssOsPrintf("\nERROR : bw of remotes and 1G ports (%d) > than maxPipeSize(%d)",total_belowBwTH_BW,maxPipeSize);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bw of remotes and 1G ports (%d) > than maxPipeSize(%d)",total_belowBwTH_BW,maxPipeSize);
        }

        requiredBW = totalBandwidth - total_belowBwTH_BW;
        freeBW     = maxPipeSize    - total_belowBwTH_BW;
        /*-----------------------------------------*
         *  calculate numbers to be allocated     *
         *-----------------------------------------*/

        for (i = 0 ; i < size ; i++)
        {
            portBW = (GT_U32)(portsConfigArrayPtr[i]);

            if (portBW >= port_BW_Thr_Mbps)  
            {
                adjustedPortBW = (portBW/1000 * freeBW)/(requiredBW/1000);
                sliceN = adjustedPortBW/minSliceResolution;
                if (sliceN == 0)
                {
                    sliceN = 1;
                }
                portsConfigArrayPtr[i]  = sliceN*minSliceResolution;
            }
        }
    }

    rc = BuildPizzaDistributionWithDummySlice(IN   portsConfigArrayPtr,
                                              IN   size,
                                              IN   maxPipeSize,
                                              IN   minSliceResolution,
                                              IN   highSpeedPortThreshPtr,
                                              IN   pizzaArraySize,
                                              OUT  pizzaArray,
                                              OUT  numberOfSlicesPtr,
                                              OUT  highSpeedPortNumPtr,
                                              OUT  highSpeedPortArr);

    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}





