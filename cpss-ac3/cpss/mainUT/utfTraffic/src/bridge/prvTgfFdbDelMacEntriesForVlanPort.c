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
* @file prvTgfFdbDelMacEntriesForVlanPort.c
*
* @brief Verify that not static entries are not aged
* although deleting static entries is enabling.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <bridge/prvTgfFdbDelMacEntriesForVlanPort.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* traffic generate stages num */
#define PRV_TGF_TRAFFIC_GEN_NUM_CNS     2

/* default FDB entry num */
#define PRV_TGF_FDB_ENTRY_NUM_CNS       12

/* default number of FDB entries per vlan */
#define PRV_TGF_FDB_VLAN_ENTRY_NUM_CNS  4

/* default packets checknum counter */
#define PRV_TGF_PACKETS_CHECK_NUM_CNS   1

/* default capture port index */
#define PRV_TGF_CPORT_IDX_CNS           3

/* Tx delay */
#define PRV_TGF_TX_DELAY                300

/* default deleted entry vlanId */
#define PRV_TGF_DEL_ENTRY_VLANID_CNS    0

/* default deleted entry portIdx */
#define PRV_TGF_DEL_ENTRY_PORTIDX_CNS   1

/* default portIdx send traffic to array */
static GT_U8  prvTgfPortIdxSendToArr[] =
{
     0, 2, 0
};

/* default VLAN Id array */
static GT_U16  prvTgfVlanIdArr[] =
{
     1, 5, 13
};

/* default active VLAN Id */
static GT_U32  prvTgfActiveVlanId     = 1;

/* default active VLAN Mask */
static GT_U32  prvTgfActiveVlanMask   = 0xFFF;

/* default active interface parameters - actIsTrunk */
static GT_U32  prvTgfActIsTrunk       = 0;/* not trunk --> port type */

/* default active interface parameters - actIsTrunkMask */
static GT_U32  prvTgfActIsTrunkMask   = 1;/*exact match*/

/* default active interface parameters - actTrunkPort */
static GT_U32  prvTgfActTrunkPortIdx  = 1;

#ifdef EXMXPM_FAMILY
/* default active interface parameters - actTrunkPortMask */
static GT_U32  prvTgfActTrunkPortMask = 0x3f;/* cpss supports only 6 bits when 'por' is used */
#else
/* default active interface parameters - actTrunkPortMask */
static GT_U32  prvTgfActTrunkPortMask = 0x7f;/* 7 bits for the field -- 127 */
#endif

/* default number of packets to send */
static GT_U32  prvTgfBurstCount       = 4;

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
    10, 2, 2,       /* 1010b, 10b, 10b */
    14, 3, 3        /* 1110b, 11b, 11b */
};

/* captured packet SA*/
static TGF_MAC_ADDR  prvTgfCapturedSa =
{
    0x00, 0x00, 0x00, 0x00, 0x11, 0x11
};

/* captured packet DA*/
static TGF_MAC_ADDR  prvTgfCapturedDa =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 1                                             /* pri, cfi, VlanId */
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
    sizeof(prvTgfPayloadDataArr),                           /* dataLength */
    prvTgfPayloadDataArr                                    /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},      /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static GT_U8 prvTgfPacketsCountRxTxArr[][2*3*PRV_TGF_TRAFFIC_GEN_NUM_CNS] =
{
    { 4,   0,   0,   0},  /* Rx count for stage #1.1 command trafic */
    { 4,   1,   2,   2},  /* Tx count for stage  #1.1 command trafic */

    { 0,   0,   4,   0},  /* Rx count for stage  #1.2 command trafic */
    { 1,   1,   4,   1},  /* Tx count for stage  #1.2 command trafic */

    { 4,   0,   0,   0},  /* Rx count for stage  #1.3 command trafic */
    { 4,   1,   1,   1},  /* Tx count for stage  #1.3 command trafic */

    { 4,   0,   0,   0},  /* Rx count for stage  #2.1 command trafic */
    { 4,   1,   2,   2},  /* Tx count for stage  #2.1 command trafic */

    { 0,   0,   4,   0},  /* Rx count for stage  #2.2 command trafic */
    { 1,   1,   4,   1},  /* Tx count for stage  #2.2 command trafic */

    { 4,   0,   0,   0},  /* Rx count for stage  #2.3 command trafic */
    { 4,   1,   1,   1},  /* Tx count for stage  #2.3 command trafic */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_MAC_VL_ENT             vlanLookupMode;      /* vlan lookup mode */

    GT_U32                      activeVlanId;        /* active vlan parms */
    GT_U32                      activeVlanMask;

    GT_U32                      actIsTrunk;          /* active interface parms */
    GT_U32                      actIsTrunkMask;
    GT_U32                      actTrunkPort;
    GT_U32                      actTrunkPortMask;

    PRV_TGF_MAC_ACTION_MODE_ENT triggeredActionMode; /* macAddr table action mode*/

    GT_BOOL                     staticDelEnable;     /* static entry del mode*/
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbEntriesVerify function
* @endinternal
*
* @brief   Check FDB entries
*
* @param[in] stage                    - index of FDB entries set: 0, 1 or 2
*                                       None
*/
static GT_VOID prvTgfFdbEntriesVerify
(
    IN GT_U32      stage
)
{
    GT_STATUS       rc                          = GT_OK;
    GT_U32          index                       = 0;
    GT_U32          entryNum                    = 0;
    GT_U32          entryPortIdx                = 0;
    GT_U32          entryVlanIdx                = 0;
    GT_BOOL         valid                       = GT_FALSE;
    GT_BOOL         skip                        = GT_FALSE;
    GT_BOOL         aged                        = GT_FALSE;
    GT_HW_DEV_NUM   hwDev                         = 0;

    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    TGF_MAC_ADDR                fdbEntryMacAddr = {0};

    PRV_UTF_LOG0_MAC("\nVerify static entries\n");

    /* verify static entries */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* get FDB entry port index */
        entryPortIdx = entryNum % prvTgfPortsNum;

        /* get FDB entry vlan index */
        entryVlanIdx = entryNum / prvTgfPortsNum;

        /* clear entry key */
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));

        /* set key parameters */
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = prvTgfVlanIdArr[entryVlanIdx];

        /* clear FDB MAC addr */
        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));

        /* set MAC addr parameters */
        fdbEntryMacAddr[5] = (GT_U8)(entryPortIdx + 1);

        /* store MAC addr in key entry*/
        cpssOsMemCpy(&macEntryKey.key.macVlan.macAddr,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* find index by macEntryKey */
        rc = prvTgfBrgFdbMacEntryIndexFind(&macEntryKey, &index);
        switch (stage)
        {
            case 0:
            case 1:
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                             "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d"\
                                             " with vlan=%02d macAddr=00:%02X portNum %d",
                                             entryNum,
                                             macEntryKey.key.macVlan.vlanId,
                                             fdbEntryMacAddr[5],
                                             prvTgfPortsArray[entryPortIdx]);
                                             break;

            default:
                /* entry with portNum 8 and vlanId 1 deleted */
                UTF_VERIFY_EQUAL4_STRING_MAC(((entryPortIdx == PRV_TGF_DEL_ENTRY_PORTIDX_CNS) &&
                                             (entryVlanIdx == PRV_TGF_DEL_ENTRY_VLANID_CNS)) ?
                                             GT_NOT_FOUND : GT_OK,
                                             rc,
                                             "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d"\
                                             " with vlan=%02d macAddr=00:%02X portNum %d",
                                             entryNum,
                                             macEntryKey.key.macVlan.vlanId,
                                             fdbEntryMacAddr[5],
                                             prvTgfPortsArray[entryPortIdx]);
        }

        /* skip, if FDB entry not exists */
        if (GT_OK != rc)
        {
            continue;
        }

        /* read MAC entry */
        rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip,
                                      &aged, &hwDev, &macEntry);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntryRead #%d"\
                                     " with vlan=%02d macAddr=00:%02X portNum %d",
                                     entryNum,
                                     macEntryKey.key.macVlan.vlanId,
                                     fdbEntryMacAddr[5],
                                     prvTgfPortsArray[entryPortIdx]);

        PRV_UTF_LOG3_MAC("Found MAC entry #%02d valid=%d, skip=%d :  ",
                         entryNum+1, valid, skip);
        PRV_UTF_LOG5_MAC("vlan=%02d port=%02d mac %02X:%02X isStatic=%d\n",
                         macEntryKey.key.macVlan.vlanId,
                         macEntry.dstInterface.devPort.portNum,
                         fdbEntryMacAddr[4], fdbEntryMacAddr[5],
                         macEntry.isStatic);

        /* check port number of found macEntry */
        UTF_VERIFY_EQUAL4_STRING_MAC(macEntry.dstInterface.devPort.portNum,
                                     prvTgfPortsArray[entryPortIdx],
                                     "ERROR of static FdbMacEntry #%d"
                                     " with vlan=%02d macAddr=00:%02X portNum=%d",
                                     entryNum,
                                     macEntryKey.key.macVlan.vlanId,
                                     fdbEntryMacAddr[5],
                                     prvTgfPortsArray[entryPortIdx]);
    }

    PRV_UTF_LOG0_MAC("\nVerify dynamic entries\n");

    /* verify dynamic entries */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* get FDB entry port index */
        entryPortIdx = entryNum % prvTgfPortsNum;

        /* get FDB entry vlan index */
        entryVlanIdx = entryNum / prvTgfPortsNum;

        /* clear entry key */
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));

        /* set key parameters */
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = prvTgfVlanIdArr[entryVlanIdx];

        /* clear FDB MAC addr */
        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));

        /* set MAC addr parameters */
        fdbEntryMacAddr[5] = (GT_U8)(entryPortIdx + 0x11);

        /* store MAC addr in key entry*/
        cpssOsMemCpy(&macEntryKey.key.macVlan.macAddr,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* find index by macEntryKey */
        rc = prvTgfBrgFdbMacEntryIndexFind(&macEntryKey, &index);
        switch (stage)
        {
            case 0:
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                             "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d"\
                                             " with vlan=%02d macAddr=00:%02X portNum %d",
                                             entryNum,
                                             macEntryKey.key.macVlan.vlanId,
                                             fdbEntryMacAddr[5],
                                             prvTgfPortsArray[entryPortIdx]);
                break;

            default:
                /* entry with portNum 8 and vlanId 1 deleted */
                UTF_VERIFY_EQUAL4_STRING_MAC(((entryPortIdx == PRV_TGF_DEL_ENTRY_PORTIDX_CNS) &&
                                             (entryVlanIdx == PRV_TGF_DEL_ENTRY_VLANID_CNS)) ?
                                             GT_NOT_FOUND : GT_OK,
                                             rc,
                                             "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d"\
                                             " with vlan=%02d macAddr=00:%02X portNum %d",
                                             entryNum,
                                             macEntryKey.key.macVlan.vlanId,
                                             fdbEntryMacAddr[5],
                                             prvTgfPortsArray[entryPortIdx]);
        }

        /* skip, if FDB entry not exists */
        if (GT_OK!=rc)
        {
            continue;
        }

        /* read MAC entry */
        rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip,
                                      &aged, &hwDev, &macEntry);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntryRead #%d"\
                                     " with vlan=%02d macAddr=00:%02X portNum %d",
                                     entryNum,
                                     macEntryKey.key.macVlan.vlanId,
                                     fdbEntryMacAddr[5],
                                     prvTgfPortsArray[entryPortIdx]);

        PRV_UTF_LOG3_MAC("Found MAC entry #%02d valid=%d, skip=%d :  ",
                         entryNum, valid, skip);

        PRV_UTF_LOG5_MAC("vlan=%02d port=%02d mac %02X:%02X isStatic=%d\n",
                         macEntryKey.key.macVlan.vlanId,
                         macEntry.dstInterface.devPort.portNum,
                         fdbEntryMacAddr[4], fdbEntryMacAddr[5],
                         macEntry.isStatic);

        /* check port number of found macEntry */
        UTF_VERIFY_EQUAL4_STRING_MAC(macEntry.dstInterface.devPort.portNum,
                                     prvTgfPortsArray[entryPortIdx],
                                     "ERROR of dynamic FdbMacEntry #%d"
                                     " with vlan=%02d macAddr=00:%02X portNum=%d",
                                     entryNum,
                                     macEntryKey.key.macVlan.vlanId,
                                     fdbEntryMacAddr[5],
                                     prvTgfPortsArray[entryPortIdx]);
    }
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfFdbDelMacEntriesForVlanPortConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc                  = GT_OK;
    GT_U8                       tagArray[]          = {1, 1, 1, 1};

    GT_U32                      entryNum            = 0;
    GT_U32                      entryPortIdx        = 0;
    GT_U32                      entryVlanIdx        = 0;


    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry            = {{0}};
    TGF_MAC_ADDR                fdbEntryMacAddr     = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    PRV_UTF_LOG0_MAC("\nSet Vlan configuration\n");

    /* AUTODOC: create VLAN 5 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanIdArr[1],
                                           prvTgfPortsArray, NULL,
                                           tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryWithPortsSet, vlan = %d ",
                                 prvTgfVlanIdArr[1]);

    /* AUTODOC: create VLAN 13 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanIdArr[2],
                                           prvTgfPortsArray, NULL,
                                           tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryWithPortsSet, vlan = %d",
                                 prvTgfVlanIdArr[2]);

    /* set the Vlan lookup mode - both MAC and Vlan are used for MAC table lookup and learning */
    rc = prvTgfBrgFdbMacVlanLookupModeGet(prvTgfDevNum, &prvTgfRestoreCfg.vlanLookupMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbMacVlanLookupModeGet");

    /* AUTODOC: set the Vlan lookup mode to Independent VLAN Learning */
    rc = prvTgfBrgFdbMacVlanLookupModeSet(CPSS_IVL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbMacVlanLookupModeSet");

    PRV_UTF_LOG0_MAC("\nAdd MAC entries\n");

    /* AUTODOC: add 12 static FDB entries with: */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[01..04], VLAN 1, port 0 */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[01..04], VLAN 5, port 1 */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[01..04], VLAN 13, port 2 */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* get FDB entry port index */
        entryPortIdx = entryNum % prvTgfPortsNum;

        /* get FDB entry vlan index */
        entryVlanIdx = entryNum / prvTgfPortsNum;

        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults */
        macEntry.key.entryType               = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.dstInterface.type           = CPSS_INTERFACE_PORT_E;
        macEntry.daCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.isStatic                    = GT_TRUE;
        macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;

        /* iterate thru vlanId */
        macEntry.key.key.macVlan.vlanId      = prvTgfVlanIdArr[entryVlanIdx];

        /* clear FDB MAC addr */
        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));

        /* iterate thru MAC addresses [01 .. 04] */
        fdbEntryMacAddr[5] = (GT_U8)(entryPortIdx + 1);
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* iterate thru ports */
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[entryPortIdx];

        /* write MAC entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntrySet at #%d",
                                     entryNum);
    }

    /* AUTODOC: add 12 dynamic FDB entries with: */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[11..14], VLAN 1, port 0 */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[11..14], VLAN 5, port 1 */
    /* AUTODOC:   4 entries MAC 00:00:00:00:00:[11..14], VLAN 13, port 2 */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* get FDB entry port index */
        entryPortIdx = entryNum % prvTgfPortsNum;

        /* get FDB entry vlan index */
        entryVlanIdx = entryNum / prvTgfPortsNum;

        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults */
        macEntry.key.entryType               = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.dstInterface.type           = CPSS_INTERFACE_PORT_E;
        macEntry.daCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.isStatic                    = GT_FALSE;
        macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;

        /* iterate thru vlanId */
        macEntry.key.key.macVlan.vlanId      = prvTgfVlanIdArr[entryVlanIdx];

        /* clear FDB MAC addr */
        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));

        /* iterate thru MAC addresses [0x11 .. 0x14] */
        fdbEntryMacAddr[5] = (GT_U8)(entryPortIdx + 0x11);
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* iterate thru ports */
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[entryPortIdx];

        /* write MAC entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntrySet at #%d",
                                     entryNum);
    }

    /* AUTODOC: check added MAC entries */
    prvTgfFdbEntriesVerify(0);

    /* store active vlanId and vlanMask */
    rc = prvTgfBrgFdbActionActiveVlanGet(prvTgfDevNum,
                                         &prvTgfRestoreCfg.activeVlanId,
                                         &prvTgfRestoreCfg.activeVlanMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveVlanGet");

    /* AUTODOC: set action active vlan and vlan mask to 1 */
    rc = prvTgfBrgFdbActionActiveVlanSet(prvTgfActiveVlanId, prvTgfActiveVlanMask);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveVlanSet:"\
                                 " vlanId 0x0%X   vlanMask 0x0%X",
                                 prvTgfActiveVlanId, prvTgfActiveVlanMask);
    /* store active interface */
    rc = prvTgfBrgFdbActionActiveInterfaceGet(prvTgfDevNum,
                                              &prvTgfRestoreCfg.actIsTrunk,
                                              &prvTgfRestoreCfg.actIsTrunkMask,
                                              &prvTgfRestoreCfg.actTrunkPort,
                                              &prvTgfRestoreCfg.actTrunkPortMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveInterfaceGet");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfActTrunkPortMask = 0xFFF;/* support
                                        trunkId        0..4095 and
                                        physical ports 0..511 */
    }

    /* AUTODOC: set active port 1 and mask 0x7F that control FDB actions */
    rc = prvTgfBrgFdbActionActiveInterfaceSet(prvTgfDevNum,
                                              prvTgfActIsTrunk, prvTgfActIsTrunkMask,
                                              prvTgfPortsArray[prvTgfActTrunkPortIdx],
                                              prvTgfActTrunkPortMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveInterfaceSet");

    PRV_UTF_LOG2_MAC("\nDelete dynamic mac entry with vlanId %d portNum %d \n",
                     prvTgfVlanIdArr[PRV_TGF_DEL_ENTRY_VLANID_CNS],
                     prvTgfPortsArray[PRV_TGF_DEL_ENTRY_PORTIDX_CNS] );

    /* save triggered action mode */
    rc = prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum, &prvTgfRestoreCfg.triggeredActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeGet");

    /* AUTODOC: starts triggered action by Triggered Address deleting */
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_DELETING_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionStart");

    /* AUTODOC: wait for the FDB triggered action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionDoneWait");

    /* AUTODOC: verify only one dynamic entry with VLAN 1 and port 1 was deleted */
    prvTgfFdbEntriesVerify(1);

    PRV_UTF_LOG2_MAC("\nDelete static mac entry with vlanId %d portNum %d \n",
                     prvTgfVlanIdArr[PRV_TGF_DEL_ENTRY_VLANID_CNS],
                     prvTgfPortsArray[PRV_TGF_DEL_ENTRY_PORTIDX_CNS] );

    /* save whether flush delete operates on static entries. */
    rc = prvTgfBrgFdbStaticDelEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.staticDelEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableGet");

    /* AUTODOC: enable delete on static entries */
    rc = prvTgfBrgFdbStaticDelEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableSet");

    /* AUTODOC: starts triggered action by Triggered Address deleting */
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_DELETING_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionStart");

    /* AUTODOC: wait for the FDB triggered action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionDoneWait");

    /* AUTODOC: verify only one static entry with VLAN 1 and port 1 was deleted */
    prvTgfFdbEntriesVerify(2);
}

/**
* @internal prvTgfFdbDelMacEntriesForVlanPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      stageNum        = 0;
    GT_U32                      portIdx         = 0;
    GT_U32                      expPacketLen    = 0;
    GT_U32                      rxPacketsCount  = 0;
    GT_U32                      txPacketsCount  = 0;
    GT_U32                      numTriggers     = 0;
    GT_U32                      expTriggers     = 0;
    GT_U32                      pIdx            = 0;
    GT_U32                      trafficPortNum  = 0;

    CPSS_INTERFACE_INFO_STC     portInterface   = {0};
    TGF_VFD_INFO_STC            vfdArray[2]     = {{0}};
    TGF_VFD_INFO_STC            eVfdArray[1]    = {{0}};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < PRV_TGF_TRAFFIC_GEN_NUM_CNS; stageNum++ )
    {

        PRV_UTF_LOG0_MAC("\n========== Traffic generate ============\n\n");

        /* send packet */
        for ( portIdx = 0; portIdx < (sizeof(prvTgfPortIdxSendToArr)/sizeof(prvTgfPortIdxSendToArr[0])); portIdx++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficTableRxPcktTblClear");

            /* reset all counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfEthCountersReset: dev=%d",
                                         prvTgfDevNum);

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");


            /* set packets DA */
            prvTgfPacketL2Part.daMac[5]=(GT_U8) ((stageNum) ? 0x01 : 0x11);

            /* set packets VlanId */
            prvTgfPacketVlanTagPart.vid = prvTgfVlanIdArr[portIdx];

            trafficPortNum = prvTgfPortsArray[prvTgfPortIdxSendToArr[portIdx]];

            PRV_UTF_LOG3_MAC("StageNum %02d,   traffic for vlanId %02d  on port = %02d\n\n",
                              stageNum+1, prvTgfVlanIdArr[portIdx], trafficPortNum);

            /* prepare ETH VFD array entry for DaMac */
            eVfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
            eVfdArray[0].modeExtraInfo  = 0;
            eVfdArray[0].incValue       = 1;
            eVfdArray[0].offset         = 0;                /* DaMac offset */
            eVfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(eVfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount,
                                     sizeof(eVfdArray)/sizeof(eVfdArray[0]), eVfdArray);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d",
                                         prvTgfDevNum);

            /* set Tx delay */
            prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY);

            /* AUTODOC: send 12 packets for each stage with SA=00:00:00:00:11:11 and: */
            /* AUTODOC:   p1. 4 packets - DAs=00:00:00:00:00:XX, VID=1 on port 0 */
            /* AUTODOC:   p2. 4 packets - DAs=00:00:00:00:00:XX, VID=5 on port 2 */
            /* AUTODOC:   p3. 4 packets - DAs=00:00:00:00:00:XX, VID=13 on port 0 */
            /* AUTODOC:   where XX = [11..14] for stage#1 and [01..04] for stage#2 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, trafficPortNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                         prvTgfDevNum, trafficPortNum);

            /* remove Tx delay */
            prvTgfSetTxSetup2Eth(0, 0);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic: */
            /* AUTODOC:   stage#1: */
            /* AUTODOC:     p1: 1 packet on port 1; 2 packets on each ports 2,3 */
            /* AUTODOC:     p2: 1 packet on each ports 0,1,3 */
            /* AUTODOC:     p3: 1 packet on each ports 1,2,3 */
            /* AUTODOC:   stage#2: */
            /* AUTODOC:     p1: 1 packet on port 1; 2 packets on each ports 2,3 */
            /* AUTODOC:     p2: 1 packet on each ports 0,1,3 */
            /* AUTODOC:     p3: 1 packet on each ports 1,2,3 */
            for (pIdx = 0; pIdx < prvTgfPortsNum; pIdx++)
            {
                /* expected number of Tx packets */
                txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (portIdx*2) + (stageNum*6)][pIdx];

                /* expected number of Rx packets */
                rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (portIdx*2) + (stageNum*6)][pIdx]+
                                 ((PRV_TGF_CPORT_IDX_CNS == pIdx) * txPacketsCount);

                expPacketLen = ((portIdx == 0) && (pIdx != 0))?
                    PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS :
                    PRV_TGF_PACKET_LEN_CNS;

                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[pIdx],
                                            rxPacketsCount, txPacketsCount, expPacketLen,
                                            PRV_TGF_PACKETS_CHECK_NUM_CNS);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                             "ERROR of prvTgfEthCountersCheck"\
                                             "  Port=%02d  expectedRx=%02d"\
                                             "  expectedTx=%02d"\
                                             "  expectedLen=%02d\n",
                                             prvTgfPortsArray[pIdx],
                                             rxPacketsCount, txPacketsCount,
                                             expPacketLen);
            }

            /* get trigger counters */
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

            /* get trigger counters where packet has MAC DA as macAddr */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfCapturedDa, sizeof(TGF_MAC_ADDR));

            /* get trigger counters where packet has MAC SA as macAddr */
            vfdArray[1].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
            vfdArray[1].modeExtraInfo = 0;
            vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
            vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfCapturedSa, sizeof(TGF_MAC_ADDR));

            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                                sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                                vfdArray, &numTriggers);

            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check trigger counters */
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum,
                                         portInterface.devPort.portNum);

            /* expected number of Rx packets */
            expTriggers = prvTgfExpTriggersArr[portIdx+ (stageNum*3)] *
                        (prvTgfPacketsCountRxTxArr[1 + (portIdx*2) + (stageNum*6)][PRV_TGF_TRAFFIC_GEN_NUM_CNS] != 0);
            PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", expTriggers);
            UTF_VERIFY_EQUAL0_STRING_MAC(expTriggers, numTriggers,
                                         "\n ERROR: Got another MAC DA/SA\n");
        }
    }
}

/**
* @internal prvTgfFdbDelMacEntriesForVlanPortConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      iterNum = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore delete mode on static entries */
    rc = prvTgfBrgFdbStaticDelEnableSet(prvTgfRestoreCfg.staticDelEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableSet");

    /* AUTODOC: restore trigger action mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(prvTgfRestoreCfg.triggeredActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* AUTODOC: restore action active interface */
    rc = prvTgfBrgFdbActionActiveInterfaceSet(prvTgfDevNum,
                                              prvTgfRestoreCfg.actIsTrunk,
                                              prvTgfRestoreCfg.actIsTrunkMask,
                                              prvTgfRestoreCfg.actTrunkPort,
                                              prvTgfRestoreCfg.actTrunkPortMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveInterfaceSet");

    /* AUTODOC: restore active vlanId and vlanMask */
    rc = prvTgfBrgFdbActionActiveVlanSet(prvTgfRestoreCfg.activeVlanId,
                                         prvTgfRestoreCfg.activeVlanMask);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbActionActiveVlanSet:"\
                                 " vlanId %d   vlanMask %d",
                                 prvTgfRestoreCfg.activeVlanId,
                                 prvTgfRestoreCfg.activeVlanMask);

    /* AUTODOC: restore vlan lookup mode */
    rc = prvTgfBrgFdbMacVlanLookupModeSet( prvTgfRestoreCfg.vlanLookupMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgFdbMacVlanLookupModeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry 5 and 13 (and reset vlan entry) */
    for (iterNum = 1; iterNum < (sizeof(prvTgfVlanIdArr) / sizeof(prvTgfVlanIdArr[0])); iterNum++)
    {
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanIdArr[iterNum]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgDefVlanEntryInvalidate #%d",
                                     prvTgfVlanIdArr[iterNum]);
    }
}


