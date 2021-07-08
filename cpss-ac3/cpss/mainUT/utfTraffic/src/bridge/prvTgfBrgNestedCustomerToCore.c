/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgInvalidVlanFiltering.c
*
* DESCRIPTION:
*       Nested customer to core
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <bridge/prvTgfVlanManipulation.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgNestedCustomerToCore.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/*  VLAN Id 3 */
#define PRV_TGF_VLANID_3_CNS                    3

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               2

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            0

/* C-VLAN ether type */
#define PRV_TGF_ETHER_TYPE_1_CNS                0x8100

/* S-VLAN ether type */
#define PRV_TGF_ETHER_TYPE_2_CNS                0x9200

/* number of vfd's */
#define PRV_TGF_VFD_NUM_CNS                             4

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS                        2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_PORTS_NUM_CNS] =
{
    1, 0, 1, 0
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_PORTS_NUM_CNS] =
{
    1, 0, 1, 0
};

/* expected number of Triggers on ports with index 0 */
static GT_U8 prvTgfPacketsCountTriggerArr[] =
{
    1, 1
};

/* indication that we check the egress port as dsa tag port */
static GT_BOOL              testEgressPortAsDsa = GT_FALSE;
/* The DSA tag type for the egress port - relevant when testEgressPortAsDsa = GT_TRUE */
static TGF_DSA_TYPE_ENT     testEgressDsaType = TGF_DSA_LAST_TYPE_E;

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
    0, 0, PRV_TGF_VLANID_3_CNS                          /* pri, cfi, VlanId */
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

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     EgressCascadePort_DsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xd0,0x20,0x10,0x03,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x20,0x00,0x1f,0xff,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* expected DSA words on egress port */
static TGF_VFD_INFO_STC     EgressCascadePort_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 8*0 /*offset*/, {0xd0,0x20,0x10,0x03,0xa0,0x00,0x1f,0xff}, {0,0,0,0,0,0,0,0}, 8/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 8*1 /*offset*/, {0x80,0x00,0x00,0x00,0x00,0x00,0x12,0x40}, {0,0,0,0,0,0,0,0}, 8/*cycleCount*/ , NULL,0,0}
};
/*************************** Restore config ***********************************/

/* Parameters needed to be restored */

/* stored default VLAN Range */
static GT_U16   prvTgfDefVidRange = 0;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgNestedCustomerToCoreConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgNestedCustomerToCoreConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 0, 0, 0};
    GT_HW_DEV_NUM hwDevNum;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (testEgressPortAsDsa &&
        UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_CAPTURE_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: create VLAN 3 with tagged port 0 and untagged port 2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_3_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* add port with index 2 to Vlan 3*/
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_3_CNS,
                                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_3_CNS,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID to 3 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_3_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_3_CNS);

    /* AUTODOC: store default VLAN range */
    rc = prvTgfBrgVlanRangeGet(&prvTgfDefVidRange);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet: %d", prvTgfDevNum);

    /* AUTODOC: set egress vlan EtherType1 to 0x9200 */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E, PRV_TGF_ETHER_TYPE_2_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

    /* AUTODODC: set VLAN EtherType1 for tagged packets transmitted via port 0 */
    rc = prvTgfBrgNestVlanPortEgressEtherTypeSelectSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                                       CPSS_VLAN_ETHERTYPE1_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanPortEgressEtherTypeSelectSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set Core or Customer port on port 0 */
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* AUTODOC: set nested VLAN Access Port on port 2 */
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_DsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
}

/**
* @internal prvTgfBrgNestedCustomerToCoreTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag on egress port
*                                       None
*/
GT_STATUS prvTgfBrgNestedCustomerToCoreTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
)
{
    testEgressPortAsDsa = GT_TRUE;
    testEgressDsaType   = egressDsaType;
    return GT_OK;
}

/**
* @internal prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    GT_U32                      packetLen       = 0;
    GT_U32                      numTriggers     = 0;
    GT_U32                      tmpTrigger      = 0;
    TGF_VFD_INFO_STC            vfdArray[4];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      sizeDSATag = 0;
    TGF_VFD_INFO_STC            *currentDsaWordsVfdPtr;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set vfd for S-VLAN  tag */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = PRV_TGF_ETHER_TYPE_2_CNS >> 8;
    vfdArray[0].patternPtr[1] = PRV_TGF_ETHER_TYPE_2_CNS & 0xFF;
    vfdArray[0].patternPtr[3] = PRV_TGF_VLANID_3_CNS;

    /* set vfd for nested Vlan (C-VLAN) tag */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[1].cycleCount = 4;
    vfdArray[1].patternPtr[0] = PRV_TGF_ETHER_TYPE_1_CNS >> 8;
    vfdArray[1].patternPtr[1] = PRV_TGF_ETHER_TYPE_1_CNS & 0xFF;
    vfdArray[1].patternPtr[3] = PRV_TGF_VLANID_3_CNS;

    /* set Egress port to be DSA tagged */
    if(GT_TRUE == testEgressPortAsDsa)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_TRUE, testEgressDsaType);
    }

    if(testEgressPortAsDsa == GT_TRUE)
    {
        sizeDSATag = (TGF_eDSA_TAG_SIZE_CNS *((testEgressDsaType == TGF_DSA_4_WORD_TYPE_E) ? 1 : 0)) +
                     (TGF_DSA_TAG_SIZE_CNS * ((testEgressDsaType == TGF_DSA_2_WORD_TYPE_E) ? 1 : 0)) -
                     TGF_VLAN_TAG_SIZE_CNS;

        prvTgfEgressPortCascadeSet(&portInterface, GT_TRUE, testEgressDsaType);

        if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
        {
            currentDsaWordsVfdPtr = &EgressCascadePort_DsaWordsVfd[0];
            if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
            {
                /* <SrcTagged> not indicate tag1 , but tag 0 only (tag 1 was recognized at ingress tag 0 not)*/
                currentDsaWordsVfdPtr->patternPtr[0] |= 1 << 5;/*bit 29 word 0*/

                /* those device not support setting the 'default port vid1' and are with value 0 */
                currentDsaWordsVfdPtr->patternPtr[2] &= 0xF0;
                currentDsaWordsVfdPtr->patternPtr[3]  = 0x00;
            }
        }
        else
        {
            currentDsaWordsVfdPtr = &EgressCascadePort_eDsaWordsVfd[0];
        }

       /* update the DSA with runtime info */
        {
            GT_U8   dsaBytes[16];
            GT_U8*  dsaBytesPtr = &dsaBytes[0];
            TGF_PACKET_DSA_TAG_STC tmpDsa;
            GT_U32  ii,jj,kk;
            GT_U32  currentOffset = 0;
            GT_U32  jjMax = 4;

            for(jj = 0 ; jj < jjMax ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    dsaBytes[currentOffset] = currentDsaWordsVfdPtr[jj].patternPtr[kk];
                }
            }

            for(/*currentOffset continue*/;currentOffset < 16 ; currentOffset++)
            {
                dsaBytes[currentOffset] = 0;
            }

            rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
                &tmpDsa);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                      prvTgfDevNum);

            /* update the DSA tag format */
            tmpDsa.dsaInfo.forward.source.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
            tmpDsa.dsaInfo.forward.origSrcPhy.portNum = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                                prvTgfDevNum,
                                                                tmpDsa.dsaInfo.forward.source.portNum);

            tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
            tmpDsa.dsaInfo.forward.dstEport = tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum;

             /* SIP6 : the <skipFdbSaLookup> in the eDSA word[2] bit [30] got value 1 , used to be 'reserved' in legacy devices*/ 
            if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_TRUE;
            }
            else
            {
                tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_FALSE;
            }
            /* convert the DSA into into bytes back */
            rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                      prvTgfDevNum);

            currentOffset = 0;

            for(jj = 0 ; jj < jjMax ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    currentDsaWordsVfdPtr[jj].patternPtr[kk] = dsaBytes[currentOffset];
                }
            }

        }

        vfdArray[0] = currentDsaWordsVfdPtr[0];
        vfdArray[1] = currentDsaWordsVfdPtr[1];
    }

    PRV_UTF_LOG0_MAC("========== Sending packet==========\n\n");

    /* AUTODOC: send tagged packet on port 2: */
    /* AUTODOC:    DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* check ETH counter */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        packetLen = prvTgfTaggedPacketInfo.totalLen + (TGF_VLAN_TAG_SIZE_CNS *
                    ((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) ? 1 : 0));
        if(testEgressPortAsDsa == GT_TRUE && portIter == 0)
        {
                packetLen +=sizeDSATag;
        }

        /* AUTODOC: verify to get traffic on port 0 */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxArr[portIter],
                                    prvTgfPacketsCountTxArr[portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters  */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check first pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & 1) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[0], tmpTrigger,
                         "get another trigger for VLAN tag that expected: expected - %d, recieved - %d\n",
                         prvTgfPacketsCountTriggerArr[0], tmpTrigger);

    /* check second pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & (1 << 1)) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[1], tmpTrigger,
                          "get another trigger for nested VLAN tag that expected: expected - %d, recieved - %d\n",
                          prvTgfPacketsCountTriggerArr[1], tmpTrigger);
    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_FALSE, testEgressDsaType);
        testEgressPortAsDsa = GT_FALSE;
        testEgressDsaType = TGF_DSA_LAST_TYPE_E;
    }
}

/**
* @internal prvTgfBrgNestedCustomerToCoreTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCustomerToCoreTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    GT_U32                      packetLen       = 0;
    GT_U32                      numTriggers     = 0;
    GT_U32                      tmpTrigger      = 0;
    TGF_VFD_INFO_STC            vfdArray[2];
    CPSS_INTERFACE_INFO_STC     portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    PRV_UTF_LOG0_MAC("========== Sending packet==========\n\n");

    /* AUTODOC: send tagged packet on port 2: */
    /* AUTODOC:    DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* check ETH counter */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        packetLen = prvTgfTaggedPacketInfo.totalLen + (TGF_VLAN_TAG_SIZE_CNS *
                    ((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) ? 1 : 0));

        /* AUTODOC: verify to get traffic on port 0 */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxArr[portIter],
                                    prvTgfPacketsCountTxArr[portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* get Trigger Counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set vfd for S-VLAN  tag */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = PRV_TGF_ETHER_TYPE_2_CNS >> 8;
    vfdArray[0].patternPtr[1] = PRV_TGF_ETHER_TYPE_2_CNS & 0xFF;
    vfdArray[0].patternPtr[3] = PRV_TGF_VLANID_3_CNS;

    /* set vfd for nested Vlan (C-VLAN) tag */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[1].cycleCount = 4;
    vfdArray[1].patternPtr[0] = PRV_TGF_ETHER_TYPE_1_CNS >> 8;
    vfdArray[1].patternPtr[1] = PRV_TGF_ETHER_TYPE_1_CNS & 0xFF;
    vfdArray[1].patternPtr[3] = PRV_TGF_VLANID_3_CNS;

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check first pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & 1) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[0], tmpTrigger,
                         "get another trigger for VLAN tag that expected: expected - %d, recieved - %d\n",
                         prvTgfPacketsCountTriggerArr[0], tmpTrigger);

    /* check second pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & (1 << 1)) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[1], tmpTrigger,
                          "get another trigger for nested VLAN tag that expected: expected - %d, recieved - %d\n",
                          prvTgfPacketsCountTriggerArr[1], tmpTrigger);
}

/**
* @internal prvTgfBrgNestedCustomerToCoreConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgNestedCustomerToCoreConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default nested VLAN access config on port 2 */
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore default VLAN EtherType for packets transmitted via port 0 */
    rc = prvTgfBrgNestVlanPortEgressEtherTypeSelectSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                                       CPSS_VLAN_ETHERTYPE0_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanPortEgressEtherTypeSelectSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: restore default egress vlan ether type */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                                         PRV_TGF_ETHER_TYPE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

    /* invalidate VLAN 3 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_3_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: restore default VLAN range */
    rc = prvTgfBrgVlanRangeSet(prvTgfDefVidRange);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVidRange);
}


