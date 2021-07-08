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
* @file prvTgfTailDrop.c
*
* @brief Enhanced UTs for CPSS Tail Drop
*
* @version   14
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfPclGen.h>
#include <tailDrop/prvTgfTailDrop.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           24

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS             4

/* number of ports destination */
#define PRV_TGF_DST_COUNT_CNS              3

/* number of FDB entries */
#define PRV_TGF_FDB_COUNT_CNS              8

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS          2
#define PRV_TGF_MC1_SEND_PORT_IDX_CNS      0
#define PRV_TGF_MC2_SEND_PORT_IDX_CNS      3

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS       1
#define PRV_TGF_MC_RECEIVE_PORT_IDX_CNS    2

/* port numbers to receive traffic from */
#define PRV_TGF_FDB_0_PORT_IDX_CNS         0
#define PRV_TGF_FDB_1_PORT_IDX_CNS         1
#define PRV_TGF_FDB_2_PORT_IDX_CNS         3

/* trafficClasses */
#define PRV_TGF_TC_0_CNS                   0
#define PRV_TGF_TC_4_CNS                   4
#define PRV_TGF_TC_7_CNS                   7

/* maximal buffers limit for Mcast */
static GT_U32        prvTgfMcBuffLimit = 600;
static GT_U32        prvTgfMcBuffLimitCurrent;

/* maximal descriptors limit for Mcast */
static GT_U32        prvTgfMcDescLimit = 650;
static GT_U32        prvTgfMcDescLimitCurrent;

/* maximal descriptors and buffers limit for shared pool */
static GT_U32        prvTgfSharedLimit = 500;

/* maximal buffers limit for shared pools */
static GT_U32        prvTgfBufLimitArr[PRV_TGF_FDB_COUNT_CNS] =
{
    200, 130, 500, 500, 701, 900, 313, 601
};

/* maximal descriptors limit for shared pools */
static GT_U32        prvTgfDescLimitArr[PRV_TGF_FDB_COUNT_CNS] =
{
    115, 200, 300, 427, 900, 511, 400, 1000
};

/* shared pool number */
static GT_U32        prvTgfPoolNum = 2;

/* FDB Table Configuration */
static TGF_MAC_ADDR  prvTgfFdbArr[PRV_TGF_FDB_COUNT_CNS] =
{
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x04},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x05},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x06},
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x07}
};

/* receive ports */
static GT_U8         prvTgfPortIdxArr[PRV_TGF_DST_COUNT_CNS] =
{
    PRV_TGF_FDB_0_PORT_IDX_CNS,
    PRV_TGF_FDB_1_PORT_IDX_CNS,
    PRV_TGF_FDB_2_PORT_IDX_CNS
};

/* the number of buffers that can be allocated to each part */
static GT_U16        prvTgfDpMaxBuffArr[PRV_TGF_DST_COUNT_CNS] =
{
    1000, 500, 200
};

/* maximal number of buffers for a port */
static GT_U16        prvTgfPortMaxBuffLimit = 1900;
static GT_U32        prvTgfPortMaxBuffLimitCurrent;
static GT_U32        prvTgfPortMaxDescLimitCurrent;

static GT_U32        prvTgfPortUcMaxBuffLimit = 10;

static GT_U32        prvTgfPortMcMaxBuffLimit = 10;
static PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS save_tailDropProfileParams;

/* parameters that can be changed from test to test */
static struct
{
    GT_U16                            maxBuffArr[PRV_TGF_DST_COUNT_CNS];
    GT_U16                            maxBuffLimit;
    GT_U8                             tc;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet;
    CPSS_DP_LEVEL_ENT                 dpLevel;
} prvTgfConfig[PRV_TGF_DROP_CONFIG_SET_LAST_E] =
{
    {   /* parameters for the test .tgfTailDropWrtdQueueDp */
        {1536,    0,    0},                 /* maxBuffArr */
        1536,                               /* maxBuffLimit */
        4,                                  /* tc */
        CPSS_PORT_TX_DROP_PROFILE_1_E,      /* profileSet */
        CPSS_DP_GREEN_E                     /* dpLevel */
    },
    {   /* parameters for the test .tgfTailDropWrtdPort */
        {1900, 1750, 1000},                 /* maxBuffArr */
        256,                                /* maxBuffLimit */
        7,                                  /* tc */
        CPSS_PORT_TX_DROP_PROFILE_5_E,      /* profileSet */
        CPSS_DP_YELLOW_E                    /* dpLevel */
    },
    {   /* parameters for the test .tgfTailDropWrtdQueueDp for BC3. 
           The DP0 does not work in BC3 for WRTD (see FE-1298799). Use DP1. */
        {0,    1536,    0},                 /* maxBuffArr */
        1536,                               /* maxBuffLimit */
        7,                                  /* tc */
        CPSS_PORT_TX_DROP_PROFILE_1_E,      /* profileSet */
        CPSS_DP_YELLOW_E                    /* dpLevel */
    }
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x18}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    6, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* Number of virtual buffers enqueued and descriptors allocated */
typedef struct
{
    GT_U32  globalDesc;
    GT_U32  sharedDesc;
    GT_U32  sniffedRxDesc;
    GT_U32  sniffedTxDesc;
    GT_U32  mcastDesc;
    GT_U32  globalBuff;
    GT_U32  sharedBuff;
    GT_U32  sniffedRxBuff;
    GT_U32  sniffedTxBuff;
    GT_U32  mcastBuff;
} PRV_TGF_PORT_TX_COUNTER_SET_STC;

/* log out values of cntrName Tx Queue counter and expected one if error */
#define PRV_UTF_TX_COUNTERS_LOG_MAC(getCntr, expCntr, cntrName)               \
    do {                                                                      \
        PRV_UTF_LOG3_MAC("\n%-9s [Buf,Desc]: [%d,%d]",                        \
                #cntrName, getCntr.cntrName##Buff, getCntr.cntrName##Desc);   \
                                                                              \
        if ((getCntr.cntrName##Buff != expCntr.cntrName##Buff)                \
        ||  (getCntr.cntrName##Desc != expCntr.cntrName##Desc))               \
        {                                                                     \
            PRV_UTF_LOG2_MAC(", expected: [%d,%d]",                           \
                expCntr.cntrName##Buff, expCntr.cntrName##Desc);              \
        }                                                                     \
    }                                                                         \
    while (0)

/* compare values of Tx Queue counters and expected ones */
#define PRV_UTF_TX_COUNTERS_CMP_MAC(getCntr, expCntr)                     \
    (((getCntr.globalBuff    != expCntr.globalBuff)    ||                 \
      (getCntr.globalDesc    != expCntr.globalDesc)    ||                 \
      (getCntr.sharedBuff    != expCntr.sharedBuff)    ||                 \
      (getCntr.sharedDesc    != expCntr.sharedDesc)    ||                 \
      (getCntr.sniffedRxBuff != expCntr.sniffedRxBuff) ||                 \
      (getCntr.sniffedTxBuff != expCntr.sniffedTxBuff) ||                 \
      (getCntr.sniffedRxDesc != expCntr.sniffedRxDesc) ||                 \
      (getCntr.sniffedTxDesc != expCntr.sniffedTxDesc) ||                 \
      (getCntr.sharedBuff    != expCntr.sharedBuff)    ||                 \
      (getCntr.mcastDesc     != expCntr.mcastDesc)) ? GT_FAIL : GT_OK)

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfTailDropTxCountersGet function
* @endinternal
*
* @brief   Get all Tx buffers and descriptors
*
* @param[in] poolNum                  - shared pool number, range 0..7
*
* @param[out] getCntrPtr               - (pointer to) port tx counters set
*                                       None
*/
static GT_VOID prvTgfTailDropTxCountersGet
(
    IN  GT_U32                          poolNum,
    OUT PRV_TGF_PORT_TX_COUNTER_SET_STC *getCntrPtr
)
{
    GT_STATUS rc;

    /* get Global Buffers */
    rc = prvTgfPortTxGlobalBufNumberGet(prvTgfDevNum, &getCntrPtr->globalBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalBufNumberGet");

    /* get Global Descriptors */
    rc = prvTgfPortTxGlobalDescNumberGet(prvTgfDevNum, &getCntrPtr->globalDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalDescNumberGet");

    /* get SharedResource Buffers */
    rc = prvTgfPortTxSharedResourceBufNumberGet(
            prvTgfDevNum, poolNum, &getCntrPtr->sharedBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxSharedResourceBufNumberGet");

    /* get SharedResource Descriptors */
    rc = prvTgfPortTxSharedResourceDescNumberGet(
            prvTgfDevNum, poolNum, &getCntrPtr->sharedDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxSharedResourceDescNumberGet");

    /* get Sniffed Buffers */
    rc = prvTgfPortTxSniffedBufNumberGet(
            prvTgfDevNum, &getCntrPtr->sniffedRxBuff, &getCntrPtr->sniffedTxBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSniffedBufNumberGet");

    /* get Sniffed Descriptors */
    rc = prvTgfPortTxSniffedDescNumberGet(
            prvTgfDevNum, &getCntrPtr->sniffedRxDesc, &getCntrPtr->sniffedTxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSniffedDescNumberGet");

    /* get Mcast Buffers */
    rc = prvTgfPortTxMcastBufNumberGet(prvTgfDevNum, &getCntrPtr->mcastBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBufNumberGet");

    /* get Mcast Descriptors */
    rc = prvTgfPortTxMcastDescNumberGet(prvTgfDevNum, &getCntrPtr->mcastDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastDescNumberGet");
}

/******************************************************************************\
 *                            Public test functions                           *
\******************************************************************************/

/* save array for profile indexes */
static CPSS_PORT_TX_DROP_PROFILE_SET_ENT save_pfSetArr[PRV_TGF_PORT_COUNT_CNS];

/**
* @internal prvTgfTailDropConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*
* @param[in] isVariousPorts           - GT_TRUE  creates FDB entries with different MAC DA
*                                      GT_FALSE creates FDB entries with  the same MAC DA
* @param[in] numEntries               - number of QoS, FDB and PCL entries to create.
* @param[in] dropPrecedence           - drop precedence for cos profile.
*                                       None
*/
GT_VOID prvTgfTailDropConfigurationSet
(
    IN GT_BOOL           isVariousPorts,
    IN GT_U32            numEntries,
    IN CPSS_DP_LEVEL_ENT dropPrecedence
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIdx, prfIdx, pclRuleIdx;
    PRV_TGF_COS_PROFILE_STC cosProfile = {0};
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT patt;
    PRV_TGF_PCL_ACTION_STC      action;

    /* 0. Init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit");

    /* Set default mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdIpL2Qos.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(patt.ruleStdIpL2Qos.macDa.arEther, prvTgfFdbArr[0], sizeof(GT_ETHERADDR));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.profileAssignIndex = GT_TRUE;

    /* 1. Set VLAN on all ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
            PRV_TGF_SEND_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* 2. Set HOL system mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA3_E))
    {
        /* disable global resource sharing */
        rc = prvTgfPortTxSharingGlobalResourceEnableSet(prvTgfDevNum, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharingGlobalResourceEnableSet");

        /* disable FC */
        for (portIdx = 0; portIdx < PRV_TGF_PORT_COUNT_CNS; portIdx++)
        {
            rc = prvTgfPortFlowControlEnableSet(prvTgfDevNum,
                                                prvTgfPortsArray[portIdx],
                                                CPSS_PORT_FLOW_CONTROL_DISABLE_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfPortFlowControlEnableSet, port %d",
                                         prvTgfPortsArray[portIdx]);
        }
    }

    /* set number of QoS, FDB and PCL entries */
    prfIdx = (numEntries == PRV_TGF_DST_COUNT_CNS) ? numEntries
           : (numEntries == PRV_TGF_FDB_COUNT_CNS) ? numEntries
           : 1;

    /* set QoS, FDB and PCL entries */
    while (prfIdx--)
    {
        /* 3. Set QoS profile table */
        cosProfile.userPriority = prfIdx;
        cosProfile.dscp         = prfIdx;
        cosProfile.exp          = prfIdx;

        cosProfile.trafficClass =
              (numEntries == PRV_TGF_DST_COUNT_CNS) ? PRV_TGF_TC_7_CNS
            : (numEntries == PRV_TGF_FDB_COUNT_CNS) ? prfIdx
            : (dropPrecedence == CPSS_DP_GREEN_E)   ? PRV_TGF_TC_4_CNS
            : PRV_TGF_TC_7_CNS;

        cosProfile.dropPrecedence = (CPSS_DP_LEVEL_ENT)
             ((numEntries == PRV_TGF_DST_COUNT_CNS) ? prfIdx
            : (numEntries == PRV_TGF_FDB_COUNT_CNS) ? 0
            : dropPrecedence);

        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E))
        {
            /* set cos entry */
            rc = prvTgfCosProfileEntrySet((GT_U8)prfIdx, &cosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
        }

        PRV_UTF_LOG3_MAC(" cosProfile%d[tc,dp] = [%d,%d]\n",
            prfIdx, cosProfile.trafficClass, cosProfile.dropPrecedence);

        /* 4. Create FDB for all receive ports */
        if( (GT_TRUE == isVariousPorts) &&
            (prfIdx < PRV_TGF_DST_COUNT_CNS) )
        {
            portIdx = prvTgfPortIdxArr[prfIdx];
        }
        else
        {
            portIdx = PRV_TGF_FDB_1_PORT_IDX_CNS;
        }

        /* set FDB entry */
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(
                prvTgfFdbArr[prfIdx], PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
                prvTgfPortsArray[portIdx], GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

        /* 5. Set PCL configuration */
        patt.ruleStdIpL2Qos.macDa.arEther[5] = prvTgfFdbArr[prfIdx][5];

        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E))
        {
            /* DX devices - use QoS Profile */
            action.qos.profileIndex = prfIdx;
        }
        else
        {
            /* Puma devices - use explicit QoS parameters */
            action.qos.tc = cosProfile.trafficClass;
            action.qos.dp = cosProfile.dropPrecedence;
            action.qos.up = cosProfile.userPriority;
            action.qos.dscp = cosProfile.dscp;
            action.qos.exp = cosProfile.exp;
        }

        pclRuleIdx = prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0/*lookupId*/,prfIdx);
        /* set PCL rule */
        rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                pclRuleIdx, &mask, &patt, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet");
    }

    /* Store bind port to default used drop profile */
    for (prfIdx = 0; prfIdx < PRV_TGF_PORT_COUNT_CNS; prfIdx++)
    {
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                         prvTgfPortsArray[prfIdx],
                                         &save_pfSetArr[prfIdx]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet, port %d",
                                     prvTgfPortsArray[prfIdx]);
    }
}

/**
* @internal prvTgfTailDropProfileConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] isVariousPorts           - GT_TRUE  creates FDB entries with different MAC DA
*                                      GT_FALSE creates FDB entries with  the same MAC DA
*                                       None
*/
GT_VOID prvTgfTailDropProfileConfigurationSet
(
    IN GT_BOOL isVariousPorts
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     prfIdx = PRV_TGF_DST_COUNT_CNS;
    GT_U32    dpMaxBuff = 0;
    GT_U32    dpMaxDesc = 0;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSetArr[PRV_TGF_DST_COUNT_CNS] = {0};

    /* 6.1 Set Dp limits for tail drop profile 7 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            prvTgfPortMaxBuffLimit, prvTgfPortMaxBuffLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

    /* check correct setting */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            &dpMaxBuff, &dpMaxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxBuff,
            "got other dpMaxBuff than expexted");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxDesc,
            "got other dpMaxDesc than expexted");

    /* 6.2 Set Dp limits for traffic class 7 */
    cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
    tailDropProfileParams.dp0MaxBuffNum  = prvTgfDpMaxBuffArr[0];
    tailDropProfileParams.dp1MaxBuffNum  = prvTgfDpMaxBuffArr[1];
    tailDropProfileParams.dp2MaxBuffNum  = prvTgfDpMaxBuffArr[2];
    tailDropProfileParams.dp0MaxDescrNum = prvTgfDpMaxBuffArr[0];
    tailDropProfileParams.dp1MaxDescrNum = prvTgfDpMaxBuffArr[1];
    tailDropProfileParams.dp2MaxDescrNum = prvTgfDpMaxBuffArr[2];
    tailDropProfileParams.tcMaxBuffNum   = 0;
    tailDropProfileParams.tcMaxDescrNum  = 0;

    /* set Dp limits for tc */
    rc = prvTgfPortTx4TcTailDropProfileSet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            PRV_TGF_TC_7_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

    /* check correct setting */
    rc = prvTgfPortTx4TcTailDropProfileGet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            PRV_TGF_TC_7_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[0], tailDropProfileParams.dp0MaxBuffNum,
            "got other tailDropProfileParams.dp0MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[2], tailDropProfileParams.dp2MaxBuffNum,
            "got other tailDropProfileParams.dp2MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[0], tailDropProfileParams.dp0MaxDescrNum,
            "got other tailDropProfileParams.dp0MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[2], tailDropProfileParams.dp2MaxDescrNum,
            "got other tailDropProfileParams.dp2MaxDescrNum than expexted");

    if(GT_TRUE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA3_E))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[1], tailDropProfileParams.dp1MaxBuffNum,
            "got other tailDropProfileParams.dp1MaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[1], tailDropProfileParams.dp1MaxDescrNum,
            "got other tailDropProfileParams.dp1MaxDescrNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxBuffNum,
            "got other tailDropProfileParams.tcMaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxDescrNum,
            "got other tailDropProfileParams.tcMaxDescrNum than expexted");
    }

    /* 7. Bind all receive ports to drop profile */
    while (prfIdx--)
    {
        if (isVariousPorts || (PRV_TGF_RECEIVE_PORT_IDX_CNS == prvTgfPortIdxArr[prfIdx]))
        {
            /* bind port to dp */
            rc = prvTgfPortTxBindPortToDpSet(
                    prvTgfDevNum,
                    prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                    CPSS_PORT_TX_DROP_PROFILE_7_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

            /* check correct setting */
            rc = prvTgfPortTxBindPortToDpGet(
                    prvTgfDevNum,
                    prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                    &profileSetArr[prfIdx]);

            /* verifying values */
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PORT_TX_DROP_PROFILE_7_E, profileSetArr[prfIdx],
                    "got other PORT_TX_DROP_PROFILE than expexted");
        }
    }
}

/**
* @internal prvTgfTailDropWrtdConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] configSetIdx             - index of set of parameters for using in the test
*                                       None
*/
GT_VOID prvTgfTailDropWrtdConfigurationSet
(
    IN PRV_TGF_DROP_CONFIG_SET_ENT configSetIdx
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    dpMaxBuff = 0;
    GT_U32    dpMaxDesc = 0;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet = 0;
    CPSS_PORT_EGRESS_CNT_MODE_ENT     setModeBmp;

    /* 6.1 Set Dp limits for tail drop profile */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            prvTgfConfig[configSetIdx].maxBuffLimit,
            prvTgfConfig[configSetIdx].maxBuffLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

    /* check correct setting */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            &dpMaxBuff, &dpMaxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffLimit, dpMaxBuff,
            "got other dpMaxBuff than expexted");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffLimit, dpMaxDesc,
            "got other dpMaxDesc than expexted");


    /* 6.2 Set Dp limits for traffic class 7 */
    cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
    tailDropProfileParams.dp0MaxBuffNum  = prvTgfConfig[configSetIdx].maxBuffArr[0];
    tailDropProfileParams.dp1MaxBuffNum  = prvTgfConfig[configSetIdx].maxBuffArr[1];
    tailDropProfileParams.dp2MaxBuffNum  = prvTgfConfig[configSetIdx].maxBuffArr[2];
    tailDropProfileParams.dp0MaxDescrNum = prvTgfConfig[configSetIdx].maxBuffArr[0];
    tailDropProfileParams.dp1MaxDescrNum = prvTgfConfig[configSetIdx].maxBuffArr[1];
    tailDropProfileParams.dp2MaxDescrNum = prvTgfConfig[configSetIdx].maxBuffArr[2];
    tailDropProfileParams.tcMaxBuffNum   = 0;
    tailDropProfileParams.tcMaxDescrNum  = 0;

    /* set Dp limits for tc */
    rc = prvTgfPortTx4TcTailDropProfileSet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            prvTgfConfig[configSetIdx].tc,
            &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

    /* check correct setting */
    rc = prvTgfPortTx4TcTailDropProfileGet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            prvTgfConfig[configSetIdx].tc,
            &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[0],
                                 tailDropProfileParams.dp0MaxBuffNum,
            "got other tailDropProfileParams.dp0MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[1],
                                 tailDropProfileParams.dp1MaxBuffNum,
            "got other tailDropProfileParams.dp1MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[2],
                                 tailDropProfileParams.dp2MaxBuffNum,
            "got other tailDropProfileParams.dp2MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[0],
                                 tailDropProfileParams.dp0MaxDescrNum,
            "got other tailDropProfileParams.dp0MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[1],
                                 tailDropProfileParams.dp1MaxDescrNum,
            "got other tailDropProfileParams.dp1MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].maxBuffArr[2],
                                 tailDropProfileParams.dp2MaxDescrNum,
            "got other tailDropProfileParams.dp2MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxBuffNum,
            "got other tailDropProfileParams.tcMaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxDescrNum,
            "got other tailDropProfileParams.tcMaxDescrNum than expexted");

    /* 7. Bind all receive ports to drop profile */
    rc = prvTgfPortTxBindPortToDpSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfConfig[configSetIdx].profileSet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

    /* check correct setting */
    rc = prvTgfPortTxBindPortToDpGet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            &profileSet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfConfig[configSetIdx].profileSet, profileSet,
            "got other PORT_TX_DROP_PROFILE than expexted");

    /* 9. set egress counters mode */
    setModeBmp = CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E;

    /* set egress counter mode */
    rc = prvTgfPortEgressCntrModeSet(
            0,
            setModeBmp,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_SEND_VLANID_CNS,
            prvTgfConfig[configSetIdx].tc,
            prvTgfConfig[configSetIdx].dpLevel);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet");
}

/**
* @internal prvTgfTailDropSharingConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] enableMode               - Drop Precedence (DP) enable mode to use the shared pool
*                                       None
*/
GT_VOID prvTgfTailDropSharingConfigurationSet
(
    IN PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableMode
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableModeGet;
    GT_U32    poolNum;
    GT_U32    sharedBufLimit  = 0;
    GT_U32    sharedDescLimit = 0;

    /* 7.1 enabled shared pool */
    rc = prvTgfPortTxTcSharedProfileEnableSet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            PRV_TGF_TC_7_CNS,
            enableMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcSharedProfileEnableSet");

    /* check the result of setting */
    rc = prvTgfPortTxTcSharedProfileEnableGet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            PRV_TGF_TC_7_CNS,
            &enableModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcSharedProfileEnableGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(enableMode, enableModeGet,
            "got other enableMode than expexted");

    /* set limits and bind pool only one time on DP_MODE_DP0 setting stage */
    if (PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E == enableMode)
    {
        /* 7.2 set buffers and descriptors limits for pool 2 */
        rc = prvTgfPortTxSharedPoolLimitsSet(
                prvTgfDevNum,
                prvTgfPoolNum,
                prvTgfSharedLimit, prvTgfSharedLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharedPoolLimitsSet");

        /* check the result of setting */
        rc = prvTgfPortTxSharedPoolLimitsGet(
                prvTgfDevNum,
                prvTgfPoolNum,
                &sharedBufLimit, &sharedDescLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharedPoolLimitsGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfSharedLimit, sharedBufLimit,
                "got other sharedBufLimit than expexted");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfSharedLimit, sharedDescLimit,
                "got other sharedDescLimit than expexted");

        /* 7.3 bind shared pool 2 to profile 7, Tc 7 */
        rc = prvTgfPortTxTcProfileSharedPoolSet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                PRV_TGF_TC_7_CNS,
                prvTgfPoolNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcProfileSharedPoolSet");

        /* check the result of setting */
        rc = prvTgfPortTxTcProfileSharedPoolGet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                PRV_TGF_TC_7_CNS,
                &poolNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcProfileSharedPoolGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPoolNum, poolNum,
                "got other poolNum than expexted");
    }
}

/**
* @internal prvTgfTailDropSharedPoolsConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*/
GT_VOID prvTgfTailDropSharedPoolsConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     prfIdx = PRV_TGF_FDB_COUNT_CNS;
    PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableModeGet;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    GT_U32    poolNum;
    GT_U32    sharedBufLimit  = 0;
    GT_U32    sharedDescLimit = 0;
    GT_U32    dpMaxBuff = 0;
    GT_U32    dpMaxDesc = 0;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet;

    /* set Dp limits for drop profile 7 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            prvTgfPortMaxBuffLimit, prvTgfPortMaxBuffLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

    /* check the result of setting */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(
            prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_7_E,
            &dpMaxBuff, &dpMaxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxBuff,
            "got other dpMaxBuff than expexted");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxDesc,
            "got other dpMaxDesc than expexted");

    while (prfIdx--)
    {
        /* set Dp limits for tc0..7 with Dp0Maxlimit = 200 */
        cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
        tailDropProfileParams.dp0MaxBuffNum  = prvTgfDpMaxBuffArr[2];
        tailDropProfileParams.dp0MaxDescrNum = prvTgfDpMaxBuffArr[2];

        /* set Dp limits for tc */
        rc = prvTgfPortTx4TcTailDropProfileSet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx, &tailDropProfileParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

        /* check the result of setting */
        rc = prvTgfPortTx4TcTailDropProfileGet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx, &tailDropProfileParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[2], tailDropProfileParams.dp0MaxBuffNum,
                "got other tailDropProfileParams.dp0MaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxBuffNum,
                "got other tailDropProfileParams.dp1MaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxBuffNum,
                "got other tailDropProfileParams.dp2MaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDpMaxBuffArr[2], tailDropProfileParams.dp0MaxDescrNum,
                "got other tailDropProfileParams.dp0MaxDescrNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxDescrNum,
                "got other tailDropProfileParams.dp1MaxDescrNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxDescrNum,
                "got other tailDropProfileParams.dp2MaxDescrNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxBuffNum,
                "got other tailDropProfileParams.tcMaxBuffNum than expexted");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxDescrNum,
                "got other tailDropProfileParams.tcMaxDescrNum than expexted");

        /* set shared limits for pools 0..7 */
        rc = prvTgfPortTxSharedPoolLimitsSet(
                prvTgfDevNum,
                prfIdx,
                prvTgfBufLimitArr[prfIdx], prvTgfDescLimitArr[prfIdx]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharedPoolLimitsSet");

        /* check the result of setting */
        rc = prvTgfPortTxSharedPoolLimitsGet(
                prvTgfDevNum,
                prfIdx,
                &sharedBufLimit, &sharedDescLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharedPoolLimitsGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBufLimitArr[prfIdx], sharedBufLimit,
                "prvTgfPortTxSharedPoolLimitsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDescLimitArr[prfIdx], sharedDescLimit,
                "prvTgfPortTxSharedPoolLimitsGet");

        /* bind shared pools 0..7 to profile 7, tc0..7 */
        rc = prvTgfPortTxTcProfileSharedPoolSet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx,
                prfIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcProfileSharedPoolSet");

        /* check the result of setting */
        rc = prvTgfPortTxTcProfileSharedPoolGet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx,
                &poolNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcProfileSharedPoolGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prfIdx, poolNum,
                "prvTgfPortTxTcProfileSharedPoolGet");

        /* enable shared pools 0..7 */
        rc = prvTgfPortTxTcSharedProfileEnableSet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx,
                PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcSharedProfileEnableSet");

        /* check the result of setting */
        rc = prvTgfPortTxTcSharedProfileEnableGet(
                prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_7_E,
                prfIdx,
                &enableModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTcSharedProfileEnableGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E, enableModeGet,
                "prvTgfPortTxTcSharedProfileEnableGet");
    }

    /* Bind all receive port to drop profile */
    rc = prvTgfPortTxBindPortToDpSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            CPSS_PORT_TX_DROP_PROFILE_7_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

    /* check correct setting */
    rc = prvTgfPortTxBindPortToDpGet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            &profileSet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PORT_TX_DROP_PROFILE_7_E, profileSet,
            "got other PORT_TX_DROP_PROFILE than expexted");
}

/**
* @internal prvTgfTailDropVariousPortsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropVariousPortsTrafficGenerate
(
    IN GT_U32 burstCount
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     prfIdx;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* Block TxQ=7 for all receive ports */
    for (prfIdx = 0; prfIdx < PRV_TGF_DST_COUNT_CNS; prfIdx++)
    {
        rc = prvTgfPortTxQueueTxEnableSet(
                prvTgfDevNum,
                prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                PRV_TGF_TC_7_CNS,
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }

    /* send traffic */
    PRV_UTF_LOG0_MAC("\n");
    for (prfIdx = 0; prfIdx < PRV_TGF_DST_COUNT_CNS; prfIdx++)
    {
        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA3_E))
        {
            if( 1 == prfIdx )
            {
                continue;
            }
        }

        PRV_UTF_LOG2_MAC("=== Transmitting %d packets to check port [%d] ===\n",
                burstCount, prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]]);

        /* set DA of the packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac,
                     prvTgfFdbArr[prfIdx],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* Release TxQ=7 for all receive ports */
    for (prfIdx = 0; prfIdx < PRV_TGF_DST_COUNT_CNS; prfIdx++)
    {
        rc = prvTgfPortTxQueueTxEnableSet(
                prvTgfDevNum,
                prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                PRV_TGF_TC_7_CNS,
                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }

    cpssOsTimerWkAfter(10);

    /* check Eth counters */
    PRV_UTF_LOG0_MAC("\n");
    for (prfIdx = 0; prfIdx < PRV_TGF_DST_COUNT_CNS; prfIdx++)
    {
        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA3_E))
        {
            if( 1 == prfIdx )
            {
                continue;
            }
        }

        /* read Eth counters */
        rc = prvTgfReadPortCountersEth(
                prvTgfDevNum,
                prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                GT_FALSE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");

        /* print number of received packets */
        PRV_UTF_LOG2_MAC("Port [%d] received %d packets\n",
                prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]],
                portCntrs.goodPktsSent.l[0]);

        /* check Eth counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(
                prvTgfDpMaxBuffArr[prfIdx],
                portCntrs.goodPktsSent.l[0],
                "got other number of packets than expected on port [%d]",
                prvTgfPortsArray[prvTgfPortIdxArr[prfIdx]]);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTailDropOnePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] fdbIdxMin                - minimum for an iterator fdbIdx
*                                      FDB index fdbIdx must be decreased from
*                                      PRV_TGF_DST_COUNT_CNS down to fdbIdxMin because different
*                                      tests must check different quantity of destinations
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropOnePortTrafficGenerate
(
    IN GT_U32 fdbIdxMin,
    IN GT_U32 burstCount
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    fdbIdx = PRV_TGF_DST_COUNT_CNS;
    GT_U32    fdbIdxMinFix;
    GT_U32    numBufs, numDesc, numExpect = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* block TxQ=7 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_7_CNS,
            GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    /* zero minimum if it is more than max value */
    fdbIdxMinFix = (fdbIdxMin >= PRV_TGF_DST_COUNT_CNS) ? 0 : fdbIdxMin;

    /* send traffic */
    while (fdbIdx-- > fdbIdxMinFix)
    {
        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA3_E))
        {
            if( 1 == fdbIdx )
            {
                continue;
            }
        }

        PRV_UTF_LOG2_MAC("\n===> Transmitting %d packets to check Drop Profile %d ",
                burstCount, fdbIdx);

        /* expected values are taken from the array except the last iteration
         * according to the TDD each the last iteration has specific expected value
         */
        numExpect = ((fdbIdx > fdbIdxMin) || (fdbIdxMin >= PRV_TGF_DST_COUNT_CNS)) ?
                    (prvTgfDpMaxBuffArr[fdbIdx]) :
                    (prvTgfDpMaxBuffArr[0] + prvTgfSharedLimit + 1);


        /* set DA of the packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[fdbIdx],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* Check allocated buffers */
        rc = prvTgfPortTxGlobalBufNumberGet(prvTgfDevNum, &numBufs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalBufNumberGet");
        PRV_UTF_LOG1_MAC("\nNumber of Buffers = %d    ", numBufs);
        UTF_VERIFY_EQUAL0_STRING_MAC(numExpect, numBufs,
                "got other number of buffers than expected");

        /* Check allocated descriptors */
        rc = prvTgfPortTxGlobalDescNumberGet(prvTgfDevNum, &numDesc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalDescNumberGet");
        PRV_UTF_LOG1_MAC("Number of Descriptors = %d    ", numDesc);
        UTF_VERIFY_EQUAL0_STRING_MAC(numExpect, numDesc,
                "got other number of descriptors than expected");
    }

    /* release TxQ=7 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_7_CNS,
            GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    /* test sends 2000 68 Bytes packets. Port speed may be 100M for tri-speed ports.
       need wait 0.01408 seconds at least */
    cpssOsTimerWkAfter(20);

    /* read Eth counters */
    rc = prvTgfReadPortCountersEth(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            GT_FALSE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");

    /* print number of received packets */
    PRV_UTF_LOG2_MAC("\n===> Port [%d] received %d packets\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            portCntrs.goodPktsSent.l[0]);

    /* check Eth counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
            numExpect,
            portCntrs.goodPktsSent.l[0],
            "got other number of packets than expected on port [%d]",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
}

/**
* @internal prvTgfTailDropSharedPoolsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropSharedPoolsTrafficGenerate
(
    IN GT_U32 burstCount
)
{
    PRV_TGF_PORT_TX_COUNTER_SET_STC txCntrsGet;
    PRV_TGF_PORT_TX_COUNTER_SET_STC txCntrsExp;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_STATUS rc = GT_OK;
    GT_U32    minLimit;
    GT_U8     prfIdx/* = PRV_TGF_FDB_COUNT_CNS*/;

    /* send traffic */
    /*while (prfIdx--)*/
    for( prfIdx = 0; prfIdx < PRV_TGF_FDB_COUNT_CNS ; prfIdx++ )
    {
        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        cpssOsMemSet(&txCntrsExp, 0, sizeof(PRV_TGF_PORT_TX_COUNTER_SET_STC));
        cpssOsMemSet(&txCntrsGet, 0, sizeof(PRV_TGF_PORT_TX_COUNTER_SET_STC));

        /* blocking the Tx Queue on receive port */
        rc = prvTgfPortTxQueueTxEnableSet(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                prfIdx,
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

        PRV_UTF_LOG2_MAC("\n===> Transmitting %d packets to check Drop Profile %d ",
                burstCount, prfIdx);

        /* set DA of the packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[prfIdx],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* calculate expected values of buffers and descriptors */
        minLimit = prvTgfBufLimitArr[prfIdx] < prvTgfDescLimitArr[prfIdx]
                 ? prvTgfBufLimitArr[prfIdx] : prvTgfDescLimitArr[prfIdx];
        txCntrsExp.sharedBuff = txCntrsExp.sharedDesc = minLimit + 1;
        txCntrsExp.globalBuff = txCntrsExp.globalDesc = minLimit + prvTgfDpMaxBuffArr[2] + 1;

        /* get Tx Queue counters */
        prvTgfTailDropTxCountersGet(prfIdx, &txCntrsGet);

        /* print Tx Queue counters */
        PRV_UTF_TX_COUNTERS_LOG_MAC(txCntrsGet, txCntrsExp, global);
        PRV_UTF_TX_COUNTERS_LOG_MAC(txCntrsGet, txCntrsExp, shared);
        PRV_UTF_TX_COUNTERS_LOG_MAC(txCntrsGet, txCntrsExp, sniffedRx);
        PRV_UTF_TX_COUNTERS_LOG_MAC(txCntrsGet, txCntrsExp, sniffedTx);
        PRV_UTF_TX_COUNTERS_LOG_MAC(txCntrsGet, txCntrsExp, mcast);

        /* check Tx Queue counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.globalBuff, txCntrsGet.globalBuff,
              "got other number of globalBuff than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.globalDesc, txCntrsGet.globalDesc,
              "got other number of globalDesc than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sharedBuff, txCntrsGet.sharedBuff,
              "got other number of sharedBuff than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sharedDesc, txCntrsGet.sharedDesc,
              "got other number of sharedDesc than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sniffedRxBuff, txCntrsGet.sniffedRxBuff,
              "got other number of sniffedRxBuff than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sniffedTxBuff, txCntrsGet.sniffedTxBuff,
              "got other number of sniffedTxBuff than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sniffedRxDesc, txCntrsGet.sniffedRxDesc,
              "got other number of sniffedRxDesc than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.sniffedTxDesc, txCntrsGet.sniffedTxDesc,
              "got other number of sniffedTxDesc than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.mcastBuff, txCntrsGet.mcastBuff,
              "got other number of mcastBuff than expected, pool/tc %d", prfIdx);

        UTF_VERIFY_EQUAL1_STRING_MAC(txCntrsExp.mcastDesc, txCntrsGet.mcastDesc,
              "got other number of mcastDesc than expected, pool/tc %d", prfIdx);

        /* release Tx Queue on receive ports */
        rc = prvTgfPortTxQueueTxEnableSet(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                prfIdx,
                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

        cpssOsTimerWkAfter(10);

        /* read Eth counters */
        rc = prvTgfReadPortCountersEth(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                GT_FALSE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");

        /* print number of received packets */
        PRV_UTF_LOG3_MAC("\nPort [%d],tc [%d], received %d packets\n",
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], prfIdx,
                portCntrs.goodPktsSent.l[0]);

        /* check Eth counters */
        UTF_VERIFY_EQUAL2_STRING_MAC(
                txCntrsExp.globalBuff,
                portCntrs.goodPktsSent.l[0],
                "got other number of packets than expected on port [%d], tc [%d]",
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], prfIdx);
    }
}

/**
* @internal prvTgfTailDropWrtdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isWrtdEnabled            - GT_TRUE for enable WRTD Threshold
* @param[in] burstCount               - number of packets in one burst to transmit
* @param[in] numExpect                - number of expected packets on receive port
* @param[in] configSetIdx             - index of set of parameters for using in the test
*                                       None
*/
GT_VOID prvTgfTailDropWrtdTrafficGenerate
(
    IN GT_BOOL                     isWrtdEnabled,
    IN GT_U32                      burstCount,
    IN GT_U32                      numExpect,
    IN PRV_TGF_DROP_CONFIG_SET_ENT configSetIdx
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numBufs, numDesc;
    PRV_TGF_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC wrtdEnablers = {0};
    PRV_TGF_PORT_TX_WRTD_MASK_LSB maskLsb = {0};
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_PORT_EGRESS_CNTR_STC     egrCntr = {0};
    GT_U32                        dp;
    GT_U8                         tc =0;

    /* Sets Weighted Random Tail Drop (WRTD) masks */
    if (PRV_TGF_DROP_CONFIG_SET_0_E == configSetIdx)
    {
        dp = 0;
        wrtdEnablers.tcDpLimit = isWrtdEnabled;
        maskLsb.tcDp = 9;
    }
    else if (PRV_TGF_DROP_CONFIG_SET_2_E == configSetIdx)
    {
        dp = 1;
        wrtdEnablers.tcDpLimit = isWrtdEnabled;
        maskLsb.tcDp = 9;
    }    
    else
    {
        dp = 1;
        wrtdEnablers.portLimit = isWrtdEnabled;
        maskLsb.port = 8;
    }

    tc = prvTgfConfig[configSetIdx].tc;

    /* Enable/disable WRTD Threshold to overcome synchronization */
    rc = prvTgfPortTxProfileWeightedRandomTailDropEnableSet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            dp,
            tc,
            &wrtdEnablers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxProfileWeightedRandomTailDropEnableSet");

    /* set wrtd mask */
    rc = prvTgfPortTxTailDropWrtdMasksSet(prvTgfDevNum, &maskLsb);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropWrtdMasksSet");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear Egress counters */
    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* block TxQ=7 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfConfig[configSetIdx].tc,
            GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    PRV_UTF_LOG2_MAC("\n===> Transmitting %d packets to check WrtdEnabledDrop = %d\n",
            burstCount, isWrtdEnabled);

    /* set DA of the packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[0],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check allocated buffers */
    rc = prvTgfPortTxGlobalBufNumberGet(prvTgfDevNum, &numBufs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalBufNumberGet");
    PRV_UTF_LOG1_MAC("Number of Buffers = %d\n", numBufs);

    /* check num of buffers */
    if (isWrtdEnabled)
    {
        rc = ((numBufs > numExpect) && (numBufs < burstCount)) ? GT_OK : GT_FAIL;
    }
    else
    {
        rc = (numBufs == numExpect) ? GT_OK : GT_FAIL;
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "got other number of buffers than expected, WRTD enabled %d: %d",
            isWrtdEnabled, numBufs);

    /* check allocated descriptors */
    rc = prvTgfPortTxGlobalDescNumberGet(prvTgfDevNum, &numDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalDescNumberGet");
    PRV_UTF_LOG1_MAC("Number of Descriptors = %d\n", numDesc);

    /* check num of descriptorss */
    if (isWrtdEnabled)
    {
        rc = ((numDesc > numExpect) && (numDesc < burstCount)) ? GT_OK : GT_FAIL;
    }
    else
    {
        rc = (numDesc == numExpect) ? GT_OK : GT_FAIL;
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "got other number of descriptorss than expected, WRTD enabled %d: %d",
            isWrtdEnabled, numDesc);

    /* release TxQ=7 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfConfig[configSetIdx].tc,
            GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    /* test sends 2000 68 Bytes packets. Port speed may be 100M for tri-speed ports.
       need wait 0.01408 seconds at least */
    cpssOsTimerWkAfter(20);

    /* read Eth counters */
    rc = prvTgfReadPortCountersEth(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            GT_FALSE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");

    /* print number of received packets */
    PRV_UTF_LOG2_MAC("  Port [%d] received %d packets\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            portCntrs.goodPktsSent.l[0]);

    /* check Eth counters */
    if (isWrtdEnabled)
    {
        rc = ((portCntrs.goodPktsSent.l[0] > numExpect)
           && (portCntrs.goodPktsSent.l[0] < burstCount)) ? GT_OK : GT_FAIL;
    }
    else
    {
        rc = (portCntrs.goodPktsSent.l[0] == numExpect) ? GT_OK : GT_FAIL;
    }

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "got other number of received packets than expected on port [%d]= %d, WRTD enabled %d",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            portCntrs.goodPktsSent.l[0], isWrtdEnabled);

    /* read Egress counters */
    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* print number of egress packets */
    PRV_UTF_LOG2_MAC("  Port [%d] transmitted %d packets\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            egrCntr.outUcFrames);

    /* check Egress counters */
    if (isWrtdEnabled)
    {
        rc = ((egrCntr.outUcFrames > numExpect)
           && (egrCntr.outUcFrames < burstCount)) ? GT_OK : GT_FAIL;
    }
    else
    {
        rc = (egrCntr.outUcFrames == numExpect) ? GT_OK : GT_FAIL;
    }

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "got other number of egress packets than expected on port [%d]= %d, WRTD enabled %d",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            egrCntr.outUcFrames, isWrtdEnabled);

    /* disable WRTD Threshold */
    cpssOsMemSet(&wrtdEnablers, 0, sizeof(wrtdEnablers));

    /* disable WRTD */
    rc = prvTgfPortTxProfileWeightedRandomTailDropEnableSet(
            prvTgfDevNum,
            prvTgfConfig[configSetIdx].profileSet,
            dp,
            tc,
            &wrtdEnablers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxProfileWeightedRandomTailDropEnableSet");

    /* clear wrtd mask */
    cpssOsMemSet(&maskLsb, 0, sizeof(maskLsb));

    /* clear wrtd mask */
    rc = prvTgfPortTxTailDropWrtdMasksSet(prvTgfDevNum, &maskLsb);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropWrtdMasksSet");
}

/**
* @internal prvTgfTailDropMcConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Mcast
*/
GT_VOID prvTgfTailDropMcConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    prfIdx;
    GT_U32    dpMaxBuff = 0, dpMaxDesc = 0;
    GT_U32    mcMaxBuff = 0, mcMaxDesc = 0;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet = 0;

    /* 1. Set VLAN on all ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
            PRV_TGF_SEND_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* 2. Set HOL system mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");

    /* 3. Create FDB for receive port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfFdbArr[0], PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: store current value of Mcast buffer limit */
    rc = prvTgfPortTxMcastBuffersLimitGet(prvTgfDevNum, &prvTgfMcBuffLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitGet");

    /* AUTODOC: store current value of Mcast descr limit */
    rc = prvTgfPortTxMcastPcktDescrLimitGet(prvTgfDevNum, &prvTgfMcDescLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitGet");

    /* 4.1 Set MC buffers limit */
    rc = prvTgfPortTxMcastBuffersLimitSet(prvTgfDevNum, prvTgfMcBuffLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitSet");

    /* 4.2 Set MC descriptors limit */
    rc = prvTgfPortTxMcastPcktDescrLimitSet(prvTgfDevNum, prvTgfMcDescLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitSet");

    /* check correct setting for Buff */
    rc = prvTgfPortTxMcastBuffersLimitGet(prvTgfDevNum, &mcMaxBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfMcBuffLimit, mcMaxBuff,
                "got other number of buffers than expected");

    /* check correct setting for Desc */
    rc = prvTgfPortTxMcastPcktDescrLimitGet(prvTgfDevNum, &mcMaxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfMcDescLimit, mcMaxDesc,
                "got other number of Descriptors than expected");

    /* print get result */
    PRV_UTF_LOG2_MAC(" McBuffers&DescrLimitGet = [%d,%d]\n", mcMaxBuff, mcMaxDesc);

    /* save setting */
    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &save_tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

    /* 5. Set Dp limits for traffic class 0 */
    cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
    tailDropProfileParams.dp0MaxBuffNum  = prvTgfPortMaxBuffLimit;
    tailDropProfileParams.dp0MaxDescrNum = prvTgfPortMaxBuffLimit;

    /* set Dp limits for tc */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

    /* check correct setting */
    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, tailDropProfileParams.dp0MaxBuffNum,
            "got other tailDropProfileParams.dp0MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxBuffNum,
            "got other tailDropProfileParams.dp1MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxBuffNum,
            "got other tailDropProfileParams.dp2MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, tailDropProfileParams.dp0MaxDescrNum,
            "got other tailDropProfileParams.dp0MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxDescrNum,
            "got other tailDropProfileParams.dp1MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxDescrNum,
            "got other tailDropProfileParams.dp2MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxBuffNum,
            "got other tailDropProfileParams.tcMaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxDescrNum,
            "got other tailDropProfileParams.tcMaxDescrNum than expexted");

    /* save tail-drop profile */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E,
            &prvTgfPortMaxBuffLimitCurrent, &prvTgfPortMaxDescLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

    /* 6. Set Dp limits for tail drop profile 3 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E,
            prvTgfPortMaxBuffLimit, prvTgfPortMaxBuffLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

    /* check correct setting */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E,
            &dpMaxBuff, &dpMaxDesc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxBuff,
            "got other dpMaxBuff than expexted");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxDesc,
            "got other dpMaxDesc than expexted");

    /* 7. Bind receive port to drop profile */
    rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            CPSS_PORT_TX_DROP_PROFILE_3_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

    /* check correct setting */
    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            &profileSet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PORT_TX_DROP_PROFILE_3_E, profileSet,
            "got other PORT_TX_DROP_PROFILE than expexted");

    /* Store bind port to default used drop profile */
    for (prfIdx = 0; prfIdx < PRV_TGF_PORT_COUNT_CNS; prfIdx++)
    {
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                         prvTgfPortsArray[prfIdx],
                                         &save_pfSetArr[prfIdx]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet, port %d",
                                     prvTgfPortsArray[prfIdx]);
    }
}

/**
* @internal prvTgfTailDropMcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount1              - number of packets in first burst to transmit
* @param[in] burstCount2              - number of packets in second burst to transmit
*                                      numExpectMc     - number of expected Mc buffers
* @param[in] numExpectGlobal          - number of expected global buffers
*                                       None
*/
GT_VOID prvTgfTailDropMcTrafficGenerate
(
    IN GT_U32 burstCount1,
    IN GT_U32 burstCount2,
    IN GT_U32 numExpectMcast,
    IN GT_U32 numExpectGlobal
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numBufsMcast, numDescMcast;
    GT_U32    numBufsGlobal, numDescGlobal;
    GT_U32    portIdx;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs[PRV_TGF_PORT_COUNT_CNS];

    /* 8. block TxQ=0 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    /* 9. reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* 10. Send 1 burst of 2000 packets from port 0 to MacDA1 (unknownUC) */
    PRV_UTF_LOG1_MAC("\n===> Transmitting %d packets to check unknown UC\n",
            burstCount1);

    /* set DA for the unknown unicast */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[1],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC1_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC1_SEND_PORT_IDX_CNS]);

    /* 11. Send 1 burst of 1000 packets from port 23 to MacDA0 (knownUC) */
    PRV_UTF_LOG1_MAC("\n===> Transmitting %d packets to check known UC\n",
            burstCount2);

    /* set DA for the known unicast */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[0],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount2, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC2_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC2_SEND_PORT_IDX_CNS]);

    /* 12. Verify there are 601 Mcast buffers */
    rc = prvTgfPortTxMcastBufNumberGet(prvTgfDevNum, &numBufsMcast);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBufNumberGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(numExpectMcast, numBufsMcast,
                                 "Mcast buffers different then expected");

    /* 13. Verify there are 601 Mcast descriptors */
    rc = prvTgfPortTxMcastDescNumberGet(prvTgfDevNum, &numDescMcast);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastDescNumberGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(numExpectMcast, numDescMcast,
                                 "Mcast descriptors different then expected");

    /* 14. Verify there are 1601 Global buffers */
    rc = prvTgfPortTxGlobalBufNumberGet(prvTgfDevNum, &numBufsGlobal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalBufNumberGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(numExpectGlobal, numBufsGlobal,
                                 "Global buffres different then expected");

    /* 15. Verify there are 1601 Global descriptors */
    rc = prvTgfPortTxGlobalDescNumberGet(prvTgfDevNum, &numDescGlobal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxGlobalDescNumberGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(numExpectGlobal, numDescGlobal,
                                 "Global descriptorss different then expected");

    /* release TxQ=0 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_0_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    cpssOsTimerWkAfter(20);

    /* read Eth counters for all ports */
    for (portIdx = 0; portIdx < PRV_TGF_PORT_COUNT_CNS; portIdx++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[portIdx],
                GT_FALSE, &portCntrs[portIdx]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(0,
                                 portCntrs[0].goodPktsSent.l[0] - burstCount1,
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(numExpectMcast,
                                 portCntrs[1].goodPktsSent.l[0],
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(numExpectMcast + burstCount2,
                                 portCntrs[2].goodPktsSent.l[0],
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[2]);

    UTF_VERIFY_EQUAL1_STRING_MAC(numExpectMcast,
                                 portCntrs[3].goodPktsSent.l[0] - burstCount2,
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[3]);
}

/**
* @internal prvTgfTailDropConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] numEntries               - number of QoS, FDB and PCL entries to restore.
*                                       None
*/
GT_VOID prvTgfTailDropConfigurationRestore
(
    IN GT_U32 numEntries
)
{
    GT_STATUS                         rc;
    GT_U32                            prfIdx, pclRuleIdx;
    PRV_TGF_COS_PROFILE_STC           cosProfile;
    GT_U8                             tc;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT pfSet;
    CPSS_PORT_EGRESS_CNT_MODE_ENT     setModeBmp = 0;

    /* restore egress counters mode */
    rc = prvTgfPortEgressCntrModeSet(
            0,
            setModeBmp,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_SEND_VLANID_CNS,
            0,
            CPSS_DP_GREEN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet");

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E))
    {
        /* disable all shared pools for all tc */
        for (tc = 0; tc < 8; tc++)
        {
            for (pfSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
                 pfSet <= CPSS_PORT_TX_DROP_PROFILE_8_E; pfSet++)
            {
                rc = prvTgfPortTxTcSharedProfileEnableSet(prvTgfDevNum, pfSet, tc,
                        PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                        "prvTgfPortTxTcSharedProfileEnableSet");
            }
        }
    }

    /* set number of QoS, FDB and PCL entries */
    prfIdx = (numEntries == PRV_TGF_DST_COUNT_CNS) ? numEntries
           : (numEntries == PRV_TGF_FDB_COUNT_CNS) ? numEntries
           : 1;

    while (prfIdx--)
    {
        /* reset QoS profile with default values of DP, UP, TC, DSCP, Exp */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));

        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E))
        {
            rc = prvTgfCosProfileEntrySet((GT_U8)prfIdx, &cosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
        }

        pclRuleIdx = prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0/*lookupId*/,prfIdx);

        /* invalidate PCL rules */
        rc = prvTgfPclRuleValidStatusSet(
                CPSS_PCL_RULE_SIZE_STD_E, pclRuleIdx, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
    }

    /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* Bind port to default used drop profile (2)*/
    for (prfIdx = 0; prfIdx < PRV_TGF_PORT_COUNT_CNS; prfIdx++)
    {
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
                                         prvTgfPortsArray[prfIdx],
                                         save_pfSetArr[prfIdx]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet, port %d",
                                     prvTgfPortsArray[prfIdx]);
    }

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA3_E))
    {
        /* enable global resource sharing */
        rc = prvTgfPortTxSharingGlobalResourceEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharingGlobalResourceEnableSet");

        /* enable FC */
         for (prfIdx = 0; prfIdx < PRV_TGF_PORT_COUNT_CNS; prfIdx++)
         {
             rc = prvTgfPortFlowControlEnableSet(prvTgfDevNum,
                                                 prvTgfPortsArray[prfIdx],
                                                 CPSS_PORT_FLOW_CONTROL_RX_TX_E);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfPortFlowControlEnableSet, port %d",
                                         prvTgfPortsArray[prfIdx]);
         }
    }
}

/**
* @internal tgfTailDropSeparateUcMcCountPerQueueConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Mcast
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueConfigurationSet
(
    IN  GT_BOOL isFirstCall,
    IN  GT_BOOL sharedDisable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    dpMaxBuff = 0, dpMaxDesc = 0;
    GT_U32    mcMaxBuff = 0, mcMaxDesc = 0;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet = 0;
    GT_U32    prfIdx;

    if(isFirstCall)
    {

        /* AUTODOC: SETUP CONFIGURATION: */

        /* AUTODOC: 1. Set VLAN on all ports */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
                PRV_TGF_SEND_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

        /* AUTODOC: 2. Set HOL system mode */
        rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");

        /* AUTODOC: 3. Create FDB for receive port */
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(
                prvTgfFdbArr[0], PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

        /* AUTODOC: store current value of Mcast buffer limit */
        rc = prvTgfPortTxMcastBuffersLimitGet(prvTgfDevNum, &prvTgfMcBuffLimitCurrent);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitGet");
        /* AUTODOC: store current value of Mcast descr limit */
        rc = prvTgfPortTxMcastPcktDescrLimitGet(prvTgfDevNum, &prvTgfMcDescLimitCurrent);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitGet");

        /* AUTODOC: 4.1 Set MC buffers limit */
        rc = prvTgfPortTxMcastBuffersLimitSet(prvTgfDevNum, prvTgfMcBuffLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitSet");

        /* AUTODOC: 4.2 Set MC descriptors limit */
        rc = prvTgfPortTxMcastPcktDescrLimitSet(prvTgfDevNum, prvTgfMcDescLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitSet");

        /* AUTODOC: check correct setting for Buff */
        rc = prvTgfPortTxMcastBuffersLimitGet(prvTgfDevNum, &mcMaxBuff);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitGet");

        /* AUTODOC: verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfMcBuffLimit, mcMaxBuff,
                    "got other number of buffers than expected");

        /* AUTODOC: check correct setting for Desc */
        rc = prvTgfPortTxMcastPcktDescrLimitGet(prvTgfDevNum, &mcMaxDesc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitGet");

        /* AUTODOC: verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfMcDescLimit, mcMaxDesc,
                    "got other number of Descriptors than expected");

        /* AUTODOC: print get result */
        PRV_UTF_LOG2_MAC(" McBuffers&DescrLimitGet = [%d,%d]\n", mcMaxBuff, mcMaxDesc);

        /* Store bind port to default used drop profile */
        for (prfIdx = 0; prfIdx < PRV_TGF_PORT_COUNT_CNS; prfIdx++)
        {
            rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                             prvTgfPortsArray[prfIdx],
                                             &save_pfSetArr[prfIdx]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet, port %d",
                                         prvTgfPortsArray[prfIdx]);
        }

        /* save setting */
        rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &save_tailDropProfileParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");
    }

    /* 5. Set Dp0 UC and MC limits + enable separate counting */
    cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
    tailDropProfileParams.dp0MaxBuffNum  = prvTgfPortUcMaxBuffLimit;
    tailDropProfileParams.dp0MaxDescrNum = prvTgfPortUcMaxBuffLimit * 10; /* don't care */
    tailDropProfileParams.dp0MaxMCBuffNum = prvTgfPortMcMaxBuffLimit;
    tailDropProfileParams.sharedUcAndMcCountersDisable = sharedDisable;

    /* AUTODOC: set Dp limits for tc */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

    /* AUTODOC: check correct setting */
    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

    /* verifying values */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortUcMaxBuffLimit, tailDropProfileParams.dp0MaxBuffNum,
            "got other tailDropProfileParams.dp0MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxBuffNum,
            "got other tailDropProfileParams.dp1MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxBuffNum,
            "got other tailDropProfileParams.dp2MaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortUcMaxBuffLimit*10, tailDropProfileParams.dp0MaxDescrNum,
            "got other tailDropProfileParams.dp0MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxDescrNum,
            "got other tailDropProfileParams.dp1MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxDescrNum,
            "got other tailDropProfileParams.dp2MaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxBuffNum,
            "got other tailDropProfileParams.tcMaxBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.tcMaxDescrNum,
            "got other tailDropProfileParams.tcMaxDescrNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(sharedDisable, tailDropProfileParams.sharedUcAndMcCountersDisable,
            "got other tailDropProfileParams.sharedUcAndMcCountersDisable than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMcMaxBuffLimit, tailDropProfileParams.dp0MaxMCBuffNum,
            "got other tailDropProfileParams.dp0MaxMCBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp1MaxMCBuffNum,
            "got other tailDropProfileParams.dp0MaxMCBuffNum than expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, tailDropProfileParams.dp2MaxMCBuffNum,
            "got other tailDropProfileParams.dp0MaxMCBuffNum than expexted");

    if(isFirstCall)
    {
        rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_3_E,
                &prvTgfPortMaxBuffLimitCurrent, &prvTgfPortMaxDescLimitCurrent);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

        /* AUTODOC: 6. Set Dp limits for tail drop profile 3 */
        rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_3_E,
                prvTgfPortMaxBuffLimit, prvTgfPortMaxBuffLimit);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

        /* check correct setting */
        rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum,
                CPSS_PORT_TX_DROP_PROFILE_3_E,
                &dpMaxBuff, &dpMaxDesc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxBuff,
                "got other dpMaxBuff than expexted");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortMaxBuffLimit, dpMaxDesc,
                "got other dpMaxDesc than expexted");

        /* AUTODOC: 7. Bind receive port to drop profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
                CPSS_PORT_TX_DROP_PROFILE_3_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* check correct setting */
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
                &profileSet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PORT_TX_DROP_PROFILE_3_E, profileSet,
                "got other PORT_TX_DROP_PROFILE than expexted");
    }
}

/**
* @internal tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount1              - number of packets in first burst to transmit
* @param[in] burstCount2              - number of packets in second burst to transmit
* @param[in] numExpectPackets         - number of expected buffers
*                                       None
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate
(
    IN GT_U32 burstCount1,
    IN GT_U32 burstCount2,
    IN GT_U32 numExpectPackets
)
{
    GT_STATUS rc = GT_OK;
    /*GT_U32    numBufsMcast, numDescMcast;*/
    /*GT_U32    numBufsGlobal, numDescGlobal;*/
    GT_U32    portIdx;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs[PRV_TGF_PORT_COUNT_CNS];

    /* AUTODOC: 8. block TxQ=0 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    /* AUTODOC: 9. reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: 10. Send 1 burst of 10 packets from port 0 to MacDA1 (unknownUC) */
    PRV_UTF_LOG1_MAC("\n===> Transmitting %d packets to check unknown UC\n",
            burstCount1);

    /* AUTODOC: set DA for the unknown unicast */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[1],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* AUTODOC: start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC1_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC1_SEND_PORT_IDX_CNS]);

    /* AUTODOC: 11. Send 1 burst of 10 packets from port 23 to MacDA0 (knownUC) */
    PRV_UTF_LOG1_MAC("\n===> Transmitting %d packets to check known UC\n",
            burstCount2);

    /* AUTODOC: set DA for the known unicast */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbArr[0],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, burstCount2, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* AUTODOC: start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC2_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC2_SEND_PORT_IDX_CNS]);

    /* AUTODOC: release TxQ=0 for receive port */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_TC_0_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

    cpssOsTimerWkAfter(10);

    /* AUTODOC: read Eth counters for all ports */
    for (portIdx = 0; portIdx < PRV_TGF_PORT_COUNT_CNS; portIdx++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[portIdx],
                GT_FALSE, &portCntrs[portIdx]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(0,
                                 portCntrs[0].goodPktsSent.l[0] - burstCount1,
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(burstCount1,
                                 portCntrs[1].goodPktsSent.l[0],
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(numExpectPackets,
                                 portCntrs[2].goodPktsSent.l[0],
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[PRV_TGF_MC_RECEIVE_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(burstCount1,
                                 portCntrs[3].goodPktsSent.l[0] - burstCount2,
                                 "wrong number of packets received on port %d",
                                 prvTgfPortsArray[3]);
}


/**
* @internal tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc;

    /*AUTODOC: CONFIGURATION RESTORE */

    /*AUTODOC: restore Mcast buff limit */
    rc = prvTgfPortTxMcastBuffersLimitSet(prvTgfDevNum, prvTgfMcBuffLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastBuffersLimitSet");

    /*AUTODOC: restore Mcast descr limit */
    rc = prvTgfPortTxMcastPcktDescrLimitSet(prvTgfDevNum, prvTgfMcDescLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxMcastPcktDescrLimitSet");

    /*AUTODOC: restore port max buff and max descr limit for profile 3 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E,
            prvTgfPortMaxBuffLimitCurrent, prvTgfPortMaxDescLimitCurrent);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet");

    /* AUTODOC: restore limits for tc */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
            CPSS_PORT_TX_DROP_PROFILE_3_E, PRV_TGF_TC_0_CNS, &save_tailDropProfileParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");


}




