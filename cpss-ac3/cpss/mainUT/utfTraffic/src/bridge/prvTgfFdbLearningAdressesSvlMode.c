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
* @file prvTgfFdbLearningAdressesSvlMode.c
*
* @brief Check SVL vlan lookup mode
*
* @version   3
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
#include <bridge/prvTgfFdbLearningAdressesSvlMode.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  default VLAN Id */
#define PRV_TGF_DEF_VLANID_CNS                  1

/*  VLAN Id 5 */
#define PRV_TGF_VLANID_5_CNS                    5

/*  VLAN Id 13 */
#define PRV_TGF_VLANID_13_CNS                   13

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            3

/* Tx delay */
#define PRV_TGF_TX_DELAY                        100

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 4;

/* vlan array to send */
static GT_U16  prvTgfSendVlanArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_DEF_VLANID_CNS,
    PRV_TGF_VLANID_5_CNS,
    PRV_TGF_VLANID_13_CNS
};

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = 
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1}
};

/* expected number of capturing packets */
static GT_U8 prvTgfTriggersArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    15, 15, 15
};

/* expected number of FDB entries */
static GT_BOOL prvTgfFdbEntryArr[PRV_TGF_SEND_PACKETS_NUM_CNS][4] =
{
    {GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE},
    {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE},
    {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE}
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x78, 0x45, 0x34},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
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

/* parameters that is needed to be restored */
static CPSS_MAC_VL_ENT prvTgfDefVlanLookupMode;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbEntryCheckByVlanAndMac function
* @endinternal
*
* @brief   Check that fdb entry with such VlanId and MacAdrr is created
*
* @param[in] fdbVlanId                -         vlan id in FDB table
* @param[in] fdbEntryMacAddr          -   mac address in FDB table
*
* @param[out] validPtr                 -         if entry with inputs parameters exists
*                                      return GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvTgfFdbEntryCheckByVlanAndMac
(
    IN  GT_U16                       fdbVlanId,
    IN  TGF_MAC_ADDR                 fdbEntryMacAddr,
    OUT GT_BOOL                      *validPtr
)
{
    GT_STATUS       rc    = GT_OK;
    GT_U32          index = 0;

    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;


    /* clear entry key */
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));

    /* set key parameters */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = fdbVlanId;

    /* store MAC addr in key entry*/
    cpssOsMemCpy(&macEntryKey.key.macVlan.macAddr, 
                 fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

    /* find index by macEntryKey */
    rc = prvTgfBrgFdbMacEntryIndexFind(&macEntryKey, &index);

    *validPtr = (GT_OK != rc) ? GT_FALSE : GT_TRUE;

    return ((GT_OK != rc) && (GT_NOT_FOUND != rc)) ? rc : GT_OK;
}

/**
* @internal prvTgfFdbLearningAdressesSvlModeConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbLearningAdressesSvlModeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 1, 1, 1};


    /* AUTODOC: SETUP CONFIGURATION: */

    /* sets the VLAN Lookup mode  */
    rc = prvTgfBrgFdbMacVlanLookupModeGet(prvTgfDevNum, &prvTgfDefVlanLookupMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacVlanLookupModeGet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 13 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_13_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: set VLAN Lookup mode to shared VLAN learning */
    rc = prvTgfBrgFdbMacVlanLookupModeSet(CPSS_SVL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacVlanLookupModeSet: %d", CPSS_SVL_E);
}

/**
* @internal prvTgfFdbLearningAdressesSvlModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbLearningAdressesSvlModeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       portIter = 0;
    GT_U8                       sendIter = 0;
    GT_U8                       burstCountIter = 0;
    GT_U32                      packetLen      = 0;
    GT_U32                      numTriggers    = 0;

    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface = {0};
    TGF_VFD_INFO_STC            eVfdArray[1]  = {{0}};

    GT_BOOL                     valid = GT_FALSE;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* AUTODOC: Iterate thru 3 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* set VLAN Id in Packet */
        prvTgfPacketVlanTagPart.vid = prvTgfSendVlanArr[sendIter];

        /* prepare ETH VFD array entry for SaMac */
        eVfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
        eVfdArray[0].modeExtraInfo  = 0;
        eVfdArray[0].incValue       = 1;
        eVfdArray[0].offset         = 6;                /* SaMac offset */
        eVfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(eVfdArray[0].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 
                                 sizeof(eVfdArray)/sizeof(eVfdArray[0]), eVfdArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY);

        /* AUTODOC: for each stage send 4 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:78:45:34, SA=00:00:00:00:00:01, VID=XX */
        /* AUTODOC:   DA=00:00:00:78:45:34, SA=00:00:00:00:00:02, VID=XX */
        /* AUTODOC:   DA=00:00:00:78:45:34, SA=00:00:00:00:00:03, VID=XX */
        /* AUTODOC:   DA=00:00:00:78:45:34, SA=00:00:00:00:00:04, VID=XX */
        /* AUTODOC:   where XX = [1,5,13] per stage */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]); 
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* remove Tx delay */
        prvTgfSetTxSetup2Eth(0, 0);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 stage - 4 untagged packets received on ports 1,2,3 */
        /* AUTODOC:   2 stage - 4 tagged packets VID=5 received on each ports 1,2,3 */
        /* AUTODOC:   3 stage - 4 tagged packets VID=13 received on each ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = prvTgfTaggedPacketInfo.totalLen - (TGF_VLAN_TAG_SIZE_CNS *
                        (((portIter != PRV_TGF_SEND_PORT_IDX_CNS) && 
                          (prvTgfPacketVlanTagPart.vid ==  PRV_TGF_DEF_VLANID_CNS)) ? 1 : 0));

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfTriggersArr[sendIter], numTriggers,
                         "get another trigger that expected: expected - %d, recieved - %d\n",
                         prvTgfTriggersArr[sendIter], numTriggers);
    }

    /* check that only those records created, that expected */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        for (burstCountIter = 0; burstCountIter < prvTgfBurstCount; burstCountIter++)
        {
            /* AUTODOC: check that FDB entry with correct VlanId and MAC is created  */
            rc = prvTgfFdbEntryCheckByVlanAndMac(prvTgfSendVlanArr[sendIter],
                                                 prvTgfSaMacArr[burstCountIter],
                                                 &valid);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfFdbEntryCheckByVlanAndMac %d, %d", 
                                         prvTgfSendVlanArr[sendIter], prvTgfSaMacArr[burstCountIter]);

            /* compare expected result with received */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfFdbEntryArr[sendIter][burstCountIter], valid, 
                                         "expected FDB state - %d, received - %d", 
                                         prvTgfFdbEntryArr[sendIter][burstCountIter], valid);
        }
    }
}

/**
* @internal prvTgfFdbLearningAdressesSvlModeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbLearningAdressesSvlModeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry 5 (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* invalidate vlan entry 13 (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_13_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_13_CNS);

    /* AUTODOC: restore the VLAN Lookup mode  */
    rc = prvTgfBrgFdbMacVlanLookupModeSet(prvTgfDefVlanLookupMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacVlanLookupModeSet: %d", prvTgfDefVlanLookupMode);
}


