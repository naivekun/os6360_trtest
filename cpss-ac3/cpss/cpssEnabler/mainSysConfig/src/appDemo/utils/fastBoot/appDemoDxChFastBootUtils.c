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
* @file appDemoDxChFastBootUtils.c
*
* @brief App demo DxCh FastBoot API.
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/cpssDxChCatchUpDbg.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <appDemo/userExit/userEventHandler.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/dxCh/dxChxGen/port/private/prcCpssDxChPortStat.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

/*16 bytes alignment for tx descriptor*/
#define TX_DESC_DMA_ALIGN_BYTES_CNS  16
/*128 bytes alignment for packets*/
#define PACKET_DMA_ALIGN_BYTES_CNS  128
/*12 bytes of mac DA,SA*/
#define MAC_ADDR_LENGTH_CNS         12

/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS  32

/**
* @internal sendPacketByTxDescriptor_HardCodedRegAddr function
* @endinternal
*
* @brief   The function put the descriptor in the queue and then trigger the queue
*
* @note   APPLICABLE DEVICES:      All.
* @param[in] devNum                   - device number
*                                      txDesc     - (pointer to) The descriptor used for that packet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
static GT_STATUS  sendPacketByTxDescriptor_HardCodedRegAddr
(
    GT_U8                   devNum,
    PRV_CPSS_TX_DESC_STC    *txDescPtr
)
{
    GT_STATUS rc;
    GT_U32  portGroupId = 0;
    GT_UINTPTR  physicalAddrOfTxDescPtr;         /* Holds the real buffer pointer.       */
    GT_U32 queue = 0;

    /* 1. put the descriptor in the queue */
    rc = cpssOsVirt2Phy((GT_UINTPTR)txDescPtr,/*OUT*/&physicalAddrOfTxDescPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (physicalAddrOfTxDescPtr & 0xffffffff00000000L))
        {
            return GT_OUT_OF_RANGE;
        }
    #endif

    rc = prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId,
        0x000026C0 + (4*queue), /*addrPtr->sdmaRegs.txDmaCdp[queue]*/
        (GT_U32)physicalAddrOfTxDescPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* The Enable DMA operation should be done only */
    /* AFTER all desc. operations where completed.  */
    GT_SYNC;

    /* 2. trigger the queue */

    /* Enable the Tx DMA.   */
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
        0x00002868,/*addrPtr->sdmaRegs.txQCmdReg*/
        (1<<queue));

    return GT_OK;
}

extern GT_VOID prvCpssDxChPortSumMibEntry
(
    IN  GT_U8  devNum,
    INOUT GT_U32 destEntryArr[/*PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN    GT_U32 addEntryArr[/*PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/]
);

/**
* @internal prvDxChPortMacCountersCGPortReadMem_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets all CG mac mib counters from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
*
* @param[out] cgMacMibsCaptureArr      - (pointer to) array of 64bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Based on prvCpssDxChPortMacCountersCGPortReadMem.
*
*/
static GT_STATUS prvDxChPortMacCountersCGPortReadMem_HardCodedRegAddr
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U64  *cgMacMibsCaptureArr /* should be sized CPSS_CG_LAST_E */
)
{
    GT_U32 *cgMacMibsRegsAddrDBArrPtr;    /* The CG MIB counters addresses */
    GT_U32  cgMacMibsRegsAddrDBLength;   /* The CG MIB counters length */
    GT_U32  i;   /* Iterator */
    GT_U32  regAddr;    /* CG MIB counter register address */
    GT_U32  portMacNum;  /* Mac number assosiated with the portNum */
    GT_U32  portGroupId; /* Group id assosiated with the portNum */
    GT_STATUS rc = GT_OK;
    GT_BOOL canReadCg;
    /* Arguments check */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsCaptureArr);
    if (portNum > 20)
    {
        return GT_BAD_PARAM;
    }

    portMacNum = portNum;

    portGroupId = 0;

    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
    canReadCg = mvHwsCgMac28nmAccessGet(devNum, portGroupId, portMacNum);
    if (canReadCg == GT_FALSE) {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        for (i=0 ; i<CPSS_CG_LAST_E ; i++)
        {
            cgMacMibsCaptureArr[i].l[0] = 0;
            cgMacMibsCaptureArr[i].l[1] = 0;
        }
        return GT_OK;
    }

    /* Capturing a new snapshot of the counters */

    /* CG unit requires a capture command to be performed prior to capturing counters for a port */
    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Failed to perform capture command on CG port");
    }
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,0/*offset*/,32/*length*/,0x32/*captureRX, captureTX, clearOnRead*/);
    if (rc!=GT_OK)
    {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        return rc;
    }

    /* Getting length of MIB regs array size */
    cgMacMibsRegsAddrDBLength = CPSS_CG_LAST_E;
    /* get reference to the CG MIB register addresses array */
    cgMacMibsRegsAddrDBArrPtr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(devNum,portMacNum);
    if (cgMacMibsRegsAddrDBArrPtr == NULL) {
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }

    /* Null pointer check */
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsRegsAddrDBArrPtr);

    /* Reading content of all CG MIBS registers */
    for (i=0 ; i<cgMacMibsRegsAddrDBLength ; i++)
    {
        /* Getting mib counter address */
        regAddr = cgMacMibsRegsAddrDBArrPtr[i];

        /* Reading first 32bits */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[0]));
        if ( rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read first CG mib counter entry from address 0x%08x", regAddr);
        }
        /* Reading second 32bits */
        regAddr += 4;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[1]));
        if ( rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read second CG mib counter entry from address 0x%08x", regAddr);
        }
    }
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacCountersCGMibsConvert_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets CG ethernet MAC counters and convert to legacy mibs structure.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number associated with channel
*
* @param[out] xlgMibEntry              - (pointer to) array of 32bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @note Based on prvCpssDxChPortMacCountersCGMibsConvert
*/
static GT_STATUS prvCpssDxChPortMacCountersCGMibsConvert_HardCodedRegAddr
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
)
{
    GT_U64  cgMacMibsCaptureArr[CPSS_CG_LAST_E];   /* Array to load to all CG counters from HW */
    GT_U64  covertedCgMacMibCounterEntry ;   /* Single counter entry */
    GT_U64  temp64;
    GT_U32  i; /* iterator */
    GT_U32  portMacNum; /* Mac number assosiated with the portNum */
    GT_STATUS rc = GT_OK;
    CPSS_PORT_MAC_COUNTERS_ENT cntrName;    /* Counter names iterator */

    /* Reading CG mibs from HW */
    rc = prvDxChPortMacCountersCGPortReadMem_HardCodedRegAddr(devNum, portNum, &cgMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }
    /* Converting CG MIBS to regular MIBS xlgMibEntry struct. Not all cg mib counters
       are needed for the legacy mib convention. */
    cpssOsMemSet(xlgMibEntry, 0, sizeof(GT_U32) * PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS);

    /* For 100G ports portMacNum=portNum */
    portMacNum = portNum;

    /* For all MIB counter */
    for(cntrName = 0; cntrName < CPSS_LAST_MAC_COUNTER_NUM_E; cntrName++)
    {
        covertedCgMacMibCounterEntry.l[0]=0;
        covertedCgMacMibCounterEntry.l[1]=0;

        /* Filtering unsupported counters */
        if(!prvCpssDxChPortMacCountersIsSupportedCheck(devNum, portMacNum, cntrName,
                                    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E/*argument relevant for AC3*/))
        {
            continue;
        }

        /* Converting a counter */
        switch (cntrName)
        {
            case CPSS_GOOD_OCTETS_RCV_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E], temp64);
                break;
            case CPSS_BAD_OCTETS_RCV_E:
                covertedCgMacMibCounterEntry = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OCTETS_E],
                                                                cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E]);
                break;
            case CPSS_MAC_TRANSMIT_ERR_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_ERRORS_E];
                break;
            case CPSS_GOOD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BAD_PKTS_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_BRDC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_BC_PKTS_E];
                break;
            case CPSS_MC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_MC_PKTS_E];
                break;
            case CPSS_PKTS_64_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E];
                break;
            case CPSS_PKTS_65TO127_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E];
                break;
            case CPSS_PKTS_128TO255_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E];
                break;
            case CPSS_PKTS_256TO511_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E];
                break;
            case CPSS_PKTS_512TO1023_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
                break;
            case CPSS_PKTS_1024TOMAX_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = prvCpssMathAdd64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E],
                            cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E]);
                break;
            case CPSS_GOOD_OCTETS_SENT_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_OUT_OCTETS_E], temp64);
                break;
            case CPSS_GOOD_PKTS_SENT_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_EXCESSIVE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_MC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_MC_PKTS_E];
                break;
            case CPSS_BRDC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_BC_PKTS_E];
                break;
            case CPSS_UNRECOG_MAC_CNTR_RCV_E:
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_FC_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
                break;
            case CPSS_GOOD_FC_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];
                break;
            case CPSS_DROP_EVENTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_DROP_EVENTS_E];
                break;
            case CPSS_UNDERSIZE_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E];
                break;
            case CPSS_FRAGMENTS_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_FRAGMENTS_E];
                break;
            case CPSS_OVERSIZE_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E];
                break;
            case CPSS_JABBER_PKTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_JABBERS_E];
                break;
            case CPSS_MAC_RCV_ERROR_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_ERRORS_E];
                break;
            case CPSS_BAD_CRC_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
                break;
            case CPSS_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_LATE_COLLISIONS_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_BadFC_RCV_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_GOOD_UC_PKTS_RCV_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_UC_PKTS_E];
                break;
            case CPSS_GOOD_UC_PKTS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_UC_PKTS_E];
                break;
            case CPSS_MULTIPLE_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                /* MIB NOT SUPPORTED IN DEVICE */
                break;
            case CPSS_DEFERRED_PKTS_SENT_E:
                /* COUNTER NOT EXIST IN CG UNIT */
                break;
            case CPSS_PKTS_1024TO1518_OCTETS_E:
                /* INTERLAKEN */
                /*covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
                break;*/
                continue;
            case CPSS_PKTS_1519TOMAX_OCTETS_E:
                /* INTERLAKEN */
                /*covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
                break;*/
                continue;
            default:
                continue;
        }

        /* Assigning the converted CG MIB counter to the regular struct array */
        xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/ 4] = covertedCgMacMibCounterEntry .l[0];

        if ( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
                (cntrName == CPSS_GOOD_OCTETS_RCV_E) ||
                      (cntrName == CPSS_GOOD_OCTETS_SENT_E) )
        {
            xlgMibEntry[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 + 1] = covertedCgMacMibCounterEntry .l[1];
        }
    }

    return GT_OK;
}

/**
* @internal appDemoDxChMacCounterGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets Ethernet MAC counter / MAC Captured counter for a
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number,
*                                      CPU port if getFromCapture is GT_FALSE
* @param[in] cntrName                 - specific counter name
*                                      getFromCapture -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The 10G MAC MIB counters are 64-bit wide.
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChMacCounterGet
*
*/
GT_STATUS appDemoDxChMacCounterGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_U32 regAddr=0;         /* register address */
    GT_U32 baseRegAddr=0;     /* base register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS rc;      /* return code */
    GT_U32 mibBaseAddr; /* mib memory base address */
    GT_U32 mibOffset; /* offset per port */
    GT_U32 *entryPtr;
    GT_U32 xlgMibEntry[PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_DXCH_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = (GT_U32) portNum >> 4;
        portNum %= 16;
    }
    else
    {
        portGroupId = 0;
    }


    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    /* init table */
    rc = cpssDxChPortStatInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    cntrValuePtr->l[0] = 0;
    cntrValuePtr->l[1] = 0;

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum))||(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        ||(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        xlgMibShadowPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[portNum];

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
        {
            if(portNum < 56)
            {
                mibOffset = portNum * 0x400;
            }
            else
            {
                mibOffset = (portNum-56) * 0x400;
                mibOffset += 0x800000;
            }

            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters; */
            mibBaseAddr = 0x12000000;
            baseRegAddr = mibBaseAddr + mibOffset;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            if(portNum < 24)
            {
                mibOffset = portNum * 0x400;
            }
            else
            {
                mibOffset = (portNum-24) * 0x400;
                mibOffset += 0x800000;
            }

            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters; */
            mibBaseAddr = 0x12000000;
            baseRegAddr = mibBaseAddr + mibOffset;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            baseRegAddr = (0x09000000 + portNum * 0x20000);
        }
        else if (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            baseRegAddr = (0x11000000 + portNum * 0x400);
        }
        if((CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
            CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily) && portNum <= 20)
        {
            /* Read CG counters */
            rc = prvCpssDxChPortMacCountersCGMibsConvert_HardCodedRegAddr(devNum, portNum, xlgMibEntry);
        }
        else
        {
            /* read the whole entry */
            rc = prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId,
                baseRegAddr,
                PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum),xlgMibEntry);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        /* add to shadow */
        prvCpssDxChPortSumMibEntry(devNum,xlgMibShadowPtr->mibShadow,xlgMibEntry);
        entryPtr = xlgMibShadowPtr->mibShadow;


        cntrValuePtr->l[0] = entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4];

        if(xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
        {
            entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4] = 0;
        }

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_TRUE ||
            cntrName == CPSS_GOOD_OCTETS_RCV_E || cntrName == CPSS_GOOD_OCTETS_SENT_E)
        {
            cntrValuePtr->l[1] = entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 +1];
            if(xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
            {
                entryPtr[PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName]/4 +1] = 0;
            }
        }

        return GT_OK;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters; */
        baseRegAddr = (0x09000000 + portNum * 0x20000);
    }
    else if ((CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) || (CPSS_PP_FAMILY_DXCH_XCAT_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].macCounters; */
        if (portNum <= 5)
        {
            baseRegAddr = (0x04010000 + portNum * 0x80);
        }
        else if (portNum >= 6 && portNum <= 11)
        {
            baseRegAddr = (0x04810000 + (portNum - 6) * 0x80);
        }
        else if (portNum >= 12 && portNum <= 17)
        {
            baseRegAddr = (0x05010000 + (portNum - 12) * 0x80);
        }
        else if (portNum >= 18 && portNum <= 23)
        {
            baseRegAddr = (0x05810000 + (portNum - 18) * 0x80);
        }
        else
        {
            baseRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    regAddr = baseRegAddr + PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrName];

        if ( prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                regAddr, &(cntrValuePtr->l[0])) != GT_OK)
            return GT_HW_ERROR;


    switch (cntrName)
    {
        case CPSS_GOOD_OCTETS_RCV_E:
        case CPSS_GOOD_OCTETS_SENT_E:
            /* this counter has 64 bits */
            regAddr = regAddr + 4;
            if ( prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &(cntrValuePtr->l[1])) != GT_OK)
                return GT_HW_ERROR;
            break;

        default:
            break;
    }

    return GT_OK;
}

/**
* @internal appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChCfgIngressDropCntrGet
*
*/
GT_STATUS appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
)
{
    GT_U32      regAddr;     /* register address */

    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = 0x0D000040;
            /*PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).ingrDropCntr.ingrDropCntr; */
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x14000040;
            /*PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).ingrDropCntr.ingrDropCntr; */
    }
    else
    {
        regAddr = 0x0B000040;
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.ingressDropCntrReg; */
    }

    return prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       regAddr, 0, 32,
                                                       counterPtr, NULL);
}


/**
* @internal mirrorToAnalyzerForwardingModeGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorToAnalyzerForwardingModeGet
*
*/
static GT_STATUS mirrorToAnalyzerForwardingModeGet_HardCodedRegAddr
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      regData;      /* register data */
    GT_STATUS   rc;           /* return status */

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = 0x0D00B000; /* PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;*/
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x1400B040; /* PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig; */
    }
    else
    {
        regAddr = 0x0B00B040; /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig; */
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 2, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(regData)
        {
            case 0:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E ;
                break;
            case 1:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E ;
                break;
            case 2:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E ;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }
    else
    {
        switch(regData)
        {
            case 0:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E ;
                break;
            case 3:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    return rc;
}

/**
* @internal appDemoDxChMirrorRxPortSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorRxPortSet
*
*/
GT_STATUS appDemoDxChMirrorRxPortSet_HardCodedRegAddr
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
)
{
    GT_U32      data;        /* data from Ports VLAN and QoS table entry */
    GT_U32      offset;      /* offset in VLAN and QoS table entry */
    GT_STATUS   rc = GT_OK;  /* function call return value */
    GT_U32      portGroupId; /* the port group Id - support multi-port-groups device */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode; /* forwarding mode */
    GT_U32      entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    GT_U32      address;    /* address to write to */
    GT_U32      entrySize;  /* table entry size in words */
    GT_U32      numBitsPerEntry;/* number of bits per entry in the LINE */
    GT_U32      numEntriesPerLine;/* number of entries per line */

    portGroupId = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Set index to Analyzer interface for the ePort */
        if(enable == GT_TRUE)
        {
            data = index + 1;
        }
        else /* No mirroring for the ePort */
        {
            data = 0;
        }

         /* check if the table entry is actually implemented as
           'several entries' in single 'line' */

        /*multiEntriesInLineIndexAndGlobalBitConvert(devNum,CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
                    INOUT &index,INOUT &offset,NULL,NULL); */
        offset = 4;

        numBitsPerEntry = 7;
        numEntriesPerLine = 4;

        offset += (mirrPort % numEntriesPerLine) * numBitsPerEntry;
        index = mirrPort;
        index  /= numEntriesPerLine;

        /* tablePtr->baseAddress + entryIndex * tablePtr->step */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            address = 0x09700000 + index*4;
        }
        else
        {
            address = 0x0d700000 + index*4;
        }

        entrySize = 1;

        /* read whole entry */
        rc = prvCpssDrvHwPpPortGroupReadRam(devNum, portGroupId, address, entrySize, entryMemoBufArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* update field */
        U32_SET_FIELD_MAC(entryMemoBufArr[0], offset, 3 /* length */, data);

        /* write whole entry */
        rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, address, entrySize, entryMemoBufArr);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Get Analyzer forwarding mode */
        rc = mirrorToAnalyzerForwardingModeGet_HardCodedRegAddr(devNum, &mode);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
        {
            return GT_NOT_SUPPORTED;
        }
        offset = 7;
    }
    else
    {
        offset = 23;
    }

    /* For xCat and above hop-by-hop forwarding mode and other DxCh devices */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        data = BOOL2BIT_MAC(enable);

        /* configure the Ports VLAN and QoS configuration entry,
           enable MirrorToIngressAnalyzerPort field */

        /* In xCat and above devices the data is updated only when the last */
        /* word in the entry was written. */

        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            address = 0x16001000 + 0*0x10; /* tablePtr->baseAddress + entryIndex * tablePtr->step */
        }
        else if(CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily || CPSS_PP_FAMILY_DXCH_XCAT_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            address = 0xC001000 + 0*0x10; /* tablePtr->baseAddress + entryIndex * tablePtr->step */
        }
        else
        {
            address = 0x1001000 + mirrPort*0x10; /* tablePtr->baseAddress + entryIndex * tablePtr->step */
        }

        entrySize = 3;

        /* read whole entry */
        rc = prvCpssDrvHwPpPortGroupReadRam(devNum, portGroupId, address, entrySize, entryMemoBufArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* update field */
        U32_SET_FIELD_MAC(entryMemoBufArr[0], offset, 1 /* length */, data);

        /* write whole entry */
        rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, address, entrySize, entryMemoBufArr);
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return rc;
}

/**
* @internal setOwnDevice_HardCodedRegAddr function
* @endinternal
*
* @brief   Sets the device Device_ID within a Prestera chipset.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
GT_STATUS setOwnDevice_HardCodedRegAddr
(
    IN  GT_U8           devNum
)
{
    GT_STATUS rc;
    GT_U32    portGroupId;    /* port group id    */

    if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) &&
        (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum) ) )
    {
        /* set on all port groups in hemisphere 0 the hwDevNum */
        for (portGroupId=0; portGroupId < PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS;
            portGroupId++)
        {
            /* skip non active port groups */
            if((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1 << portGroupId)) == 0)
            {
                continue;
            }

            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,0x58,4,5,0x10);
            if (rc != GT_OK)
            {
                return rc;
            }

        }

        /* set on all port groups in hemisphere 1 the hwDevNum+1 */
        for (portGroupId=PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS;
            portGroupId <PRV_CPSS_LION2_PORT_GROUPS_NUM_CNS; portGroupId++)
        {
            /* skip non active port groups */
            if((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1 << portGroupId)) == 0)
            {
                continue;
            }

            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,0x58,4,5,0x10+1);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
    }
    else
    {
        /* set 'own device' to be 0x10 */
        rc = prvCpssHwPpSetRegField(devNum,0x58,4,5,0x10);
        if(rc != GT_OK)
            return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorAnalyzerInterfaceSet
*
*/
GT_STATUS appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;             /* register address */
    GT_U32      regData = 0;         /* register data */
    GT_U32      hwDev, hwPort;
    GT_U32      dataLength;

    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        hwDev = interfacePtr->interface.devPort.hwDevNum;
        hwPort = interfacePtr->interface.devPort.portNum;

        regData = (hwDev | hwPort << 10);

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            dataLength = 25;
        }
        else
        {
            dataLength = 23;
        }

        /* getting register address */
        /* rxDmaNum (0-72) represents Port */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            regAddr = 0x0900B200 + index * 4;
        }
        else
        {
            regAddr = 0x0D00B200 + index * 4;
        }

        /* PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                 mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index]; */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, dataLength, regData);
    }
    else
    {

        /* set 1 bit for MonitorType (value = 0 incase of portType)
           5 bits for devNum and 13 bits for portNum */
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,0,2,0);
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,2,5,interfacePtr->interface.devPort.hwDevNum);
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,7,13,interfacePtr->interface.devPort.portNum);

        /* getting register address */
        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            regAddr = 0x1400B020 + index * 4;
        }
        else
        {
            regAddr = 0x0B00B020 + index * 4;
        }

        /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        bufferMng.eqBlkCfgRegs.mirrorInterfaceParameterReg[index];*/

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, regData);
    }

    return rc;
}

/**
* @internal appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring from all engines except
*         port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
*
*/
GT_STATUS appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data */

    /* getting register address */
    if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
        CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x0900B000;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
         regAddr = 0x0D00B000; /*PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig; */
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x1400B040; /* PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig; */
    }
    else
    {
        regAddr = 0x0B00B040; /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig; */
    }

    /* 0 is used for no mirroring */
    regData = (enable == GT_TRUE) ? (index + 1) : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, 2, 3, regData);
}

/**
* @internal appDemoDxChIngressCscdPortSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set ingress port as cascaded/non-cascade .
*         (effect all traffic on ingress pipe only)
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChIngressCscdPortSet
*
*/
GT_STATUS appDemoDxChIngressCscdPortSet_HardCodedRegAddr
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_U32          value;           /* value of field */
    GT_U32          fieldOffset = 0; /* The start bit number in the register */
    GT_U32          regAddr = 0;     /* address of register */
    GT_U32          portGroupId = 0; /*the port group Id - support multi port groups device */

    if(portType == CPSS_CSCD_PORT_NETWORK_E)
    {
        value = 0;
   }
    else
    {
        value = 1;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /*PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum]; */
        if(CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            if (portRxDmaNum < 72)
                regAddr = 0x19000804 + (portRxDmaNum%12)*0x4 + ((portRxDmaNum-(portRxDmaNum/36)*36)/12)*0x1000000 + (portRxDmaNum/36)*0x80000000;
            else if (portRxDmaNum == 72)
                regAddr = 0x1b000834;
            else if (portRxDmaNum == 73)
                regAddr = 0x9b000834;
            else if (portRxDmaNum == 74)
                regAddr = 0x19000834;
            else if (portRxDmaNum == 75)
                regAddr = 0x1a000834;
            else if (portRxDmaNum == 76)
                regAddr = 0x99000834;
            else if (portRxDmaNum == 77)
                regAddr = 0x9a000834;
            else
                regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        else if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            if (portRxDmaNum < 24)
                regAddr = 0x19000804 + (portRxDmaNum%12)*0x04 + (portRxDmaNum/12)*0x1000000;
            else if (portRxDmaNum >= 24 && portRxDmaNum < 48)
                regAddr = 0x1b000804 + (portRxDmaNum%24)*0x04;
            else if (portRxDmaNum >=48 && portRxDmaNum <72)
                regAddr = 0x99000804 + (portRxDmaNum%24)*0x04;
            else if (portRxDmaNum == 72)
                regAddr = 0x19000864;
            else if (portRxDmaNum == 74)
                regAddr = 0x99000864;
            else if (portRxDmaNum == 75)
                regAddr = 0x1a000864;
            else if (portRxDmaNum == 76)
                regAddr = 0x1b000864;
            else if (portRxDmaNum == 77)
                regAddr = 0x19000860;
            else
                regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        else
        {
            /* each RXDMA port hold it's own registers , so bit index is not parameter of port number */
            fieldOffset = 3;

            regAddr = 0x17000804 + portRxDmaNum*0x4; /* where n (0-72) represents Port */
                /*PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum]; */
        }
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily || CPSS_PP_FAMILY_DXCH_XCAT_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        fieldOffset = (portNum == CPSS_CPU_PORT_NUM_CNS) ? 31 : portNum;
        regAddr = 0x0F000004;
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        fieldOffset = (portNum == CPSS_CPU_PORT_NUM_CNS) ? 31 : portNum;
        regAddr = 0x1C000004;
    }
    else
    {
        fieldOffset = (portNum == CPSS_CPU_PORT_NUM_CNS) ? 15 : portNum;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            regAddr = 0x0F000064;
            /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctCascadingPortReg; */
        }
        else
        {
            regAddr = 0x0F000020;
            /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctCascadingPortReg; */
        }
    }

    /* Enable/disable DSA tag recognition on rxDma of ingress port */
    return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                               regAddr, fieldOffset, 1, value);
}

/**
* @internal waitForSendToEnd_HardCodedRegAddr function
* @endinternal
*
* @brief   The function waits until PP sent the packet.
*         DMA procedure may take a long time for Jumbo packets.
* @param[in] devNum                   - device number
*                                      txDesc     - (pointer to) The descriptor used for that packet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note copied from dxChNetIfSdmaTxPacketSend()
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
static GT_STATUS  waitForSendToEnd_HardCodedRegAddr
(
    GT_U8                   devNum,
    PRV_CPSS_TX_DESC_STC    *txDescPtr
)
{
    GT_U32  numOfBufs = 1;
    GT_U32  numOfAddedBuffs = 0;

#ifndef ASIC_SIMULATION
    GT_U32 loopIndex = (numOfBufs + numOfAddedBuffs) * 104000;
#else
    GT_U32 loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
    GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */

    /* allow another task to process it if ready , without 1 millisecond penalty */
    /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
    sleepTime = 0;

    tryMore_lbl:
#endif /*ASIC_SIMULATION*/

    /* Wait until PP sent the packet. Or HW error if while block
               run more then loopIndex times */
    while (loopIndex && (TX_DESC_GET_OWN_BIT_MAC(devNum,txDescPtr) == TX_DESC_DMA_OWN))
    {
#ifdef ASIC_SIMULATION
        /* do some sleep allow the simulation process the packet */
        cpssOsTimerWkAfter(sleepTime);
#endif /*ASIC_SIMULATION*/
        loopIndex--;
    }

    if(loopIndex == 0)
    {
#ifdef ASIC_SIMULATION
        if(sleepTime == 0)/* the TX was not completed ? we need to allow more retries with 'sleep (1)'*/
        {
            loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
            sleepTime = 1;
            goto tryMore_lbl;
        }
#endif /*ASIC_SIMULATION*/
        return GT_HW_ERROR;
    }

    return GT_OK;
}

/**
* @internal buildTxSdmaDescriptor function
* @endinternal
*
* @brief   The function build the Tx descriptor and then trigger the transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS buildTxSdmaDescriptor(
    IN PRV_CPSS_TX_DESC_STC    *txDescPtr,
    IN GT_U8                   *buffArr,
    IN GT_U32                   length
)
{
    GT_STATUS rc = GT_OK;
    GT_UINTPTR  physicalAddrOfBuffPtr;         /* Holds the real buffer pointer.       */

    /* Set the first descriptor parameters. */
    TX_DESC_RESET_MAC(txDescPtr);

    txDescPtr->word1 = (1 << 12);/* recalc CRC */

    /* Set bit for first buffer of a frame for Tx descriptor */
    TX_DESC_SET_FIRST_BIT_MAC(txDescPtr,1);
    /* single descriptor --> send the pull packet */
    TX_DESC_SET_BYTE_CNT_MAC(txDescPtr,length);
    /* Set bit for last buffer of a frame for Tx descriptor */
    TX_DESC_SET_LAST_BIT_MAC(txDescPtr,1);

    /* update the packet header to the first descriptor */
    rc = cpssOsVirt2Phy((GT_UINTPTR)buffArr,&physicalAddrOfBuffPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
    if (0 != (physicalAddrOfBuffPtr & 0xffffffff00000000L))
    {
        return GT_OUT_OF_RANGE;
    }
    #endif

    txDescPtr->buffPointer = CPSS_32BIT_LE((GT_U32)physicalAddrOfBuffPtr);

    /* Set the descriptor own bit to start transmitting.  */
    TX_DESC_SET_OWN_BIT_MAC(txDescPtr,TX_DESC_DMA_OWN);

    txDescPtr->word1  = CPSS_32BIT_LE(txDescPtr->word1);
    txDescPtr->word2  = CPSS_32BIT_LE(txDescPtr->word2);

    txDescPtr->nextDescPointer = 0;/* single descriptor without 'next' */

    return rc;

}
/**
* @internal setTxSdmaRegConfig_HardCodedRegAddr function
* @endinternal
*
* @brief   Set the needed values for SDMA registers to enable Tx activity.
*
* @param[in] devNum                   - The Pp device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
static GT_STATUS setTxSdmaRegConfig_HardCodedRegAddr
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      portGroupId = 0;

    /********* Since working in SP the configure transmit queue WRR value to 0 ************/
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                              0x00002708,/*addrPtr->sdmaRegs.txQWrrPrioConfig[queue]*/
                                              0);
    if(rc != GT_OK)
        return rc;

    /********* Tx SDMA Token-Bucket Queue<n> Counter ************/
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                              0x00002700,/*addrPtr->sdmaRegs.txSdmaTokenBucketQueueCnt[queue]*/
                                              0);
    if(rc != GT_OK)
        return rc;

    /********** Tx SDMA Token Bucket Queue<n> Configuration ***********/
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                              0x00002704,/*addrPtr->sdmaRegs.txSdmaTokenBucketQueueConfig[queue]*/
                                              0xfffffcff);
    if(rc != GT_OK)
        return rc;

    /*********************/
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                              0x00002874,/*addrPtr->sdmaRegs.txSdmaWrrTokenParameters */
                                              0xffffffc1);
    if(rc != GT_OK)
        return rc;

    /*********** Set all queues to Fix priority **********/
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                              0x00002870,/*addrPtr->sdmaRegs.txQFixedPrioConfig */
                                              0xFF);
    if(rc != GT_OK)
        return rc;

    /*** temp settings ****/
    /* Lion RM#2701: SDMA activation */
    /* the code must be before calling phase1Part4Init(...) because attempt
       to access register 0x2800 will cause the PEX to hang */
    rc = prvCpssDrvHwPpSetRegField(devNum,0x58,20,1,1);

    return rc;
}

/**
* @internal freesSdmaMemoryAllocation function
* @endinternal
*
* @brief   This function frees sdma memory allocation
*
* @param[in] buffArrMAllocPtr         - pointer to array of alocated memory
* @param[in] txDescAllocPtr           - pointer to descriptor
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_VOID  freesSdmaMemoryAllocation
(
    IN GT_U8 *buffArrMAllocPtr ,
    IN PRV_CPSS_TX_DESC_STC *txDescAllocPtr
)
{
    if(buffArrMAllocPtr)
    {
        cpssOsCacheDmaFree(buffArrMAllocPtr);
    }

    if(txDescAllocPtr)
    {
        cpssOsCacheDmaFree(txDescAllocPtr);
    }

    return;
}

/**
* @internal printPacket function
* @endinternal
*
* @brief   print packet
*
* @param[in] bufferPtr                - pointer to buffer of packet data.
* @param[in] length                   -  of the packet data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS printPacket
(
    IN GT_U8           *bufferPtr,
    IN GT_U32           length
)
{
    GT_U32  iter = 0;

    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);


    cpssOsPrintf("\nSending brodcast packet with DSA tag\n");

    for(iter = 0; iter < length; iter++)
    {
        if((iter & 0x0F) == 0)
        {
            cpssOsPrintf("0x%4.4x :", iter);
        }

        cpssOsPrintf(" %2.2x", bufferPtr[iter]);

        if((iter & 0x0F) == 0x0F)
        {
            cpssOsPrintf("\n");
        }
    }

    cpssOsPrintf("\n\n");

    return GT_OK;
}

/**
* @internal appDemoDxChPortEgressCntrsGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChPortEgressCntrsGet
*
*/
GT_STATUS appDemoDxChPortEgressCntrsGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
{
    GT_U32 regAddr;         /* register address */

    CPSS_NULL_PTR_CHECK_MAC(egrCntrPtr);

    /* read Bridge Egress Filtered Packet Count Register */
    if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
        CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x55093240 + (cntrSetNum*0x4);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.peripheralAccess.egressMibCounterSet.bridgeEgressFilteredPacketCounter[cntrSetNum];*/
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = 0x3C093240+ (cntrSetNum*0x4);
            /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setBridgeEgrFilteredPktCntr[cntrSetNum];*/
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x02B40150 + (cntrSetNum*0x20);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].brgEgrFiltered; */
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = 0x01B40150 + (cntrSetNum*0x20);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].brgEgrFiltered; */
    }
    else if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x10093240 + (cntrSetNum*0x4);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].brgEgrFiltered; */
    }
    else
    {
        regAddr = 0xA093240 + (cntrSetNum*0x4);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.peripheralAccess.egressMibCounterSet.bridgeEgressFilteredPacketCounter[cntrSetNum];*/
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,0,32, &egrCntrPtr->brgEgrFilterDisc) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    /* read Transmit Queue Filtered Packet Count Register */
    if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
        CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x55093250 + (cntrSetNum*0x4);
            /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[cntrSetNum];*/
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = 0x3C093250 + (cntrSetNum*0x4);
            /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[cntrSetNum];*/
    }
    else if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x02B40154 + (cntrSetNum*0x20);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].txQFiltered; */
    }
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = 0x01B40154 + (cntrSetNum*0x20);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].txQFiltered; */
    }
   else if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = 0x10093250+ (cntrSetNum*0x4);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].brgEgrFiltered; */
    }
    else
    {
        regAddr = 0xA093250 + (cntrSetNum*0x4);
            /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.peripheralAccess.egressMibCounterSet.tailDroppedPacketCounter[cntrSetNum]; */
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,0,32, &egrCntrPtr->txqFilterDisc) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E) && (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) != GT_TRUE))
    {
        /* read  egress forwarding restriction dropped Packets Count Register
           Supported from ch2 devices.*/
        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            regAddr = 0x02B4015C + (cntrSetNum*0x20);
                /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].egrFrwDropPkts; */
        }
        else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            regAddr = 0x01B4015C + (cntrSetNum*0x20);
                /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQCountSet[cntrSetNum].egrFrwDropPkts; */
        }
       else if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            regAddr = 0x10093270+ (cntrSetNum*0x4);
             /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.peripheralAccess.egressMibCounterSet.
                            egressForwardingRestrictionDroppedPacketsCounter[cntrSetNum]; */

        }
        else
        {
            regAddr = 0xA093270 + (cntrSetNum*0x4);
                /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.peripheralAccess.egressMibCounterSet.
                            egressForwardingRestrictionDroppedPacketsCounter[cntrSetNum]; */
        }

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,0,32, &egrCntrPtr->egrFrwDropFrames) != GT_OK)
        {
            return GT_HW_ERROR;
        }
    }
    else
    {
        egrCntrPtr->egrFrwDropFrames = 0;
    }

    /* read Transmit Queue Filtered Packet Count Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setMcFIFO3_0DroppedPktsCntr[cntrSetNum];*/
        if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
            CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            regAddr = 0x55093280 + (cntrSetNum*0x4);
        }
        else
        {
            regAddr = 0x3C093280 + (cntrSetNum*0x4);
        }
             /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setMcFIFO3_0DroppedPktsCntr[cntrSetNum];*/

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,0,32, &egrCntrPtr->mcFifo3_0DropPkts) != GT_OK)
        {
            return GT_HW_ERROR;
        }

        /*PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setMcFIFO7_4DroppedPktsCntr[cntrSetNum];*/
        if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
            CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            regAddr = 0x55093290 + (cntrSetNum*0x4);
        }
        else
        {
            regAddr = 0x3C093290 + (cntrSetNum*0x4);
        }

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,0,32, &egrCntrPtr->mcFifo7_4DropPkts) != GT_OK)
        {
            return GT_HW_ERROR;
        }

    }
    else
    {
        egrCntrPtr->mcFifo3_0DropPkts = 0;
        egrCntrPtr->mcFifo7_4DropPkts = 0;
    }

    #if 0
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
                        devNum,
                        PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E))
     {
        egressCntrShadowPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E.
                      egressCntrShadow[cntrSetNum]);

        egrCntrPtr->brgEgrFilterDisc -= egressCntrShadowPtr->brgEgrFilterDisc;
        egrCntrPtr->txqFilterDisc    -= egressCntrShadowPtr->txqFilterDisc;
        egrCntrPtr->egrFrwDropFrames -= egressCntrShadowPtr->egrFrwDropFrames;
        egrCntrPtr->mcFifo3_0DropPkts -= egressCntrShadowPtr->mcFifo3_0DropPkts;
        egrCntrPtr->mcFifo7_4DropPkts -= egressCntrShadowPtr->mcFifo7_4DropPkts;

        egressCntrShadowPtr->brgEgrFilterDisc += egrCntrPtr->brgEgrFilterDisc;
        egressCntrShadowPtr->txqFilterDisc    += egrCntrPtr->txqFilterDisc;
        egressCntrShadowPtr->egrFrwDropFrames += egrCntrPtr->egrFrwDropFrames;
        egressCntrShadowPtr->mcFifo3_0DropPkts += egrCntrPtr->mcFifo3_0DropPkts;
        egressCntrShadowPtr->mcFifo7_4DropPkts += egrCntrPtr->mcFifo7_4DropPkts;

     }
    #endif
    return GT_OK;
}

/**
* @internal appDemoDxChSdmaTxPacketSend_HardCodedRegAddr function
* @endinternal
*
* @brief   This function sends a single packet.
*         The packet is sent through interface port type to port given by
*         dstPortNum with VLAN vid.
* @param[in] isPacketWithVlanTag      - indication that the packetDataPtr[12..15] hold vlan tag.
* @param[in] vid                      - VLAN ID. (used when isPacketWithVlanTag ==GT_FALSE,
*                                      otherwise taken from packetDataPtr[14..15])
* @param[in] packetPtr                - pointer to the packet data and length in bytes.
* @param[in] dstPortNum               - Destination port number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
GT_STATUS appDemoDxChSdmaTxPacketSend_HardCodedRegAddr
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPacketWithVlanTag,
    IN GT_U8    vid,
    IN TGF_PACKET_PAYLOAD_STC  *packetPtr,
    IN GT_U32   dstPortNum
)
{
    GT_U8 *buffArr,*origBuffArrMAllocPtr = NULL;
    PRV_CPSS_TX_DESC_STC    *txDescPtr,*origTxDescAllocPtr = NULL;/* pointer to txDescriptor*/
    GT_U32 buffSize4alloc;
    CPSS_DXCH_NET_DSA_PARAMS_STC   dsaInfo;
    GT_U32  dsaTagNumOfBytes;   /* DSA tag length */
    GT_U32  dataOffset;/* the offset from start of packetPtr->dataLength till end of vlan tag if exists,
                        if not thet till end of macSa */
    GT_U32  totalLength;/* length of packet including the DSA that may replace the vlan tag */
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(packetPtr);

    /***************************************************/
    /* set SDMA registers to enable Tx activity */
    setTxSdmaRegConfig_HardCodedRegAddr(devNum);

    /***************************************************/
    /* allocate descriptor */
    origTxDescAllocPtr = cpssOsCacheDmaMalloc(16/*length*/ + TX_DESC_DMA_ALIGN_BYTES_CNS/*alignment*/);
    txDescPtr = origTxDescAllocPtr;
    if (txDescPtr == NULL)
    {
        rc = GT_NO_RESOURCE;
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }

    /* we use 16 bytes alignment */
    if((((GT_UINTPTR)txDescPtr) % TX_DESC_DMA_ALIGN_BYTES_CNS) != 0)
    {
        txDescPtr = (PRV_CPSS_TX_DESC_STC*)(((GT_UINTPTR)txDescPtr) +
                           (TX_DESC_DMA_ALIGN_BYTES_CNS - (((GT_UINTPTR)txDescPtr) % TX_DESC_DMA_ALIGN_BYTES_CNS)));
    }
    /***************************************************/

    /***************************************************/
    /* allocate buffer for packet */

    /* packet length is 128 bytes, and it must be 128-byte aligned */
    buffSize4alloc = (packetPtr->dataLength + PACKET_DMA_ALIGN_BYTES_CNS);

    /* save original allocation pointer ... needed when calling cpssOsFree() */
    origBuffArrMAllocPtr = cpssOsCacheDmaMalloc(buffSize4alloc);

    buffArr = origBuffArrMAllocPtr;
    if (buffArr == NULL)
    {
        rc = GT_NO_RESOURCE;
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }
    /* we use 128 bytes alignment */
    if((((GT_UINTPTR)buffArr) % PACKET_DMA_ALIGN_BYTES_CNS) != 0)
    {
        buffArr = (GT_U8*)(((GT_UINTPTR)buffArr) +
                           (PACKET_DMA_ALIGN_BYTES_CNS - (((GT_UINTPTR)buffArr) % PACKET_DMA_ALIGN_BYTES_CNS)));
    }

    /***************************************************/
    /* copy packet from caller into buffer allocated in the SDMA */

    /* break to 2 parts : macDa,Sa and rest of the packet */
    cpssOsMemCpy(&buffArr[0],packetPtr->dataPtr,MAC_ADDR_LENGTH_CNS);/* 12 bytes of macDa,Sa */

    dsaTagNumOfBytes = 8;/* number of bytes in the DSA */

    if(isPacketWithVlanTag == GT_TRUE)
    {
        /* skip the 4 bytes of the vlan tag , as it is embedded inside the DSA */
        /* copy the reset of the packet without the vlan tag */
        dataOffset = MAC_ADDR_LENGTH_CNS + 4;/*16*/
    }
    else
    {
        dataOffset = MAC_ADDR_LENGTH_CNS;/*12*/
    }

    /* copy the reset of the packet  */
    cpssOsMemCpy(&buffArr[MAC_ADDR_LENGTH_CNS + dsaTagNumOfBytes] ,
        &packetPtr->dataPtr[dataOffset],
        (packetPtr->dataLength-dataOffset));

    totalLength = (packetPtr->dataLength - dataOffset) + /* length of 'rest of packet' */
                  (MAC_ADDR_LENGTH_CNS + dsaTagNumOfBytes);/*length of DSA + macDa,Sa */

    /***************************************************/
    /* Build Extended FROM_CPU DSA tag which is used also by legacy devices; 2 words */

    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT;
    if(isPacketWithVlanTag == GT_FALSE)
    {
        /* the buffer hold no vlan tag info */
        dsaInfo.commonParams.vid = vid;
    }
    else
    {
        /* take the 12 bits from the buffer */
        dsaInfo.commonParams.vid = (packetPtr->dataPtr[14] & 0x0F) << 8 |
                                    packetPtr->dataPtr[15];
        dsaInfo.commonParams.vpt = (packetPtr->dataPtr[14] >> 5) & 0x7;
        dsaInfo.commonParams.cfiBit = (packetPtr->dataPtr[14] >> 4) & 1;
    }
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

    dsaInfo.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
    dsaInfo.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum = dstPortNum;
    dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = 0x10;

    dsaInfo.dsaInfo.fromCpu.srcHwDev = 0x1f;/* different than 'dstInterface.devPort.hwDevNum' */


    /***************************************************/
    /* put the DSA info into the buffer after 12 bytes of macDa,Sa*/
    rc = cpssDxChNetIfDsaTagBuild(devNum, &dsaInfo, &buffArr[MAC_ADDR_LENGTH_CNS]);
    if(rc != GT_OK)
    {
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }

    printPacket(buffArr,totalLength);

    /***************************************************/
    /* fill the descriptor with proper info */
    rc = buildTxSdmaDescriptor(txDescPtr,buffArr,totalLength);
    if(rc != GT_OK)
    {
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }

    /***************************************************/
    /* 1. put the descriptor in the queue */
    /* 2. trigger the queue */
    rc = sendPacketByTxDescriptor_HardCodedRegAddr(devNum,txDescPtr);
    if(rc != GT_OK)
    {
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }

    /***************************************************/
    /* wait for the send of the packet to end (like synchronic send) */
    rc = waitForSendToEnd_HardCodedRegAddr(devNum,txDescPtr);
    if(rc != GT_OK)
    {
        freesSdmaMemoryAllocation(origBuffArrMAllocPtr,origTxDescAllocPtr);
        return rc;
    }

    return rc;
}

/**
* @internal appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChPortForceLinkPassEnableSet
*
*/
GT_STATUS appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* MAC unit of port */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

     if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = (GT_U32) portNum >> 4;
        portNum %= 16;
    }
    else
    {
        portGroupId = 0;
    }

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(state);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        return GT_INIT_ERROR;

    /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].autoNegCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = 0x0A80000C + (portNum & 0xf) * 0x400;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (portNum < 24)
        {
            regAddr =  0x0A80000C + (portNum * 0x400);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr =  0x0A80000C + (portNum * 0x400);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (portNum < 24)
        {
            regAddr = 0x1200000C + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (portNum < 56) /* 0-55 network ports */
        {
            regAddr = 0x1000000C + (portNum * 0x1000);
        }
        else if (portNum >=56 && portNum <=71) /* 56-71 pabric ports */
        {
            regAddr = 0x1000000C + 0x200000 + ((portNum-56) * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (portNum < 33) /* 0-32 network ports */
        {
            regAddr = 0x1000000C + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (portNum < 72)
        {
            regAddr = 0x1000000C + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
        }
        else if (portNum == 72)
        {
            regAddr = 0x1002400c;
        }
        else if (portNum == 73)
        {
            regAddr = 0x1042400c;
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else
    {
        regAddr = 0x0880000C + (portNum * 0x1000);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_XG_E].macCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = 0x08800000 + (portNum & 0xf) * 0x400;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (portNum > 23)
        {
            regAddr = 0x120C0000 + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (portNum >= 48 &&  portNum < 56) /* 48-55 represents Network XLG Mac */
        {
            regAddr = 0x100C0000  + (portNum * 0x1000);
        }
        else if (portNum >= 56 && portNum <=71) /* (0-15) represents Fabric XLG Mac */
        {
            regAddr = 0x100C0000  + 0x200000 + ((portNum-56) * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (portNum < 32) /* 0-31 represents Network XLG Mac */
        {
            regAddr = 0x100C0000  + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (portNum < 72)
        {
            regAddr = 0x100C0000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
        }
        else if (portNum == 72)
        {
            regAddr = 0x100E4000;
        }
        else if (portNum == 73)
        {
            regAddr = 0x104E4000;
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = 0x088C0000 + (portNum * 0x1000);
    }
    else
    {
        regAddr = 0x08800000 + (portNum * 0x400);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 3;
    }

    /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_XLG_E].macCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (portNum > 23)
        {
            regAddr = 0x120C0000 + 0x0 + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (portNum >= 48 &&  portNum < 56) /* 48-55 represents Network XLG Mac */
        {
            regAddr = 0x100C0000  + (portNum * 0x1000);
        }
        else if (portNum >= 56 && portNum <=71) /* (0-15) represents Fabric XLG Mac */
        {
            regAddr = 0x100C0000  + 0x200000 + ((portNum-56) * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (portNum < 32) /* 0-31 represents Network XLG Mac */
        {
            regAddr = 0x100C0000  + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (portNum < 72)
        {
            regAddr = 0x100C0000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
        }
        else if (portNum == 72)
        {
            regAddr = 0x100E4000;
        }
        else if (portNum == 73)
        {
            regAddr = 0x104E4000;
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = 0x088C0000 + (portNum * 0x1000);
    }
    else
    {
        regAddr = 0x08800000 + 0x0 + (portNum * 0x400);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 3;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        regAddr = 0x10340000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 27;
        }
    }

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;
        }

        if(regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                    regDataArray[portMacType].regAddr,
                                                    regDataArray[portMacType].fieldOffset,
                                                    regDataArray[portMacType].fieldLength,
                                                    regDataArray[portMacType].fieldData) != GT_OK)
            {
                return GT_HW_ERROR;
            }
        }
    }

    return GT_OK;
}

/**
* @internal portPcsLoopbackEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set the PCS Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortPcsLoopbackEnableSet
*
*/
static GT_STATUS portPcsLoopbackEnableSet_HardCodedRegAddr
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_U32 regAddr;     /* register address                    */
    GT_U32 value;       /* value to write into the register    */
    GT_U32          portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* MAC unit used by port */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        return GT_INIT_ERROR;

    value = BOOL2BIT_MAC(enable);

    /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl1; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = 0x0A800004 + 0x0 + (portNum & 0xf) * 0x400;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = 0x0A80FC04 + 0x0 + (portNum * 0x400);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        regAddr = 0x0A800004 + 0x0 + (portNum * 0x400);
    }
    else
    {
        regAddr = 0x08800004 + 0x0 + (portNum * 0x400);
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 6;
    }

    /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_XG_E].macCtrl1; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = 0x08800004 + 0x0 + (portNum & 0xf) * 0x400;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = 0x0A80FC04 + 0x0 + (portNum * 0x400);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* regAddr = 0x08806204 + 0x0 + (portNum * 0x400); */
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else
    {
        regAddr = 0x08800004 + 0x0 + (portNum * 0x400);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 13;

        /*
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 1;
        }
        */
    }

    /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_XLG_E].macCtrl1; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = 0x0A80FC04 + 0x0 + (portNum * 0x400);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else
    {
        regAddr = 0x08800004 + 0x0 + (portNum * 0x400);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 13;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* put port link down before changes */
        GT_U32 regAddr2;

        regAddr2 = 0x0A800000 + 0x0 + (portNum * 0x400);
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr2, 0, 1, 0) != GT_OK)
        {
            return GT_HW_ERROR;
        }

    }

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;
        }

        if(regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                                    regDataArray[portMacType].regAddr,
                                                    regDataArray[portMacType].fieldOffset,
                                                    regDataArray[portMacType].fieldLength,
                                                    regDataArray[portMacType].fieldData) != GT_OK)
            {
                return GT_HW_ERROR;
            }
        }
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* put port link up after changes */
        GT_U32 regAddr2;

        regAddr2 = 0x0A800000 + 0x0 + (portNum * 0x400);
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr2, 0, 1, 1) != GT_OK)
        {
            return GT_HW_ERROR;
        }

    }

    return GT_OK;
}

/**
* @internal portPreLion2InternalLoopbackEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortPreLion2InternalLoopbackEnableSet
*
*/
static GT_STATUS portPreLion2InternalLoopbackEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 regAddr;               /* register address                    */
    GT_U32 value;                 /* value to write into the register    */
    GT_U32 fieldOffset;           /* bit field offset */
    GT_U32 fieldLength;           /* number of bits to be written to register */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* MAC unit used by port */
    GT_STATUS rc;       /* return code */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);


    if((rc = portPcsLoopbackEnableSet_HardCodedRegAddr(devNum,portNum,enable)) != GT_OK)
        return rc;

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if((portMacNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;

        if(portMacType == PRV_CPSS_PORT_GE_E)
        /* Set GMII loopback mode */
        {
            fieldOffset = 5;
            fieldLength = 1;
            value = (enable == GT_TRUE) ? 1 : 0;

            /* PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].
                            macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl1; */
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {
                regAddr = 0x0A800004 + 0x0 + (portNum & 0xf) * 0x400;
            }
            else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
               regAddr = 0x0A800004 + 0x0 + (portNum * 0x400);
            }
            else
            {
                regAddr = 0x08800004 + 0x0 + (portNum * 0x400);
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                /* put port link down before changes */
                GT_U32 regAddr2;

                regAddr2 = 0x0A800000 + 0x0 + (portNum * 0x400);
                if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr2, 0, 1, 0) != GT_OK)
                {
                    return GT_HW_ERROR;
                }

            }

            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,regAddr,
                                                   fieldOffset, fieldLength,
                                                   value) != GT_OK)
            {
                return GT_HW_ERROR;
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                /* put port link up after changes */
                GT_U32 regAddr2;

                regAddr2 = 0x0A800000 + 0x0 + (portNum * 0x400);
                if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr2, 0, 1, 1) != GT_OK)
                {
                    return GT_HW_ERROR;
                }

            }
        }

        else
            continue;


    }

    return GT_OK;
}

/**
* @internal ptpPortUnitResetSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChPtpPortUnitResetSet
*
*/
static GT_STATUS ptpPortUnitResetSet_HardCodedRegAddr
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
)
{
    GT_U32      regAddr=0;        /* register address */
    GT_U32      resetMask;      /* reset bits mask */
    GT_U32      resetData;      /* reset bits data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /*PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        if (portNum < 56) /* 0-55 represents Network PTP */
        {
            regAddr = 0x10180808 + (portNum * 0x1000);
        }
        else if (portNum >=56 && portNum <=71) /* 56-71 represents Fabric PTP */
        {
            regAddr = 0x10180808 + 0x200000 + ((portNum-56) * 0x1000);
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    resetData = 0;
    /* Any PP: bit1 - PTP reset */
    resetMask = 2;
    if (resetTxUnit == GT_FALSE)
    {
        resetData |= 2;
    }
    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* Caelum bit13 - PTP Rx reset */
        resetMask |= 0x2000;
        if (resetRxUnit == GT_FALSE)
        {
            resetData |= 0x2000;
        }
    }

    /* Set PTP unit software reset bit [1:1] [13:13] */
    return prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, resetMask, resetData);
}


/**
* @internal portBobcat2HardCodedInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] value                    - enable/disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortLion2InternalLoopbackEnableSet
*
*/
static GT_STATUS portBobcat2HardCodedInternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                   value
)
{
    GT_U32                  regAddr=0;
    MV_HWS_PORT_STANDARD    portMode = QSGMII;
    GT_STATUS rc;
    if (portNum < 56) /* 0-55 network ports */
    {
        regAddr = 0x10000000 + (portNum * 0x1000);
        portMode = QSGMII;
    }
    else if (portNum >=56 && portNum <=71) /* 56-71 pabric ports */
    {
        regAddr = 0x10000000 + 0x200000 + ((portNum-56) * 0x1000);
        portMode = _1000Base_X;
    }

     /* disable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                           regAddr,
                                           0,
                                           1,
                                           0) != GT_OK)
    {
        return GT_HW_ERROR;
    }
    /*Reset PTP unit after port state is disabled*/
    rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_TRUE,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_MAC,TX_2_RX_LB);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_PCS,TX_2_RX_LB);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Unreset PTP before port state is enabled */
    rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_FALSE,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }
     /* enable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                           regAddr,
                                           0,
                                           1,
                                           1) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    return GT_OK;
}

/**
* @internal portBobcat2HardCodedInternalLoopbackXGEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on XG port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] value                    - enable/disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortLion2InternalLoopbackEnableSet
*
*/
GT_STATUS portBobcat2HardCodedInternalLoopbackXGEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                   value
)
{
    GT_U32                  regAddr=0;
    MV_HWS_PORT_STANDARD    portMode = QSGMII;
    GT_STATUS rc;
    if (portNum >=56 && portNum <=71) /* 56-71 pabric ports */
    {
        regAddr = 0x100C0000 + 0x200000 + ((portNum-56) * 0x1000);
        portMode = _10GBase_KR;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (portNum <= 31)
        {
            regAddr = 0x100C0000 + portNum * 0x1000;
            portMode = _10GBase_KR;
        }
    }

     /* disable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                           regAddr,
                                           0,
                                           1,
                                           0) != GT_OK)
    {
        return GT_HW_ERROR;
    }
    /*Reset PTP unit after port state is disabled*/
    rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_TRUE,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_MAC,TX_2_RX_LB);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_PCS,TX_2_RX_LB);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Unreset PTP before port state is enabled */
    rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_FALSE,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }
     /* enable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                           regAddr,
                                           0,
                                           1,
                                           1) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    return GT_OK;
}


/**
* @internal portEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:     Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS portEnableSet_HardCodedRegAddr
(
    IN GT_U8    devNum,
    IN GT_U32   portNum,
    IN GT_BOOL  enable
)
{
    PRV_CPSS_PORT_TYPE_ENT              portMacType;    /* MAC unit of port */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_U32                              regAddr;

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        return GT_INIT_ERROR;

    if (portNum < 72)
    {
        regAddr = 0x10000000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = enable;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    if (portNum < 72)
    {
        regAddr = 0x100c0000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = enable;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    if (portNum < 72)
    {
        regAddr = 0x100c0000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = enable;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    }

    if (portNum < 72)
    {
        regAddr = 0x10340000 + (portNum%36) * 0x1000 + (portNum/36)*0x400000;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = enable;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 20;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
    }

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;
        }

        if(regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                                    regDataArray[portMacType].regAddr,
                                                    regDataArray[portMacType].fieldOffset,
                                                    regDataArray[portMacType].fieldLength,
                                                    regDataArray[portMacType].fieldData) != GT_OK)
            {
                return GT_HW_ERROR;
            }
        }
    }

    return GT_OK;
}

/**
* @internal portBobcat3HardCodedInternalLoopbackXGEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on XG port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] value                    - enable/disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortLion2InternalLoopbackEnableSet
*
*/
GT_STATUS portBobcat3HardCodedInternalLoopbackXGEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                   value
)
{
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD    portMode;

    if (portNum == 0 || portNum == 4 || portNum == 8 || portNum == 12 || portNum == 16 || portNum == 20)
    {
        portMode = _100GBase_KR4;
    }
    else if (portNum >=24 && portNum < 72)
    {
        portMode = _10GBase_KR;
    }
    else
    {
        return GT_BAD_PARAM;
    }

    rc = portEnableSet_HardCodedRegAddr(devNum, portNum, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_PCS,TX_2_RX_LB);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = portEnableSet_HardCodedRegAddr(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}


/**
* @internal portLion2InternalLoopbackEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortLion2InternalLoopbackEnableSet
*
*/
static GT_STATUS portLion2InternalLoopbackEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{

    GT_U32      regAddr=0;    /* register address */
    GT_U32      value;      /* value to write */
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* MAC unit of port */
    MV_HWS_PORT_STANDARD    portMode;

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        return GT_INIT_ERROR;

    value = BOOL2BIT_MAC(enable);

    /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        return portBobcat2HardCodedInternalLoopbackEnableSet(devNum,portNum,value);
    }
    else

     if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        if (portNum < 56) /* 0-55 network ports */
        {
            regAddr = 0x08800000 + (portNum * 0x1000);
        }
        else
        {
            regAddr = 0x08800000 + 0x200000 + ((portNum-56) * 0x1000);
        }

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (portNum <= 23)
        {
            regAddr = 0x12000004 + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 5;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }


    /*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macRegsPerType[PRV_CPSS_PORT_XLG_E].macCtrl; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (portNum < 16) /* 0-15 represents Fabric XLG Mac */
        {
            regAddr = 0x100C0000 + 0x200000 + (portNum * 0x1000);
        }
        else if (portNum >=48 && portNum <=55) /* 48-55 represents Network XLG Mac */
        {
            regAddr = 0x100C0000 + (portNum * 0x1000);
        }
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        }
    }else
     if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        if (portNum < 16)
        {
            regAddr = 0x088C0000 + (portNum * 0x1000);
        }
        else
        {
            regAddr = 0x08800000 + 0x200000 + ((portNum-56) * 0x1000);
        }
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (portNum > 23)
        {
            regAddr = 0x120C0004 + (portNum * 0x1000);
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 13;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;
        }

        if(regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                                    regDataArray[portMacType].regAddr,
                                                    regDataArray[portMacType].fieldOffset,
                                                    regDataArray[portMacType].fieldLength,
                                                    regDataArray[portMacType].fieldData) != GT_OK)
            {
                return GT_HW_ERROR;
            }
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_TRUE,GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* configuration according to Bobcat2 specific Eeprom*/
    portMode  = (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)?QSGMII:_1000Base_X;

    if (portNum < 56) /* 0-55 network ports */
    {
        rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_MAC,TX_2_RX_LB);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = mvHwsPortLoopbackSet(devNum,0,portNum,portMode,HWS_PCS,TX_2_RX_LB);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else if (portNum >=56 && portNum <=71) /* 56-71 pabric ports */
    {
        rc = mvHwsPortLoopbackSet(devNum,0,portNum,_1000Base_X,HWS_MAC,TX_2_RX_LB);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = mvHwsPortLoopbackSet(devNum,0,portNum,_1000Base_X,HWS_PCS,TX_2_RX_LB);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        rc = ptpPortUnitResetSet_HardCodedRegAddr(devNum,portNum,GT_FALSE,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (loopback/no loopback)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       based on cpssDxChPortInternalLoopbackEnableSet
*
*/
GT_STATUS appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
)
{
    GT_U32      portNum;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    devNum = devNum;
    portNum = portInterfacePtr->devPort.portNum;


    if((CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(devNum)->devFamily) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        if(portNum != CPSS_CPU_PORT_NUM_CNS)
        {
            return portLion2InternalLoopbackEnableSet_HardCodedRegAddr(devNum, portNum, enable);
        }
    }

    /* for CPU port and older devices */
    return portPreLion2InternalLoopbackEnableSet_HardCodedRegAddr(devNum, portNum, enable);
}

/**
* @internal appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure TxQ port 2 TxDMA mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - txq port
* @param[in] txDmaNum                 - txDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingTxQPort2TxDMAMapSet
*
*/
GT_STATUS appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /*PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[txqNum];*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (txqNum >= 576)
            return GT_BAD_PARAM;
        if (txqNum < (72+96*((txqNum/96))) || txqNum >= (96 + 96*((txqNum/96)+1)))
        {
            regAddr = 0x0B0002C0 + (txqNum%96)*0x4 + ((txqNum/96)-(txqNum/288)*3)*0x1000000 + (txqNum/288)*0x80000000;
        }
        else
        {
            regAddr = 0x0b00e220 + (txqNum - 96*(txqNum/96) - 72)*0x4 + ((txqNum/96)-(txqNum/288)*3)*0x1000000 + (txqNum/288)*0x80000000;
        }
    }
    else
    {
        regAddr = 0x400002C0 + txqNum*0x4; /* where txqNum (0-71) represents Port */
    }

    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                         0,
                                         8,
                                         txDmaNum);
    if (GT_OK != rc )
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - physical port
*                                      rxDmaNum   - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet
*
*/
GT_STATUS appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 physPort,
    IN GT_U32 rxDMAPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    address;    /* address to write to */
    GT_U32    entrySize;  /* table entry size in words */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* tablePtr->baseAddress + entryIndex * tablePtr->step */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        address = 0x4105a000 + physPort*0x4;
    }
    else
    {
        address = 0x2d01a000 + physPort*0x4;
    }
    entrySize = 1;

    /* write whole entry */
    rc = prvCpssHwPpWriteRam(devNum, address, entrySize, &rxDMAPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingRxDMA2LocalPhysSet
*
*/
GT_STATUS appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr;            /* register's address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->rxDMA[0].singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum]; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (rxDmaNum < 72)
            regAddr = 0x19000950 + (rxDmaNum%12)*0x04 + ((rxDmaNum-(rxDmaNum/36)*36)/12)*0x1000000 + (rxDmaNum/36)*0x80000000;
        else if (rxDmaNum == 72)
            regAddr = 0x1b000980;
        else if (rxDmaNum == 73)
            regAddr = 0x9b000980;
        else if (rxDmaNum == 74)
            regAddr = 0x19000980;
        else if (rxDmaNum == 75)
            regAddr = 0x1a000980;
        else if (rxDmaNum == 76)
            regAddr = 0x99000980;
        else if (rxDmaNum == 77)
            regAddr = 0x9a000980;
        else
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if (rxDmaNum < 24)
            regAddr = 0x19000950 + (rxDmaNum%12)*0x04 + (rxDmaNum/12)*0x1000000;
        else if (rxDmaNum >= 24 && rxDmaNum < 48)
            regAddr = 0x1b000950 + (rxDmaNum%24)*0x04;
        else if (rxDmaNum >=48 && rxDmaNum <72)
            regAddr = 0x99000950 + (rxDmaNum%24)*0x04;
        else if (rxDmaNum == 72)
            regAddr = 0x190009b0;
        else if (rxDmaNum == 74)
            regAddr = 0x990009b0;
        else if (rxDmaNum == 75)
            regAddr = 0x1a0009b0;
        else if (rxDmaNum == 76)
            regAddr = 0x1b0009b0;
        else if (rxDmaNum == 77)
            regAddr = 0x190009ac;
        else
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    else
    {
        regAddr = 0x17000950 + rxDmaNum*0x4;  /* rxDmaNum (0-72) represents Port */
    }

    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                         0, /* PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS */
                                            /* PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS */
          PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort,
                                         physPort);

    if (GT_OK != rc )
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Bobcat2 Port mapping initial configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChBobcat2PortMappingConfigSet
*
*/
GT_STATUS appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr
(
    IN  GT_U8  devNum,
    IN  GT_U32 physPort
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 address;     /* register/table address */
    GT_U32 regValue;    /* register value */
    GT_U32  wordIndex;/*index in registers array*/
    GT_U32  bitIndex;/*index in the register*/

/* CPU PORT NUMBER Definition */
#define CPSS_CPU_PORT_NUM_CNS           63

    /***************************************/
    /* Bobcat2 Port Mapping Configurations */
    /***************************************/

    /* Enable TXQ to DMA port mapping */
    address = 0x40000000;
        /* PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.globalDequeueConfig; */
    rc = prvCpssDrvHwPpSetRegField(devNum, address, 10, 1, 1);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Port To DMA Map Table Configuration */
    address = 0x400002C0 + physPort*0x4;  /* (0-71) represents Port */
            /* PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[i]; */

    regValue = (physPort == CPSS_CPU_PORT_NUM_CNS) ? 72 : physPort;

    rc = prvCpssDrvHwPpSetRegField(devNum, address, 0, 8, regValue);
    if(GT_OK != rc)
    {
        return rc;
    }


    /* EGF Physical port To TXQ port Table Configuration */

    /* 1:1 mapping for all TXQ ports */
    address = 0x3b900000 + physPort*4; /* tablePtr->baseAddress + entryIndex * tablePtr->step; */

    rc = prvCpssHwPpPortGroupWriteRam(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, address, 1, &physPort);
    if(GT_OK != rc)
    {
        return rc;
    }

    /*Virtual 2 Physical Port Remap <%n>*/
    /* set 1:1 the 'virtual port' to the physical port mapping for the
       'EGF_EFT' link up/down issues */

    address = 0x35010120 ;  /* (0-63) represents offset */
        /* PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.virtual2PhysicalPortRemap[0]*/

    wordIndex = (physPort / 4);
    bitIndex  = (physPort % 4) * 8;

    rc = prvCpssHwPpSetRegField(devNum,
             address + wordIndex*4,
             bitIndex ,
             8,
             physPort); /*global physical port*/
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
*
*/
GT_STATUS appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwFormat = 0;
    GT_U32    hwDev, hwPort;
    GT_U32    vidx;
    GT_U32    numOfBits;
    GT_U32    entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    GT_U32    address;    /* address to write to */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(physicalInfoPtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    numOfBits = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 23 : 22;


    switch (physicalInfoPtr->type)
    {
        case CPSS_INTERFACE_PORT_E:

            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                   physicalInfoPtr->devPort.portNum);

            hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                         physicalInfoPtr->devPort.portNum);
            hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                          physicalInfoPtr->devPort.portNum);

            if((hwPort >= BIT_8) ||
               (hwDev > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)))
            {
                /* 8 bits for the physical port number and 10 bits for device number */
                return GT_BAD_PARAM;
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,0); /* target is not trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,0); /* target not VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,10,hwDev);
            U32_SET_FIELD_MAC(hwFormat,14,
                PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 9 : 8,hwPort);

            break;
         case CPSS_INTERFACE_TRUNK_E:
            if(physicalInfoPtr->trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                /* 12 bits for the trunkId */
                return GT_BAD_PARAM;
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,1); /* target is trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,0); /* target not VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,12,physicalInfoPtr->trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            if(physicalInfoPtr->type == CPSS_INTERFACE_VID_E)
            {
                vidx = 0xFFF;
            }
            else
            {
                vidx = physicalInfoPtr->vidx;

                if (vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum))
                {
                    return GT_BAD_PARAM;
                }
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,0); /* target is not trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,1); /* target is VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,12,vidx);
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* MTU Index was added by a separate API: cpssDxChBrgGenMtuPortProfileIdxSet */
    /* we not change the bits 0,1 <MTU Index> */
    hwFormat >>= 2;

    /* set ePort-to-physical-port-target-mapping table */

    /* tablePtr->baseAddress + entryIndex * tablePtr->step; */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        address = 0x9C000000 + portNum * 4;
    }
    else
    {
        address = 0x0dc00000 + portNum * 4;
    }
    /* read whole entry */
    rc = prvCpssHwPpPortGroupReadRam(devNum, 0, address, 1 /* entrySize */,
                                                     entryMemoBufArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update field */
    U32_SET_FIELD_MAC(entryMemoBufArr[0], 2 /*fieldOffset*/, numOfBits - 2 /*fieldLength*/, hwFormat /*fieldValue*/);

    /* write whole entry */
    rc = prvCpssHwPpPortGroupWriteRam(devNum, 0, address, 1 /*entrySize*/,
                                                     entryMemoBufArr);
    if (rc != GT_OK)
    {
        return rc;
    }


    return rc;
}



/**
* @internal appDemoDxChCpuPortPizzaResources_HardCodedRegAddr function
* @endinternal
*
* @brief   Configure cpu port pizza resources.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoDxChCpuPortPizzaResources_HardCodedRegAddr
(
    IN GT_U8                            devNum
)
{
    GT_STATUS rc = GT_OK;
    cpssOsPrintf("config cpu port pizza resources\n");
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x17000924, 0, 3, 0);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x26003900, 0, 9, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x26003918, 0x10, 0x10, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x26003918, 0, 0x10, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x2600390c, 0, 0xa, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x2600390c, 0xa, 0xa, 4);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x27000720, 0, 3, 3);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x27000128, 0, 7, 2);
    return rc;

}


/**
* @internal appDemoDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Lion2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Only Cetus applicable for now, not Caelum
*
*/

GT_STATUS appDemoDxChPortModeSpeedSet
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              portsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |CPSS_XCAT2_E|CPSS_XCAT3_E|
        CPSS_LION_E);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        switch(PRV_CPSS_PP_MAC(devNum)->devType)
        {
            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                {
                    /* TBD, we override eeprom configuration with this 1G eeprom */
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    portsBmp.ports[1]=0x0F000000; /* ports 56-59 */
                    portsBmp.ports[2]=0x00000003; /* ports 64,65 */
                    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                    speed = CPSS_PORT_SPEED_10000_E;
                    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                    if (GT_OK != rc )
                    {
                        cpssOsPrintf( " 10G ports failed do powerup\n");

                        return rc;
                    }

                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    portsBmp.ports[2]=0x000000FC; /* ports 66-71 */
                    ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
                    speed = CPSS_PORT_SPEED_1000_E;
                    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                    if (GT_OK != rc )
                    {
                        cpssOsPrintf( " 1G ports failed do powerup\n");

                        return rc;
                    }
                }
                break;
            default:
                {
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    ifMode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;/* ports from 0-47 QSGMII */
                    speed = CPSS_PORT_SPEED_1000_E;
                    portsBmp.ports[0]=0xFFFFFFEF;/* do not override configuration done by eeprom to port 4 */
                    portsBmp.ports[1]=0x0000FFFF;
                    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                    if (GT_OK != rc )
                    {
                        cpssOsPrintf( " 1G ports failed do powerup\n");
                        return rc;
                    }
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    portsBmp.ports[1]=0x020E0000; /* do not override port 58 */
                    portsBmp.ports[2]=0x00050000;
                    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;/* ports 49,50,51,57,58,80,82 KR */
                    speed = CPSS_PORT_SPEED_10000_E;
                    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                    if (GT_OK != rc )
                    {
                        cpssOsPrintf( " 10G ports failed do powerup\n");

                        return rc;
                    }
                }
                break;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /*Exclude ports that are already configured in eeprom*/

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);

        portsBmp.ports[0]=0x0efb0e00;
        portsBmp.ports[1]=0x0bff0fef;
        portsBmp.ports[2]=0x0fff0fff;
        portsBmp.ports[3]=0x0fff0fff;

        ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;/*default interface mode for Lion2*/
        speed =  CPSS_PORT_SPEED_10000_E;/*default speed for Lion2*/

        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode,speed);

        if (GT_OK != rc )
        {
            return rc;
        }

    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);

        portsBmp.ports[0]=0xffffffff;

        ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        speed =  CPSS_PORT_SPEED_10000_E;

        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode,speed);

        if (GT_OK != rc )
        {
            return rc;
        }

    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;/* ports from 0-47 QSGMII */
        speed = CPSS_PORT_SPEED_100G_E;
        portsBmp.ports[0]=0x00111111;
        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
        if (GT_OK != rc )
        {
            cpssOsPrintf( " 1G ports failed do powerup\n");
            return rc;
        }
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        portsBmp.ports[0]=0xFF000000; /* do not override port 58 */
        portsBmp.ports[1]=0x07FFFFFF;
        portsBmp.ports[2]=0x00003FFF;
        ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;/* ports 49,50,51,57,58,80,82 KR */
        speed = CPSS_PORT_SPEED_10000_E;
        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
        if (GT_OK != rc )
        {
            cpssOsPrintf( " 10G ports failed do powerup\n");

            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoDxChPortFastInit function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2: xCat3; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChPortFastInit
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              portsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |CPSS_XCAT2_E|CPSS_XCAT3_E|
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E| CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_LION2_E);


    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /* Fast ports configuration (interface, speed, serdes power up)
        Same like cpssDxChPortModeSpeedSet(powerUp=true) but faster.
        Purposed for use on system init, so assumes that ports are in HW default
        i.e. all port's units in reset and serdes in power down.     */
        portsBmp.ports[0]=0xFFF0CAA;/* do not override configuration done by eeprom to ports 0,2,4,6,8,9 */
        portsBmp.ports[1]=0xFFF0FFF;
        ifMode = CPSS_PORT_INTERFACE_MODE_RXAUI_E;/* by default configure ports of Lion 48 to RXAUI mode */
        speed = CPSS_PORT_SPEED_10000_E;
        rc =cpssDxChPortsFastInit(devNum,&portsBmp,ifMode,speed);

        if (GT_OK != rc )
        {
            return rc;
        }
    }
    else
    {
        /* For other families when needed :
        set portsBmp,ifMode and speed values according to the device */
    }
    return GT_OK;
}

/**
* @internal appDemoDxChPortMappingRestore function
* @endinternal
*
* @brief   Restore port mapping configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Only Cetus applicable for now, not Caelum
*
*/

GT_STATUS appDemoDxChPortMappingRestore
(
    IN  GT_U8 devNum
)
{
    GT_STATUS                           rc;
    GT_PORT_NUM                         portNum;
    GT_HW_DEV_NUM                       hwDevNum;
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    GT_U32                              numOfPhysicalPorts;
    GT_U32                              mapArrLen;
    CPSS_DXCH_PORT_MAP_STC              *mapArrPtr;
    GT_U8 boardRevId = 1;

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
            /* configBoardAfterPhase1MappingGet() returns wrong values for cetus */
            rc = configBoardAfterPhase1MappingGetCetus(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
            break;
        case CPSS_ALDRIN_DEVICES_CASES_MAC:
            /* configBoardAfterPhase1MappingGet() returns wrong values for Aldrin */
            rc = configBoardAfterPhase1MappingGetAldrin(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
            break;
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
            /* configBoardAfterPhase1MappingGet() returns wrong values for Bobcat3 */
            rc = configBoardAfterPhase1MappingGetBobcat3(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
            break;
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            /* configBoardAfterPhase1MappingGet() returns wrong values for Aldrin2 */
            rc = configBoardAfterPhase1MappingGetAldrin2(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
            break;
        default:
            rc = configBoardAfterPhase1MappingGet(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
            break;
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    if (mapArrPtr == NULL)
    {
        cpssOsPrintf("\n-->ERROR : appDemoDxChPortMappingRestore() mapping : rev =%d is not supported\n", boardRevId);
        return GT_NOT_SUPPORTED;
    }
    for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
    {
        prvCpssDxChPortPhysicalPortMapShadowDBClear(devNum,portNum);
    }

    rc = cpssDxChPortPhysicalPortMapSet(devNum, mapArrLen, mapArrPtr);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChPortPhysicalPortMapSet is failed, rc =%d\n",rc);
        return rc;
    }

    hwDevNum = 0x10;
    rc = cpssDxChCfgHwDevNumSet(devNum, hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }
    /* restore e2phy */
    numOfPhysicalPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    cpssOsPrintf("numOfPhysicalPorts =%d\n",numOfPhysicalPorts);
    for(portNum=0; portNum < numOfPhysicalPorts; portNum++)
    {
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,
                                                                 portNum,
                                                                 &physicalInfo);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChBrgEportToPhysicalPortTargetMappingTableGet is failed, rc =%d\n",rc);

            return rc;
        }
        /* If <Target Is Trunk> == 0 && <Use VIDX> == 0 */
        if(physicalInfo.type == CPSS_INTERFACE_PORT_E)
        {
            physicalInfo.devPort.hwDevNum = hwDevNum;
            rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     portNum,
                                                                     &physicalInfo);
            if(rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet is failed, rc =%d\n",rc);
                return rc;
            }
        }
    }
    return GT_OK;
}


/**
* @internal appDemoDxChEventRestore function
* @endinternal
*
* @brief   Restore appDemo events
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChEventRestore(void)
{
    GT_STATUS rc;
    rc = appDemoEventRequestDrvnModeInit();
    return rc;
}


/**
* @internal appDemoDxChVlanPvidRestore function
* @endinternal
*
* @brief   Restore default vlan and pvid
*
* @note   APPLICABLE DEVICES:      xCat; xCat2; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat3; Aldrin2; Falcon; AC3X.
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS appDemoDxChVlanPvidRestore
(
    IN  GT_U8 dev
)
{
    GT_STATUS           rc = GT_OK;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_U16              vid;            /* default VLAN id          */
    GT_U32              port;           /* current port number      */

    /*********************************************************/
    /* Default VLAN configuration: VLAN 1 contains all ports */
    /*********************************************************/

    /* default VLAN id is 1 */
    vid = 1;

    /* Fill Vlan info */
    osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0xFFF;

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;

    if(appDemoSysConfig.forceAutoLearn == GT_FALSE)
    {
        cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    }

    cpssVlanInfo.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv6McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.floodVidx              = 0xFFF;

    cpssVlanInfo.mirrToRxAnalyzerIndex = 0;
    cpssVlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
    cpssVlanInfo.mirrToTxAnalyzerIndex = 0;
    cpssVlanInfo.fidValue = vid;
    cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerIndex = 0;
    cpssVlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv6McMirrToAnalyzerIndex = 0;

    /* Fill ports and tagging members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set all ports as VLAN members */
    for (port = 0; port < (appDemoPpConfigList[dev].maxPortNumber); port++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(dev,port);

        /* set the port as member of vlan */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);

        /* Set port pvid */
        rc = cpssDxChBrgVlanPortVidSet(dev, port, CPSS_DIRECTION_INGRESS_E,vid);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
        if(rc != GT_OK)
             return rc;

        portsTaggingCmd.portsCmd[port] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }
    /* for port 83 that could not be mapped:*/
    if ( (PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
         (PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) )
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 83);
        portsTaggingCmd.portsCmd[83] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* Write default VLAN entry */
    rc = cpssDxChBrgVlanEntryWrite(dev, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);

    return rc;
}



