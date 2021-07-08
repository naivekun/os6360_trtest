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
* @file prvTgfFdbAuNaMessageExtFormat.c
*
* @brief Bridge FDB AU NA Message with new format UT.
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <bridge/prvTgfBrgGen.h>
#include <bridge/prvTgfFdbAuNaMessageExtFormat.h>
#include <appDemo/userExit/userEventHandler.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_2_CNS  2

/* VLAN 5 Id */
#define PRV_TGF_VLANID_5_CNS  5

/* VLAN 7 Id */
#define PRV_TGF_VLANID_7_CNS  7

/* User Priority value0 */
#define PRV_TGF_UP_4_CNS  4

/* User Priority value0 */
#define PRV_TGF_UP_7_CNS  7

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_1_CNS         1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_2_CNS         2

/* trunk number to send traffic to */
#define PRV_TGF_SEND_TRUNK_ID_127_CNS       127

/* not existed device number */
#define PRV_TGF_DEV_NUM_8_CNS               8

/* number of iterations in the test */
#define PRV_AU_NA_ITERAT_NUM_CNS            4

#define PRV_AU_NA_MULTI_HASH_ITERAT_NUM_CNS 16

/* Maximal number of messages in AUQ */
#define PRV_TGF_AU_MAX_MESSAGES_CNS         (2048 + 64)

/* Number of packets sent before WA */
#define PRV_TGF_NEW_MAC_ADDRESS_SENT_CNS    4

extern GT_STATUS   appDemoAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
);

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* macro to trace mac address */
#define TGF_TRACE_MAC_ADDR_MAC(name, value)                                    \
    PRV_UTF_LOG7_MAC("%s [%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x]\n", #name,      \
                     value[0],value[1],value[2],value[3],value[4],value[5])


static GT_VOID prvTgfBrgFdbMacAddrByteIncrement
(
    IN GT_ETHERADDR * macAddrPtr,
    IN GT_U32 byteOffset
);

static GT_VOID prvTgfBrgFdbAuMacRangeCheck
(
    IN CPSS_MAC_UPDATE_MSG_EXT_STC     *auMessagesPtr,
    IN GT_U32  numberOfAuMessages
);

/******************************* Test packet **********************************/

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}};

/* L2 part of packet - for restore */
static TGF_PACKET_L2_STC prvTgfPacketL2PartBackup = {
    PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_DA_CNS,               /* dstMac */
    PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_SA_CNS                /* srcMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_DA_CNS,               /* dstMac */
    PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_SA_CNS                /* srcMac */
};

/* Unicast unknown da mac */
static TGF_MAC_ADDR     prvTgfUcastUnknownMac =
    PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_DA_CNS;

/* Unicast da mac */
static TGF_MAC_ADDR     prvTgfUcastKnownMac =
    {0x00, 0x00, 0x00, 0x44, 0x55, 0x44};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    PRV_TGF_UP_4_CNS, 0, PRV_TGF_VLANID_2_CNS           /* pri, cfi, VlanId */
};


/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/************************ Double tagged packet ********************************/
/* PARTS of double tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketDoubleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of double tagged packet with CRC */
#define PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Double tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoDoubleTag =
{
    PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketDoubleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDoubleTagPartArray                                        /* partsArray */
};

/************************ Single tagged packet ********************************/

/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSingleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_SINGLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSingleTag =
{
    PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketSingleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSingleTagPartArray                                        /* partsArray */
};

/******************************* WA Test packet **********************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             2

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbAuNaMessageExtFormatCfg function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] isPktDoubleTagged        - GT_TRUE: packet is double tagged
*                                      - GT_FALSE: packet is single tagged
* @param[in] up0                      - User priority of the Tag0
*                                      vlan1               - vlan Id of the Tag1
* @param[in] isDaMacNew               - GT_TRUE: DA MAC is unknown unicast
*                                      GT_FALSE: DA MAC is known unicast
* @param[in] srcId                    - source Id of the address
* @param[in] dstInterfacePtr          - The associated information with the entry
*                                       None
*/
static GT_VOID prvTgfFdbAuNaMessageExtFormatCfg
(
    IN GT_BOOL                  isPktDoubleTagged,
    IN TGF_PRI                  up0,
    IN GT_U16                   vid1,
    IN GT_BOOL                  isDaMacNew,
    IN GT_U32                   srcId,
    IN CPSS_INTERFACE_INFO_STC  *dstInterfacePtr
)
{
    GT_STATUS                   rc;
    PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfMacEntry;

    if (isPktDoubleTagged == GT_TRUE)
    {
        /* AUTODOC: create VLAN with VID(i) (for Double Tagged traffic) */
        rc = prvTgfBrgDefVlanEntryWrite(vid1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, vid1);

        /* AUTODOC: enable vid1 assignment (for Double Tagged traffic) */
        rc = prvTgfBrgFdbNaMsgVid1EnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaMsgVid1EnableSet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: set TAG1 = VID(i) for Double Tagged traffic */
        prvTgfPacketVlanTag1Part.vid = vid1;
    }

    /* AUTODOC: set User Priority of Tag0 to UP(i) */
    prvTgfPacketVlanTag0Part.pri = up0;

    if (isDaMacNew)
    {
        /* Set destination addres to unknown unicast */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfUcastUnknownMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }
    else
    {
        /* Set destination addres to known unicast */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfUcastKnownMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    if (isDaMacNew)
    {
        prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_2_CNS;
        cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther,
                     prvTgfPacketL2Part.saMac,
                     sizeof(TGF_MAC_ADDR));

        /* Delete the SA mac address from the previous iteration , because we may
           work in mode of 'SP filter enabled' */
        rc = prvTgfBrgFdbMacEntryDelete(&prvTgfMacEntry.key);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
    }
    else
    {
        prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_2_CNS;
        prvTgfMacEntry.dstInterface.type            = dstInterfacePtr->type;
        if (prvTgfMacEntry.dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            /* Dev/Port */
            prvTgfMacEntry.dstInterface.devPort.hwDevNum  = dstInterfacePtr->devPort.hwDevNum;
            prvTgfMacEntry.dstInterface.devPort.portNum = dstInterfacePtr->devPort.portNum;
        }
        else
        {
            /* Trunk */
            prvTgfMacEntry.dstInterface.trunkId = dstInterfacePtr->trunkId;
        }

        prvTgfMacEntry.sourceId                     = srcId;
        prvTgfMacEntry.isStatic                     = GT_FALSE;
        prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
        prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
        prvTgfMacEntry.daRoute                      = GT_FALSE;
        prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
        prvTgfMacEntry.userDefined                  = 0;
        prvTgfMacEntry.daQosIndex                   = 0;
        prvTgfMacEntry.saQosIndex                   = 0;
        prvTgfMacEntry.daSecurityLevel              = 0;
        prvTgfMacEntry.saSecurityLevel              = 0;
        prvTgfMacEntry.appSpecificCpuCode           = GT_FALSE;
        prvTgfMacEntry.pwId                         = 0;
        prvTgfMacEntry.spUnknown                    = GT_FALSE;

        cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther,
                     prvTgfUcastKnownMac,
                     sizeof(TGF_MAC_ADDR));
        /* Add static MAC Entry */
        rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
    }

    /* AUTODOC: disable learning and allow accumulation of AU messages */
    rc = appDemoAllowProcessingOfAuqMessages(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");
}

/**
* @internal prvTgfFdbAuNaMessageExtFormatPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] isPktDoubleTagged        - GT_TRUE: packet is double tagged
*                                      GT_FALSE: packet is single tagged
* @param[in] portNum                  - port number to send packet from
*                                       None
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatPacketSend
(
    IN GT_BOOL  isPktDoubleTagged,
    IN GT_U32    portNum
)
{
    GT_STATUS       rc;
    GT_U8           portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if (isPktDoubleTagged == GT_TRUE)
    {
        /* setup Packet with double tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoDoubleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevNum, prvTgfBurstCount, 0, NULL);
    }
    else
    {

        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevNum, prvTgfBurstCount, 0, NULL);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

}


/**
* @internal prvTgfFdbAuNaMessageExtFormatCheckAuFdbEntryOffset function
* @endinternal
*
* @brief   Get MAC entry.
*/
static GT_VOID prvTgfFdbAuNaMessageExtFormatCheckAuFdbEntryOffset
(
    IN GT_U32                      entryOffset,
    IN CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          actNumOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC     auMessages[1];
    GT_U32                          numOfAu = 1; /* each time only one message
                                                    is expected */

    if (auMessagesPtr == NULL)
    {
        auMessagesPtr = &auMessages[0];
    }

    actNumOfAu = numOfAu;

    /* get first NA MAC ADDR message */
    do
    {
        rc = prvTgfBrgFdbAuMsgBlockGet(prvTgfDevNum, &actNumOfAu, auMessagesPtr);
        if (rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
        }
    } while ((auMessagesPtr->updType != CPSS_NA_E)
             || (auMessagesPtr->macEntry.key.entryType != CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E));

    if (actNumOfAu == numOfAu)
    {
        /* Check the received message */

        /* check entry offset */
        UTF_VERIFY_EQUAL2_STRING_MAC(auMessagesPtr->entryOffset, entryOffset, "The entry offset of the AU message is not as expected: %d, %d",
                                     auMessagesPtr->entryOffset, entryOffset);
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfAu, actNumOfAu, "The number of received messages is wrong: %d, %d",
                                 actNumOfAu, numOfAu);
    }
}

/**
* @internal prvTgfFdbAuNaMessageExtFormatCheckAuMessage function
* @endinternal
*
* @brief   Get MAC entry.
*
* @param[in] isPktDoubleTagged        - GT_TRUE: packet is double tagged
*                                      - GT_FALSE: packet is single tagged
* @param[in] up0                      - User priority of the Tag0
*                                      vlan1               - vlan Id of the Tag1
* @param[in] isDaMacNew               - GT_TRUE: DA MAC is unknown unicast
*                                      GT_FALSE: DA MAC is known unicast
* @param[in] oldSrcId                 - old source Id of the address
* @param[in] oldDstInterfacePtr       - The aold ssociated information with the entry
* @param[in] oldAssociatedHwDevNum    - The old HW device number the entry is associated with
*                                       None
*/
static GT_VOID prvTgfFdbAuNaMessageExtFormatCheckAuMessage
(
    IN GT_BOOL                  isPktDoubleTagged,
    IN GT_U32                   up0,
    IN GT_U16                   vid1,
    IN GT_BOOL                  isDaMacNew,
    IN GT_U32                   oldSrcId,
    IN CPSS_INTERFACE_INFO_STC  *oldDstInterfacePtr,
    IN GT_HW_DEV_NUM             oldAssociatedHwDevNum
)
{
    GT_STATUS                       rc;
    GT_U32                          actNumOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC     auMessages[1] , *auMessagesPtr = &auMessages[0];
    GT_U32                          numOfAu = 1; /* each time only one message
                                                    is expected */

    actNumOfAu = numOfAu;

    /* get AU messages */
    rc = prvTgfBrgFdbAuMsgBlockGet(prvTgfDevNum, &actNumOfAu, auMessagesPtr);
    if (rc != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
    }

    if (actNumOfAu == numOfAu)
    {
        /* Check the received message */

        /* check up0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(up0, auMessagesPtr->up0, "The up0 of the AU message is not as expected.");

        if (isPktDoubleTagged == GT_TRUE)
        {
            /* Bobcat2 A0 has erratum (L2I-497) and VID1 is always 0 in NA messages.
               The erratum was fixed started from Bobcat2 B0 devices. */
            vid1 = (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) ||
                    (!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))) ? vid1 : 0;

            /* check vid1 */
            UTF_VERIFY_EQUAL0_STRING_MAC(vid1, auMessagesPtr->vid1,  "The vid1 of the AU message is not as expected.");
        }

        if (isDaMacNew == GT_TRUE)
        {
            /* check that isMoved == GT_FALSE */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, auMessagesPtr->isMoved, "The 'isMoved' of the AU message is not as expected.");

        }
        else
        {
            /* check that isMoved == GT_TRUE */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, auMessagesPtr->isMoved, "The 'isMoved' of the AU message is not as expected.");
            /* check oldSrcId */
            UTF_VERIFY_EQUAL0_STRING_MAC(oldSrcId, auMessagesPtr->oldSrcId, "The 'oldSrcId' of the AU message is not as expected.");
            /* check old associated devNum */
            UTF_VERIFY_EQUAL0_STRING_MAC(oldAssociatedHwDevNum, auMessagesPtr->oldAssociatedHwDevNum,
                                         "The 'oldAssociatedHwDevNum' of the AU message is not as expected.");

            if (oldDstInterfacePtr->type == CPSS_INTERFACE_PORT_E)
            {
                /* check old devNum and portNum */
                UTF_VERIFY_EQUAL0_STRING_MAC(oldDstInterfacePtr->devPort.hwDevNum, auMessagesPtr->oldDstInterface.devPort.hwDevNum,
                                             "The 'devNum' of the AU message is not as expected.");

                UTF_VERIFY_EQUAL0_STRING_MAC(oldDstInterfacePtr->devPort.portNum, auMessagesPtr->oldDstInterface.devPort.portNum,
                                             "The 'portNum' of the AU message is not as expected.");
            }
            else
            {
                /* check old trunk */
                UTF_VERIFY_EQUAL0_STRING_MAC(oldDstInterfacePtr->trunkId, auMessagesPtr->oldDstInterface.trunkId,
                                             "The 'trunkId' of the AU message is not as expected.");
            }
        }
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfAu, actNumOfAu, "The number of received messages is wrong: %d, %d",
                                     actNumOfAu, numOfAu);
    }
}


/**
* @internal prvTgfFdbAuNaMessageSingleHashIndexTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfFdbAuNaMessageSingleHashIndexTrafficGenerate
(
    IN GT_U32                    ingressPort,
    IN TGF_MAC_ADDR             srcMac,
    IN GT_U32                   entryOffset
)
{
    GT_STATUS                       rc;             /* returned status */
    /* L2 part of packet */
    TGF_PACKET_L2_STC prvTgfBcPacketL2Part = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                /* srcMac */
    };

    CPSS_INTERFACE_INFO_STC         dstInterface;
    CPSS_MAC_UPDATE_MSG_EXT_STC     auMessage;
    PRV_TGF_MAC_ENTRY_KEY_STC       macEntryKey;
    GT_U32                          incrementAmount = 1;
    GT_U32                          i;

    /* clear entry */
    cpssOsMemSet(&dstInterface, 0, sizeof(dstInterface));

    if(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.fdbHashParams.hashMode == CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
    {
        for (i = 0; (i < incrementAmount); i++)
        {
            rc = prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, entryOffset, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", entryOffset);
        }
    }

    /* copy source MAC to packet structure */
    cpssOsMemCpy(prvTgfBcPacketL2Part.saMac, srcMac, sizeof(TGF_MAC_ADDR));

    prvTgfPacketSingleTagPartArray[0].partPtr = &prvTgfBcPacketL2Part;

    /* Configure AU NA Message */
    prvTgfFdbAuNaMessageExtFormatCfg(GT_FALSE,
                                     PRV_TGF_UP_4_CNS,
                                     PRV_TGF_VLANID_2_CNS,
                                     GT_TRUE,
                                     0,
                                     &dstInterface);

    /* AUTODOC: send packet on Port(i) */
    prvTgfFdbAuNaMessageExtFormatPacketSend(GT_FALSE, ingressPort);

    /* give time for device/simulation for packet processing */
    cpssOsTimerWkAfter(10);

    /* check AU message */
    auMessage.updType = CPSS_AA_E; /* stamp - NA only expected */
    prvTgfFdbAuNaMessageExtFormatCheckAuFdbEntryOffset(entryOffset, &auMessage);

    /* AUTODOC: enable learning */
    rc = appDemoAllowProcessingOfAuqMessages(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.fdbHashParams.hashMode == CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
    {
        for (i = 0; (i < incrementAmount); i++)
        {
            rc = prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, entryOffset, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", entryOffset);
        }

        /* for multi hash must be cleaned - FDB bank counters used to cause bank usising */
        /* for other cases the busy FDB entry causes using of another bank               */
        if ((auMessage.updType == CPSS_NA_E)
            && (auMessage.macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E))
        {
            /* remove entry from FDB - it should not be added but probably due */
            /* to Falcon GM bug it added to FDB                                */
            macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
            macEntryKey.key.macVlan.vlanId  = auMessage.macEntry.key.key.macVlan.vlanId;
            macEntryKey.key.macVlan.macAddr = auMessage.macEntry.key.key.macVlan.macAddr;
            rc = prvTgfBrgFdbMacEntryDelete(&macEntryKey);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryDelete failed\n");
        }
    }


    /* set original MAC info to packet structure */
    prvTgfPacketSingleTagPartArray[0].partPtr = &prvTgfPacketL2Part;
}

/**
* @internal prvTgfFdbAuNaMessageExtFormatTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*
* @param[in] isPktDoubleTagged        - GT_TRUE: packet is double tagged
*                                      - GT_FALSE: packet is single tagged
* @param[in] up0                      - User priority of the Tag0
*                                      vlan1               - vlan Id of the Tag1
* @param[in] isDaMacNew               - GT_TRUE: DA MAC is unknown unicast
*                                      GT_FALSE: DA MAC is known unicast
* @param[in] oldSrcId                 - old source Id of the address
* @param[in] oldDstInterfacePtr       - The aold ssociated information with the entry
* @param[in] oldAssociatedHwDevNum    - The old HW device number the entry is associated with
* @param[in] ingressPort              - The port number to send packet from
*                                       None
*/
static GT_VOID prvTgfFdbAuNaMessageExtFormatTrafficGenerate
(
    IN GT_BOOL                  isPktDoubleTagged,
    IN TGF_PRI                  up0,
    IN GT_U16                   vid1,
    IN GT_BOOL                  isDaMacNew,
    IN GT_U32                   oldSrcId,
    IN CPSS_INTERFACE_INFO_STC  *oldDstInterfacePtr,
    IN GT_HW_DEV_NUM            oldAssociatedHwDevNum,
    IN GT_U32                    ingressPort
)
{
    GT_STATUS                       rc;             /* returned status */

    /* Configure AU NA Message */
    prvTgfFdbAuNaMessageExtFormatCfg(isPktDoubleTagged,
                                     up0,
                                     vid1,
                                     isDaMacNew,
                                     oldSrcId,
                                     oldDstInterfacePtr);

    /* AUTODOC: send packet on Port(i) */
    prvTgfFdbAuNaMessageExtFormatPacketSend(isPktDoubleTagged, ingressPort);

    /* give time for device/simulation for packet processing */
    cpssOsTimerWkAfter(10);

    /* check AU message */
    prvTgfFdbAuNaMessageExtFormatCheckAuMessage(isPktDoubleTagged,
                                                up0,
                                                vid1,
                                                isDaMacNew,
                                                oldSrcId,
                                                oldDstInterfacePtr,
                                                oldAssociatedHwDevNum);

    if (isPktDoubleTagged == GT_TRUE)
    {
        /* AUTODOC: invalidate vlan entry VID(i) (after double tagged packet) */
        rc = prvTgfBrgDefVlanEntryInvalidate(vid1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, vid1);

        /* AUTODOC: restore vid assignment to be from Tag0 (after double tagged packet) */
        rc = prvTgfBrgFdbNaMsgVid1EnableSet(prvTgfDevNum, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaMsgVid1EnableSet: %d",
                                     prvTgfDevNum);
    }

    /* AUTODOC: enable learning */
    rc = appDemoAllowProcessingOfAuqMessages(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbAuNaMessageExtFormatConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}

/**
* @internal prvTgfFdbAuNaMessageExtFormatWithFdbBankIndex function
* @endinternal
*
* @brief   Disable learning.
*         Send tagged packet with unknown unicast.
*         Generate traffic:
*         Send tagged packet to be learned on diferent FDB banks:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: entryOffset (FDB bank index).
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatWithFdbBankIndex
(
    void
)
{
    GT_U32               ingressPort;
    TGF_MAC_ADDR        srcMacArr[PRV_AU_NA_MULTI_HASH_ITERAT_NUM_CNS];
    GT_U32              ii;
    GT_STATUS           rc;
    GT_U32              numIter;
    PRV_TGF_MAC_ENTRY_KEY_STC       macEntryKey;

    /* AUTODOC: Generate MAC addresses for all banks: */
    rc = prvTgfBrgFdbMacBankArrayGet(prvTgfDevNum, srcMacArr, &numIter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacBankArrayGet");

    /* AUTODOC: Generate traffic: */
    for (ii = 0; ii < numIter; ii++)
    {
        ingressPort = prvTgfPortsArray[ii % 4];

        if(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.supportSingleFdbInstance == GT_FALSE)
        {
            /* use single core for all entries , because each core to it's own indexes */
            ingressPort = prvTgfPortsArray[3];
        }

        /* AUTODOC: start iteration 0..4: */
        prvTgfFdbAuNaMessageSingleHashIndexTrafficGenerate(ingressPort, srcMacArr[ii], ii);
        /* AUTODOC: verify AU NA message updated correctly including extended parameters: VID1, UP0 and isMoved */
        /* AUTODOC: end iteration */
    }

    /* if FDB shadow used the learned MACs not cleaned from FDB */
    for (ii = 0; ii < numIter; ii++)
    {
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId  = prvTgfPacketVlanTag0Part.vid;
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, srcMacArr[ii], sizeof(TGF_MAC_ADDR));
        rc = prvTgfBrgFdbMacEntryDelete(&macEntryKey);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryDelete failed\n");
    }
}

/**
* @internal prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddrTrafficGenerate function
* @endinternal
*
* @brief   Disable learning.
*         Send double tagged packet with unknown unicast.
*         Generate traffic:
*         Send from port1 double tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: vid1, up0 and isMoved.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddrTrafficGenerate
(
    void
)
{
    GT_BOOL     isPktDoubleTagged = GT_TRUE;
    TGF_PRI     up0;
    GT_U16      vid1;
    GT_BOOL     isDaMacNew = GT_TRUE;
    GT_U32      srcId;
    CPSS_INTERFACE_INFO_STC  dstInterface;
    GT_HW_DEV_NUM            associatedHwDevNum = 0;
    GT_U32       ingressPort;
    TGF_PRI     up0Arr[PRV_AU_NA_ITERAT_NUM_CNS]   = { 4,    7,  5,    0};
    GT_U16      vid1Arr[PRV_AU_NA_ITERAT_NUM_CNS]  = { 5, 4094,  3, 2789};
    GT_U32      srcIdArr[PRV_AU_NA_ITERAT_NUM_CNS] = { 9,   19, 31,    0};
    GT_U32      ii;
    GT_BOOL     vid1EnableRestore = GT_FALSE;
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT     muxingModeRestore = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E;
    GT_STATUS   rc;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear entry */
    cpssOsMemSet(&dstInterface, 0, sizeof(dstInterface));

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeGet(&muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
    else
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* eArch/SIP_5 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbVid1AssignmentEnableGet(&vid1EnableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbVid1AssignmentEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableSet: %d", prvTgfDevNum);
    }


    for (ii = 0; ii < PRV_AU_NA_ITERAT_NUM_CNS; ii++)
    {
        /* AUTODOC: configure following parameters for iteration: */
        /* AUTODOC:   UP(i) = [4, 7, 5, 0] */
        /* AUTODOC:   VID(i) = [5, 4094, 3, 2789] */
        /* AUTODOC:   SrcID(i) = [9, 19, 31, 0] */
        /* AUTODOC:   Port(i) = [0, 1, 2, 3] */
        /* AUTODOC: setup Double Tagged packet: */
        /* AUTODOC:  DA = 00:00:00:00:11:AA, SA = 00:00:00:00:11:AA */
        /* AUTODOC:  TAG0 = 2, TAG1 = VID(i) */
        up0 = up0Arr[ii];
        vid1 = vid1Arr[ii];
        srcId = srcIdArr[ii];
        ingressPort = prvTgfPortsArray[ii];

        /* AUTODOC: start iteration 0..4: */
        prvTgfFdbAuNaMessageExtFormatTrafficGenerate(isPktDoubleTagged,
                                                     up0,
                                                     vid1,
                                                     isDaMacNew,
                                                     srcId,
                                                     &dstInterface,
                                                     associatedHwDevNum,
                                                     ingressPort);
        /* AUTODOC: verify AU NA message updated correctly including extended parameters: VID1, UP0 and isMoved */
        /* AUTODOC: end iteration */
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
    else
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        rc = prvTgfBrgFdbVid1AssignmentEnableSet(vid1EnableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableSet: %d", prvTgfDevNum);
    }
}


/**
* @internal prvTgfFdbAuNaMessageExtFormatDueToMovedPortTrafficGenerate function
* @endinternal
*
* @brief   Learn MAC on FDB on port2.
*         Disable learning.
*         Send single tagged packet with known unicast, but from different
*         source (port).
*         Generate traffic:
*         Send from port1 single tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: up0, isMoved oldSrcId, oldDstInterface and
*         oldAssociatedHwDevNum.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatDueToMovedPortTrafficGenerate
(
    void
)
{
    GT_BOOL     isPktDoubleTagged = GT_FALSE;
    TGF_PRI     up0;
    GT_BOOL     isDaMacNew = GT_FALSE;
    GT_U32      srcId;
    CPSS_INTERFACE_INFO_STC  dstInterface;
    GT_HW_DEV_NUM            associatedHwDevNum;
    GT_U32       ingressPort;
    TGF_PRI     up0Arr[PRV_AU_NA_ITERAT_NUM_CNS]   = {    3,    6,    4,    1};
    GT_U32      srcIdArr[PRV_AU_NA_ITERAT_NUM_CNS] = {   11,   23,   30,    1};
    GT_U32      ii;
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT     muxingModeRestore = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E;
    GT_STATUS   rc;

    /* AUTODOC: GENERATE TRAFFIC: */

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require SRC_ID mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeGet(&muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }

    /* clear entry */
    cpssOsMemSet(&dstInterface, 0, sizeof(dstInterface));

    dstInterface.type = CPSS_INTERFACE_PORT_E;
    dstInterface.devPort.hwDevNum = prvTgfDevNum;
    associatedHwDevNum = dstInterface.devPort.hwDevNum;

    for (ii = 0; ii < PRV_AU_NA_ITERAT_NUM_CNS; ii++)
    {
        /* AUTODOC: configure following parameters for iteration: */
        /* AUTODOC:   UP(i) = [3, 6, 4, 1] */
        /* AUTODOC:   SrcID(i) = [11, 23, 30, 1] */
        /* AUTODOC:   Port(i) = [1, 2, 3, 0] */
        /* AUTODOC: setup Tagged packet: */
        /* AUTODOC:  DA = 00:00:00:00:11:AA, SA = 00:00:00:44:55:44 */
        /* AUTODOC:  TAG0 = 2 */
        /* AUTODOC: add FDB entry with MAC 00:00:00:44:55:44, VLAN 2, port Port(i) */
        up0 = up0Arr[ii];
        srcId = srcIdArr[ii];
        dstInterface.devPort.portNum = prvTgfPortsArray[ii];
        ingressPort = prvTgfPortsArray[(ii + 1) % 4];

        /* AUTODOC: start iteration 0..4: */
        prvTgfFdbAuNaMessageExtFormatTrafficGenerate(isPktDoubleTagged,
                                                 up0,
                                                 0,
                                                 isDaMacNew,
                                                 srcId,
                                                 &dstInterface,
                                                 associatedHwDevNum,
                                                 ingressPort);
        /* AUTODOC: verify AU NA message updated correctly including extended parameters: */
        /* AUTODOC:   up0, isMoved oldSrcId, oldDstInterface and oldAssociatedHwDevNum */
        /* AUTODOC: end iteration */
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
}



/**
* @internal prvTgfFdbAuNaMessageExtFormatDueToMovedTrunkTrafficGenerate function
* @endinternal
*
* @brief   Learn MAC on FDB on trunk.
*         Disable learning.
*         Send single tagged packet with known unicast, but from different
*         source (port).
*         Generate traffic:
*         Send from port1 single tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: up0, isMoved oldSrcId, oldDstInterface and
*         oldAssociatedDevNum.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatDueToMovedTrunkTrafficGenerate
(
    void
)
{
    GT_BOOL     isPktDoubleTagged = GT_FALSE;
    TGF_PRI     up0;
    GT_BOOL     isDaMacNew = GT_FALSE;
    GT_U32      srcId;
    CPSS_INTERFACE_INFO_STC  dstInterface;
    GT_U8       associatedDevNum;
    GT_U32       ingressPort;
    TGF_PRI     up0Arr[PRV_AU_NA_ITERAT_NUM_CNS]      = {  0,   2,  7,  3};
    GT_U32      srcIdArr[PRV_AU_NA_ITERAT_NUM_CNS]    = {  3,  28, 11,  7};
    GT_TRUNK_ID trunkIdArr[PRV_AU_NA_ITERAT_NUM_CNS]  = { 23, 127,  1, 58};
    GT_U32      ii;
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT     muxingModeRestore = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E;
    GT_STATUS   rc;

    /* AUTODOC: GENERATE TRAFFIC: */

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require SRC_ID mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeGet(&muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }

    /* clear entry */
    cpssOsMemSet(&dstInterface, 0, sizeof(dstInterface));
    dstInterface.type = CPSS_INTERFACE_TRUNK_E;

    /* associated devNum is set by CPSS to be HW devNum */
    associatedDevNum = 0;


    for (ii = 0; ii < PRV_AU_NA_ITERAT_NUM_CNS; ii++)
    {
        /* AUTODOC: configure following parameters for iteration: */
        /* AUTODOC:   UP(i) = [0, 2, 7, 3] */
        /* AUTODOC:   SrcID(i) = [3, 28, 11, 7] */
        /* AUTODOC:   TrunkID(i) = [23, 127, 1, 58] */
        /* AUTODOC:   Port(i) = [0, 1, 2, 3] */
        /* AUTODOC: setup Tagged packet: */
        /* AUTODOC:  DA = 00:00:00:00:11:AA, SA = 00:00:00:44:55:44 */
        /* AUTODOC:  TAG0 = 2 */
        /* AUTODOC: add FDB entry with MAC 00:00:00:44:55:44, VLAN 2, trunk TrunkID(i) */
        up0 = up0Arr[ii];
        srcId = srcIdArr[ii];
        dstInterface.trunkId = trunkIdArr[ii];
        ingressPort = prvTgfPortsArray[ii];

        /* AUTODOC: start iteration 0..4: */
        prvTgfFdbAuNaMessageExtFormatTrafficGenerate(isPktDoubleTagged,
                                                 up0,
                                                 0,
                                                 isDaMacNew,
                                                 srcId,
                                                 &dstInterface,
                                                 associatedDevNum,
                                                 ingressPort);
        /* AUTODOC: verify AU NA message updated correctly including extended parameters: */
        /* AUTODOC:   up0, isMoved oldSrcId, oldDstInterface and oldAssociatedDevNum */
        /* AUTODOC: end iteration */
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
}


/**
* @internal prvTgfFdbAuNaMessageExtFormatConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatConfigurationRestore
(
    void
)
{
    GT_STATUS   rc;

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
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
    /* restore variable used for several tests*/
    prvTgfPacketL2Part = prvTgfPacketL2PartBackup;

}
/**
* @internal prvTgfFdbAuNaMessageSendTrafic function
* @endinternal
*
* @brief   Send BC traffic from each port to setup FDB table
*/
GT_VOID prvTgfFdbAuNaMessageSendTrafic
(
    void
)
{
    GT_STATUS       rc           = GT_OK;
    GT_U32          portIter     = 0;
    GT_U32          portsCount   = prvTgfPortsNum;

    /* -------------------------------------------------------------------------
     * 1. brodcast phase - send brodcast packet from each port to setup FDB table
     */
    PRV_UTF_LOG0_MAC("======= Sending brodcast packets to setup FDB table =======\n");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* send Packet from each port */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* set source MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[portIter],
                     sizeof(prvTgfPacketL2Part.saMac));

        /* send Packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfFdbAuFdbFlushTriggerAction function
* @endinternal
*
* @brief   Start FDB flush by cpssDxChBrgFdbTrigActionStart and check action finish
*/
static GT_VOID prvTgfFdbAuFdbFlushTriggerAction
(
    GT_BOOL expectedSuccess
)
{
    GT_STATUS                   rc;

    /* Flush FDB */
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_DELETING_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionStart");


    if(expectedSuccess == GT_TRUE)
    {
        /* Wait for finishing action - action finished */
        rc = prvTgfBrgFdbActionDoneWait(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionDoneWait");
    }
    else
    {
        /* Wait for finishing action - action not finished */
        rc = prvTgfBrgFdbActionDoneWait(GT_TRUE);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionDoneWait");
    }
}

/**
* @internal prvTgfFdbAuFillQueues function
* @endinternal
*
* @brief   Fill primary AUQ by QA messages.
*
* @param[in] macEntryKeyPtr           - pointer to source MAC address entry
* @param[in,out] numOfAuMessagesPtr       - pointer to number of QA messages to send to device
* @param[in,out] numOfAuMessagesPtr       - pointer to number of QA messages sended to device
*                                       None
*
* @note Disable handling of AU and send 2K + 64 incremental MAC
*       to the hardware MAC address.
*
*/
static GT_VOID prvTgfFdbAuFillQueues
(
    IN PRV_TGF_MAC_ENTRY_KEY_STC   * macEntryKeyPtr,
    INOUT GT_U32 * numOfAuMessagesPtr
)
{
    GT_U32                      ii, kk;
    GT_U32                      auSize;
    GT_STATUS                   rc;
    GT_BOOL                     origByMessageMustSucceed;
    GT_BOOL                     origByMessageMustCheck;

    /* Disable learning and allow accumulation of AU messages */
    rc = appDemoAllowProcessingOfAuqMessages(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");

    origByMessageMustSucceed = prvTgfBrgFdbEntryByMessageMustSucceedSet(GT_FALSE);
    origByMessageMustCheck = prvTgfBrgFdbEntryByMessageStatusCheckSet(GT_FALSE);

    auSize = *numOfAuMessagesPtr;
    for (ii = 0; ii < auSize; ii++)
    {
        kk = 0;

        do
        {
            rc = prvTgfBrgFdbQaSend(macEntryKeyPtr);
            if(rc != GT_OK)
            {
#ifdef ASIC_SIMULATION
                cpssOsTimerWkAfter(1);
#endif
                kk++;
                if(kk > 20)
                {

                    PRV_UTF_LOG1_MAC("prvTgfFdbAuStuckWaTest status [%d]\n", kk);
                    break;
                }
            }
        }while(rc != GT_OK);

        if(rc == GT_BAD_STATE)
        {
            /* AUQ is full */
            break;
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(
                                    GT_OK, rc, "prvTgfBrgFdbQaSend: iteration %d", ii);

        prvTgfBrgFdbMacAddrByteIncrement(&macEntryKeyPtr->key.macVlan.macAddr, 5);
    }

    PRV_UTF_LOG1_MAC("prvTgfFdbAuStuckWaTest done on iteration[%d]\n", ii);

    *numOfAuMessagesPtr = ii;

    /* Restore previous state of the flags */
    prvTgfBrgFdbEntryByMessageMustSucceedSet(origByMessageMustSucceed);
    prvTgfBrgFdbEntryByMessageStatusCheckSet(origByMessageMustCheck);
}

GT_VOID prvTgfFdbAuStuckWaReadAuqMessages
(
    GT_U32                      numOfWaAu,
    GT_BOOL                     expectRewind
)
{
    GT_BOOL                     rewindAuq;      /* AUQ rewind indication */
    GT_U32                      auIndex;        /* AUQ message array index */
    GT_U32                      tempNumOfWaAu;  /* Number of AU messages per iteration */
    GT_STATUS                   rc;
    GT_U16                      portGroupNum;   /* Number of port groups */
    CPSS_MAC_UPDATE_MSG_EXT_STC  *auMessagesPtr; /* Pointer to AUQ messages array */


    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,PRV_CPSS_DXCH_UNIT_FDB_E,&portGroupNum);
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    /* Total number of AUQ messages to read from all port groups */
    numOfWaAu *= portGroupNum;

    auMessagesPtr  = (CPSS_MAC_UPDATE_MSG_EXT_STC*)
                cpssOsMalloc(sizeof(CPSS_MAC_UPDATE_MSG_EXT_STC) * numOfWaAu);

    cpssOsMemSet(auMessagesPtr, 0, (sizeof(CPSS_MAC_UPDATE_MSG_EXT_STC) * numOfWaAu));

    for(auIndex = 0; auIndex < numOfWaAu; auIndex++)
    {
        /* Get AUQ messages one-by-one */
        tempNumOfWaAu = 1;
        rc = prvTgfBrgFdbAuMsgBlockGet(prvTgfDevNum, &tempNumOfWaAu, &auMessagesPtr[auIndex]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbAuMsgBlockGet FAILED, rc = [%d]", rc);
            /* free allocated memory */
            cpssOsFree(auMessagesPtr);

            return;
        }
    }

    /* Get primary AUQ rewind status */
    rc = prvTgfBrgFdbQueueRewindStatusGet(prvTgfDevNum, &rewindAuq);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbQueueRewindStatusGet FAILED, rc = [%d]", rc);
        /* free allocated memory */
        cpssOsFree(auMessagesPtr);

        return;
    }

    if (expectRewind != rewindAuq)
    {
        PRV_UTF_LOG1_MAC("[TGF]: AUQ rewind not expected status, rewindAuq = [%d]", rewindAuq);
        /* free allocated memory */
        cpssOsFree(auMessagesPtr);

        return;
    }


    prvTgfBrgFdbMessagesQueueManagerInfoGet(prvTgfDevNum);

    /* Check MAC addresses range */
    prvTgfBrgFdbAuMacRangeCheck(auMessagesPtr, numOfWaAu);

    cpssOsFree(auMessagesPtr);
}

GT_VOID prvTgfFdbAuStuckWaTest
(
    PRV_TGF_MAC_ENTRY_KEY_STC   * macEntryKeyPtr,
    GT_U32                      * numberOfWaAuMessagesPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      numOfValid = 0; /* Number of valid entries in the FDB */
    GT_U32                      numOfSkip = 0;  /* Number of entries with skip bit set */
    GT_BOOL                     rewindAuq;      /* AUQ rewind indication */

    /* AUTODOC: Send 4 unknown UC traffic */
    prvTgfFdbAuNaMessageSendTrafic();

    cpssOsTimerWkAfter(100);

    /* AUTODOC: get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid, &numOfSkip, NULL, NULL, NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    /* AUTODOC: There are four MAC entries in FDB table  */
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, numOfValid - numOfSkip,
                                 "prvTgfBrgFdbCount number of FDB valid entries %d\n", numOfValid - numOfSkip);

    /* AUTODOC: Send 2K + 64 new MAC addresses to the hardware MAC address */
    prvTgfFdbAuFillQueues(macEntryKeyPtr, numberOfWaAuMessagesPtr);

    /* AUTODOC: Disable sending AA and TA messages to AUQ */
    rc = prvTgfBrgFdbAAandTAToCpuSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_OK, rc, "prvTgfBrgFdbAAandTAToCpuSet");

    /* AUTODOC: Start flush FDB triggred action - action fail */
    prvTgfFdbAuFdbFlushTriggerAction(GT_FALSE);

    /* AUTODOC: Execute WA */
    rc = prvTgfBrgFdbTriggerAuQueueWa(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_OK, rc, "prvTgfBrgFdbTriggerAuQueueWa");

    /* AUTODOC: Get primary AUQ rewind status - expected no rewind */
    rc = prvTgfBrgFdbQueueRewindStatusGet(prvTgfDevNum, &rewindAuq);
    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_OK, rc, "prvTgfBrgFdbQueueRewindStatusGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_FALSE, rewindAuq, "prvTgfBrgFdbQueueRewindStatusGet");

    /* AUTODOC: Start flush FDB triggred action - action finished */
    prvTgfFdbAuFdbFlushTriggerAction(GT_TRUE);

    cpssOsTimerWkAfter(100);

    /* AUTODOC: Get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid, &numOfSkip, NULL, NULL, NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    /* AUTODOC: FDB is empty */
    UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfValid - numOfSkip,
                                 "prvTgfBrgFdbCount number of FDB valid entries %d\n", numOfValid - numOfSkip);
}

GT_VOID prvTgfFdbAuStuckWaBasicAuqTest
(
    GT_VOID
)
{
    PRV_TGF_MAC_ENTRY_KEY_STC   saMacEntryKey;  /* Source MAC address */
    GT_U32                      numberOfWaAuMessages = PRV_TGF_AU_MAX_MESSAGES_CNS; /* Number of QA messages */
    GT_BOOL                     expectRewind = GT_TRUE;
    GT_STATUS                   rc;

    saMacEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    saMacEntryKey.key.macVlan.vlanId = 1;
    cpssOsBzero((GT_CHAR_PTR)&(saMacEntryKey.key.macVlan.macAddr), sizeof(saMacEntryKey.key.macVlan.macAddr));

    saMacEntryKey.key.macVlan.macAddr.arEther[2] = 5;
    saMacEntryKey.key.macVlan.macAddr.arEther[3] = 5;
    saMacEntryKey.key.macVlan.macAddr.arEther[4] = 5;
    saMacEntryKey.key.macVlan.macAddr.arEther[5] = 5;

    /* AUTODOC: Start WA */
    prvTgfFdbAuStuckWaTest(&saMacEntryKey, &numberOfWaAuMessages);

    prvTgfBrgFdbMessagesQueueManagerInfoGet(prvTgfDevNum);

    /* AUTODOC: Read AUQ messages - expect rewind */
    prvTgfFdbAuStuckWaReadAuqMessages(numberOfWaAuMessages, expectRewind);

    /* restore configuration */
    /* AUTODOC: Enable sending AA and TA messages to AUQ */
    rc = prvTgfBrgFdbAAandTAToCpuSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_OK, rc, "prvTgfBrgFdbAAandTAToCpuSet");

    /* AUTODOC: Enable learning */
    rc = appDemoAllowProcessingOfAuqMessages(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");
}

GT_VOID prvTgfFdbAuStuckWaDoubleQueueTest
(
    void
)
{
    PRV_TGF_MAC_ENTRY_KEY_STC   saMacEntryKey;  /* Source MAC address */
    GT_U32                      numberOfWaAuMessages; /* Number of QA messages */
    GT_U32                      numberOfAuToRead;   /* Number of QA messages to read */
    GT_U32                      i;                  /* WA iteration index */
    GT_STATUS                   rc;

    saMacEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    saMacEntryKey.key.macVlan.vlanId = 1;

    for(i = 0; i < 4; i++)
    {
        cpssOsBzero((GT_CHAR_PTR)&(saMacEntryKey.key.macVlan.macAddr), sizeof(saMacEntryKey.key.macVlan.macAddr));

        saMacEntryKey.key.macVlan.macAddr.arEther[2] = 5 + (GT_U8)i;
        saMacEntryKey.key.macVlan.macAddr.arEther[3] = 5 + (GT_U8)i;
        saMacEntryKey.key.macVlan.macAddr.arEther[4] = 5 + (GT_U8)i;
        saMacEntryKey.key.macVlan.macAddr.arEther[5] = 5 + (GT_U8)i;

        numberOfWaAuMessages = PRV_TGF_AU_MAX_MESSAGES_CNS;

        /* AUTODOC: Start WA */
        prvTgfFdbAuStuckWaTest(&saMacEntryKey, &numberOfWaAuMessages);

        /* Read from AUQ */
        numberOfAuToRead = PRV_TGF_AU_MAX_MESSAGES_CNS / 2;

        /* AUTODOC: Read AUQ messages - expect rewind */
        prvTgfFdbAuStuckWaReadAuqMessages(numberOfAuToRead, GT_TRUE);

        /* AUTODOC: Enable learning */
        rc = appDemoAllowProcessingOfAuqMessages(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");

        /* give time to process AUQ messages */
        cpssOsTimerWkAfter(10);
    }

    /* restore configuration */
    /* AUTODOC: Enable sending AA and TA messages to AUQ */
    rc = prvTgfBrgFdbAAandTAToCpuSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
                                GT_OK, rc, "prvTgfBrgFdbAAandTAToCpuSet");
}

static GT_VOID prvTgfBrgFdbMacAddrByteIncrement
(
    IN GT_ETHERADDR * macAddrPtr,
    IN GT_U32 byteOffset
)
{
    if(byteOffset > 5)
    {
        PRV_UTF_LOG1_MAC("prvTgfMacAddrIncrement wrong MAC byte offset [%d]", byteOffset);
        return;
    }

    if (macAddrPtr->arEther[byteOffset] == 0xff)
    {
        /* Reset current byte */
        macAddrPtr->arEther[byteOffset] = 0;
        /* Increment next byte */
        prvTgfBrgFdbMacAddrByteIncrement(macAddrPtr, byteOffset - 1);
    }
    else
    {
        /* Increment current byte */
        macAddrPtr->arEther[byteOffset]++;
    }

    /* Reached MAC MSB */
    if(byteOffset == 0)
    {
        return;
    }
}

static GT_VOID prvTgfBrgFdbAuMacRangeCheck
(
    IN CPSS_MAC_UPDATE_MSG_EXT_STC     *auMessagesPtr,
    IN GT_U32                           numberOfAuMessages
)
{
    GT_U32                  currentMessageIndex;    /* Index of current AU message */
    GT_ETHERADDR            * macEntryPrevPtr;      /* Pointer to previouse MAC entry in AU array */
    GT_ETHERADDR            * macEntryCurrentPtr;   /* Pointer to current MAC entry in AU in array */
    GT_STATUS               rc = GT_OK;             /* Return code */
    CPSS_UPD_MSG_TYPE_ENT   updType;                /* Update message type */

    macEntryPrevPtr = &auMessagesPtr[0].macEntry.key.key.macVlan.macAddr;

    for(currentMessageIndex = 0;
         currentMessageIndex < numberOfAuMessages; currentMessageIndex++)
    {
        macEntryCurrentPtr = &auMessagesPtr[currentMessageIndex].macEntry.key.key.macVlan.macAddr;
        updType = auMessagesPtr[currentMessageIndex].updType;

        /* Skip NA and AA messages */
        if(updType == CPSS_AA_E || updType == CPSS_NA_E)
        {
            continue;
        }

        /* compare MAC address */
        rc = cpssOsMemCmp(macEntryPrevPtr->arEther, macEntryCurrentPtr->arEther,
                          sizeof(GT_ETHERADDR)) > 0 ? GT_FAIL : GT_OK;

        if(rc == GT_FAIL)
        {
            PRV_UTF_LOG1_MAC("prvTgfBrgFdbAuMacRangeCheck FAIL: current MAC index [%d]\n", currentMessageIndex);
            TGF_TRACE_MAC_ADDR_MAC(Current  MAC Adderss:, macEntryCurrentPtr->arEther);
            TGF_TRACE_MAC_ADDR_MAC(Previous MAC Adderss:, macEntryPrevPtr->arEther);
            break;
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbAuMacRangeCheck: MAC Address is out of range\n");

        macEntryPrevPtr = macEntryCurrentPtr;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbAuMacRangeCheck: MAC Address is out of range\n");
}


