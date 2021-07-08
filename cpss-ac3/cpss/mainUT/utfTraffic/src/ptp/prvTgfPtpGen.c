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
* @file prvTgfPtpGen.c
*
* @brief CPSS Precision Time Protocol (PTP) Technology facility implementation.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <appDemo/userExit/userEventHandler.h>

#include <oam/prvTgfOamGeneral.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPtpGen.h>
#include <common/tgfCncGen.h>
#include <ptp/prvTgfPtpGen.h>
#include <stdlib.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <common/tgfIpGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* saved VID */
static GT_U16 prvTgfDefVlanId = 1;
static PRV_TGF_PTP_TSU_CONTROL_STC origTgfTsuControl;
static GT_U32 prvTgfermrkTimestampQueueSize = 256;
static CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 91;
static GT_BOOL origOamEnabler = GT_TRUE;
static PRV_TGF_OAM_COMMON_CONFIG_STC  origOamConfig;
static PRV_TGF_PTP_TS_CFG_ENTRY_STC   origTimeStampCfgEntry;
static PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   origTsTagGlobalCfg;
static PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     origTsTagPortCfg;
static PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     origTsTagPortCfg1;
/******************************* Test packet **********************************/

/******************************* common payload *******************************/

/* Data of packet (default version) */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

/* Payload part (default version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, 0 /*pri*/, 0 /*cfi*/,
    PRV_TGF_PTP_GEN_VLAN_CNS
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{0x88FF};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPtpV2Part =
{
    5    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPtpV2PacketPartArray)
        / sizeof(prvTgfPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfPtpV2PacketPartArray                    /* partsArray */
};

/* Data of packet ptp version1  */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

/* Payload part (default version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart =
{
    sizeof(prvTgfPacket1PayloadDataArr), /* dataLength */
    prvTgfPacket1PayloadDataArr          /* dataPtr */
};

/* Data of PTP hybrid time stamp(6B) */
static GT_U8 prvTgfPacketPtpPayloadDataArr[] =
{
    0x07, 0x07, 0x07, 0x03, 0x03, 0x03
};

static TGF_PACKET_PAYLOAD_STC prvTgfPacketPtpPayloadPart =
{
    sizeof(prvTgfPacketPtpPayloadDataArr), /* dataLength */
    prvTgfPacketPtpPayloadDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacket1L2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, 0 , 0 ,
    PRV_TGF_PTP_GEN_VLAN_CNS
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
{0x88F7};

static TGF_PACKET_PTP_V2_STC prvTgfPacket1PtpV2Part =
{
    5    /*messageType*/,
    0xF  /*transportSpecific*/,
    1    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of PTP hybrid timstamp packet */
static TGF_PACKET_PART_STC prvTgfPtpV2Packet1PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPtpPayloadPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacket1PtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPtpV2Packet1Info =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPtpV2Packet1PartArray)
        / sizeof(prvTgfPtpV2Packet1PartArray[0])), /* numOfParts */
    prvTgfPtpV2Packet1PartArray                    /* partsArray */
};

/* Data of PTP over UDP packet */
static GT_U8 prvTgfPacket2PayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x09, 0x08, 0x07, 0x05, 0x04, 0x03,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x14,
    0x04, 0x00, 0x00, 0x4a, 0x0c, 0x0d
};

static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart =
{
    sizeof(prvTgfPacket2PayloadDataArr), /* dataLength */
    prvTgfPacket2PayloadDataArr          /* dataPtr */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket2Ipv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                                          /* version */
    0xF,                                          /* headerLen */
    0,                                          /* typeOfService */
    0x84,                                       /* totalLen */
    0,                                          /* id */
    0,                                          /* flags */
    0,                                          /* offset */
    0x40,                                       /* timeToLive */
    0x11,                                       /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,          /* csum */
    { 2,  2,  2,  2},                           /* srcAddr */
    { 1,  1,  1,  3}                            /* dstAddr */
};

/* Data of packet ptp */
static GT_U8 prvTgfPacket2PtpPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PtpPayloadPart = {
    sizeof(prvTgfPacket2PtpPayloadDataArr),/* dataLength */
    prvTgfPacket2PtpPayloadDataArr         /* dataPtr */
};

/* PARTS of PTP over UDP packet */
 static TGF_PACKET_PART_STC prvTgfPtpV2Packet2PartArray[] =
 {
     {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
     {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket2Ipv4EtherTypePart},
     {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart},/*6B field of TS */
     {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPtpV2Part},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PtpPayloadPart}
};

/* PTP over UDp packet to send */
static TGF_PACKET_STC prvTgfPtpV2Packet2Info =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPtpV2Packet2PartArray)
        / sizeof(prvTgfPtpV2Packet2PartArray[0])), /* numOfParts */
    prvTgfPtpV2Packet2PartArray                    /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/********************************************************************************/
/*                                     UTILS                                    */
/********************************************************************************/
/**
* @internal prvTgfPtpIsSingleTaiSystem function
* @endinternal
*
* @brief   Returns single TAI flag
*
* @retval 1                        - single TAI
* @retval 0                        - multiple TAI
*/
GT_U32 prvTgfPtpIsSingleTaiSystem(void)
{
    /* BobK devices has single TAI */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E))
    {
        return 1;
    }
    return 0;
}
/**
* @internal prvTgfPtpGenUtilTaiTodCounterSet function
* @endinternal
*
* @brief   Set TOD Counter current time.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodValuePtr           - (pointer to) TOD value.
*                                       relevant returned code
*/
GT_STATUS prvTgfPtpGenUtilTaiTodCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
)
{
    GT_STATUS rc;

    rc = prvTgfPtpTaiTodSet(
        tgfTaiIdPtr,
        PRV_TGF_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,
        tgfTodValuePtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTaiTodSet");

    rc = prvTgfPtpTodCounterFunctionSet(
        CPSS_PORT_DIRECTION_BOTH_E,
        tgfTaiIdPtr,
        PRV_TGF_PTP_TOD_COUNTER_FUNC_UPDATE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTodCounterFunctionSet");

    return GT_OK;
}

/**
* @internal prvTgfPtpGenUtilTaiTodCounterGet function
* @endinternal
*
* @brief   Get TOD Counter current time.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] tgfTodValuePtr           - (pointer to) TOD value.
*                                       relevant returned code
*/
GT_STATUS prvTgfPtpGenUtilTaiTodCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
)
{
    GT_STATUS rc;

    rc = prvTgfPtpTaiTodCaptureStatusSet(
        tgfTaiIdPtr,
        0 /*captureIndex*/,
        GT_FALSE /*valid*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTaiTodCaptureStatusSet");

    rc = prvTgfPtpTodCounterFunctionSet(
        CPSS_PORT_DIRECTION_BOTH_E,
        tgfTaiIdPtr,
        PRV_TGF_PTP_TOD_COUNTER_FUNC_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTodCounterFunctionSet");

    rc = prvTgfPtpTodCounterFunctionTriggerSet(
        CPSS_PORT_DIRECTION_BOTH_E,
        tgfTaiIdPtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTodCounterFunctionTriggerSet");

    rc = prvTgfPtpTaiTodGet(
        tgfTaiIdPtr,
        PRV_TGF_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,
        tgfTodValuePtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTaiTodSet");

    return GT_OK;
}

/********************************************************************************/
/*                                Test Functions                                */
/********************************************************************************/

/**
* @internal prvTgfPtpGenVidAndFdbDefCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PTP_GEN_VLAN_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], PRV_TGF_PTP_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_PTP_GEN_VLAN_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfPtpGenFdbDefCfgSet function
* @endinternal
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenFdbDefCfgSet
(
    IN GT_U32 receive_port_index
)
{
    GT_STATUS rc = GT_OK;

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_PTP_GEN_VLAN_CNS, prvTgfDevNum,
            prvTgfPortsArray[receive_port_index], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpGenFdbDefCfgSet");

    return rc;


};


/**
* @internal prvTgfPtpGenVidAndFdbDefCfgRestoreEgress function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgRestoreEgress
(
    GT_VOID
)
{
    GT_STATUS                                          rc, rc1 = GT_OK;
    PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC    tgfTsPortEntry;

    GT_U32                                             i;

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* Vlan resrore */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PTP_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* clear queues */
    for (i = 0; (i < 20); i++)
    {
        /*read from global queue*/
        rc = prvTgfPtpTsEgressTimestampQueueEntryRead(
            0 /*queueNum*/, &tgfTsQueueEntry);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPtpTsEgressTimestampQueueEntryRead, rc is %d queue 0",rc);
        /*sip 5 and above have also per port queue */
        if(PRV_CPSS_PP_MAC(prvTgfDevNum) && PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) && tgfTsQueueEntry.entryValid)
        {
           rc = cpssDxChPtpTsuTxTimestampQueueRead(prvTgfDevNum,tgfTsQueueEntry.portNum,0,&tgfTsPortEntry);
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
                GT_OK, rc, "cpssDxChPtpTsuTxTimestampQueueRead, rc is %d queue 0",rc);
        }

        /*read from global queue*/
        rc = prvTgfPtpTsEgressTimestampQueueEntryRead(
            1 /*queueNum*/, &tgfTsQueueEntry);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK,rc, "prvTgfPtpTsEgressTimestampQueueEntryRead, rc is %d queue 1",rc );
        /*sip 5 and above have also per port queue */

        if(PRV_CPSS_PP_MAC(prvTgfDevNum) && PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)&& tgfTsQueueEntry.entryValid)
        {

          rc = cpssDxChPtpTsuTxTimestampQueueRead(prvTgfDevNum,tgfTsQueueEntry.portNum,1,&tgfTsPortEntry);
          UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
                GT_OK, rc, "cpssDxChPtpTsuTxTimestampQueueRead, rc is %d queue 1",rc);
        }
    }

    return rc1;
};

/**
* @internal prvTgfPtpGenVidAndFdbDefCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgRestore
(
    GT_VOID
)
{
    GT_STATUS                                          rc, rc1 = GT_OK;
    PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;
    GT_U32                                             i;

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* Vlan resrore */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PTP_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* clear queues */
    for (i = 0; (i < 20); i++)
    {
        rc = prvTgfPtpTsIngressTimestampQueueEntryRead(
            0 /*queueNum*/, &tgfTsQueueEntry);

        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPtpTsIngressTimestampQueueEntryRead, rc is %d 1",rc);
        rc = prvTgfPtpTsIngressTimestampQueueEntryRead(
            1 /*queueNum*/, &tgfTsQueueEntry);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK,rc, "prvTgfPtpTsIngressTimestampQueueEntryRead, rc is %d 2",rc );
    }

    return rc1;
};

/**
* @internal prvTgfPtpGenAddTimeAfterPtpHeaderCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenAddTimeAfterPtpHeaderCfgSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction;

    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    localAction.ingrLinkDelayEnable = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;

    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsLocalActionTableSet");

    return GT_OK;
}

/**
* @internal prvTgfPtpGenIngressCaptureCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenIngressCaptureCfgSet
(
        IN GT_U32 receive_port_index
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction;

    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[receive_port_index],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    localAction.ingrLinkDelayEnable = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction = PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;

    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[receive_port_index],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsLocalActionTableSet");

    rc = prvTgfPtpTsMessageTypeToQueueIdMapSet(
        (1 << prvTgfPacketPtpV2Part.messageType));
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsMessageTypeToQueueIdMapSet");

    return GT_OK;
}

/**
* @internal prvTgfPtpGenEgressCaptureCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenEgressCaptureCfgSet
(
        IN GT_U32 receive_port_index
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction;

    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[receive_port_index],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    localAction.ingrLinkDelayEnable = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction = PRV_TGF_PTP_TS_ACTION_CAPTURE_E;

    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[receive_port_index],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsLocalActionTableSet");

    rc = prvTgfPtpTsMessageTypeToQueueIdMapSet(
        (1 << prvTgfPacketPtpV2Part.messageType));
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsMessageTypeToQueueIdMapSet");

    return GT_OK;
}

/* TOD_NANOSEC on ingress port TAI instance that is used for correctionField update */
#define PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_USED_CNS   20000

/* TOD_NANOSEC on ingress port TAI instance that is NOT used for correctionField update */
#define PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_OTHER_CNS  0

/* TOD_NANOSEC on egress port TAI instance that is used for correctionField update */
#define PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_USED_CNS    40000

/* TOD_NANOSEC on egress port TAI instance that is NOT used for correctionField update */
#define PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_OTHER_CNS   60000

GT_STATUS appDemoBc2PtpTaiCheck
(
    IN  GT_U8  devNum
);

/**
* @internal prvTgfPtpGenAddCorrectedTimeTodSet function
* @endinternal
*
* @brief   Set TODs
*
* @param[in] egressPortNanosec        - nanoseconds value for egress port TOD
*                                       None
*/
GT_STATUS prvTgfPtpGenAddCorrectedTimeTodSet
(
    GT_U32 egressPortNanosec
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_TAI_ID_STC                tgfTaiId;
    PRV_TGF_PTP_TOD_COUNT_STC             tgfTodValue;

    /* BC3 and above has only two global TAIs, tgfTaiId.portNum is ignored */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Set not used Egress Port TAI 0 TOD to be different from TAI 1*/
        cpssOsMemSet(&tgfTodValue, 0, sizeof(tgfTodValue));
        tgfTodValue.nanoSeconds = PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_OTHER_CNS;
        tgfTaiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
        tgfTaiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_0_E;
        tgfTaiId.portNum     =
            prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
        rc = prvTgfPtpGenUtilTaiTodCounterSet(&tgfTaiId, &tgfTodValue);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpGenUtilTaiTodCounterSet");
    }

    /* AUTODOC: Set Egress Port used TAI 1 TOD by input parameter */
    cpssOsMemSet(&tgfTodValue, 0, sizeof(tgfTodValue));
    tgfTodValue.nanoSeconds = egressPortNanosec;
    tgfTaiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
    tgfTaiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_1_E;
    tgfTaiId.portNum     =
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
    rc = prvTgfPtpGenUtilTaiTodCounterSet(&tgfTaiId, &tgfTodValue);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpGenUtilTaiTodCounterSet");

    /* AUTODOC: Set Ingress Port used TAI 0 TOD to be same as Egress port TAI 1 */
    cpssOsMemSet(&tgfTodValue, 0, sizeof(tgfTodValue));
    tgfTodValue.nanoSeconds = PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_USED_CNS;
    tgfTaiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
    tgfTaiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_0_E;
    tgfTaiId.portNum     =
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS];
    rc = prvTgfPtpGenUtilTaiTodCounterSet(&tgfTaiId, &tgfTodValue);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpGenUtilTaiTodCounterSet");

    /* BC3 and above has only two global TAIs, tgfTaiId.portNum is ignored */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Set not used Ingress Port TAI 1 TOD to be different from TAI 0*/
        cpssOsMemSet(&tgfTodValue, 0, sizeof(tgfTodValue));
        tgfTodValue.nanoSeconds = PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_OTHER_CNS;
        tgfTaiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
        tgfTaiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_1_E;
        tgfTaiId.portNum     =
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS];
        rc = prvTgfPtpGenUtilTaiTodCounterSet(&tgfTaiId, &tgfTodValue);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpGenUtilTaiTodCounterSet");
    }

    /* AUTODOC: Trigger function for all TAIs simulteneously */
    rc = prvTgfPtpTaiTodCounterFunctionAllTriggerSet();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTaiTodCounterFunctionAllTriggerSet");

    /* appDemoBc2PtpTaiCheck(0); */

    return GT_OK;
}

/**
* @internal prvTgfPtpGenAddCorrectedTimeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenAddCorrectedTimeCfgSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction;
    PRV_TGF_PTP_TSU_CONTROL_STC           tgfTsuControl;

    /* AUTODOC: Configure classification of PTP packet */
    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEgressDomainTableSet");

    /* AUTODOC: Configure Add Corrected Time for PTP packet by Local Action table */
    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    localAction.ingrLinkDelayEnable = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction = PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;

    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsLocalActionTableSet");

    /* AUTODOC: Set Egress Port used TAI 1 TOD to be same as Ingress port TAI 0 */
    prvTgfPtpGenAddCorrectedTimeTodSet(PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_USED_CNS);

    rc = prvTgfPtpTsuControlGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],&origTgfTsuControl);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsuControlGet");

    /* AUTODOC: Configure Ingress port to use TAI 0 */
    tgfTsuControl.unitEnable          = GT_TRUE;
    tgfTsuControl.tsQueOverrideEnable = GT_TRUE;
    tgfTsuControl.rxTaiSelect         = 0;
    rc = prvTgfPtpTsuControlSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &tgfTsuControl);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsuControlSet");

    /* AUTODOC: Configure Egress port to use TAI 1 */
    rc = prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet");

    if (0 == prvTgfPtpIsSingleTaiSystem())
    {
        rc = prvTgfPtpTsDelayEgressTimestampTaiSelectSet(
            prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
            1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsDelayEgressTimestampTaiSelectSet");
    }
    return GT_OK;
}


/**
* @internal prvTgfPtpGenEgressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenEgressCaptureQueue1Check
(
     IN GT_U32 expected_valid
)
{
    GT_STATUS                                          rc = GT_OK;
    PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC    tgfTsPortEntry;

    rc = prvTgfPtpTsEgressTimestampQueueEntryRead(
        1 /*queueNum*/, &tgfTsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPtpTsEgressTimestampQueueEntryRead");

     if (GT_TRUE == expected_valid)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_FALSE, tgfTsQueueEntry.entryValid, "entryValid");
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(
      PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tgfTsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(
      prvTgfPacketPtpV2Part.sequenceId  , tgfTsQueueEntry.sequenceId, "sequenceId");

    if(PRV_CPSS_PP_MAC(prvTgfDevNum) && PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)&& tgfTsQueueEntry.entryValid)
        {
            rc = cpssDxChPtpTsuTxTimestampQueueRead(prvTgfDevNum,tgfTsQueueEntry.portNum,1,&tgfTsPortEntry);
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
                GT_OK, rc, "cpssDxChPtpTsuTxTimestampQueueRead, rc is %d queue 1",rc);
        }


    PRV_UTF_LOG1_MAC("[TGF]: prvTgfPtpGenEgressCaptureQueue1Check end. valid entry: %d\n",tgfTsQueueEntry.entryValid);
}

/**
* @internal prvTgfPtpGenIngressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenIngressCaptureQueue1Check
(
    IN  GT_U32 expected_valid
)
{
    GT_STATUS                                          rc = GT_OK;
    PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;

    rc = prvTgfPtpTsIngressTimestampQueueEntryRead(
        1 /*queueNum*/, &tgfTsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPtpTsIngressTimestampQueueEntryRead");
    if (GT_TRUE == expected_valid)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_FALSE, tgfTsQueueEntry.entryValid, "entryValid");
        return;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");

    UTF_VERIFY_EQUAL0_STRING_MAC(
      PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tgfTsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(
      prvTgfPacketPtpV2Part.sequenceId  , tgfTsQueueEntry.sequenceId, "sequenceId");

    PRV_UTF_LOG1_MAC("[TGF]: prvTgfPtpGenIngressCaptureQueue1Check end. the entry is valid: %d\n",tgfTsQueueEntry.entryValid);
}

/**
* @internal prvTgfPtpGenCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction;

    rc = prvTgfPtpOverEthernetEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, 0);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[0],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpTsLocalActionTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[0],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpTsLocalActionTableSet");


    rc = prvTgfPtpTsMessageTypeToQueueIdMapSet(0);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPtpTsMessageTypeToQueueIdMapSet");

    return rc1;
}

/**
* @internal prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam = {0};
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;

    /* VLAN Configuration */

    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");


    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* check that timestamp overlapped the payload first 8 bytes */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + TGF_PTP_V2_HDR_SIZE_CNS;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadDataArr, 8);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, memCmpRc, "timestamp updated check");
}

/**
* @internal prvTgfPtpGenTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet.
*/
GT_STATUS prvTgfPtpGenTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2PacketInfo,
        1 /*burstCount*/, 0 /*numVfd*/, NULL /*vfdArray*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(200 /*sleepTime*/);

    return GT_OK;
}


/**
* @internal prvTgfPtpGenAddCorrectionTimeTrafficSendAndFieldGet function
* @endinternal
*
* @brief   Send traffic and get value of correction field from capture
*
* @param[out] correctFieldValuePtr     - pointer to corrected field
*                                       None
*/
GT_VOID prvTgfPtpGenAddCorrectionTimeTrafficSendAndFieldGet
(
    OUT double  *correctFieldValuePtr
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    GT_U32      correctFieldOffset;
    GT_STATUS   expectedRcRxToCpu = GT_OK;
    char        wordStr[40];       /* buffer for double number string */

    *correctFieldValuePtr = 0;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];

    /* AUTODOC: Send PTP packet with correctionField = 0 and get it's updated
       value by capture egress packet */
    PRV_UTF_LOG0_MAC("Send Packet to Port");
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2PacketInfo,
        1 /*burstCount*/,
        0 /*numVfd*/,
        NULL /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    PRV_UTF_LOG1_MAC(
        "Capture Packet from Port [%d]",
        portInterface.devPort.portNum);

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet %d");

    correctFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 8 /* Offset in PTP header */;

    /* AUTODOC: get correctionField value of nanosecond part (48 MSBs) */
    /* AUTODOC: The correctionField holds latency of packet processing */
    *correctFieldValuePtr = ((packetBuff[correctFieldOffset] << 8) +
                            (packetBuff[correctFieldOffset+1]))* 4294967296.00 +
                            (packetBuff[correctFieldOffset+2] << 24) +
                            (packetBuff[correctFieldOffset+3] << 16) +
                            (packetBuff[correctFieldOffset+4] << 8) +
                            (packetBuff[correctFieldOffset+5]);

    cpssOsSprintf(wordStr, "%f", (*correctFieldValuePtr));
    PRV_UTF_LOG1_MAC("Capture Packet correctField in nanoseconds [%s]\n",wordStr);
}

/**
* @internal prvTgfPtpGenAddCorrectionTimeTrafficCheck function
* @endinternal
*
* @brief   Send traffic and check results
*/
GT_VOID prvTgfPtpGenAddCorrectionTimeTrafficCheck
(
    GT_VOID
)
{
    double                      correctFieldValueInNs;
    double                      correctFieldValueInNs1;
    double                      correctFieldValueDiff;
    double                      correctFieldValueDiffTolerance;
    double                      expectedLatency;
    double                      expectedDiff;

    prvTgfPtpGenAddCorrectionTimeTrafficSendAndFieldGet(&correctFieldValueInNs);

    /* AUTODOC: Change Egress Port used TAI 1 TOD to be different from Ingress port TAI 0 */
    prvTgfPtpGenAddCorrectedTimeTodSet(PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_USED_CNS);

    /* AUTODOC: The correctionField holds latency of packet processing and additional 200000 nanoseconds */
    prvTgfPtpGenAddCorrectionTimeTrafficSendAndFieldGet(&correctFieldValueInNs1);

    if (prvTgfPtpIsSingleTaiSystem())
    {
        if (prvUtfIsTrafficManagerUsed())
        {
            expectedLatency = 2282;
            expectedDiff = 200;
        }
        else
        {
            expectedLatency = 1700;
            expectedDiff = 100;
        }

        /* system with single TAI always shows latency between ingress and egress PTP units.
           Need to check that latency in proper range
           - approx 1.7 micro in Cetus without TM, 2.28 with TM
           - approx 9.800 micro in Caelum - 1G port is used
           - approx 1.150 micro in Aldrin, AC3X */
        if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            expectedLatency = 1150;
        }
        else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
        {
            expectedLatency = 9800;
            expectedDiff = 200;
        }

        if ((correctFieldValueInNs1 > (expectedLatency - expectedDiff)) && (correctFieldValueInNs1 < (expectedLatency + expectedDiff)))
        {
            PRV_UTF_LOG1_MAC(
                "Test pass OK, Correction Field Value [%d] as expected\n",(GT_U32)correctFieldValueInNs1);
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((GT_U32)expectedLatency, (GT_U32)correctFieldValueInNs1, "correctField is wrong\n");
        }
    }
    else
    {

        correctFieldValueDiff = correctFieldValueInNs1 - correctFieldValueInNs;
        correctFieldValueDiffTolerance = 400;

        /* AUTODOC: Compare two values of correctionField. Difference should be 200000 nanoseconds */
        if ((correctFieldValueDiff > (PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_USED_CNS - PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_USED_CNS - correctFieldValueDiffTolerance)) &&
            (correctFieldValueDiff < (PRV_TGF_PTP_GEN_TOD_NANOSEC_EGRESS_USED_CNS - PRV_TGF_PTP_GEN_TOD_NANOSEC_INGRESS_USED_CNS + correctFieldValueDiffTolerance)))
        {
            PRV_UTF_LOG1_MAC(
                "Test pass OK, correctFieldValueDiff [%d] as expected\n",(GT_U32)correctFieldValueDiff);
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, (GT_U32)correctFieldValueDiff, "correctField is wrong\n");
        }
    }
}

/**
* @internal prvTgfPtpGenAddCorrectedTimeCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenAddCorrectedTimeCfgRestore
(
    GT_VOID
)
{

    GT_STATUS       rc;

    rc = prvTgfPtpTsuControlSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &origTgfTsuControl);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpTsuControlSet");

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();

}
/**
* @internal prvTgfPtpGenGetEvent function
* @endinternal
*
* @brief   check events number
*/
GT_VOID prvTgfPtpGenGetEvent
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEvent,
    IN  GT_U32 eventsNum,
    IN  GT_BOOL check
)
{
    GT_U32  numOfEvents;    /*number of cpss events         */
    GT_STATUS rc;           /* return code                  */

    rc = utfGenEventCounterGet(prvTgfDevNum,
                              uniEvent,GT_TRUE,&numOfEvents);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenGetEvent , utfGenEventCounterGet");
    if (GT_TRUE == check)
    {
        /* bobcat 2 errata 4402776 and 4683196 for ermrk and per port queues: full interrupts does not work */
        if ((eventsNum != CPSS_PP_GTS_GLOBAL_FIFO_FULL_E) && (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT2_E) )
        {
          UTF_VERIFY_EQUAL3_STRING_MAC(eventsNum, numOfEvents, "prvTgfPtpGenGetEvent %d: expected num of evens: %d, actual num of events: %d\n", uniEvent,eventsNum, numOfEvents);
        }
    }
}



/**
* @internal prvTgfPtpGenEgressTimestempPortsConfig function
* @endinternal
*
* @brief   Configure tested ports for specific systems
*/
GT_VOID  prvTgfPtpGenEgressTimestempPortsConfig
(
    IN GT_U32 egressPortIndexPipe0,
    IN GT_U32 egressPortIndexPipe1
)
{
    GT_STATUS rc;               /* return code                  */

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure ports are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   egressPortIndexPipe0,
                                   egressPortIndexPipe1,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }
}

/**
* @internal prvTgfPtpGenEgressTimestempQueueInteruptsConfig function
* @endinternal
*
* @brief   set ermrk ptp queue size.
*/
GT_VOID  prvTgfPtpGenEgressTimestempQueueInteruptsConfig
(
    IN GT_U32 egressPortIndexPipe0,
    IN GT_U32 egressPortIndexPipe1
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32  ermrkQueuSize = 10;

    prvTgfPtpGenEgressTimestempPortsConfig(egressPortIndexPipe0,egressPortIndexPipe1);

    /* mask mmpcs interrupt noise */
    rc = cpssEventDeviceMaskSet(prvTgfDevNum,
                           CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                           CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenEgressTimestempQueueInteruptsConfig failed!\n");

    /*clear event counters*/
    prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,0,GT_FALSE);
    prvTgfPtpGenGetEvent(CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,0,GT_FALSE);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /*Bobcat 3 have 2 port groups and the test send 12 packets, 6 to port group 0 and 6 to port group 1*/
        ermrkQueuSize = 5;
    }

    /*Save and set ermrk timestamp queue size*/
    rc = cpssDxChPtpTsQueuesSizeGet(prvTgfDevNum,&prvTgfermrkTimestampQueueSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenEgressTimestempQueueInteruptsConfig failed!\n");

    rc = cpssDxChPtpTsQueuesSizeSet(prvTgfDevNum,ermrkQueuSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenEgressTimestempQueueInteruptsConfig failed!\n");

}

/**
* @internal prvTgfPtpGenEgressTimestempQueueInteruptsRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID  prvTgfPtpGenEgressTimestempQueueInteruptsRestore
(
    GT_VOID
)
{
    GT_STATUS rc;               /* return code           */

    /*clear event counter*/
    prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,0,GT_FALSE);
    prvTgfPtpGenGetEvent(CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,0,GT_FALSE);

    prvTgfPtpGenCfgRestore();
    prvTgfPtpGenVidAndFdbDefCfgRestoreEgress();
    /*Restore ermrk timestamp queue size*/
    rc = cpssDxChPtpTsQueuesSizeSet(prvTgfDevNum, prvTgfermrkTimestampQueueSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenEgressTimestempQueueInteruptsRestore failed!\n");

}

/**
* @internal prvTgfPtpGenAddTimeStampTagAndMirrorSet function
* @endinternal
*
* @brief   1)config ptp action to timestamp
*         2)config oam ptp table
*         3)config pcl rule to mirror and oam action
*         4)config timestamp tag format
*/
GT_VOID prvTgfPtpGenAddTimeStampTagAndMirrorSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;

    /* ptp configuration */
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry; /*ptp domain entry*/
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC localAction; /* ptp action*/

    /* oam table configuration */
    GT_U32                                oamIndex;           /* OAM entry index: oam table entry --> ptp entry */
    GT_U32                                entryIndex; /* oam table entry index: pcl rule --> oam table entry*/
    PRV_TGF_OAM_COMMON_CONFIG_STC         oamConfig; /* OAM configuration */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry; /* TS configuiration table structure */
    /* pcl configuration */
    PRV_TGF_PCL_RULE_FORMAT_UNT           mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           pattern;
    PRV_TGF_PCL_ACTION_STC                action;
    GT_U32                                ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat;
    PRV_TGF_PCL_ACTION_OAM_STC            oamAction;

    /* set global and per port configuration timestamp tag */
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;


    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    cpssOsMemSet(&localAction, 0, sizeof(localAction));
    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&oamAction, 0, sizeof(oamAction));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

    /* --------------------- ptp configuration --------------------- */

    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpEtherTypeSet");

    domainEntry.ptpOverEhernetTsEnable  = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable  = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable  = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp  =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEgressDomainTableSet");

    localAction.ingrLinkDelayEnable     = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction                = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E; /*add timestamp tag to the packet*/

    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacketPtpV2Part.messageType,
        &localAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsLocalActionTableSet");

   /* --------------------- oam table configuration ---------------------*/

    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, &origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableGet");

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    oamIndex                            = 0;
    entryIndex                          = oamIndex + 1;
    oamConfig.oamEntryIndex             = entryIndex;

    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    origOamConfig =  oamConfig;
    oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
    oamConfig.oamEntry.timestampEnable     = GT_TRUE;
    oamConfig.oamEntry.oamPtpOffsetIndex   = oamIndex;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    timeStampCfgEntry.packetFormat         = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;
    timeStampCfgEntry.offset               = 0;

    rc = prvTgfPtpTsCfgTableGet(oamIndex,&origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* --------------------- pcl configuration --------------------- */

    /* AUTODOC: init PCL Engine for sending to port 1 */
    rc = prvTgfPclDefPortInit( prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: add PCL rule 0 with following parameters: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd MIRROR TO CPU */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    oamAction.timeStampEnable   = GT_TRUE;
    oamAction.oamProcessEnable  = GT_FALSE;
    oamAction.offsetIndex       = entryIndex;
    oamAction.oamProfile        = 0;

    ruleIndex                   = 0; /* call index function client CPSS_PCL_LOOKUP_0_E ipcl*/
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd               = CPSS_PACKET_CMD_MIRROR_TO_CPU_E   ;
    action.mirror.cpuCode       = cpuCode;
    action.oam                  = oamAction;
    action.egressPolicy         = GT_FALSE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    tsTagGlobalCfg.tsTagParseEnable      = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType = 0xFFFF;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /*set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");
}

/**
* @internal prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag function
* @endinternal
*
* @brief   send traffic , get packet from the cpu and check the timestamp tag.
*/
GT_VOID prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag
(
    GT_VOID
)
{

    GT_STATUS       rc;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    static  GT_U8   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;

    prvTgfTrafficTableRxStartCapture(GT_TRUE);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2PacketInfo,
        1 /*burstCount*/, 0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(200 /*sleepTime*/);

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS];

    /* check that packet received at the CPU from the right ingress port  */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                            TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE /*getFirst*/, GT_TRUE/*trace*/, packetBuff/*the length of the user space for the packet*/,
                                           &buffLen/*packet's buffer (pre allocated by the user)*/, &packetActualLength/*Rx packet original length*/,
                                           &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed!\n");

    if ((packetBuff[12] != 0xff) || (packetBuff[13] != 0xff) || (packetBuff[28] != 0x81))
    {
        rc = GT_FAIL;
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "packetBuff[12] =  %2.2x (expected 0xff), packetBuff[13] =  %2.2x (expected 0xff), packetBuff[28] =  %2.2x (expected 0x81)",
                                     packetBuff[12], packetBuff[13], packetBuff[28]);
    }

}

/**
* @internal prvTgfPtpGenAddTimeStampTagAndMirrorrRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenAddTimeStampTagAndMirrorrRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                                oamIndex = 0; /* oam index*/

    /* Restore OAM Enabler configurations */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    /* Restore common OAM configurations */
    prvTgfOamConfigurationSet(&origOamConfig);

    /* Restore ptp configuration table */
    rc = prvTgfPtpTsCfgTableSet(oamIndex,&origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

     prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfPtpGenIngressEgressCaptureCfgSet function
* @endinternal
*
* @brief        None
*
* @param[in]    None
*/
GT_STATUS prvTgfPtpGenIngressEgressCaptureCfgSet
(
        GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    GT_U32                                oamIndex = 0;   /* OAM entry index: oam table entry --> ptp entry */

    /* set global and per port configuration timestamp tag */
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;

    /* TS configuiration table structure */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

    /* --------------------- ptp configuration --------------------- */

    /* AUTODOC: Configure classification of PTP packet */
    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEtherTypeSet");

    domainEntry.ptpOverEhernetTsEnable  = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable  = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable  = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp  =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpEgressDomainTableSet");

    /* --------------------- TS config table --------------------- */

    /* AUTODOC: Configure Add Ingress and Egress time for PTP packet by TS action table */
    timeStampCfgEntry.packetFormat         = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
    timeStampCfgEntry.offset               = 0;

    rc = prvTgfPtpTsCfgTableGet(oamIndex, &origTimeStampCfgEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsCfgTableSet");

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    /* AUTODOC: Set global configuration timestamp tag */
    tsTagGlobalCfg.tsTagParseEnable      = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType = 0xFFFF;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
                                  &tsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgSet");
    return GT_OK;
}

/**
* @internal prvTgfPtpGenIngressEgressCaptureRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenIngressEgressCaptureRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    oamIndex = 0;   /* OAM entry index: oam table entry --> ptp entry */

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore ptp configuration table */
    rc = prvTgfPtpTsCfgTableSet(oamIndex, &origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* AUTODOC: Restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Restore timestamp tag port configuration*/
    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}


/**
* @internal prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[2][256];
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam = {0};
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_U8           receiveDevNum;
    GT_U8           i;
    GT_U32          egressTime[2], ingressTime[2];
    GT_U32          egressDelay, ingressDelay;
    GT_BOOL         getFirst;
    CPSS_INTERFACE_INFO_STC portInterface;

    cpssOsMemSet(packetBuff, 0, sizeof(packetBuff));

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
    portInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];

    /* AUTODOC: Enable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset");

    rc = prvTgfTransmitPacketsWithDelay(
            prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
            &prvTgfPtpV2PacketInfo,
            2      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            125    /*Delay in miliseconds*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithDelay");

    /* AUTODOC: Disable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset");

    buffLen  = sizeof(packetBuff[i]);
    getFirst = GT_TRUE;
    for (i = 0; i<2; i++)
    {
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst /*getFirst*/, GT_TRUE /*trace*/, packetBuff[i],
                &buffLen, &packetActualLength,
                &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        tsOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
        egressTime[i] = (GT_U32)(packetBuff[i][tsOffset + 3] | (packetBuff[i][tsOffset + 2] << 8) |
                                (packetBuff[i][tsOffset + 1] << 16) | (packetBuff[i][tsOffset] << 24));

        tsOffset+=16;
        ingressTime[i] = (GT_U32)(packetBuff[i][tsOffset + 3] | (packetBuff[i][tsOffset + 2] << 8) |
                                (packetBuff[i][tsOffset + 1] << 16) | (packetBuff[i][tsOffset] << 24));
        PRV_UTF_LOG2_MAC("[TGF] - egressTime =%d, ingressTime = %d\n", egressTime[i], ingressTime[i]);
        getFirst = GT_FALSE;
    }

    /* Differance between ingress delay of 2 packets & egress delay of 2 packets,
     * should not be more than 2 milisec, ideal case should be 0 */
    /* AUTODOC: Compare two values of ingress delay and egress delay. Difference should be less than 2 miliseconds */
    egressDelay = abs(egressTime[1] - egressTime[0]);
    ingressDelay = abs(ingressTime[1] - ingressTime[0]);
    PRV_UTF_LOG2_MAC("[TGF] - egressTime diff =%d, ingressTime diff= %d\n",
            egressDelay, ingressDelay);
    if(abs(egressDelay - ingressDelay) > 2000000)
    {
        rc = GT_FAIL;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck");
}

/*
* @internal prvTgfPtpEgresssExceptionCfgSet function
* @endinternal
*
* @brief Configuration to receive PTP Packet
*        1. Enable PTP over ethernet
*        2. Set PTP local action
*        3. Set ethertype of hybrid TStag to match ingress packet format
*        4. Configure per port TS config
*        4. Set PTP egress exception Config:
*            pkt cmd  = TRAP_TO_CPU
*            CPU code = CPSS_NET_PTP_HEADER_ERROR_E
*/
GT_VOID prvTgfPtpEgresssExceptionCfgSet
(
    GT_VOID
)
{
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc = GT_OK;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC        dxchEgrExceptionCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC               tsTagPortCfg;
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC             tsTagGlobalCfg;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC           domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC         localAction;

    /* AUTODOC: Configure classification of PTP packet */
    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacket1EtherTypePart.etherType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacket1PtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacket1PtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEgressDomainTableSet");

    cpssOsMemSet(&localAction, 0, sizeof(localAction));

    localAction.ingrLinkDelayEnable     = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction                = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;

    /* AUTODOC: Configure PTP local Action */
    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacket1PtpV2Part.messageType,
        &localAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsLocalActionTableSet");

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* AUTODOC: Set Global Timestamp tag Configuration*/
    tsTagGlobalCfg.tsTagParseEnable       = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_TRUE;
    tsTagGlobalCfg.tsTagEtherType         = 0x0000;
    tsTagGlobalCfg.hybridTsTagEtherType   = 0x88F7;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Configure per port configuration timestamp tag */
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    /* AUTODOC: Configure PTP Egress Exception */
    dxchEgrExceptionCfg.invalidPtpPktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    dxchEgrExceptionCfg.invalidOutPiggybackPktCmd = CPSS_PACKET_CMD_FORWARD_E;
    dxchEgrExceptionCfg.invalidInPiggybackPktCmd  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    dxchEgrExceptionCfg.invalidInPiggybackCpuCode = CPSS_NET_PTP_HEADER_ERROR_E;
    dxchEgrExceptionCfg.invalidTsPktCmd           = CPSS_PACKET_CMD_FORWARD_E;

    /* call device specific API */
    rc = cpssDxChPtpEgressExceptionCfgSet(
        devNum, &dxchEgrExceptionCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpEgressExceptionCfgSet");
}

/*
* @internal prvTgfPtpInvalidPtpCfgSet function
* @endinternal
*
* @brief Configuration to receive PTP Packet
*        1. Enable PTP over UDP IPv4
*        2. Set destination UDP port to match packet UDP header
*        3. Set PTP local action
*        4. Set ethertype of hybrid TStag to match ingress packet format
*        5. Configure per port TS config
*        6. Set PTP egress exception Config:
*            pkt cmd  = TRAP_TO_CPU
*            CPU code = CPSS_NET_PTP_HEADER_ERROR_E
*/
GT_VOID prvTgfPtpInvalidPtpCfgSet
(
    GT_VOID
)
{
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc = GT_OK;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC        dxchEgrExceptionCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC               tsTagPortCfg;
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC             tsTagGlobalCfg;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC           domainEntry;
    PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC         localAction;

    /* AUTODOC: Configure classification of PTP packet */
    rc = prvTgfPtpOverUdpEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpUdpDestPortsSet(0,0x400);/* Dest Udp port */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpUdpDestPortsSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        CPSS_CPU_PORT_NUM_CNS,/*63*/
        4 /*domainNum*/,
        &domainEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEgressDomainTableSet");

    localAction.ingrLinkDelayEnable     = GT_FALSE;
    localAction.packetDispatchingEnable = GT_FALSE;
    localAction.tsAction                = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;

    /* AUTODOC: Configure PTP local Action */
    rc = prvTgfPtpTsLocalActionTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        prvTgfPacket1PtpV2Part.messageType,
        &localAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsLocalActionTableSet");

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* AUTODOC: Set Global Timestamp tag Configuration*/
    tsTagGlobalCfg.tsTagParseEnable       = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_TRUE;
    tsTagGlobalCfg.tsTagEtherType         = 0x0000;
    tsTagGlobalCfg.hybridTsTagEtherType   = 0x88F7;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Configure per port configuration timestamp tag */
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    /* AUTODOC: Configure PTP Egress Exception */
    dxchEgrExceptionCfg.invalidPtpPktCmd          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    dxchEgrExceptionCfg.invalidPtpCpuCode         = CPSS_NET_PTP_HEADER_ERROR_E;
    dxchEgrExceptionCfg.invalidOutPiggybackPktCmd = CPSS_PACKET_CMD_FORWARD_E;
    dxchEgrExceptionCfg.invalidInPiggybackPktCmd  = CPSS_PACKET_CMD_FORWARD_E;
    dxchEgrExceptionCfg.invalidTsPktCmd           = CPSS_PACKET_CMD_FORWARD_E;

    /* call device specific API */
    rc = cpssDxChPtpEgressExceptionCfgSet(
        devNum, &dxchEgrExceptionCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpEgressExceptionCfgSet");

    /* Mask Ip header length error */
    rc = prvTgfIpHeaderErrorMaskSet(prvTgfDevNum, PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT,
                                    CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpHeaderErrorMaskSet");
}

/**
* @internal prvTgfPtpEgressExceptionTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpEgressExceptionTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam = {0};
    GT_U8           queue    = 0;
    GT_U8           receiveDevNum;
    CPSS_NET_RX_CPU_CODE_ENT expectedCpuCode = CPSS_NET_PTP_HEADER_ERROR_E;

    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2Packet1Info,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");


    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_ANY_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, rxParam.dsaCmdIsToCpu, "Mirror to CPU Failed");

    /* Verify Packet is Trapped toCPU */
    if (rxParam.dsaCmdIsToCpu)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        expectedCpuCode,rxParam.cpuCode,"Cpu Code Check Failed");
    }
}

/**
* @internal prvTgfPtpInvalidPtpTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpInvalidPtpTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam = {0};
    GT_U8           queue    = 0;
    GT_U8           receiveDevNum;
    CPSS_NET_RX_CPU_CODE_ENT expectedCpuCode = CPSS_NET_PTP_HEADER_ERROR_E;

    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpV2Packet2Info,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");


    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_ANY_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, rxParam.dsaCmdIsToCpu, "Mirror to CPU Failed");

    /* Verify Packet is Trapped toCPU */
    if (rxParam.dsaCmdIsToCpu)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        expectedCpuCode,rxParam.cpuCode,"Cpu Code Check Failed");
    }
}

/**
* @internal prvTgfPtpEgressExceptionTestRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpEgressExceptionTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfPtpInvalidPtpCfgRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpInvalidPtpCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpTsTagPortCfgSet(CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    rc = prvTgfIpHeaderErrorMaskSet(prvTgfDevNum, PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT,
                                    CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpHeaderErrorMaskSet");

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

#if 0
/**
* @internal prvTgfPtpGenTimeStampUDBCfgSet function
* @endinternal
*
* @brief        None
*
* @param[in]    None
*/

GT_VOID prvTgfPtpGenTimeStampUDBCfgSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U8                            udbValue = 0;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           oamIndex = 0;

    /* TS configuiration table structure */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

    /* --------------------- ptp configuration --------------------- */

    /* AUTODOC: Configure classification of PTP packet */
    rc = prvTgfPtpOverEthernetEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPtpOverEthernetEnableSet");

    rc = prvTgfPtpEtherTypeSet(0, prvTgfPacketEtherTypePart.etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPtpEtherTypeSet");

    domainEntry.ptpOverEhernetTsEnable  = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable  = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable  = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp  =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpEgressDomainTableSet(
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        4 /*domainNum*/,
        &domainEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPtpEgressDomainTableSet");

    /* --------------------- TS config table --------------------- */

    /* AUTODOC: Configure Add Ingress and Egress time for PTP packet by TS action table */
    timeStampCfgEntry.packetFormat         = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
    timeStampCfgEntry.offset               = 0;

    rc = prvTgfPtpTsCfgTableGet(oamIndex, &origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPtpTsCfgTableSet");

    /* Set global and per port configuration timestamp tag */
    /* AUTODOC: init PCL Engine for sending to port 1 */
    PRV_UTF_LOG0_MAC("==== TEST of UDB ====\n");
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: configure 1 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_UDB_10_E */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offset 33 */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
            CPSS_PCL_DIRECTION_INGRESS_E,
            33,
            PRV_TGF_PCL_OFFSET_METADATA_E,
            1);

    /* AUTODOC: Add 8 PCL rules with following parameters: */
    /* AUTODOC: format PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
    /* AUTODOC: cmd CPSS_PACKET_CMD_FORWARD_E */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    ruleIndex     = appDemoDxChTcamIpclBaseIndexGet(prvTgfDevNum, 1);
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.matchCounter.enableMatchCount = GT_TRUE;

    for(udbValue=0; udbValue<8; udbValue++)
    {
        mask.ruleEgrUdbOnly.udb[0] = 0x38;
        pattern.ruleEgrUdbOnly.udb[0] = udbValue;
        action.matchCounter.matchCounterIndex = ruleIndex;
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex++, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
    }

    /* AUTODOC: Configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
            0,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure:, client %d");

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    /* AUTODOC: Set global configuration timestamp tag */
    tsTagGlobalCfg.tsTagParseEnable      = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType = 0xFFFF;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], &origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], &tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");
}

/**
* @internal prvTgfPtpGenTimeStampUDBRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenTimeStampUDBRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ruleIndex = appDemoDxChTcamIpclBaseIndexGet(prvTgfDevNum, 1);
    GT_U8     udbRules;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate PCL rules */
    for(udbRules=0; udbRules<8; udbRules++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E,
                ruleIndex++, GT_FALSE);
    }

    /* AUTODOC: Restore ptp configuration table */
    rc = prvTgfPtpTsCfgTableSet(0, &origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* AUTODOC: Restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Restore timestamp tag port configuration*/
    rc = prvTgfPtpTsTagPortCfgSet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS], &origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfPtpGenTimeStampTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenTimeStampTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    PRV_TGF_CNC_COUNTER_STC     ctr;
    GT_U32                      burstCount = 8;
    GT_U32                      i;

    /* AUTODOC: Send 8 packets with 125 miliseconds gap */
    rc = prvTgfTransmitPacketsWithDelay(
            prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
            &prvTgfPtpV2PacketInfo,
            burstCount /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            125);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTransmitPacketsWithDelay");

    for(i=0 ;i < burstCount; i++)
    {
        /* AUTODOC: verify CNC counters */
        rc = prvTgfCncCounterGet(
                prvTgfCncBlock,
                0,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
                &ctr);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCncCounterGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(1, ctr.packetCount.l[0],
                "Unexpected packet count" );
    }
}
#endif
