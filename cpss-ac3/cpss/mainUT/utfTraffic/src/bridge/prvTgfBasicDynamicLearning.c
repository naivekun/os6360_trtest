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
* @file prvTgfBasicDynamicLearning.c
*
* @brief Basic Dynamic Learning
*
* @version   27
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBasicDynamicLearning.h>
#include <common/tgfCscdGen.h>

/* for VC as usually used compiler - check mismatch of sizeof().  */
#ifdef _VISUALC
    #define CHECK_SIZE_MISMATCH
#endif

extern GT_VOID prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(IN GT_BOOL setConfig);
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS 0
/* indication if current run is for DSA tag testing */
static GT_BOOL      testingDsaTag = GT_FALSE;
/* indication if packet from cascade port should bypass the bridge or not (for destination) */
static GT_BOOL      testingDsaBypassBridge = GT_FALSE;

/* the DSA 'src devNum' */
#define SRC_DSA_DEV_NUM(devNum)     (GT_HW_DEV_NUM)((devNum) + 4)

static GT_U32   origDevTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] = {0};

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}};
/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[][PRV_TGF_PORTS_NUM_CNS - 1] = {
                                        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
                                        },
                                        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
                                        },
                                        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x04}
                                        },
                                        {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
                                        }
                                       };

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
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
    0x01, 0x04, 0x07, 0x0a, 0x0d, 0x01, 0x04, 0x07,
    0x02, 0x05, 0x08, 0x0b, 0x0e, 0x02, 0x05, 0x08,
    0x03, 0x06, 0x09, 0x0c, 0x0f, 0x03, 0x06, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x43
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
#define VLAN_TAG_INFO_INDEX  1 /* index in prvTgfPacketPartArray[]*/
#define DSA_INFO_INDEX  2 /* index in prvTgfPacketPartArray[]*/

/* the eDsa target eport - relevant to target when eDSA and bypassBridge = GT_TRUE */
#define TARGET_EPORT_NUM_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1CAB)

/* dummy value : the eDSA target eport - relevant to target when eDSA and bypassBridge = GT_FALSE */
#define DUMMY_TARGET_EPORT_NUM_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1BBB)

/* the eDsa source eport - relevant to source learning when eDSA .
make sure that the 6 LSB are 0 , to allow device map table to get the 6 bits from
the physical port ... see use of :
source eport = SOURCE_EPORT_NUM_CNS + (prvTgfPortsArray[portIter] + 1)
*/
#define SOURCE_EPORT_NUM_CNS    (ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1A80) & ~(0x7f))

/* the eDSA supports 17 bits for the srcEPort even though the ingress pipe of the device supports less */
#define EDSA_SOURCE_EPORT_NUM_CNS(basePort)    (basePort | BIT_16)

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    3,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        19/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_FALSE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_CNS,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E    , NULL},/* will be used for DSA tag testing */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of untagged packet */
#define PRV_TGF_UNTAGGED_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr) + TGF_CRC_LEN_CNS)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (calculatedPacketLenIncludeCrc /*+ TGF_CRC_LEN_CNS*/)

/* Tx delay */
#define PRV_TGF_TX_DELAY_CNS                300

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};


/* PARTS of the dummy packet for capturing the sender */
static TGF_PACKET_PART_STC dummyExpectedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E    , NULL},/* will be used for DSA tag testing */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};


/* dummy packet for capturing the sender */
static TGF_PACKET_STC dummyExpectedPacket = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                      /* totalLen */
    sizeof(dummyExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    dummyExpectedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default L2 part */
static TGF_PACKET_L2_STC prvTgfDefPacketL2Part;

static GT_U32   captureByTtiPortIndex = 0;
static PRV_TGF_TTI_MAC_MODE_ENT origMacMode = 0;

/* tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           88
/* PCLID for the rule - for the ethernet packets */
#define PRV_TGF_TTI_ETHERNET_PCL_ID_CNS           0x1FC

/* original PCL id for ethernet packets -- in the TTi lookup */
static GT_U32   origEthernetTtiPclId = 0;

static GT_BOOL supportTunnelCapture = GT_TRUE;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal tunnelInit function
* @endinternal
*
* @brief   init tunnel termination relate to the test.
*
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID tunnelInit
(
    IN GT_U32  senderPortIndex
)
{
    GT_STATUS   rc      ;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_2_STC      ruleAction;
    GT_U32      ii;

    if(supportTunnelCapture == GT_FALSE)
    {
        return;
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    rc = prvTgfTtiMacModeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_MAC_MODE_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&origEthernetTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_ETHERNET_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.command         = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    ruleAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 15;
    ruleAction.redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
    /* the TTI override this field overrides any previous assignment,
       that may have been assigned by the port configuration , so the 'per port'
       is not relevant any more ! */
    ruleAction.bridgeBypass = testingDsaBypassBridge;

    /* configure TTI rule */
    pattern.eth.common.pclId    = PRV_TGF_TTI_ETHERNET_PCL_ID_CNS;
    pattern.eth.common.srcIsTrunk = GT_FALSE;
    pattern.eth.common.srcPortTrunk = prvTgfPortsArray[senderPortIndex];
    cpssOsMemCpy(pattern.eth.common.mac.arEther, prvTgfSaMacArr[senderPortIndex], sizeof(TGF_MAC_ADDR));

    mask.eth.common.pclId    = BIT_10 - 1;
    mask.eth.common.srcIsTrunk = 1;
    mask.eth.common.srcPortTrunk = BIT_7 - 1;
    for(ii = 0 ; ii < 6 ; ii++)
    {
        mask.eth.common.mac.arEther[ii] = 0xFF;
    }

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");


}

/**
* @internal checkCaptureIngressCopy function
* @endinternal
*
* @brief   check captured traffic for the sending port.
*
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static void checkCaptureIngressCopy
(
    IN GT_U32       senderPortIndex,
    IN GT_U32       numOfExpectedPackets
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC *packetInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/

    if(supportTunnelCapture == GT_FALSE)
    {
        return;
    }


#ifdef CHECK_SIZE_MISMATCH
    if(sizeof(prvTgfPacketPartArray) != sizeof(dummyExpectedPacketPartArray))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"checkCaptureIngressCopy: mismatch packets size");
        return;
    }
#endif /*CHECK_SIZE_MISMATCH*/

    /* we wait for :
        number of expected copies for mirror to CPU (by the TTI)
    */

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait( numOfExpectedPackets , 500, NULL);

    cpssOsMemCpy(dummyExpectedPacketPartArray,
        prvTgfPacketPartArray,
        sizeof(dummyExpectedPacketPartArray));

    packetInfoPtr = &dummyExpectedPacket;

    /* the capture should have the vlan tag and not the DSA tag when captured to the CPU */
    dummyExpectedPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
    dummyExpectedPacketPartArray[DSA_INFO_INDEX].partPtr = NULL;
    dummyExpectedPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    /* although we set capture on the senderPortIndex , we get in the CPU
       indication about the 'orig port' --> from the DSA */

    portInterface.devPort.hwDevNum = prvTgfPacketDsaTagPart_forward.srcHwDev;
    portInterface.devPort.portNum = prvTgfPacketDsaTagPart_forward.source.portNum;

    if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        /* supports for port >= 128 */
        portInterface.devPort.portNum &= 0x7f;/* 7 bits in the DSA */
    }

    /* relevant to E_ARCH devices : state that we want the 'ePort' compared */
    tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", prvTgfPortsArray[senderPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            packetInfoPtr,
            numOfExpectedPackets,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* for E_ARCH device we also check the physical port value */
        /* relevant to E_ARCH devices : state that we want the 'phyPort' compared */
        tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_FORCE_PHYSICAL_PORT_NUM_E);

        /* reset the previous value */
        actualCapturedNumOfPackets = 0;

        portInterface.devPort.hwDevNum = prvTgfPacketDsaTagPart_forward.srcHwDev;
        portInterface.devPort.portNum = prvTgfPortsArray[senderPortIndex];/*the physical port in the 'own' device */

        /* the TO_CPU is 4 WORDs EDSA regardless to 'test 1 words' and it
           includes all needed bits of 'physical port' in the 'own' device !!!
        */

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", prvTgfPortsArray[senderPortIndex]);

        /* print captured packets and check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                packetInfoPtr,
                numOfExpectedPackets,/*numOfPackets*/
                0/*vfdNum*/,
                NULL /*vfdArray*/,
                NULL, /* bytesNum's skip list */
                0,    /* length of skip list */
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    }
    /* restore default */
    tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}


/**
* @internal tunnelRestore function
* @endinternal
*
* @brief   restore tunnel configurations.
*/
static GT_VOID tunnelRestore
(
    IN GT_U32       senderPortIndex
)
{
    GT_STATUS   rc      ;

    if(supportTunnelCapture == GT_FALSE)
    {
        return;
    }

    /* stop capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* restore PCL id for the ethernet packets */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,origEthernetTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");


    prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS,GT_FALSE);

    /* disable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* restore mac mode for the ethernet packets  */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/* restore e2Phy configurations */
static void restoreE2Phy(void)
{
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U32  portIter;
    GT_U32  portsCount = prvTgfPortsNum;
    GT_PORT_NUM trgEPort;

#ifdef CHX_FAMILY
    if(0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        return;
    }
#endif /*CHX_FAMILY*/

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = 0;

    for (portIter = 0; portIter < portsCount; portIter++)
    {
        trgEPort = TARGET_EPORT_NUM_CNS + portIter;

        prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                    trgEPort,
                    &physicalInfo);
    }

    /* restore also cases of phy-port range mapping ! */
    prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_FALSE);


    return;
}

GT_VOID prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(IN GT_BOOL setConfig)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  portIter,portsCount = prvTgfPortsNum;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_PORT_NUM trgEPort;/* target eport */

    if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->hwDevNum) ||
       GT_FALSE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* the 7'th bit come from the hwDevNum bit 0 */
        /* so no WA needed */
        return;
    }


    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;

    for (portIter = 0; portIter < portsCount; portIter++)
    {
        if ((setConfig == GT_TRUE) && (prvTgfPortsArray[portIter] >= 64))
        {
            /* this value can not be passed in DSA tag .
               so map the e2PHY to redirect such 6 bits (ePort)
               value to it's full 7/8 bits value PHYSICAL port value.
               to egress proper physical port.
            */
            physicalInfo.devPort.portNum = prvTgfPortsArray[portIter];
            trgEPort = prvTgfPortsArray[portIter] & 0x3f;
        }
        else
        {
            physicalInfo.devPort.portNum = prvTgfPortsArray[portIter];
            trgEPort = prvTgfPortsArray[portIter];
        }

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                    trgEPort,
                    &physicalInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                     prvTgfDevNum, trgEPort);
    }
}

/**
* @internal prvTgfBrgBdlConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgBdlConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter     = 0;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_PORT_NUM trgEPort;/* analyzer target eport */

    /* AUTODOC: SETUP CONFIGURATION: */

    /* save L2 packet's part */
    cpssOsMemCpy(&prvTgfDefPacketL2Part, &prvTgfPacketL2Part, sizeof(prvTgfDefPacketL2Part));

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    if(testingDsaTag == GT_TRUE)
    {
        /* set tunnel without termination just to make 'mirror to cpu' */
        tunnelInit(captureByTtiPortIndex);

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            /* set the e2Phy to set physical port for the eport TARGET_EPORT_NUM_CNS */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum = prvTgfDevNum;

            for (portIter = 0; portIter < portsCount; portIter++)
            {
                physicalInfo.devPort.portNum = prvTgfPortsArray[portIter];

                trgEPort = TARGET_EPORT_NUM_CNS + portIter;

                rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                            trgEPort,
                            &physicalInfo);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                             prvTgfDevNum, trgEPort);
            }
        }
        else if(0 == PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->hwDevNum))/* TGF_DSA_2_WORD_TYPE_E */
        {
            prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_TRUE);
        }
    }

}

/**
* @internal prvTgfBrgBdlTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgBdlTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;
    GT_U32          portsCount   = 4;
    GT_U32          portIter     = 0;
    GT_U32          ucPortIter   = 0;
    GT_U32          ucPortToIter = 0;
    GT_U32          fdbIter      = 0;
    GT_U32          fdbCount     = sizeof(prvTgfSaMacArr) / sizeof(TGF_MAC_ADDR);
    GT_U32          calculatedPacketLenIncludeCrc;/* calculated packet length that sent from the CPU */
    GT_U32          expectedFdbPort[4] = {0,0,0,0};/*same size as prvTgfSaMacArr*/
    GT_U32          targetPortIndex;/*target port index */

    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: 1. BC phase - send BC packet from each port to setup FDB table */
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

    if(testingDsaTag == GT_TRUE)
    {
        /* set the target port of the packet in the DSA tag info
            NOTE: when the mode is 'NOT bypass bridge' this info will NOT be used (so do DA lookup ... flooding),
                  but when the mode is 'bypass bridge' this info will be used

                  see testingDsaBypassBridge
        */
        prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
        prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = prvTgfDevNum;

        /* indication that the source device found the FDB entry in it's FDB */
        prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_TRUE;

    }

    /* AUTODOC: send BC packet from each port with different MAC SA: */
    /* AUTODOC:   00:00:00:00:00:01 */
    /* AUTODOC:   00:00:00:00:00:02 */
    /* AUTODOC:   00:00:00:00:00:03 */
    /* AUTODOC:   00:00:00:00:00:04 */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* set source MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[portIter],
                     sizeof(prvTgfPacketL2Part.saMac));

        /* set the DSA info to be compatible */
        if(testingDsaTag == GT_TRUE)
        {
            /* set the target port of the packet in the DSA tag info
                NOTE: when the mode is 'NOT bypass bridge' this info will NOT be used (so do DA lookup ... flooding),
                      but when the mode is 'bypass bridge' this info will be used

                      see testingDsaBypassBridge
            */
            /* when testingDsaBypassBridge = GT_TRUE -->
               make the traffic from port prvTgfPortsArray[x] to be send to prvTgfPortsArray[x+1] */
            prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum = prvTgfPortsArray[(portIter+1)%portsCount];

            if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
            {
                /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
                expectedFdbPort[portIter] = prvTgfPortsArray[portIter + prvTgfPortsNum];/* take valid port beyond the tested ports */
            }
            else
            {
                /* set the info for the 'source learning' */
                expectedFdbPort[portIter] = (prvTgfPortsArray[portIter] + 1);/* use the 'similar' port numbers as on 'local' device */
            }

            if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                expectedFdbPort[portIter] += SOURCE_EPORT_NUM_CNS;

                prvTgfPacketDsaTagPart_forward.isTrgPhyPortValid = GT_FALSE;
                if(testingDsaBypassBridge == GT_TRUE)
                {
                    prvTgfPacketDsaTagPart_forward.dstEport = TARGET_EPORT_NUM_CNS + ((portIter + 1) % prvTgfPortsNum) ;
                }
                else
                {
                    prvTgfPacketDsaTagPart_forward.dstEport = DUMMY_TARGET_EPORT_NUM_CNS;
                }

                prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk = GT_FALSE;
                prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
            }

            if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                /* the eDSA can hold 17 bits of srcEPort , even though the FDB will hold less bits ! */
                prvTgfPacketDsaTagPart_forward.source.portNum = EDSA_SOURCE_EPORT_NUM_CNS(expectedFdbPort[portIter]);
            }
            else
            {
                prvTgfPacketDsaTagPart_forward.source.portNum = expectedFdbPort[portIter];
            }

            /* do this right before the send of packet */
            /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
            prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

            rc =tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter],
                            prvTgfPortsArray[portIter],
                            GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                /* NOTE: this not change the ingress eDSA but only the 'Expected captured ingress eDSA' */

                /* restore value for the 'compare' as the device ignored the
                   extra bits and more ever also reset those bits in it's descriptor */
                prvTgfPacketDsaTagPart_forward.source.portNum = expectedFdbPort[portIter];
            }
        }

        /* send Packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(testingDsaTag == GT_TRUE)
        {
            rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter],
                            prvTgfPortsArray[portIter],
                            GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


            if(captureByTtiPortIndex == portIter)
            {
                /* check captured traffic for the sending port . */
                checkCaptureIngressCopy(captureByTtiPortIndex, prvTgfBurstCount );
            }
        }
    }

    /* calculate the packet that was sent from the CPU */
    rc = tgfTrafficEnginePacketLengthCalc(&prvTgfPacketInfo,&calculatedPacketLenIncludeCrc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketLengthCalc: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* AUTODOC: read and verify counters on all ports */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(testingDsaTag == GT_TRUE &&
           prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr)
        {
            GT_U32  txPacketSize[2];
            GT_U32  txPackets ;
            GT_U32  rxPacketSize;

            if(testingDsaBypassBridge == GT_TRUE)/* no flooding */
            {
                txPackets = 2; /*from_CPU+previous port send traffic to me*/
            }
            else
            {
                txPackets = portsCount; /*from_CPU + all other ports that send traffic to me*/
            }

            /* the packet that sent from the CPU to the port (when was looped back) */
            txPacketSize[0] = calculatedPacketLenIncludeCrc;

            /* the packet that sent from previous port(s) egress untagged */
            txPacketSize[1] = PRV_TGF_UNTAGGED_PACKET_LEN_CNS;
            if(txPacketSize[1] < 0x40)
            {
                txPacketSize[1] = 0x40;/* padding of the device */
            }

            txPacketSize[1] *= (txPackets - 1);

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((txPacketSize[0]+txPacketSize[1]) * prvTgfBurstCount,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(txPackets * prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(txPackets * prvTgfBurstCount, portCntrs.brdcPktsSent.l[0],
                                         "get another brdcPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                         "get another ucPktsSent counter than expected");

            rxPacketSize = txPacketSize[0];/* only the packets that looped back */

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(rxPacketSize * prvTgfBurstCount,
                                         portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.brdcPktsRcv.l[0],
                                         "get another brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                         "get another ucPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * (portsCount - 1) + PRV_TGF_PACKET_CRC_LEN_CNS) * prvTgfBurstCount,
                                     portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(portsCount * prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(portsCount * prvTgfBurstCount, portCntrs.brdcPktsSent.l[0],
                                     "get another brdcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                     "get another ucPktsSent counter than expected");

        /* check Rx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS) * prvTgfBurstCount,
                                     portCntrs.goodOctetsRcv.l[0],
                                     "get another goodOctetsRcv counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                     "get another goodPktsRcv counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.brdcPktsRcv.l[0],
                                     "get another brdcPktsRcv counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                     "get another ucPktsRcv counter than expected");
    }


    /* AUTODOC: 2. Check FDB entries phase - read dynamic FDB entries and verify MACs, VLANs, ports */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    for (fdbIter = 0; fdbIter < fdbCount; fdbIter++)
    {
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[fdbIter], sizeof(TGF_MAC_ADDR));
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* AUTODOC: get each FDB entry */
        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

        /* compare entryType */
        rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d", macEntry.key.entryType);

        /* AUTODOC: compare MAC address to packet's MAC SA */
        rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                          macEntryKey.key.macVlan.macAddr.arEther,
                          sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "MAC address[0]: %d",
                                     macEntry.key.key.macVlan.macAddr.arEther[0]);

        /* AUTODOC: verify vlanId */
        rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d", macEntryKey.key.macVlan.vlanId);

        /* compare dstInterface.type */
        rc = (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.type: %d", macEntry.dstInterface.type);

        /* AUTODOC: verify dstInterface */
        if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) {
            if(testingDsaTag == GT_TRUE)
            {
                GT_U32  expectedLearnPortNum;
                rc = (macEntry.dstInterface.devPort.hwDevNum == SRC_DSA_DEV_NUM(prvTgfDevNum)) ? GT_OK : GT_FAIL;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.devPort.hwDevNum: %d",
                                             macEntry.dstInterface.devPort.hwDevNum);

                expectedLearnPortNum = expectedFdbPort[fdbIter];
                if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E &&
                   (0 == PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->hwDevNum)) &&
                   (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)))
                {
                    expectedLearnPortNum &= 0x3f;
                }
                else
                if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
                {
                    /* supports for port > 128 */
                    expectedLearnPortNum &= 0x7f;/* 7 bits in the DSA */
                }

                UTF_VERIFY_EQUAL2_STRING_MAC(expectedLearnPortNum,macEntry.dstInterface.devPort.portNum,
                            "expected fdb port [%d] received [%d]",
                            expectedLearnPortNum,macEntry.dstInterface.devPort.portNum);
            }
            else
            {
                rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.devPort.hwDevNum: %d",
                                             macEntry.dstInterface.devPort.hwDevNum);

                rc = (macEntry.dstInterface.devPort.portNum == prvTgfPortsArray[fdbIter]) ? GT_OK : GT_FAIL;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.devPort.portNum: %d",
                                             macEntry.dstInterface.devPort.portNum);
            }
        }
    }


    /* AUTODOC: 3. UC phase - send UC packets from each port to all ports */
    PRV_UTF_LOG0_MAC("======= Sending unicast packets to check FDB table =======\n");

    /* AUTODOC: send known UC packet from each port with learned MACs */
    for (ucPortIter = 0; ucPortIter < portsCount; ucPortIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        PRV_UTF_LOG1_MAC("------- Sending packets from port %d -------\n", prvTgfPortsArray[ucPortIter]);
        for (ucPortToIter = 0; ucPortToIter < portsCount - 1; ucPortToIter++)
        {
            /* set destination MAC address in Packet */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[ucPortIter][ucPortToIter],
                         sizeof(prvTgfPacketL2Part.daMac));

            /* set source MAC address in Packet */
            cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[ucPortIter],
                         sizeof(prvTgfPacketL2Part.saMac));


            /* set the DSA info to be compatible */
            if(testingDsaTag == GT_TRUE)
            {
                /* set the target port of the packet in the DSA tag info
                    NOTE: when the mode is 'NOT bypass bridge' this info will NOT be used (so do DA lookup ... flooding),
                          but when the mode is 'bypass bridge' this info will be used

                          see testingDsaBypassBridge
                */
                /* when testingDsaBypassBridge = GT_TRUE -->
                   the destination port is according to the DA mac address */
                targetPortIndex = (prvTgfPacketL2Part.daMac[5] - 1) % portsCount;
                prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum = prvTgfPortsArray[targetPortIndex];

                if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
                {
                    if(testingDsaBypassBridge == GT_TRUE)
                    {
                        prvTgfPacketDsaTagPart_forward.dstEport = TARGET_EPORT_NUM_CNS + targetPortIndex;
                    }
                    else
                    {
                        prvTgfPacketDsaTagPart_forward.dstEport = DUMMY_TARGET_EPORT_NUM_CNS;
                    }
                }

                /* set the info for the 'source learning' */
                prvTgfPacketDsaTagPart_forward.source.portNum = expectedFdbPort[ucPortIter];/* use the 'similar' port numbers as on 'local' device */

                /* do this right before the send of packet */
                /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
                prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

                rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[ucPortIter],
                                prvTgfPortsArray[ucPortIter],
                                GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            /* send Packet */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ucPortIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[ucPortIter]);

            if(testingDsaTag == GT_TRUE)
            {
                rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[ucPortIter],
                                prvTgfPortsArray[ucPortIter],
                                GT_FALSE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                if(captureByTtiPortIndex == ucPortIter && testingDsaBypassBridge == GT_FALSE)
                {
                    /* check captured traffic for the sending port . */
                    checkCaptureIngressCopy(captureByTtiPortIndex, prvTgfBurstCount);
                }
            }
        }


        /* AUTODOC: read counters and verify traffic forwarded according to dynamic FDB entries */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            GT_U32  txPacketSize;
            GT_U32  txPackets;
            GT_U32  rxPacketSize;
            GT_U32  rxPackets ;

            if (ucPortIter == portIter)/* this port send UC traffic to all the neighbors */
            {
                txPackets = (portsCount - 1);/* from_cpu (from me) to all the other ports */
                /* the packet that sent from the CPU to the port (when was looped back) */
                txPacketSize = calculatedPacketLenIncludeCrc;
                rxPackets = txPackets;/* port is looped back*/
                rxPacketSize = txPacketSize; /* port is looped back*/

            }
            else /* this port get UC traffic from the current sender */
            {
                txPackets = 1;/* each port will get 1 packet (from me)*/

                /* the packet that sent from previous port(s) egress untagged */
                txPacketSize = PRV_TGF_UNTAGGED_PACKET_LEN_CNS;

                rxPackets = 0;
                rxPacketSize = 0;
            }

            if(txPacketSize < 0x40)
            {
                txPacketSize = 0x40;/* padding of the device */
            }

            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((rxPacketSize) * (portsCount - 1) * prvTgfBurstCount,
                                         portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((rxPackets) * prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((rxPackets) * prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                         "get another ucPktsRcv counter than expected");

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((txPacketSize) * (txPackets) * prvTgfBurstCount,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((txPackets) * prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((txPackets) * prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                         "get another ucPktsSent counter than expected");

            /* check zero counters for unused traffic type */
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.brdcPktsSent.l[0],
                                         "get another brdcPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.mcPktsSent.l[0],
                                         "get another mcPktsSent counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.brdcPktsRcv.l[0],
                                         "get another brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.mcPktsRcv.l[0],
                                         "get another mcPktsRcv counter than expected");
        }

    }
}

/**
* @internal prvTgfBrgBdlConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBdlConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    if(prvTgfResetModeGet() == GT_TRUE)
    {
        /* skip CPSS/HW restoration specific for the test and run full system reset and initialization */
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_CNS);

        if(testingDsaTag == GT_TRUE)
        {
            /* restore tunnel configurations */
            tunnelRestore(captureByTtiPortIndex);

            /* restore e2Phy configurations */
            restoreE2Phy();

            /* restore tested ports */
            prvUtfRestoreOrigTestedPorts();
        }
    }

    /* restore L2 packet's part */
    cpssOsMemCpy(&prvTgfPacketL2Part, &prvTgfDefPacketL2Part, sizeof(prvTgfPacketL2Part));

    /* restore DSA tag testing mode */
    prvTgfBrgBdlConfigurationDsaTagEnableSet(GT_FALSE,GT_FALSE,TGF_DSA_2_WORD_TYPE_E);
}

/**
* @internal prvTgfFdbLearning128KAddressesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbLearning128KAddressesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U32                        portIter = 0;

    TGF_VFD_INFO_STC              eVfdArray[1]  = {{0}};
    GT_U32                        portsCount   = prvTgfPortsNum;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

#ifdef ASIC_SIMULATION
    prvTgfBurstCount = 20;
#else
    prvTgfBurstCount = 131072;
#endif

    prvTgfPacketL2Part.saMac[5] = 1;

    /* AUTODOC: prepare ETH VFD array entry for SaMac */
    eVfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
    eVfdArray[0].modeExtraInfo  = 1;
    eVfdArray[0].incValue       = 20;
    eVfdArray[0].offset         = 11;                /* SaMac offset */
    eVfdArray[0].cycleCount = 1;
    cpssOsMemCpy(eVfdArray[0].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount,
                             sizeof(eVfdArray)/sizeof(eVfdArray[0]), eVfdArray);


    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                 prvTgfDevNum, prvTgfBurstCount);

    /* AUTODOC: set Tx delay */
    prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY_CNS);

    /* AUTODOC: send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: remove Tx delay */
    prvTgfSetTxSetup2Eth(0, 0);


    /* AUTODOC: check counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* AUTODOC: check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    prvTgfPacketL2Part.daMac[0] = 0;
    prvTgfPacketL2Part.daMac[1] = 0;
    prvTgfPacketL2Part.daMac[2] = 0;
    prvTgfPacketL2Part.daMac[3] = 0;
    prvTgfPacketL2Part.daMac[4] = 0;
    prvTgfPacketL2Part.daMac[5] = 1;


    /* AUTODOC: prepare ETH VFD array entry for DaMac */
    eVfdArray[0].offset         = 5;                /* DaMac offset */
    eVfdArray[0].cycleCount = 1;
    cpssOsMemCpy(eVfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount,
                             sizeof(eVfdArray)/sizeof(eVfdArray[0]), eVfdArray);


    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                 prvTgfDevNum, prvTgfBurstCount);

    /* AUTODOC: set Tx delay */
    prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY_CNS);

    /* AUTODOC: send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: remove Tx delay */
    prvTgfSetTxSetup2Eth(0, 0);


    /* AUTODOC: check counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {

            /* AUTODOC: check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* AUTODOC: Check UC */
        if(PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount - 1, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
        }
    }

}

/**
* @internal dsaTagEnableSet function
* @endinternal
*
* @brief   1. enable/disable running the test in DSA tag mode.
*         2. set bridge bypass mode
* @param[in] enable                   - enable/disable running the test in DSA tag mode.
* @param[in] dsaBypassBridge          - relevant when enable = TRUE
*                                      indicate that the traffic with 'forward' DSA tag do or not
*                                      'bypass' to the bridge.
*                                       None
*/
static void dsaTagEnableSet
(
    IN GT_BOOL                  enable,
    IN GT_BOOL                  dsaBypassBridge
)
{
    GT_HW_DEV_NUM  srcDsaHwDevNum     = SRC_DSA_DEV_NUM(prvTgfDevNum);
    GT_U32         portIter;
    static GT_U32  devTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    GT_BOOL     dualDeviceId = GT_FALSE;
    GT_STATUS   rc;
    GT_U32      expectedFdbPort;/* expected port */

    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    if(enable == GT_TRUE)
    {
        /* set DSA tag info related values */

        /* bind the DSA tag info */
        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart;
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

        prvTgfPacketDsaTagPart_forward.srcHwDev = srcDsaHwDevNum;

        /* allow traffic to be in the FDB that is associated with the 'remote device' */
        rc = prvTgfBrgFdbDeviceTableGet(&origDevTableBmp[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        devTableBmp[0] = origDevTableBmp[0] | (1<<srcDsaHwDevNum);
        rc = prvTgfBrgFdbDeviceTableSet(&devTableBmp[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],dsaBypassBridge);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if((prvTgfPortsArray[portIter] >= 64) && (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) == GT_FALSE))
            {
                dualDeviceId = GT_TRUE;
            }
        }

        if(0 == PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->hwDevNum))
        {
            dualDeviceId = GT_FALSE;
        }

        if(dualDeviceId == GT_TRUE &&
           sysGenGlobalInfo.supportDualDeviceId == GT_TRUE)
        {
            /* allow the src port to be >= 64 */
            rc = prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum,CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* set the device map table to know where to send traffic destined to SRC_DSA_DEV_NUM(prvTgfDevNum)

           DO it only after call to : prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum)
        */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            cascadeLink.linkNum = prvTgfPortsArray[portIter];


            if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
            {
                /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
                expectedFdbPort = prvTgfPortsArray[portIter + prvTgfPortsNum];/* take valid port beyond the tested ports */
            }
            else
            {
                /* set the info for the 'source learning' */
                expectedFdbPort = (prvTgfPortsArray[portIter] + 1);/* use the 'similar' port numbers as on 'local' device */
            }

            rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                     srcDsaHwDevNum,
                                     expectedFdbPort,
                                     &cascadeLink,
                                     GT_FALSE,
                                     GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }


    }
    else    /*when restore values make sure to do all restore without rc checking !*/
    {
        /* restore values */
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;

        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = NULL;

        prvTgfBrgFdbDeviceTableSet(&origDevTableBmp[0]);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],GT_TRUE);
            tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],GT_FALSE);
            /* set the device map table to know where to send traffic destined to SRC_DSA_DEV_NUM(prvTgfDevNum) */
            cascadeLink.linkNum = CPSS_NULL_PORT_NUM_CNS;

            if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
            {
                /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
                expectedFdbPort = prvTgfPortsArray[portIter + prvTgfPortsNum];/* take valid port beyond the tested ports */
            }
            else
            {
                /* set the info for the 'source learning' */
                expectedFdbPort = (prvTgfPortsArray[portIter] + 1);/* use the 'similar' port numbers as on 'local' device */
            }

            prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                     srcDsaHwDevNum,
                                     expectedFdbPort,
                                     &cascadeLink,
                                     GT_FALSE,
                                     GT_FALSE);
        }

        prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum,CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
        prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
    }
}

/**
* @internal prvTgfBrgBdlConfigurationDsaTagEnableSet function
* @endinternal
*
* @brief   1. enable/disable running the test in DSA tag mode.
*         2. set bridge bypass mode
*         3. set DSA tag type
* @param[in] dsaTagEnable             - enable/disable running the test in DSA tag mode.
* @param[in] dsaBypassBridge          - relevant when dsaTagEnable = TRUE
*                                      indicate that the traffic with 'forward' DSA tag do or not
*                                      'bypass' to the bridge.
* @param[in] dsaType                  - DSA type.
*
* @retval GT_TRUE                  - test can start - parameters are valid
* @retval GT_FALSE                 - test can't start - parameters are NOT valid
*/
GT_BOOL prvTgfBrgBdlConfigurationDsaTagEnableSet
(
    IN GT_BOOL                  dsaTagEnable,
    IN GT_BOOL                  dsaBypassBridge,
    IN TGF_DSA_TYPE_ENT         dsaType
)
{
#ifdef CHX_FAMILY
    if(((dsaType !=TGF_DSA_2_WORD_TYPE_E) && (dsaType != TGF_DSA_4_WORD_TYPE_E)) ||
        ((0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) &&
          (dsaType == TGF_DSA_4_WORD_TYPE_E)))
    {
        /*  the dsaType != 2/4 words ,or
            dsaType == 4 words and the device not support it  */
        return GT_FALSE;
    }
#endif/*CHX_FAMILY*/

    utfGeneralStateMessageSave(0,"dsaTagEnable = %d",dsaTagEnable);
    utfGeneralStateMessageSave(1,"dsaBypassBridge = %d",dsaBypassBridge);
    utfGeneralStateMessageSave(2,"dsaType = %s",
        dsaType == TGF_DSA_1_WORD_TYPE_E ? "TGF_DSA_1_WORD_TYPE_E" :
        dsaType == TGF_DSA_2_WORD_TYPE_E ? "TGF_DSA_2_WORD_TYPE_E" :
        dsaType == TGF_DSA_3_WORD_TYPE_E ? "TGF_DSA_3_WORD_TYPE_E" :
        dsaType == TGF_DSA_4_WORD_TYPE_E ? "TGF_DSA_4_WORD_TYPE_E" :
        "unknown");

    testingDsaBypassBridge = dsaBypassBridge;
    prvTgfPacketDsaTagPart.dsaType = dsaType;

    if(testingDsaTag == dsaTagEnable)
    {
        /* no 'restore' / 'new config' needed */
        return GT_TRUE;
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* some ports in DSA tagging may not be OK when reduced bits in the DSA format.
            the next function will remove those ports from the test.

            will be restored by calling to prvUtfRestoreOrigTestedPorts()
        */
        prvTgfFilterDsaSpecialPortsArray(
            prvTgfPacketDsaTagPart.dsaType,
            prvTgfPortsArray,
            8);/* the test is using up to 8 ports */
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) && !MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        if((!dsaTagEnable) || (dsaType == TGF_DSA_4_WORD_TYPE_E && dsaBypassBridge == GT_TRUE))
        {
            /* restore port configuration according to device family */
            prvTgfPortsArrayByDevTypeSet();
        }
        else
        {
            GT_BOOL     holdAmbiguously = GT_FALSE;
            GT_U32      ii,jj;
            GT_STATUS   rc;
            GT_U32       portsArray[4];

            portsArray[0] = prvTgfPortsArray[0];
            portsArray[1] = prvTgfPortsArray[1];
            portsArray[2] = prvTgfPortsArray[2];
            portsArray[3] = prvTgfPortsArray[3];

            /* check if any of those 4 ports hold diff of 64*ii from any other port
                meaning that any prvTgfPortsArray[x]+64*ii == prvTgfPortsArray[y]

               it will cause E2PHY to send traffic to 'highest port'

               so we need to add (+1) until no such cases

               also needed for 'eDSA' when dsaBypassBridge == GT_FALSE because
               we use the device map table in 6bits port num mode !!!
            */

            startResolveAmbiguously_lbl:

            for(ii = 0 ; ii < 4 ; ii++)
            {
                for(jj = 0 ; jj < 4 ; jj++)
                {
                    if(ii == jj)
                    {
                        continue;
                    }

                    if((portsArray[ii] & 0x3f) == (portsArray[jj] & 0x3f))
                    {
                        /* will cause ambiguous in 'trgPort' on the TGF_DSA_2_WORD_TYPE_E */
                        /* need to solve it */
                        portsArray[ii] = (portsArray[ii] + 1) %
                            UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(prvTgfDevNum);

                        holdAmbiguously = GT_TRUE;

                        goto startResolveAmbiguously_lbl;
                    }
                }
            }

            if(holdAmbiguously == GT_TRUE)
            {
                rc = prvTgfDefPortsArraySet(portsArray, 4);
                if(rc != GT_OK)
                {
                    return GT_FALSE;
                }
            }
        }
    }

    testingDsaTag = dsaTagEnable;

    dsaTagEnableSet(dsaTagEnable,dsaBypassBridge);

    return GT_TRUE;
}

/**
* @internal cpssDxChFdbDump function
* @endinternal
*
* @brief   debug tool --- print FDB entries
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChFdbDump(IN GT_U8 devNum)
{
    GT_STATUS   rc;
    GT_U32  index = 0,associatedHwDevNum;
    GT_BOOL  isValid,isSkip,isNew;
    CPSS_MAC_ENTRY_EXT_STC  entry;

    if(prvUtfIsGmCompilation())
    {
        cpssOsPrintf("ERROR : can't iterate over GM FDB due to poor performance \n");
        return GT_NOT_IMPLEMENTED;
    }


/*  Example:
index | mac addr        |vlan|intface|A   R  MS  |dc|sc|ud|dQi|sQi|dSl|sSl
                         (Hex)         S   M   MD                    v1|sId
                                        SP  C                       (Hex)
000000|00:00:00:00:00:00|0000|t512d00|A----------|0|0|007|0|0|0|0|000|256
000008|00:00:00:00:00:02|0002|p020d04|A-SP-------|0|0|019|0|0|0|0|000|013
049160|00:00:00:00:00:04|0002|p083d04|A----------|0|0|018|0|0|0|0|000|013
065544|00:00:00:00:00:03|0002|p059d04|A-SP-------|0|0|026|0|0|0|0|000|013
098312|00:00:00:00:00:01|0002|p001d04|A----------|0|0|000|0|0|0|0|000|013
*/
    cpssOsPrintf(
"index | mac addr        |vlan|intface|A   R  MS  |dc|sc|ud|dQi|sQi|dSl|sSl \n"
"                         (Hex)         S   M   MD                    v1|sId\n"
"                                        SP  C                       (Hex)  \n"
    );


    do
    {
        rc = cpssDxChBrgFdbMacEntryStatusGet(
            devNum,index,&isValid,&isSkip);
        if (rc != GT_OK)
        {
            /* break on invalid index */
            break;
        }
        if(isValid == GT_FALSE || isSkip == GT_TRUE)
        {
            index++;
            continue;
        }

        rc = cpssDxChBrgFdbMacEntryRead(
            devNum,index,&isValid,&isSkip,&isNew,&associatedHwDevNum,
            &entry);
        index++;
        if(rc != GT_OK)
        {
            continue;
        }
        cpssOsPrintf("%6.6d",index-1);

        if(entry.key.entryType != CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
        {
            cpssOsPrintf("entry.key.entryType[%d] not implemented \n",entry.key.entryType);
            continue;
        }
        cpssOsPrintf("|");
        cpssOsPrintf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
            entry.key.key.macVlan.macAddr.arEther[0],
            entry.key.key.macVlan.macAddr.arEther[1],
            entry.key.key.macVlan.macAddr.arEther[2],
            entry.key.key.macVlan.macAddr.arEther[3],
            entry.key.key.macVlan.macAddr.arEther[4],
            entry.key.key.macVlan.macAddr.arEther[5]);
        cpssOsPrintf("|");
        cpssOsPrintf("%4.4x",entry.key.key.macVlan.vlanId);
        cpssOsPrintf("|");

        if(entry.dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            cpssOsPrintf("p%3.3d",entry.dstInterface.devPort.portNum);
            cpssOsPrintf("d%2.2d",entry.dstInterface.devPort.hwDevNum);
        }
        else
        if(entry.dstInterface.type == CPSS_INTERFACE_TRUNK_E)
        {
            cpssOsPrintf("t%3.3d",entry.dstInterface.trunkId);
            cpssOsPrintf("d%2.2d",associatedHwDevNum);
        }
        else
        if(entry.dstInterface.type == CPSS_INTERFACE_VIDX_E)
        {
            cpssOsPrintf("vx%3.3d",entry.dstInterface.vidx);
            cpssOsPrintf("d%2.2d",associatedHwDevNum);
        }
        else
        if(entry.dstInterface.type == CPSS_INTERFACE_VID_E)
        {
            cpssOsPrintf("vl   ");
            cpssOsPrintf("d%2.2d",associatedHwDevNum);
        }
        cpssOsPrintf("|");

        cpssOsPrintf("%s",isNew ? "A" : "-");
        cpssOsPrintf("%s",entry.isStatic ? "S" : "-");
        cpssOsPrintf("%s",entry.spUnknown ? "SP":"--");
        cpssOsPrintf("%s",entry.daRoute ? "R" : "-");
        cpssOsPrintf("%s",entry.mirrorToRxAnalyzerPortEn ? "M" : "-");
        cpssOsPrintf("%s",entry.appSpecificCpuCode ? "C":"-");
        cpssOsPrintf("%s",entry.saMirrorToRxAnalyzerPortEn ? "MS":"--");
        cpssOsPrintf("%s",entry.daMirrorToRxAnalyzerPortEn ? "MD":"--");
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.daCommand);
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.saCommand);
        cpssOsPrintf("|");
        cpssOsPrintf("%3.3d",entry.userDefined);
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.daQosIndex);
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.saQosIndex);
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.daSecurityLevel);
        cpssOsPrintf("|");
        cpssOsPrintf("%d",entry.saSecurityLevel);
        cpssOsPrintf("|");
        cpssOsPrintf("%3.3x",entry.key.vid1);
        cpssOsPrintf("|");
        cpssOsPrintf("%3.3d",entry.sourceID);

        cpssOsPrintf("\n");
    }while(1);

    cpssOsPrintf("Finished FDB scan of [%d K] entries \n",(index/_1K));

    return GT_OK;
}

