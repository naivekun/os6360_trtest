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
* @file prvTgfBrgNestedCoreToCustomer.c
*
* @brief Nested core to customer
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <bridge/prvTgfVlanManipulation.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgNestedCoreToCustomer.h>
#include <common/tgfMirror.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/*  VLAN Id 3 */
#define PRV_TGF_VLANID_3_CNS                    3

/*  VLAN Id 5 */
#define PRV_TGF_VLANID_5_CNS                    5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            2

/* C-VLAN ether type */
#define PRV_TGF_ETHER_TYPE_1_CNS                0x8100

/* S-VLAN ether type */
#define PRV_TGF_ETHER_TYPE_2_CNS                0x9200

/* max Vid Range */
#define PRV_TGF_MAX_VID_RANGE_CNS               10

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* stored default VLAN Range */
static GT_U16   prvTgfDefVidRange = 0;

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

/* expected number of Triggers on portIdx 2 */
static GT_U8 prvTgfPacketsCountTrigger = 1;

/* expected number of Triggers on portIdx 2 */
static GT_U8 prvTgfEgrCascPortPacketsCountTrigger = 3;


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
    PRV_TGF_ETHER_TYPE_2_CNS,                           /* etherType */
    0, 0, PRV_TGF_VLANID_3_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x81, 0x00, 0x00, 0x05, 0x11, 0x12, 0x07, 0x08,
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

/*************************** Restore config ***********************************/

/* Parameters needed to be restored */

/* default number of packets to send */
static GT_U32  prvTgfTpidBmpRestore;

/* indication that we check the egress port as dsa tag port */
static GT_BOOL              testEgressPortAsDsa = GT_FALSE;
/* The DSA tag type for the egress port - relevant when testEgressPortAsDsa = GT_TRUE */
static TGF_DSA_TYPE_ENT     testEgressDsaType = TGF_DSA_LAST_TYPE_E;

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     prvTgfBrgNestedCoreToCustomerEgressCascadePort_dsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x00,0x10,0x03,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x00,0x00,0x1f,0xff,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0,0,0,0,0,0} ,             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0,0,0,0,0,0} ,             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     prvTgfBrgNestedCoreToCustomerEgressCascadePort_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x00,0x10,0x03,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x80,0x00,0x1f,0xff,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0x80,0x00,0x00,0x01,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0x02,0x00,0x00,0x40,0,0} , {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/**
* @internal prvTgfBrgNestedCoreToCustomerEgressCascadePortPacketSend function
* @endinternal
*
* @brief   Function sends packet to egress port. Substitutes VLAN tag with DSA tag on egress port
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
*                                       None
*/
static GT_VOID  prvTgfBrgNestedCoreToCustomerEgressCascadePortPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   portIter        = 0;
    GT_U32                   packetLen       = 0;
    GT_U32                   numTriggers     = 0;
    TGF_VFD_INFO_STC         vfdArray[10];
    TGF_VFD_INFO_STC         *currentDsaWordsVfdPtr;
    GT_U32                   ii = 0;

    prvTgfEgressPortCascadeSet(portInterfacePtr, GT_TRUE, testEgressDsaType);
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
    {
        currentDsaWordsVfdPtr = &prvTgfBrgNestedCoreToCustomerEgressCascadePort_dsaWordsVfd[0];
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
        currentDsaWordsVfdPtr = &prvTgfBrgNestedCoreToCustomerEgressCascadePort_eDsaWordsVfd[0];
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
        tmpDsa.dsaInfo.forward.origSrcPhy.portNum = tmpDsa.dsaInfo.forward.source.portNum;

        tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
        tmpDsa.dsaInfo.forward.dstEport = tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum;


        /* convert the DSA into into bytes back */
        rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                  prvTgfDevNum);

        if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            /* the device want bit 25 in word 3 to be set to 1 <Src Tag0 Is Outer Tag> */
            dsaBytes[(3*4)+0] |= 1 << 1;
        }

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


    vfdArray[ii++] = currentDsaWordsVfdPtr[0];
    vfdArray[ii++] = currentDsaWordsVfdPtr[1];
    if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
    {
        vfdArray[ii++] = currentDsaWordsVfdPtr[2];
        vfdArray[ii++] = currentDsaWordsVfdPtr[3];
    }

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("========== Sending packet==========\n\n");

    /* AUTODOC: send tagged packet on port 0: */
    /* AUTODOC:    DA=00:00:00:00:00:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:    EtherType=0x9200, VID=3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterfacePtr->devPort.portNum);

    /* AUTODOC: verify to get packet on port 2 with: */
    /* AUTODOC:    DSA/eDSA tag */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        packetLen = prvTgfTaggedPacketInfo.totalLen +
                    (TGF_eDSA_TAG_SIZE_CNS *(((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) & (testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)) ? 1 : 0)) +
                    (TGF_DSA_TAG_SIZE_CNS * (((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) & (testEgressDsaType == TGF_DSA_2_WORD_TYPE_E)) ? 1 : 0)) -
                    (TGF_VLAN_TAG_SIZE_CNS * ((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) ? 1 : 0));

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxArr[portIter],
                                    prvTgfPacketsCountTxArr[portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* get Trigger Counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(portInterfacePtr, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfEgrCascPortPacketsCountTrigger, numTriggers,
                         "get another trigger for VLAN tag that expected: expected - %d, recieved - %d\n",
                         prvTgfEgrCascPortPacketsCountTrigger, numTriggers);

}
/**
* @internal prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*/
GT_STATUS prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
)
{
    testEgressPortAsDsa = GT_TRUE;
    testEgressDsaType   = egressDsaType;
    return GT_OK;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgNestedCoreToCustomerConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgNestedCoreToCustomerConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 0, 0, 0};
    GT_U32      tpidBmp = 0;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */


    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_SEND_PORT_IDX_CNS,
                                   testEgressPortAsDsa ? PRV_TGF_CAPTURE_PORT_IDX_CNS : -1,
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
                                prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_3_CNS,
                                 prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* AUTODOC: store default VLAN range */
    rc = prvTgfBrgVlanRangeGet(&prvTgfDefVidRange);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet: %d", prvTgfDevNum);

    /* AUTODOC: set egress vlan EtherType0 to 0x9200 */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_ETHER_TYPE_2_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

    /* AUTODOC: set ingress vlan EtherType1 to 0x9200 and max VID range to 10 */
    rc = prvTgfBrgVlanIngressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                                          PRV_TGF_ETHER_TYPE_2_CNS, PRV_TGF_MAX_VID_RANGE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeSeti");

    /* Sets VLAN EtherType for tagged packets transmitted via ingress portIdx 0 */
    rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         &prvTgfTpidBmpRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d",
                                 prvTgfDevNum);


    /* bitmap */
    tpidBmp = CPSS_VLAN_ETHERTYPE1_E;

    /* AUTODOC: set VLAN EtherType1 for tagged packets transmitted via ingress port 0 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         tpidBmp);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: configure port 0 as Core or Customer port */
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: configure port 2 as nested VLAN Access Port */
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBrgNestedCoreToCustomerEgressCascadePort_dsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBrgNestedCoreToCustomerEgressCascadePort_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
}

/**
* @internal prvTgfBrgNestedCoreToCustomerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCoreToCustomerTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    GT_U32                      packetLen       = 0;
    GT_U32                      numTriggers     = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      errata_eqToCpuForRxAnalyzerSrcBasedMirrorTrgInfoInsteadOfSrcInfo;
    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mirrorMode;
    /* get mirror mode */
    prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,&mirrorMode);

    /* in eArch devices:
       cpu port which is rx analyzer port in MirroringMode == SRC_FWD_OVERRIDE_MIRRORING_MODE
                        get 'target port,device' instead of 'source port,device' */
    errata_eqToCpuForRxAnalyzerSrcBasedMirrorTrgInfoInsteadOfSrcInfo =
        mirrorMode != PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E ? 0 :
        (GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS)) ? 1 : 0;

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
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    PRV_UTF_LOG0_MAC("========== Sending packet==========\n\n");

    /* AUTODOC: send tagged packet on port 0: */
    /* AUTODOC:    DA=00:00:00:00:00:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:    EtherType=0x9200, VID=3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: verify to get packet on port 2 with: */
    /* AUTODOC:    EtherType=0x8100, VID=5 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        packetLen = prvTgfTaggedPacketInfo.totalLen - (TGF_VLAN_TAG_SIZE_CNS * ((portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS) ? 1 : 0));

        /* check ETH counters */
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
    vfdArray[0].patternPtr[0] = PRV_TGF_ETHER_TYPE_1_CNS >> 8;
    vfdArray[0].patternPtr[1] = PRV_TGF_ETHER_TYPE_1_CNS & 0xFF;
    vfdArray[0].patternPtr[3] = PRV_TGF_VLANID_5_CNS;

    if(errata_eqToCpuForRxAnalyzerSrcBasedMirrorTrgInfoInsteadOfSrcInfo)
    {
        portInterface.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;
    }

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    if(errata_eqToCpuForRxAnalyzerSrcBasedMirrorTrgInfoInsteadOfSrcInfo)
    {
        /*check also the src eport */

        tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E);

        portInterface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("ePort check : numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E);
    }



    /* check TriggerCounters */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTrigger, numTriggers,
                         "get another trigger for VLAN tag that expected: expected - %d, recieved - %d\n",
                         prvTgfPacketsCountTrigger, numTriggers);
}

/**
* @internal prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    CPSS_INTERFACE_INFO_STC     portInterface;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

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
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* Send packet */
    prvTgfBrgNestedCoreToCustomerEgressCascadePortPacketSend(&portInterface);

    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_FALSE, testEgressDsaType);
        testEgressPortAsDsa = GT_FALSE;
        testEgressDsaType = TGF_DSA_LAST_TYPE_E;
    }
}

/**
* @internal prvTgfBrgNestedCoreToCustomerConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgNestedCoreToCustomerConfigRestore
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
    rc = prvTgfBrgNestVlanAccessPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgNestVlanAccessPortEnableSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* AUTODOC: restore VLAN EtherType1 for tagged packets transmitted via ingress port 0 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         prvTgfTpidBmpRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: restore default ingress vlan EtherType1 and max VID range */
    rc = prvTgfBrgVlanIngressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                                          PRV_TGF_ETHER_TYPE_1_CNS, 4094);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeSet");

    /* AUTODOC: restore default egress vlan EtherType0 */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_ETHER_TYPE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

    /* AUTODOC: restore default egress vlan EtherType1 */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E, PRV_TGF_ETHER_TYPE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

    /* invalidate VLAN 3 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_3_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: restore default VLAN range */
    rc = prvTgfBrgVlanRangeSet(prvTgfDefVidRange);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVidRange);
}


