#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>


GT_STATUS appDemoAldrinPortMappingDump
(
    IN  GT_U8  dev
)
{
    GT_STATUS   rc;         /* return code */
    GT_PHYSICAL_PORT_NUM portIdx;
    GT_U32 dpIdx;
    GT_U32 maxPortNum;
    GT_U32 maxDp;

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+");
    cpssOsPrintf("\n| Port |   mapping Type  | mac txq rxdma txdma | rx-dma | tx-dma |");
    cpssOsPrintf("\n|      |                 |                     | dp loc | dp loc |");
    cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+");
    maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    for (dpIdx = 0 ; dpIdx < maxDp;  dpIdx++)
    {
        for (portIdx = 0 ; portIdx < maxPortNum; portIdx++)
        {
            static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
            CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
            GT_U32 dpIdxRxDma, localRxDma;
            GT_U32 dpIdxTxDma, localTxDma;

            rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portIdx,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portMapPtr->valid == GT_TRUE)
            {
                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);

                if (dpIdxRxDma == dpIdx)
                {

                    cpssOsPrintf("\n| %4d |",portIdx);
                    cpssOsPrintf(" %-15s |",CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType));
                    cpssOsPrintf(" %3d",portMapPtr->portMap.macNum);
                    cpssOsPrintf(" %3d",portMapPtr->portMap.txqNum);
                    cpssOsPrintf(" %5d",portMapPtr->portMap.rxDmaNum);
                    cpssOsPrintf(" %5d",portMapPtr->portMap.txDmaNum);
                    cpssOsPrintf(" | %2d %3d | %2d %3d |",dpIdxRxDma, localRxDma,dpIdxTxDma, localTxDma);
                      if (portMapPtr->portMap.trafficManagerEn == GT_TRUE)
                    {
                        cpssOsPrintf("TM is enabled , something wrong !!!");
                    }
                }
            }
        }
        cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


GT_STATUS appDemoAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC *groupResorcesStatusPtr;
    GT_U32 dpIndex;
    GT_U32 maxDp;
    GT_U32 totalBWMbps;

    groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;

    maxDp = 1;
    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
    {
        maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    }

    totalBWMbps = 0;
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n|     |  TxQ Descr's  |   Header Cnt  |   PayLoad Cnt |  Core    |");
    cpssOsPrintf("\n| DP# |---------------|---------------|---------------| Overall  |");
    cpssOsPrintf("\n|     |  used   total |  used   total |  used   total |  Speed   |");
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    for (dpIndex = 0 ; dpIndex < maxDp; dpIndex++)
    {
        cpssOsPrintf("\n| %3d |",dpIndex);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedDescCredits   [dpIndex], groupResorcesStatusPtr->maxDescCredits   [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedHeaderCredits [dpIndex], groupResorcesStatusPtr->maxHeaderCredits [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedPayloadCredits[dpIndex], groupResorcesStatusPtr->maxPayloadCredits[dpIndex]);
        cpssOsPrintf("  %6d  |",groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex]);
        totalBWMbps += groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex];
    }
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n|Total|               |               |               |  %6d  |",totalBWMbps);
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n");
    return GT_OK;
}



GT_STATUS appDemoAldrinPortListResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC    resource;
    GT_U32  dpIdx;
    GT_U32  maxPortNum;
    GT_U32  maxDp;

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+---------------------------------------------------------------------------------------------------------------------+-------------+");
    cpssOsPrintf("\n|                             Port resources                                                                          |     GOP     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+---+-----------------+-----+-----------------+-----------+------+------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    |TXQ| DMA Mapping     |RXDMA|   TXDMA SCDMA   |   TXFIFO  | FCA  | PTP  |");
    cpssOsPrintf("\n|    |      |          |       |              |             | WC+-----------------+-----|-----+-----------|-----+-----|------+------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- |   | rx-dma | tx-dma |  IF | TxQ |  TX-FIFO  | Out | Pay | Bus  | Bus  |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma |   | dp loc | dp loc |Width|Descr|-----+-----|Going| Load|width |width |");
    cpssOsPrintf("\n|    |      |          |       |              |             |   |        |        | bits|     | Hdr | Pay | Bus |Start|      |      |");
    cpssOsPrintf("\n|    |      |          |       |              |             |   |        |        |     |     |Thrsh|Load |Width|Trasm| bits | bits |");
    cpssOsPrintf("\n|    |      |          |       |              |             |   |        |        |     |     |     |Thrsh|Bytes|Thrsh|      |      |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+---+--------+--------+-----+-----+-----+-----+-----+-----+------+------+");

    portIdx = 0;
    maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    for (dpIdx = 0; dpIdx < maxDp; dpIdx++)
    {
        for (portNum = 0 ; portNum < 256; portNum++)
        {
            rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portMapPtr->valid == GT_FALSE)
            {
                continue;
            }
            if (portMapPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                rc = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else /* CPU  */
            {
                speed  = CPSS_PORT_SPEED_1000_E;
                ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
            }
            if (speed != CPSS_PORT_SPEED_NA_E )
            {
                GT_U32 dpIdxRxDma, localRxDma;
                GT_U32 dpIdxTxDma, localTxDma;
                GT_U32             fcaWidth;
                GT_U32             ptpIfWidth;

                GT_BOOL txqWCstatus;
                GT_CHAR txqWCstatusStr = '-';

                if (portMapPtr->portMap.txqNum != (GT_U32)(-1))
                {
                    rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet(dev,portMapPtr->portMap.txqNum,/*OUT*/&txqWCstatus);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    if (txqWCstatus == GT_TRUE)
                    {
                        txqWCstatusStr = '1';
                    }
                    else
                    {
                        txqWCstatusStr = '0';
                    }
                }
                fcaWidth = 0;
                ptpIfWidth = 0;
                if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
                {
                    rc = prvCpssDxChPortBcat2FcaBusWidthGet(dev,portNum,/*OUT*/&fcaWidth);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChPortBcat2PtpInterfaceWidthGet(dev,portNum,/*OUT*/&ptpIfWidth);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }


                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);

                if (dpIdxRxDma == dpIdx)
                {
                    cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                            ,portNum
                                                            ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                            ,CPSS_SPEED_2_STR(speed)
                                                            ,CPSS_IF_2_STR(ifMode));
                    cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                    cpssOsPrintf(" %1c |"          ,txqWCstatusStr);
                    cpssOsPrintf(" %2d %3d | %2d %3d |"          ,dpIdxRxDma, localRxDma,dpIdxTxDma, localTxDma);


                    rc = prvCpssDxChCaelumPortResourceConfigGet(dev, portNum,/*OUT*/ &resource);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    cpssOsPrintf("%4s |",RXDMA_IfWidth_2_STR(resource.rxdmaScdmaIncomingBusWidth));
                    cpssOsPrintf("%4d |",resource.txdmaCreditValue);
                    cpssOsPrintf("%4d |",resource.txdmaTxfifoHeaderCounterThresholdScdma);
                    cpssOsPrintf("%4d |",resource.txdmaTxfifoPayloadCounterThresholdScdma);
                    cpssOsPrintf("%4s |",TX_FIFO_IfWidth_2_STR(resource.txfifoScdmaShiftersOutgoingBusWidth));
                    cpssOsPrintf("%4d |",resource.txfifoScdmaPayloadStartTransmThreshold);
                    cpssOsPrintf("%5d |",fcaWidth);
                    cpssOsPrintf("%5d |",ptpIfWidth);

                    if (portMapPtr->portMap.trafficManagerEn == GT_TRUE)
                    {
                        cpssOsPrintf("TM is enabled , somthing wrong !!!");
                    }
                    portIdx++;
                }
            }
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+---+--------+--------+-----+-----+-----+-----+-----+-----+------+------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


/*---------------------------------------------------------------------------*
 *  print Pizza Arbiter port statistics                                      *
 *    - ethernet ports                                                       *
 *    - CPU SDMA ports                                                       *
 *    - remote ports                                                         *
 *---------------------------------------------------------------------------*/



GT_STATUS appDemoAldrinPortPizzaArbiterResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32  dpIdx;
    GT_U32  maxPortNum;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC pizzaDeviceState;
    GT_U32 dpIdxRxDma, localRxDma  = APPDEMO_BAD_VALUE;
    GT_U32 dpIdxTxDma, localTxDma  = APPDEMO_BAD_VALUE;
    GT_U32 dqIdx,      localTxqNum = APPDEMO_BAD_VALUE;
    GT_U32 minDist,maxDist;
    GT_U32 speedMbps;
    GT_U32 maxDP;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
         return rc;
    }

    rc = cpssDxChPortPizzaArbiterDevStateGet(dev,0, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+----------------------------------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port PA statistics                                                                                                           |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+-----------------+-----------------+----------------------+-----------------+-----------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    | DMA  Mapping    |      RXDMA      |           TXQ        |      TXDMA      |     TXFIFO      |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- | rx-dma | tx-dma | Slice-N  dist   | WC | Slice-N  dist   | Slice-N  dist   | Slice-N  dist   |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dp loc | dp loc | tot own min max |    | tot own min max | tot own min max | tot own min max |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    portIdx = 0;
    maxDP = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    for (dpIdx = 0; dpIdx < maxDP; dpIdx++)
    {
        for (portNum = 0 ; portNum < 256; portNum++)
        {
            rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portMapPtr->valid == GT_FALSE)
            {
                continue;
            }
            localRxDma  = APPDEMO_BAD_VALUE;
            localTxDma  = APPDEMO_BAD_VALUE;
            localTxqNum = APPDEMO_BAD_VALUE;

            switch (portMapPtr->portMap.mappingType)
            {
                case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                    rc = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
                    prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

                break;
                case CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:
                    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                    rc = prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
                    prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);
                break;
                case CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E:
                {
                    rc = prvCpssDxChRemotePortSpeedGet(dev,portNum,/*OUT*/&speedMbps);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    speed = CPSS_SPEED_Mbps_2_ENM(speedMbps);
                    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                    prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);
                }
                break;
                default:
                    {
                        return GT_BAD_PARAM;
                    }
            }


            if (speed != CPSS_PORT_SPEED_NA_E)
            {
                if (dpIdxRxDma == dpIdx)
                {
                    CPSS_DXCH_PA_UNIT_ENT unitList   [5];
                    GT_U32                localOnUnit[5];
                    GT_U32 i;
                    GT_U32 sliceN;
                    GT_U32 totalN;
                    GT_BOOL txqWc;

                    cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                            ,portNum
                                                            ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                            ,CPSS_SPEED_2_STR(speed)
                                                            ,CPSS_IF_2_STR(ifMode));
                    cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                    if (portMapPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                    {
                        cpssOsPrintf(" %2d %3d | %2d %3d |"          ,dpIdxRxDma, localRxDma,dpIdxTxDma, localTxDma);
                    }
                    else
                    {
                        cpssOsPrintf(" %2s %3s | %2s %3s |","--", "--","--", "--");
                    }



                    unitList[0] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_RXDMA_0_E   + dpIdxRxDma); localOnUnit[0] = localRxDma;
                    unitList[1] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TXQ_0_E                 ); localOnUnit[1] = localTxqNum;
                    unitList[2] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TXDMA_0_E   + dpIdxTxDma); localOnUnit[2] = localTxDma;
                    unitList[3] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TX_FIFO_0_E + dpIdxTxDma); localOnUnit[3] = localTxDma;
                    unitList[4] = CPSS_DXCH_PA_UNIT_UNDEFINED_E;

                    for (i = 0 ; unitList[i] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
                    {
                        rc = appDemoBobKPortPizzaArbiterResourcesUnitStatisticsGet(dev,localOnUnit[i],unitList[i],&pizzaDeviceState,/*OUT*/&totalN,&sliceN,&minDist,&maxDist);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        if (unitList[i] == CPSS_DXCH_PA_UNIT_TXQ_0_E)
                        {
                            if (portMapPtr->portMap.txqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
                            {
                                rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet(dev,portMapPtr->portMap.txqNum,/*OUT*/&txqWc);
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
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}



