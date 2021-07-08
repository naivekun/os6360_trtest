/*******************************************************************************
*                Copyright 2014, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file osNetworkStack.c
*
* @brief CPSS <=> OS TCP/IP stack
* Linux BM only now
*
* @version   1
********************************************************************************
*/
#include <extUtils/osNetworkStack/osNetworkStack.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <extUtils/iterators/extUtilDevPort.h>
#ifdef CHX_FAMILY
#  include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#  include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#  include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#  include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#endif
/*TODO: EXMXPM */

static GT_BOOL  nsInitialized = GT_FALSE;
static CPSS_OS_MUTEX nsMtx = (CPSS_OS_MUTEX)0;
#define NSLOCK() cpssOsMutexLock(nsMtx)
#define NSUNLOCK() cpssOsMutexUnlock(nsMtx)

/* interface MAC address (current) */
static GT_U8    nsMacAddr[6];
static GT_U8    nsBroadcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
static GT_BOOL  nsConnected = GT_FALSE;
static GT_U8    nsDevId;
static GT_HW_DEV_NUM nsHwDevNum;
static GT_U16   nsVlanId;
/* */
#ifdef CHX_FAMILY
static CPSS_MAC_ENTRY_EXT_STC nsMacEntry;
#endif

#define NS_TRANSMIT_BUFF_SIZE 2000
static GT_U8    *nsTransmitBuff = NULL;

/* os depended */
GT_STATUS prvOsNetworkStackDeviceOpen(void);
GT_STATUS prvOsNetworkStackDeviceClose(void);
GT_STATUS prvOsNetworkStackDeviceRead(
   OUT   GT_U8  *pktBuf,
   INOUT GT_U32 *pktLen
);
GT_STATUS prvOsNetworkStackDeviceWrite(
  IN    GT_U8   *pktBuf,
  IN    GT_U32  pktLen
);
GT_STATUS prvOsNetworkStackMacGet(
  OUT GT_ETHERADDR *etherPtr
);
GT_STATUS prvOsNetworkStackMacSet(
  IN  GT_ETHERADDR *etherPtr
);
GT_STATUS prvOsNetworkStackIfconfig(
  IN const char *config
);

/**
* @internal osNetworkStackInit function
* @endinternal
*
* @brief   Initialize library
*/
GT_STATUS osNetworkStackInit(void)
{
    char nm[]="nsMtx";
    GT_STATUS rc;
    if (nsInitialized == GT_TRUE)
    {
        return GT_OK;
    }
    rc = cpssOsMutexCreate(nm, &nsMtx);
    if (rc != GT_OK)
        return rc;
#ifdef CHX_FAMILY
    cpssOsMemSet(&nsMacEntry, 0, sizeof(nsMacEntry));
#endif
    nsTransmitBuff = (GT_U8*)cpssOsCacheDmaMalloc(NS_TRANSMIT_BUFF_SIZE);

    nsInitialized = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvNetworkStackRxEvHandler function
* @endinternal
*
* @brief   Callback function for incoming Rx packet in the CPU
*
* @param[in] cookie                   -  value
* @param[in] devNum                   - Device number
* @param[in] evType                   - event type
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvNetworkStackRxEvHandler
(
  IN  GT_UINTPTR                cookie,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  IN  GT_U8                     devNum,
  IN  GT_U8                     queueIdx,
  IN  GT_U32                    numOfBuff,
  IN  GT_U8                     *packetBuffs[],
  IN  GT_U32                    buffLen[],
  IN  GT_VOID                   *rxParamsPtr
)
{
    GT_U8   buff[2000];
    GT_U32  ii, packetLen;
    GT_BOOL pass = GT_FALSE;
    NSLOCK();
#ifdef CHX_FAMILY
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* only packets which are forwarded will pass */
        CPSS_DXCH_NET_RX_PARAMS_STC *params =
                (CPSS_DXCH_NET_RX_PARAMS_STC*)rxParamsPtr;
        if (params->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
        {
            if (params->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_BRIDGED_PACKET_FORWARD_E
                || params->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_INTERVENTION_ARP_E)
            {
                pass = GT_TRUE;
            }
        }
        if (params->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_FORWARD_E)
        {
            pass = GT_TRUE;
        }
    }
#endif
    if (pass != GT_TRUE)
    {
        NSUNLOCK();
        return GT_OK;
    }
    /* skip if dstMac != nsMacAddr  &&  dstMac != ff:ff:ff:ff:ff:ff */
    if (    (cpssOsMemCmp(packetBuffs[0], nsMacAddr, 6) != 0) &&
            (cpssOsMemCmp(packetBuffs[0], nsBroadcast, 6) != 0))
    {
        NSUNLOCK();
        return GT_OK;
    }
    /* send packet to OS network stack */
    if (numOfBuff == 1)
    {
        /* one buffer, no copy */
        prvOsNetworkStackDeviceWrite(packetBuffs[0], buffLen[0]);
        NSUNLOCK();
        return GT_OK;
    }
    /* copy packet data */
    packetLen = 0;
    for(ii = 0; ii < numOfBuff; ii++)
    {
        if (packetLen + buffLen[ii] > sizeof(buff))
        {
            /* packet length is bigger than destination buffer size */
            cpssOsMemCpy(buff + packetLen, packetBuffs[ii], sizeof(buff) - packetLen);
            packetLen = sizeof(buff);
            break;
        }
        cpssOsMemCpy(buff + packetLen, packetBuffs[ii], buffLen[ii]);
        packetLen += buffLen[ii];
    }
    prvOsNetworkStackDeviceWrite(buff, packetLen);
    NSUNLOCK();
    return GT_OK;
}



/**
* @internal osNetworkStackDisconnect function
* @endinternal
*
* @brief   Disconnect PP from OS network stack
*/
GT_STATUS osNetworkStackDisconnect(void)
{
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    if (nsConnected != GT_TRUE)
    {
        NSUNLOCK();
        return GT_OK;
    }

#ifdef CHX_FAMILY
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(nsDevId)->devFamily))
    {
        cpssDxChBrgFdbMacEntryDelete(nsDevId, &(nsMacEntry.key));
        cpssDxChBrgVlanPortDelete(nsDevId, nsVlanId, CPSS_CPU_PORT_NUM_CNS);
    }
#endif

    rxEventHandlerRemoveCallback(
            nsDevId, RX_EV_HANDLER_QUEUE_ALL,
            RX_EV_HANDLER_TYPE_RX_E,
            prvNetworkStackRxEvHandler, 0);
    prvOsNetworkStackDeviceClose();

    /*TODO stop rx from tunnel task */


    nsConnected = GT_FALSE;
    NSUNLOCK();
    return GT_OK;
}

/**
* @internal osNetworkStackMacGet function
* @endinternal
*
* @brief   Query for MAC address of OS network interface
*/
GT_STATUS osNetworkStackMacGet(
  OUT GT_ETHERADDR *etherPtr
)
{
    GT_STATUS rc;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    rc = prvOsNetworkStackMacGet(etherPtr);
    if (rc != GT_OK)
    {
        NSUNLOCK();
        return rc;
    }
    cpssOsMemCpy(nsMacAddr, etherPtr->arEther, 6);

    if (nsConnected)
    {
#ifdef CHX_FAMILY
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(nsDevId)->devFamily))
        {
            cpssDxChBrgFdbMacEntryDelete(nsDevId, &(nsMacEntry.key));
        }
#endif
    }

#ifdef CHX_FAMILY
    cpssOsMemSet(&nsMacEntry, 0, sizeof(nsMacEntry));
    nsMacEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    nsMacEntry.key.key.macVlan.macAddr = *etherPtr;
    nsMacEntry.key.key.macVlan.vlanId = nsVlanId;
    nsMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    nsMacEntry.dstInterface.devPort.hwDevNum = nsHwDevNum;
    nsMacEntry.dstInterface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;
    nsMacEntry.age = GT_FALSE;
    nsMacEntry.isStatic = GT_TRUE;
    nsMacEntry.daCommand = CPSS_MAC_TABLE_FRWRD_E;
#endif

    if (nsConnected)
    {
#ifdef CHX_FAMILY
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(nsDevId)->devFamily))
        {
            rc = cpssDxChBrgFdbMacEntrySet(nsDevId, &nsMacEntry);
        }
#else
        rc = GT_NOT_IMPLEMENTED;
#endif
    }
    NSUNLOCK();
    return rc;
}

#ifdef CHX_FAMILY
/*******************************************************************************
* prvRxFromTunnelTaskDxCh
*
* DESCRIPTION:
*       Transmit packets from OS network stack to PP
*       DxCh only
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0
*
* COMMENTS:
*
*******************************************************************************/
static unsigned __TASKCONV prvRxFromTunnelTaskDxCh(void)
{
    GT_STATUS rc;
    GT_U32  buflen;
    GT_U32  vpt       = 0;
    GT_U32  cfiBit    = 0;
    GT_U32  vid       = 0;
    GT_U32  i;
    GT_BOOL packetIsTagged = GT_FALSE;
    GT_U8   *buffList[1];
    GT_U32  buffLenList[1];
    CPSS_DXCH_NET_TX_PARAMS_STC       dxChPcktParams;
    CPSS_DXCH_NET_DSA_FROM_CPU_STC    *fromCpuPtr = &dxChPcktParams.dsaParam.dsaInfo.fromCpu;

    /* fill in common packet TX params */
    cpssOsMemSet(&dxChPcktParams, 0, sizeof(dxChPcktParams));
    dxChPcktParams.cookie = NULL;
    dxChPcktParams.sdmaInfo.recalcCrc = GT_TRUE;
    dxChPcktParams.sdmaInfo.txQueue   = 7;
    dxChPcktParams.sdmaInfo.evReqHndl = 0;
    dxChPcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;
    dxChPcktParams.dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
    dxChPcktParams.dsaParam.commonParams.dropOnSource   = GT_FALSE;
    dxChPcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;
    dxChPcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    fromCpuPtr->dstInterface.type   = CPSS_INTERFACE_VID_E;
    fromCpuPtr->dstInterface.vlanId = nsVlanId;
    fromCpuPtr->tc                  = 7;
    fromCpuPtr->dp                  = CPSS_DP_GREEN_E;
    fromCpuPtr->egrFilterEn         = GT_FALSE;
    fromCpuPtr->cascadeControl      = GT_TRUE;
    fromCpuPtr->egrFilterRegistered = GT_FALSE;
    fromCpuPtr->srcId               = 0;
    fromCpuPtr->srcHwDev            = nsHwDevNum;
    fromCpuPtr->extDestInfo.multiDest.excludeInterface = GT_FALSE;
    fromCpuPtr->extDestInfo.multiDest.mirrorToAllCPUs = GT_FALSE;
    fromCpuPtr->extDestInfo.multiDest.srcIsTagged = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    while (1)
    {
        buflen = NS_TRANSMIT_BUFF_SIZE-4;
        rc = prvOsNetworkStackDeviceRead(nsTransmitBuff, &buflen);
        if (!nsConnected)
            break;
        if (rc != GT_OK)
            continue;
        /* add 4 byte crc */
        for (i = 0; i < 4; i++)
            nsTransmitBuff[buflen++] = 0;
        /* minimal length == 64, add padding */
        while (buflen < 64)
            nsTransmitBuff[buflen++] = 0;

        /* transmit buf */
        if ((nsTransmitBuff[12] == 0x81) && (nsTransmitBuff[13] == 0x00))
        {
            packetIsTagged = GT_TRUE;
            vpt    = (GT_U8) (nsTransmitBuff[14] >> 5);
            cfiBit = (GT_U8) ((nsTransmitBuff[14] >> 4) & 1);
            vid    = ((nsTransmitBuff[14] & 0xF) << 8) | nsTransmitBuff[15];
        }
        else
        {
            packetIsTagged = GT_FALSE;
            vpt    = 0;
            cfiBit = 0;
            vid    = nsVlanId;
        }

        dxChPcktParams.packetIsTagged = packetIsTagged;
        dxChPcktParams.dsaParam.commonParams.vpt = (GT_U8)vpt;
        dxChPcktParams.dsaParam.commonParams.cfiBit = (GT_U8)cfiBit;
        dxChPcktParams.dsaParam.commonParams.vid = (GT_U16)vid;

        buffList[0] = nsTransmitBuff;
        buffLenList[0] = buflen;

        rc = cpssDxChNetIfSdmaSyncTxPacketSend(nsDevId, &dxChPcktParams, buffList, buffLenList, 1);
        /*TODO: handle rc??? */
    }
    return 0;
}
#endif /* CHX_FAMILY */

/**
* @internal osNetworkStackMacSet function
* @endinternal
*
* @brief   Change MAC address of OS network interface, update FDB
*/
GT_STATUS osNetworkStackMacSet(
  IN  GT_ETHERADDR *etherPtr
)
{
    GT_STATUS rc;
    GT_ETHERADDR addr;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    if (!nsConnected)
    {
        NSUNLOCK();
        return GT_NOT_INITIALIZED;
    }
    rc = prvOsNetworkStackMacSet(etherPtr);
    if (rc == GT_OK)
    {
        rc = osNetworkStackMacGet(&addr);
    }
    NSUNLOCK();
    return rc;
}

/**
* @internal osNetworkStackConnect function
* @endinternal
*
* @brief   Connect device/vlan to OS network stack (unicast and broadcast)
*         Only one OS network interface can be confugured
* @param[in] devId                    - device ID
* @param[in] vlanId                   - Vlan ID (already configured)
* @param[in] etherAddr                - MAC address of interface
*                                       GT_STATUS
*/
GT_STATUS osNetworkStackConnect(
  IN  GT_U8         devId,
  IN  GT_U16        vlanId,
  IN  GT_ETHERADDR  *etherAddr
)
{
    GT_STATUS rc;
    GT_ETHERADDR myMac;
    CPSS_TASK tid;
    GT_VOIDFUNCPTR transmitFunc = NULL;

    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    if (nsConnected != GT_FALSE)
    {
        NSUNLOCK();
        return GT_ALREADY_EXIST;
    }
    if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devId))
    {
        NSUNLOCK();
        return GT_BAD_PARAM;
    }

    /* single pass loop for error handling */
    do {
    /* open tunnel */
    rc = prvOsNetworkStackDeviceOpen();
    if (rc != GT_OK)
        break;

    nsDevId = devId;
    nsVlanId = vlanId;
    extUtilHwDeviceNumberGet(devId, &nsHwDevNum);

    /* configure MAC address */
    rc = prvOsNetworkStackMacSet(etherAddr);
    if (rc != GT_OK)
        break;
    /* fill nsMacEntry */
    rc = osNetworkStackMacGet(&myMac);
    if (rc != GT_OK)
        break;

    /* configure Rx callback */
    rc = rxEventHandlerAddCallback(
            devId, RX_EV_HANDLER_QUEUE_ALL,
            RX_EV_HANDLER_TYPE_RX_E,
            prvNetworkStackRxEvHandler, 0);
    if (rc != GT_OK)
        break;

#ifdef CHX_FAMILY
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devId)->devFamily))
    {
        transmitFunc =  (GT_VOIDFUNCPTR)prvRxFromTunnelTaskDxCh;
        /* add CPU port to vlan */
        rc = cpssDxChBrgVlanMemberAdd(devId, vlanId,
                CPSS_CPU_PORT_NUM_CNS, GT_FALSE,
                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
        if (rc != GT_OK)
            break;

        rc = cpssDxChBrgFdbMacEntrySet(devId, &nsMacEntry);

        if (rc != GT_OK)
        {
            cpssDxChBrgVlanPortDelete(devId, vlanId, CPSS_CPU_PORT_NUM_CNS);
            break;
        }
    }
#else
    rc = GT_NOT_IMPLEMENTED;
#endif
    } while (0);

    if (rc != GT_OK)
    {
        rxEventHandlerRemoveCallback(
                devId, RX_EV_HANDLER_QUEUE_ALL,
                RX_EV_HANDLER_TYPE_RX_E,
                prvNetworkStackRxEvHandler, 0);
        prvOsNetworkStackDeviceClose();
        NSUNLOCK();
        return rc;
    }
    
    /* start rx from tunnel task */
    if (transmitFunc != NULL)
    {
        rc = cpssOsTaskCreate("nsTransmit",
                    5, /* prio */
                    32768, /* stack */
                    (unsigned (__TASKCONV *)(void*))transmitFunc,
                    NULL, /*arg*/
                    &tid);
    }
    else
    {
        rc = GT_NOT_SUPPORTED;
    }

    nsConnected = GT_TRUE;
    NSUNLOCK();
    return rc;
}

/**
* @internal osNetworkStackIfconfig function
* @endinternal
*
* @brief   Configure OS network interface
*
* @param[in] config                   - Configuration string
*                                       GT_STATUS
*
* @note Example:
*       osNetworkStackIfconfig("up inet 10.1.2.3 netmask 255.255.255.0")
*
*/
GT_STATUS osNetworkStackIfconfig(
  IN const char *config
)
{
    GT_STATUS rc;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    rc = prvOsNetworkStackIfconfig(config);
    NSUNLOCK();
    return rc;
}

