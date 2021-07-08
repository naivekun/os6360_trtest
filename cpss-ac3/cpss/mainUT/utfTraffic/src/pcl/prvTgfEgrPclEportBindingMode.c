/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfErgPclEportBindingMode.c
*
* DESCRIPTION:
*       Test ePort-based binding mode for Egress PCL Configuration Table
*       Entry Selection.
*
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
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
#include <common/tgfMirror.h>
#include <common/tgfCscdGen.h>
#include <pcl/prvTgfEgrPclEportBindingMode.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* calculate ePort number according to physical port number  */
#define PRV_TGF_EPORT_MAC(phyPort) ((phyPort) + (IS_AC3X_DEV_MAC(prvTgfDevNum)?512:_2K))

/* number of local physical ports checked for the traffic.
   Maximum used port index shouldn't exceed this value! */
#define PRV_TGF_ALL_PORTS_COUNT_CNS   4

/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1
#define PRV_TGF_CASCADE_PORT_IDX_CNS  2
#define PRV_TGF_ANALYZER_PORT_IDX_CNS 3
#define PRV_TGF_REMOTE_DEV_CNS        15
#define PRV_TGF_REMOTE_PORT_CNS       7

/* vlan for local traffic. */
#define PRV_TGF_VLAN_LOCAL_CNS   534

/* vlan for traffic to remote device. */
#define PRV_TGF_VLAN_REMOTE_CNS  619

/* analyzer index */
#define PRV_TGF_ANALYZER_IDX_CNS 2


#define PRV_TGF_RULES_COUNT_CNS  9

/* ePorts != physical port number will be used for this ports indexes.  */
static GT_U8 prvTgfIdxExArray[] = {PRV_TGF_TX_PORT_IDX_CNS,
                                   PRV_TGF_CASCADE_PORT_IDX_CNS,
                                   PRV_TGF_ANALYZER_PORT_IDX_CNS,
                                   PRV_TGF_REMOTE_PORT_CNS};

/* size of rvTgfIndexesExArray  */
#define PRV_TGF_IDX_EX_COUNT (sizeof prvTgfIdxExArray / sizeof prvTgfIdxExArray[0])

/* struct with some EPCL Lookup settings.*/
typedef struct
{
    GT_U32 label; /* is used to calculate a Configuration Table entry's index.
                    if label = 0, index = idx.
                    if label = 1, index = prvTgfPortsArray[idx]
                    if label = 2, index = ePort of prvTgfPortsArray[idx] */

    GT_U16 idx;
    GT_U16 pclId;            /* pclId, common for both a lookup Configuration
                                table entry and for a rule's key entry.*/
    GT_U16 newVlanId;        /* vlan tag0 to be set by the rule */
} PRV_TGF_LOOKUP_CFG_STC;

static PRV_TGF_LOOKUP_CFG_STC prvTgfCfgArray[PRV_TGF_RULES_COUNT_CNS] = {
    {1, PRV_TGF_TX_PORT_IDX_CNS,       1, 0x10},
    {2, PRV_TGF_TX_PORT_IDX_CNS,       2, 0x1E},
    {1, PRV_TGF_CASCADE_PORT_IDX_CNS,  3, 0x20},
    {2, PRV_TGF_CASCADE_PORT_IDX_CNS,  4, 0x2E},
    {1, PRV_TGF_ANALYZER_PORT_IDX_CNS, 5, 0x30},
    {2, PRV_TGF_ANALYZER_PORT_IDX_CNS, 6, 0x3E},
    {0, CPSS_CPU_PORT_NUM_CNS,         7, 0xC0},
    {0, PRV_TGF_REMOTE_PORT_CNS,       8, 0x70}};


/* prvTgfCaptArr_XXXX  - a packet's vlanId captured on ports. XXXX means:
   1st letter - U,M - Unicast/Multicast
   2nd letter - L,R - sent to local/remote device.
   3rd letter - L,R,_ - mirrored to local/remote device. "_" - wasn't mirrored.
   4rd letter - 0,1,_ - <assign Egr Attr locally> bit. "_" - don't care.
   prvTgfCaptArr_CPU - relevant to the packet mirrored (from rx port) to CPU.
*/
static GT_U16 prvTgfCaptArr_ULL_[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0x1E, 0,    0x1E};
static GT_U16 prvTgfCaptArr_ULR0[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0x1E, 0x1E, 0};
static GT_U16 prvTgfCaptArr_ULR1[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0x1E, 0x1E, 0};
static GT_U16 prvTgfCaptArr_UR_0[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0,    0x20, 0};
static GT_U16 prvTgfCaptArr_UR_1[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0,    0x70, 0};
static GT_U16 prvTgfCaptArr_MLR0[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0x10, 0x10, 0};
static GT_U16 prvTgfCaptArr_MLR1[PRV_TGF_ALL_PORTS_COUNT_CNS] = {0,    0x10, 0x10, 0};
static GT_U16 prvTgfCaptArr_CPU [PRV_TGF_ALL_PORTS_COUNT_CNS] = {0xC0, 0,    0,    0};

/* some original values saved to be restored at the end of the test. */
static GT_BOOL                                      prvTgfSavedTxMirrorModeEnabled;
static GT_U32                                       prvTgfSavedTxMirrorAnalyzerIdx;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC        prvTgfSavedAnalyzerIface;
static CPSS_CSCD_LINK_TYPE_STC                      prvTgfSavedCascadeLink;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT             prvTgfSavedSrcPortTrunkHashEn;
static CPSS_INTERFACE_INFO_STC                      prvTgfSavedEportPhyInfoArray[PRV_TGF_IDX_EX_COUNT];
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                      prvTgfSavedGlobalMirroringEn;
static GT_U32                                       prvTgfSavedGlobalAnalazyerIndex;
static GT_BOOL                                      prvTgfSavedCpuEpclEnable;
static PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT prvTgfSavedCpuEpclAccessMode;

/******************************* Test packets *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                   /* etherType */
    0, 0, PRV_TGF_VLAN_LOCAL_CNS                          /* pri, cfi, VlanId */
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
    PRV_TGF_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketPartArray                                       /* partsArray */
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;


/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/**************************** PRIVATE FUNCTIONS *******************************/



/**
* @internal prvTgfEgrPclEportBindingModeEgressPclEnable function
* @endinternal
*
* @brief   Enable/disable Egress PCL Engine per device.
*
* @param[in] enable                   - GT_TRUE to enable, GT_FALSE to disable
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEgrPclEportBindingModeEgressPclEnable
(
    IN GT_BOOL enable
)
{
    GT_STATUS                       rc;

    /* AUTODOC: Init PCL */
    if (GT_TRUE == enable)
    {
        rc = prvTgfPclInit();
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");
    }

    /* AUTODOC: Enables egress policy per device */
    rc = prvTgfPclEgressPolicyEnable(enable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPolicyEnable");

    return GT_OK;
}


/**
* @internal prvTgfEgressPclEportInit function
* @endinternal
*
* @brief   Enable/disable EPCL on the port and setup ePort-based access to
*         EPCL configuration table
* @param[in] portNum                  - physical port number
* @param[in] packetType               - packet type.
* @param[in] enable                   - GT_TRUE to enable, GT_FALSE to disable EPCL
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEgressPclEportInit
(
    IN GT_U32                            portNum,
    IN PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT  packetType,
    IN GT_BOOL                          enable
)
{
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    GT_STATUS                       rc;

    TGF_RESET_PARAM_MAC(interfaceInfo);

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, portNum,
                                          packetType, enable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(portNum,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
    return GT_OK;
}

/**
* @internal prvTgfEgrPclEportBindingModeSetupRules function
* @endinternal
*
* @brief   setup/invalidate EPCL rules
*
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate rules
*                                       None
*/
static GT_VOID prvTgfEgrPclEportBindingModeSetupRules
(
    IN GT_BOOL enable
)
{
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;      /* rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;   /* rule pattern */
    PRV_TGF_PCL_ACTION_STC      action;    /* rule aciton */
    GT_U32                      i;
    GT_STATUS                   rc;
    TGF_RESET_PARAM_MAC(mask);
    TGF_RESET_PARAM_MAC(pattern);
    TGF_RESET_PARAM_MAC(action);

    if (GT_FALSE == enable)
    {
        for (i=0; i < PRV_TGF_RULES_COUNT_CNS; ++i)
        {
            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, i,
                                             GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfPclRuleValidStatusSet: %d", i);
        }
    }
    else
    {
        /* mask for MAC SA */
        cpssOsMemSet(mask.ruleEgrStdNotIp.macSa.arEther, 0xFF,
                     sizeof(mask.ruleEgrStdNotIp.macSa.arEther));

        /* pattern for MAC SA */
        cpssOsMemCpy(pattern.ruleEgrStdNotIp.macSa.arEther,
                    prvTgfPacketL2Part.saMac, sizeof(prvTgfPacketL2Part.saMac));

        mask.ruleEgrStdNotIp.common.pclId = 0x3FF;

        /* action - set vlan tag0 */
        action.egressPolicy          = GT_TRUE;
        action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;
        action.vlan.egressVlanId0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
        for (i=0; i<PRV_TGF_RULES_COUNT_CNS; ++i)
        {
            /* pclId should be the same as lookupCfg.pclId above */
            pattern.ruleEgrStdNotIp.common.pclId = prvTgfCfgArray[i].pclId;
            action.vlan.vlanId = prvTgfCfgArray[i].newVlanId;

            rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                                  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(i), &mask, &pattern, &action);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                   "prvTgfPclRuleSet failed, rule index=%d", prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(i));
        }
    }
}

/**
* @internal prvTgfEgrPclEportBindingModeSetupEpcl function
* @endinternal
*
* @brief   Activate/deactivate EPCL and create/invalidate a rules to modify
*         packet's tag0 vlanId.
* @param[in] enable                   - GT_TRUE to activate EPCL, GT_FALSE - to deactivate
*
* @retval GT_OK                    - on success
*/
static GT_VOID prvTgfEgrPclEportBindingModeSetupEpcl
(
    IN GT_BOOL enable
)
{

    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg; /* PCL Configuration Table Entry */
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    GT_U32                      i;
    GT_STATUS                   rc;

    TGF_RESET_PARAM_MAC(lookupCfg);
    TGF_RESET_PARAM_MAC(interfaceInfo);

    /* enable EPCL globally */
    rc = prvTgfEgrPclEportBindingModeEgressPclEnable(enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfEgrPclEportBindingModeEgressPclEnable");

    /* enable EPCL on a tx port */
    rc = prvTgfEgressPclEportInit(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                  PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEportInit");

    /* enable EPCL on a Cascade port for forwarded packets */
    rc = prvTgfEgressPclEportInit(prvTgfPortsArray[PRV_TGF_CASCADE_PORT_IDX_CNS],
                                  PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEportInit");

    /* enable EPCL on a Cascade port for mirrored packets */
    rc = prvTgfEgressPclEportInit(prvTgfPortsArray[PRV_TGF_CASCADE_PORT_IDX_CNS],
                                  PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEportInit");

    /* enable EPCL on an Analyzer port */
    rc = prvTgfEgressPclEportInit(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                  PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEportInit");

    /* AUTODOC: setup EPCL configuration table entries. */
    lookupCfg.enableLookup           = enable;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    interfaceInfo.type = CPSS_INTERFACE_INDEX_E;

    for (i = 0; i < PRV_TGF_RULES_COUNT_CNS; ++i)
    {
        /* set an entry's index in Configuration Table */
        if (0 == prvTgfCfgArray[i].label)
        {
            interfaceInfo.index = prvTgfCfgArray[i].idx;
        }
        else if (1 == prvTgfCfgArray[i].label)
        {
            interfaceInfo.index = prvTgfPortsArray[prvTgfCfgArray[i].idx];
        }
        else
        {
            interfaceInfo.index = PRV_TGF_EPORT_MAC(
                                       prvTgfPortsArray[prvTgfCfgArray[i].idx]);
        }
        interfaceInfo.index &= 0xFFF;
        lookupCfg.pclId     = prvTgfCfgArray[i].pclId;
        /* write an entry into the EPCL configuration table */
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    /* AUTODOC: setup EPCL rules. */
    prvTgfEgrPclEportBindingModeSetupRules(enable);
}


/**
* @internal prvTgfEgrPclEportBindingModeConfigSave function
* @endinternal
*
* @brief   Save values that should be restored at the end of the test
*/
static GT_VOID prvTgfEgrPclEportBindingModeConfigSave
(
    GT_VOID
)
{
    GT_U32    i;
    GT_STATUS rc;
    GT_PORT_NUM ePort;


    /* AUTODOC: save CPU's EPCL settings */
    rc = prvTgfPclEgressPclPacketTypesGet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, &prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeGet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeGet");

    /* AUTODOC: save forwaringd mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeGet");

    /* AUTODOC: save global Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfSavedGlobalAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save a mirroring status of tx port */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                               &prvTgfSavedTxMirrorModeEnabled,
                               &prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortGet error");

    /* AUTODOC: save an old Analyzer bound to index PRV_TGF_ANALYZER_IDX_CNS  */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          PRV_TGF_ANALYZER_IDX_CNS,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save used ePorts info and mapping to physical ports. */
    for (i = 0; i< PRV_TGF_IDX_EX_COUNT; i++)
    {
        ePort = PRV_TGF_EPORT_MAC(prvTgfPortsArray[prvTgfIdxExArray[i]]);

        prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum, ePort,
                                              &prvTgfSavedEportPhyInfoArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d",
                        ePort);
    }

    /* AUTODOC: save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                                  PRV_TGF_REMOTE_PORT_CNS,
                                  &prvTgfSavedCascadeLink,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");
}

/**
* @internal prvTgfEgrPclEportBindingModeConfigurationSet function
* @endinternal
*
* @brief   setup a base configuration
*/
static GT_VOID prvTgfEgrPclEportBindingModeConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC iface;
    CPSS_CSCD_LINK_TYPE_STC               cascadeLink;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC      defEgressInfo;
    CPSS_INTERFACE_INFO_STC               physicalInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC             macEntry;
    GT_PORT_NUM                           ePort;

    /* fields used to read vlan info by prvTgfBrgVlanEntryRead ()*/
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    GT_U32                                i;
    GT_STATUS                             rc;
    /* ports that will be placed in the same vlan. Will be filled later */
    GT_U32 portsArr[2];
    /* tags of ports. Relevant to values in portsArr, portsRemoteArr */
    GT_U8 tagsArr[2]                     = {1,1};
    TGF_RESET_PARAM_MAC(iface);
    TGF_RESET_PARAM_MAC(cascadeLink);
    TGF_RESET_PARAM_MAC(defEgressInfo);
    TGF_RESET_PARAM_MAC(physicalInfo);
    TGF_RESET_PARAM_MAC(macEntry);
    TGF_RESET_PARAM_MAC(ePort);
    TGF_RESET_PARAM_MAC(portsMembers);
    TGF_RESET_PARAM_MAC(portsTagging);
    TGF_RESET_PARAM_MAC(vlanInfo);
    TGF_RESET_PARAM_MAC(portsTaggingCmd);

    prvTgfEgrPclEportBindingModeConfigSave();

    /*-------------------------SETUP CONFIGURATION----------------------------*/

    /*------------------SETUP FORWARDING MODE---------------------------------*/
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
                            PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
                                                      PRV_TGF_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

    /*-------------------------CREATE VLANS-----------------------------------*/
    /* AUTODOC:  create vlan for local traffic with tagged rx, tx ports. */
    portsArr[0] = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    portsArr[1] = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLAN_LOCAL_CNS,
                                           portsArr, NULL, tagsArr, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC:  create vlan for remote traffic with tagged rx, cascade port. */
    portsArr[1] = prvTgfPortsArray[PRV_TGF_CASCADE_PORT_IDX_CNS];

    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLAN_REMOTE_CNS,
                                           portsArr, NULL, tagsArr, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: set <Flood eVIDX Mode> of "local" vlan to apply <Flood eVIDX>
       for ALL flooded traffic. To be sure that packet will be sent to all
       vlan's ports */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_VLAN_LOCAL_CNS,
                                &portsMembers, &portsTagging, &isValid,
                                &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

    vlanInfo.floodVidxMode = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_VLAN_LOCAL_CNS,
                                &portsMembers, &portsTagging,
                                &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite");


    /*------------------------CREATE AN FDB ENTRIES---------------------------*/

    /* AUTODOC: configure an FDB entry to sent packet in vlan 5 to tx-port */

    /* commont part used by two FDB entries*/
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_VLAN_LOCAL_CNS;
    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                     = GT_TRUE;
    macEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum =
                   PRV_TGF_EPORT_MAC(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d",
                                 PRV_TGF_VLAN_LOCAL_CNS);

    /* AUTODOC: set an FDB entry to send traffic in vlan 6 to remote device */
    macEntry.dstInterface.devPort.hwDevNum  = PRV_TGF_REMOTE_DEV_CNS;
    macEntry.dstInterface.devPort.portNum = PRV_TGF_REMOTE_PORT_CNS;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_VLAN_REMOTE_CNS;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d",
                                 PRV_TGF_VLAN_REMOTE_CNS);

    /* AUTODOC: setup tx-port(1) to be mirrored. */

    /*3rd argument is ignored in hop-by-hop forwarding mode */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                               GT_TRUE, PRV_TGF_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d ",
                                 prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* AUTODOC: setup ePorts info (all 0) and map ePorts to physical ports. */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;

    for (i = 0; i< PRV_TGF_IDX_EX_COUNT; i++)
    {
        ePort = PRV_TGF_EPORT_MAC(prvTgfPortsArray[prvTgfIdxExArray[i]]);
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum, ePort,
                                                           &defEgressInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                   "prvTgfBrgEportToPhysicalPortEgressPortInfoSet: %d", ePort);

        physicalInfo.devPort.portNum = prvTgfPortsArray[prvTgfIdxExArray[i]];

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, ePort,
                                                               &physicalInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", ePort);

    }

    /* AUTODOC: enable EPCL with ePort-based binding mode on ports 1, 2, 3 */
    prvTgfEgrPclEportBindingModeSetupEpcl(GT_TRUE);
}


/**
* @internal prvTgfEgrPclEportBindingModeSendPacket function
* @endinternal
*
* @brief   Send packet
*/
static GT_VOID prvTgfEgrPclEportBindingModeSendPacket
(
    GT_VOID
)
{
    GT_U32                          portIter;
    GT_STATUS                       rc;


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
* @internal prvTgfEgrPclEportBindingModeCheckPacketOnPort function
* @endinternal
*
* @brief   Check packet's vlan Id tag captured on the port
*
* @param[in] ifacePtr                 -  (pointer to) egress interface.
* @param[in] expVlanId                -  expected vlan id.
*                                       None
*/
static GT_VOID prvTgfEgrPclEportBindingModeCheckPacketOnPort
(
    IN CPSS_INTERFACE_INFO_STC         *ifacePtr,
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
    packetType = (prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] ==
                  ifacePtr->devPort.portNum) ?
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
                                 "Unexpected vlanId on port %d. Expected 0x%X",
                                         ifacePtr->devPort.portNum, expVlanId);
        }
    }
    while (GT_TRUE);

    rc = (GT_NO_MORE == rc) ? GT_OK : rc;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                              "prvTgfIpv4Uc2MultipleVrCheckCountersAfterBurst");

    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, packetsCount,
                                 "Captured %d packets on port %d",
                                 packetsCount, ifacePtr->devPort.portNum);
}


/**
* @internal prvTgfEgrPclEportBindingModeCheckResults function
* @endinternal
*
* @brief   Check counters on ports and MAC DA of the packet after transmitting.
*
* @param[in] vlansArray[PRV_TGF_ALL_PORTS_COUNT_CNS] - vlanArray[i] - vlanId received on a port/
*                                      if 0, a packet shouldn't be received on this port.
*                                      Last element is vlanId of packet, rx-mirrored to CPU.
*                                       None
*/
static GT_VOID prvTgfEgrPclEportBindingModeCheckResults
(
    GT_U16 vlansArray[PRV_TGF_ALL_PORTS_COUNT_CNS]
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_U32     expRx;          /* expected rx packet counter value */
    GT_U32     expTx;          /* expected tx packet counter value */
    GT_U32     i;
    GT_STATUS  rc;

    TGF_RESET_PARAM_MAC(iface);

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < PRV_TGF_ALL_PORTS_COUNT_CNS; ++i)
    {
        expTx = (0 == vlansArray[i] && i != PRV_TGF_RX_PORT_IDX_CNS) ? 0 : 1;
        expRx = expTx;
        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRx, expTx, PRV_TGF_PACKET_SIZE_CNS, 1);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d Expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRx, expTx);
    }

    /* AUTODOC: Check trigger counters */
    iface.type                = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum    = prvTgfDevNum;

    for (i = 0; i < PRV_TGF_ALL_PORTS_COUNT_CNS; ++i)
    {
        if  (0 != vlansArray[i])
        {
            iface.devPort.portNum = prvTgfPortsArray[i];
            prvTgfEgrPclEportBindingModeCheckPacketOnPort(&iface, vlansArray[i]);
        }
    }
}

#define DEBUG_SPECIFIC_CASE_DISABLED    0
/* allow to debug specific case , when not '0' need to be 1..8 for the 'CASE' that need to be tested */
/* traffic of other cases bypassed  */
static GT_U32   allowDebug_prvTgfEgrPclEportBindingMode = DEBUG_SPECIFIC_CASE_DISABLED;
/* allow to debug 'specific case' */
void allowDebug_prvTgfEgrPclEportBindingMode_Set(GT_U32 debugCase)
{
    allowDebug_prvTgfEgrPclEportBindingMode = debugCase;
}

/**
* @internal prvTgfEgrPclEportBindingModeTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
static GT_VOID prvTgfEgrPclEportBindingModeTrafficGenerate
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  ifaceMir;
    CPSS_INTERFACE_INFO_STC                iface;
    CPSS_CSCD_LINK_TYPE_STC                cascadeLink;
    PRV_TGF_BRG_MAC_ENTRY_STC              macEntry;
    GT_U32                                 i;
    GT_STATUS                              rc;

    TGF_RESET_PARAM_MAC(macEntry);
    TGF_RESET_PARAM_MAC(ifaceMir);
    TGF_RESET_PARAM_MAC(iface);
    TGF_RESET_PARAM_MAC(cascadeLink);

    /* AUTODOC: generate traffic */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* AUTODOC: enable packet capturing on all ports except rx. */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;

    for (i=0; i < PRV_TGF_ALL_PORTS_COUNT_CNS; ++i)
    {
        if (i != PRV_TGF_RX_PORT_IDX_CNS)
        {
            iface.devPort.portNum  = prvTgfPortsArray[i];
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                               TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                   "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                   iface.devPort.portNum);
        }
    }

    /**************************************************************************/
    /* AUTODOC: CASE 1. Unicast, send to local ePort, mirror to local ePort */
    PRV_UTF_LOG0_MAC("\nCASE 1. Unicast, send to local, mirror to local\n");

    /* setup Analyzer index 0 to be an Analyzer ePort */
    ifaceMir.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMir.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceMir.interface.devPort.portNum  =
             PRV_TGF_EPORT_MAC(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    rc=prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS, &ifaceMir);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                ifaceMir.interface.devPort.portNum);

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 1)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_ULL_);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 2. Unicast, send to local ePort, mirror to remote device,
                        <assign Egress attributes locally> = 0 */
    PRV_UTF_LOG0_MAC("\nCASE 2. Unicast, send to local, mirror to remote. <assign Egr attr locally> = 0\n");

    /* setup Analyzer index 0 to be a remote port */
    ifaceMir.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMir.interface.devPort.hwDevNum = PRV_TGF_REMOTE_DEV_CNS;
    ifaceMir.interface.devPort.portNum  = PRV_TGF_REMOTE_PORT_CNS;

    rc=prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS, &ifaceMir);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                ifaceMir.interface.devPort.portNum);

    /* configure Cascade port */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_CASCADE_PORT_IDX_CNS];

    /* set <assign Egress attributes locally> = 0 */
    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");


    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 2)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_ULR0);
    }

    /**************************************************************************/
    /* AUTODOC: CASE 3. Unicast, send to local ePort, mirror to remote device,
                        <assign Egress attributes locally> = 1 */
    PRV_UTF_LOG0_MAC("\nCASE 3. Unicast, send to local, mirror to remote. <assign Egr attr locally> = 1\n");

    /* set <assign Egress attributes locally> = 1 */
    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 3)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_ULR1);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 4. Unicast, send to remote device,
                     <assign Egress attributes locally> = 0 */
    PRV_UTF_LOG0_MAC("\nCASE 4. Unicast, send to remote, <assign Egr attr locally> = 0\n");
    /* set vlanId = 6 in the packet's body. */
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLAN_REMOTE_CNS;

    /* set <assign Egress attributes locally> = 0 */
    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                           PRV_TGF_REMOTE_PORT_CNS,
                           &cascadeLink, 0, GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 4)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_UR_0);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 5. Unicast, send to remote device,
                        <assign Egress attributes locally> = 1 */
    PRV_UTF_LOG0_MAC("\nCASE 5. Unicast, send to remote, <assign Egr attr locally> = 1\n");

    /* set <assign Egress attributes locally> = 1 */
    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 5)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_UR_1);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 6. Multicast, mirror to remote device
       <assign Egress attributes locally> = 0 */
    PRV_UTF_LOG0_MAC("\nCASE 6. Multicast, mirror to remote, <assign Egr attr locally> = 0\n");

    /* set vlanId = PRV_TGF_VLAN_LOCAL_CNS in the packet's body again. */
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLAN_LOCAL_CNS;

    /* clear FDB to make the packet an unregistered. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* mirror index 0 is assigned to remote device in previos case */
    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 6)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_MLR0);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 7. Multicast, mirror to remote port,
                        <assign Egress attributes locally> = 1 */
    PRV_UTF_LOG0_MAC("\nCASE 7. Multicast, mirror to remote, <assign Egr attr locally> = 1\n");

    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 7)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_MLR1);
    }


    /**************************************************************************/
    /* AUTODOC: CASE 8. send UC packet to CPU. */

    PRV_UTF_LOG0_MAC("\nCASE 8. Unicast, ingress mirrored to CPU\n");
    /* enable EPCL on the CPU port */
    rc = prvTgfEgressPclEportInit(CPSS_CPU_PORT_NUM_CNS,
                                  PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEgressPclEportInit");

    /* configure FDB entry to trap packets to CPU */
    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                     = GT_TRUE;
    macEntry.daCommand                    = PRV_TGF_PACKET_CMD_CNTL_E;
    macEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_VLAN_LOCAL_CNS;
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet error");

    if(allowDebug_prvTgfEgrPclEportBindingMode == DEBUG_SPECIFIC_CASE_DISABLED ||
       allowDebug_prvTgfEgrPclEportBindingMode == 8)
    {
        prvTgfEgrPclEportBindingModeSendPacket();
        prvTgfEgrPclEportBindingModeCheckResults(prvTgfCaptArr_CPU);
    }
}


/**
* @internal prvTgfEgrPclEportBindingModeConfigurationRestore function
* @endinternal
*
* @brief   restore configuration
*/
static GT_VOID prvTgfEgrPclEportBindingModeConfigurationRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_U32                  i;
    GT_PORT_NUM             ePort;
    GT_STATUS               rc;

    TGF_RESET_PARAM_MAC(iface);
    /* AUTODOC: RESTORE CONFIGURATION */
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLAN_LOCAL_CNS;

    /* AUTODOC: restore CPU's EPCL settings */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: restore forwaringd mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore global Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(
                prvTgfSavedGlobalMirroringEn, prvTgfSavedGlobalAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore a mirroring status of tx port */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                               prvTgfSavedTxMirrorModeEnabled,
                               prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet error");

    /* AUTODOC: restore an old Analyzer bound to index PRV_TGF_ANALYZER_IDX_CNS */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore used ePorts info and mapping to physical ports. */
    for (i = 0; i< PRV_TGF_IDX_EX_COUNT; i++)
    {
        ePort = PRV_TGF_EPORT_MAC(prvTgfPortsArray[prvTgfIdxExArray[i]]);

        prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, ePort,
                                              &prvTgfSavedEportPhyInfoArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", ePort);
    }

    /* AUTODOC: restore the current cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                                  PRV_TGF_REMOTE_PORT_CNS,
                                  &prvTgfSavedCascadeLink,
                                  prvTgfSavedSrcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* AUTODOC : disable EPCL configurations */
    prvTgfEgrPclEportBindingModeSetupEpcl(GT_FALSE);

    /* AUTODOC: disable EPCL globally */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable");

    /* AUTODOC: clear FDB */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate used vlan entries */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_LOCAL_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLAN_LOCAL_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_REMOTE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLAN_REMOTE_CNS);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable packet capturing on all ports except rx. */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;

    for (i=0; i < PRV_TGF_ALL_PORTS_COUNT_CNS; ++i)
    {
        if (i != PRV_TGF_RX_PORT_IDX_CNS)
        {
            iface.devPort.portNum  = prvTgfPortsArray[i];
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                   "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                   iface.devPort.portNum);
        }
    }
}


/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfEgrPclEportBindingModeTest function
* @endinternal
*
* @brief   Test ePort-based binding mode for Egress PCL Configuration Table
*         Entry Selection.
*/
GT_VOID prvTgfEgrPclEportBindingModeTest
(
    GT_VOID
)
{
    prvTgfEgrPclEportBindingModeConfigurationSet();
    prvTgfEgrPclEportBindingModeTrafficGenerate();
    prvTgfEgrPclEportBindingModeConfigurationRestore();
}

