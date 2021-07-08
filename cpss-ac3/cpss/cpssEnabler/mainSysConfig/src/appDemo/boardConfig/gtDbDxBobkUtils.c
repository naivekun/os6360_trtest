#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/sysHwConfig/gtAppDemoSfPhyConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>

#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>


#define MAX_PHY_SMI_ADDR 8

  /*Port number to PHY SMI ADDRESS mapping:
    Port 56 - SMI address  3
    Port 57 - SMI address  0
    Port 58 - SMI address  4
    Port 59 - SMI address  2
    Port 64 - SMI address  6
    Port 65 - SMI address  5
    Port 66 - SMI address  1
    Port 67 - SMI address  7*/

typedef struct port2PhySmiAddr
{
    GT_U32      portNum;
    GT_U32      phySmiAddr;
}PORT_2_PHY_SMI_ADDR_MAPPING;

typedef struct pDevInfo
{
    GT_U32      devNum;
    GT_U32      portGroup;
    GT_U32  xsmiInterface;
}PDEVICE_INFO;

PORT_2_PHY_SMI_ADDR_MAPPING port2PhySmiAddrTable[MAX_PHY_SMI_ADDR] = {
    {56,3},
    {57,0},
    {58,1},
    {59,2},
    {64,6},
    {65,4},
    {66,5},
    {67,7}
};

typedef struct
{
    GT_U32  devNum;
    GT_U32  portGroup;
    GT_U32  xsmiInterface;
}APPDEMO_PHY_INFO;


GT_U16 portToPhyArrayAldrinRd[][2] =
{
    {8,  0x8},
    {9,  0x9},
    {10, 0xa},
    {11, 0xb},
    {12, 0x6},
    {13, 0x7},
    {14, 0x5},
    {15, 0x4},
    {16, 0x3},
    {17, 0x2},
    {18, 0x1},
    {19, 0x0},
    {20, 0xb},
    {21, 0xa},
    {22, 0x9},
    {23, 0x8},
    {24, 0x7},
    {25, 0x6},
    {26, 0x5},
    {27, 0x4},
    {28, 0x0},
    {29, 0x1},
    {30, 0x2},
    {31, 0x3},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc3xRd[][2] =
{
    {8,  0x7},
    {9,  0x6},
    {10, 0x5},
    {11, 0x4},
    {12, 0x0},
    {13, 0x1},
    {14, 0x2},
    {15, 0x3},
    {16, 0x4},
    {17, 0x5},
    {18, 0x6},
    {19, 0x7},
    {20, 0x0},
    {21, 0x1},
    {22, 0x2},
    {23, 0x3},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAldrin2Rd[][2] =
{
    {24, 0xb},
    {25, 0xa},
    {26, 0x9},
    {27, 0x8},
    {28, 0x7},
    {29, 0x6},
    {30, 0x5},
    {31, 0x4},
    {32, 0x3},
    {33, 0x2},
    {34, 0x1},
    {35, 0x0},
    {36, 0xb},
    {37, 0xa},
    {38, 0x9},
    {39, 0x8},
    {40, 0x7},
    {41, 0x6},
    {42, 0x5},
    {43, 0x4},
    {44, 0x3},
    {45, 0x2},
    {46, 0x1},
    {47, 0x0},
    {48, 0xb},
    {49, 0xa},
    {50, 0x9},
    {51, 0x8},
    {52, 0x7},
    {53, 0x6},
    {54, 0x5},
    {55, 0x4},
    {56, 0x3},
    {57, 0x2},
    {58, 0x1},
    {64, 0x0},
    {65, 0xb},
    {66, 0xa},
    {67, 0x9},
    {68, 0x8},
    {69, 0x7},
    {70, 0x6},
    {71, 0x5},
    {72, 0x4},
    {73, 0x3},
    {74, 0x2},
    {75, 0x1},
    {76, 0x0},
    {0xff, 0xff}
};

extern GT_STATUS cpssXsmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
);

extern GT_STATUS mtdHwGetPhyRegField
(
    IN  CTX_PTR_TYPE contextPtr,
    IN  GT_U16      port,
    IN  GT_U16      dev,
    IN  GT_U16      regAddr,
    IN  GT_U8       fieldOffset,
    IN  GT_U8       fieldLength,
    OUT GT_U16      *data
);

extern GT_STATUS mtdHwXmdioWrite
(
    IN CTX_PTR_TYPE contextPtr,
    IN GT_U16 port,
    IN GT_U16 dev,
    IN GT_U16 reg,
    IN GT_U16 value
);


extern GT_STATUS mtdGetSpeedsEnabled
(
    IN CTX_PTR_TYPE contextPtr,
    IN GT_U16 port,
    OUT GT_U16 *speed_bits
);

extern GT_STATUS mtdEnableSpeeds
(
    IN CTX_PTR_TYPE contextPtr,
    IN GT_U16 port,
    IN GT_U16 speed_bits,
    IN GT_BOOL anRestart
);

GT_STATUS appDemoBobk2PortListMacPcsStatePrint
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
    GT_BOOL isPortEnabled;
    GT_BOOL isForceLinkDownUp;
    GT_BOOL isLinkDown;
    GT_BOOL isMacReset;
    GT_U32 maxPortNum;

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+---------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port status                                               |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----+-----------+-----+-----------+");
    cpssOsPrintf("\n|    |      |          |       |              |     | Port|    Link   | Mac |   Pcs     |");
    cpssOsPrintf("\n|    |      |          |       |              |     |-----|-----+-----|-----|-----+-----+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | mac | En  |  Up |Force| Rst |  Rx |  Tx |");
    cpssOsPrintf("\n|    |      |          |       |              |     |     |     |Down |     | Rst | Rst |");
    cpssOsPrintf("\n|    |      |          |       |              |     |     |     |     |     |     |     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+-----+-----+-----+-----+-----+-----+");

    portIdx = 0;
    for (portNum = 0 ; portNum < maxPortNum; portNum++)
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
        else /* CPU */
        {
            speed  = CPSS_PORT_SPEED_1000_E;
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            rc = cpssDxChPortEnableGet(dev,portNum,&isPortEnabled);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortForceLinkDownEnableGet(dev,portNum,&isForceLinkDownUp);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortLinkStatusGet(dev,portNum,&isLinkDown);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChPortMacResetStateGet(dev,portNum,&isMacReset);
            if (rc != GT_OK)
            {
                return rc;
            }
            /*
            rc = cpssDxChPortPcsResetGet(dev,portNum,CPSS_PORT_PCS_RESET_MODE_RX_E,&isPcsRxReset);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortPcsResetGet(dev,portNum,CPSS_PORT_PCS_RESET_MODE_TX_E,&isPcsTxReset);
            if (rc != GT_OK)
            {
                return rc;
            }
            */

            cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                    ,portNum
                                                    ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                    ,CPSS_SPEED_2_STR(speed)
                                                    ,CPSS_IF_2_STR(ifMode));
            cpssOsPrintf(" %5d |",portMapPtr->portMap.macNum);
            cpssOsPrintf(" %3d |",isPortEnabled);
            cpssOsPrintf(" %3d |",isLinkDown);
            cpssOsPrintf(" %3d |",isForceLinkDownUp );
            cpssOsPrintf(" %3d |",isMacReset);
            /*
            cpssOsPrintf(" %3d |",isPcsRxReset);
            cpssOsPrintf(" %3d |",isPcsTxReset);
            */

            portIdx++;
        }
    }
    return GT_OK;
}


GT_STATUS appDemoBobk2PortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC *groupResorcesStatusPtr;
    GT_U32 dpIndex;
    GT_U32 maxDp;

    groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;

    maxDp = 1;
    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
    {
        maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    }

    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+----------+");
    cpssOsPrintf("\n|     |  TxQ Descr's  |   Header Cnt  |   PayLoad Cnt |    TM    |  Core    |");
    cpssOsPrintf("\n| DP# |---------------|---------------|---------------|  cum BW  | Overall  |");
    cpssOsPrintf("\n|     |  used   total |  used   total |  used   total |  (Mbps)  |  Speed   |");
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+----------+");
    for (dpIndex = 0 ; dpIndex < maxDp; dpIndex++)
    {
        cpssOsPrintf("\n| %3d |",dpIndex);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedDescCredits   [dpIndex], groupResorcesStatusPtr->maxDescCredits   [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedHeaderCredits [dpIndex], groupResorcesStatusPtr->maxHeaderCredits [dpIndex]);
        cpssOsPrintf("  %3d     %3d  |",groupResorcesStatusPtr->usedPayloadCredits[dpIndex], groupResorcesStatusPtr->maxPayloadCredits[dpIndex]);
        cpssOsPrintf("  %6d  |",groupResorcesStatusPtr->trafficManagerCumBWMbps);
        cpssOsPrintf("  %6d  |",groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex]);
    }
    cpssOsPrintf("\n+-----+---------------+---------------+---------------+----------+----------+");
    cpssOsPrintf("\n");
    return GT_OK;
}

GT_STATUS appDemoBobk2PortListResourcesPrint
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
    GT_U32 maxPortNum;

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port resources                                                                       |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----------------+-----------+-----------+");
    cpssOsPrintf("\n|    |      |          |       |              |                    |RXDMA|   TXDMA SCDMA   |   TXFIFO  | Eth_TXFIFO|");
    cpssOsPrintf("\n|    |      |          |       |              |                    |-----|-----+-----------|-----+-----|-----+-----|");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF | TxQ |  TX-FIFO  | Out | Pay | Out | Pay |");
    cpssOsPrintf("\n|    |      |          |       |              |                    |Width|Descr|-----+-----|Going| Load|Going| Load|");
    cpssOsPrintf("\n|    |      |          |       |              |                    |bits |     | Hdr | Pay | Bus |Thrsh| Bus |Thrsh|");
    cpssOsPrintf("\n|    |      |          |       |              |                    |     |     |Thrsh|Load |Width|     |Width|     |");
    cpssOsPrintf("\n|    |      |          |       |              |                    |     |     |     |     |Bytes|     |Bytes|     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+");

    portIdx = 0;
    for (portNum = 0 ; portNum < maxPortNum; portNum++)
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
        if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
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
        else /* CPU */
        {
            speed  = CPSS_PORT_SPEED_1000_E;
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                    ,portNum
                                                    ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                    ,CPSS_SPEED_2_STR(speed)
                                                    ,CPSS_IF_2_STR(ifMode));
            cpssOsPrintf(" %5d %3d %5d %2d |"  ,portMapPtr->portMap.rxDmaNum
                                               ,portMapPtr->portMap.txqNum
                                               ,portMapPtr->portMap.txDmaNum
                                               ,portMapPtr->portMap.tmPortIdx);

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
            if (portMapPtr->portMap.trafficManagerEn == GT_FALSE)
            {
                cpssOsPrintf("%4s |%4s |","--","--");
            }
            else
            {
                cpssOsPrintf("%4s |",TX_FIFO_IfWidth_2_STR(resource.ethTxfifoOutgoingBusWidth));
                cpssOsPrintf("%4d |",resource.ethTxfifoScdmaPayloadThreshold);
            }

            portIdx++;
        }
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+");
    cpssOsPrintf("\n");
    return GT_OK;
}



#ifndef ASIC_SIMULATION

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/******************************************************************************************
 appAc3xRdEnableSpeed

  Description:
    Set referenced speed on PHY 88E2180.
    The function sets the speeds to be advertised to the
    link partner and enables auto-negotiation.

 Inputs:
    devNum        - device number
    port          - port number
    speed_bits    - speed
                               0x0002 10M FD
                               0x0008 100M FD
                               0x0020 1G FD
                               0x0800  2.5GBASE-T FD 88X33X0 family only
                               0x1000  5GBASE-T full-duplex, 88X33X0 family only

 Returns:
    GT_OK or GT_FAIL
******************************************************************************/
GT_STATUS appAc3xRdEnableSpeed
(
    IN GT_U32 devNum,
    IN GT_U16 portNum,
    IN GT_U16 speed_bits
)
{
    GT_STATUS rc;
    APPDEMO_PHY_INFO    context;
    GT_U16 smiAddr = 0;
    CTX_PTR_TYPE contextPtr;
    GT_U32 i;

    GT_U32  boardIdx;
    GT_U32  boardRevId;
    GT_U32  reloadEeprom;

    for (i = 0; portToPhyArrayAc3xRd[i][0] != 0xff; i++)
    {
        if (portNum == portToPhyArrayAc3xRd[i][0])
        {
            smiAddr = portToPhyArrayAc3xRd[i][1];
            break;
        }
    }

    if (0xff == portToPhyArrayAc3xRd[i][0])
    {
       cpssOsPrintf("wrong port number %d\n", portNum);
       return GT_FAIL;
    }

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
    if( (32 == boardIdx) && (5 == boardRevId) )
    {
        context.xsmiInterface = (portNum < 16) ? CPSS_PHY_XSMI_INTERFACE_1_E :
                                                 CPSS_PHY_XSMI_INTERFACE_0_E;
    }
    else
    {
        context.xsmiInterface = (portNum < 16) ? CPSS_PHY_XSMI_INTERFACE_0_E :
                                                 CPSS_PHY_XSMI_INTERFACE_1_E;
    }
    context.devNum = devNum;
    context.portGroup = 0;
    contextPtr = &context;

    cpssOsPrintf("appAldrinRdEnableSpeed for port %d speed 0x%x\n",portNum,speed_bits);

    rc = mtdEnableSpeeds(contextPtr, smiAddr, speed_bits, GT_TRUE);
    if (rc != GT_OK)
    {
        cpssOsPrintf("mtdEnableSpeeds(%d,0x%x):rc=%d\n",
                        context.xsmiInterface, smiAddr, rc);
        return rc;
    }

    return GT_OK;

}

/******************************************************************************************
 appAldrin2RdEnableSpeed

  Description:
    Set referenced speed on PHY3340.
    The function sets the speeds to be advertised to the
    link partner and enables auto-negotiation.

 Inputs:
    devNum        - device number
    port          - port number
    speed_bits    - speed
                                0x0002 10M
                                0x0008 100M
                                0x0020 1G
                                0x0040 10G
                                0x0800 2.5GBASE-T FD 88X33X0 family only
                                0x1000 5GBASE-T full-duplex, 88X33X0 family only
 Returns:
    GT_OK or GT_FAIL
******************************************************************************/
GT_STATUS appAldrin2RdEnableSpeed
(
    IN GT_U32 devNum,
    IN GT_U16 portNum,
    IN GT_U16 speed_bits
)
{
    GT_STATUS rc;
    APPDEMO_PHY_INFO    context;
    GT_U16 smiAddr = 0;
    CTX_PTR_TYPE contextPtr;
    GT_U32 i;

    for (i = 0; portToPhyArrayAldrin2Rd[i][0] != 0xff; i++)
    {
        if (portNum ==  portToPhyArrayAldrin2Rd[i][0])
        {
            smiAddr =  portToPhyArrayAldrin2Rd[i][1];
            break;
        }
    }

    if (0xff ==  portToPhyArrayAldrin2Rd[i][0])
    {
       cpssOsPrintf("wrong port number %d\n", portNum);
       return GT_FAIL;
    }

    if (portNum >= 24 && portNum <= 35)/* ports 24-35 are XSMI 0*/
    {
        context.xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E;
    }
    else if (portNum >= 36 && portNum <= 47)/* ports 36-47 are XSMI 1*/
    {
        context.xsmiInterface = CPSS_PHY_XSMI_INTERFACE_1_E;
    }
    else if ((portNum >= 48 && portNum <= 58)||(portNum == 64))/* ports 48-58,64 are XSMI 2*/
    {
        context.xsmiInterface = CPSS_PHY_XSMI_INTERFACE_2_E;
    }
    else if (portNum >= 65 && portNum <= 76)/* ports 65-76 are XSMI 3*/
    {
        context.xsmiInterface = CPSS_PHY_XSMI_INTERFACE_3_E;
    }
    else
    {
       cpssOsPrintf("wrong port number %d\n", portNum);
       return GT_FAIL;
    }

    context.devNum = devNum;
    context.portGroup = 0;
    contextPtr = &context;

    cpssOsPrintf("appAldrin2RdEnableSpeed for port %d speed 0x%x\n",portNum,speed_bits);

    /* Writing values for the Phy registers */
    /* for 1G,100M and 10M need to write values to a specified XSMI Register */
    switch (speed_bits)
    {
        case 0x20:/*1G*/
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x4);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x8004);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x20, 7, 0x0);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x8000, 7, 0x310);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x0, 7, 0x3200);
            break;

        case 0x8:/*100M*/
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x1C);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x801C);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x10, 7, 0x1181);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x8000, 7, 0x10);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x20, 7, 0x0);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x0, 7, 0x3200);
            break;

        case 0x2: /*10M*/
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x1C);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0xf001, 31, 0x801C);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x10, 7, 0x1061);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x8000, 7, 0x10);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x20, 7, 0x0);
            cpssXsmiRegisterWrite(0, context.xsmiInterface, smiAddr, 0x0, 7, 0x3200);
            break;

        default:/*for all the other speeds*/
            rc = mtdEnableSpeeds(contextPtr, smiAddr, speed_bits, GT_TRUE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("mtdEnableSpeeds(%d,0x%x):rc=%d\n",
                            context.xsmiInterface, smiAddr, rc);
                return rc;
            }
    }

    return GT_OK;

}

/******************************************************************************************
  Description:
    Set referenced speed on PHY 883x40.
    The function sets the speeds to be advertised to the
    link partner and enables auto-negotiation.

 Inputs:
    devNum        - device number
    port          - port number
    speed_bits    - speed      0x0001 1M HD
                               0x0002 10M FD
                               0x0004 100M HD
                               0x0008 100M FD
                               0x0010 1G HD
                               0x0020 1G FD
                               0x0040 10G FD
                               0x0800  2.5GBASE-T FD 88X33X0 family only
                               0x1000  5GBASE-T full-duplex, 88X33X0 family only

 Returns:
    G_OK or GT_FAIL
******************************************************************************/
GT_STATUS appAldrinRdEnableSpeed
(
    IN GT_U32 devNum,
    IN GT_U16 portNum,
    IN GT_U16 speed_bits
)
{
    GT_STATUS rc;
    APPDEMO_PHY_INFO    context;
    GT_U16 smiAddr = 0;
    CTX_PTR_TYPE contextPtr;
    GT_U32 i;

    for (i = 0; portToPhyArrayAldrinRd[i][0] != 0xff; i++)
    {
        if (portNum == portToPhyArrayAldrinRd[i][0])
        {
            smiAddr = portToPhyArrayAldrinRd[i][1];
            break;
        }
    }

    if (0xff == portToPhyArrayAldrinRd[i][0])
    {
       cpssOsPrintf("wrong port number %d\n", portNum);
       return GT_FAIL;
    }


    context.xsmiInterface = (portNum < 20) ? CPSS_PHY_XSMI_INTERFACE_1_E :
                                            CPSS_PHY_XSMI_INTERFACE_0_E;
    context.devNum = devNum;
    context.portGroup = 0;
    contextPtr = &context;

    cpssOsPrintf("appAldrinRdEnableSpeed for port %d speed 0x%x\n",portNum,speed_bits);

    rc = mtdEnableSpeeds(contextPtr, smiAddr, speed_bits, GT_TRUE);
    if (rc != GT_OK)
    {
        cpssOsPrintf("mtdEnableSpeeds(%d,0x%x):rc=%d\n",
                        context.xsmiInterface, smiAddr, rc);
        return rc;
    }

    return GT_OK;

}



/******************************************************************************************
  Description:
    Set referenced speed on PHY 883x40.
    The function sets the speeds to be advertised to the
    link partner and enables auto-negotiation.

    Should be called after bokb port creation

GT_STATUS appBobkEnableSpeed(IN GT_U32 devNum,
                             IN GT_U16 port,
        `                    IN GT_U16 speed_bit);

 Inputs:
    devNum        - device number
    port          - port number
    speed_bits    - speed      0x0001 1M HD
                               0x0002 10M FD
                               0x0004 100M HD
                               0x0008 100M FD
                               0x0010 1G HD
                               0x0020 1G FD
                               0x0040 10G FD
                               0x0800  2.5GBASE-T FD 88X33X0 family only
                               0x1000  5GBASE-T full-duplex, 88X33X0 family only

 Returns:
    G_OK or GT_FAIL
******************************************************************************/
GT_STATUS appBobkEnableSpeed
(
    IN GT_U32 devNum,
    IN GT_U16 port,
    IN GT_U16 speed_bits
)
{
    GT_STATUS rc;
    CTX_PTR_TYPE contextPtr;
    PDEVICE_INFO context;
    GT_U16 autoNegEnabled;
    GT_U16 smiAddr = 0,speed_val, currInd;

    context.devNum = devNum;
    context.portGroup = 0/*CPSS_PORT_GROUP_UNAWARE_MODE_CNS*/;
    context.xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E;

    contextPtr = &context;
    for (currInd = 0;currInd < MAX_PHY_SMI_ADDR; currInd++)
    {
        if (port2PhySmiAddrTable[currInd].portNum == port)
        {
            smiAddr = port2PhySmiAddrTable[currInd].phySmiAddr;
            break;
        }
    }
    if (currInd ==  MAX_PHY_SMI_ADDR)
    {
       cpssOsPrintf("wrong port number %d\n", port);
       return GT_FAIL;
    }
    else
    {
        if (port == 65) /* fix polarity swap on port 65 */
        {
            rc = mtdHwXmdioWrite(contextPtr, smiAddr, 4,0xf004,0x1004);

            if(rc != GT_OK)
                return rc;
        }
        rc = mtdHwGetPhyRegField(contextPtr, smiAddr, 7, 0, 12, 1, &autoNegEnabled);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = mtdGetSpeedsEnabled(contextPtr,smiAddr, &speed_val);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ( (autoNegEnabled == 0) || (speed_val != speed_bits) )
        {
            rc = mtdEnableSpeeds(contextPtr, smiAddr,speed_bits,GT_TRUE);
        }
     }
     return rc;
}

/**
* @internal appDemoBobkEom function
* @endinternal
*
* @brief   Displays on the screen serdes vbtc and hbtc calculation and eye matrix.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
* @param[in] noeye                    - if 1 - Displays on the screen serdes vbtc and hbtc calculation only and not displays eye matrix.
* @param[in] min_dwell_bits           - minimum dwell bits - It is lower limit on how long to sample at each data point.
*                                      valid range: 100000 --- 100000000 .If 0 - transform to default 100000.
* @param[in] max_dwell_bits           - maximum dwell bits - It is the upper limit on how long to sample at each data point.
*                                      valid range: 100000 --- 100000000 .If 0 - transform to default 100000000.
*                                      should be min_dwell_bits <= max_dwell_bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval else                     - on error
*/
GT_STATUS appDemoBobkEom
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  GT_U32                           serdesNum,
    IN  GT_BOOL                          noeye,
    IN  GT_U32                           min_dwell_bits,
    IN  GT_U32                           max_dwell_bits
)
{
    GT_STATUS rc;
    GT_U32    len_matrix;
    GT_U32    i;
    CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC eye_results;
    CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC eye_input;

    GT_U32 dwell_bits_bottom = 100000;
    GT_U32 dwell_bits_top = 100000000;
    if(0 == min_dwell_bits)
    {
        min_dwell_bits = dwell_bits_bottom;
    }
    if(0 == max_dwell_bits)
    {
        max_dwell_bits = dwell_bits_top;
    }
    if((min_dwell_bits < dwell_bits_bottom) || (min_dwell_bits > dwell_bits_top))
    {
         cpssOsPrintSync("minimum dwell bits = %d is out of range:   %d - %d\n", min_dwell_bits, dwell_bits_bottom, dwell_bits_top);
         return GT_BAD_PARAM;
    }
    if((max_dwell_bits < dwell_bits_bottom) || (max_dwell_bits > dwell_bits_top))
    {
         cpssOsPrintSync("maximum dwell bits = %d is out of range:   %d - %d\n", max_dwell_bits, dwell_bits_bottom, dwell_bits_top);
         return GT_BAD_PARAM;
    }
    if(max_dwell_bits < min_dwell_bits)
    {
         cpssOsPrintSync("maximum dwell bits = %d is less then minimum dwell bits = %d\n", max_dwell_bits, min_dwell_bits);
         return GT_BAD_PARAM;
    }

    eye_input.min_dwell_bits = min_dwell_bits;
    eye_input.max_dwell_bits = max_dwell_bits;
    eye_results.matrixPtr = NULL;
    eye_results.vbtcPtr = NULL;
    eye_results.hbtcPtr = NULL;
    rc=cpssDxChPortSerdesEyeMatrixGet(devNum, portNum, serdesNum, &eye_input, &eye_results);
    if(GT_OK == rc)
    {
        cpssOsPrintSync("\nSerDes No. %d: has an eye height of %d mV.\n", eye_results.globalSerdesNum, eye_results.height_mv);
        cpssOsPrintSync("SerDes No. %d: has an eye width of  %d mUI.\n", eye_results.globalSerdesNum, eye_results.width_mui);
        if(NULL != eye_results.vbtcPtr)
        {
            cpssOsPrintSync("%s", eye_results.vbtcPtr);
        }
        if(NULL != eye_results.hbtcPtr)
        {
            cpssOsPrintSync("%s\n", eye_results.hbtcPtr);
        }
        if(noeye == GT_FALSE)
        {
            if(NULL != eye_results.matrixPtr)
            {
                len_matrix = cpssOsStrlen(eye_results.matrixPtr);
                for(i = 0; i < len_matrix; i++)
                {
                    cpssOsPrintSync("%c", eye_results.matrixPtr[i]);
                }
            }
        }
    }
    else
    {
        cpssOsPrintSync("Can't get eye data\n");
        return rc;
    }

    if(NULL != eye_results.matrixPtr)
    {
        cpssOsFree(eye_results.matrixPtr);
        eye_results.matrixPtr = NULL;
    }
    if(NULL != eye_results.vbtcPtr)
    {
        cpssOsFree(eye_results.vbtcPtr);
        eye_results.vbtcPtr = NULL;
    }
    if(NULL != eye_results.hbtcPtr)
    {
        cpssOsFree(eye_results.hbtcPtr);
        eye_results.hbtcPtr = NULL;
    }
    cpssOsPrintSync("\n");
    return GT_OK;
}

#endif

extern GT_BOOL  changeToDownHappend[CPSS_MAX_PORTS_NUM_CNS];
extern GT_STATUS intMaskSet
(
    IN  GT_U8                   dev,
    IN  GT_PHYSICAL_PORT_NUM    port,
    IN  CPSS_EVENT_MASK_SET_ENT operation
);

/**
* @internal prvAppDemoPortsConfig function
* @endinternal
*
* @brief   Init required ports of Bobcat2, Bobk and Aldrin to specified ifMode and speed for init system
*
* @param[in] devNumber                - device number in appDemo
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] powerUp                  -   GT_TRUE - port power up
*                                      GT_FALSE - serdes power down
* @param[in] squelch                  - threshold for signal OK (0-15) to change - If out range - ignored
*                                      - to not change  squelch - use value out of range, for example 0xFF
* @param[in] initPortsBmp             - physical ports numbers bitmap (or CPU port)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvAppDemoPortsConfig
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          squelch,
    CPSS_PORTS_BMP_STC                  initPortsBmp
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      i;                  /* interator */
    GT_PHYSICAL_PORT_NUM portNum=0; /* port number */
    CPSS_PORTS_BMP_STC *initPortsBmpPtr;/* pointer to bitmap */
    CPSS_PORTS_BMP_STC sdTrainingBmp; /* bitmap of ports to run serdes
                                                    auto-tuning/taining on */
    GT_U32  serdesTraining = 0; /* serdes training required or not */
    GT_U32  trxTraining;    /* what kind of auto-tuning run on port */
    GT_BOOL isLinkUp;       /* link status on port */
    GT_BOOL serdesTrainingVsr = GT_FALSE;
/* #ifndef ASIC_SIMULATION */
    GT_BOOL signalState = GT_TRUE;    /* is there signal on serdes */
/* #endif */
    CPSS_PORT_INTERFACE_MODE_ENT    currentIfMode;  /* interface mode configured
                                                                already on port */
    CPSS_PORT_SPEED_ENT             currentSpeed;   /* speed configured
                                                                already on port */
    GT_U8                           dev;   /* drvice number in cpss */
    GT_U32  currBoardType;

    initPortsBmpPtr = &initPortsBmp;
    dev = appDemoPpConfigList[devNumber].devNum;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Not implemented for Bobcat2 A0 */
    if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == PRV_CPSS_PP_MAC(appDemoPpConfigList[dev].devNum)->devFamily)
        &&(CPSS_PP_SUB_FAMILY_NONE_E == PRV_CPSS_PP_MAC(appDemoPpConfigList[dev].devNum)->devSubFamily)
        && (PRV_CPSS_PP_MAC(dev)->revision == 0))
    {
        return GT_FAIL;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&sdTrainingBmp);

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(initPortsBmpPtr, portNum))
        {
            continue;
        }
        rc = cpssDxChPortInterfaceModeGet(dev, portNum, &currentIfMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortSpeedGet(dev, portNum, &currentSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if((GT_TRUE == powerUp) && (currentIfMode == ifMode) && (currentSpeed == speed))
        {
            rc = cpssDxChPortLinkStatusGet(dev, portNum, &isLinkUp);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(isLinkUp)
            { /* nothing to do this is just reconfiguration */
                continue;
            }
        }

        rc = cpssDxChPortEnableSet(dev, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortEnableSet", rc);
            return rc;
        }

        /* In BobK devices port 90 is 1GIG only so the interrupts masking workaround is not needed.
           Port 90 in those devices is connected to external phy that will be configured next */
        /*
            In Cetus and Caelum DB and Cygnus and Lewis RD boards port 90 is connected to external 88E1512 phy that
            needs to be configured. No furthur training sequence and masking should be performed.
        */

        if (portNum == 90 && (currBoardType == APP_DEMO_CETUS_BOARD_DB_CNS ||
             currBoardType == APP_DEMO_CAELUM_BOARD_DB_CNS                 ||
             currBoardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS          ||
             currBoardType == APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS) )
        {
            if ((GT_TRUE == powerUp))
            {
                CPSS_PORTS_BMP_STC portsBmp;
                /* Switching 88E1512 phy page selector to page 0x12, setting SGMII mode and reset,
                   then restoring page selector to 0 */
                rc = cpssDxChPhyPortSmiRegisterWrite(dev, 90, 0x16, 0x12);
                if(rc != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    return rc;
                }
                rc = cpssDxChPhyPortSmiRegisterWrite(dev, 90, 0x14, 0x8001);
                if(rc != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    return rc;
                }
                rc = cpssDxChPhyPortSmiRegisterWrite(dev, 90, 0x16, 0x0);
                if(rc != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    return rc;
                }
                /* Continuing regular sequence of setting port mode and speed, and port enable */

                /* Removing port 90 from inputed initial port bitmap */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&initPortsBmp, portNum);

                /* Creating for port 90 special port bitmap */
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

                rc = cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed);
                if(rc != GT_OK)
                {
                    /* TRACE commented out due to JIRA CPSS-5442 */
                    /*CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);*/
                    return rc;
                }
                rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
                    return rc;
                }
                continue;
            }
        }
        else
        {
            rc = waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_MASK_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, powerUp, ifMode, speed);
    if(rc != GT_OK)
    {
        /* TRACE commented out due to JIRA CPSS-5442 */
        /*CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);*/
        return rc;
    }

    if(0 == powerUp)
    {/* don't unmask interrupts if ports powered down to not get them when port
        will be restored */
        return GT_OK;
    }

    if ((PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) &&
        ((CPSS_PORT_INTERFACE_MODE_SGMII_E== ifMode) ||
         (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E== ifMode) ||
         ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) && (CPSS_PORT_SPEED_5000_E == speed)))) {
        serdesTrainingVsr = GT_TRUE;
    }

    if ((   (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) && (CPSS_PORT_SPEED_5000_E != speed))
         || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
         ||(CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
         ||(CPSS_PORT_INTERFACE_MODE_XHGS_E  == ifMode)
         ||(CPSS_PORT_INTERFACE_MODE_XHGS_SR_E  == ifMode)
        /* if vsr training is needed */
         || (serdesTrainingVsr)
       )
    {
        if(appDemoDbEntryGet("serdesTraining", &serdesTraining) != GT_OK)
        {
            serdesTraining = 1;
        }
    }
    else
    {
        serdesTraining = 0;
    }

    if(serdesTraining)
    {
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&sdTrainingBmp, &sdTrainingBmp, initPortsBmpPtr);
    }

    if(appDemoDbEntryGet("trxTraining", &trxTraining) != GT_OK)
    {
        trxTraining = 0;
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(initPortsBmpPtr, portNum))
        {
            continue;
        }
        if (serdesTrainingVsr)
        {
            /* Due to issue in Aldrin RD phy, do not perform VSR for 1G SGMII, however treat it as low level speed */
            if (!((CPSS_PORT_INTERFACE_MODE_SGMII_E == ifMode) && (CPSS_PORT_SPEED_1000_E == speed) &&
                (APP_DEMO_ALDRIN_BOARD_RD_CNS == currBoardType)))
            {
                rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                            CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E);
                if (rc != GT_OK) {
                    cpssOsPrintf("bobkAppDemoPortsConfig:RX training vsr failed:portNum=%d:rc=%d\n", portNum, rc);
                }
            }
            goto noGearBoxLock;
        }

        /* if squelch delivered explicitly to the this API */
        if(squelch <= 15)
        {
            rc = cpssDxChPortSerdesSquelchSet(dev, portNum, squelch);
            if(GT_OK != rc)
            {
                cpssOsPrintSync("cpssDxChPortSerdesSquelchSet failed:  dev=%d, portNum=%d, squelch=%d, rc=%d\n",dev, portNum, squelch,rc);
            }
        }
        else
        {
            /* overriding default squelch for some cases */
            if (currBoardType == APP_DEMO_BOBCAT3_BOARD_DB_CNS ||
                currBoardType == APP_DEMO_ALDRIN2_BOARD_DB_CNS ||
                currBoardType == APP_DEMO_ALDRIN_BOARD_DB_CNS)
            {
                /* default HWS squelch for some of the port modes on those DB boards can be problematic,
                   so there is a need to set a custom value */
                if (speed == CPSS_PORT_SPEED_10000_E)
                {
                    /* value 2 is the normalized value of signal_ok level (squelch) of value 100mV, to Avago API units */
                    rc = cpssDxChPortSerdesSquelchSet(dev, portNum, 2);
                    if(GT_OK != rc)
                    {
                        cpssOsPrintSync("cpssDxChPortSerdesSquelchSet failed:  dev=%d, portNum=%d, squelch=100mV, rc=%d\n",
                                        dev, portNum, rc);
                        return rc;
                    }
                }
            }
        }

        TRAINING_DBG_PRINT_MAC(("portNum=%d:ifMode=%d,speed=%d\n", portNum, ifMode, speed));
/* #ifndef ASIC_SIMULATION */

        if(!trxTraining)
        {
            if((CPSS_PP_FAMILY_DXCH_XCAT3_E == appDemoPpConfigList[dev].devFamily) ||
               (PRV_CPSS_SIP_5_15_CHECK_MAC(dev))) /* Bobk, Aldrin, AC3X, Bobcat3 */
            {
                /* check that Serdes Signal detected on port is OK and stable - on all its serdes lanes */
                if(1 == serdesTraining)
                {
                    rc = cpssDxChPortPcsSyncStableStatusGet(dev, portNum, &signalState);
                    if (rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssDxChPortPcsSyncStableStatusGet fail port=%d,rc=%d\n",
                                     portNum, rc);
                        continue;
                    }
                }
            }
            else
            {
                /* check that PCS sync OK detected on port - on all its serdes lanes */
                rc = cpssDxChPortPcsSyncStatusGet(dev, portNum, &signalState);
                if (rc != GT_OK)
                {
                    cpssOsPrintSync("cpssDxChPortPcsSyncStatusGet fail port=%d,rc=%d\n",
                                 portNum, rc);
                    continue;
                }
            }

        }
        else
        {
            rc = cpssDxChPortSerdesSignalDetectGet(dev, portNum, &signalState);
            if (rc != GT_OK)
            {
                cpssOsPrintSync("cpssDxChPortSerdesSignalDetectGet fail port=%d,rc=%d\n",
                             portNum, rc);
                continue;
            }
        }
        TRAINING_DBG_PRINT_MAC(("port=%d:signalState=%d\n", portNum, signalState));
        if(!signalState)
        {
            changeToDownHappend[portNum] = GT_TRUE;
            rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
            if(rc != GT_OK)
            {
                return rc;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
            continue;
        }
        else
        {
             changeToDownHappend[portNum] = GT_FALSE;
        }

        if(serdesTraining)
        {
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
                {
                    continue;
                }
                TRAINING_DBG_PRINT_MAC(("port=%d:start training\n", portNum));
                if(trxTraining)
                {/* if other side started TRX training no gearBox lock could be */
                    cpssOsPrintf("SERDES TRx training - port %d\n",portNum);

                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssDxChPortSerdesAutoTune(TRX_CFG,portNum=%d):rc=%d\n", portNum, rc);
                        return rc;
                    }

                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssDxChPortSerdesAutoTune(TRX_START,portNum=%d):rc=%d\n", portNum, rc);
                        return rc;
                    }
                }
                else
                {
                    GT_BOOL gearBoxLock;

                    for(i = 0; i < 100; i++)
                    {
                        rc = cpssDxChPortPcsGearBoxStatusGet(dev, portNum,
                                                                    &gearBoxLock);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("cpssDxChPortPcsGearBoxStatusGet:rc=%d,portNum=%d\n",
                                            rc, portNum);
                            return rc;
                        }
                        if(gearBoxLock)
                        {
                            break;
                        }
                        cpssOsTimerWkAfter(10);
                    }

                    if (!gearBoxLock)
                    {
                        changeToDownHappend[portNum] = GT_TRUE;
                        rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsPrintf("no gearBoxLock - port %d\n",portNum);
                        goto noGearBoxLock;
                    }

                    cpssOsPrintf("SERDES RX training - port %d\n",portNum);
                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintf("bobkAppDemoPortsConfig:RX training failed:portNum=%d:rc=%d\n", portNum, rc);
                        changeToDownHappend[portNum] = GT_TRUE;
                        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
                        rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                        }
                        continue;
                    }
                    if((CPSS_PP_FAMILY_DXCH_XCAT3_E == appDemoPpConfigList[dev].devFamily) ||
                       (PRV_CPSS_SIP_5_15_CHECK_MAC(dev))) /* Bobk, Aldrin, AC3X, Bobcat3 */
                    {
                         CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
                         CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;
                         int i;
                         for(i = 0; i < 100; i++)
                         {
                             rc = cpssDxChPortSerdesAutoTuneStatusGet(dev, portNum, &rxTuneStatus, &txTuneStatus);
                             if(rc == GT_OK)
                             {
                                  if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E == rxTuneStatus)
                                  {
                                      rc = waTriggerInterruptsMask(dev, portNum,
                                                             /* here important not RXAUI and not XGMII */
                                                             CPSS_PORT_INTERFACE_MODE_KR_E,
                                                             CPSS_EVENT_UNMASK_E);

                                      if(rc != GT_OK)
                                      {
                                           cpssOsPrintSync("WaitRxTrainingFinishTask:waTriggerInterruptsMask(portNum=%d):rc=%d\n",
                                                        portNum, rc);
                                      }
                                      break;
                                  }
                             }
                             else
                             {
                                  cpssOsPrintf("bobkAppDemoPortsConfig:RX training failed:portNum=%d:rc=%d\n", portNum, rc);
                                  goto noGearBoxLock;
                             }
                             cpssOsTimerWkAfter(10);
                         }
                         if(100 == i)
                         {
                             cpssOsPrintf("bobkAppDemoPortsConfig:RX training failed:portNum=%d:rc=%d\n", portNum, rc);
                             goto noGearBoxLock;
                         }
                    }
                }
            }
        }
        else
        {
            if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
                && (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E))
            {
                rc = appDemoDxChLion2PortLinkWa(dev, portNum);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("appDemoDxChLion2PortLinkWa(portNum=%d):rc=%d\n", portNum, rc);
                }
            }

            rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                cpssOsPrintSync("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
            }

           (GT_VOID)waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_UNMASK_E);

            continue;
        }
/* #endif */
    }

    if ((   (CPSS_PORT_INTERFACE_MODE_KR_E    == ifMode) && (CPSS_PORT_SPEED_5000_E != speed))
         || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_E  == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E  == ifMode)
       )
    {
        if(trxTraining)
        {
            cpssOsTimerWkAfter(500);
        }

        for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
            {
                continue;
            }

            if(trxTraining)
            {
#ifndef ASIC_SIMULATION
                rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("TRX training failed on portNum=%d:rc=%d\n",
                                                                  portNum, rc);
                    changeToDownHappend[portNum] = GT_TRUE;
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
                    rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                    }
                    continue;
                }

                cpssOsPrintf("port %d Trx training PASS\n", portNum);
#endif
            }
        }

        if(trxTraining)
        {
            cpssOsTimerWkAfter(10);
        }

        for(i = 0; i < 10; i++)
        {
            for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
                {
                    continue;
                }

                if((i > 0) && (!changeToDownHappend[portNum]))
                {/* WA succeeded in one of previous iterations */
                    continue;
                }

                if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
                      && (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E))
                {
                     TRAINING_DBG_PRINT_MAC(("i=%d)appDemoDxChLion2PortLinkWa(portNum=%d):",
                                            i, portNum));
                     rc = appDemoDxChLion2PortLinkWa(dev, portNum);
                }
                else
                {
                    rc = GT_OK;
                }
                if(rc != GT_OK)
                {
                    if(rc != GT_NOT_READY)
                    {
                        cpssOsPrintSync("ConfigPortsDefaultByProfile:appDemoDxChLion2PortLinkWa(portNum=%d):rc=%d\n",
                                        portNum, rc);
                    }
                    changeToDownHappend[portNum] = GT_TRUE;
                }
                else
                {
                    rc = cpssDxChPortLinkStatusGet(dev, portNum, &isLinkUp);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("bobkAppDemoPortsConfig:cpssDxChPortLinkStatusGet(portNum=%d):rc=%d\n",
                                        portNum, rc);
                        return rc;
                    }
                    if(isLinkUp)
                    {
                          changeToDownHappend[portNum] = GT_FALSE;
                    }
                }
                TRAINING_DBG_PRINT_MAC(("rc=%d,changeToDownHappend=%d\n",
                                        rc, changeToDownHappend[portNum]));
            }
            cpssOsTimerWkAfter(10);
        }

        for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
            {
                continue;
            }

            if(!changeToDownHappend[portNum])
            {
                rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortEnableSet", rc);
                    return rc;
                }
            }
            else
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
/* #ifndef ASIC_SIMULATION */
                rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                }
/* #endif */
                TRAINING_DBG_PRINT_MAC(("ConfigPortsDefaultByProfile:portNum=%d down\n",
                                portNum));
            }
        }
    }

/* #ifndef ASIC_SIMULATION */
noGearBoxLock:
/* #endif */
    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(initPortsBmpPtr, portNum))
        {
            continue;
        }

        rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintSync("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
        }

        if(signalState)
        {
            (GT_VOID)waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_UNMASK_E);
        }
        else
        {
            (GT_VOID)waTriggerInterruptsMaskNoLinkStatusChangeInterrupt(dev, portNum, ifMode, CPSS_EVENT_UNMASK_E);
        }
    }

    return GT_OK;
}

/**
* @internal bobkAppDemoPortsConfig function
* @endinternal
*
* @brief   Init required ports of Bobk and Aldrin to specified ifMode and speed for init system
*
* @param[in] devNumber                - device number in appDemo
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] powerUp                  -   GT_TRUE - port power up
*                                      GT_FALSE - serdes power down
* @param[in] squelch                  - threshold for signal OK (0-15) to change - If out range - ignored
*                                      - to not change  squelch - use value out of range, for example 0xFF
* @param[in] numOfPorts               - quantity of ports to configure
*                                      use 0 if all device ports wanted
*                                      ... - numbers of ports to configure
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bobkAppDemoPortsConfig
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          squelch,
    IN  GT_U32                          numOfPorts,
    ...
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      i;                  /* interator */
    GT_PHYSICAL_PORT_NUM portNum=0; /* port number */
    va_list     ap;                 /* arguments list pointer */

    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                        *initPortsBmpPtr;/* pointer to bitmap */

    initPortsBmpPtr = &initPortsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
    va_start(ap, numOfPorts);
    for(i = 1; i <= numOfPorts; i++)
    {
        portNum = va_arg(ap, GT_U32);
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portNum);
    }
    va_end(ap);
    rc = prvAppDemoPortsConfig(devNumber, ifMode, speed, powerUp, squelch, initPortsBmp);
    return rc;
}


