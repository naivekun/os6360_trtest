/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfEgrPclEvlanBindingMode.c
*
* @brief Test eVLAN-based binding mode for Egress PCL Configuration Table
* Entry Selection.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfMirror.h>
#include <pcl/prvTgfEgrPclEvlanBindingMode.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS         0
#define PRV_TGF_TX_PORT_IDX_CNS         1
#define PRV_TGF_ANALYZER_PORT_IDX_CNS   2

/* an incoming packet and rx port are tagged in  vlan PRV_TGF_RX_VLANID_CNS. */
#define PRV_TGF_RX_VLANID_CNS 5

/* eVlanId assinged to the incoming packet by IPCL */
#define PRV_TGF_IPCL_E_VLAN_CNS  ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC((_4K + 0x0E))

/* number of vlans used to configure ports */
#define PRV_TGF_VLANS_COUNT_CNS 2

/* packet's vlan tag0 set by rule with pclId specified by an EPCL Configuration
   entry corresponding to an original vlanId of the packet */
#define PRV_TGF_NEW_VLANID_ORIG_CNS 7

/* packet's vlan tag0 set by rule with pclId specified by an EPCL Configuration
   entry corresponding to a  packet's eVlan set by the ingress pipe */
#define PRV_TGF_NEW_VLANID_INGR_CNS 8

/* analyzer index */
#define PRV_TGF_ANALYZER_IDX_CNS 0

/* IPCL rule index */
#define PRV_TGF_IPCL_RULE_IDX 0

/* EPCL rule indexes */
#define PRV_TGF_EPCL_RULE1_IDX (PRV_TGF_IPCL_RULE_IDX + 1)
#define PRV_TGF_EPCL_RULE2_IDX (PRV_TGF_IPCL_RULE_IDX + 2)

/* struct connects an EPCL Configuration Table Entry and a Rule's key entry. */
typedef struct
{
    GT_U16 eVlan;            /* packet eVlan. Will be used to get configuration
                                from EPCL Lookup Configuration table */
    GT_U16 pclId;            /* pclId, common for both a lookup Configuration
                                entry and a rule's key entry.*/
    GT_U32 ruleIndex;        /* a rule index */
    GT_U16 newVlanId;        /* vlan tag0 to be set by the rule */
} PRV_TGF_RULE_CFG_STC;

/* operations on a packet. */
typedef enum
{
    PRV_TGF_FORWARD_E,
    PRV_TGF_MIRROR_TO_ANALYZER_E,
    PRV_TGF_TRAP_TO_CPU_E
} PRV_TGF_PACKET_ACTION_ENT;

/* rule index and vlanId values used by EPCL rules. */
static PRV_TGF_RULE_CFG_STC prvTgfEgrRulesCfgArray[PRV_TGF_VLANS_COUNT_CNS] = {
    {PRV_TGF_RX_VLANID_CNS,   1, PRV_TGF_EPCL_RULE1_IDX, PRV_TGF_NEW_VLANID_ORIG_CNS},
    {0/*set in run time to PRV_TGF_IPCL_E_VLAN_CNS*/, 2, PRV_TGF_EPCL_RULE2_IDX, PRV_TGF_NEW_VLANID_INGR_CNS}};

/* some original values saved to be restored at the end of the test. */

/* interface of an analyzer bound to an rx-port */
PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          savedAnalyzerIface;
/* rx port in mirroring mode */
GT_BOOL                                        savedRxMirrorModeEnabled;
/* Analizer index of rx port */
GT_U32                                         savedRxMirrorAnalyzerIdx;
/* true is prvTgfMirrorRxPortGet (to save dafaults) was saccessfull */
GT_BOOL                                        isRxMirrorStatusSaved;
/* value of bit <EPCL Key Ing Analyzer Use Orig VID> of Global Register */
GT_BOOL                                        savedUseOrigVid;
/* a maximum allowed eVLAN-ID */
GT_U16                                         savedMaxVid;
/* value of EPCL Packet type enable bits */
GT_BOOL                                        savedNonTsEpclEnable;
GT_BOOL                                        savedToCpuEpclEnable;
GT_BOOL                                        savedToAnalyzerEpclEnable;

/******************************* Test packets *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_RX_VLANID_CNS                         /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[48] ={0};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* size of packet */
#define PRV_TGF_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                 + TGF_VLAN_TAG_SIZE_CNS         \
                                 + sizeof(prvTgfPayloadDataArr))

/* packet info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_SIZE_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;


/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/**************************** PRIVATE FUNCTIONS *******************************/

/**
* @internal prvTgfEgrPclEvlanBindingModeEgressPclEnable function
* @endinternal
*
* @brief   Enable Egress PCL Engine per device.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEgrPclEvlanBindingModeEgressPclEnable
(
    GT_VOID
)
{
    GT_STATUS                       rc;

    /* AUTODOC: Init PCL */
    rc = prvTgfPclInit();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");

    /* AUTODOC: Enables egress policy per device */
    rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPolicyEnable");

    return GT_OK;
}

/**
* @internal prvTgfEgressPclEvlanInit function
* @endinternal
*
* @brief   Enable EPCL on the port and setup eVlan-based access to
*         EPCL configuration table
* @param[in] portNum                  - physical port number
* @param[in] packetType               - packet type.
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEgressPclEvlanInit
(
    IN GT_U32                            portNum,
    IN PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT  packetType,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_STATUS                       rc;

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, portNum,
                                          packetType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure eVlan access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                            portNum, CPSS_PCL_DIRECTION_EGRESS_E, lookupNum, 0,
                            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
    return GT_OK;
}

/**
* @internal prvTgfEgrPclEvlanBindingModeSetupIpcl function
* @endinternal
*
* @brief   Activate IPCL and create a rule to modify packet's eVlan to be
*         equal PRV_TGF_IPCL_E_VLAN_CNS.
*
* @retval GT_OK                    - on success
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeSetupIpcl
(
    GT_VOID
)
{
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;
    GT_STATUS                   rc;

    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    /* AUTODOC: enable IPCL on rx port */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit, port=%d",
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    /* AUTODOC: setup IPCL rule to change packet's eVlan=PRV_TGF_IPCL_E_VLAN_CNS */

    /* pattern for the source port */
    cpssOsMemSet(&mask.ruleStdNotIp.common.sourcePort, 0xFF,
                 sizeof(mask.ruleStdNotIp.common.sourcePort));

    /* pattern for the source port */
    pattern.ruleStdNotIp.common.sourcePort =
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] &
                                      0xFF;

    /* action - set eVlan to PRV_TGF_IPCL_E_VLAN_CNS */
    action.egressPolicy    =  GT_FALSE;
    action.pktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_IPCL_E_VLAN_CNS;

    rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                           PRV_TGF_IPCL_RULE_IDX,
                           &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet, rule index=%d",
                                 PRV_TGF_IPCL_RULE_IDX);
}

/**
* @internal prvTgfEgrPclEvlanBindingModeSetupEpcl function
* @endinternal
*
* @brief   Activate EPCL and create a rules to modify packet's tag0 vlanId to be
*         equal to prvTgfEgrRulesCfgArray[i].newVlanId
*
* @retval GT_OK                    - on success
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeSetupEpcl
(
    GT_VOID
)
{

    PRV_TGF_PCL_RULE_FORMAT_UNT mask;      /* rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;   /* rule pattern */
    PRV_TGF_PCL_ACTION_STC      action;    /* rule aciton */
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg; /* PCL Configuration Table Entry */
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    GT_U32                      i;
    GT_STATUS                   rc;

    cpssOsMemSet(&lookupCfg,      0, sizeof(lookupCfg));
    cpssOsMemSet(&interfaceInfo,  0, sizeof(interfaceInfo));
    cpssOsMemSet(&mask,           0, sizeof(mask));
    cpssOsMemSet(&pattern,        0, sizeof(pattern));
    cpssOsMemSet(&action,         0, sizeof(action));

    /* AUTODOC: enable EPCL on egress ports (tx, Analyzer, CPU) */
    /* enable EPCL globally */
    rc = prvTgfEgrPclEvlanBindingModeEgressPclEnable();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfEgrPclEvlanBindingModeEgressPclEnable");

    /* Save EPCL Packet type enable bits */
    rc = prvTgfPclEgressPclPacketTypesGet(
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
                            &savedNonTsEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");
    rc = prvTgfPclEgressPclPacketTypesGet(
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                            PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E,
                            &savedToAnalyzerEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");
    rc = prvTgfPclEgressPclPacketTypesGet(
                            prvTgfDevNum,
                            CPSS_CPU_PORT_NUM_CNS,
                            PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                            &savedToCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");

    /* enable EPCL on tx port */
    rc = prvTgfEgressPclEvlanInit(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                               PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
                               CPSS_PCL_LOOKUP_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEvlanInit");

    /* enable EPCL on Analyzer port */
    rc = prvTgfEgressPclEvlanInit(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                               PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E,
                               CPSS_PCL_LOOKUP_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEvlanInit");

    /* enable EPCL on CPU port */
    rc = prvTgfEgressPclEvlanInit(CPSS_CPU_PORT_NUM_CNS,
                               PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                               CPSS_PCL_LOOKUP_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEvlanInit");

    /* AUTODOC: set EPCL configuration table entries. */
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    interfaceInfo.type  = CPSS_INTERFACE_VID_E;

    for (i = 0; i < PRV_TGF_VLANS_COUNT_CNS; ++i)
    {
        lookupCfg.pclId      = prvTgfEgrRulesCfgArray[i].pclId;
        interfaceInfo.vlanId = prvTgfEgrRulesCfgArray[i].eVlan % 4096;

        /* write an entry into the EPCL configuration table */
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    /* AUTODOC: setup EPCL rules. */
    /* mask for MAC DA */
    cpssOsMemSet(mask.ruleEgrStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleEgrStdNotIp.macDa.arEther));

    /* pattern for MAC DA */
    cpssOsMemCpy(pattern.ruleEgrStdNotIp.macDa.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    mask.ruleEgrStdNotIp.common.pclId = 0x3FF;

    /* action - set vlan tag0 */
    action.egressPolicy          = GT_TRUE;
    action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
    for (i=0; i<PRV_TGF_VLANS_COUNT_CNS; ++i)
    {
        /* pclId should be the same as lookupCfg.pclId above */
        pattern.ruleEgrStdNotIp.common.pclId = prvTgfEgrRulesCfgArray[i].pclId;
        action.vlan.vlanId = prvTgfEgrRulesCfgArray[i].newVlanId;

        rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                               prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(prvTgfEgrRulesCfgArray[i].ruleIndex),
                               &mask, &pattern, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleSet failed, rule index=%d",
                                     prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(prvTgfEgrRulesCfgArray[i].ruleIndex));
    }
}

/**
* @internal prvTgfEgrPclEvlanBindingModeSendPacket function
* @endinternal
*
* @brief   Send packet
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeSendPacket
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC         analyzerPortInterface;
    GT_U32                          portIter;
    GT_STATUS                       rc;

    cpssOsMemSet(&analyzerPortInterface, 0, sizeof(analyzerPortInterface));

    /* reset ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficTableRxPcktTblClear error");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "StartTransmitting error: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    /* wait the packets come to CPU */
    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);
}

/**
* @internal prvTgfEgrPclEvlanBindingModeCheckPacketOnPort function
* @endinternal
*
* @brief   Check packet came on the interface with expected vlanId.
*
* @param[in] ifacePtr                 -  (pointer to) egress interface.
* @param[in] expVlanId                -  expected vlan id.
*                                       None
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeCheckPacketOnPort
(
    IN CPSS_INTERFACE_INFO_STC        *ifacePtr,
    IN GT_U16                          expVlanId
)
{
    TGF_NET_DSA_STC     rxParam;
    GT_U8               packetBuf[PRV_TGF_PACKET_SIZE_CNS];
    GT_U32              packetLen        = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32              origPacketLen    = 0;
    GT_U8               queue            = 0;
    GT_U8               devNum           = 0;
    GT_BOOL             isFirst          = GT_TRUE;
    GT_U32              packetsCount     = 0;
    TGF_PACKET_TYPE_ENT packetType;
    GT_STATUS           rc;

    /* if the packet was trapped to CPU we expect it comes from RX port. */
    packetType = (prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] == ifacePtr->devPort.portNum) ?
                  TGF_PACKET_TYPE_REGULAR_E : TGF_PACKET_TYPE_CAPTURE_E;

    /* get first captured packet from expected interface */
    isFirst = GT_TRUE;

    /* iterate through all captured packets */
    packetsCount = 0;
    do
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(
                      ifacePtr, packetType, isFirst, GT_TRUE, packetBuf,
                      &packetLen, &origPacketLen, &devNum, &queue, &rxParam);

        isFirst = GT_FALSE;  /* next time we'll get the next packet*/
        if (rc != GT_OK)
        {
            break;
        }
        packetsCount++;

        /* compare vlanId (12 bits). */
        if ((GT_U8)(expVlanId>>8) != (packetBuf[TGF_L2_HEADER_SIZE_CNS + 2] & 0x0F) ||
            (GT_U8)(expVlanId)    !=  packetBuf[TGF_L2_HEADER_SIZE_CNS + 3])
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "Unexpected vlanId on port %d. Expected: %d",
                                         rxParam.portNum, expVlanId);
        }
    }
    while (GT_TRUE);

    rc = (GT_NO_MORE == rc ? GT_OK : rc);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                              "prvTgfIpv4Uc2MultipleVrCheckCountersAfterBurst");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, packetsCount,
                                 "Number of captured packets is wrong.");
}

/**
* @internal prvTgfEgrPclEvlanBindingModeCheckResults function
* @endinternal
*
* @brief   Check counters on ports and MAC DA of the packet after transmitting.
*
* @param[in] ifacePtr                 -  (pointer to) egress port interface.
* @param[in] ifaceAnPtr               -  (pointer to) Analyzer port interface.
* @param[in] usedOrigVid              - GT_TRUE  - an original VID of the incoming packet was used
*                                      for access to the EPCL Configuration table
*                                      GT_FALSE - an eVlan[11:0] set by ingress pipe was used
*                                      for access to the EPCL Configuration table
* @param[in] packetAction             - action that should be done with the packet
*                                      PRV_TGF_FORWARD_E - send to egress port.
*                                      PRV_TGF_MIRROR_TO_ANALYZER_E - send to egress port and
*                                      mirror to Analyzer port
*                                      PRV_TGF_TRAP_TO_CPU_E - trap to CPU
*                                       None
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeCheckResults
(
    IN CPSS_INTERFACE_INFO_STC        *ifacePtr,
    IN CPSS_INTERFACE_INFO_STC        *ifaceAnPtr,
    IN GT_BOOL                         usedOrigVid,
    IN PRV_TGF_PACKET_ACTION_ENT       packetAction
)
{
    GT_U32     expRx;          /* expected rx packet counter value */
    GT_U32     expTx;          /* expected tx packet counter value */
    GT_U16     expVlanIdTx;    /* expected vlan id on tx port*/
    GT_U16     expVlanIdAn;    /* expected vlan id on analyzer or CPU port*/
    GT_U32     i;
    GT_STATUS  rc;

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        if (PRV_TGF_RX_PORT_IDX_CNS == i)
        {
            expTx = prvTgfBurstCount;
            expRx = expTx;
        }
        else if (PRV_TGF_TX_PORT_IDX_CNS == i )
        {
            expTx =  (PRV_TGF_TRAP_TO_CPU_E == packetAction)?
                       0 : prvTgfBurstCount;
            expRx = expTx;
        }
        else if (PRV_TGF_ANALYZER_PORT_IDX_CNS == i)
        {
            expTx = (PRV_TGF_MIRROR_TO_ANALYZER_E == packetAction) ?
                      prvTgfBurstCount : 0;
            expRx = expTx;
        }
        else
        {
            expTx = 0;
            expRx = 0;
        }

        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRx, expTx, PRV_TGF_PACKET_SIZE_CNS, 1);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d Expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRx, expTx);
    }

    /* calculate expected vlan id */
    expVlanIdTx = PRV_TGF_NEW_VLANID_INGR_CNS;
    expVlanIdAn = (GT_TRUE == usedOrigVid && (PRV_TGF_MIRROR_TO_ANALYZER_E == packetAction
                                                ||   PRV_TGF_TRAP_TO_CPU_E == packetAction)) ?
                   PRV_TGF_NEW_VLANID_ORIG_CNS :
                   PRV_TGF_NEW_VLANID_INGR_CNS;

    /* AUTODOC: Check vlanId on tx port(1) if packet was not trap to CPU */
    if (PRV_TGF_TRAP_TO_CPU_E != packetAction)
    {
        prvTgfEgrPclEvlanBindingModeCheckPacketOnPort(ifacePtr, expVlanIdTx);
    }

    /* AUTODOC: Check vlanId on Analyzer port(2) or on CPU port (if required) */
    if (PRV_TGF_MIRROR_TO_ANALYZER_E == packetAction ||
          PRV_TGF_TRAP_TO_CPU_E      == packetAction)
    {
        prvTgfEgrPclEvlanBindingModeCheckPacketOnPort(ifaceAnPtr, expVlanIdAn);
    }
}

/**
* @internal prvTgfEgrPclEvlanBindingModeConfigurationSet function
* @endinternal
*
* @brief   setup base configuration
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  iface;
    GT_U8                                  tag         = 1;
    GT_U32                                 rxPort;
    GT_U32                                 txPort;
    GT_STATUS                              rc;

    /* init value that can not be initialized at compilation time */
    prvTgfEgrRulesCfgArray[1].eVlan = PRV_TGF_IPCL_E_VLAN_CNS;

    cpssOsMemSet(&iface, 0, sizeof(iface));
    rxPort = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    txPort = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /*----------------SAVE SOME VALUES TO BE RESTORED AT THE END--------------*/

    /* AUTODOC: save a mirroring status of rx port */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               &savedRxMirrorModeEnabled,
                               &savedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet error");

    /* AUTODOC: save a Global Register field <EPCL Key Ing Analyzer Use Orig VID> */
    prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet(prvTgfDevNum,
                                                 &savedUseOrigVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet");
    /* AUTODOC: save an old analyzer interface bound to PRV_TGF_ANALYZER_IDX_CNS */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_ANALYZER_IDX_CNS,
                                          &savedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a maximum allowed eVLAN-ID */
    rc = prvTgfBrgVlanRangeGet(&savedMaxVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet");

    /*-------------------------SETUP CONFIGURATION----------------------------*/

    /* AUTODOC: set a maximum eVLAN-ID greater then all VID used by the test. */
    rc = prvTgfBrgVlanRangeSet(PRV_TGF_IPCL_E_VLAN_CNS + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /*-------------------------CREATE VLANS-----------------------------------*/
    /* AUTODOC:  create vlan with tagged rx port. */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RX_VLANID_CNS,
                                           &rxPort, NULL, &tag, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: create vlan used by IPCL to replace  packet's original vlan.
       add tx-port into this vlan. */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_IPCL_E_VLAN_CNS,
                                      &txPort, NULL, &tag, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /*------------------------CREATE AN FDB ENTRY-----------------------------*/
    /* create FDB entry with packet MAC DA on tx port. It is not required by
       test's logic. This is done to get round a bug in the BC2 GM */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                     PRV_TGF_IPCL_E_VLAN_CNS, prvTgfDevNum,
                                     txPort, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: setup port #2 to be an Analyzer interface */
    rc = prvTgfMirrorRxAnalyzerPortSet(
                 prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS], prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorRxAnalyzerPortSet error");


    /*-------------------------ENABLE IPCL, EPCL------------------------------*/
    prvTgfEgrPclEvlanBindingModeSetupIpcl();
    prvTgfEgrPclEvlanBindingModeSetupEpcl();
}

/**
* @internal prvTgfEgrPclEvlanBindingModeConfigurationRestore function
* @endinternal
*
* @brief   restore configuration
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS  rc;

    /* AUTODOC: RESTORE CONFIGURATION */

    /* AUTODOC: restore a mirroring state of the rx-port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               savedRxMirrorModeEnabled,
                               savedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet error");

    /* AUTODOC: restore a Global Register field <EPCL Key Ing Analyzer Use Orig VID> */
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(savedUseOrigVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet");

    /* AUTODOC: restore Analyzer interface bound to index PRV_TGF_ANALYZER_INDEX_CNS. */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS,
                                          &savedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error");

    /* AUTODOC : disable all IPCL, EPCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

   /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_IDX, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d",
                                 PRV_TGF_IPCL_RULE_IDX, GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_EPCL_RULE1_IDX, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d",
                                 PRV_TGF_EPCL_RULE1_IDX, GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_EPCL_RULE2_IDX, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d",
                                 PRV_TGF_EPCL_RULE2_IDX, GT_FALSE);

    /* AUTODOC: Restore access mode for EPCL lookup on ePort*/
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                            CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet: %d",
                                 prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                            CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "prvTgfPclPortLookupCfgTabAccessModeSet: %d",
                               prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                            CPSS_CPU_PORT_NUM_CNS,
                            CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet: %d",
                                 CPSS_CPU_PORT_NUM_CNS);

    /* AUTODOC: disable IPCL globally */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable EPCL globally */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable");

    /* Restore EPCL Packet type enable bits */
    rc = prvTgfPclEgressPclPacketTypesSet(
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
                            savedNonTsEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                            PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E,
                            savedToAnalyzerEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(
                            prvTgfDevNum,
                            CPSS_CPU_PORT_NUM_CNS,
                            PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                            savedToCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: clear FDB */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore a maximum allowed eVLAN-ID. */
    rc = prvTgfBrgVlanRangeSet(savedMaxVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* AUTODOC: invalidate used vlan entries */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RX_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_RX_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_IPCL_E_VLAN_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_IPCL_E_VLAN_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}


/**
* @internal prvTgfEgrPclEvlanBindingModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfEgrPclEvlanBindingModeTrafficGenerate
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    CPSS_INTERFACE_INFO_STC       iface;
    CPSS_INTERFACE_INFO_STC       ifaceRx;
    CPSS_INTERFACE_INFO_STC       ifaceAn; /* Analyzer port interface */
    GT_STATUS                     rc;

    cpssOsMemSet(&macEntry,  0, sizeof(macEntry));
    cpssOsMemSet(&iface,     0, sizeof(iface));
    cpssOsMemSet(&ifaceRx,   0, sizeof(ifaceRx));
    cpssOsMemSet(&ifaceAn,   0, sizeof(ifaceAn));

    /* AUTODOC: generate traffic */

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* set rx port interface */
    ifaceRx.type              = CPSS_INTERFACE_PORT_E;
    ifaceRx.devPort.hwDevNum  = prvTgfDevNum;
    ifaceRx.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    /* set tx port interface */
    iface.type                = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum    = prvTgfDevNum;
    iface.devPort.portNum     = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* set Analyzer port interface */
    ifaceAn.type              = CPSS_INTERFACE_PORT_E;
    ifaceAn.devPort.hwDevNum  = prvTgfDevNum;
    ifaceAn.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    /* enable capturing on the tx port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                          TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "tgfTrafficGeneratorPortTxEthCaptureSet error");

    /* enable capturing on the Analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ifaceAn,
                                         TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "tgfTrafficGeneratorPortTxEthCaptureSet error");

    /* AUTODOC: CASE 1: enable <EPCL Key Ing Analyzer Use Orig VID> and send */
    PRV_UTF_LOG0_MAC("\nCASE 1: enable original VLAN usage and send\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceAn, GT_TRUE,
                                             PRV_TGF_FORWARD_E);

    /* AUTODOC: CASE 2: disable <EPCL Key Ing Analyzer Use Orig VID> and send */
    PRV_UTF_LOG0_MAC("\nCASE 2: disable original VLAN usage and send\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceAn, GT_FALSE,
                                             PRV_TGF_FORWARD_E);

    /* enable mirroring on the rx-port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               GT_TRUE, PRV_TGF_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet error");

    /* AUTODOC: CASE 3: disable <EPCL Key Ing Analyzer Use Orig VID> and send
       with mirroring to Analyzer */
    PRV_UTF_LOG0_MAC("\nCASE 3: disable Original VLAN usage and send with mirroring to Analyzer\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceAn, GT_FALSE,
                                             PRV_TGF_MIRROR_TO_ANALYZER_E);

    /* AUTODOC: CASE 4: enable <EPCL Key Ing Analyzer Use Orig VID> and send
       with mirroring to Analyzer */
    PRV_UTF_LOG0_MAC("\nCASE 4: enable Original VLAN usage and send with mirroring to Analyzer\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceAn, GT_TRUE,
                                             PRV_TGF_MIRROR_TO_ANALYZER_E);

    /* AUTODOC: disable mirroring on rx port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               GT_FALSE, PRV_TGF_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet error");

    /* AUTODOC: prepare configuration to trap packet to CPU */

    /* configure FDB entry to trap packets to CPU */
    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                     = GT_TRUE;
     /* select DA command - control (unconditionally trap to CPU) */
    macEntry.daCommand                    = PRV_TGF_PACKET_CMD_CNTL_E;
    macEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_IPCL_E_VLAN_CNS;
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet error");

    /* AUTODOC: CASE 5: disable <EPCL Key Ing Analyzer Use Orig VID>
       and send the packet to be trapped to CPU */
    PRV_UTF_LOG0_MAC("\nCASE 5: disable Original VLAN usage and send the packet to be trapped to CPU\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceRx, GT_FALSE,
                                             PRV_TGF_TRAP_TO_CPU_E);

   /* AUTODOC: CASE 6: enable <EPCL Key Ing Analyzer Use Orig VID>
      and send the packet to be trapped to CPU */
    PRV_UTF_LOG0_MAC("\nCASE 6: enable Original VLAN usage and send the packet to be trapped to CPU\n");
    rc = prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet error");

    prvTgfEgrPclEvlanBindingModeSendPacket();

    prvTgfEgrPclEvlanBindingModeCheckResults(&iface, &ifaceRx, GT_TRUE,
                                             PRV_TGF_TRAP_TO_CPU_E);

   /* disable capturing on the tx port */
   rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                        TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthCaptureSet error");

   /* disable capturing on the Analyzer port */
   rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ifaceAn,
                                        TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthCaptureSet error");
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfEgrPclEvlanBindingModeTest function
* @endinternal
*
* @brief   eVLAN-based binding mode Egress PCL Configuration Table Entry Selection.
*/
GT_VOID prvTgfEgrPclEvlanBindingModeTest
(
   GT_VOID
)
{
    prvTgfEgrPclEvlanBindingModeConfigurationSet();
    prvTgfEgrPclEvlanBindingModeTrafficGenerate();
    prvTgfEgrPclEvlanBindingModeConfigurationRestore();
}

