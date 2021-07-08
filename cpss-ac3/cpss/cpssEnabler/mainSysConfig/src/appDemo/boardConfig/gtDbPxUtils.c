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
* @file gtDbPxUtils.c
*
* @brief PX (PIPE) utils
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortAp.h>

#define BAD_VALUE (GT_U32)(~0)

GT_CHAR * CPSS_PX_MAPPING_2_STR
(
    CPSS_PX_PORT_MAPPING_TYPE_ENT mapEnm
)
{
    typedef struct
    {
        CPSS_PX_PORT_MAPPING_TYPE_ENT mapEnm;
        GT_CHAR                      *mapStr;
    }APPDEMO_MAPPING_2_STR_STC;


    static APPDEMO_MAPPING_2_STR_STC prv_mappingTypeStr[] =
    {
             { CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           "ETHERNET" }
            ,{ CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E,               "CPU-SDMA" }
            ,{ CPSS_PX_PORT_MAPPING_TYPE_INVALID_E,                "--------" }
    };
    GT_U32 i;
    for (i = 0 ; prv_mappingTypeStr[i].mapEnm != CPSS_PX_PORT_MAPPING_TYPE_INVALID_E; i++)
    {
        if (prv_mappingTypeStr[i].mapEnm == mapEnm)
        {
            return prv_mappingTypeStr[i].mapStr;
        }
    }
    return "--------";
}


GT_STATUS appDemoPipePortMappingDump
(
    IN  GT_SW_DEV_NUM  dev
)
{
    GT_STATUS   rc;         /* return code */
    GT_PHYSICAL_PORT_NUM portIdx;
    GT_U32 maxPortNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(dev);
    maxPortNum = PRV_CPSS_PX_PORTS_NUM_CNS;

    cpssOsPrintf("\n+------+-----------------+---------------------+--------+");
    cpssOsPrintf("\n| Port |   mapping Type  | mac txq rxdma txdma |   txQ  |");
    cpssOsPrintf("\n|      |                 |                     | dq loc |");
    cpssOsPrintf("\n+------+-----------------+---------------------+--------+");


    for (portIdx = 0 ; portIdx < maxPortNum; portIdx++)
    {
        static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
        CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
        GT_U32 dqIdx,      localTxqNum;




        rc = cpssPxPortPhysicalPortDetailedMapGet(dev,portIdx,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_TRUE)
        {
            prvCpssPxPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,/*OUT*/&dqIdx,  &localTxqNum);

            cpssOsPrintf("\n| %4d |",portIdx);
            cpssOsPrintf(" %-15s |",CPSS_PX_MAPPING_2_STR(portMapPtr->portMap.mappingType));
            cpssOsPrintf(" %3d",portMapPtr->portMap.macNum);
            cpssOsPrintf(" %3d",portMapPtr->portMap.txqNum);
            cpssOsPrintf(" %5d",portMapPtr->portMap.dmaNum);
            cpssOsPrintf(" %5d",portMapPtr->portMap.dmaNum);
            cpssOsPrintf(" | %2d %3d |",dqIdx,localTxqNum);
        }
    }
    cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+--------+");
    cpssOsPrintf("\n");
    return GT_OK;
}


/*-------------------------------------------------------------------------------
**     Pizza Arbiter State Print Function :
**         prints configuration and state of all slice for each unit
**         <Rx/Tx DMA DMA_CTU  BM TxQ
**-------------------------------------------------------------------------------*/


static void gtPipePortPizzaArbiterIfDumpSliceStatePrint(GT_U32 configuredSlices, GT_U32 sliceId, GT_BOOL isEnable, GT_U32 occupiedBy)
{
    if (GT_FALSE  != isEnable )
    {
        cpssOsPrintf(" %2d",occupiedBy);
    }
    else
    {
        if (sliceId < configuredSlices)
        {
            cpssOsPrintf(" %2s",".");
        }
        else
        {
            cpssOsPrintf(" %2s","x");
        }
    }
}

static void gtPipePortPizzaArbiterIfDumpUnitStatePrint
(
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC * pUnitPtr,
    char * msg
)
{
    GT_U32      sliceId;
    GT_U32      slicesNumberPerGop = 20;
    GT_U32      nonEmptySlices;

    /* count number of non-X assigned slices*/
    nonEmptySlices = 0;
    for (sliceId = 0 ; sliceId < pUnitPtr->totalSlicesOnUnit ; sliceId++)
    {
        if (pUnitPtr->slice_enable[sliceId] != GT_FALSE)
        {
            nonEmptySlices ++;
        }
    }

    cpssOsPrintf("\n    %s",msg);
    cpssOsPrintf("\n       Total/Configured Slices/of them assigned  = %d/%d/%d" , pUnitPtr->totalSlicesOnUnit,
                                                                        pUnitPtr->totalConfiguredSlices,
                                                                        nonEmptySlices);
    cpssOsPrintf("\n       Work Conserving Bit = %d",pUnitPtr->workConservingBit);
    if (pUnitPtr->totalSlicesOnUnit > 0)
    {
        cpssOsPrintf("\n       Slice to port map   : slice :");
        for (sliceId = 0 ; sliceId < slicesNumberPerGop ; sliceId++)
        {
            cpssOsPrintf(" %2d",sliceId);
        }
        cpssOsPrintf("\n                                   :");
        for (sliceId = 0 ; sliceId < slicesNumberPerGop ; sliceId++)
        {
            cpssOsPrintf("---");
        }


        for (sliceId = 0 ; sliceId < pUnitPtr->totalSlicesOnUnit ; sliceId++)
        {
            if (0 == sliceId % slicesNumberPerGop)
            {
                cpssOsPrintf("\n       Slice %4d : %4d   : port  :",sliceId, sliceId + slicesNumberPerGop -1 );
            }
            gtPipePortPizzaArbiterIfDumpSliceStatePrint(pUnitPtr->totalConfiguredSlices,sliceId,
                                                                      pUnitPtr->slice_enable[sliceId],
                                                                      pUnitPtr->slice_occupied_by[sliceId]);
        }
    }
}



typedef struct PA_UNIT_2_STR_STCT
{
    CPSS_PX_PA_UNIT_ENT  paUnit;
    GT_CHAR               *str;
}PA_UNIT_2_STR_STC;

static PA_UNIT_2_STR_STC pa_unit2str_List[] =
{
     { CPSS_PX_PA_UNIT_RXDMA_0_E,            "PA UNIT RXDMA_0"       }
    ,{ CPSS_PX_PA_UNIT_TXQ_0_E,              "PA UNIT TXQ_0"         }
    ,{ CPSS_PX_PA_UNIT_TXQ_1_E,              "PA UNIT TXQ_1"         }
    ,{ CPSS_PX_PA_UNIT_TXDMA_0_E,            "PA UNIT TXDMA_0"       }
    ,{ CPSS_PX_PA_UNIT_TX_FIFO_0_E,          "PA UNIT TX_FIFO_0"     }
    ,{ CPSS_PX_PA_UNIT_UNDEFINED_E,          (GT_CHAR *)NULL         }
};


static GT_CHAR * pipePaUnit22StrFind
(
    CPSS_PX_PA_UNIT_ENT  paUnit
)
{
    GT_U32 i;
    static GT_CHAR badUnitNameMsg[30];
    for (i = 0 ; pa_unit2str_List[i].paUnit != CPSS_PX_PA_UNIT_UNDEFINED_E; i++)
    {
        if (pa_unit2str_List[i].paUnit  == paUnit)
        {
            return pa_unit2str_List[i].str;
        }
    }
    cpssOsPrintf(badUnitNameMsg,"Unit %d : Unknown Name",paUnit);
    return &badUnitNameMsg[0];
}



GT_STATUS appDemoPipeRegAddrGet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32    offsInStr,
    OUT GT_U32  *regAddrPtr
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    GT_U8                                *regsAddrVer1_U8;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;
    regsAddrVer1_U8 = (GT_U8 *)regsAddrVer1;
    *regAddrPtr = (GT_U32)*((GT_U32*)&regsAddrVer1_U8[offsInStr]);
    return GT_OK;
}


GT_CHAR *  appDemoPipePizzaArbiterFldPrint
(
    GT_U32 offset,
    GT_U32 len
)
{
    static GT_CHAR str[20];
    cpssOsSprintf(&str[0],"%2d-%2d,%2d",offset, offset+ len-1,len);
    return &str[0];
}


/**
* @internal appDemoPipePizzaArbiterUnitsDefPrint function
* @endinternal
*
* @brief   Dump structure of register for Pipe Pizza Arbiter Registers
*
* @note   APPLICABLE DEVICES:      Pipe
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPipePizzaArbiterUnitsDefPrint
(
    IN GT_SW_DEV_NUM  dev
)
{
    CPSS_PX_PA_UNIT_ENT i;
    PRV_CPSS_PX_PA_UNIT_STC * unitDefPtr;
    GT_CHAR *unitNamePtr;
    GT_U32 ctrlRegAddr;
    GT_U32 mapRegAddr;
    PRV_CPSS_PX_PP_CONFIG_STC *devPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(dev);

    cpssOsPrintf("\n+----------------+---------------------------------------------+---------------------------------------------------------+");
    cpssOsPrintf("\n|                |                Control                      |           Map                                           |");
    cpssOsPrintf("\n|                +------------+--------------------------------+--------------------+--------------+---------------------+");
    cpssOsPrintf("\n|   Unit         |            |          Fields                |        Register    | Flds Statist |  Single  Fld        |");
    cpssOsPrintf("\n|                | Register   +----------+----------+----------+------------+-------+------+-------+----------+----------+");
    cpssOsPrintf("\n|                | adress     |  Slice   |  Map     |    WC    | Adress     | Total |  per | Total |  Slice   |  Slice   |");
    cpssOsPrintf("\n|                |            | To-Run   | Load-En  |    mode  |            |   N   |  reg |       |   Map    |   En     |");
    cpssOsPrintf("\n+----------------+------------+----------+----------+----------+------------+-------+------+-------+----------+----------+");


    devPtr = PRV_CPSS_PX_PP_MAC(dev);
    for (i = (CPSS_PX_PA_UNIT_ENT)0 ; i < CPSS_PX_PA_UNIT_MAX_E; i++)
    {
        unitDefPtr = (PRV_CPSS_PX_PA_UNIT_STC*)&(devPtr->paData.paUnitDrvList[i]);
        if (unitDefPtr->unit != CPSS_PX_PA_UNIT_UNDEFINED_E)
        {
            unitNamePtr = pipePaUnit22StrFind(unitDefPtr->unit);
            appDemoPipeRegAddrGet(dev,unitDefPtr->ctrlReg.baseAddressPtrOffs,&ctrlRegAddr);
            appDemoPipeRegAddrGet(dev,unitDefPtr->mapReg.baseAddressPtrOffs,&mapRegAddr);
            cpssOsPrintf("\n| %-14s |",&unitNamePtr[7]);
            cpssOsPrintf(" 0x%08x |",ctrlRegAddr);
            cpssOsPrintf(" %s |",appDemoPipePizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldSliceNum2run.offset,      unitDefPtr->ctrlReg.fldSliceNum2run.len));
            cpssOsPrintf(" %s |",appDemoPipePizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldMapLoadEn.offset,         unitDefPtr->ctrlReg.fldMapLoadEn.len));
            cpssOsPrintf(" %s |",appDemoPipePizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldWorkConservModeEn.offset, unitDefPtr->ctrlReg.fldWorkConservModeEn.len));

            cpssOsPrintf(" 0x%08x |",mapRegAddr);
            cpssOsPrintf(" %5d |",unitDefPtr->mapReg.totalReg);
            cpssOsPrintf(" %4d |",unitDefPtr->mapReg.numberFldsPerReg);
            cpssOsPrintf(" %5d |",unitDefPtr->mapReg.totalFlds);
            cpssOsPrintf(" %s |",appDemoPipePizzaArbiterFldPrint(unitDefPtr->mapReg.fldSlice2PortMap.offset, unitDefPtr->mapReg.fldSlice2PortMap.len));
            cpssOsPrintf(" %s |",appDemoPipePizzaArbiterFldPrint(unitDefPtr->mapReg.fldSliceEn.offset,       unitDefPtr->mapReg.fldSliceEn.len));

        }
    }
    cpssOsPrintf("\n+----------------+------------+----------+----------+----------+------------+-------+------+-------+----------+----------+");
    return GT_OK;
}


/**
* @internal gtPipePortPizzaArbiterIfStateDump function
* @endinternal
*
* @brief   Dump Pipe Pizza Arbiter Registers
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portGroupId  - number of port group
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtPipePortPizzaArbiterIfStateDump
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_STATUS   rc;         /* return code */
    static CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;
    GT_U32 unitIdx;
    GT_CHAR * unitNamePtr;
    CPSS_PX_PA_UNIT_ENT unitType;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortPizzaArbiterDevStateGet(devNum, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nPizza Arbiter State device = %2d", devNum);
    for (unitIdx = 0 ; pizzaDeviceState.devState.pipe.unitList[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = pizzaDeviceState.devState.pipe.unitList[unitIdx];
        unitNamePtr = pipePaUnit22StrFind(unitType);
        gtPipePortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.pipe.unitState[unitIdx], unitNamePtr);
    }
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal gtPipePortPizzaArbiterIfUnitStateDump function
* @endinternal
*
* @brief   Dump Pipe Pizza Arbiter Specific Unit Registers
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portGroupId  - number of port group
* @param[in] unit                     -  (RxDMA/TxDMA/TxQ/Tx-FIFO)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtPipePortPizzaArbiterIfUnitStateDump
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  CPSS_PX_PA_UNIT_ENT unit
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      unitIdx;
    GT_BOOL     unitFound;
    static CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;
    GT_CHAR *   unitNamePtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortPizzaArbiterDevStateGet(devNum,/*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }

    unitFound = GT_FALSE;
    cpssOsPrintf("\nPizza Arbiter State device = %2d", devNum);
    for (unitIdx = 0 ; pizzaDeviceState.devState.pipe.unitList[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        if (pizzaDeviceState.devState.pipe.unitList[unitIdx] == unit)
        {
            unitFound = GT_TRUE;
            break;
        }
    }
    if (unitFound == GT_TRUE)
    {
        unitNamePtr = pipePaUnit22StrFind(unit);
        gtPipePortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.pipe.unitState[unitIdx], unitNamePtr);
    }
    else
    {
        cpssOsPrintf("\nunit %d : Undefined Unit",unit);
        return GT_BAD_PARAM;
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


/*---------------------------------------------------------------------------*
 *  print Pizza Arbiter port statistics                                      *
 *    - ethernet ports                                                       *
 *    - CPU SDMA ports                                                       *
 *---------------------------------------------------------------------------*/
static GT_STATUS appDemoPipePortPizzaArbiterResourcesUnitStatisticsCompute
(
    IN GT_SW_DEV_NUM       devNum,
    IN GT_U32              localNumOnUnit,
    IN CPSS_PX_PA_UNIT_ENT unit,
    IN CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr,
    OUT GT_U32            *totalPtr,
    OUT GT_U32            *sliceNptr,
    OUT GT_U32            *minDistPtr,
    OUT GT_U32            *maxDistPtr
)
{
    GT_U32 unitIdx;
    GT_BOOL unitFound;
    GT_U32  i;
    GT_U32  sliceN;
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC  *unitStatePtr;

     #define BAD_VALUE (GT_U32)(~0)
    *minDistPtr = BAD_VALUE;
    *maxDistPtr = BAD_VALUE;

    unitFound = GT_FALSE;
    unitStatePtr = NULL;
    for (unitIdx = 0 ; pizzaDeviceStatePtr->devState.pipe.unitList[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        if (pizzaDeviceStatePtr->devState.pipe.unitList[unitIdx] == unit)
        {
            unitFound = GT_TRUE;
            unitStatePtr = &pizzaDeviceStatePtr->devState.pipe.unitState[unitIdx];
            break;
        }
    }
    if (unitFound != GT_TRUE)
    {
        return GT_BAD_PARAM;
    }
    /* count slice N*/
    sliceN = 0;
    for (i = 0 ; i < unitStatePtr->totalConfiguredSlices; i++)
    {
        if (unitStatePtr->slice_enable[i] == GT_TRUE)
        {
            if (unitStatePtr->slice_occupied_by[i] == localNumOnUnit)
            {
                sliceN++;
            }
        }
    }
    *totalPtr  = unitStatePtr->totalConfiguredSlices;
    *sliceNptr = sliceN;

    if (sliceN > 1)
    {
        /* find first slice */
        GT_U32 firstSliceIdx;
        GT_U32 lastSliceIdx;
        GT_U32 dist;
        GT_U32 minDist;
        GT_U32 maxDist;
        for (firstSliceIdx = 0 ; firstSliceIdx < unitStatePtr->totalConfiguredSlices; firstSliceIdx++)
        {
            if (unitStatePtr->slice_enable[firstSliceIdx] == GT_TRUE)
            {
                if (unitStatePtr->slice_occupied_by[firstSliceIdx] == localNumOnUnit)
                {
                    break;
                }
            }
        }
        lastSliceIdx = firstSliceIdx;
        minDist = unitStatePtr->totalConfiguredSlices;
        maxDist = 0;
        for (i = firstSliceIdx+1 ; i < unitStatePtr->totalConfiguredSlices; i++)
        {
            if (unitStatePtr->slice_enable[i] == GT_TRUE)
            {
                if (unitStatePtr->slice_occupied_by[i] == localNumOnUnit)
                {
                    dist = i - lastSliceIdx;
                    if (dist < minDist)
                    {
                        minDist = dist;
                    }
                    if (dist > maxDist)
                    {
                        maxDist = dist;
                    }
                    lastSliceIdx = i;
                }
            }
        }
        *minDistPtr = minDist;
        *maxDistPtr = maxDist;
    }
    return GT_OK;
}


/**
* @internal appDemoPipePortPizzaArbiterResourcesPrint function
* @endinternal
*
* @brief   Print Pipe Port Pizza Arbiter related statistics
*
* @note   APPLICABLE DEVICES:      Pipe
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPipePortPizzaArbiterResourcesPrint
(
    IN GT_SW_DEV_NUM  dev
)
{
    GT_STATUS rc;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32  maxPortNum;
    static CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC pizzaDeviceState;
    GT_U32 dqIdx,      localTxqNum = BAD_VALUE;
    GT_U32 minDist,maxDist;

    PRV_CPSS_PX_DEV_CHECK_MAC(dev);

    maxPortNum = PRV_CPSS_PX_PORTS_NUM_CNS;

    rc = cpssPxPortPizzaArbiterDevStateGet(dev, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+-------------------------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port PA statistics                                                                                                  |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+-----------------+----------------------+-----------------+-----------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    |TXQ-DQ  |      RXDMA      |           TXQ        |      TXDMA      |     TXFIFO      |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+-----------------+----+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- |   txq  | Slice-N  dist   | WC | Slice-N  dist   | Slice-N  dist   | Slice-N  dist   |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dq loc | tot own min max |    | tot own min max | tot own min max | tot own min max |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    portIdx = 0;
    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        rc = cpssPxPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }
        localTxqNum = BAD_VALUE;

        switch (portMapPtr->portMap.mappingType)
        {
            case CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                speed  = PRV_CPSS_PX_PORT_SPEED_MAC (dev,portMapPtr->portMap.dmaNum);
                ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(dev,portMapPtr->portMap.dmaNum);
                prvCpssPxPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

            break;
            case CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E:
                ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                rc = prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
                if (rc != GT_OK)
                {
                    return rc;
                }
                prvCpssPxPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);
            break;
            default:
                {
                    return GT_BAD_PARAM;
                }
        }


        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            CPSS_PX_PA_UNIT_ENT unitList   [5];
            GT_U32                localOnUnit[5];
            GT_U32 i;
            GT_U32 sliceN;
            GT_U32 totalN;
            GT_BOOL txqWc;

            cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx ,portNum
                ,CPSS_PX_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                ,CPSS_SPEED_2_STR(speed)
                ,CPSS_IF_2_STR(ifMode));
            cpssOsPrintf(
                " %3d %3d %3d |",portMapPtr->portMap.dmaNum
                ,portMapPtr->portMap.txqNum, portMapPtr->portMap.dmaNum);
            cpssOsPrintf(" %2d %3d |",dqIdx,localTxqNum);



            unitList[0] = (CPSS_PX_PA_UNIT_ENT)(CPSS_PX_PA_UNIT_RXDMA_0_E               );
            localOnUnit[0] = portMapPtr->portMap.dmaNum;
            unitList[1] = (CPSS_PX_PA_UNIT_ENT)(CPSS_PX_PA_UNIT_TXQ_0_E     + dqIdx     );
            localOnUnit[1] = localTxqNum;
            unitList[2] = (CPSS_PX_PA_UNIT_ENT)(CPSS_PX_PA_UNIT_TXDMA_0_E               );
            localOnUnit[2] = portMapPtr->portMap.dmaNum;
            unitList[3] = (CPSS_PX_PA_UNIT_ENT)(CPSS_PX_PA_UNIT_TX_FIFO_0_E             );
            localOnUnit[3] = portMapPtr->portMap.dmaNum;
            unitList[4] = CPSS_PX_PA_UNIT_UNDEFINED_E;

            for (i = 0 ; unitList[i] != CPSS_PX_PA_UNIT_UNDEFINED_E; i++)
            {
                rc = appDemoPipePortPizzaArbiterResourcesUnitStatisticsCompute(
                    dev,localOnUnit[i],unitList[i],&pizzaDeviceState,
                    /*OUT*/&totalN,&sliceN,&minDist,&maxDist);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (unitList[i] == CPSS_PX_PA_UNIT_TXQ_0_E + dqIdx)
                {
                    if (portMapPtr->portMap.txqNum != CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS)
                    {
                        rc = prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet(
                            dev,portMapPtr->portMap.txqNum,/*OUT*/&txqWc);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsPrintf(" %2d |",txqWc);
                    }
                    else
                    {
                        cpssOsPrintf(" %2s |","-");
                    }
                }
                switch (sliceN)
                {
                    case 0:
                        cpssOsPrintf(" %3d  --  --  -- |",totalN);
                    break;
                    case 1:
                        cpssOsPrintf(" %3d %3d  --  -- |",totalN,sliceN);
                    break;
                    default:
                    {
                        cpssOsPrintf(" %3d %3d %3d %3d |",totalN,sliceN,minDist,maxDist);
                    }
                }
            }
            portIdx++;
        }
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n");
    return GT_OK;
}


GT_STATUS appDemoPipePortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_U32 dpIndex;
    GT_U32 maxDp;
    GT_U32 totalBWMbps;
    PRV_CPSS_PX_RESOURCES_STATUS_STC  *resourcesStatusPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(dev);

    resourcesStatusPtr = &(PRV_CPSS_PX_PP_MAC(dev)->port.resourcesStatus);
    maxDp = PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.numTxqDq;


    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n|     |  TxQ Descr's  |   Header Cnt  |   PayLoad Cnt |  Core    |");
    cpssOsPrintf("\n| DP# |---------------|---------------|---------------| Overall  |");
    cpssOsPrintf("\n|     |  used   total |  used   total |  used   total |  Speed   |");
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");

    totalBWMbps = 0;

    for (dpIndex = 0 ; dpIndex < maxDp; dpIndex++)
    {
        cpssOsPrintf("\n| %3d |",dpIndex);
        cpssOsPrintf("  %3d     %3d  |",resourcesStatusPtr->usedDescCredits   [dpIndex], resourcesStatusPtr->maxDescCredits   [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",resourcesStatusPtr->usedHeaderCredits [dpIndex], resourcesStatusPtr->maxHeaderCredits [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",resourcesStatusPtr->usedPayloadCredits[dpIndex], resourcesStatusPtr->maxPayloadCredits[dpIndex]);
        cpssOsPrintf("  %6d  |",resourcesStatusPtr->coreOverallSpeedSummary[dpIndex]);
        totalBWMbps += resourcesStatusPtr->coreOverallSpeedSummary[dpIndex];
    }
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n|Total|               |               |               |  %6d  |",totalBWMbps);
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n");
    return GT_OK;
}

GT_STATUS appDemoPipePortListResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC    resource;
    GT_U32  prevDqIdx;
    GT_U32  dqIdx;
    GT_U32  localTxqNum;
    GT_U32  fcaWidth;
    GT_U32  ptpIfWidth;
    GT_U32  coreClockHW;

    PRV_CPSS_PX_DEV_CHECK_MAC(dev);

    rc = cpssPxHwCoreClockGet(dev,/*OUT*/&coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nClock %d MHz\n\n",coreClockHW);

    cpssOsPrintf("\n+----------------------------------------------------------------------------------------------------------------------------+-------------+");
    cpssOsPrintf("\n|                             Port resources                                                                                 |     GOP     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+---------+--------+-----+-----------------------------------------+-----------+------+------+");
    cpssOsPrintf("\n|    |      |          |       |              |Mapping  | TXQ-DQ |RXDMA|            TXDMA SCDMA                  |   TXFIFO  | FCA  | PTP  |");
    cpssOsPrintf("\n|    |      |          |       |              +---------+--------+-----+-----+-----+-----+-----------+-----------+-----+-----|------+------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq |   txq  |  IF | TxQ |Burst|Burst|Rate Limit |  TX-FIFO  | Out | Pay | Bus  | Bus  |");
    cpssOsPrintf("\n|    |      |          |       |              | tx-     | dq loc |Width|Descr| All | Full+-----+-----+-----+-----|Going| Load|width |width |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     |        | bits|     | Most|Thrsh|Int  |Resid| Hdr | Pay | Bus |Start|      |      |");
    cpssOsPrintf("\n|    |      |          |       |              | tx-     |        |     |     | Full|     |Thrsh|Vect |Thrsh|Load |Width|Trasm| bits | bits |");
    cpssOsPrintf("\n|    |      |          |       |              |fifo     |        |     |     |Thrsh|     |     |Thrsh|     |Thrsh|Bytes|Thrsh|      |      |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+---------+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+------+------+");
    portIdx = 0;
    prevDqIdx = 0xFFFFFFFF;
    for (portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssPxPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }
        if ((portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E) &&
            (portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E))
        {
            continue;
        }

        dqIdx       =
            (portMapPtr->portMap.txqNum
             / PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);
        localTxqNum =
            (portMapPtr->portMap.txqNum
             % PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);

        if ((prevDqIdx != dqIdx) && (dqIdx != 0))
        {
            cpssOsPrintf("\n+----+------+----------+-------+--------------+---------+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+------+------+");
        }
        prevDqIdx = dqIdx;

        if (portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            rc = cpssPxPortSpeedGet(dev,portNum,/*OUT*/&speed);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssPxPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else /* CPU  */
        {
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
            rc = prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (speed == CPSS_PORT_SPEED_NA_E)
        {
            continue;
        }

        cpssOsPrintf(
            "\n| %2d | %4d | %-8s | %s | %s |",portIdx ,portNum
            ,CPSS_PX_MAPPING_2_STR(portMapPtr->portMap.mappingType)
            ,CPSS_SPEED_2_STR(speed)
            ,CPSS_IF_2_STR(ifMode));
        cpssOsPrintf(
            " %3d %3d |",
            portMapPtr->portMap.dmaNum,portMapPtr->portMap.txqNum);
        cpssOsPrintf(" %2d %3d |", dqIdx, localTxqNum);

        rc = prvCpssPxPipePortResourseConfigGet(
            dev, portNum,/*OUT*/ &resource);
        if(rc != GT_OK)
        {
            return rc;
        }

        fcaWidth = 0;
        ptpIfWidth = 0;
        if (portMapPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            rc = prvCpssPxPortFcaBusWidthGet(dev,portNum,/*OUT*/&fcaWidth);
            if(rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssPxPortPtpInterfaceWidthGet(dev,portNum,/*OUT*/&ptpIfWidth);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        cpssOsPrintf("%4s |",RXDMA_IfWidth_2_STR(resource.rxdmaScdmaIncomingBusWidth));
        cpssOsPrintf("%4d |",resource.txdmaDescCreditScdma);
        cpssOsPrintf("%4d |",resource.txdmaScdmaBurstAmostFullThrshold);
        cpssOsPrintf("%4d |",resource.txdmaScdmaBurstFullThrshold);
        cpssOsPrintf("%4d |",resource.txdmaScdmaRateLimitIntegerThreshold);
        cpssOsPrintf(
            "%5s|",TXDMA_RateLimitResidueValue_2_STR(
                resource.txdmaScdmaRateLimitResidueVectThreshold));
        cpssOsPrintf("%4d |",resource.txdmaScdmaHeaderTxfifoThrshold);
        cpssOsPrintf("%4d |",resource.txdmaScdmaPayloadTxfifoThrshold);
        cpssOsPrintf(
            "%4s |",TX_FIFO_IfWidth_2_STR(
                resource.txfifoScdmaShiftersOutgoingBusWidth));
        cpssOsPrintf("%4d |",resource.txfifoScdmaPayloadStartTransmThreshold);
        cpssOsPrintf("%5d |",fcaWidth);
        cpssOsPrintf("%5d |",ptpIfWidth);
        portIdx++;
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+---------+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+------+------+");
    cpssOsPrintf("\n");

    cpssOsPrintf("\n");
    cpssOsPrintf("\n");
    cpssOsPrintf("\n+---------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port resource counters                                                |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----------------------------------------+-----+");
    cpssOsPrintf("\n|    |      |          |       |              |TXQ_Q|  TXQ_DQ   |           TXDMA             | XLG |");
    cpssOsPrintf("\n|    |      |          |       |              +-----+-----+-----+-----+-----+-----------+-----+-----+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        |Port |Port |Crdt |Reset|SCDMA|SCDMA|GLBL |GLBL |MAC  |");
    cpssOsPrintf("\n|    |      |          |       |              |Descr|Crdt |cntr |SCDMA|HDR  |PayLd|Src  |Targ |Tx   |");
    cpssOsPrintf("\n|    |      |          |       |              |cntr |cntr |Reset|Descr|TxFif|TxFif|DMA  |DMA  |Fifo |");
    cpssOsPrintf("\n|    |      |          |       |              |     |     |     |Crdt |cntr |cntr |Speed|Speed|Write|");
    cpssOsPrintf("\n|    |      |          |       |              |     |     |     |cntr |     |     |     |     |Thrsh|");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
    portIdx = 0;
    prevDqIdx = 0xFFFFFFFF;
    for (portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssPxPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }
        if ((portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E) &&
            (portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E))
        {
            continue;
        }

        dqIdx       =
            (portMapPtr->portMap.txqNum
             / PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);
        localTxqNum =
            (portMapPtr->portMap.txqNum
             % PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);
        if ((prevDqIdx != dqIdx) && (dqIdx != 0))
        {
            cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
        }
        prevDqIdx = dqIdx;

        if (portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            rc = cpssPxPortSpeedGet(dev,portNum,/*OUT*/&speed);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssPxPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else /* CPU  */
        {
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
            rc = prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (speed == CPSS_PORT_SPEED_NA_E)
        {
            continue;
        }
        dqIdx       =
            (portMapPtr->portMap.txqNum
             / PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);
        localTxqNum =
            (portMapPtr->portMap.txqNum
             % PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp);

        cpssOsPrintf(
            "\n| %2d | %4d | %-8s | %s | %s |",portIdx ,portNum
            ,CPSS_PX_MAPPING_2_STR(portMapPtr->portMap.mappingType)
            ,CPSS_SPEED_2_STR(speed)
            ,CPSS_IF_2_STR(ifMode));

        rc = prvCpssPxPipePortResourseConfigGet(
            dev, portNum,/*OUT*/ &resource);
        if(rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("%4d |",resource.txqQPortDescriptorsCounter);
        cpssOsPrintf("%4d |",resource.txqDqPortCreditCounter);
        cpssOsPrintf("%4d |",resource.txqDqCreditCountersReset);
        cpssOsPrintf("%4d |",resource.txdmaScdmaResetDescCreditsCounter);
        cpssOsPrintf("%4d |",resource.txdmaScdmaHeaderTxfifoCounter);
        cpssOsPrintf("%4d |",resource.txdmaScdmaPayloadTxfifoCounter);
        cpssOsPrintf("%4d |",resource.txdmaGlobalSourceDmaSpeed);
        cpssOsPrintf("%4d |",resource.txdmaGlobalTargetDmaSpeed);
        cpssOsPrintf("%4d |",resource.xlgMacFifoTxWriteThreshold);
        portIdx++;
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
    cpssOsPrintf("\n");
    return GT_OK;
}

GT_STATUS appDemoPxPortsDisconnectConnectSimulation
(
    IN  GT_U8    devNum,
    IN  GT_U16   numOfLoops,
    IN  GT_U16   timeout,
    IN  GT_U8    numOfPorts,
    ...
)
{

    va_list  ap;                 /* arguments list pointer */
    GT_U32   i,j;
    GT_U32 portNum,localReturnCode;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    GT_STATUS rc;

    va_start(ap, numOfPorts);

    cpssOsPrintf("\nnumOfLoops= %d\n",numOfLoops);
    cpssOsPrintf("\nnumOfPorts= %d\n",numOfPorts);

    for(i = 1; i <= numOfPorts;i++)
    {
        portNum = va_arg(ap, GT_U32);

        cpssOsPrintf("\nportNum= %d\n",portNum);

        if (portNum != PRV_CPSS_PX_PORTS_NUM_CNS)
        {
            rc = cpssPxPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                va_end(ap);
                return rc;
            }
            if (portMapPtr->valid == GT_FALSE)
            {
                va_end(ap);
                return GT_BAD_PARAM;
            }
            if (portMapPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                va_end(ap);
                return GT_BAD_PARAM;
            }
        }


        for (j=1; j <=numOfLoops; j++)
        {

            localReturnCode=cpssPxPortSerdesTxEnableSet(devNum,portNum,GT_FALSE);
            if (localReturnCode!=GT_OK)
            {
                cpssOsPrintf("\n Port %d Error Disable %d\n",portNum,localReturnCode);
                va_end(ap);
                return localReturnCode;
            }

            if (timeout)
            {
                osTimerWkAfter(timeout);
            }

            localReturnCode=cpssPxPortSerdesTxEnableSet(devNum,portNum,GT_TRUE);
            if (localReturnCode!=GT_OK)
            {
                cpssOsPrintf("\n Port %d Error Enable %d\n",portNum,localReturnCode);
                va_end(ap);
                return localReturnCode;
            }


            if (timeout)
            {
                osTimerWkAfter(timeout);
            }
        }
    }
    va_end(ap);
    return GT_OK;
}

GT_STATUS appDemoPxPortApLinkUpDebug
(
    GT_U8                           devNum,
    GT_PHYSICAL_PORT_NUM            portNum0,
    GT_PHYSICAL_PORT_NUM            portNum1,
    CPSS_PORT_SPEED_ENT             speed,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    GT_BOOL                         noneceDisableValue,
    GT_U32                          timesLoop,
    GT_U32                          linkCheckDelay,
    GT_U32                          enDelay,
    GT_U32                          disDelay
)
{
    GT_STATUS                   rc;
    CPSS_PX_PORT_AP_PARAMS_STC  apParams;
    GT_BOOL                     isLinkUp_0, isLinkUp_1;
    GT_U32                      times;

    /* Port AP params */
    cpssOsMemSet(&apParams, 0, sizeof(apParams));

    apParams.fcPause = GT_TRUE;
    apParams.fcAsmDir = 0;
    apParams.fecSupported = GT_TRUE;
    apParams.fecRequired =  GT_TRUE;
    apParams.noneceDisable = noneceDisableValue;
    apParams.laneNum = 0;
    apParams.modesAdvertiseArr[0].ifMode = ifMode;
    apParams.modesAdvertiseArr[0].speed = speed;

    for (times = 0; times < timesLoop; times++)
    {
        /* Enable AP */
        rc = cpssPxPortApPortConfigSet(devNum, portNum0, GT_TRUE, &apParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssPxPortApPortConfigSet 0 fail, return %d\n\r", rc);
            return rc;
        }

        cpssOsTimerWkAfter(1000*enDelay);

        rc = cpssPxPortApPortConfigSet(devNum, portNum1, GT_TRUE, &apParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssPxPortApPortConfigSet 1 fail, return %d\n\r", rc);
            return rc;
        }

        cpssOsTimerWkAfter(1000*linkCheckDelay);

        /* Check the status */
        rc = cpssPxPortLinkStatusGet(devNum, portNum0, &isLinkUp_0);
        if (rc != GT_OK) 
        {
            cpssOsPrintf("cpssPxPortLinkStatusGet 0 fail, return %d\n\r", rc);
            return rc;
        }

        rc = cpssPxPortLinkStatusGet(devNum, portNum1, &isLinkUp_1);
        if (rc != GT_OK) 
        {
            cpssOsPrintf("cpssPxPortLinkStatusGet 1 fail, return %d\n\r", rc);
            return rc;
        }

        if ((GT_FALSE == isLinkUp_1) || (GT_FALSE == isLinkUp_0))
        {
            rc = cpssPxPortApDebugInfoGet(devNum, 255, GT_FALSE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortApDebugInfoGet fail, return %d\n\r", rc);
                return rc;
            }
            cpssOsPrintf("Link status: Port0=%d Port4=%d\n\r", isLinkUp_0, isLinkUp_1);
            return GT_FAIL;
        }

        /* Disable AP */
        rc = cpssPxPortApPortConfigSet(devNum, portNum0, GT_FALSE, &apParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssPxPortApPortConfigSet 0_0 fail, return %d\n\r", rc);
            return rc;
        }

        cpssOsTimerWkAfter(disDelay*1000);

        rc = cpssPxPortApPortConfigSet(devNum, portNum1, GT_FALSE, &apParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssPxPortApPortConfigSet 1_1 fail, return %d\n\r", rc);
            return rc;
        }

        cpssOsPrintf("Test %d done\n", times);

        cpssOsTimerWkAfter(disDelay*1000);
    }

    return GT_OK;
}




