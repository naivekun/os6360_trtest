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
* @file prvTgfFdbTriggeredAgingEntries.c
*
* @brief Verify that not static entries are not aged
* although deleting static entries is enabling.
*
* @version   4
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
#include <bridge/prvTgfFdbTriggeredAgingEntries.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS              1

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* FDB entry num */
#define PRV_TGF_FDB_ENTRY_NUM_CNS       8

/* trigger aging tests num */
#define PRV_TGF_T_A_TESTS_NUM_CNS       2

/* default packets checknum counter */
#define PRV_TGF_PACKETS_CHECK_NUM_CNS   1

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS           3

/* Tx delay */
#define PRV_TGF_TX_DELAY                300

/* default number of packets to send */
static GT_U32  prvTgfBurstCount       = 8;

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
     0x0E, 0x3AA /*  1110b , 1110101010b */
};

/* default number of FDB static entry */
static GT_U32  prvTgfFdbStaticEntryNum  = 4;

/* captured packet SA*/
static TGF_MAC_ADDR  prvTgfCapturedSa =
{
    0x00, 0x00, 0x00, 0x00, 0x11, 0x11
};

/* captured packet DA*/
static TGF_MAC_ADDR  prvTgfCapturedDa =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08
};

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][2*PRV_TGF_T_A_TESTS_NUM_CNS] =
{
    {8, 0, 0, 0},  /* Rx count for phase #1 command trafic */
    {8, 2, 2, 2},  /* Tx count for phase #1 command trafic */
    {8, 0, 0, 0},  /* Rx count for phase #2 command trafic */
    {8, 5, 5, 5},  /* Tx count for phase #2 command trafic */
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},       /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}        /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,            /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                    /* pri, cfi, VlanId */
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

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MAC_ACTION_MODE_ENT agingMode;
    GT_BOOL                     staticDelEnable;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbAgingVerify function
* @endinternal
*
* @brief   Check and display aging state of FDB entries
*
* @param[in] stage                    - index of FDB entries set, 0 or 1
*                                       None
*/
static GT_VOID prvTgfFdbAgingVerify
(
    IN GT_U32      stage
)
{
    GT_STATUS       rc              = GT_OK;
    GT_U32          index           = 0;
    GT_U32          entryNum        = 0;
    GT_BOOL         valid           = GT_FALSE;
    GT_BOOL         skip            = GT_FALSE;
    GT_BOOL         aged            = GT_FALSE;
    GT_HW_DEV_NUM   hwDev             = 0;

    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    TGF_MAC_ADDR                fdbEntryMacAddr = {0};

    PRV_UTF_LOG0_MAC("\nVerify aging \n");

    /* verify FDB entries aging */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* clear entry key */
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));

        /* set key parameters */
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

        /* clear FDB MAC addr */
        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));

        /* set MAC addr parameters */
        if (0 == stage)
        {
            fdbEntryMacAddr[5] = (GT_U8)(entryNum + 1);
        }
        else
        {
            fdbEntryMacAddr[4] = (GT_U8) ((entryNum<prvTgfFdbStaticEntryNum) ? 0x00 : 0x11);
            fdbEntryMacAddr[5] = (GT_U8) ((entryNum<prvTgfFdbStaticEntryNum) ?
                                 (GT_U8)(entryNum + 1) :
                                 (GT_U8)(entryNum - prvTgfFdbStaticEntryNum + 0x11));
        }
        /* store MAC addr in key entry*/
        cpssOsMemCpy(&macEntryKey.key.macVlan.macAddr,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* find index by macEntryKey */
        rc = prvTgfBrgFdbMacEntryIndexFind(&macEntryKey, &index);
        if (0 == stage)
        {
            /* check find result */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d",
                                         entryNum);
            if (GT_OK != rc)
            {
                continue;
            }
            /* AUTODOC:   read MAC entry */
            rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip,
                                          &aged, &hwDev, &macEntry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfBrgFdbMacEntryRead #%d",
                                         entryNum);
            if (GT_OK != rc)
            {
                continue;
            }
            /* AUTODOC:   check for dynamic entries age bit FALSE, for static entries TRUE */
            UTF_VERIFY_EQUAL3_STRING_MAC((entryNum < prvTgfFdbStaticEntryNum) ?
                                             GT_TRUE : GT_FALSE, aged,
                                             "ERROR of FdbMacEntry entry #%d: "\
                                             "isAged=%d, isStatic=%d",
                                             entryNum, aged,
                                             entryNum < prvTgfFdbStaticEntryNum);
        }
        else
        {
            /* check only 5 entries exists */
            UTF_VERIFY_EQUAL1_STRING_MAC((entryNum < prvTgfFdbStaticEntryNum+1) ?
                                         GT_OK : GT_NOT_FOUND, rc,
                                         "ERROR of prvTgfBrgFdbMacEntryIndexFind #%d",
                                         entryNum);
            if (GT_OK != rc)
            {
                continue;
            }
            /* read MAC entry */
            rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip,
                                      &aged, &hwDev, &macEntry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntryRead #%d",
                                     entryNum);
            if (GT_OK != rc)
            {
                continue;
            }
            PRV_UTF_LOG5_MAC("Found MAC entry #%d"\
                             "valid=%02d, skip=%d, aged=%d, port=%02d ",
                             entryNum, valid, skip,
                             aged, macEntry.dstInterface.devPort.portNum );
            PRV_UTF_LOG3_MAC("MAC = %02X:%02X   isStatic = %d\n",
                             fdbEntryMacAddr[4], fdbEntryMacAddr[5],
                             macEntry.isStatic);
        }
    }
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfFdbTriggeredAgingEntriesConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbTriggeredAgingEntriesConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc                  = GT_OK;
    GT_U32      entryNum            = 0;
    GT_U32      portIdx             = 0;

    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    TGF_MAC_ADDR                fdbEntryMacAddr = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* save FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum, &prvTgfRestoreCfg.agingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeGet");

    /* AUTODOC: set trigger mode: action is done via trigger from CPU */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* save whether flush delete operates on static entries. */
    rc = prvTgfBrgFdbStaticDelEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.staticDelEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableGet");

    /* AUTODOC: enable delete operation on static entries */
    rc = prvTgfBrgFdbStaticDelEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableSet");

    PRV_UTF_LOG0_MAC("\nAdd MAC entries\n");

    /* AUTODOC: add 4 static FDB entries with: */
    /* AUTODOC:   1. MAC 00:00:00:00:00:01, VLAN 1, port 0 */
    /* AUTODOC:   2. MAC 00:00:00:00:00:02, VLAN 1, port 1 */
    /* AUTODOC:   3. MAC 00:00:00:00:00:03, VLAN 1, port 2 */
    /* AUTODOC:   4. MAC 00:00:00:00:00:04, VLAN 1, port 3 */
    /* AUTODOC: add 4 dynamic FDB entries with: */
    /* AUTODOC:   1. MAC 00:00:00:00:00:05, VLAN 1, port 0 */
    /* AUTODOC:   2. MAC 00:00:00:00:00:06, VLAN 1, port 1 */
    /* AUTODOC:   3. MAC 00:00:00:00:00:07, VLAN 1, port 2 */
    /* AUTODOC:   4. MAC 00:00:00:00:00:08, VLAN 1, port 3 */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults */
        macEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId   = PRV_TGF_VLANID_CNS;
        macEntry.dstInterface.type        = CPSS_INTERFACE_PORT_E;
        macEntry.daCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
        /* half of FDB entries is static */
        macEntry.isStatic = (entryNum < prvTgfFdbStaticEntryNum) ? GT_TRUE : GT_FALSE;
        /* iterate thru MAC addresses */
        fdbEntryMacAddr[5] = (GT_U8)(entryNum + 1);
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
        portIdx = entryNum - ((entryNum >= prvTgfFdbStaticEntryNum) * prvTgfFdbStaticEntryNum);
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[portIdx];

        /* write MAC entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntrySet at #%d",
                                     entryNum);
    }

    PRV_UTF_LOG0_MAC("\n--------------------------------------------------\n");

    PRV_UTF_LOG0_MAC("\nCheck added MAC entries\n");

    /* AUTODOC: check added MAC entries */
    for (entryNum = 0; entryNum < PRV_TGF_FDB_ENTRY_NUM_CNS; entryNum++)
    {
        PRV_UTF_LOG1_MAC("Check MAC entry #%d\n", entryNum);

        /* clear entry key*/
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));

        /* set key parameters */
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
        fdbEntryMacAddr[5] = (GT_U8)(entryNum + 1);
        cpssOsMemCpy(&macEntryKey.key.macVlan.macAddr,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        PRV_UTF_LOG2_MAC("Scan for: VLAN ID = %d  EntryType = %d\n",
                         macEntryKey.key.macVlan.vlanId, macEntryKey.entryType);

        PRV_UTF_LOG6_MAC("MAC addr:  %02d:%02d:%02d:%02d:%02d:%02d\n",
                         fdbEntryMacAddr[0], fdbEntryMacAddr[1], fdbEntryMacAddr[2],
                         fdbEntryMacAddr[3], fdbEntryMacAddr[4], fdbEntryMacAddr[5]);

        /* scan FDB to MAC entry by key */
        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of FdbMacEntry #%d", entryNum);

        portIdx = entryNum - ((entryNum >= prvTgfFdbStaticEntryNum) * prvTgfFdbStaticEntryNum);

        /* check entry parameters */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfDevNum,
                                     macEntry.dstInterface.devPort.hwDevNum,
                                     "ERROR of FdbMacEntry #%d devnum=%d",
                                     entryNum,
                                     macEntry.dstInterface.devPort.hwDevNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPortsArray[portIdx],
                                     macEntry.dstInterface.devPort.portNum,
                                     "ERROR of FdbMacEntry #%d portnum=%d",
                                     entryNum,
                                     macEntry.dstInterface.devPort.portNum);

        UTF_VERIFY_EQUAL2_STRING_MAC((entryNum < prvTgfFdbStaticEntryNum) ? GT_TRUE : GT_FALSE,
                                     macEntry.isStatic,
                                     "ERROR of FdbMacEntry #%d isStatic=%d",
                                     entryNum, macEntry.isStatic);
    }
}

/**
* @internal prvTgfFdbTriggeredAgingEntriesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbTriggeredAgingEntriesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc              = GT_OK;
    GT_U32          portIdx         = 0;

    GT_U32          stageNum        = 0;
    GT_U32          expPacketLen    = 0;
    GT_U32          rxPacketsCount  = 0;
    GT_U32          txPacketsCount  = 0;
    GT_U32          numTriggers     = 0;
    GT_U32          expTriggers     = 0;
    PRV_TGF_FDB_ACTION_MODE_ENT actionMode =
                PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum) ?
                PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E :/*the multi-port group devices not support age with removal */
                PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E;

    CPSS_INTERFACE_INFO_STC     portInterface = {0};
    TGF_VFD_INFO_STC            vfdArray[2]   = {{0}};
    TGF_VFD_INFO_STC            eVfdArray[1]  = {{0}};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < PRV_TGF_T_A_TESTS_NUM_CNS; stageNum++ )
    {
        PRV_UTF_LOG1_MAC("\n========== Trigger aging #%d ===========\n\n", stageNum+1);

        PRV_UTF_LOG0_MAC("\nAction start \n");

        /* AUTODOC: starts triggered action by Automatic aging with removal */
        rc = prvTgfBrgFdbActionStart(actionMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionStart");

        /* AUTODOC: wait for the FDB triggered action to end */
        rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbActionDoneWait");

        /* AUTODOC: verify aging with set of FDB entries: */
        prvTgfFdbAgingVerify(stageNum);

        PRV_UTF_LOG0_MAC("\n Send \n");

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
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY);

        /* AUTODOC: send 8 packets on port 0 with: */
        /* AUTODOC:   DAs=00:00:00:00:00:[01..08], SA=00:00:00:00:11:11 */
        /* AUTODOC:   VID=1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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
        /* AUTODOC:   1 iter - get 2 packets on each ports 1,2,3 */
        /* AUTODOC:   2 iter - get 5 packets on each ports 1,2,3 */
        for (portIdx = 0; portIdx < prvTgfPortsNum; portIdx++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1+(stageNum*2)][portIdx];

            /* expected number of Rx packets */
            rxPacketsCount =  prvTgfPacketsCountRxTxArr[stageNum*2][portIdx] +
                              ((PRV_TGF_CPORT_IDX_CNS == portIdx) * txPacketsCount);

            /* expected packet size */
            expPacketLen = (PRV_TGF_SEND_PORT_IDX_CNS == portIdx) ?
                            PRV_TGF_PACKET_LEN_CNS :
                            PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIdx],
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        PRV_TGF_PACKETS_CHECK_NUM_CNS);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfEthCountersCheck"\
                                         "  Port=%02d  ExpectedRx=%02d"\
                                         "  ExpectedTx=%02d BurstCnt=%d\n",
                                         prvTgfPortsArray[portIdx],
                                         rxPacketsCount, txPacketsCount,
                                         PRV_TGF_PACKETS_CHECK_NUM_CNS);
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

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum,
                                     portInterface.devPort.portNum);

        /* expected number of Rx packets */
        expTriggers = prvTgfExpTriggersArr[stageNum] *
                    (prvTgfPacketsCountRxTxArr[1+(stageNum*2)][PRV_TGF_CPORT_IDX_CNS] != 0);

        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", expTriggers);
        UTF_VERIFY_EQUAL0_STRING_MAC(expTriggers, numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* @internal prvTgfFdbTriggeredAgingEntriesConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbTriggeredAgingEntriesConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore mode whether flush operates on static entries */
    rc = prvTgfBrgFdbStaticDelEnableSet(prvTgfRestoreCfg.staticDelEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbStaticDelEnableSet");

    /* AUTODOC: restore FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(prvTgfRestoreCfg.agingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");
}


