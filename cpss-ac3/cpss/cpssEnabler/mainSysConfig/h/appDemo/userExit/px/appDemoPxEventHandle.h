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
* @file appDemoPxEventHandle.h
*
* @brief this library contains the implementation of the event handling functions
* for the PX device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/userExit/userEventHandler.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>

/**
* @internal appDemoPxNetRxPacketCbRegister function
* @endinternal
*
* @brief   register a CB function to be called on every RX packet to CPU
*         NOTE: function is implemented in the 'AppDemo' (application)
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*/
GT_STATUS appDemoPxNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);


/**
* @internal appDemoPxNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- DXCH function
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
);

/**
* @internal appDemoPxNetRxPktGet function
* @endinternal
*
* @brief   application routine to get the packets from the queue and
*         put it in msgQ. -- DXCH function
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
);

/**
* @internal appDemoPxNetRxPktTreat function
* @endinternal
*
* @brief   application routine to treat the packets . -- DXCH function
*
* @param[in] rxParamsPtr              - (pointer to) rx paacket params
*                                       None.
*/
GT_STATUS appDemoPxNetRxPktTreat
(
    IN APP_DEMO_RX_PACKET_PARAMS  *rxParamsPtr
);

/**
* @internal appDemoPxNetPtpOverUdpWaCb function
* @endinternal
*
* @brief   ?
*
* @note   APPLICABLE DEVICES:      ALL Devices
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (Pointer to)information parameters of received packets
*
* @retval GT_OK                    - no error
*/
GT_STATUS appDemoPxNetPtpOverUdpWaCb
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               *packetBuffs[],
    IN GT_U32                               buffLen[],
    IN void                                 *rxParamsPtr
);


