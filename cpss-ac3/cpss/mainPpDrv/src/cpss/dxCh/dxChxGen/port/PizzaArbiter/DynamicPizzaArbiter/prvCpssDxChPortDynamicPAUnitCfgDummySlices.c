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
* @file prvCpssDxChPortDynamicPAUnitCfgDummySlices.c
*
* @brief pizza algorithm with dummy slices
*
* add number of 1G dummy ports up to BW
* Configure pizza using base DMA pizza algorithm.
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


GT_STATUS BuildPizzaDistributionWithDummySlice
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
    GT_U32      totalDummyPorts;
    GT_U32      dummyPortIdx;
    GT_U32     *dummyPortNumMapPtr;


    if (maxPipeSize == 0) /* nothing to configure */
    {
        goto nothing_to_configure;
    }

    dummyPortNumMapPtr = cpssOsMalloc(PRV_CPSS_MAX_PP_PORTS_NUM_CNS * sizeof(*dummyPortNumMapPtr));
    if (dummyPortNumMapPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        dummyPortNumMapPtr[i] = PA_INVALID_PORT_CNS;
    }
    /*-----------------------------------------*
     *  calculate bw to be allocated          *
     *-----------------------------------------*/
    totalBandwidth = 0;
    for (i = 0 ; i < size ; i++)
    {
        totalBandwidth += (GT_U32)(portsConfigArrayPtr[i]);
    }

    if(maxPipeSize < totalBandwidth)
    {
        cpssOsFree(dummyPortNumMapPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "\navailble maxPipeSize(%d) < summary required Bandwidth(%d) in MBPS\n",
            maxPipeSize, totalBandwidth);
    }

    /*-----------------------------------------*
     *  compute number of dummy ports          *
     *-----------------------------------------*/
    if ((maxPipeSize - totalBandwidth) % minSliceResolution != 0)
    {
        cpssOsFree(dummyPortNumMapPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* problem with resolution, something wrong in the system  */
    }
    totalDummyPorts = (maxPipeSize - totalBandwidth)/minSliceResolution;
    /*------------------------------------------------*
     *  Select dummy ports to be the ports with 0 BW  *
     *   set their BW as minSliceResolution            *
     *------------------------------------------------*/
    if (totalDummyPorts > 0)
    {
        dummyPortIdx = 0;
        for (i = 0 ; i < size ; i++)
        {
            if (portsConfigArrayPtr[i] == 0)   /* set it to be dummy */
            {
                portsConfigArrayPtr[i] = minSliceResolution;
                dummyPortNumMapPtr[i] = i; /* set  map : port i is valid , its value - i -- port number */
                dummyPortIdx++;
                if (dummyPortIdx == totalDummyPorts)
                {
                    break;
                }
            }
        }
    }
    /*-----------------------------------------*
     *  compute pizza                          *
     *-----------------------------------------*/
    rc = BuildPizzaDistribution(IN   portsConfigArrayPtr,
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
        cpssOsFree(dummyPortNumMapPtr);
        return rc;
    }
    /*-----------------------------------------*
     *  Make dummy ports' slices to be invalid *
     *-----------------------------------------*/
    if (totalDummyPorts > 0)
    {
        GT_U32 sliceIdx;
        GT_U32 portNum;
        for (sliceIdx = 0 ; sliceIdx < *numberOfSlicesPtr ; sliceIdx++)
        {
            portNum = pizzaArray[sliceIdx];
            if (dummyPortNumMapPtr[portNum] == portNum) /* really dummy port */
            {
                pizzaArray[sliceIdx] = PA_INVALID_PORT_CNS;
            }
        }
    }

    cpssOsFree(dummyPortNumMapPtr);
    return rc;

nothing_to_configure:
    *numberOfSlicesPtr = 0;
    for (i = 0; i < pizzaArraySize ; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    return GT_OK;
}





