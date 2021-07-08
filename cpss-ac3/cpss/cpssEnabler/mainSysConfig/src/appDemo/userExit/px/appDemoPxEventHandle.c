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
* @file appDemoPxEventHandle.c
*
* @brief this library contains the implementation of the event handling functions
* for the PX device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* ---> this file is compiled only under PX_FAMILY <---
*
*
* @version   1
********************************************************************************
*/

#include <appDemo/userExit/px/appDemoPxEventHandle.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/ingress/cpssPxIngress.h>

#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>

/****************************************************************************
* Debug                                                                     *
****************************************************************************/

#define BUFF_LEN    20

#define MAX_REGISTER    5
extern GT_UINTPTR rxEventHanderHnd;

#define PX_APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF  7
#define PX_APP_DEMO_RX_BUFF_SIZE_DEF        800

/**
* @internal appDemoPxNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- PX function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*/
GT_STATUS appDemoPxNetRxPktHandle
(
    IN GT_UINTPTR                           evHandler,
    IN RX_EV_HANDLER_EV_TYPE_ENT            evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
)
{
    GT_STATUS                           rc;
    GT_U32                              numOfBuff = BUFF_LEN;
    GT_U8                               *packetBuffs[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];

    /* get the packet from the device */
    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssPxNetIfSdmaRxPacketGet(devNum, queueIdx,&numOfBuff,
                            packetBuffs,buffLenArr);
    }
    else
    {
        rc = GT_NOT_INITIALIZED;
    }

    if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    rxEventHandlerDoCallbacks(evHandler, evType,
            devNum,queueIdx,numOfBuff,
            packetBuffs,buffLenArr,NULL);

    /* now you need to free the buffers */
    if(PRV_CPSS_PX_PP_MAC(devNum)->netIf.allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
    {
        rc = cpssPxNetIfRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
    }
    else
    {
        /* move the first buffer pointer to the original place*/
        packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> PX_APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                  << PX_APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

        /* fix the last buffer's size*/
        buffLenArr[numOfBuff-1] = PX_APP_DEMO_RX_BUFF_SIZE_DEF;

        rc = cpssPxNetIfRxBufFreeWithSize(devNum,queueIdx,packetBuffs,buffLenArr,numOfBuff);
    }

    return rc;
}

/**
* @internal appDemoPxNetRxPktGet function
* @endinternal
*
* @brief   application routine to get the packets from the queue and
*         put it in msgQ. -- PX function
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] msgQId                   - Message queue Id.
*                                       None.
*/
GT_STATUS appDemoPxNetRxPktGet
(
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN CPSS_OS_MSGQ_ID  msgQId
)
{
    GT_STATUS                 rc;
    APP_DEMO_RX_PACKET_PARAMS rxParams;
    rxParams.numOfBuff = BUFF_LEN;
    cpssOsMutexLock(rxMutex);
    /* get the packet from the device */
    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssPxNetIfSdmaRxPacketGet(devNum, queueIdx,&(rxParams.numOfBuff),
                            rxParams.packetBuffs,rxParams.buffLenArr);
    }
    else
    {
        rc = GT_NOT_INITIALIZED;
    }

    cpssOsMutexUnlock(rxMutex);

    if (rc != GT_OK)
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    rxParams.devNum = devNum;
    rxParams.queue = queueIdx;
    /* put in msgQ */
    rc = cpssOsMsgQSend(msgQId,&rxParams,
                        sizeof(APP_DEMO_RX_PACKET_PARAMS),CPSS_OS_MSGQ_WAIT_FOREVER);
    return rc;
}

/**
* @internal appDemoPxNetRxPktTreat function
* @endinternal
*
* @brief   application routine to treat the packets . -- PX function
*
* @param[in] rxParamsPtr              - (pointer to) rx packet params
*                                       None.
*/
GT_STATUS appDemoPxNetRxPktTreat
(
    IN APP_DEMO_RX_PACKET_PARAMS  *rxParamsPtr
)
{
    GT_STATUS   rc;
    GT_U8       devNum ;
    if (rxParamsPtr == NULL)
    {
        /* debug */
        return GT_BAD_PTR;
    }

    devNum = rxParamsPtr->devNum;

    rxEventHandlerDoCallbacks(rxEventHanderHnd, RX_EV_HANDLER_TYPE_RX_E,
            rxParamsPtr->devNum,rxParamsPtr->queue,
            rxParamsPtr->numOfBuff,rxParamsPtr->packetBuffs,
            rxParamsPtr->buffLenArr,NULL);

    cpssOsMutexLock(rxMutex);
    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(PRV_CPSS_PX_PP_MAC(devNum)->netIf.allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssPxNetIfRxBufFree(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                        rxParamsPtr->numOfBuff);
        }
        else
        {

            /* move the first buffer pointer to the original place*/
            rxParamsPtr->packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(rxParamsPtr->packetBuffs[0])) >> PX_APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << PX_APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);
            /* fix the last buffer's size*/
            rxParamsPtr->buffLenArr[rxParamsPtr->numOfBuff-1] = PX_APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssPxNetIfRxBufFreeWithSize(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                                rxParamsPtr->buffLenArr,rxParamsPtr->numOfBuff);
        }
    }
    else
    {
        rc = GT_NOT_INITIALIZED;
    }
    cpssOsMutexUnlock(rxMutex);

    return rc;
}

/**
* @internal pipe_linkChange function
* @endinternal
*
* @brief   function to handle link change in sip5_20 .
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   pipe_linkChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum,
    IN GT_BOOL printLinkChangeEnabled
)
{
    GT_STATUS rc;
    GT_BOOL linkUp;
    GT_BOOL isPortInBiodirectionalMode;/*is port of a 'link down' in mode of 'Biodirectional' */
    GT_BOOL                     apEnable;
    CPSS_PX_PORT_AP_PARAMS_STC  apParams;

    /* we got valid physical port for our MAC number */
    rc = cpssPxPortLinkStatusGet(devNum, phyPortNum, &linkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortApPortConfigGet(devNum, phyPortNum, &apEnable, &apParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - cpssPxPortApPortConfigGet:rc=%d,portNum=%d\n",rc, phyPortNum);
        return rc;
    }

    if (apEnable)
    {
        CPSS_PX_DETAILED_PORT_MAP_STC   portMapShadow;
        GT_U32                          macNum;

        rc = cpssPxPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMapShadow);
        if (rc != GT_OK)
        {
           cpssOsPrintf("cpssPxPortPhysicalPortDetailedMapGet:error!devNum=%d,port=%d,rc=%d\n",
                    devNum, phyPortNum, rc);
           return rc;
        }

        macNum = portMapShadow.portMap.macNum;

        /* Yakov - TBD - code below uses prv functions - must replace by right CPSS API calls */
        if (PRV_CPSS_PORT_CG_E == PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType)
        {
            GT_U32  regAddr;
            GT_U32  data;

            /* AP state machine sends this interrupt on end of treat of port up/down,
                so I'm sure I get here right value */
            regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, macNum).CGMAConvertersResets;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = cpssDrvPpHwRegBitMaskRead(devNum, 0, regAddr, 0xffffffff, &data);
                 if (rc != GT_OK)
                 {
                    cpssOsPrintf("cpssDrvPpHwRegBitMaskRead: error, devNum=%d, port=%d, rc=%d\n",
                                    devNum, phyPortNum, rc);
                    return rc;
                 }
            }

            if (((data>>26)&0x1) == 0)
            {
                /* update port DB with the default port type value in order to prevent using CG MAC
                   when it is disabled  */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;

                /* init CG port register database */
                rc = prvCpssPxCgPortDbInvalidate(devNum, macNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("pipe_linkChange:prvCpssPxCgPortDbInvalidate:rc=%d,portNum=%d,GT_TRUE\n",rc, phyPortNum);
                    return rc;
                }
            }
        }
        else
        {
            CPSS_PORT_SPEED_ENT speed;
            CPSS_PORT_INTERFACE_MODE_ENT ifMode;
            GT_BOOL             isCgUnitInUse;

            /* Check current mode */
            rc = cpssPxPortSpeedGet(devNum, phyPortNum, &speed);
            if(rc != GT_OK)
            {
                cpssOsPrintf("pipe_linkChange:cpssPxPortSpeedGet:rc=%d,portNum=%d\n", rc, phyPortNum);
                return rc;
            }
            rc = cpssPxPortInterfaceModeGet(devNum, phyPortNum, &ifMode);
            if(rc != GT_OK)
            {
                cpssOsPrintf("pipe_linkChange:cpssPxPortInterfaceModeGet:rc=%d,portNum=%d\n", rc, phyPortNum);
                return rc;
            }

            rc = prvCpssPxIsCgUnitInUse(devNum, phyPortNum, ifMode, speed, &isCgUnitInUse);
            if(rc != GT_OK)
            {
                cpssOsPrintf("pipe_linkChange:prvCpssPxIsCgUnitInUse:rc=%d,portNum=%d,ifMode=%d,speed=%d\n",
                                rc, phyPortNum, ifMode, speed);
                return rc;
            }

            if(isCgUnitInUse)
            {/* must restore CG DB, because if we here it means link was restored after fail */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;

                /* init CG port register database */
                rc = prvCpssPxCgPortDbInvalidate(devNum, macNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("pipe_linkChange:prvCpssPxCgPortDbInvalidate:rc=%d,portNum=%d,GT_FALSE\n", rc, phyPortNum);
                    return rc;
                }
            }
        }
    }

    if(printLinkChangeEnabled)
    {
        cpssOsPrintSync("LINK %s : devNum[%ld] (physical port[%ld]) \n",
                            (linkUp == GT_FALSE) ? "DOWN" : "UP",
                                        (GT_U32)devNum, phyPortNum);
    }

    isPortInBiodirectionalMode = GT_FALSE;
#if 0 /* no such function in PX (yet ?!)*/
    if(linkUp == GT_FALSE)
    {
        rc = cpssDxChVntOamPortUnidirectionalEnableGet(devNum,phyPortNum,
            &isPortInBiodirectionalMode);
#ifdef GM_USED
        isPortInBiodirectionalMode = GT_FALSE;
        rc = GT_OK;
#endif /*GM_USED*/

        if (rc != GT_OK)
        {
            DBG_LOG(("cpssDxChVntOamPortUnidirectionalEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, phyPortNum, rc, 4, 5, 6));

            /* 100G (CG) : the port not supports the feature */
        }
    }
#endif /*0*/

    if(linkUp == GT_FALSE && isPortInBiodirectionalMode == GT_TRUE)
    {
        if(printLinkChangeEnabled)
        {
            cpssOsPrintSync("MAC Bidirectional enabled : even though the devNum[%ld] (physical port[%ld]) is DOWN the port can egress packets \n",
                                (GT_U32)devNum, phyPortNum);
        }

        /* !!! do not modify the filter !!! keep it as 'link up' */
        return GT_OK;
    }

    /* need to set the ingress link status filter according to new state of the
       port */
    rc = cpssPxIngressPortTargetEnableSet(devNum, phyPortNum,linkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}



