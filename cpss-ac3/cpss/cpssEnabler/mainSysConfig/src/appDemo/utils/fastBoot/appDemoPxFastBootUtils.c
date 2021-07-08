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
* @file appDemoPxFastBootUtils.c
*
* @brief App demo Px FastBoot API.
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
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <appDemo/userExit/userEventHandler.h>

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/port/cpssPortStat.h>

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

static GT_U32 internal_gtMacCounterOffset_all64Bits[CPSS_PX_PORT_MAC_COUNTER____LAST____E] =
{
     0x00   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E                */
    ,0x08   /*CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E                 */
    ,0x10   /*CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E                     */
    ,0x18   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E        */
/*0x20 in other place */
    ,0x28   /*CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E           */
    ,0x30   /*CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E           */
    ,0x38   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E                    */
    ,0x40   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E             */
    ,0x48   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E            */
    ,0x50   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E            */
    ,0x58   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E           */
    ,0x60   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E           */
    ,0x20   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E          */
    ,0x68   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E                    */
    ,0x70   /*CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E                  */
/*0x78 -- reserved*/
    ,0x80   /*CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E               */
    ,0x88   /*CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E               */
/*0x90 -- reserved*/
    ,0x98   /*CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E                             */
    ,0xA0   /*CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E                         */
    ,0xA8   /*CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E               */
    ,0xB0   /*CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E                           */
    ,0xB8   /*CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E                           */
    ,0xC0   /*CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E                            */
    ,0xC8   /*CPSS_PX_PORT_MAC_COUNTER_JABBER_E                              */
    ,0xD0   /*CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E             */
    ,0xD8   /*CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E                             */
    ,0xE0   /*CPSS_PX_PORT_MAC_COUNTER_COLLISION_E                           */
    ,0xE8   /*CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E                      */
};


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

extern GT_VOID prvCpssPxPortSumMibEntry
(
    INOUT GT_U32 destEntryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN    GT_U32 addEntryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/]
);

/**
* @internal appDemoPxMacCounterGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets Ethernet MAC counter / MAC Captured counter for a
*         specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on prvCpssPxPortMacCounterGet
*
*/
GT_STATUS appDemoPxMacCounterGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;      /* return code */
    GT_U32 regAddr=0;         /* register address */
    GT_U32 *entryPtr;
    GT_U32 xlgMibEntry[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32  portGroupId; /* Group id associated with the portNum */
    GT_U32 byteOffset;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    CPSS_PARAM_CHECK_MAX_MAC(cntrName,CPSS_PX_PORT_MAC_COUNTER____LAST____E);

    /* Get port group according to port mac num */
    portGroupId = 0;

    cntrValuePtr->l[0] = 0;
    cntrValuePtr->l[1] = 0;

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    xlgMibShadowPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portNum];

    /*PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].macCounters*/
    regAddr = 0x12000000 + 0x400 * portNum;

    /* read the whole entry */
    rc = prvCpssDrvHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum),portGroupId, regAddr,
          PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS,xlgMibEntry);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* add to shadow */
    prvCpssPxPortSumMibEntry(xlgMibShadowPtr->mibShadow,xlgMibEntry);
    entryPtr = xlgMibShadowPtr->mibShadow;

    byteOffset = internal_gtMacCounterOffset_all64Bits[cntrName];

    /* get the counter from the entryPtr */
    cntrValuePtr->l[0] = entryPtr[byteOffset/4];
    cntrValuePtr->l[1] = entryPtr[byteOffset/4 + 4];

    if(xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
    {
        /* Clearing the specific entry in the shadow */
        entryPtr[byteOffset/4] = 0;
        entryPtr[byteOffset/4 + 4] = 0;
    }

    return GT_OK;
}

/**
* @internal appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxCfgCntrGet
*
*/
GT_STATUS appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
)
{
    GT_U32      regAddr;     /* register address */

    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    regAddr = 0x1B000024;
    /*PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MCFC.cntrs.ingrDropCntr; */
    return prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                                       0, regAddr, 0,
                                                       32, counterPtr);
}

/**
* @internal appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable forwarding to the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable forwarding to the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssPxIngressPortTargetEnableSet
*
*/
GT_STATUS appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable
)
{
    GT_U32 regAddr;             /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* PCP Ports Enable Config */
    regAddr = 0xE004614; /*regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPortsEnableConfig;*/
    /* PCP Enable Port Config <portNum> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal appDemoPxIngressPortMapEntrySet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] portsBmp                 - the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Source, Destination port map tables, LAG designated table and Port Enable
*       map table are used in calculation of egress ports
*       to which the unicast or multicast packet is forwarded.
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssPxIngressPortMapEntrySet
*
*/
GT_STATUS appDemoPxIngressPortMapEntrySet_HardCodedRegAddr
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_TABLE_ENT       tableType,
    IN  GT_U32                  entryIndex,
    IN  CPSS_PX_PORTS_BMP       portsBmp
)
{
    GT_U32    address;    /* address to write to */
    GT_U32    entrySize;  /* table entry size in words */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp);

    switch (tableType)
    {
        case CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E:
            address = 0x0E018000 + entryIndex*0x4; /* tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes */
            break;
        case CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E:
            address = 0x0E008000 + entryIndex*0x4; /* tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes */
            break;
        default:
            return GT_BAD_PARAM;
    }
    entrySize = 1;

    return prvCpssHwPpWriteRam(devNum, address, entrySize, &portsBmp);
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
* @note copied from pxNetIfSdmaTxPacketSend()
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


    cpssOsPrintf("\nSending brodcast packet\n");

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
* @internal appDemoPxPortEgressCntrsGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxPortEgressCntrsGet
*
*/
GT_STATUS appDemoPxPortEgressCntrsGet_HardCodedRegAddr
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(egrCntrPtr);

    if (cntrSetNum > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read Outgoing Packet Count Register */
    regAddr = 0x1E093210 + (cntrSetNum*0x4);
    /*regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setOutgoingPktCntr[cntrSetNum];*/
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr, 0, 32, &egrCntrPtr->outFrames) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    /* read Transmit Queue Filtered Packet Count Register */
    regAddr = 0x1E093250 + (cntrSetNum*0x4);
    /*regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[cntrSetNum];*/
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr, 0, 32, &egrCntrPtr->txqFilterDisc) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    return GT_OK;
}

/**
* @internal appDemoPxSdmaTxPacketSend_HardCodedRegAddr function
* @endinternal
*
* @brief   This function sends a single packet.
*         The packet is sent through interface port type to port given by
*         dstPortNum.
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
GT_STATUS appDemoPxSdmaTxPacketSend_HardCodedRegAddr
(
    IN GT_U8    devNum,
    IN TGF_PACKET_PAYLOAD_STC  *packetPtr,
    IN GT_U32   dstPortNum
)
{
    GT_U8 *buffArr,*origBuffArrMAllocPtr = NULL;
    PRV_CPSS_TX_DESC_STC    *txDescPtr,*origTxDescAllocPtr = NULL;/* pointer to txDescriptor*/
    GT_U32 buffSize4alloc;
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

    dsaTagNumOfBytes = 0;/* number of bytes in the DSA */

    dataOffset = MAC_ADDR_LENGTH_CNS;/*12*/

    /* copy the reset of the packet  */
    cpssOsMemCpy(&buffArr[MAC_ADDR_LENGTH_CNS + dsaTagNumOfBytes] ,
        &packetPtr->dataPtr[dataOffset],
        (packetPtr->dataLength-dataOffset));

    totalLength = (packetPtr->dataLength - dataOffset) + /* length of 'rest of packet' */
                  (MAC_ADDR_LENGTH_CNS + dsaTagNumOfBytes);/*length of DSA + macDa,Sa */

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
* @internal appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxPortForceLinkPassEnableSet
*
*/
GT_STATUS appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* MAC unit of port */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

    portGroupId = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(state);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        return GT_INIT_ERROR;

    /*PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].autoNegCtrl; */
    if (portNum < 16) /* 0-15 network ports */
    {
        regAddr = 0x1000000C + (portNum * 0x1000);
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    /* PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].\
                        macRegsPerType[PRV_CPSS_PORT_XLG_E].macCtrl; */
    if (portNum < 16) /* 0-15 represents Network XLG Mac */
    {
        regAddr = 0x100C0000  + (portNum * 0x1000);
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 3;
    }

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if (regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,
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
* @internal ptpPortUnitResetSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on prvCpssPxPtpPortUnitResetSet
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
    GT_U32      regAddr;        /* register address */
    GT_U32      resetMask;      /* reset bits mask */
    GT_U32      resetData;      /* reset bits data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = 0x10180808 + 0x1000 * portNum;
    /*PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;*/
    resetData = 0;
    resetMask = (1 << 1) | (1 << 8);

    if (resetTxUnit == GT_FALSE)
    {
        U32_SET_FIELD_MAC(resetData, 1, 1, 1);
    }

    if (resetRxUnit == GT_FALSE)
    {
        U32_SET_FIELD_MAC(resetData, 8, 1, 1);
    }

    return prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, resetMask, resetData);
}


/**
* @internal appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - physical port number (not CPU port)
* @param[in] value                    - enable/disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssPxPortInternalLoopbackEnableSet
*
*/
GT_STATUS appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN  GT_U32                   value
)
{
    GT_U32                  portNum;
    GT_U32                  regAddr;
    GT_U32                  regAddr2;
    MV_HWS_PORT_STANDARD    portMode = _10GBase_KR;
    GT_STATUS               rc;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    portNum = portInterfacePtr->devPort.portNum;

    regAddr = 0x100C0000 + (portNum * 0x1000);
    regAddr2 = 0x0E004614;

    /* disable forwarding to the target port */
    if (prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr2, portNum, 1, 0) != GT_OK)
    {
        return GT_HW_ERROR;
    }

     /* disable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr, 0, 1, 0) != GT_OK)
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

    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0,
                                           regAddr,
                                           0,
                                           1,
                                           1) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    /* enable port */
    if (prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr, 0, 1, 1) != GT_OK)
    {
        return GT_HW_ERROR;
    }

    /* enable forwarding to the target port */
    if (prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr2, portNum, 1, 1) != GT_OK)
    {
        return GT_HW_ERROR;
    }
    return GT_OK;
}

/**
* @internal appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure TxQ port 2 TxDMA mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on TxQPort2TxDMAMapSet
*
*/
GT_STATUS appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(txDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    CPSS_PARAM_CHECK_MAX_MAC(txqNum,PRV_CPSS_PX_MAX_DQ_PORTS_CNS);

    /* need to write the 'local' DMA number */
    if (txqNum < 10)
    {
        regAddr = 0x210002C0 + txqNum*0x4; /* where txqNum (0-9) represents Port */    
    }
    else
    {
        regAddr = 0x220002C0 + (txqNum - 10)*0x4; /* where txqNum (10-19) represents Port */    
    }
    /*PRV_PX_REG1_UNIT_TXQ_DQ_MAC(0).global.globalDQConfig.portToDMAMapTable[txqNum];*/
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,0,8,txDmaNum);
    return rc;

}

/**
* @internal appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - physical port
* @param[in] rxDmaNum                 - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on BMAMapOfLocalPhys2RxDMASet
*
*/
GT_STATUS appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 physPort,
    IN GT_U32 rxDmaNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    address;    /* address to write to */
    GT_U32    entrySize;  /* table entry size in words */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(rxDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    address = 0x1805A000 + physPort*0x4; /* tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes */
    entrySize = 1;

    /* write whole entry */
    rc = prvCpssHwPpWriteRam(devNum, address, entrySize, &rxDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on rxDMA2LocalPhysSet
*
*/
GT_STATUS appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(rxDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    /* PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->rxDMA.singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum]; */
    regAddr = 0x1000950 + rxDmaNum*0x4;  /* rxDmaNum (0-16) represents Port */

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,9,physPort);

    return rc;
}

/**
* @internal appDemoPxPipePortMappingConfigSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Pipe Port mapping initial configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssPxPipePortMappingConfigSet
*
*/
GT_STATUS appDemoPxPipePortMappingConfigSet_HardCodedRegAddr
(
    IN  GT_U8  devNum,
    IN  GT_U32 physPort
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    /***************************************/
    /* PIPE -- Port Mapping Configurations */
    /***************************************/

    /* Enable TXQ to DMA port mapping */
    regAddr = 0x21000000;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 1, 1);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* bind the DQ port to the DMA number */
    /* need to write the 'local' DMA number */
    if (physPort < 10)
    {
        regAddr = 0x210002C0 + physPort*0x4; /* where physPort (0-9) represents Port */    
    }
    else
    {
        regAddr = 0x220002C0 + (physPort - 10)*0x4; /* where physPort (10-19) represents Port */    
    }
    /*PRV_PX_REG1_UNIT_TXQ_DQ_MAC(0).global.globalDQConfig.portToDMAMapTable[physPort];*/
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,0,8,physPort);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPxCpuPortPizzaResources_HardCodedRegAddr function
* @endinternal
*
* @brief   Configure cpu port pizza resources.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPxCpuPortPizzaResources_HardCodedRegAddr
(
    IN GT_U8                            devNum
)
{
    GT_STATUS rc = GT_OK;
    cpssOsPrintf("config cpu port pizza resources\n");
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x01000844, 0, 3, 0);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x02003200, 0, 9, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x02003218, 0x10, 0x10, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x02003218, 0, 0x10, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x0200320C, 0, 0xa, 2);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x0200320C, 0xa, 0xa, 4);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x04000640, 0, 3, 3);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x04000048, 0, 7, 2);
    return rc;

}

/**
* @internal appDemoPxPortModeSpeedSet function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS appDemoPxPortModeSpeedSet
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              portsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    portsBmp.ports[0]=0x00000FFF; /* ports 0-11 */
    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
    speed = CPSS_PORT_SPEED_10000_E;
    rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
    if (GT_OK != rc )
    {
        cpssOsPrintf( " 10G ports failed do powerup\n");

        return rc;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    portsBmp.ports[0]=0x0000F000; /* ports 12-15 */
    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
    speed = CPSS_PORT_SPEED_25000_E;
    rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
    if (GT_OK != rc )
    {
        cpssOsPrintf( " 25G ports failed do powerup\n");

        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPxEventRestore function
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
GT_STATUS appDemoPxEventRestore(void)
{
    GT_STATUS rc;
    rc = appDemoEventRequestDrvnModeInit();
    return rc;
}


