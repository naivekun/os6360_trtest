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
* @file prvTgfExactMatchTtiPclFullPath.c
*
* @brief Test Exact Match functionality with TTI and PCL configuration
*
* @version 1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>

#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS  4

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL                                           profileIdEn;
    GT_U32                                            profileId;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT                    firstLookupClientType;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC        keyParams;

    GT_BOOL                                           exactMatchEntryIndex;
    GT_BOOL                                           exactMatchEntryValid;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT               exactMatchEntryActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                    exactMatchEntryAction;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                     exactMatchEntry;
    GT_U32                                            exactMatchEntryExpandedActionIndex;

    GT_BOOL                                           exactMatchEntryIndex2;
    GT_BOOL                                           exactMatchEntryValid2;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT               exactMatchEntryActionType2;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                    exactMatchEntryAction2;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                     exactMatchEntry2;
    GT_U32                                            exactMatchEntryExpandedActionIndex2;

    GT_BOOL                                           exactMatchActivityEnable;

} prvTgfExactMatchRestoreCfg;

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                   5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL ID */
#define PRV_TGF_PCL_CONFIG_INDEX_CNS         0x55
#define PRV_TGF_PCL_ID_CNS                   0x66

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;


/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,         /* totalLen   */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

#define PRV_TGF_CPU_PORT_IDX_CNS        3

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 1

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};
/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* max number for UDB */
#define PRV_TGF_TTI_MAX_UDB_CNS 30

static struct {
    PRV_TGF_TTI_KEY_SIZE_ENT  keyTypeSize[20];
} prvTgfRestoreCfg;

static CPSS_PACKET_CMD_ENT pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_DROP_HARD_E;
static GT_BOOL reducedUsedForExactMatchEntry = GT_FALSE;

/* default vlanid */
static GT_U16        prvTgfDefVlanId  =   1;

static GT_U32   currentRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        currentRuleIndex =  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_PCL_RULE_IDX_CNS); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        currentRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_PCL_RULE_IDX_CNS);
    }
}

/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet function
* @endinternal
*
* @brief   Set TTI Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc=GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT    keyType;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    /* AUTODOC: Set Vlan And FDB configuration */

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet");

    /* Set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* TTI configuration */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    for(keyType = PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E; keyType <= PRV_TGF_TTI_KEY_UDB_UDE6_E; keyType++)
    {
        /* enable the TTI lookup for all keyType  */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],keyType, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* AUTODOC: set UDB byte for all packet types anchor Metadata */
        /* AUTODOC: set key size for all packet types  */

        /* set UDB byte for all packet types anchor Metadata  */
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         keyType,                        /* keyType    */
                                         0,                              /* udbIndex   */
                                         PRV_TGF_TTI_OFFSET_METADATA_E,  /* offsetType */
                                         24);                            /* offset - eVLAN LSB [7:0] */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         keyType,                        /* keyType    */
                                         1,                              /* udbIndex   */
                                         PRV_TGF_TTI_OFFSET_METADATA_E,  /* offsetType */
                                         25);                            /* offset - eVLAN MSB [12:8] */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         keyType,                        /* keyType    */
                                         2,                              /* udbIndex   */
                                         PRV_TGF_TTI_OFFSET_METADATA_E,  /* offsetType */
                                         26);                            /* local device ePort/TrunkId [7:0] */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         keyType,                        /* keyType    */
                                         3,                              /* udbIndex   */
                                         PRV_TGF_TTI_OFFSET_METADATA_E,  /* offsetType */
                                         27);                            /* local device ePort/TrunkId [12:8] */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

        /* save for restore - key size for all packet types */
        rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum, keyType, &prvTgfRestoreCfg.keyTypeSize[keyType]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

        /* set key size for all packet types */
        rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, PRV_TGF_TTI_KEY_SIZE_10_B_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);
    }


    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0xFF, sizeof(ttiMask));/* full mask for first stage */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* TTI rule action */
    ttiAction.pcl0OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
    ttiAction.iPclConfigIndex = PRV_TGF_PCL_CONFIG_INDEX_CNS;
    ttiAction.iPclUdbConfigTableEnable = GT_TRUE;
    ttiAction.iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE6_E;

    /* TTI pattern */
    ttiPattern.udbArray[0]=(PRV_TGF_VLANID_CNS&0xFF);/* eVLAN LSB [7:0] */
    ttiPattern.udbArray[1]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    ttiPattern.udbArray[2]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* local device ePort/TrunkId [7:0]  */
    ttiPattern.udbArray[3]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* local device ePort/TrunkId [12:8] */

    keyType = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_RULE_IDX_CNS,
                             PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern,
                             &ttiMask,
                             &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: set default TTI rule with DROP command */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.command = CPSS_PACKET_CMD_DROP_HARD_E;
    ttiAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 2;

    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_RULE_IDX_CNS+1,
                             PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern,
                             &ttiMask,
                             &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: validate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS+1, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    return;
}

/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc=GT_OK;
    /*GT_U32                      ii;*/
    PRV_TGF_TTI_KEY_TYPE_ENT    keyType;

    /* TTI configuration */
    for(keyType = PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E; keyType <= PRV_TGF_TTI_KEY_UDB_UDE6_E; keyType++)
    {
        /* disable the TTI lookup for all key types */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],keyType, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* restore key size for all packet types */
        rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, prvTgfRestoreCfg.keyTypeSize[keyType]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS+1, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* Vlan restore */

    /* Restore default PVID on port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");
}

/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet function
* @endinternal
*
* @brief   Set PCL Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_LOOKUP_CFG_STC          lookupCfg;
    CPSS_INTERFACE_INFO_STC             interfaceInfo;

    GT_U32                              udbAmount;
    GT_U32                              udbOffsetBase;
    GT_U32                              udbOffsetIncrement;
    GT_U32                              udbIndexBase;

    GT_U8                               offset;
    GT_U32                              udbIndex;
    GT_U32                              i;

    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;


    /*****Ingress PCL Configuration **************************************/

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* Enabling IPCL for port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Set PCL0 configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index           = PRV_TGF_PCL_CONFIG_INDEX_CNS;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = PRV_TGF_PCL_ID_CNS;
    lookupCfg.exactMatchLookupSubProfileId = 0;
    lookupCfg.groupKeyTypes.ipv4Key = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    lookupCfg.groupKeyTypes.ipv6Key = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo,
                            CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E,
                            &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* PCL UDBs configuration */
    /*
     { udbIndex ,   offsetType,                     offset
     { 0,           PRV_TGF_PCL_OFFSET_L2_E,      0 },  Dst Mac MSB
     { 1,           PRV_TGF_PCL_OFFSET_L2_E,      1 },  Dst Mac byte 1
     { 2,           PRV_TGF_PCL_OFFSET_L2_E,      2 },  Dst Mac byte 2
     { 3,           PRV_TGF_PCL_OFFSET_L2_E,      3 },  Dst Mac byte 3
     { 4,           PRV_TGF_PCL_OFFSET_L2_E,      4 },  Dst Mac byte 4
     { 5,           PRV_TGF_PCL_OFFSET_L2_E,      5 }   Dst Mac LSB */

    udbAmount = 20;
    udbOffsetBase = 0;
    udbOffsetIncrement = 1;
    udbIndexBase = 0;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                         PRV_TGF_PCL_PACKET_TYPE_UDE6_E,
                                         CPSS_PCL_DIRECTION_INGRESS_E,
                                         udbIndex,
                                         PRV_TGF_PCL_OFFSET_L2_E,
                                         offset);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet");
    }

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    udbSelect.ingrUdbReplaceArr[0]=GT_TRUE;/* UDB0  replaced with {PCL-ID[7:0]}                     */
    udbSelect.ingrUdbReplaceArr[1]=GT_TRUE;/* UDB1  replaced with {UDB Valid,reserved,PCL-ID[9:8]}  */
    udbSelect.ingrUdbReplaceArr[2]=GT_TRUE;/* UDB2  replaced with eVLAN[7:0]                        */
    udbSelect.ingrUdbReplaceArr[3]=GT_TRUE;/* UDB3  replaced with eVLAN[12:8]                       */
    udbSelect.ingrUdbReplaceArr[4]=GT_TRUE;/* UDB4  replaced with Source-ePort[7:0]                 */
    udbSelect.ingrUdbReplaceArr[5]=GT_TRUE;/* UDB5  replaced with Source-ePort[12:8]                */
    udbSelect.udbSelectArr[6]=0;/* Dst Mac byte MSB */
    udbSelect.udbSelectArr[7]=1;/* Dst Mac byte 1 */
    udbSelect.udbSelectArr[8]=2;/* Dst Mac byte 2 */
    udbSelect.udbSelectArr[9]=3;/* Dst Mac byte 3 */
    udbSelect.udbSelectArr[10]=4;/* Dst Mac byte 4 */
    udbSelect.udbSelectArr[11]=5;/* Dst Mac byte LSB */
    udbSelect.egrUdb01Replace=GT_FALSE;
    udbSelect.egrUdbBit15Replace=GT_FALSE;

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,/* ruleFormat */
                                            PRV_TGF_PCL_PACKET_TYPE_UDE6_E,             /* packetType*/
                                            CPSS_PCL_LOOKUP_0_E,                        /* lookupNum */
                                            &udbSelect);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet");

}

/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore function
* @endinternal
*
* @brief   Restore PCL Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                           rc=GT_OK;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    GT_U32                              udbAmount;
    GT_U32                              udbOffsetBase;
    GT_U32                              udbOffsetIncrement;
    GT_U32                              udbIndexBase;
     GT_U8                              offset;
    GT_U32                              udbIndex;
    GT_U32                              i;

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

     /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);


    /* AUTODOC: restore UDB CFG table */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                0,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    udbAmount = 20;
    udbOffsetBase = 0;
    udbOffsetIncrement = 1;
    udbIndexBase = 0;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                         PRV_TGF_PCL_PACKET_TYPE_UDE6_E,
                                         CPSS_PCL_DIRECTION_INGRESS_E,
                                         udbIndex,
                                         PRV_TGF_PCL_OFFSET_INVALID_E,
                                         offset);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet");
    }


    /* Reset UDB Only PCL Key UDB selection */
    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,/* ruleFormat */
                                            PRV_TGF_PCL_PACKET_TYPE_UDE6_E,          /* packetType */
                                            CPSS_PCL_LOOKUP_0_E,                     /* lookupNum  */
                                            &udbSelect);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet");


}

/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateRule
*           function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateRule
(
    GT_VOID
)
{
    GT_STATUS                      rc=GT_OK;
    CPSS_PCL_RULE_SIZE_ENT         ruleSize;
    GT_U32                         ruleIndex;
    GT_BOOL                        valid;

    setRuleIndex(CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_0_0_E);

    ruleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E;
    ruleIndex = currentRuleIndex;
    valid = GT_FALSE;

    /* invalidate the rule configured in setRuleIndex  */
    rc =  prvTgfPclRuleValidStatusSet(ruleSize,ruleIndex,valid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
}
/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS                                       rc=GT_OK;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT                          direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT                     pclPacketType;
    GT_U32                                          subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  exactMatchLookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;

    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;

    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];

    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;

    /* AUTODOC: keep exactMatchActivityEnable configured - for restore */
    rc =  prvTgfExactMatchActivityBitEnableGet(prvTgfDevNum,&prvTgfExactMatchRestoreCfg.exactMatchActivityEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set exactMatchActivityEnable */
    rc =  prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_UDE6_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdGet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         &prvTgfExactMatchRestoreCfg.profileIdEn , &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId=5 for PCL first lookup */
    enableExactMatchLookup = GT_TRUE;
    profileId = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId,
                                         pclLookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    switch(pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code'
           so relevant for drops too. */
        actionData.pclAction.mirror.cpuCode= PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        reducedUsedForExactMatchEntry=GT_FALSE;
        break;
    case CPSS_PACKET_CMD_FORWARD_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        actionData.pclAction.bypassBridge = GT_TRUE;
        actionData.pclAction.bypassIngressPipe = GT_TRUE;
        actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
        reducedUsedForExactMatchEntry=GT_FALSE;
        break;
    default:
        rc=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    actionData.pclAction.egressPolicy = GT_FALSE;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Expanded Action for PCL lookup, profileId=5, ActionType=PCL, packet Command = DROP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    keyParams.keyStart=0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;  /* {PCL-ID[7:0]}                             */
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}          */
    pattern.ruleIngrUdbOnly.udb[2]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;   /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[3] = 0x1F;
    mask.ruleIngrUdbOnly.udb[5] = 0x1F;
    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));
    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */

    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;

    /* here we check exactly the same pattern set for the PCL match in TCAM */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-2];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex=index;

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
    return;
}

/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathReducedConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS                                       rc=GT_OK;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT                          direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT                     pclPacketType;
    GT_U32                                          subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  exactMatchLookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;

    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;

    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];

    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;

    /* AUTODOC: keep exactMatchActivityEnable configured - for restore */
    rc =  prvTgfExactMatchActivityBitEnableGet(prvTgfDevNum,&prvTgfExactMatchRestoreCfg.exactMatchActivityEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set exactMatchActivityEnable */
    rc =  prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_UDE6_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdGet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         &prvTgfExactMatchRestoreCfg.profileIdEn , &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId=5 for PCL first lookup */
    enableExactMatchLookup = GT_TRUE;
    profileId = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId,
                                         pclLookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    switch(pktCmd)
    {
    case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code'
           so relevant for drops too. */
        actionData.pclAction.mirror.cpuCode= PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        reducedUsedForExactMatchEntry=GT_FALSE;
        break;
    case CPSS_PACKET_CMD_FORWARD_E:
        actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        actionData.pclAction.bypassBridge = GT_TRUE;
        actionData.pclAction.bypassIngressPipe = GT_TRUE;
        actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
        pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
        reducedUsedForExactMatchEntry=GT_FALSE;

        /* the outInterface should be taken from reduced entry */
        expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;

        break;
    default:
        rc=GT_NOT_SUPPORTED;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchTtiPclFullPathConfigSet FAILED pktCmd not supported in the test : %d", pktCmd);
        break;
    }

    actionData.pclAction.egressPolicy = GT_FALSE;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Expanded Action for PCL lookup, profileId=5, ActionType=PCL, packet Command = DROP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    keyParams.keyStart=0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;  /* {PCL-ID[7:0]}                             */
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}          */
    pattern.ruleIngrUdbOnly.udb[2]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;   /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[3] = 0x1F;
    mask.ruleIngrUdbOnly.udb[5] = 0x1F;

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));
    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */

    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;

    /* here we check exactly the same pattern set for the PCL match in TCAM */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-2];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex=index;

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
    if(pktCmd==CPSS_PACKET_CMD_FORWARD_E)
    {
        reducedUsedForExactMatchEntry=GT_TRUE;
        actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
        actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    }
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);


    return;
}
/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be != 0
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet
(
     GT_VOID
)
{
    GT_STATUS                                       rc=GT_OK;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;


     /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    keyParams.keyStart=1;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    /*pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS; keyParams.keyStart=1; so this UDB will not be used in EM key*//* {PCL-ID[7:0]}*/
    pattern.ruleIngrUdbOnly.udb[0]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}*/
    pattern.ruleIngrUdbOnly.udb[1]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[2]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[3]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[4]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[5]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[7]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[8]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[9]=0x22;    /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[10]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[2] = 0x1F;
    mask.ruleIngrUdbOnly.udb[4] = 0x1F;

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */
    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;

    /* here we check pattern set for the PCL match in TCAM but withouth the UDB0 defined in PCL */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-2];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2=index;

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
    reducedUsedForExactMatchEntry=GT_TRUE;
    pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
    actionData.pclAction.bypassBridge = GT_TRUE;
    actionData.pclAction.bypassIngressPipe = GT_TRUE;
    actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];

    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);


    return;
}
/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keyMask to be != 0xFF
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet
(
     GT_VOID
)
{
    GT_STATUS                                       rc=GT_OK;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;


     /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;
    keyParams.keyStart=0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;/* {PCL-ID[7:0]}*/
    pattern.ruleIngrUdbOnly.udb[1]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}*/
    pattern.ruleIngrUdbOnly.udb[2]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[3]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[4]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[5]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[7]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[8]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[9]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[10]=0x22;    /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[11]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[0] = PRV_TGF_PCL_ID_CNS;
    mask.ruleIngrUdbOnly.udb[1] = 0x80;
    mask.ruleIngrUdbOnly.udb[2] = (PRV_TGF_VLANID_CNS&0xFF);
    mask.ruleIngrUdbOnly.udb[3] = ((PRV_TGF_VLANID_CNS>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[4] = (prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);
    mask.ruleIngrUdbOnly.udb[5] = ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[6] = 0;
    mask.ruleIngrUdbOnly.udb[7] = 0;
    mask.ruleIngrUdbOnly.udb[8] = 0x44;
    mask.ruleIngrUdbOnly.udb[9] = 0x33;
    mask.ruleIngrUdbOnly.udb[10] = 0x22;
    mask.ruleIngrUdbOnly.udb[11] = 0x11;

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */
    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E;

    /* here we check pattern set for the PCL match in TCAM but withouth the UDB0 defined in PCL */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-2];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2=index;

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
    reducedUsedForExactMatchEntry=GT_TRUE;
    pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
    actionData.pclAction.bypassBridge = GT_TRUE;
    actionData.pclAction.bypassIngressPipe = GT_TRUE;
    actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];

    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);


    return;
}
/**
* @internal prvTgfExactMatchTtiPclFullPathUdb47ConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathUdb47ConfigSet
(
     GT_VOID
)
{
    GT_STATUS                                       rc=GT_OK;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;


     /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    keyParams.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;
    keyParams.keyStart=1;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(pattern.ruleIngrUdbOnly.udb, 0, sizeof(pattern.ruleIngrUdbOnly.udb));
    cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */

    /* pattern */
    /*pattern.ruleIngrUdbOnly.udb[0]=PRV_TGF_PCL_ID_CNS;*//* {PCL-ID[7:0]}*/
    pattern.ruleIngrUdbOnly.udb[0]=0x80;                /* {UDB Valid,reserved,PCL-ID[9:8]}*/
    pattern.ruleIngrUdbOnly.udb[1]=(PRV_TGF_VLANID_CNS&0xFF);     /* eVLAN LSB [7:0] */
    pattern.ruleIngrUdbOnly.udb[2]=((PRV_TGF_VLANID_CNS>>8)&0x1F);/* eVLAN MSB [12:8]*/
    pattern.ruleIngrUdbOnly.udb[3]=(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);/* Source-ePort[7:0] */
    pattern.ruleIngrUdbOnly.udb[4]=((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);/* Source-ePort[12:8] */

    /*{ 0x00, 0x00, 0x44, 0x33, 0x22, 0x11 } - daMac */
    pattern.ruleIngrUdbOnly.udb[5]=0;       /* Dst Mac MSB      */
    pattern.ruleIngrUdbOnly.udb[6]=0;       /* Dst Mac byte 1   */
    pattern.ruleIngrUdbOnly.udb[7]=0x44;    /* Dst Mac byte 2   */
    pattern.ruleIngrUdbOnly.udb[8]=0x33;    /* Dst Mac byte 3   */
    pattern.ruleIngrUdbOnly.udb[9]=0x22;    /* Dst Mac byte 4   */
    pattern.ruleIngrUdbOnly.udb[10]=0x11;   /* Dst Mac LSB      */

    mask.ruleIngrUdbOnly.udb[0] = 0x80;
    mask.ruleIngrUdbOnly.udb[1] = (PRV_TGF_VLANID_CNS&0xFF);
    mask.ruleIngrUdbOnly.udb[2] = ((PRV_TGF_VLANID_CNS>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[3] = (prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]&0xFF);
    mask.ruleIngrUdbOnly.udb[4] = ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]>>8)&0x1F);
    mask.ruleIngrUdbOnly.udb[5] = 0;
    mask.ruleIngrUdbOnly.udb[6] = 0;
    mask.ruleIngrUdbOnly.udb[7] = 0x44;
    mask.ruleIngrUdbOnly.udb[8] = 0x33;
    mask.ruleIngrUdbOnly.udb[9] = 0x22;
    mask.ruleIngrUdbOnly.udb[10] = 0x11;

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], mask.ruleIngrUdbOnly.udb, sizeof(keyParams.mask));

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */
    exactMatchEntry.key.keySize=PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E;

    /* here we check pattern set for the PCL match in TCAM but withouth the UDB0 defined in PCL */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], pattern.ruleIngrUdbOnly.udb, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-4];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }

    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2=index;

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry2,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Exact Match Entry - Reduced entry set forward port to be 2 */
    reducedUsedForExactMatchEntry=GT_TRUE;
    pktCmdUsedForExactMatchEntry = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
    actionData.pclAction.bypassBridge = GT_TRUE;
    actionData.pclAction.bypassIngressPipe = GT_TRUE;
    actionData.pclAction.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=prvTgfDevNum;
    actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];

    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);


    return;
}

/**
* @internal prvTgfExactMatchTtiPclFullPathActivitySet function
* @endinternal
*
* @brief   Set Expected Activity
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathActivitySet
(
     GT_BOOL    expectedActivityHit
)
{
    GT_STATUS           rc=GT_OK;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_BOOL             exactMatchActivityStatus;

    rc =  prvTgfExactMatchActivityStatusGet(prvTgfDevNum,
                                            portGroupsBmp,
                                            prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2,
                                            GT_TRUE, /* exactMatchClearActivity*/
                                            &exactMatchActivityStatus);
    if (exactMatchActivityStatus!=expectedActivityHit)
    {
        /* value different then expected */
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityStatusGet value different then expected - FAILED");
    }
    else
    {
         PRV_UTF_LOG2_MAC("exactMatchEntryIndex = 0x%X, exactMatchActivityStatus = %d,\n\n",
                          prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2,exactMatchActivityStatus);
    }
    return;
}

/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                           rc=GT_OK;

    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      zeroAction;
    GT_PORT_GROUPS_BMP                  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    /* AUTODOC: Restore exactMatchActivityEnable */
    rc =  prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,prvTgfExactMatchRestoreCfg.exactMatchActivityEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchActivityBitEnableSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         prvTgfExactMatchRestoreCfg.profileIdEn , prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    if (prvTgfExactMatchRestoreCfg.exactMatchEntryValid==GT_TRUE)
    {
        /* AUTODOC: restore Exact Match Entry */
        rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryIndex,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex,
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,/* same as configured in Extpanded Entry */
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntryAction);/* same a sconfigured in Extpanded Entry */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    }
    else
    {
        /* invalidate the entry */
        rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
    }
    if (prvTgfExactMatchRestoreCfg.exactMatchEntryValid2==GT_TRUE)
    {
        /* AUTODOC: restore Exact Match Entry */
        rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex2,
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntry2,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryActionType2,/* same as configured in Extpanded Entry */
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntryAction2);/* same a sconfigured in Extpanded Entry */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    }
    else
    {
        /* invalidate the entry */
        rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
    }

    /* AUTODOC: restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateEmEntry
(
    GT_VOID
)
{
    GT_STATUS           rc=GT_OK;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* invalidate the entry */
    rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;
}
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry function
* @endinternal
*
* @brief   Invalidate Second Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry
(
    GT_VOID
)
{
    GT_STATUS           rc=GT_OK;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* invalidate the entry */
    rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);

    /* return to test default value */
    pktCmdUsedForExactMatchEntry=CPSS_PACKET_CMD_DROP_HARD_E;
}

/*
* @internal prvTgfExactMatchTtiPclFullPathTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped/dropped/forwarded
*
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*/
GT_VOID prvTgfExactMatchTtiPclFullPathTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic prvTgfTunnelTermEtherTypeTrafficGenerateExpectTraffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* Reset the port Counter */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth:");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear:");

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    PRV_UTF_LOG0_MAC("======= set capture on all ports =======\n");
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* enable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* enable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* enable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* ======= disable capture on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* disable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* disable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* disable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get no traffic - dropped */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;
        case PRV_TGF_EGR1_PORT_IDX_CNS:
                if ((expectForwardTraffic == GT_TRUE)&&(pktCmdUsedForExactMatchEntry!=CPSS_PACKET_CMD_FORWARD_E))
                {
                    /* port has both Rx and Tx counters because of capture */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                else
                {
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                break;
            case PRV_TGF_EGR2_PORT_IDX_CNS:
                if ((expectForwardTraffic == GT_TRUE)&&
                    (pktCmdUsedForExactMatchEntry==CPSS_PACKET_CMD_FORWARD_E)&&
                    (reducedUsedForExactMatchEntry==GT_TRUE))
                {
                    /* port has both Rx and Tx counters because of capture */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                else
                {
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                break;
            case PRV_TGF_EGR3_PORT_IDX_CNS:
                if ((expectForwardTraffic == GT_TRUE)&&
                    (pktCmdUsedForExactMatchEntry==CPSS_PACKET_CMD_FORWARD_E)&&
                    (reducedUsedForExactMatchEntry==GT_FALSE))
                {
                    /* port has both Rx and Tx counters because of capture */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                else
                {
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                break;
        default:
            expectedCntrs.goodOctetsSent.l[0] = 0;
            expectedCntrs.goodPktsSent.l[0]    = 0;
            expectedCntrs.ucPktsSent.l[0]      = 0;
            expectedCntrs.brdcPktsSent.l[0]    = 0;
            expectedCntrs.mcPktsSent.l[0]      = 0;
            expectedCntrs.goodOctetsRcv.l[0]   = 0;
            expectedCntrs.goodPktsRcv.l[0]     = 0;
            expectedCntrs.ucPktsRcv.l[0]       = 0;
            expectedCntrs.brdcPktsRcv.l[0]     = 0;
            expectedCntrs.mcPktsRcv.l[0]       = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", portCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", portCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", portCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", portCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", portCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", portCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    if(pktCmdUsedForExactMatchEntry==CPSS_PACKET_CMD_FORWARD_E)/* forward from Exact Match*/
    {
        if (reducedUsedForExactMatchEntry==GT_TRUE)
        {
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
        }
        else
        {
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
        }
    }
    else /* pktCmdUsedForExactMatchEntry != forward , we get forward from FDB */
    {
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    }

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (expectForwardTraffic==GT_TRUE)
    {
        /* check if there are captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers, "the test expected traffic to be forwarded\n");
    }
    else
    {
        /* check if no captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers, "the test expected traffic NOT to be forwarded\n");

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
        {
            GT_U8                           packetBufPtr[64] = {0};
            GT_U32                          packetBufLen = 64;
            GT_U32                          packetLen;
            GT_U8                           devNum;
            GT_U8                           queueCpu;
            TGF_NET_DSA_STC                 rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);

            if(expectTrapTraffic==GT_FALSE)
            {
                /* we want to verify no packet was trapped to CPU
                   the command in the Exact Match Default Action was DROP */
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,
                                                rxParams.cpuCode);

                PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
            }
            else
            {
                 /* we want to verify packet was trapped to CPU */
                 UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                 UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,rxParams.cpuCode);

                 PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                 PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
            }
        }
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}
