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
* @file prvTgfBrgRateLimitMcastReg.c
*
* @brief Bridge Generic Port Rate Limit Multicast Registered packets UT.
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <bridge/prvTgfBrgGen.h>
#include <bridge/prvTgfBrgRateLimitMcastReg.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  2

/* VDIX to send traffic to */
#define PRV_TGF_SEND_VIDX_CNS   0xF

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 4;

/* the number of vidx group members (two first indexes) */
static GT_U8  prvTgfMcMembersNum = 2;

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   prvTgfMcMembersNum


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x22, 0x33, 0x44, 0x55, 0x66},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x22}                /* srcMac */
};

/* Multicast unregistered da mac */
static TGF_MAC_ADDR     prvTgfMcastUnregDaMac =
    {0x01, 0x77, 0x88, 0x99, 0xAA, 0xBB};

/* Multicast registered da mac */
static TGF_MAC_ADDR     prvTgfMcastRegDaMac =
    {0x01, 0x22, 0x33, 0x44, 0x55, 0x66};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* stored default drop mode */
static PRV_TGF_BRG_DROP_CNTR_MODE_ENT prvTgfDefDropMode = PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E;

/* stored default rate limit value */
static GT_U32  prvTgfDefRateLimit = 0xFFFF;

/* stored default rate limit parameters */
static PRV_TGF_BRG_GEN_RATE_LIMIT_STC   prvTgfDefRateLimitCfg;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgRateLimitMcastRegCfg function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] enableMcastReg           - GT_TRUE: enable rate limit for multicast registered packets
* @param[in] enableMcastUnReg         - GT_TRUE: enable rate limit for multicast unregistered packets
* @param[in] rateLimit                - rate limit specified in 64 bytes/packets
*                                      depends on ratelimit mode, range 0 - 65535
*                                       None
*/
static GT_VOID prvTgfBrgRateLimitMcastRegCfg
(
    IN GT_BOOL                              enableMcastReg,
    IN GT_BOOL                              enableMcastUnReg,
    IN GT_U32                               rateLimit
)
{
    GT_STATUS   rc;
    PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC     portGfg;
    PRV_TGF_BRG_GEN_RATE_LIMIT_STC          globalRateLimitCfg;


    /* AUTODOC: create VIDX 0xF with ports [0,1] */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_SEND_VIDX_CNS, prvTgfPortsArray,
                               NULL, prvTgfMcMembersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VIDX_CNS);

    /* AUTODOC: add FDB entry with MAC 01:22:33:44:55:66, VLAN 2, VIDX 0xF */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfMcastRegDaMac, PRV_TGF_VLANID_CNS,
                                          PRV_TGF_SEND_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d", prvTgfDevNum);

    /* Store default global rate limit configuration */
    rc = prvTgfBrgGenRateLimitGlobalCfgGet(prvTgfDevNum, &prvTgfDefRateLimitCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgGet: %d", prvTgfDevNum);

    globalRateLimitCfg.dropMode = CPSS_DROP_MODE_HARD_E;
    globalRateLimitCfg.rMode = CPSS_RATE_LIMIT_PCKT_BASED_E;
    globalRateLimitCfg.win1000Mbps = (0x3F * 256);
    globalRateLimitCfg.win100Mbps = (0x1FF * 256);
    globalRateLimitCfg.win10Gbps = ((0xFFF * 256) / 10);
    globalRateLimitCfg.win10Mbps = (0xFFF * 256);
    /* use value from the 'default setting' */
    globalRateLimitCfg.rateLimitModeL1 = prvTgfDefRateLimitCfg.rateLimitModeL1;

    /* AUTODOC: set rate limit global parameters wtih Hard drop mode and packet rate mode */
    rc = prvTgfBrgGenRateLimitGlobalCfgSet(prvTgfDevNum, &globalRateLimitCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgSet: %d", prvTgfDevNum);


    portGfg.enableMc = enableMcastUnReg;
    portGfg.enableMcReg = enableMcastReg;
    portGfg.rateLimit = rateLimit;
    /* default values */
    portGfg.enableBc = GT_FALSE;
    portGfg.enableTcpSyn = GT_FALSE;
    portGfg.enableUcKnown = GT_FALSE;
    portGfg.enableUcUnk = GT_FALSE;

    /* configure rate limit for specific ingress port */
    rc = prvTgfBrgGenPortRateLimitSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      &portGfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgGenRateLimitMcastRegPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
*                                       None
*/
static GT_VOID prvTgfBrgGenRateLimitMcastRegPacketSend
(
    IN GT_U32 portNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           portIter;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: send packet with configured DA, SA = 00:00:00:00:22:22 from port 2 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);
}


/**
* @internal prvTgfBrgGenRateLimitMcastRegTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*         INPUTS:
*/
static GT_VOID prvTgfBrgGenRateLimitMcastRegTrafficGenerate
(
    IN GT_BOOL                              enableMcastReg,
    IN GT_BOOL                              enableMcastUnReg,
    IN GT_U32                               rateLimit,
    IN GT_BOOL                              sendMcastRegPkts
)
{
    GT_STATUS rc;                               /* returned status */
    GT_U8  portIter;                            /* loop port iterator */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;    /* port counters */
    GT_U32  expectedPktsNum;                    /* expected packets number */
    GT_U32  expectedDropPkts;                   /* expected dropped packets */
    GT_U32  dropPkts;                           /* dropped packets */
    PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC  portGfg;   /* port rate limit configurations */

    /* Configure Rate Limit and global rate limit parameters */
    prvTgfBrgRateLimitMcastRegCfg(enableMcastReg,
                                  enableMcastUnReg,
                                  rateLimit);

   /* AUTODOC: configure drop counter to count only packets dropped due to rate limit  */
    rc = prvTgfBrgCntDropCntrModeSet(PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefDropMode);

    /* clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);


    if (sendMcastRegPkts == GT_TRUE)
    {
        /* Set destination addres to multicast registered */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcastRegDaMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }
    else
    {
        /* Set destination addres to multicast unregistered */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcastUnregDaMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }

    /* send L2 packet */
    prvTgfBrgGenRateLimitMcastRegPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    if (rateLimit == 0)
    {
        /* one or two packets will egress. second packet may be egressed
           because rate limit window may start again during burst */
        rateLimit = 1;
    }

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }

        else if (portIter < prvTgfMcMembersNum)
        {
            if ((enableMcastReg == GT_TRUE) && (sendMcastRegPkts == GT_TRUE))
            {
                /* one or two packets are OK for test */
                expectedPktsNum = rateLimit + 1;
                if (portCntrs.goodPktsSent.l[0] != 0 && /* it is not OK to get 0 packets */
                    portCntrs.goodPktsSent.l[0] <= expectedPktsNum)
                {
                    expectedPktsNum = portCntrs.goodPktsSent.l[0];
                }
            }
            else
            {
                expectedPktsNum = prvTgfBurstCount;
            }

            /* check that packets received on Tx ports are as expected */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPktsNum, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        else
        {
            if (sendMcastRegPkts == GT_TRUE)
            {
                expectedPktsNum = 0;
            }
            else
            {
                expectedPktsNum = prvTgfBurstCount;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPktsNum, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* get drop counter */
    rc = prvTgfBrgCntDropCntrGet(&dropPkts);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

    if ((enableMcastReg == GT_TRUE) && (sendMcastRegPkts == GT_TRUE))
    {
        /* three or two packets should be dropped */
        expectedDropPkts = prvTgfBurstCount - rateLimit - 1;
        if (dropPkts < expectedDropPkts)
        {
            /* check that dropped packets number is as expected */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedDropPkts, dropPkts,
                                         "get another drop counter than expected");
        }
    }
    else
    {
        expectedDropPkts = 0;
        /* check that dropped packets number is as expected */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedDropPkts, dropPkts,
                                     "get another drop counter than expected");
    }


    /* Restore default configurations */

    /* AUTODOC: restore Drop counter mode */
    rc = prvTgfBrgCntDropCntrModeSet(prvTgfDefDropMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefDropMode);
    /* AUTODOC: clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* AUTODOC: restore default global rate limit configuration */
    rc = prvTgfBrgGenRateLimitGlobalCfgSet(prvTgfDevNum, &prvTgfDefRateLimitCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgSet: %d", prvTgfDevNum);

    /* AUTODOC: restore default port rate limit configurations */
    portGfg.enableMc = GT_FALSE;
    portGfg.enableMcReg = GT_FALSE;
    portGfg.rateLimit = prvTgfDefRateLimit;
    portGfg.enableBc = GT_FALSE;
    portGfg.enableTcpSyn = GT_FALSE;
    portGfg.enableUcKnown = GT_FALSE;
    portGfg.enableUcUnk = GT_FALSE;

    /* configure rate limit for ingress port2 */
    rc = prvTgfBrgGenPortRateLimitSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      &portGfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitSet: %d", prvTgfDevNum);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfBrgGenRateLimitMcastRegConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgGenRateLimitMcastRegEnableRegTrafficGenerate function
* @endinternal
*
* @brief   Enable Rate Limit on port port2 for multicast registered packets.
*         Create vidx 0xF with port members port0 and port1.
*         Configure MAC 01:22:33:44:55:66 on vidx 0xF
*         Set window size to maximum and drop command to hard
*         Set rate limit to 1.
*         Generate traffic:
*         Send from port2 single tagged packet:
*         macDa = 01:22:33:44:55:66
*         macSa = 00:00:00:00:22:22
*         Success Criteria:
*         One packet is received on port0 and port1 and one packet is
*         dropped.
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegEnableRegTrafficGenerate
(
    void
)
{
    /* AUTODOC: enable rate limit on port 2 for MC registered packets */
    /* AUTODOC: disable rate limit on port 2 for MC unregistered packets */
    /* AUTODOC: set rate limit to 1 on port 2 */
    /* AUTODOC: configure DA MAC to send MC reg packet to 01:22:33:44:55:66 */
    prvTgfBrgGenRateLimitMcastRegTrafficGenerate(GT_TRUE, /* enable rate limit for
                                                             multicast registered */
                                                 GT_FALSE, /* disable rate limit for
                                                              multicast unregistered */
                                                 0,         /* rate limit */
                                                 GT_TRUE   /* send multicast registered */
                                                 );
    /* AUTODOC: verify one packet is received on port 0 and port 1 and one packet is dropped */
}

/**
* @internal prvTgfBrgGenRateLimitMcastRegDisableRegTrafficGenerate function
* @endinternal
*
* @brief   Disable Rate Limit on port port2 for multicast registered packets.
*         Create vidx 0xF with port members port0 and port1.
*         Configure MAC 01:22:33:44:55:66 on vidx 0xF
*         Set window size to maximum and drop command to hard
*         Set rate limit to 1.
*         Generate traffic:
*         Send from port2 single tagged packet:
*         macDa = 01:22:33:44:55:66
*         macSa = 00:00:00:00:22:22
*         Success Criteria:
*         Two packets are received on port0 and port1 and no packet is
*         dropped.
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegDisableRegTrafficGenerate
(
    void
)
{
    /* AUTODOC: disable rate limit on port 2 for MC registered packets */
    /* AUTODOC: disable rate limit on port 2 for MC unregistered packets */
    /* AUTODOC: set rate limit to 1 on port 2 */
    /* AUTODOC: configure DA MAC to send MC reg packet to 01:22:33:44:55:66 */
    prvTgfBrgGenRateLimitMcastRegTrafficGenerate(GT_FALSE, /* disable rate limit for
                                                              multicast registered */
                                                 GT_FALSE, /* disable rate limit for
                                                              multicast unregistered */
                                                 0,         /* rate limit */
                                                 GT_TRUE   /* send multicast registered */
                                                 );
    /* AUTODOC: two packets are received on port 0 and port 1 and no packet is dropped */
}

/**
* @internal prvTgfBrgGenRateLimitMcastRegEnableUnregTrafficGenerate function
* @endinternal
*
* @brief   Enable Rate Limit on port port2 for multicast registered packets.
*         Create vidx 0xF with port members port0 and port1.
*         Configure MAC 01:22:33:44:55:66 on vidx 0xF
*         Set window size to maximum and drop command to hard
*         Set rate limit to 1.
*         Generate traffic:
*         Send from port2 single tagged packet:
*         macDa = 01:77:88:99:AA:BB
*         macSa = 00:00:00:00:22:22
*         Success Criteria:
*         Two packets are received on port0 and port1 and no packet is
*         dropped.
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegEnableUnregTrafficGenerate
(
    void
)
{
    /* AUTODOC: enable rate limit on port 2 for MC registered packets */
    /* AUTODOC: disable rate limit on port 2 for MC unregistered packets */
    /* AUTODOC: set rate limit to 1 on port 2 */
    /* AUTODOC: configure DA MAC to send MC unreg packet to 01:77:88:99:AA:BB */
    prvTgfBrgGenRateLimitMcastRegTrafficGenerate(GT_TRUE, /* enable rate limit for
                                                             multicast registered */
                                                 GT_FALSE, /* disable rate limit for
                                                              multicast unregistered */
                                                 0,          /* rate limit */
                                                 GT_FALSE   /* send multicast
                                                               registered traffic */
                                                 );
    /* AUTODOC: two packets are received on port 0 and port 1 and no packet is dropped */
}


/**
* @internal prvTgfBrgGenRateLimitMcastRegDisableUnregEnableRegTrafficGenerate function
* @endinternal
*
* @brief   Disable Rate Limit on port port2 for multicast registered packets.
*         Enable Rate Limit on port port2 for multicast unregistered packets.
*         Create vidx 0xF with port members port0 and port1.
*         Configure MAC 01:22:33:44:55:66 on vidx 0xF
*         Set window size to maximum and drop command to hard
*         Set rate limit to 1.
*         Generate traffic:
*         Send from port2 single tagged packet:
*         macDa = 01:22:33:44:55:66
*         macSa = 00:00:00:00:22:22
*         Success Criteria:
*         Both packets are received on port0 and port1 and no packet is
*         dropped.
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegDisableUnregEnableRegTrafficGenerate
(
    void
)
{
    /* AUTODOC: disable rate limit on port 2 for MC registered packets */
    /* AUTODOC: enable rate limit on port 2 for MC unregistered packets */
    /* AUTODOC: set rate limit to 1 on port 2 */
    /* AUTODOC: configure DA MAC to send MC reg packet to 01:22:33:44:55:66 */
    prvTgfBrgGenRateLimitMcastRegTrafficGenerate(GT_FALSE, /* disable rate limit for
                                                              multicast registered */
                                                 GT_TRUE, /* enable rate limit for
                                                             multicast unregistered */
                                                 0,         /* rate limit */
                                                 GT_TRUE   /* send multicast registered */
                                                 );
    /* AUTODOC: both packets are received on port 0 and port 1 and no packet is dropped */
}

/**
* @internal prvTgfBrgGenRateLimitMcastRegConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenRateLimitMcastRegConfigurationRestore
(
    void
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

#define PRV_TGF_RATE_WS_VID1_CNS 245
#define PRV_TGF_RATE_WS_VIDX1_CNS 33
#define PRV_TGF_RATE_WS_VID2_CNS 439
#define PRV_TGF_RATE_WS_VIDX2_CNS 22
static GT_U32 sendPortNum = 2;
static GT_U16 sendVidArr[] = {PRV_TGF_RATE_WS_VID1_CNS, PRV_TGF_RATE_WS_VID2_CNS};
/* array of Send ports:
  step 0 - 24, 26;
  step 1 - 25, 27; */
static GT_U32 sendPortIdx[2][2] = {{0, 2}, {1, 3}};
static GT_U32 sendPortRateValue[2][2] = {{10, 20}, {30, 40}};

/* RL windows are used for stack ports:
  xCat - 10G window
  xCat2 - 1G window */
#define XCAT_STACK_WINDOW_CNS  500
#define XCAT2_STACK_WINDOW_CNS (XCAT_STACK_WINDOW_CNS * 10)

GT_VOID prvTgfBrgGenRateLimitWsTestTrafficSend
(
    GT_U32  testStep
);

GT_VOID prvTgfBrgGenRateLimitWsTestRateCfg
(
    GT_U32  testStep
)
{
    GT_STATUS                               rc;      /* return code*/
    PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC     portGfg; /* port rate limit config*/
    PRV_TGF_BRG_GEN_RATE_LIMIT_STC          globalRateLimitCfg; /* global config */
    GT_U32          i;                           /* iterator */
    GT_U32           portNum;                     /* port number */
    GT_U32          egressPortIdx[2];            /* indexes of egress ports */
    GT_U32          rateArr[PRV_TGF_MAX_PORTS_NUM_CNS]; /* array of rates */
    GT_U32          tmp;                         /* variable */
    GT_U32          window;                      /* RL window */
    GT_U32          expectRate;                  /* expected rate */

    if (testStep == 0)
    {
        /* Store default global rate limit configuration */
        rc = prvTgfBrgGenRateLimitGlobalCfgGet(prvTgfDevNum, &prvTgfDefRateLimitCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgGet: %d", prvTgfDevNum);

        globalRateLimitCfg.dropMode    = CPSS_DROP_MODE_HARD_E;
        globalRateLimitCfg.rMode       = CPSS_RATE_LIMIT_BYTE_BASED_E;
        globalRateLimitCfg.win10Gbps   = XCAT_STACK_WINDOW_CNS;  /* 0.5 milisecond */
        globalRateLimitCfg.win1000Mbps = globalRateLimitCfg.win10Gbps * 10;  /* 5 milisecond */
        globalRateLimitCfg.win100Mbps  = globalRateLimitCfg.win1000Mbps * 10;  /* 50 milisecond */
        globalRateLimitCfg.win10Mbps   = globalRateLimitCfg.win100Mbps * 10;  /* 500 milisecond */
        globalRateLimitCfg.rateLimitModeL1 = PRV_TGF_BRG_GEN_L1_RATE_LIMIT_INCL_E;

        /* AUTODOC: configure rate limit global parameters to: */
        /* AUTODOC:   rate limit byte count includes the L1 value */
        /* AUTODOC:   drop mode HARD, byte based rate mode */
        /* AUTODOC:   window sizes to 0.5, 5, 50, 500 msec. */
        rc = prvTgfBrgGenRateLimitGlobalCfgSet(prvTgfDevNum, &globalRateLimitCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgSet: %d", prvTgfDevNum);
    }

    egressPortIdx[0] = egressPortIdx[1] = 0;
    for (i = 0; i < sendPortNum; i++)
    {
        portNum = prvTgfPortsArray[sendPortIdx[testStep][i]];

        /* store egress port - pair of the ingress one */
        egressPortIdx[i] = sendPortIdx[(testStep + 1) & 1][i];

        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* xCat2 and above use separate MC register and
               unregister configuration  */
            portGfg.enableMcReg = GT_TRUE;
            portGfg.enableMc    = GT_FALSE;
        }
        else
        {
            /* xCat2 and above use enableMc for both MC register and
               unregister configuration. The enableMcReg is not used  */
            portGfg.enableMcReg = GT_FALSE;
            portGfg.enableMc    = GT_TRUE;
        }

        portGfg.rateLimit = sendPortRateValue[testStep][i];

        PRV_UTF_LOG2_MAC("Rate Limit on port %d is %d\n", portNum, portGfg.rateLimit);

        /* default values */
        portGfg.enableBc = GT_FALSE;
        portGfg.enableTcpSyn = GT_FALSE;
        portGfg.enableUcKnown = GT_FALSE;
        portGfg.enableUcUnk = GT_FALSE;

        /* AUTODOC: enable MC packets rate limiting on ingress port */
        rc = prvTgfBrgGenPortRateLimitSet(prvTgfDevNum,
                                          portNum,
                                          &portGfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: generate WS traffic on ingress ports */
    prvTgfBrgGenRateLimitWsTestTrafficSend(testStep);

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E))
    {
        window = XCAT2_STACK_WINDOW_CNS;
    }
    else
    {
        window = XCAT_STACK_WINDOW_CNS;
    }

    /* AUTODOC: get Tx rate on egress ports */
    prvTgfCommonPortsTxRateGet(egressPortIdx, sendPortNum, 1000, rateArr);
    for (i = 0; i < sendPortNum; i++)
    {
        /* calculate expected rate in packets per second */
        expectRate = 1000000 * sendPortRateValue[testStep][i] * 64 /
                      (window * (20 + PRV_TGF_PACKET_CRC_LEN_CNS));

        portNum = prvTgfPortsArray[egressPortIdx[i]];
        PRV_UTF_LOG2_MAC("Expect port %d, rate %d\n", portNum, expectRate);

        /* calculate difference between expected and actual rate */
        tmp = prvTgfCommonDiffInPercentCalc(expectRate, rateArr[i]);
        PRV_UTF_LOG2_MAC("Actual rate %d diff %d\n", rateArr[i], tmp);

        /* AUTODOC: verify 14% threshold for expected rate */
        if (tmp >= 14)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(0, tmp, "wrong rate: port %d expect %d actual %d",
                                             portNum, expectRate, rateArr[i]);
        }
    }
}

GT_VOID prvTgfBrgGenRateLimitWsTestTrafficSend
(
    GT_U32  testStep
)
{
    GT_STATUS       rc;                          /* return code*/
    GT_U32          portsCount = prvTgfPortsNum; /* number of used ports */
    GT_U32          portIter;                    /* iterator */
    TGF_PACKET_STC  packetInfo;                  /* packet info */
    GT_U32          i;                           /* iterator */
    GT_U32           portNum;                     /* port number */
    GT_U32          rateArr[PRV_TGF_MAX_PORTS_NUM_CNS]; /* array of rates */
    GT_U32          tmp;                         /* variable */
    GT_BOOL         needCheck;                   /* check WS traffic flag */
    GT_U32          checkCount;                  /* WS check counter */
    GT_U32          sendPortIndex;

   packetInfo = prvTgfPacketInfo;

    /* AUTODOC: GENERATE TRAFFIC: */
    if (testStep == 0)
    {
        /* Send packet Vlan Tagged */
        tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

        /* Set destination address to multicast registered */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcastRegDaMac,
                         sizeof(prvTgfPacketL2Part.daMac));

        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1 /* burst */, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                     prvTgfDevNum, 1, 0, NULL);

        /* AUTODOC: reset counters and set port speed 100M */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            portNum = prvTgfPortsArray[portIter];
            rc = prvTgfResetCountersEth(prvTgfDevNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, portNum);

            rc = prvTgfPortSpeedForce(prvTgfDevNum, portNum, CPSS_PORT_SPEED_100_E);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortSpeedForce: %d, %d",
                                         prvTgfDevNum, portNum);

            /* AUTODOC:   Drain the ports after mode reconfiguration. Because port
               in loopback does not behave good after port mode change when traffic always
               flown in port. MAC Reset-Unreset is part of sequence and it's harm MAC Loopback functionality.
               First packet may be dropped. */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                         prvTgfDevNum, portNum);

            /* manipulate link down-up to restore port's functionality */
            rc = cpssDxChPortForceLinkPassEnableSet(prvTgfDevNum, portNum, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortMacResetStateSet: %d, %d",
                                         prvTgfDevNum, portNum);
            rc = cpssDxChPortForceLinkPassEnableSet(prvTgfDevNum, portNum, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortMacResetStateSet: %d, %d",
                                         prvTgfDevNum, portNum);

            rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                         prvTgfDevNum, portNum);

        }
    }

    for (i = 0; i < sendPortNum; i++)
    {
        prvTgfPacketVlanTag0Part.vid = sendVidArr[i];

        portNum = prvTgfPortsArray[sendPortIdx[testStep][i]];

        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1 /* burst */, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                     prvTgfDevNum, 1, 0, NULL);

        /* configure WS mode */
        rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                  PRV_TGF_TX_MODE_CONTINUOUS_E,
                                  PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                  0);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                     prvTgfDevNum, portNum);

        /* AUTODOC: send 2 packets from ports 0,2 - 1 step; 1, 3 - 2 step: */
        /* AUTODOC:   DA=01:22:33:44:55:66, SA=00:00:00:00:22:22, VIDs=245,439 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                     prvTgfDevNum, portNum);
    }

    PRV_UTF_LOG0_MAC("\n");

    needCheck = GT_TRUE;
    checkCount = 10;
    while (needCheck && checkCount)
    {
        needCheck = GT_FALSE;
        checkCount--;

        /* AUTODOC: verify the WS traffic */
        prvTgfCommonPortsRxRateGet(&(sendPortIdx[testStep][0]), sendPortNum, 1000, rateArr);
        for (i = 0; i < sendPortNum; i++)
        {
            sendPortIndex = sendPortIdx[testStep][i];
            portNum = prvTgfPortsArray[sendPortIndex];
            PRV_UTF_LOG2_MAC("[TrafficSend]: Rx port %d rate %d\n", portNum, rateArr[i]);

            tmp = prvTgfPortWsRateCheck(sendPortIndex, rateArr[i], PRV_TGF_PACKET_CRC_LEN_CNS);
            if (tmp && (rateArr[i] < prvTgfCommonPortWsRateGet(sendPortIndex,PRV_TGF_PACKET_CRC_LEN_CNS)))
            {
                /* check all ports again after retransmission */
                needCheck = GT_TRUE;

                /* the rate is not good .
                   start again */
                prvTgfPacketVlanTag0Part.vid = sendVidArr[i];
                PRV_UTF_LOG2_MAC("[BrgGenRateLimitWsTestTrafficSend]: wrong Rx port %d rate %d\n", portNum, rateArr[i]);

                /* setup Packet */
                rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                             prvTgfDevNum, 1, 0, NULL);

                /* send Packet from port portNum */
                rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                             prvTgfDevNum, portNum);
            }
        }
    }
}


GT_VOID prvTgfBrgGenRateLimitWsTestBridgeCfg
(
    GT_VOID
)
{
    GT_U32       portsArray[2];
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 245 with untagged ports 0, 1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RATE_WS_VID1_CNS,
            prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: create VLAN 439 with untagged ports 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RATE_WS_VID2_CNS,
            (prvTgfPortsArray + 2), NULL, NULL, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: create VIDX 33 with ports 0, 1 */
    portsArray[0] = prvTgfPortsArray[0];
    portsArray[1] = prvTgfPortsArray[1];
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_RATE_WS_VIDX1_CNS, portsArray,
                               NULL, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VIDX1_CNS);

    /* AUTODOC: add FDB entry with MAC 01:22:33:44:55:66, VLAN 245, VIDX 33 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfMcastRegDaMac, PRV_TGF_RATE_WS_VID1_CNS,
                                          PRV_TGF_RATE_WS_VIDX1_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d", prvTgfDevNum);

    /* AUTODOC: create VIDX 22 with ports 2, 3 */
    portsArray[0] = prvTgfPortsArray[2];
    portsArray[1] = prvTgfPortsArray[3];
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_RATE_WS_VIDX2_CNS, portsArray,
                               NULL, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VIDX2_CNS);

    /* AUTODOC: add FDB entry with MAC 01:22:33:44:55:66, VLAN 439, VIDX 22 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfMcastRegDaMac, PRV_TGF_RATE_WS_VID2_CNS,
                                          PRV_TGF_RATE_WS_VIDX2_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d", prvTgfDevNum);

}

GT_VOID prvTgfBrgGenRateLimitWsTestRestore
(
    GT_U32 testStep
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          i;
    GT_U32           portNum;
    PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC     portGfg;
    CPSS_INTERFACE_INFO_STC                 portInterface;

    /* AUTODOC: RESTORE CONFIGURATION: */

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;

    for (i = 0; i < sendPortNum; i++)
    {
        portNum = prvTgfPortsArray[sendPortIdx[testStep][i]];

        /* stop send Packet from port portNum */
        rc = prvTgfStopTransmitingEth(prvTgfDevNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StopTransmitting: %d, %d",
                                     prvTgfDevNum, portNum);

        /* configure default mode */
        rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                  PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                  PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                  0);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                     prvTgfDevNum, portNum);

    }

    /* wait some time to guaranty that traffic stopped */
    cpssOsTimerWkAfter(10);

    if (testStep == 0)
    {
        /* do not restore configuration after step #0 */
        return;
    }
    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RATE_WS_VID1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VID1_CNS);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RATE_WS_VID2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VID2_CNS);

    /* AUTODOC: invalidate VIDX 33 */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_RATE_WS_VIDX1_CNS, NULL,
                               NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VIDX1_CNS);

    /* AUTODOC: invalidate VIDX 22 */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_RATE_WS_VIDX2_CNS, NULL,
                               NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RATE_WS_VIDX2_CNS);

    /* AUTODOC: restore global rate limit config */
    rc = prvTgfBrgGenRateLimitGlobalCfgSet(prvTgfDevNum, &prvTgfDefRateLimitCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitGlobalCfgSet: %d", prvTgfDevNum);

    /* AUTODOC: restore default port rate limit configurations */
    for (i = 0; i < prvTgfPortsNum; i++)
    {
        /* Restore default port rate limit configurations */
        portGfg.enableMc = GT_FALSE;
        portGfg.enableMcReg = GT_FALSE;
        portGfg.rateLimit = prvTgfDefRateLimit;
        portGfg.enableBc = GT_FALSE;
        portGfg.enableTcpSyn = GT_FALSE;
        portGfg.enableUcKnown = GT_FALSE;
        portGfg.enableUcUnk = GT_FALSE;

        /* configure rate limit for ingress port2 */
        rc = prvTgfBrgGenPortRateLimitSet(prvTgfDevNum,
                                          prvTgfPortsArray[i],
                                          &portGfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenRateLimitSet: %d", prvTgfDevNum);

        /* disable force link pass */
        portInterface.devPort.portNum = prvTgfPortsArray[i];
        rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ForceLinkUpEnableSet: %d", prvTgfPortsArray[i]);
    }

    /* restore VLAN */
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_CNS;

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/**
* @internal prvTgfBrgGenRateLimitWsTest function
* @endinternal
*
* @brief   Test for rate limit on stack ports under WS traffic.
*         Step 0 Ingress Ports 24 26
*         Egress Ports 25 27
*         Step 1 Ingress Ports 25 27
*         Egress Ports 24 26
*         Use registered multicast traffic for rate limit checks.
*         Success Criteria:
*         Traffic Rate on egress ports is same as rate limit
*         on ingress one
*/
GT_VOID prvTgfBrgGenRateLimitWsTest
(
    GT_VOID
)
{
    prvTgfPortsArray[0] = 24;
    prvTgfPortsArray[1] = 25;
    prvTgfPortsArray[2] = 26;
    prvTgfPortsArray[3] = 27;

    /* configure bridging */
    prvTgfBrgGenRateLimitWsTestBridgeCfg();

    /* AUTODOC: start 1 step */

    /* configure rate limit on ports 24 and 26 and send WS 100 M traffic from ports 24 and 26 */
    prvTgfBrgGenRateLimitWsTestRateCfg(0);

    /* stop traffic for ports 24 and 26 */
    prvTgfBrgGenRateLimitWsTestRestore(0);

    /* AUTODOC: start 2 step */

    /* configure rate limit on ports 25 and 27 and send WS 100 M traffic from ports 25 and 27 */
    prvTgfBrgGenRateLimitWsTestRateCfg(1);

    /* stop traffic for ports 25 and 26,
       restore global configuration */
    prvTgfBrgGenRateLimitWsTestRestore(1);
}


