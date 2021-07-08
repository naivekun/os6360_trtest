#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>

#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern GT_STATUS gtBobcat2PortPizzaArbiterIfUnitStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  CPSS_DXCH_PA_UNIT_ENT unit
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#define BOBCAT3_DP_NUM 6

GT_STATUS appDemoBobcat3PortMappingDump
(
    IN  GT_U8  dev
)
{
    GT_STATUS   rc;         /* return code */
    GT_PHYSICAL_PORT_NUM portIdx;
    GT_U32 dpIdx;
    GT_U32 maxPortNum;
    GT_U32 maxDp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+--------+");
    cpssOsPrintf("\n| Port |   mapping Type  | mac txq rxdma txdma | rx-dma |   txQ  | tx-dma |");
    cpssOsPrintf("\n|      |                 |                     | dp loc | dq loc | dp loc |");
    cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+--------+");

    maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    for (dpIdx = 0 ; dpIdx < maxDp; dpIdx++)
    {
        for (portIdx = 0 ; portIdx < maxPortNum; portIdx++)
        {
            static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
            CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
            GT_U32 dpIdxRxDma, localRxDma;
            GT_U32 dpIdxTxDma, localTxDma;
            GT_U32 dqIdx,      localTxqNum;




            rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portIdx,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (portMapPtr->valid == GT_TRUE)
            {
                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
                prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

                if (dpIdxRxDma == dpIdx)
                {

                    cpssOsPrintf("\n| %4d |",portIdx);
                    cpssOsPrintf(" %-15s |",CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType));
                    cpssOsPrintf(" %3d",portMapPtr->portMap.macNum);
                    cpssOsPrintf(" %3d",portMapPtr->portMap.txqNum);
                    cpssOsPrintf(" %5d",portMapPtr->portMap.rxDmaNum);
                    cpssOsPrintf(" %5d",portMapPtr->portMap.txDmaNum);
                    cpssOsPrintf(" | %2d %3d | %2d %3d | %2d %3d |",dpIdxRxDma, localRxDma,dqIdx,localTxqNum,dpIdxTxDma, localTxDma);
                    if (portMapPtr->portMap.trafficManagerEn == GT_TRUE)
                    {
                        cpssOsPrintf("TM is enabled , something wrong !!!");
                    }
                }
            }
        }
        cpssOsPrintf("\n+------+-----------------+---------------------+--------+--------+--------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


GT_STATUS appDemoBobcat3PortGlobalResourcesPrint
(
    IN GT_U8 dev
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC *groupResorcesStatusPtr;
    GT_U32 dpIndex;
    GT_U32 maxDp;
    GT_U32 totalBWMbps;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;

    maxDp = 1;
    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
    {
        maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    }

    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");
    cpssOsPrintf("\n|     |  TxQ Descr's  |   Header Cnt  |   PayLoad Cnt |  Core    |");
    cpssOsPrintf("\n| DP# |---------------|---------------|---------------| Overall  |");
    cpssOsPrintf("\n|     |  used   total |  used   total |  used   total |  Speed   |");
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+");

    totalBWMbps = 0;

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



GT_STATUS appDemoBobcat3PortListResourcesPrint
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

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
         return rc;
    }

    cpssOsPrintf("\n+--------------------------------------------------------------------------------------------------------------------------------------------------+-------------+");
    cpssOsPrintf("\n|                             Port resources                                                                                                       |     GOP     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------------------------+-----+-----------------------------------------+-----------+------+------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    | DMA & TXQ-DQ Mapping     |RXDMA|            TXDMA SCDMA                  |   TXFIFO  | FCA  | PTP  |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+--------+--------+-----+-----+-----+-----+-----------+-----------+-----+-----|------+------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- | rx-dma |   txq  | tx-dma |  IF | TxQ |Burst|Burst|Rate Limit |  TX-FIFO  | Out | Pay | Bus  | Bus  |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dp loc | dq loc | dp loc |Width|Descr| All | Full+-----+-----+-----+-----|Going| Load|width |width |");
    cpssOsPrintf("\n|    |      |          |       |              |             |        |        |        | bits|     | Most|Thrsh|Int  |Resid| Hdr | Pay | Bus |Start|      |      |");
    cpssOsPrintf("\n|    |      |          |       |              |             |        |        |        |     |     | Full|     |Thrsh|Vect |Thrsh|Load |Width|Trasm| bits | bits |");
    cpssOsPrintf("\n|    |      |          |       |              |             |        |        |        |     |     |Thrsh|     |     |Thrsh|     |Thrsh|Bytes|Thrsh|      |      |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+------+------+");
    portIdx = 0;
    for (dpIdx = 0; dpIdx < BOBCAT3_DP_NUM; dpIdx++)
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
            if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ||
                portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
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
                    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                    rc = prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if (speed != CPSS_PORT_SPEED_NA_E)
                {
                    GT_U32 dpIdxRxDma, localRxDma;
                    GT_U32 dpIdxTxDma, localTxDma;
                    GT_U32 dqIdx,      localTxqNum;
                    GT_U32             fcaWidth;
                    GT_U32             ptpIfWidth;

                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
                    prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

                    if (dpIdxRxDma == dpIdx)
                    {
                        cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                                ,portNum
                                                                ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                                ,CPSS_SPEED_2_STR(speed)
                                                                ,CPSS_IF_2_STR(ifMode));
                        cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                        cpssOsPrintf(" %2d %3d | %2d %3d | %2d %3d |"          ,dpIdxRxDma, localRxDma,dqIdx, localTxqNum,dpIdxTxDma, localTxDma);

                        rc = prvCpssDxChCaelumPortResourceConfigGet(dev, portNum,/*OUT*/ &resource);
                        if(rc != GT_OK)
                        {
                            return rc;
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
                        cpssOsPrintf("%4s |",RXDMA_IfWidth_2_STR(resource.rxdmaScdmaIncomingBusWidth));
                        cpssOsPrintf("%4d |",resource.txdmaCreditValue);
                        cpssOsPrintf("%4d |",resource.txdmaScdmaBurstAmostFullThrshold);
                        cpssOsPrintf("%4d |",resource.txdmaScdmaBurstFullThrshold);
                        cpssOsPrintf("%4d |",resource.txdmaScdmaRateLimitIntegerThreshold);
                        cpssOsPrintf("%5s|",TXDMA_RateLimitResidueValue_2_STR(resource.txdmaScdmaRateLimitResidueVectThreshold));
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
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+------+------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


GT_STATUS appDemoBobcat3PortListResourcesRegsInfoPrint
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

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
         return rc;
    }

    cpssOsPrintf("\n+----------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port resources                                                                     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------------------------+-------------------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    | DMA & TXQ-DQ Mapping     | TxQ Reset Credit        |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+--------+--------+------------+-----+------");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- | rx-dma |   txq  | tx-dma |  reg       | Bit | Size |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dp loc | dq loc | dp loc | addr       |     |      |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+------------+-----+------+");
    portIdx = 0;
    for (dpIdx = 0; dpIdx < BOBCAT3_DP_NUM; dpIdx++)
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
            if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ||
                portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
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
                    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                    rc = prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if (speed != CPSS_PORT_SPEED_NA_E)
                {
                    GT_U32 dpIdxRxDma, localRxDma;
                    GT_U32 dpIdxTxDma, localTxDma;
                    GT_U32 dqIdx,      localTxqNum;
                    GT_U32      regAddr;        /*register address*/
                    GT_U32      fieldOffset;    /*register field offset*/
                    GT_U32      fieldLen;       /*register field length*/

                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
                    prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
                    prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

                    if (dpIdxRxDma == dpIdx)
                    {
                        cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                                ,portNum
                                                                ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                                ,CPSS_SPEED_2_STR(speed)
                                                                ,CPSS_IF_2_STR(ifMode));
                        cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                        cpssOsPrintf(" %2d %3d | %2d %3d | %2d %3d |"          ,dpIdxRxDma, localRxDma,dqIdx, localTxqNum,dpIdxTxDma, localTxDma);


                        rc = prvCpssDxChCaelumPortRegAddrGet(dev, portNum, PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
                                                             &regAddr, &fieldOffset, &fieldLen);
                        if(GT_OK != rc)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }


                        cpssOsPrintf(" 0x%08X |",regAddr);
                        cpssOsPrintf("%4d |",fieldOffset);
                        cpssOsPrintf("%5d |",fieldLen);
                        portIdx++;
                    }
                }
            }
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+------------+-----+------+");
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



GT_STATUS appDemoBobcat3PortPizzaArbiterResourcesPrint
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
    GT_U32    speedMbps;

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


    cpssOsPrintf("\n+-------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port PA statistics                                                                                                                    |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------------------------+-----------------+----------------------+-----------------+-----------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    | DMA & TXQ-DQ Mapping     |      RXDMA      |           TXQ        |      TXDMA      |     TXFIFO      |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- | rx-dma |   txq  | tx-dma | Slice-N  dist   | WC | Slice-N  dist   | Slice-N  dist   | Slice-N  dist   |");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dp loc | dq loc | dp loc | tot own min max |    | tot own min max | tot own min max | tot own min max |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    portIdx = 0;
    for (dpIdx = 0; dpIdx < BOBCAT3_DP_NUM; dpIdx++)
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
                        cpssOsPrintf(" %2d %3d | %2d %3d | %2d %3d |"          ,dpIdxRxDma, localRxDma,dqIdx, localTxqNum,dpIdxTxDma, localTxDma);
                    }
                    else
                    {
                        cpssOsPrintf(" %2s %3s | %2d %3d | %2s %3s |"          ,"--", "--", dqIdx,localTxqNum,"--", "--");
                    }



                    unitList[0] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_RXDMA_0_E   + dpIdxRxDma); localOnUnit[0] = localRxDma;
                    unitList[1] = (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TXQ_0_E     + dqIdx     ); localOnUnit[1] = localTxqNum;
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
                        if (unitList[i] == CPSS_DXCH_PA_UNIT_TXQ_0_E + dqIdx)
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
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-----------------+----+-----------------+-----------------+-----------------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}


/*---------------------------------------------------------------------------*
 *  print Pizza Arbiter units for specific DP (i.e. RxDMA,TxQ,TxDma, TxFifo  *
 *  gtBobcat2PortPizzaArbiterIfStateDump in case of BC3 prints too much      *
 *  (4 units per DP x 6 DP = 24 units                                        *
 *---------------------------------------------------------------------------*/
GT_STATUS appDemoBobcat3PizzaArbiterDPDump
(
    IN GT_U8 dev,
    IN GT_U32 dataPathIdx
)
{
    GT_STATUS rc;
    CPSS_DXCH_PA_UNIT_ENT unitList[] = { CPSS_DXCH_PA_UNIT_RXDMA_0_E,CPSS_DXCH_PA_UNIT_TXQ_0_E,CPSS_DXCH_PA_UNIT_TXDMA_0_E,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E, CPSS_DXCH_PA_UNIT_UNDEFINED_E};
    GT_U32 i;

    if (dataPathIdx >= PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp)
    {
        return GT_BAD_PARAM;
    }

    for (i = 0; unitList[i] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,(CPSS_DXCH_PA_UNIT_ENT)(unitList[i]+dataPathIdx));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/*-------------------------------------------------------------------------------------------------------+
 |                             Port scheduler profile                                                    |
 *-------------------------------------------------------------------------------------------------------+*/
GT_STATUS appDemoBobcat3PortListSchedulerProfilePrint
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
    GT_32   bcValue;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_U32 dpIdxRxDma, localRxDma;
    GT_U32 dpIdxTxDma, localTxDma;
    GT_U32 dqIdx,      localTxqNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
         return rc;
    }


    cpssOsPrintf("\n+-------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port shecduler profile                                                    |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------------------------+----------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    | DMA & TXQ-DQ Mapping     |     Scheduler  |");
    cpssOsPrintf("\n|    |      |          |       |              |             +--------+--------+--------+-------+--------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- | rx-dma |   txq  | tx-dma |Profile|BC const|");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma | dp loc | dq loc | dp loc |  ID   | value  |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-------+--------+");

    portIdx = 0;
    for (dpIdx = 0; dpIdx < BOBCAT3_DP_NUM; dpIdx++)
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

            prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
            prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
            prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

            if (dpIdxRxDma == dpIdx)
            {

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
                    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                    rc = prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet(dev,/*OUT*/&speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                rc = cpssDxChPortTxBindPortToSchedulerProfileGet(dev,portNum,/*OUUT*/&profileSet);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = cpssDxChPortTxByteCountChangeValueGet(dev,portNum,/*OUT*/&bcValue);
                if (rc != GT_OK)
                {
                    return rc;
                }


                cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                        ,portNum
                                                        ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                        ,CPSS_SPEED_2_STR(speed)
                                                        ,CPSS_IF_2_STR(ifMode));
                cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                cpssOsPrintf(" %2d %3d | %2d %3d | %2d %3d |",dpIdxRxDma, localRxDma,dqIdx, localTxqNum,dpIdxTxDma, localTxDma);
                cpssOsPrintf(" %5s |",CPSS_SCHEDULER_PROFILE_2_STR(profileSet));
                cpssOsPrintf(" %5d |",bcValue);
                portIdx++;
            }
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+--------+--------+--------+-------+--------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}

/*-------------------------------------------*/

GT_STATUS appDemoBobcat3LedConfigPrint
(
    GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 ledIf;
    GT_U32 regAddr;


    GT_U32 LEDControl[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS];
    GT_U32 blinkGlobalControl[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS];
    GT_U32 classGlobalControl[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS];
    GT_U32 classesAndGroupConfig[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);


    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->GOP.LED[ledIf].LEDControl;
        rc = prvCpssDrvHwPpGetRegField(dev, regAddr, 0, 24, /*OUT*/&LEDControl[ledIf]);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->GOP.LED[ledIf].blinkGlobalControl;
        rc = prvCpssDrvHwPpGetRegField(dev, regAddr, 0, 26, /*OUT*/&blinkGlobalControl[ledIf]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->GOP.LED[ledIf].classGlobalControl;
        rc = prvCpssDrvHwPpGetRegField(dev, regAddr, 0, 26, /*OUT*/&classGlobalControl[ledIf]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->GOP.LED[ledIf].classesAndGroupConfig;
        rc = prvCpssDrvHwPpGetRegField(dev, regAddr, 0, 26, /*OUT*/&classesAndGroupConfig[ledIf]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsPrintf("\n+------------------------------------------------------------------+");
    cpssOsPrintf("\n| Global LED IF config reg DUMP                                    |");
    cpssOsPrintf("\n+------------------------------------------------------------------+");

    cpssOsPrintf("\n+-%-30s-+", "------------------------------");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf("-%8s-+","--------");
    }

    cpssOsPrintf("\n| %-30s |", "");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf(" %8d |",ledIf);
    }
    cpssOsPrintf("\n+-%-30s-+", "------------------------------");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf("-%8s-+","--------");
    }

    cpssOsPrintf("\n| %-30s |", "LEDControl");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf(" %08x |",LEDControl[ledIf]);
    }
    cpssOsPrintf("\n| %-30s |", "blinkGlobalControl");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf(" %08x |",blinkGlobalControl[ledIf]);
    }
    cpssOsPrintf("\n| %-30s |", "classGlobalControl");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf(" %08x |",classGlobalControl[ledIf]);
    }
    cpssOsPrintf("\n| %-30s |", "classesAndGroupConfig");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf(" %08x |",classesAndGroupConfig[ledIf]);
    }


    cpssOsPrintf("\n+-%-30s-+", "------------------------------");
    for (ledIf = 0 ; ledIf < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledIf++)
    {
        cpssOsPrintf("-%8s-+","--------");
    }
    return GT_OK;
}



GT_STATUS appDemoBobcat3PortLedConfigHWGet
(
    IN   GT_U8 devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32 *portIndicationSelectPtr,
    OUT  GT_U32 *portTypeConfigPtr,
    OUT  GT_U32 *portIndicationPolarityPtr
)
{
    GT_U32      ledInterfaceNum;
    GT_U32      ledPort;
    GT_U32      portMacNum;      /* port MAC number */
    GT_U32      regAddr;         /* register address */
    GT_STATUS   rc;              /* return status*/
    GT_U32      bitOffset;       /* register field offset */

    rc = cpssDxChLedStreamPortPositionGet(devNum, portNum, &ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDevLedInterfacePortCheck(devNum, ledInterfaceNum,ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*-----------------------------*
     * portTypeConfig
     *-----------------------------*/
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portTypeConfig[ledPort/16];

    bitOffset = (ledPort%16) * 2;

    /* Ports type configuration */
    rc =  prvCpssDrvHwPpGetRegField(devNum, regAddr, bitOffset, 2, /*OUT*/portTypeConfigPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationSelect[ledPort];
    bitOffset = 0;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, bitOffset, 30, /*OUT*/portIndicationSelectPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationPolarity[ledPort/4];
    bitOffset = (ledPort % 4) * 6;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, bitOffset, 6, /*OUT*/portIndicationPolarityPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS appDemoBobcat3PortLedConfigPrint
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
    GT_U32 dpIdxRxDma, localRxDma;
    GT_U32 dpIdxTxDma, localTxDma;
    GT_U32 dqIdx,      localTxqNum;
    GT_BOOL            isLinkUp;
    GT_CHAR *          linkStatusStr[] = { " -" ,"Up" };
    GT_U32             ledInterfaceNum;
    GT_U32             ledPort;
    GT_U32             geMacLedPos;
    GT_U32             xlgMacLedPos;
    GT_U32             cgMacLedPos;
    GT_U32             portIndicationSelect;
    GT_U32             portTypeConfig;
    GT_U32             portIndicationPolarity;
    GT_CHAR *          portTypeConfigStr[] = { "CIG", "XLG", "CG", "Res"};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
         return rc;
    }

    rc = appDemoBobcat3LedConfigPrint(dev);
    if (rc != GT_OK)
    {
         return rc;
    }

    cpssOsPrintf("\n+---------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port LED configuration                                                      |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+----+--------+-----------+-------------------+");
    cpssOsPrintf("\n|    |      |          |       |              |  Mapping    |Link| LED-IF |MAC LED pos|    Port Led Conf  |");
    cpssOsPrintf("\n|    |      |          |       |              |             +----+--------+-----------+-------------------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rx- txq tx- |    | IF pos | GE XLG CG | Indicat Type Polar|");
    cpssOsPrintf("\n|    |      |          |       |              | dma     dma |    |        |           | Sel               |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+----+--------+-----------+-------------------+");

    portIdx = 0;
    for (dpIdx = 0; dpIdx < BOBCAT3_DP_NUM; dpIdx++)
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

            prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.rxDmaNum,/*OUT*/&dpIdxRxDma,  &localRxDma);
            prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,portMapPtr->portMap.txDmaNum,/*OUT*/&dpIdxTxDma,  &localTxDma);
            prvCpssDxChPpResourcesTxqGlobal2LocalConvert(dev,portMapPtr->portMap.txqNum,  /*OUT*/&dqIdx,       &localTxqNum);

            if (dpIdxRxDma == dpIdx)
            {
                if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
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
                    rc = cpssDxChPortLinkStatusGet(dev,portNum,&isLinkUp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    /* if (speed != CPSS_PORT_SPEED_NA_E) */
                    {
                        rc = prvCpssDxChPortLedInterfaceGet(dev, portNum, &ledInterfaceNum);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }

                        rc = cpssDxChLedStreamPortPositionGet(dev,portNum,&ledPort);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }

                        rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_GE_E, /*OUT*/&geMacLedPos);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }

                        rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_XLG_E,/*OUT*/&xlgMacLedPos);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        if ((0 == (portMapPtr->portMap.macNum %4 )) && (portMapPtr->portMap.macNum < 72))
                        {
                            rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet(dev,portNum,PRV_CPSS_PORT_CG_E, /*OUT*/&cgMacLedPos);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }

                        if (ledPort != PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS)
                        {

                            rc = appDemoBobcat3PortLedConfigHWGet(dev,portNum,/*PUT*/&portIndicationSelect,&portTypeConfig,&portIndicationPolarity);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }

                        cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                                ,portNum
                                                                ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                                ,CPSS_SPEED_2_STR(speed)
                                                                ,CPSS_IF_2_STR(ifMode));
                        cpssOsPrintf(" %3d %3d %3d |"  ,portMapPtr->portMap.rxDmaNum,portMapPtr->portMap.txqNum, portMapPtr->portMap.txDmaNum);
                        cpssOsPrintf(" %2s |",linkStatusStr[isLinkUp]);
                        if (ledPort != PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS)
                        {
                            cpssOsPrintf(" %2d %3d |",ledInterfaceNum,ledPort);
                        }
                        else
                        {
                            cpssOsPrintf(" %2d  3f |",ledInterfaceNum);
                        }
                        if ((0 == (portMapPtr->portMap.macNum %4 )) && (portMapPtr->portMap.macNum < 72))
                        {
                            cpssOsPrintf(" %2x %3x %2x |",geMacLedPos,xlgMacLedPos,cgMacLedPos);
                        }
                        else
                        {
                            cpssOsPrintf(" %2x %3x -- |",geMacLedPos,xlgMacLedPos);
                        }
                        if (ledPort != PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS)
                        {
                            cpssOsPrintf(" %8x  %3s  %2x |",portIndicationSelect,portTypeConfigStr[portTypeConfig],portIndicationPolarity);
                        }
                        else
                        {
                            cpssOsPrintf("       --   --  -- |");
                        }
                        portIdx++;
                    }
                }
            }
        }
        cpssOsPrintf("\n+----+------+----------+-------+--------------+-------------+----+--------+-----------+-------------------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}

/* -----------------------------------------------------------------------------*
 * usage example :                                                              *
 * we would like to assign dq1 slice 0 to txq port 15                           *
 * we would like to assign dq1 slice 1 to txq port 17                           *
 *    appDemoBobcat3TxqDqSet(dev=0, txqDqNum=1, txqSliceN=2,15,17 ...)          *
 *------------------------------------------------------------------------------*/

GT_STATUS appDemoBobcat3TxqDqSet(GT_U8 dev, GT_U32 txqDqNum, GT_U32 txqSliceN, ...)
{
    GT_STATUS rc;
    GT_U32 slice2PortMap[10];
    va_list     ap;                 /* arguments list pointer */
    GT_U32      i;

    static CPSS_DXCH_PA_UNIT_ENT dqN2enm[] =
    {
         CPSS_DXCH_PA_UNIT_TXQ_0_E              /*   6  BC2 BobK-Caelum  BobK-Cetus  Aldrin  BC3 */
        ,CPSS_DXCH_PA_UNIT_TXQ_1_E              /*   7                                       BC3 */
        ,CPSS_DXCH_PA_UNIT_TXQ_2_E              /*   8                                       BC3 */
        ,CPSS_DXCH_PA_UNIT_TXQ_3_E              /*   9                                       BC3 */
        ,CPSS_DXCH_PA_UNIT_TXQ_4_E              /*  10                                       BC3 */
        ,CPSS_DXCH_PA_UNIT_TXQ_5_E              /*  11                                       BC3 */
    };

    CPSS_DXCH_PA_UNIT_ENT dqUnit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);
    if (txqDqNum == (GT_U32)(-1))
    {
       cpssOsPrintf("usage example :                                                              ");
       cpssOsPrintf(" we would like to assign dq1 slice 0 to txq port 15                          ");
       cpssOsPrintf("                         dq1 slice 1 to txq port 17                          ");
       cpssOsPrintf("   appDemoBobcat3TxqDqSet(/*dev=*/0, /*txqDqNum=*/1, /*txqSliceN=*/2,15,17   ");
       return GT_OK;
    }
    if (txqDqNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    dqUnit = dqN2enm[txqDqNum];

    va_start(ap, txqSliceN);
    for(i = 0; i < sizeof(slice2PortMap)/sizeof(slice2PortMap[0]); i++)
    {
        slice2PortMap[i] = 0;
    }
    for(i = 0; i < txqSliceN; i++)
    {
        slice2PortMap[i] = va_arg(ap, GT_U32);
    }
    va_end(ap);

    rc = bobcat2PizzaArbiterUnitDrvSet
    (
        IN dev,
        IN dqUnit,
        IN txqSliceN,
        IN &slice2PortMap[0]
    );

    return rc;
}

/***********************************************************************************************************/
#define MAX_PA_CHANNELS_CNS 128

GT_STATUS appDemoBobcat3PaChannelSpeedsPrint
(
    IN  GT_U32                  devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId
)
{
    GT_STATUS rc;
    GT_U32    channelArrActualSize;
    GT_U32    channelNumArr[MAX_PA_CHANNELS_CNS];
    GT_U32    channelSpeedInMBitArr[MAX_PA_CHANNELS_CNS];
    GT_U32    i;

    rc = prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedGet(
        devNum, unitId, MAX_PA_CHANNELS_CNS,
        &channelArrActualSize, channelNumArr, channelSpeedInMBitArr);
    if (rc != GT_OK)
    {
        cpssOsPrintf(
            "prvCpssDxChPortDynamicPizzaArbiterUnitCannelsSpeedGet failed, rc = 0x%X", rc);
        return rc;
    }
    for (i = 0; (i < channelArrActualSize); i++)
    {
        cpssOsPrintf(
            "channel %d \t SpeedInMBit %d\n",
            channelNumArr[i], channelSpeedInMBitArr[i]);
    }
    return GT_OK;
}

GT_STATUS appDemoBobcat3PaChannelSpeedsSet
(
    IN  GT_U32                  devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT   unitId,
    ... /* pairs <channel_number, channel_spesd_in_mbps>; -1 is end mark */
)
{
    GT_U32    channelArrSize;
    GT_U32    channelNumArr[MAX_PA_CHANNELS_CNS];
    GT_U32    channelSpeedInMBitArr[MAX_PA_CHANNELS_CNS];
    va_list   ap;                 /* arguments list pointer */
    GT_U32    channelNum;
    GT_U32    channelSpeedInMbps;

    channelArrSize = 0;
    va_start(ap, unitId);
    for (channelNum = 0; (channelNum < MAX_PA_CHANNELS_CNS); channelNum++)
    {
        channelNum         = va_arg(ap, GT_U32);
        channelSpeedInMbps = va_arg(ap, GT_U32);
        if (channelNum == (GT_U32)-1) break;
        channelNumArr[channelArrSize]         = channelNum;
        channelSpeedInMBitArr[channelArrSize] = channelSpeedInMbps;
        channelArrSize ++;
    }
    va_end(ap);

    if (channelArrSize >= MAX_PA_CHANNELS_CNS)
    {
        cpssOsPrintf("amount of channels %d is too big\n", channelArrSize);
        return GT_FAIL;
    }

    return prvCpssDxChPortDynamicPizzaArbiterUnitChannelsSpeedUpdate(
        devNum, unitId, channelArrSize,
        channelNumArr, channelSpeedInMBitArr);
}


