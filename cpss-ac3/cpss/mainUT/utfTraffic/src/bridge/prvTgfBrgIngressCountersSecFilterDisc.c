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
* @file prvTgfBrgIngressCountersSecFilterDisc.c
*
* @brief The test checks that packets are received in
* counter set 1, field <SecFilterDisc> in all modes.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgIngressCountersSecFilterDisc.h>
#include <utf/private/prvUtfExtras.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN ID */
#define PRV_TGF_DEF_VLANID_CNS                  1

/* VLAN ID */
#define PRV_TGF_VLANID_CNS                      2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_1_CNS             0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_2_CNS             3

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            2

/* namber of iter */
#define PRV_TGF_ITER_NUM_CNS                    4

/* namber of McEntrys for test */
#define PRV_TGF_MC_ENTRY_NUM_CNS                4

/* default number of packets to send */
static GT_U32  prvTgfBurstCountArr[PRV_TGF_SEND_PACKETS_NUM_CNS]   =
{
    3, 5
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_ITER_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0}
    },
    {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1}
    }
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_ITER_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0}
    },
    {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1}
    }
};

/* expected number Bridge ingress counter */
static GT_U32 prvTgfBridgeIngresssCountrArr[PRV_TGF_ITER_NUM_CNS][4] =
{
    {8, 0, 8, 0},
    {5, 0, 5, 0},
    {3, 0, 3, 0},
    {1, 0, 1, 0}
};

/* array of DA macs for test */
static TGF_MAC_ADDR prvTgfBrgDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_ITER_NUM_CNS] =
{
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
    },
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
    }
};

/* array of SA macs for test */
static TGF_MAC_ADDR prvTgfBrgSaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_ITER_NUM_CNS] =
{
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}
    },
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x44}
    }
};

/* array of VlanId for test */
static  TGF_VLAN_ID prvTgfBrgVlanIdArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_DEF_VLANID_CNS,
    PRV_TGF_VLANID_CNS
};

/* array of send portIdx for test */
static  GT_U8 prvTgfBrgSendPortArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_1_CNS,
    PRV_TGF_SEND_PORT_IDX_2_CNS
};

/* DA command array for test */
static PRV_TGF_PACKET_CMD_ENT  prvTgfDaCommandArr[PRV_TGF_MC_ENTRY_NUM_CNS] =
{
    PRV_TGF_PACKET_CMD_DROP_SOFT_E,
    PRV_TGF_PACKET_CMD_DROP_SOFT_E,
    PRV_TGF_PACKET_CMD_FORWARD_E,
    PRV_TGF_PACKET_CMD_FORWARD_E
};

/* VLAN Id array for mcEntry */
static TGF_VLAN_ID  prvTgfVlanIdArr[PRV_TGF_MC_ENTRY_NUM_CNS] =
{
    1, 2, 1, 2
};

/* PortIdx array for mcEntry */
static GT_U16  prvTgfPortIdxArr[PRV_TGF_MC_ENTRY_NUM_CNS] =
{
    1, 2, 1, 2
};

static TGF_MAC_ADDR prvTgfMcEntryAddrArr[PRV_TGF_MC_ENTRY_NUM_CNS] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_DEF_VLANID_CNS                        /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/
/*************************** Restore config ***********************************/
/* Parameters needed to be restored */
static struct
{
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode;
    GT_PORT_NUM                         port;
    GT_U16                              vlan;
    GT_BOOL                             enable1;
    GT_BOOL                             enable2;
}prvTgfConfigRestore;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgIngressCountersSecFilterDiscConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* save Bridge Ingress counters mode */
    rc = prvTgfBrgCntBridgeIngressCntrModeGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                              &prvTgfConfigRestore.setMode,
                                              &prvTgfConfigRestore.port,
                                              &prvTgfConfigRestore.vlan);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeGet");

    /* save Security Breach for Moved Static addresses mode on portIdx 0 */
    rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet(prvTgfDevNum,
                                                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                                             &prvTgfConfigRestore.enable1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet");

    /* save Security Breach for Moved Static addresses mode on portIdx 3 */
    rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet(prvTgfDevNum,
                                                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                             &prvTgfConfigRestore.enable2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet");
}

/**
* @internal prvTgfBrgIngressCountersSecFilterDiscTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           i = 0;
    GT_U8                           portIter = 0;
    GT_U8                           sendIter = 0;
    GT_U8                           packetIter = 0;
    GT_U32                          packetLen = 0;
    GT_U32                          burstCount = 0;
    CPSS_BRIDGE_INGRESS_CNTR_STC    ingressCntr;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    /* AUTODOC: Iterate thru 4 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_ITER_NUM_CNS; sendIter++)
    {
        switch (sendIter)
        {
            case 0:
                /* AUTODOC: Stage#1 - for cntSet0 set Bridge Ingress cnt mode CNT_MODE_0 */
                rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                                          CPSS_BRG_CNT_MODE_0_E,
                                                          0, 0 /* not relevant on this mode */);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeSet");

                for(i = 0; i < PRV_TGF_MC_ENTRY_NUM_CNS; i++)
                {
                    /* clear entry */
                    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

                    /* fill mac entry */
                    macEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
                    macEntry.key.key.macVlan.vlanId   = prvTgfVlanIdArr[i];
                    macEntry.isStatic                 = GT_TRUE;
                    macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
                    macEntry.dstInterface.type        = CPSS_INTERFACE_PORT_E;
                    macEntry.dstInterface.hwDevNum      = prvTgfDevNum;
                    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortIdxArr[i]];

                    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                                 prvTgfMcEntryAddrArr[i], sizeof(TGF_MAC_ADDR));

                    /* set DA command */
                    macEntry.daCommand                = prvTgfDaCommandArr[i];

                    /* AUTODOC: add 4 FDB entries with: */
                    /* AUTODOC:   MAC 00:00:00:00:00:01, VLAN 1, port 1, cmd DROP_SOFT */
                    /* AUTODOC:   MAC 00:00:00:00:00:02, VLAN 2, port 2, cmd DROP_SOFT */
                    /* AUTODOC:   MAC 00:00:00:00:00:03, VLAN 1, port 1, cmd FORWARD */
                    /* AUTODOC:   MAC 00:00:00:00:00:04, VLAN 2, port 2, cmd FORWARD */
                    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");
                }
                break;

            case 1:
                /* AUTODOC: Stage#2 - for cntSet0 set Bridge Ingress cnt mode CNT_MODE_1, port 3 */
                rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                                          CPSS_BRG_CNT_MODE_1_E,
                                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                          PRV_TGF_DEF_VLANID_CNS);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeSet");

                /* AUTODOC: Stage#2 - enable dropping Ethernet packets with MC MAC SA */
                rc = prvTgfBrgGenDropInvalidSaEnableSet(GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgGenDropInvalidSaEnableSet");

                break;

            case 2:
                /* AUTODOC: Stage#3 - for cntSet0 set Bridge Ingress cnt mode CNT_MODE_2, VLAN 1 */
                rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                                          CPSS_BRG_CNT_MODE_2_E,
                                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                          PRV_TGF_DEF_VLANID_CNS);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeSet");

                /* AUTODOC: Stage#3 - on port 0 enable Security Breach for Moved Static addresses */
                rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                                                         GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

                /* AUTODOC: Stage#3 - on port 3 enable Security Breach for Moved Static addresses */
                rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                                         GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

                if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
                {
                    /* AUTODOC: E_Arch devices*/
                    /* AUTODOC: set packet command to soft/hard drop , so it will be counted */
                    rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
                        CPSS_PACKET_CMD_DROP_SOFT_E);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurBreachEventPacketCommandSet");
                }

                break;

            case 3:
                /* AUTODOC: Stage#3 - for cntSet0 set Bridge Ingress cnt mode CNT_MODE_3, VLAN 2 */
                rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                                          CPSS_BRG_CNT_MODE_3_E,
                                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                          PRV_TGF_VLANID_CNS);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeSet");

                /* AUTODOC: Stage#3 - set dropping unknown source mac address on port 0 */
                rc = prvTgfBrgPortUnkSrcMacDropSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgPortUnkSrcMacDropSet");

                /* AUTODOC: Stage#3 - set dropping unknown source mac address on port 3 */
                rc = prvTgfBrgPortUnkSrcMacDropSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgPortUnkSrcMacDropSet");

                break;

            default:
                continue;
        }

        /* configure and set test packets  */
        for (packetIter = 0; packetIter < PRV_TGF_SEND_PACKETS_NUM_CNS; packetIter++)
        {
            /* reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* set Dst Mac */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfBrgDaMacArr[packetIter][sendIter], sizeof(TGF_MAC_ADDR));

            /* set Srs Mac */
            cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfBrgSaMacArr[packetIter][sendIter], sizeof(TGF_MAC_ADDR));

            /* set VlanId */
            cpssOsMemCpy(&prvTgfPacketVlanTagPart.vid, &prvTgfBrgVlanIdArr[packetIter], sizeof(TGF_VLAN_ID));

            burstCount = (sendIter != 3) ? prvTgfBurstCountArr[packetIter] : 1;
            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                     burstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            /* AUTODOC: for Stage#1 send 3 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:11, VID=1 */
            /* AUTODOC: for Stage#1 send 5 packets on port 3 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:22, VID=2 */

            /* AUTODOC: for Stage#2 send 3 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:03, SA=11:11:11:11:11:11, VID=1 */
            /* AUTODOC: for Stage#2 send 5 packets on port 3 with: */
            /* AUTODOC:   DA=00:00:00:00:00:04, SA=11:11:11:11:11:11, VID=2 */

            /* AUTODOC: for Stage#3 send 3 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:03, SA=00:00:00:00:00:01, VID=1 */
            /* AUTODOC: for Stage#3 send 5 packets on port 3 with: */
            /* AUTODOC:   DA=00:00:00:00:00:04, SA=00:00:00:00:00:02, VID=2 */

            /* AUTODOC: for Stage#4 send packet on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:03, SA=00:00:00:00:00:33, VID=1 */
            /* AUTODOC: for Stage#4 send packet on port 3 with: */
            /* AUTODOC:   DA=00:00:00:00:00:04, SA=00:00:00:00:00:44, VID=2 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfBrgSendPortArr[packetIter]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[prvTgfBrgSendPortArr[packetIter]]);

            /* AUTODOC: verify traffic is dropped */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                packetLen = (portIter == prvTgfBrgSendPortArr[packetIter]) ?
                    prvTgfTaggedPacketInfo.totalLen :
                    prvTgfTaggedPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS;

                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxArr[packetIter][sendIter][portIter],
                                            prvTgfPacketsCountTxArr[packetIter][sendIter][portIter],
                                            packetLen, burstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
        }

        /* clear current counters value */
        cpssOsMemSet((GT_VOID*) &ingressCntr, 0, sizeof(ingressCntr));

        /* AUTODOC: get Bridge ingress counters: */
        rc = prvTgfBrgCntBridgeIngressCntrsGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E, &ingressCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrsGet");

        /* AUTODOC:   check Bridge ingress counters - gtBrgInFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][0],
                                     ingressCntr.gtBrgInFrames,
                   "get another gtHostInPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][0], ingressCntr.gtBrgInFrames);

        /* AUTODOC:   check Bridge ingress counters - gtBrgVlanIngFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][1],
                                     ingressCntr.gtBrgVlanIngFilterDisc,
                   "get another gtHostOutBroadcastPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][1], ingressCntr.gtBrgVlanIngFilterDisc);

        /* AUTODOC:   check Bridge ingress counters - gtBrgSecFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][2],
                                     ingressCntr.gtBrgSecFilterDisc,
                   "get another gtHostOutMulticastPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][2], ingressCntr.gtBrgSecFilterDisc);

        /* AUTODOC:   check Bridge ingress counters - gtBrgLocalPropDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][3],
                                     ingressCntr.gtBrgLocalPropDisc,
                   "get another gtHostOutPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][3], ingressCntr.gtBrgLocalPropDisc);
    }
}

/**
* @internal prvTgfBrgIngressCountersSecFilterDiscConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate dropping unknown source mac address on port 0 */
    rc = prvTgfBrgPortUnkSrcMacDropInvalidate(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgPortUnkSrcMacDropInvalidate");

    /* AUTODOC: invalidate dropping unknown source mac address on port 3 */
    rc = prvTgfBrgPortUnkSrcMacDropInvalidate(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgPortUnkSrcMacDropInvalidate");

    /* AUTODOC: restore Security Breach for Moved Static addresses mode on port 0 */
    rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                                             prvTgfConfigRestore.enable1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

    /* AUTODOC: restore Security Breach for Moved Static addresses mode on port 3 */
    rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                                             prvTgfConfigRestore.enable2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* AUTODOC: E_Arch devices */
        /* AUTODOC: set packet command to forward , so it will be not drop 'static moved' */
        rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            CPSS_PACKET_CMD_FORWARD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurBreachEventPacketCommandSet");
    }


    /* AUTODOC: restore Bridge Ingress counters mode */
    rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                              prvTgfConfigRestore.setMode, prvTgfConfigRestore.port,
                                              prvTgfConfigRestore.vlan);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrModeSet");
}


