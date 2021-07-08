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
* @file prvTgfPortIfModeSpeed.c
*
* @brief Port interface mode and speed configuration testing
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <port/prvTgfPortFWS.h>
#include <common/tgfVntGen.h>

/*
 * Typedef: struct PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STC
 *
 * Description:
 *        This structure contains port attributes which could be changed by test
 *        and must be restored afterwards
 *
 *
 * Fields:
 *        ifMode                - original port interface
 *        speed                 - original speed of port
 *        inbandAutoNeg         - original inband auto-neg.
 *        inbandAutoNegBypass   - original inband auto-neg. bypass state
 *        speedAutoNeg          - original speed auto-neg.
 *        duplexAutoNeg         - original duplex auto-neg.
 *        fcAnState             - original flow ctrl auto-neg.
 *        pauseAdvertise        - original pause advertise state
 *        fcStatus              - original flow control status
 *        duplex                - original duplex state
 *
 * Comments:
 *
 */
typedef struct PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STCT
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    GT_BOOL                         inbandAutoNeg;
    GT_BOOL                         inbandAutoNegBypass;
    GT_BOOL                         speedAutoNeg;
    GT_BOOL                         duplexAutoNeg;
    GT_BOOL                         fcAnState;
    GT_BOOL                         pauseAdvertise;
    CPSS_PORT_FLOW_CONTROL_ENT      fcStatus;
    CPSS_PORT_DUPLEX_ENT            duplex;

} PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STC;



/* L2 part of packet LSB of MAC SA is changed in run time */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet 1 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET 1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

#ifndef  ASIC_SIMULATION
#define PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(delay) cpssOsTimerWkAfter(delay)
#else
#define PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(delay)
#endif


/**
* @internal prvTgfPortIfModeSpeedVlanConfig function
* @endinternal
*
* @brief   Create VLAN loops and add FDB entries.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - number of port
*                                       None
*/
static GT_STATUS prvTgfPortIfModeSpeedVlanConfig
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   st;             /* execution status */
    GT_U16      vlanId;         /* vlan ID */
    GT_U32       portsArray[1];  /* array of ports in vlan */
    GT_U8       devArray[1];    /* array of devices in vlan */

    /* make unique vlan for every port, skip vlan #0 and #1 */
    vlanId = (GT_U16)(portNum+2);
    st = prvTgfBrgVlanPortVidSet(devNum, portNum, vlanId);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("prvTgfBrgVlanPortVidSet FAILED:st-%d,dev-%d,port-%d,vlan-%d\n",
                                                        st, devNum, portNum, vlanId);
        return st;
    }

    portsArray[0] = portNum;
    devArray[0]   = devNum;
    st = prvTgfBrgDefVlanEntryWithPortsSet(vlanId, portsArray, devArray, NULL, 1);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfBrgDefVlanEntryWithPortsSet FAILED:st-%d,vlan-%d\n",
                                                        st, vlanId);
        return st;
    }

    return GT_OK;
}

/*******************************************************************************
* prvTgfPortIfModeSpeedPortConfig
*
* DESCRIPTION:
*       Configure ports interface and speed and loopback mode
*
* INPUTS:
*       devNum  - device number
*       portNum - port number
*       ifMode  - interface mode to configure on port
*       speed   - speed to configure on port
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
#ifdef CHX_FAMILY
extern GT_STATUS gtAppDemoLionPortModeSpeedSet
(
    GT_U8 devNum,
    GT_U8 portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT speed
);
#if 0
extern GT_BOOL lioncubConnected
(
    GT_U8  dev,
    GT_U32 portGroup
);
extern GT_STATUS lioncubRevisionGet
(
    GT_U8  dev,
    GT_U32 portGroup,
    GT_U32 *rev
);
#endif
#endif

/**
* @internal prvTgfPortIfModeSpeedPortConfig function
* @endinternal
*
* @brief   Set port configuration speed and mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - number of port
* @param[in] ifMode                   - mode to set
* @param[in] speed                    -  to set
*                                       None
*/
GT_STATUS prvTgfPortIfModeSpeedPortConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS st = GT_OK;

#ifdef CHX_FAMILY

    /* must disable here force link up to cause port to accept new auto-neg.
        configurations, otherwise traffic not pass for SGMII 1G, 2.5G and
        1000BaseX */
    st = cpssDxChPortForceLinkPassEnableSet(devNum, portNum, GT_FALSE);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortForceLinkPassEnableSet FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE\n",
                            st, devNum, portNum);
        return st;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        return GT_NOT_SUPPORTED;
    }
    else
    {
        CPSS_PORTS_BMP_STC portsBmp;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            /* shutdown port */
            st = cpssDxChPortModeSpeedSet(devNum,&portsBmp,GT_FALSE,ifMode,speed);
            if (GT_OK != st)
            {
                PRV_UTF_LOG4_MAC("cpssDxChPortModeSpeedSet FAILED:st-%d,devNum-%d,GT_TRUE,ifMode-%d,speed-%d\n",
                                                            st, devNum, ifMode, speed);
                return st;
            }
        }
        else
        {
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    st = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG3_MAC("cpssDxChPortSpeedAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE\n",
                                                                    st, devNum, portNum);
                        return st;
                    }
                    st = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG3_MAC("cpssDxChPortDuplexAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE\n",
                                                                    st, devNum, portNum);
                        return st;
                    }
                    st = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum,
                                                                    GT_FALSE, GT_FALSE);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG3_MAC("cpssDxChPortFlowCntrlAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE,GT_FALSE\n",
                                                                    st, devNum, portNum);
                        return st;
                    }
                    break;
                default:
                    break;
            }

            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);
            st = cpssDxChPortModeSpeedSet(devNum,&portsBmp,GT_TRUE,ifMode,speed);
            if (GT_OK != st)
            {
                PRV_UTF_LOG4_MAC("cpssDxChPortModeSpeedSet FAILED:st-%d,devNum-%d,GT_TRUE,ifMode-%d,speed-%d\n",
                                                            st, devNum, ifMode, speed);
                return st;
            }

            st = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
            if (GT_OK != st)
            {
                PRV_UTF_LOG3_MAC("cpssDxChPortEnableSet FAILED:st-%d,devNum-%d,portNum-%d\n",
                                    st, devNum, portNum);
                return st;
            }
        }
    }

    if((speed <= CPSS_PORT_SPEED_1000_E) && (ifMode != CPSS_PORT_INTERFACE_MODE_1000BASE_X_E))
    {
        st = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_FALSE);
        if (GT_OK != st)
        {
            PRV_UTF_LOG3_MAC("cpssDxChPortInbandAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE\n",
                                st, devNum, portNum);
            return st;
        }
    }
#endif

    return st;
}

/**
* @internal prvTgfPortIfModeSpeedConfigGet function
* @endinternal
*
* @brief   Restore port configuration previous to test
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - number of port
*
* @param[out] originalAttributesPtr    - current attributes of port
*                                       None
*/
static GT_STATUS prvTgfPortIfModeSpeedConfigGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
        OUT PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STC    *originalAttributesPtr
)
{
    GT_STATUS st = GT_OK; /* execution status */

#ifdef CHX_FAMILY
    /* get interface mode */
    st = cpssDxChPortInterfaceModeGet(devNum, portNum,
                                    &(originalAttributesPtr->ifMode));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortInterfaceModeGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    /* Get port speed */
    st = cpssDxChPortSpeedGet(devNum, portNum, &(originalAttributesPtr->speed));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortSpeedGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortInbandAutoNegEnableGet(devNum, portNum,
                                            &(originalAttributesPtr->inbandAutoNeg));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortInbandAutoNegEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortInBandAutoNegBypassEnableGet(devNum, portNum,
                                                    &(originalAttributesPtr->inbandAutoNegBypass));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortInBandAutoNegBypassEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortSpeedAutoNegEnableGet(devNum, portNum,
                                            &(originalAttributesPtr->speedAutoNeg));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortSpeedAutoNegEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortDuplexAutoNegEnableGet(devNum, portNum,
                                            &(originalAttributesPtr->duplexAutoNeg));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortDuplexAutoNegEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortDuplexModeGet(devNum, portNum,&(originalAttributesPtr->duplex));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortDuplexModeGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortFlowCntrlAutoNegEnableGet(devNum, portNum,
                                                &(originalAttributesPtr->fcAnState),
                                                &(originalAttributesPtr->pauseAdvertise));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortFlowCntrlAutoNegEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    st = cpssDxChPortFlowControlEnableGet(devNum, portNum,
                                            &(originalAttributesPtr->fcStatus));
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortFlowControlEnableGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

#endif

    return st;
}

/**
* @internal prvTgfPortIfModeSpeedConfigRestore function
* @endinternal
*
* @brief   Restore port configuration previous to test
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - number of port
* @param[in] originalAttributesPtr    - original attributes of port for rollback
*                                       None
*/
static GT_STATUS prvTgfPortIfModeSpeedConfigRestore
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  const PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STC  *originalAttributesPtr
)
{
    GT_STATUS   st;             /* execution status */
    GT_U16      vlanId;         /* vlan ID */

    /* invalidate vlan entry (and reset vlan entry) */
    vlanId = (GT_U16)(portNum+2);
    st = prvTgfBrgVlanEntryInvalidate(vlanId);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfBrgVlanEntryInvalidate FAILED:st-%d,vlan-%d\n",
                                                        st, vlanId);
        return st;
    }

    /* restore default VLAN */
    st = prvTgfBrgVlanPortVidSet(devNum, portNum, 1);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfBrgVlanPortVidSet FAILED:st-%d,devNum-%d,portNum-%d,vlan-1\n",
                                                        st, devNum, portNum);
        return st;
    }

    /* configure default mode */
    st = prvTgfTxModeSetupEth(devNum, portNum,
                                PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                0);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfTxModeSetupEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    /* ----- rollback attributes of port to pre-test state ----- */

    st = prvTgfPortIfModeSpeedPortConfig(devNum, portNum,
                                            originalAttributesPtr->ifMode,
                                            originalAttributesPtr->speed);
    if (GT_OK != st)
    {
        PRV_UTF_LOG5_MAC("prvTgfPortIfModeSpeedPortConfig FAILED:st-%d,devNum-%d,portNum-%d,\
                            ifMode-%d,speed-%d\n", st, devNum, portNum,
                            originalAttributesPtr->ifMode,
                            originalAttributesPtr->speed);
        return st;
    }

#ifdef CHX_FAMILY
    st = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum,
                                            originalAttributesPtr->speedAutoNeg);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortSpeedAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,speedAutoNeg-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->speedAutoNeg);
        return st;
    }

    st = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum,
                                            originalAttributesPtr->duplexAutoNeg);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortDuplexAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,duplexAutoNeg-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->duplexAutoNeg);
        return st;
    }

    st = cpssDxChPortDuplexModeSet(devNum,portNum,originalAttributesPtr->duplex);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortDuplexModeSet FAILED:st-%d,devNum-%d,portNum-%d,duplex-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->duplex);
        return st;
    }

    st = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum,
                                                originalAttributesPtr->fcAnState,
                                                originalAttributesPtr->pauseAdvertise);
    if (GT_OK != st)
    {
        PRV_UTF_LOG5_MAC("cpssDxChPortFlowCntrlAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,fcAnState-%d,pauseAdvertise-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->fcAnState,
                            originalAttributesPtr->pauseAdvertise);
        return st;
    }

    st = cpssDxChPortFlowControlEnableSet(devNum, portNum,
                                            originalAttributesPtr->fcStatus);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortFlowControlEnableSet FAILED:st-%d,devNum-%d,portNum-%d,fcStatus-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->fcStatus);
        return st;
    }

    st = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum,
                                                    originalAttributesPtr->inbandAutoNegBypass);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortInBandAutoNegBypassEnableSet FAILED:st-%d,devNum-%d,portNum-%d,inbandAutoNegBypass-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->inbandAutoNegBypass);
        return st;
    }

    st = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum,
                                            originalAttributesPtr->inbandAutoNeg);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortInbandAutoNegEnableSet FAILED:st-%d,devNum-%d,portNum-%d,inbandAutoNeg-%d\n",
                            st, devNum, portNum,
                            originalAttributesPtr->inbandAutoNeg);
        return st;
    }

    st = cpssDxChPortInbandAutoNegRestart(devNum,portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortInbandAutoNegRestart FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(300);

    #endif

    return GT_OK;
}

/**
* @internal prvTgfPortIfModeSpeedPortLoopbackEnable function
* @endinternal
*
* @brief   Enable/disable serdes loopback on port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  loopback
*                                      GT_FALSE - disable loopback
*                                       None
*
* @note Pay attention - global variable prvTgfLoopbackUnit changed here
*
*/
static GT_STATUS prvTgfPortIfModeSpeedPortLoopbackEnable
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS               st;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum = portNum;
#ifndef ASIC_SIMULATION
    /* if loopback needed define it on serdes for this test */
    prvTgfLoopbackUnit = enable ? PRV_TGF_LOOPBACK_UNIT_SERDES_E
            /* otherwise back to default - loopback in MAC */
                                    : PRV_TGF_LOOPBACK_UNIT_MAC_E;
#else
    prvTgfLoopbackUnit = PRV_TGF_LOOPBACK_UNIT_MAC_E;
#endif

    st = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, enable);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("tgfTrafficGeneratorPortLoopbackModeEnableSet FAILED:st-%d,enable-%d\n",
                                                    st, enable);
    }

    return st;
}

/**
* @internal prvTgfPortIfModeSpeedTrafficGenerate function
* @endinternal
*
* @brief   Configure, start, check, stop traffic to port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
static GT_STATUS prvTgfPortIfModeSpeedTrafficGenerate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS   st;
    GT_U32      txRate, /* actual TX rate on port */
                wsRate, /* theoretically expected WS rate */
                percent;/* difference persentage between expected and real rates */
    TGF_PACKET_STC  packetInfo; /* packet info */
    GT_BOOL     rateOk; /* is real rate correct */

    /* reset counters */
    st = prvTgfResetCountersEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfResetCountersEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* setup Packet */
    packetInfo = prvTgfPacket1Info;
    st = prvTgfSetTxSetupEth(devNum, &packetInfo, 1, 0, NULL);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth FAILED:st-%d,devNum-%d\n",
                                                    st, devNum);
        return st;
    }

    /* configure WS mode */
    st = prvTgfTxModeSetupEth(devNum, portNum,
                              PRV_TGF_TX_MODE_CONTINUOUS_E,
                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                              0);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfSetTxSetupEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* send Packet from port portNum */
    st = prvTgfStartTransmitingEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfStartTransmitingEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* wait to get stable traffic */
    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(10);

    /* ---- check that egress port get WS traffic --------- */
    /* get port's actual rate */
    prvTgfCommonPortTxRateGet(PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS, 1000, &txRate);
    if (0 == txRate)
    {
        PRV_UTF_LOG1_MAC(" wrong zero actual TX rate, portNum-%d\n", portNum);
        prvTgfStopTransmitingEth(devNum, portNum);
        return GT_FAIL;
    }

    PRV_UTF_LOG1_MAC("prvTgfPortIfModeSpeedTrafficGenerate: port actual rate %d\n", txRate);

    /* get port's theoretical wire-speed rate */
    wsRate = prvTgfCommonPortWsRateGet(PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS,
                                            PRV_TGF_PACKET_CRC_LEN_CNS);
    if (0 == wsRate)
    {
        PRV_UTF_LOG1_MAC(" wrong zero expected TX rate, portNum-%d\n", portNum);
        prvTgfStopTransmitingEth(devNum, portNum);
        return GT_FAIL;
    }
    PRV_UTF_LOG1_MAC("prvTgfPortIfModeSpeedTrafficGenerate: port expected rate %d\n", wsRate);

    /* calculate difference between ports WS rate and input one */
    percent = prvTgfCommonDiffInPercentCalc(wsRate, txRate);
    rateOk = (percent <= 2) ? GT_TRUE : GT_FALSE;

    /* ----- stop send Packet from port portNum -------- */
    st = prvTgfStopTransmitingEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfStopTransmitingEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    if (GT_TRUE != rateOk)
    {
        PRV_UTF_LOG4_MAC(" wrong rate:portNum-%d,percent-%d,expected-%d,actual-%d\n",
                            portNum, percent, wsRate, txRate);
        return GT_FAIL;
    }

    /* wait some time to guaranty that traffic stopped */
    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(10);

    return GT_OK;
}

/**
* @internal prvTgfPortIfModeSpeedTrafficTest function
* @endinternal
*
* @brief   Send traffic to specified port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode to configure on port
* @param[in] speed                    -  to configure on port
*                                       None
*/
static GT_STATUS prvTgfPortIfModeSpeedTrafficTest
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   st, testResult;     /* status of command */
    GT_BOOL     linkUp = GT_FALSE; /* port link status */

    PRV_UTF_LOG4_MAC("\nprvTgfPortIfModeSpeedTrafficTest:devNum=%d,portNum=%d,ifMode=%d,speed=%d\n",
                        devNum, portNum, ifMode, speed);

    /* Enable to use any port for test.
       Place it here to enable engage this function directly from command line
       for debug */
    prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS+1;

    /* set port to check rates on (rateGet functions accept index in global
        prvTgfPortsArray instead of real port number)
        Place it here to enable engage this function directly from command line
        for debug */
    prvTgfPortsArray[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS] = (GT_U8)portNum;

    st = prvTgfPortIfModeSpeedPortConfig(devNum, portNum, ifMode, speed);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("prvTgfPortIfModeSpeedPortConfig FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                    st, portNum, ifMode, speed);
        return st;
    }

    /* to be on safe side - wait interface configuration finished */
    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(20);

    /* must be after prvTgfPortIfModeSpeedPortConfig, because number of lanes
        where to define loopback could be calculated after interface/speed
        configured */
    st = prvTgfPortIfModeSpeedPortLoopbackEnable(devNum, portNum, GT_TRUE);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfPortIfModeSpeedPortLoopbackEnable FAILED:st-%d,devNum-%d,portNum-%d,GT_TRUE\n",
                                                    st, devNum, portNum);
        return st;
    }

#ifdef CHX_FAMILY
    if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily) &&
        (CPSS_PORT_SPEED_40000_E == speed))
    {/* no analog loopback on 40G port serdeses */
        st = cpssDxChPortSerdesLoopbackModeSet(devNum, portNum,
                                                 0xFF, /* exact bitmap not important here */
                                                 CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E);
        if (GT_OK != st)
        {
            PRV_UTF_LOG3_MAC("cpssDxChPortSerdesLoopbackModeSet(DIGITAL) FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                        st, devNum, portNum);
            return st;
        }
    }
#endif

    /* allow the port 'get UP' before we check link */
    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(20);

    /* get link status */
#ifdef CHX_FAMILY
    st = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("cpssDxChPortLinkStatusGet FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }
#endif
    if(GT_FALSE == linkUp)
    {
        PRV_UTF_LOG0_MAC("FAIL:no link after serdes loopback enabled.\n");
        (GT_VOID)prvTgfPortIfModeSpeedPortLoopbackEnable(devNum, portNum, GT_FALSE);
        return GT_FAIL;
    }

    testResult = prvTgfPortIfModeSpeedTrafficGenerate(devNum,portNum);
    if (GT_OK != testResult)
    {
        PRV_UTF_LOG3_MAC("prvTgfPortIfModeSpeedTrafficGenerate FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
    }

    /* clear MAC's learned from tests from FDB */
    st = prvTgfBrgFdbFlush(GT_FALSE);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("prvTgfBrgFdbFlush(GT_FALSE) FAILED:st-%d\n", st);
        return st;
    }

    st = prvTgfPortIfModeSpeedPortLoopbackEnable(devNum, portNum, GT_FALSE);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfPortIfModeSpeedPortLoopbackEnable FAILED:st-%d,devNum-%d,portNum-%d,GT_FALSE\n",
                                                    st, devNum, portNum);
        return st;
    }
    /* to be on safe side - wait loopback disabled  */
    PRV_TGF_PORT_IFMODE_TEST_SLEEP_MAC(20);

    PRV_UTF_LOG2_MAC("***** ifMode=%d,speed=%d ", ifMode, speed);
    if (GT_OK == testResult)
    {
        PRV_UTF_LOG0_MAC("passed OK *****\n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("FAILED *****\n");
    }

    return testResult;
}

/**
* @internal prvTgfPortIfModeSpeedPortModeTest function
* @endinternal
*
* @brief   Run test per port per (ifMode;speed) pair
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*                                       None
*/
GT_STATUS prvTgfPortIfModeSpeedPortModeTest
(
    GT_U8                                   devNum,
    GT_PHYSICAL_PORT_NUM                       portNum,
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    CPSS_PORT_SPEED_ENT                         speed
)
{
    GT_STATUS   st;         /* status of command */
    GT_BOOL     supported;  /* is this ifMode/speed supported by port */

#ifdef CHX_FAMILY
    st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum, ifMode,
                                                speed, &supported);
    if (GT_OK != st)
    {
        PRV_UTF_LOG4_MAC("cpssDxChPortInterfaceSpeedSupportGet FAILED:%d,%d,%d,%d\n",
                            devNum, portNum, ifMode, speed);
        return st;
    }

    if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily) &&
        (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode))
    {/* local XGMII mode supposed to be used with MAC loopback and could not be
        tested in this scope */
        return GT_OK;
    }
#else
    supported = GT_FALSE;
#endif

    if(supported)
    {
        utfPrintKeepAlive();
        st = prvTgfPortIfModeSpeedTrafficTest(devNum,portNum,ifMode,speed);
        if (GT_OK != st)
        {
            PRV_UTF_LOG4_MAC("prvTgfPortIfModeSpeedTrafficTest FAILED:%d,%d,%d,%d\n",
                                devNum, portNum, ifMode, speed);
            return st;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfPortIfModeSpeedPortTest function
* @endinternal
*
* @brief   Run test per port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
GT_STATUS prvTgfPortIfModeSpeedPortTest
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS               st;             /* status of command */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* interfaces iterator */
    CPSS_PORT_SPEED_ENT     speed;          /* speeds loop iterator */
    PRV_CPSS_PORT_TYPE_ENT  portType;       /* MAC unit/type used by port */
    PRV_TGF_PORT_ORIGINAL_ATTRIBUTES_STC originalAttribs;

    st = prvUtfPortTypeGet(devNum, (UTF_PHYSICAL_PORT_NUM)portNum, &portType);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvUtfPortTypeGet FAILED:%d,%d\n",
                            devNum, portNum);
        return st;
    }

    if (PRV_CPSS_PORT_FE_E == portType)
    {
        PRV_UTF_LOG2_MAC("prvTgfPortIfModeSpeedPortTest skip FE port (%d;%d)\n",
                            devNum, portNum);
        return GT_OK;
    }

    st = prvTgfPortIfModeSpeedVlanConfig(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfPortIfModeSpeedVlanConfig FAILED:%d,%d\n",
                            devNum, portNum);
        return st;
    }

    st = prvTgfPortIfModeSpeedConfigGet(devNum,portNum,&originalAttribs);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfPortIfModeSpeedConfigGet FAILED:%d,%d\n",
                            devNum, portNum);
        return st;
    }

    for(ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
        ifMode < CPSS_PORT_INTERFACE_MODE_NA_E; ifMode++)
    {
        for(speed = CPSS_PORT_SPEED_10_E; speed < CPSS_PORT_SPEED_NA_E; speed++)
        {
            st = prvTgfPortIfModeSpeedPortModeTest(devNum,portNum,ifMode,speed);
    /* print error message if bug */
            if (GT_FALSE == utfEqualVerify(st, GT_OK, __LINE__, __FILE__))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "prvTgfPortIfModeSpeedPortModeTest FAILED:%d,%d,%d,%d\n",
                              devNum, portNum,ifMode,speed);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }

        }/* loop over speeds */

    } /* loop over interfaces */

    st = prvTgfPortIfModeSpeedConfigRestore(devNum, portNum, &originalAttribs);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfPortIfModeSpeedConfigRestore FAILED:%d,%d\n",
                            devNum, portNum);
        return st;
    }

    return st;
}

/**
* @internal prvTgfPortIfModeSpeed function
* @endinternal
*
* @brief   Run test over all ports and supported (interface;speed) pairs
*/
GT_VOID prvTgfPortIfModeSpeed
(
    GT_VOID
)
{
    GT_STATUS   st;     /* status of command */
    GT_U8       devNum; /* device number */
    UTF_PHYSICAL_PORT_NUM       portNum; /* port number */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
            devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E
            | UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E | UTF_PUMA2_E);

        st = prvTgfPortFcHolSysModeSet(devNum,PRV_TGF_PORT_PORT_HOL_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                        "prvTgfPortFcHolSysModeSet:dev-%d,fcMode-%d",
                                        devNum, PRV_TGF_PORT_PORT_HOL_E);

        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* For all active devices go over all available physical (not CPU) ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            st = prvTgfPortIfModeSpeedPortTest(devNum,portNum);
            /* print error message if bug */
            if (GT_FALSE == utfEqualVerify(st, GT_OK, __LINE__, __FILE__))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "prvTgfPortIfModeSpeedPortModeTest FAILED:%d,%d\n",
                              devNum, portNum);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }
        } /* loop over ports */

        /* restore FC mode */
        st = prvTgfPortFcHolSysModeSet(devNum, PRV_TGF_PORT_PORT_FC_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfPortFcHolSysModeSet: %d, \
                                        PRV_TGF_PORT_PORT_FC_E", devNum);

CPSS_TBD_BOOKMARK /* restore original ifMode/speed of all ports of device ? */
    } /* loop over devices */

    /* restore default value of prvTgfPortsNum */
    prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;

    return;
}



static GT_VOID prvTgfPortExtendedModeTestSendTrafic
(
    GT_U8                 dev,
    GT_PHYSICAL_PORT_NUM  port,
    GT_BOOL               isBadPacketCounterExpected
)
{
    GT_STATUS                       st;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          counterExpectedValue;

    /* reset counters */
    st = prvTgfResetCountersEth(dev, port);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfResetCountersEth FAILED:st-%d,dev-%d,port-%d\n", st, dev, port);
        return;
    }

    /* setup Packet */
    st = prvTgfSetTxSetupEth(dev, &prvTgfPacket1Info, 1, 0, NULL);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth FAILED:st-%d,dev-%d\n",
                                                    st, dev);
        return;
    }

    /* send Packet from port */
    st = prvTgfStartTransmitingEth(dev, port);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfStartTransmitingEth FAILED:st-%d,dev-%d,port-%d\n",
                                                    st, dev, port);
        return;
    }

    PRV_UTF_LOG0_MAC("\n\n");
    st = prvTgfReadPortCountersEth(dev, port, GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfReadPortCountersEth: port=%d\n", port);

    counterExpectedValue = (GT_TRUE == isBadPacketCounterExpected) ? 1 : 0;

    if((50*counterExpectedValue) != portCntrs.badOctetsRcv.l[0])
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(counterExpectedValue, portCntrs.badPktsRcv.l[0], 
                                        "TEST FAILURE for port=%d\n", port);
    }
}


/**
* @internal prvTgfPortExtendedModeTest function
* @endinternal
*
* @brief   Run test extended port
*/
GT_VOID prvTgfPortExtendedModeTest
(
    GT_PHYSICAL_PORT_NUM  port
)
{
    GT_STATUS    st = GT_OK;
    GT_U8        dev = 0;
    GT_BOOL      enableGet;
    GT_U32       mruGet;
    

    /* Save MAC == PORT extended mode config */
    st = cpssDxChPortExtendedModeEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortExtendedModeEnableGet: dev = %d, port = %d", dev, port);

    /* Save MRU value */
    st = cpssDxChPortMruGet(dev, port, &mruGet);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortMruGet: dev = %d, port = %d", dev, port);

    /* Set MAC == PORT MRU value less then packet size */
    st = cpssDxChPortMruSet(dev, port, 50);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortMruSet: dev = %d, port = %d", dev, port);

    /* Check the trafic - MAC == PORT  expect bad octets counter to be set */
    prvTgfPortExtendedModeTestSendTrafic(dev, port, GT_TRUE);


    /* Enable extended mode on port:  MAC != PORT */
    st = cpssDxChPortExtendedModeEnableSet(dev, port, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortExtendedModeEnableSet: dev = %d, port = %d", dev, port);

    /* Check the trafic - MAC != PORT expect NO bad packet octets counter to be set */
    prvTgfPortExtendedModeTestSendTrafic(dev, port, GT_FALSE);

    /* Restore MAC == PORT extended mode config */
    st = cpssDxChPortExtendedModeEnableSet(dev, port, enableGet);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortExtendedModeEnableSet: dev = %d, port = %d", dev, port);

    /* Restore MRU value */
    st = cpssDxChPortMruSet(dev, port, mruGet);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
           "cpssDxChPortMruGet: dev = %d, port = %d", dev, port);
}


