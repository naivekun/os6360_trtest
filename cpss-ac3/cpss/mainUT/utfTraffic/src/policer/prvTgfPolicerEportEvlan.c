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
* @file prvTgfPolicerEportEvlan.c
*
* @brief Specific Policer Eport and Evlan features testing
*
* @version   16
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerEportEvlan.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>

/* indication that due to Errata the billing not performed */
static GT_BOOL  errataBillingNoPerformed = GT_FALSE;

/* default VLAN Id 5 */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id 5 */
#define PRV_TGF_VLANID_500_CNS          500

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port index 0 */
#define PRV_TGF_PORT_IDX_0_CNS     0
/* port index 1 */
#define PRV_TGF_PORT_IDX_1_CNS     1
/* port index 2 */
#define PRV_TGF_PORT_IDX_2_CNS     2
/* port index 3 */
#define PRV_TGF_PORT_IDX_3_CNS     3

/* maximal number of meter entries (PRV_TGF_POLICER_STORM_TYPE_ENT + 1) */
#define MAX_NUM_OF_METER_ENTRIES_CNS    6

/* not exist policer index */
#define PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS 0xFFFFFFFF

/* Constant to configure all offsets in the trigger entry */
#define ALL_OFFSETS_IN_TRIGGER_ENTRY_CONFIG_CNS    8

/* number of 1 packet to send */
static GT_U32   prvTgfBurst1Count   = 1;

/* number of 2 packets to send */
static GT_U32   prvTgfBurst2Count   = 2;

/* default meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT
    defaultMeterMode[PRV_TGF_POLICER_STAGE_NUM];

/* default counting mode */
static PRV_TGF_POLICER_COUNTING_MODE_ENT
    defaultCountMode[PRV_TGF_POLICER_STAGE_NUM];

/* parameters that is needed to be restored */
static PRV_TGF_TTI_MAC_MODE_ENT        restoreTtiMacMode;

/* TTI Rule index */
static GT_U32   prvTgfTtiRuleIndex = 1;

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1

/* PCL ID */
#define PRV_TGF_IPCL_PCL_ID_CNS  1

/* Flag for eVlan Policer work mode */
static GT_BOOL  policerWorkModeIsEvlan = GT_FALSE;

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* size of prvTgfPacketPayloadLongDataArr */
#define  PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS  \
    sizeof(prvTgfPacketPayloadLongDataArr)

/* LENGTH of ipv4 TCP packet */
#define PRV_TGF_IPV4_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS



/**
* @struct PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC
 *
 * @brief Policer Per stage info for ePort/eVlan testing
*/
typedef struct{

    /** GT_TRUE: the stage is configured for metering/counting */
    GT_BOOL stageCfgExist;

    /** policer mode: Flow or Port */
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT policerMode;

    /** counting mode: disable, billing, policy or vlan */
    PRV_TGF_POLICER_COUNTING_MODE_ENT countMode;

    /** GT_TRUE: Enable/Disable ePort */
    GT_BOOL ePortEn;

    /** @brief ePort number and index for ePort trigger entry.
     *  Note: value of 0xFFFFFFFF - means ePort is disabled
     */
    GT_U32 ePortTriggerIdx;

    /** ePort trigger entry */
    PRV_TGF_POLICER_TRIGGER_ENTRY_STC ePortTriggerEntry;

    /** GT_TRUE: Enable/Disable eVlan */
    GT_BOOL eVlanEn;

    /** @brief index for eVlan trigger entry.
     *  Note: value of 0xFFFFFFFF - means eVlan is disabled
     */
    GT_U32 eVlanTriggerIdx;

    /** @brief eVlan trigger entry
     *  portMeterIdx    - Port number to enable for Metering.
     *  value of 0xFFFFFFFF - no port to enable
     */
    PRV_TGF_POLICER_TRIGGER_ENTRY_STC eVlanTriggerEntry;

    /** @brief GT_TRUE: enable port for metering
     *  GT_FALSE: disable port for metering
     */
    GT_BOOL portMeterEn;

    /** Storm type */
    PRV_TGF_POLICER_STORM_TYPE_ENT stormType;

    /** index for given storm type and port */
    GT_U32 stormTypeIndex;

    /** policer Index to bind to TTI rule */
    GT_U32 ttiPolicerIndex;

    /** GT_TRUE: Enable source ePort assignment in TTI rule or E2FE table */
    GT_BOOL ePortAssignmentEn;

    /** @brief GT_TRUE: TTI rule is valid
     *  GT_FALSE: TTI rule is not valid
     */
    GT_BOOL ttiValid;

    /** number of actual meter entries to configure */
    GT_U32 numOfMeterEntries;

    GT_U32 meterEntryIdxArr[MAX_NUM_OF_METER_ENTRIES_CNS];

    PRV_TGF_POLICER_ENTRY_STC meterEntriesPtrArr[MAX_NUM_OF_METER_ENTRIES_CNS];

} PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC;


/* Policer information for each stage */
static PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC
    policerStageInfoArr[PRV_TGF_POLICER_STAGE_NUM];
/* Expected management set numbers for each state */
static PRV_TGF_POLICER_MNG_CNTR_SET_ENT
    expectManagCntrSetArr[PRV_TGF_POLICER_STAGE_NUM];
/* Expected management counters for each state */
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC
        expectManagEntriesArr[PRV_TGF_POLICER_STAGE_NUM][PRV_TGF_POLICER_MNG_CNTR_DROP_E + 1];
/* Expected Billing indexes for each stage */
static GT_U32
        expectCountIdxArr[PRV_TGF_POLICER_STAGE_NUM][MAX_NUM_OF_METER_ENTRIES_CNS];
/* Expected Billing counters for each stage */
static PRV_TGF_POLICER_BILLING_ENTRY_STC
        expectBillingEntriesArr[PRV_TGF_POLICER_STAGE_NUM][MAX_NUM_OF_METER_ENTRIES_CNS];

static GT_U32   expectDropPkts = 0;

/******************************* Test packet **********************************/


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xBB}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* Unicast unknown da mac */
static TGF_MAC_ADDR     prvTgfUcastUnknownMac =
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA};

/* Unicast da mac */
static TGF_MAC_ADDR     prvTgfUcastKnownMac =
    {0x00, 0x00, 0x00, 0x44, 0x55, 0x44};

/* Multicast unregistered da mac */
static TGF_MAC_ADDR     prvTgfMcastUnregDaMac =
    {0x01, 0x77, 0x88, 0x99, 0xAA, 0xBB};

/* Multicast registered da mac */
static TGF_MAC_ADDR     prvTgfMcastRegDaMac =
    {0x01, 0x22, 0x33, 0x44, 0x55, 0x66};

/* Broadcast da mac */
static TGF_MAC_ADDR     prvTgfBcastDaMac =
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoTag =
{
    PRV_TGF_PACKET_TAG_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTagPartArray                                        /* partsArray */
};

/******************************* Test TCP packet **********************************/

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    5,                  /* data offset */
    0,                  /* reserved */
    0x02,               /* flags -> SYN flag is on */
    4096,               /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    0                   /* urgent pointer */
};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4TcpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_TCP_E, /* TCP -- protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum - need calc */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv4 - TCP */
static TGF_PACKET_PART_STC prvTgfIpv4TcpPacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4TcpPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* ipv4 TCP to send */
static TGF_PACKET_STC prvTgfIpv4TcpPacketInfo = {
    PRV_TGF_IPV4_TCP_PACKET_LEN_CNS,                      /* totalLen */
    sizeof(prvTgfIpv4TcpPacketArray) / sizeof(prvTgfIpv4TcpPacketArray[0]), /* numOfParts */
    prvTgfIpv4TcpPacketArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfPolicerEportEvlanManagementCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerEportEvlanManagementCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrGreen;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrRed;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrYellow;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrDrop;

    cpssOsMemSet(&prvTgfMngCntrGreen, 0, sizeof(prvTgfMngCntrGreen));
    cpssOsMemSet(&prvTgfMngCntrRed, 0, sizeof(prvTgfMngCntrRed));
    cpssOsMemSet(&prvTgfMngCntrYellow, 0, sizeof(prvTgfMngCntrYellow));
    cpssOsMemSet(&prvTgfMngCntrDrop, 0, sizeof(prvTgfMngCntrDrop));

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerEportEvlanTtiEnable function
* @endinternal
*
* @brief   This function enables the TTI lookup
*         and sets the lookup Mac mode
*         for the specified key type
* @param[in] portNum                  - port number
* @param[in] ttiKeyType               - TTI key type
* @param[in] macMode                  - MAC mode to use
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPolicerEportEvlanTtiEnable
(
    IN  GT_U32                    portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT ttiKeyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT macMode,
    IN  GT_BOOL                  isTtiEnable
)
{
    GT_STATUS rc = GT_OK;

    if (GT_TRUE == isTtiEnable)
    {
        /* save current Eth TTI key lookup MAC mode */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, ttiKeyType, &restoreTtiMacMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeGet");
    }
    else
    {
        /* restore Eth TTI key lookup MAC mode */
        macMode = restoreTtiMacMode;
    }

    /* enable/disable port for ttiKeyType TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(portNum, ttiKeyType, isTtiEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* set TTI lookup MAC mode */
    rc = prvTgfTtiMacModeSet(ttiKeyType, macMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeSet");

    return rc;
};

/**
* @internal prvTgfPolicerEportEvlanDefTtiEthRuleSet function
* @endinternal
*
* @brief   Configure Tunnel rule and action
*
* @param[in] vid                      - Vlan ID to set the rule
* @param[in] macAddr                  - MAC Address to set the rule
* @param[in] receivePortIdx           - index of port that the packet should egress
* @param[in] policerInfoPtr           - (pointer to) policer info structure
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPolicerEportEvlanDefTtiEthRuleSet
(
    GT_U16              vid,
    TGF_MAC_ADDR        macAddr,
    GT_U32              receivePortIdx,
    PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC    * policerInfoPtr
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC      ttiAction;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction2;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;
    GT_U32                      policerIndex = policerInfoPtr->ttiPolicerIndex;
    GT_BOOL                     sourceEPortAssignmentEn = policerInfoPtr->ePortAssignmentEn;
    GT_U32                      sourceEPort = policerInfoPtr->ePortTriggerIdx;
    GT_BOOL                     meterEn = policerInfoPtr->meterEntriesPtrArr[0].policerEnable;
    GT_BOOL                     counterEnable = policerInfoPtr->meterEntriesPtrArr[0].counterEnable;

    /* set TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemCpy(ttiPattern.eth.common.mac.arEther, macAddr, sizeof(TGF_MAC_ADDR));
    ttiPattern.eth.common.vid      = vid;
    ttiPattern.eth.common.isTagged = GT_TRUE;

    /* set TTI mask */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(TGF_MAC_ADDR));
    ttiMask.eth.common.vid = 0xFFF;
    ttiMask.eth.common.isTagged = 0x1;
    ttiMask.eth.common.pclId = 0x0;

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.tunnelTerminate                           = GT_FALSE;
    ttiAction.command                                   = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                           = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.interfaceInfo.type                        = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum              = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum             = prvTgfPortsArray[receivePortIdx];
    ttiAction.tag0VlanCmd                               = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    if (policerIndex < PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS)
    {
        ttiAction.meterEnable                               = meterEn;
        ttiAction.counterEnable                             = counterEnable;
        ttiAction.bindToPolicer                             = GT_TRUE;
        ttiAction.policerIndex                              = policerIndex;
    }
    else
    {
        ttiAction.meterEnable                               = GT_FALSE;
        ttiAction.counterEnable                             = GT_FALSE;
        ttiAction.policerIndex                              = 0;

    }

    ttiAction.pcl0OverrideConfigIndex                   = GT_FALSE;
    ttiAction.bridgeBypass                              = GT_TRUE;
    ttiAction.ingressPipeBypass                         = GT_TRUE;

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");


    if (sourceEPortAssignmentEn == GT_TRUE)
    {
        cpssOsMemSet(&ttiAction2, 0, sizeof(ttiAction2));
        ttiAction2.tunnelTerminate                  = ttiAction.tunnelTerminate;
        ttiAction2.command                          = ttiAction.command;
        ttiAction2.redirectCommand                  = ttiAction.redirectCommand;
        ttiAction2.egressInterface.type             = ttiAction.interfaceInfo.type;
        ttiAction2.egressInterface.devPort.hwDevNum   = ttiAction.interfaceInfo.devPort.hwDevNum;
        ttiAction2.egressInterface.devPort.portNum  = ttiAction.interfaceInfo.devPort.portNum;
        ttiAction2.tag0VlanCmd                      = ttiAction.tag0VlanCmd;
        ttiAction2.tag1VlanCmd                      = ttiAction.tag1VlanCmd;
        ttiAction2.bindToPolicer                    = ttiAction.bindToPolicer;
        ttiAction2.bindToPolicerMeter               = ttiAction.meterEnable;
        ttiAction2.policerIndex                     = ttiAction.policerIndex;
        ttiAction2.pcl0OverrideConfigIndex          = ttiAction.pcl0OverrideConfigIndex;
        ttiAction2.bridgeBypass                     = ttiAction.bridgeBypass;
        ttiAction2.ingressPipeBypass                = ttiAction.ingressPipeBypass;
        ttiAction2.sourceEPortAssignmentEnable      = sourceEPortAssignmentEn;
        ttiAction2.sourceEPort                      = sourceEPort;

        rc = prvTgfTtiRuleAction2Set(prvTgfTtiRuleIndex, &ttiAction2);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleAction2Set");
    }

    /* check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleGet");

    return rc;
}

/**
* @internal prvTgfPolicerEportEvlanTunelCfg function
* @endinternal
*
* @brief   TTI Configuration
*
* @param[in] macAddr                  - MAC Address to set the rule
*                                      policerIndex    - policer Index to bind
* @param[in] sendPortIdx              - index of ingress port
* @param[in] receivePortIdx           - index of egress port
* @param[in] policerInfoPtr           - (pointer to) policer info structure
*                                       None
*/
static GT_VOID prvTgfPolicerEportEvlanTunelCfg
(
    GT_U16              vid,
    TGF_MAC_ADDR        macAddr,
    GT_U32              sendPortIdx,
    GT_U32              receivePortIdx,
    PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC    * policerInfoPtr
)
{
    GT_STATUS rc = GT_OK;

    /* enable TTI lookup for Eth key and MAC DA mode */
    rc = prvTgfPolicerEportEvlanTtiEnable(prvTgfPortsArray[sendPortIdx],
                         PRV_TGF_TTI_KEY_ETH_E,
                         PRV_TGF_TTI_MAC_MODE_DA_E,
                         GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

    /* set TTI rules for Eth key */
    if (GT_OK == rc)
    {
        rc = prvTgfPolicerEportEvlanDefTtiEthRuleSet(vid, macAddr,
                                                     receivePortIdx,
                                                     policerInfoPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiEthRuleSet: %d", prvTgfDevNum);
    }
}


/**
* @internal prvTgfPolicerEportEvlanCfg function
* @endinternal
*
* @brief   Set Eport Evlan configuration.
*
* @param[in] macAddr                  - MAC Address to configure in MAC Table
* @param[in] vid                      - Vlan ID
* @param[in] sendPortIdx              - Port index to send the traffic from
* @param[in] receivePortIdx           - index of egress port
* @param[in] packetTypeToSend         - packet type to send
*                                       None
*/
static GT_VOID prvTgfPolicerEportEvlanCfg
(
    IN  TGF_MAC_ADDR                            macAddr,
    IN  GT_U16                                  vid,
    IN  GT_U32                                  sendPortIdx,
    IN  GT_U32                                  receivePortIdx,
    IN  PRV_TGF_POLICER_STORM_TYPE_ENT          packetTypeToSend
)
{
    GT_STATUS                               rc;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT     tbParams;
    GT_U32                                   portNum;
    GT_PORT_NUM                             fdbPortNum;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    GT_U32                                  stage;
    PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC    policerInfo;
    GT_U32                                  i;
    PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT   mode;


    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        policerInfo = policerStageInfoArr[stage];
        /*cpssOsMemCpy(&policerInfo, &(policerStageInfoArr[stage]), sizeof(PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC));*/

        if (policerInfo.stageCfgExist == GT_FALSE)
        {
            /* no configurations at this stage */
            continue;
        }

        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            portNum = prvTgfPortsArray[receivePortIdx];
        }
        else
        {
            portNum = prvTgfPortsArray[sendPortIdx];
        }

        /* store the default meterMode */
        rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, stage,
                                            &defaultMeterMode[stage]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d, %d",
                                     prvTgfDevNum, stage);

        /* Configure the given stage to work in new mode */
        rc = prvTgfPolicerStageMeterModeSet(stage,
                                            policerInfo.policerMode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                     stage, policerInfo.policerMode);

        /* Enable metering */
        rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

        /* Set metered Packet Size Mode as CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E */
        rc = prvTgfPolicerPacketSizeModeSet(stage, CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", stage);

        /* store counting mode */
        rc = prvTgfPolicerCountingModeGet(prvTgfDevNum, stage,
                                          &defaultCountMode[stage]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeGet: %d, %d",
                                     prvTgfDevNum, stage);

        /* Configure counting mode */
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                          policerInfo.countMode);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d %d",
                                     prvTgfDevNum, stage, policerInfo.countMode);

        /* Clear all management counters */
        for (i = PRV_TGF_POLICER_MNG_CNTR_SET0_E; i <= PRV_TGF_POLICER_MNG_CNTR_SET2_E; i++)
        {
            prvTgfPolicerEportEvlanManagementCountersReset(stage, i);
        }


        /* Set ePort/eVLAN Metering mode disabled by default */
        mode = PRV_TGF_POLICER_E_ATTR_METER_MODE_DISABLED_E;
        if(policerInfo.ePortEn == GT_TRUE)
        {
            /* ePort enable */
            mode = PRV_TGF_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E;
        }
        else if(policerInfo.eVlanEn == GT_TRUE)
        {
            /* eVLAN enable */
            mode = PRV_TGF_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E;
        }

        /* Configure ePort/eVlan meter mode */
        rc = prvTgfPolicerEAttributesMeteringModeSet(prvTgfDevNum, stage, mode);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterModeEnableSet: %d, %d",
                                     prvTgfDevNum, stage);

        if (policerInfo.ePortEn)
        {
            /* Configure Trigger entry - ePort mode */
            rc = prvTgfPolicerTriggerEntrySet(prvTgfDevNum,
                                              stage,
                                              policerInfo.ePortTriggerIdx,
                                              &policerInfo.ePortTriggerEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerTriggerEntrySet: %d, %d, %d",
                                         prvTgfDevNum, stage, policerInfo.ePortTriggerIdx);
        }
        if (policerInfo.eVlanEn)
        {
            /* Configure Trigger entry - eVlan mode */
            rc = prvTgfPolicerTriggerEntrySet(prvTgfDevNum,
                                              stage,
                                              policerInfo.eVlanTriggerIdx,
                                              &policerInfo.eVlanTriggerEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerTriggerEntrySet: %d, %d, %d",
                                         prvTgfDevNum, stage, policerInfo.eVlanTriggerIdx);
        }


        /* Configure Port Metering, if enabled */
        if (policerInfo.portMeterEn == GT_TRUE)
        {
            /* Enable port metering on the given port */
            rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage, portNum, GT_TRUE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d",
                                         prvTgfDevNum, stage, portNum);

            /* Configure index for given storm type */
            rc = prvTgfPolicerPortStormTypeIndexSet(prvTgfDevNum, stage, portNum,
                                                    policerInfo.stormType,
                                                    policerInfo.stormTypeIndex);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortStormTypeIndexSet: %d, %d, %d, %d",
                                         prvTgfDevNum, stage, portNum,
                                         policerInfo.stormType);

            /* Enable Port Triggered counting */
            rc = prvTgfPolicerCountingTriggerByPortEnableSet(prvTgfDevNum, stage, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingTriggerByPortEnableSet: %d, %d",
                                         prvTgfDevNum, stage);
        }

        /* configure metering entries */
        for (i = 0; i < policerInfo.numOfMeterEntries; i++)
        {
            if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
            {
                policerInfo.meterEntriesPtrArr[i].modifyDp =
                                    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                policerInfo.meterEntriesPtrArr[i].modifyDscp =
                                    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                policerInfo.meterEntriesPtrArr[i].modifyUp =
                                    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            }

            /* Configure metering entry */
            rc = prvTgfPolicerEntrySet(stage, policerInfo.meterEntryIdxArr[i],
                                       &(policerInfo.meterEntriesPtrArr[i]),
                                       &tbParams);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d", prvTgfDevNum,
                                         stage, policerInfo.meterEntryIdxArr[i]);
        }

        /* Configure FDB entry if needed */
        if (packetTypeToSend == PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E)
        {
            if (policerInfo.ePortAssignmentEn == GT_TRUE)
            {
                fdbPortNum = policerInfo.ePortTriggerIdx;
            }
            else
            {
                fdbPortNum = prvTgfPortsArray[receivePortIdx];
            }

            rc = prvTgfBrgDefFdbMacEntryOnPortSet(macAddr,
                                                  vid,
                                                  prvTgfDevNum,
                                                  fdbPortNum,
                                                  GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d, %d, %d",
                                         macAddr, vid, fdbPortNum);
        }

        if (packetTypeToSend == PRV_TGF_POLICER_STORM_TYPE_MC_REGISTERED_E)
        {
            rc = prvTgfBrgDefFdbMacEntryOnVidSet(macAddr, vid, vid, GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidSet: %d, %d, %d",
                                         macAddr, vid, vid);
        }

        /* configuration only for Egress stage */
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            if (policerInfo.ePortAssignmentEn == GT_TRUE)
            {
                /* configure eToPhy table */
                cpssOsMemSet(&physicalInfo, 0x0, sizeof(CPSS_INTERFACE_INFO_STC));
                physicalInfo.hwDevNum = prvTgfDevNum;
                physicalInfo.vlanId = vid;
                physicalInfo.devPort.portNum = portNum;
                physicalInfo.devPort.hwDevNum = prvTgfDevNum;
                physicalInfo.type = CPSS_INTERFACE_PORT_E;

                rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, policerInfo.ePortTriggerIdx,
                                                                       &physicalInfo);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d, %d",
                                             prvTgfDevNum, policerInfo.ePortTriggerIdx);
            }
        }/* end Egress stage */
        else
        {
            /* configurations only for Ingress stages */

            /* configure TTI, if needed */
            if (policerInfo.ttiValid == GT_TRUE)
            {
                prvTgfPolicerEportEvlanTunelCfg(vid, macAddr,
                                                sendPortIdx,
                                                receivePortIdx,
                                                &policerInfo);
            }/* end ttiValid */
        }/* end Ingress stage */
    }/* end for stage */
}

/**
* @internal prvTgfPolicerEportEvlanPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNumIdx               - port number index to send packet from
* @param[in] numOfPackets             - number of packets to send
* @param[in] packetInfoPtr            - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfPolicerEportEvlanPacketSend
(
    IN GT_U32           portNumIdx,
    IN  GT_U32          numOfPackets,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, numOfPackets, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, numOfPackets, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portNumIdx]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portNumIdx]);
}


/**
* @internal prvTgfPolicerEportEvlanManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerEportEvlanManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT    mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            mngCntrSet,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d, ", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(
        duLowCntr, prvTgfMngCntr.duMngCntr.l[0],
        "prvTgfMngCntr.duMngCntr.l[0] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.duMngCntr.l[0], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        duHiCntr, prvTgfMngCntr.duMngCntr.l[1],
        "prvTgfMngCntr.duMngCntr.l[1] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.duMngCntr.l[1], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
        "prvTgfMngCntr.packetMngCntr = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.packetMngCntr, policerStage, mngCntrSet, mngCntrType);
}


/**
* @internal prvTgfPolicerEportEvlanBillingCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Billing Counters
*/
static GT_VOID prvTgfPolicerEportEvlanBillingCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT        policerStage,
    IN GT_U32                                entryIndex,
    IN PRV_TGF_POLICER_BILLING_ENTRY_STC     billingCntr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_BILLING_ENTRY_STC     readBillingCntr;

    if(errataBillingNoPerformed == GT_TRUE)
    {
        cpssOsMemSet(&billingCntr,0,sizeof(billingCntr));
        PRV_UTF_LOG0_MAC("NOTE: errataBillingNoPerformed = GT_TRUE --> expect 0 in the billing counters \n");
    }


    /* read Billing Entry */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum,
                                      policerStage,
                                      entryIndex,
                                      GT_TRUE,
                                      &readBillingCntr);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssDxCh3PolicerBillingEntryGet: %d, policerStage = %d, entryIndex = %d",
                                 prvTgfDevNum, policerStage, entryIndex);

    /* check counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.greenCntr.l[0], readBillingCntr.greenCntr.l[0],
        "billingCntr.greenCntr.l[0] = %d, readBillingCntr.greenCntr.l[0] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.greenCntr.l[0], readBillingCntr.greenCntr.l[0], policerStage, entryIndex);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.greenCntr.l[1], readBillingCntr.greenCntr.l[1],
        "billingCntr.greenCntr.l[1] = %d, readBillingCntr.greenCntr.l[1] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.greenCntr.l[1], readBillingCntr.greenCntr.l[1], policerStage, entryIndex);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.yellowCntr.l[0], readBillingCntr.yellowCntr.l[0],
        "billingCntr.yellowCntr.l[0] = %d, readBillingCntr.yellowCntr.l[0] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.yellowCntr.l[0], readBillingCntr.yellowCntr.l[0], policerStage, entryIndex);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.yellowCntr.l[1], readBillingCntr.yellowCntr.l[1],
        "billingCntr.yellowCntr.l[1] = %d, readBillingCntr.yellowCntr.l[1] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.yellowCntr.l[1], readBillingCntr.yellowCntr.l[1], policerStage, entryIndex);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.redCntr.l[0], readBillingCntr.redCntr.l[0],
        "billingCntr.redCntr.l[0] = %d, readBillingCntr.redCntr.l[0] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.redCntr.l[0], readBillingCntr.redCntr.l[0], policerStage, entryIndex);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        billingCntr.redCntr.l[1], readBillingCntr.redCntr.l[1],
        "billingCntr.redCntr.l[1] = %d, readBillingCntr.redCntr.l[1] = %d, policerStage = %d, entryIndex = %d\n",
        billingCntr.redCntr.l[1], readBillingCntr.redCntr.l[1], policerStage, entryIndex);
}

/**
* @internal prvTgfPolicerEportEvlanClearBillingCounters function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerEportEvlanClearBillingCounters
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT        stage
)
{
    GT_U32                              ii;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   billingCntr;
    GT_STATUS                           rc;

    cpssOsMemSet(&billingCntr, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));

    billingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E;
    billingCntr.greenCntr.l[0] = 0;
    billingCntr.greenCntr.l[1] = 0;
    billingCntr.yellowCntr.l[0] = 0;
    billingCntr.yellowCntr.l[1] = 0;
    billingCntr.redCntr.l[0] = 0;
    billingCntr.redCntr.l[1] = 0;
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        billingCntr.packetSizeMode =
            CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    }
    for (ii = 0; ii < policerStageInfoArr[stage].numOfMeterEntries; ii++)
    {
        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum, stage,
                                          policerStageInfoArr[stage].meterEntryIdxArr[ii],
                                          &billingCntr);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerBillingEntrySet: %d, stage:%d, index:%d", prvTgfDevNum,
                                     stage, policerStageInfoArr[stage].meterEntryIdxArr[ii]);
    }
}

/**
* @internal prvTgfPolicerEportEvlanCheckResults function
* @endinternal
*
* @brief   Check results
*
* @param[in] vid                      - Vlan ID
* @param[in] sendPortIdx              - Port index to send the traffic from
* @param[in] receivePortIdx           - index of egress port
* @param[in] numOfSentPackets         - number of sent packets
* @param[in] packetType               - packet type
*                                       None
*/
static GT_VOID prvTgfPolicerEportEvlanCheckResults
(
    IN  GT_U16                                  vid,
    IN  GT_U32                                  sendPortIdx,
    IN  GT_U32                                  receivePortIdx,
    IN  GT_U32                                  numOfSentPackets,
    IN  PRV_TGF_POLICER_STORM_TYPE_ENT          packetType
)
{
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          dropPktsNum;    /* number of dropped packets    */
    GT_U32                          greenPktsNum;   /* number of green packets      */
    GT_U32                          yellowPktsNum;  /* number of yellow packets     */
    GT_U32                          redPktsNum;     /* number of red packets        */
    GT_U32                          expectedPktsNum; /* the expected packets number to receive */
    GT_U32                          expectedBytes;  /* expected bytes number        */
    PRV_TGF_POLICER_STAGE_TYPE_ENT  stage;          /* stage iterator               */
    PRV_TGF_POLICER_MNG_CNTR_SET_ENT  mngCounterSet; /* Management counter set      */
    GT_U32                          ii;             /* loop iterator                */
    GT_STATUS                       rc;             /* return code                  */

    TGF_PARAM_NOT_USED(vid);/* avoid compiler warning */

    dropPktsNum     = 0;
    greenPktsNum    = 0;
    yellowPktsNum   = 0;
    redPktsNum      = 0;


    /* check ETH counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (sendPortIdx == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfSentPackets, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        if (receivePortIdx == portIter)
        {
            expectedPktsNum = numOfSentPackets - expectDropPkts;

            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPktsNum, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }/* end (receivePortIdx == portIter) */
    }/* end for portIter*/


    for (stage = 0; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        if (expectManagCntrSetArr[stage] != PRV_TGF_POLICER_MNG_CNTR_DISABLED_E)
        {
            /* metering was done for this stage */

            /* Check management counters */

            mngCounterSet = expectManagCntrSetArr[stage];

            greenPktsNum =
                expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].packetMngCntr;
            yellowPktsNum =
                expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].packetMngCntr;
            redPktsNum =
                expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].packetMngCntr;
            dropPktsNum =
                expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_DROP_E].packetMngCntr;

            if ((stage == PRV_TGF_POLICER_STAGE_EGRESS_E) &&
                (policerWorkModeIsEvlan == GT_TRUE))
            {
                if (packetType != PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E)
                {
                    /* each packet in this case is counted four times:
                      - FROM CPU packet
                      - each egress port for flooded packets */
                    if (tgfTrafficGeneratorEtherTypeIgnoreGet() == GT_FALSE)
                    {
                        greenPktsNum *= 4;
                        yellowPktsNum *= 4;
                        redPktsNum *= 4;
                        dropPktsNum *= 4;
                    }
                    else
                    {
                        greenPktsNum *= 3;
                        yellowPktsNum *= 3;
                        redPktsNum *= 3;
                        dropPktsNum *= 3;
                    }
                }
                else
                {
                    if (tgfTrafficGeneratorEtherTypeIgnoreGet() == GT_FALSE)
                    {
                        /* each packet in this case is counted twice:
                          - FROM CPU packet
                          - egress port for unicast packet */
                        greenPktsNum *= 2;
                        yellowPktsNum *= 2;
                        redPktsNum *= 2;
                        dropPktsNum *= 2;
                    }
                }
            }

            expectedBytes = (greenPktsNum * 64);
            prvTgfPolicerEportEvlanManagementCountersCheck(stage,
                                                           mngCounterSet,
                                                           PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                                           expectedBytes, 0, greenPktsNum);

            expectedBytes = (yellowPktsNum * 64);
            prvTgfPolicerEportEvlanManagementCountersCheck(stage,
                                                           mngCounterSet,
                                                           PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                                           expectedBytes, 0,
                                                           yellowPktsNum);

            expectedBytes = (redPktsNum * 64);
            prvTgfPolicerEportEvlanManagementCountersCheck(stage,
                                                           mngCounterSet,
                                                           PRV_TGF_POLICER_MNG_CNTR_RED_E,
                                                           expectedBytes, 0,
                                                           redPktsNum);

            expectedBytes = (dropPktsNum * 64);
            prvTgfPolicerEportEvlanManagementCountersCheck(stage,
                                                           mngCounterSet,
                                                           PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                                                           expectedBytes, 0,
                                                           dropPktsNum);
        }

        /* Do not check Billing counters for Egress stage,
           Remarking should be taken into account */
        if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            for (ii = 0; ii < policerStageInfoArr[stage].numOfMeterEntries; ii++)
            {
                if (expectCountIdxArr[stage][ii] < PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS)
                {
                    /* billing was done for this stage,
                       we assume that counters count in 1Byte mode */
                    prvTgfPolicerEportEvlanBillingCountersCheck((PRV_TGF_POLICER_STAGE_TYPE_ENT)stage,
                                                                expectCountIdxArr[stage][ii],
                                                                expectBillingEntriesArr[stage][ii]);
                }
            }/* end for ii */
        }/* end if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E) */
    }/* end for stage */
}



/**
* @internal prvTgfPolicerEportEvlanTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*
* @param[in] vid                      - Vlan ID
* @param[in] sendPortIdx              - Port index to send the traffic from
* @param[in] receivePortIdx           - index of egress port
* @param[in] numOfPackets             - number of packets to send
* @param[in] packetTypeToSend         - packet type to send
*                                       None
*/
static GT_VOID prvTgfPolicerEportEvlanTrafficGenerate
(
    IN  GT_U16                                  vid,
    IN  GT_U32                                  sendPortIdx,
    IN  GT_U32                                  receivePortIdx,
    IN  GT_U32                                  numOfPackets,
    IN  PRV_TGF_POLICER_STORM_TYPE_ENT          packetTypeToSend
)
{
    GT_STATUS                               rc;             /* returned status */
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT     tbParams;
    PRV_TGF_POLICER_ENTRY_STC               tmpMeterEntry;
    GT_U32                                  portNum;
    TGF_PACKET_STC                          packetInfo;
    GT_U32                                  stage;
    PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC    policerInfo;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    GT_U32                                  ii;


    switch (packetTypeToSend)
    {
        case PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E:
            /* Set destination addres to known unicast */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUcastKnownMac,
                             sizeof(prvTgfPacketL2Part.daMac));
            break;
        case PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E:
        case PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E:
            /* Set destination addres to unknown unicast */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUcastUnknownMac,
                             sizeof(prvTgfPacketL2Part.daMac));
            break;
        case PRV_TGF_POLICER_STORM_TYPE_MC_UNREGISTERED_E:
            /* Set destination addres to unregistered multicast */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcastUnregDaMac,
                             sizeof(prvTgfPacketL2Part.daMac));
            break;
        case PRV_TGF_POLICER_STORM_TYPE_MC_REGISTERED_E:
            /* Set destination addres to registered multicast */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcastRegDaMac,
                             sizeof(prvTgfPacketL2Part.daMac));
            break;
        case PRV_TGF_POLICER_STORM_TYPE_BC_E:
            /* Set destination addres to broadcast */
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfBcastDaMac,
                            sizeof(prvTgfPacketL2Part.daMac));
            break;
        default:
            break;
    }

    if (packetTypeToSend == PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E)
    {
        cpssOsMemCpy(&packetInfo, &prvTgfIpv4TcpPacketInfo,
                        sizeof(TGF_PACKET_STC));
    }
    else
    {
        cpssOsMemCpy(&packetInfo, &prvTgfPacketInfoTag,
                         sizeof(TGF_PACKET_STC));
    }

    /* Set VLAN packet part */
    prvTgfPacketVlanTagPart.vid = vid;

    /* Configure Policer */
    prvTgfPolicerEportEvlanCfg(prvTgfPacketL2Part.daMac, vid,
                               sendPortIdx, receivePortIdx,
                               packetTypeToSend);
    /* Send packet */
    prvTgfPolicerEportEvlanPacketSend(sendPortIdx, numOfPackets, &packetInfo);

    /* give time for device/simulation for packet processing */
    cpssOsTimerWkAfter(10);

    /* check Counters */
    prvTgfPolicerEportEvlanCheckResults(vid,
                                        sendPortIdx,
                                        receivePortIdx,
                                        numOfPackets,
                                        packetTypeToSend);

    /* Restore Policer Configurations */

    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        policerInfo = policerStageInfoArr[stage];

        if (policerInfo.stageCfgExist == GT_FALSE)
        {
            /* no configurations at this stage */
            continue;
        }

        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            /* Egress port */
            portNum = prvTgfPortsArray[receivePortIdx];
        }
        else
        {
            /* Ingress port */
            portNum = prvTgfPortsArray[sendPortIdx];
        }


        /* Configure Port Metering, if enabled */
        if (policerInfo.portMeterEn == GT_TRUE)
        {
            /* Disable port metering on the given port */
            rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage, portNum, GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d",
                                         prvTgfDevNum, stage, portNum);

            /* Set index for given storm type to 0 */
            rc = prvTgfPolicerPortStormTypeIndexSet(prvTgfDevNum, stage, portNum,
                                                    policerInfo.stormType, 0);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortStormTypeIndexSet: %d, %d, %d, %d",
                                         prvTgfDevNum, stage, portNum,
                                         policerInfo.stormType);
        }
        else
        {
            if (policerInfo.countMode != PRV_TGF_POLICER_COUNTING_DISABLE_E)
            {
                /* Reset Billing counters */

                /* Disable metering, it will be enabled later */
                rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

                /* Disable Billing, it will be restored later */
                rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage, GT_FALSE);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

                /* Flush billing counters */
                rc = prvTgfPolicerCountingWriteBackCacheFlush(stage);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet");

                /* Clear Billing counters */
                prvTgfPolicerEportEvlanClearBillingCounters(stage);
            }
        }

        /* Restore metering entries */

        cpssOsMemSet(&tmpMeterEntry, 0, sizeof(tmpMeterEntry));
        tmpMeterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
        tmpMeterEntry.yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
        tmpMeterEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        tmpMeterEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        tmpMeterEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        /* Set maximal values */
        tmpMeterEntry.tbParams.srTcmParams.cir = 0xFFFFFFFF;
        tmpMeterEntry.tbParams.srTcmParams.cbs = 0xFFFFFFFF;
        tmpMeterEntry.tbParams.srTcmParams.ebs = 0xFFFFFFFF;

        for (ii = 0; ii < policerInfo.numOfMeterEntries; ii++)
        {
            rc = prvTgfPolicerEntrySet(stage, policerInfo.meterEntryIdxArr[ii],
                                       &tmpMeterEntry, &tbParams);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d", prvTgfDevNum,
                                         stage, policerInfo.meterEntryIdxArr[ii]);
        }

        /* configuration only for Egress stage */
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            if (policerInfo.ePortAssignmentEn == GT_TRUE)
            {
                /* Restore eToPhy table */
                cpssOsMemSet(&physicalInfo, 0x0, sizeof(CPSS_INTERFACE_INFO_STC));
                physicalInfo.hwDevNum = prvTgfDevNum;
                physicalInfo.vlanId = 0x0;
                physicalInfo.devPort.portNum = 0x0;
                physicalInfo.devPort.hwDevNum = prvTgfDevNum;
                physicalInfo.type = CPSS_INTERFACE_PORT_E;

                rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, policerInfo.ePortTriggerIdx,
                                                                       &physicalInfo);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d, %d",
                                             prvTgfDevNum, policerInfo.ePortTriggerIdx);
            }
        }/* end Egress stage */
        else
        {
            /* configurations only for Ingress stages */

            /* Restore TTI, if needed */
            if (policerInfo.ttiValid == GT_TRUE)
            {
                /* Invalidate TTI rule */
                rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

                /* disable TTI lookup for Eth key and MAC DA mode */
                rc = prvTgfPolicerEportEvlanTtiEnable(prvTgfPortsArray[sendPortIdx],
                                                      PRV_TGF_TTI_KEY_ETH_E,
                                                      PRV_TGF_TTI_MAC_MODE_DA_E,
                                                      GT_FALSE);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
            }/* end ttiValid */
        }/* end Ingress stage */
    }/* end for stage */
}


/**
* @internal prvTgfPolicerEportEvlanInitAllStages function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPolicerEportEvlanInitAllStages
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32                                  jj, ii;

    /* initialize all stages */
    for (jj = 0; jj < PRV_TGF_POLICER_STAGE_NUM; jj++)
    {
        cpssOsMemSet(&(policerStageInfoArr[jj]), 0x0,
                     sizeof(PRV_TGF_POLICER_EPORT_EVLAN_INFO_STC));
        policerStageInfoArr[jj].stageCfgExist = GT_FALSE;

        /* Initialize Billing and Management arrays */
        cpssOsMemSet(expectManagEntriesArr[jj], 0x0,
                     (sizeof(PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC) * (PRV_TGF_POLICER_MNG_CNTR_DROP_E + 1)));
        cpssOsMemSet(expectCountIdxArr[jj], 0x0,
                     (sizeof(GT_U32) * (MAX_NUM_OF_METER_ENTRIES_CNS)));
        cpssOsMemSet(expectBillingEntriesArr[jj], 0x0,
                     (sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC) * (MAX_NUM_OF_METER_ENTRIES_CNS)));

        if (jj != (GT_U32)stage)
        {
            /* Management sets */
            expectManagCntrSetArr[jj] = PRV_TGF_POLICER_MNG_CNTR_DISABLED_E;

            /* Billing counter indexes */
            for (ii = 0; ii < MAX_NUM_OF_METER_ENTRIES_CNS; ii++)
            {
                expectCountIdxArr[jj][ii] = PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS;
            }
        }
    }
}

/**
* @internal prvTgfPolicerEportEvlanTrigEntryCfg function
* @endinternal
*
* @brief   Configure Policer offset for specific traffic type
*
* @param[in] countingEnable           - GT_TRUE: enable counting for this entry
* @param[in] meteringEnable           - GT_TRUE: enable metering for this entry
* @param[in] policerIndex             - policer index for this entry
* @param[in] packetTypeToSend         - packet type to send
* @param[in] eOffset                  - policer index offset for specific traffic type
* @param[in,out] triggerEntryPtr          - (pointer to) policer trigger entry
* @param[in] stage                    - policer stage
* @param[in,out] triggerEntryPtr          - (pointer to) policer trigger entry
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanTrigEntryCfg
(
    IN GT_BOOL                                 countingEnable,
    IN GT_BOOL                                 meteringEnable,
    IN GT_U32                                  policerIndex,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT          packetTypeToSend,
    IN GT_U32                                  eOffset,
    INOUT PRV_TGF_POLICER_TRIGGER_ENTRY_STC    *triggerEntryPtr,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT          stage
)
{
    triggerEntryPtr->countingEnable = countingEnable;
    triggerEntryPtr->meteringEnable = meteringEnable;
    triggerEntryPtr->policerIndex = policerIndex;

    if (eOffset < 0x7)
    {
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            /* Egress stage should use offset 0 for all traffic types */
            triggerEntryPtr->ucKnownEnable = GT_TRUE;
            triggerEntryPtr->ucKnownOffset = 0;
            triggerEntryPtr->ucUnknownEnable = GT_TRUE;
            triggerEntryPtr->ucUnknownOffset = 0;
            triggerEntryPtr->mcUnregisteredEnable = GT_TRUE;
            triggerEntryPtr->mcUnregisteredOffset = 0;
            triggerEntryPtr->mcRegisteredEnable = GT_TRUE;
            triggerEntryPtr->mcRegisteredOffset = 0;
            triggerEntryPtr->bcEnable = GT_TRUE;
            triggerEntryPtr->bcOffset = 0;
            triggerEntryPtr->tcpSynEnable = GT_TRUE;
            triggerEntryPtr->tcpSynOffset = 0;
        }
        else
        {
            switch (packetTypeToSend)
            {
                case PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E:
                    triggerEntryPtr->ucKnownEnable = GT_TRUE;
                    triggerEntryPtr->ucKnownOffset = eOffset;
                    break;
                case PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E:
                    triggerEntryPtr->ucUnknownEnable = GT_TRUE;
                    triggerEntryPtr->ucUnknownOffset = eOffset;
                    break;
                case PRV_TGF_POLICER_STORM_TYPE_MC_UNREGISTERED_E:
                    triggerEntryPtr->mcUnregisteredEnable = GT_TRUE;
                    triggerEntryPtr->mcUnregisteredOffset = eOffset;
                    break;
                case PRV_TGF_POLICER_STORM_TYPE_MC_REGISTERED_E:
                    triggerEntryPtr->mcRegisteredEnable = GT_TRUE;
                    triggerEntryPtr->mcRegisteredOffset = eOffset;
                    break;
                case PRV_TGF_POLICER_STORM_TYPE_BC_E:
                    triggerEntryPtr->bcEnable = GT_TRUE;
                    triggerEntryPtr->bcOffset = eOffset;
                    break;
                case PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E:
                    triggerEntryPtr->tcpSynEnable = GT_TRUE;
                    triggerEntryPtr->tcpSynOffset = eOffset;
                    break;
                default:
                    break;
            }
        }
    }
}


/**
* @internal prvTgfPolicerEportEvlanCfgAndTrafficGenerate function
* @endinternal
*
* @brief   Configure Policer and send traffic
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanCfgAndTrafficGenerate
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  GT_U16                                  vid,
    IN  GT_U32                                  sendPortIdx,
    IN  GT_U32                                  receivePortIdx,
    IN  GT_U32                                  numOfPackets,
    IN  PRV_TGF_POLICER_STORM_TYPE_ENT          packetTypeToSend,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT        mngCounterSet,
    IN  PRV_TGF_POLICER_STAGE_METER_MODE_ENT    policerMode,
    IN  PRV_TGF_POLICER_COUNTING_MODE_ENT       countMode,
    IN  GT_BOOL                                 policerEnable,
    IN  GT_BOOL                                 counterEnable,
    IN  GT_BOOL                                 portMeterEn,
    IN  GT_U32                                  stormTypeIndex,
    IN  GT_BOOL                                 eVlanEn,
    IN  GT_BOOL                                 ePortEn,
    IN  GT_U32                                  eVlanPolicerIndex,
    IN  GT_U32                                  ePortPolicerIndex,
    IN  GT_U32                                  ePortTrigNum,
    IN  GT_BOOL                                 eTrigEntryCountEn,
    IN  GT_BOOL                                 eTrigEntryMeterEn,
    IN  GT_U32                                  eOffset,
    IN  GT_BOOL                                 ttiValid,
    IN  GT_U32                                  ttiPolicerIndex,
    IN  GT_BOOL                                 ePortAssignmentEn,
    IN  GT_U32                                  cir[MAX_NUM_OF_METER_ENTRIES_CNS],
    IN  GT_U32                                  cbs[MAX_NUM_OF_METER_ENTRIES_CNS],
    IN  GT_U32                                  ebs[MAX_NUM_OF_METER_ENTRIES_CNS]
)
{
    GT_U32                  i;

    /* check for conditions of of bc2 erratum : in FLOW mode :
       Bobcat2 Policer Billing for Per Physical Port metering does not works */
    if((GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,  UTF_CPSS_PP_E_ARCH_CNS)) &&
        (policerMode == PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E) && /*flow mode*/
        ((eVlanEn == GT_FALSE) && (ePortEn == GT_FALSE)) && /* no eport/evlan mode*/
        (countMode == PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E) &&/* billing/ipfix count mode */
        (portMeterEn == GT_TRUE) /* port metering */
        )
    {
        if(GT_TRUE == prvUtfIsGmCompilation())
        {
            /* the GM act 'ok' (no such errata) this behavior */
            errataBillingNoPerformed = GT_FALSE;
        }
        else
        {
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                /* FE: POLICER-1322 - Bobcat2 Policer Billing for Per Physical Port metering does not works */
                errataBillingNoPerformed = GT_TRUE;

                PRV_UTF_LOG0_MAC("WARNING: in FLOW mode Bobcat2 Policer Billing for Per Physical Port metering does not works \n");
            }

        }
    }
    else
    {
        errataBillingNoPerformed = GT_FALSE;
    }

    /* Fill Policer info */
    policerStageInfoArr[stage].stageCfgExist = GT_TRUE;
    policerStageInfoArr[stage].policerMode = policerMode;
    policerStageInfoArr[stage].countMode = countMode;
    policerStageInfoArr[stage].portMeterEn = portMeterEn;
    policerStageInfoArr[stage].stormType = packetTypeToSend;
    policerStageInfoArr[stage].stormTypeIndex = stormTypeIndex;
    policerStageInfoArr[stage].ttiPolicerIndex = ttiPolicerIndex;
    policerStageInfoArr[stage].ePortAssignmentEn = ePortAssignmentEn;
    policerStageInfoArr[stage].ttiValid = ttiValid;


    /* Fill in Meter entries info */
    for (i = 0; i < policerStageInfoArr[stage].numOfMeterEntries; i++)
    {
        policerStageInfoArr[stage].meterEntriesPtrArr[i].policerEnable = policerEnable;
        policerStageInfoArr[stage].meterEntriesPtrArr[i].counterEnable = counterEnable;
        policerStageInfoArr[stage].meterEntriesPtrArr[i].mngCounterSet = mngCounterSet;
        policerStageInfoArr[stage].meterEntriesPtrArr[i].tbParams.srTcmParams.
            cir = cir[i];
        policerStageInfoArr[stage].meterEntriesPtrArr[i].tbParams.srTcmParams.
            cbs = cbs[i];
        policerStageInfoArr[stage].meterEntriesPtrArr[i].tbParams.srTcmParams.
            ebs = ebs[i];
        /* Counter index */
        policerStageInfoArr[stage].meterEntriesPtrArr[i].countingEntryIndex =
            policerStageInfoArr[stage].meterEntryIdxArr[i];

        /* Set expected Billing results, indexes */
        expectCountIdxArr[stage][i] =
            policerStageInfoArr[stage].meterEntryIdxArr[i];
    }

    /* Fill in info for eVlan metering */
    policerStageInfoArr[stage].eVlanEn = eVlanEn;
    policerStageInfoArr[stage].eVlanTriggerIdx = vid;

    /* Clear Billing counters */
    prvTgfPolicerEportEvlanClearBillingCounters(stage);

    if (eVlanEn == GT_TRUE)
    {
        if (eOffset == ALL_OFFSETS_IN_TRIGGER_ENTRY_CONFIG_CNS)
        {
            for (eOffset = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
                  eOffset <= PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E; eOffset++)
            {
                prvTgfPolicerEportEvlanTrigEntryCfg(eTrigEntryCountEn,
                                                    eTrigEntryMeterEn,
                                                    eVlanPolicerIndex,
                                                    (PRV_TGF_POLICER_STORM_TYPE_ENT)eOffset,
                                                    eOffset,
                                                    &(policerStageInfoArr[stage].eVlanTriggerEntry),
                                                    stage
                                                    );
            }
        }
        else
        {
            prvTgfPolicerEportEvlanTrigEntryCfg(eTrigEntryCountEn,
                                                eTrigEntryMeterEn,
                                                eVlanPolicerIndex,
                                                packetTypeToSend,
                                                eOffset,
                                                &(policerStageInfoArr[stage].eVlanTriggerEntry),
                                                stage);
        }
    }

    /* Fill in info about ePort metering/counting */
    policerStageInfoArr[stage].ePortEn = ePortEn;
    policerStageInfoArr[stage].ePortTriggerIdx = ePortTrigNum;

    if (ePortEn == GT_TRUE)
    {
        if (eOffset == ALL_OFFSETS_IN_TRIGGER_ENTRY_CONFIG_CNS)
        {
            for (eOffset = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
                  eOffset <= PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E; eOffset++)
            {
                prvTgfPolicerEportEvlanTrigEntryCfg(eTrigEntryCountEn,
                                                    eTrigEntryMeterEn,
                                                    ePortPolicerIndex,
                                                    (PRV_TGF_POLICER_STORM_TYPE_ENT)eOffset,
                                                    eOffset,
                                                    &(policerStageInfoArr[stage].ePortTriggerEntry),
                                                    stage);
            }
        }
        else
        {
            prvTgfPolicerEportEvlanTrigEntryCfg(eTrigEntryCountEn,
                                                eTrigEntryMeterEn,
                                                ePortPolicerIndex,
                                                packetTypeToSend,
                                                eOffset,
                                                &(policerStageInfoArr[stage].ePortTriggerEntry),
                                                stage);
        }
    }

    /* Fill in expected management counters values */
    expectManagCntrSetArr[stage] = mngCounterSet;

    /* Generate traffic */
    prvTgfPolicerEportEvlanTrafficGenerate(vid,
                                           sendPortIdx,
                                           receivePortIdx,
                                           numOfPackets,
                                           packetTypeToSend);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/* default memory mode */
static  PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT  defaultMemMode;
static GT_U32 orig_numEntriesIngressStage0;
static GT_U32 orig_numEntriesIngressStage1;



/* default address select mode */
static PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT defaultAddrSelTypeStage_1;
/**
* @internal prvTgfPolicerEportEvlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfPolicerEportEvlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32    numEntriesEgressStage;
    GT_U32      totalNumEntries;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* 1. Create new vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_500_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_500_CNS);

    /* configure Mode4 (both stages use same size) to support physical port
      metering for whole port range 0..256 (at least 1K entries). */

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        &numEntriesEgressStage
    );
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* configure memory mode */
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum, PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E,
                0,0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d", prvTgfDevNum,
                                     PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E);
        /* Aldrin has only 2K IPLR entries and POLICER_MEMORY_CTRL_MODE_4 provides only 512 entries for stage_1.
           It's not enough to use FULL index mode for port based mirroring.
           Change index mode to be COMPRESSED for Stage_1. This mode is OK for all SIP_5 devices also. */
        rc = prvTgfPolicerPortModeAddressSelectGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &defaultAddrSelTypeStage_1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectGet: %d, %d", prvTgfDevNum,
                                     PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        rc = prvTgfPolicerPortModeAddressSelectSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectGet: %d, %d", prvTgfDevNum,
                                     PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }
    else
    {
        totalNumEntries =
            orig_numEntriesIngressStage0+
            orig_numEntriesIngressStage1+
            numEntriesEgressStage;
        /* configure memory mode */
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum, PRV_TGF_POLICER_MEMORY_FLEX_MODE_E,
                (totalNumEntries/2)+50,
                (totalNumEntries/4)+50);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d", prvTgfDevNum,
                                     PRV_TGF_POLICER_MEMORY_FLEX_MODE_E);

        rc = prvTgfPolicerPortModeAddressSelectGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &defaultAddrSelTypeStage_1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectGet: %d, %d", prvTgfDevNum,
                                     PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }
}

/**
* @internal prvTgfPolicerPhyPortMeterIndexCalc function
* @endinternal
*
* @brief   Calculate meter entry index for physical port metering
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer stage
* @param[in,out] stormTypeOffsetPtr       - storm control policer index (may be changed if found too big)
* @param[in] portIdx                  - port index
* @param[in,out] stormTypeOffsetPtr       - storm control policer index (may be changed if found too big)
*                                       None
*/
static GT_U32 prvTgfPolicerPhyPortMeterIndexCalc
(
    IN GT_U8                            devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   stage,
    INOUT GT_U32                       *stormTypeOffsetPtr,
    IN GT_U32                           portIdx
)
{
    GT_U32      meterEntryIdx; /* meter entry index */
    PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT   addrSelType;
    GT_U32      numPorts;
    GT_U32      stormTypeOffset = *stormTypeOffsetPtr;
    GT_STATUS   rc;

    rc = prvTgfPolicerPortModeAddressSelectGet(devNum, stage, &addrSelType);
    if(rc != GT_OK)
    {
        GT_CHAR     errorMsg[256]; /* error message string */
        cpssOsSprintf(errorMsg,
                      "prvTgfPolicerPortModeAddressSelectGet: %d, %d",prvTgfDevNum, stage);
        (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
        return 0xFFFFFFFF;
    }

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* storm type is ignored for Egress Policer */
        stormTypeOffset = 0;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* 10 bits used (Independent of L2I mode) */
        numPorts = 1024;
    }
    else
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* 9 bits used (also for Aldrin2 that supports 128 physical ports !) */
        numPorts = 512;
    }
    else
    {
        numPorts = 256;
    }

    if (addrSelType == PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E)
    {
        meterEntryIdx = (numPorts * stormTypeOffset) + prvTgfPortsArray[portIdx];
        if ((meterEntryIdx >= PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[stage]) &&
            (PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[stage] > prvTgfPortsArray[portIdx]))
        {
            stormTypeOffset = (PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[stage] - prvTgfPortsArray[portIdx] - 1) / numPorts;
            meterEntryIdx = (numPorts * stormTypeOffset) + prvTgfPortsArray[portIdx];
            *stormTypeOffsetPtr = stormTypeOffset;
        }
    }
    else
    {
        meterEntryIdx = stormTypeOffset + (prvTgfPortsArray[portIdx] << 2);
    }

    return meterEntryIdx;
}

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanDisabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW .
*         2. Enable Billing.
*         3. Disable eVlan and ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10.
*         enable unicast
*         Configure unicast offset to 1 -> Metering Entry [201]
*         5. Enable port index 1 for metering.
*         6. Configure index 3 for Unicast traffic for port 8 -> the index in
*         the Meter Table should be 200 (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to eVlan: cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 1 and ebs = 10304.
*         9. Send one unicast packet from port port index 1 with vlan 5.
*         10. Check management counters: one packet should be yellow.
*         11. Check Billing counters on index matching to physical port: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         12. Check Billing counters matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanDisabledTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      eVlanPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      portIdx;            /* port index                           */
    GT_U32      stormTypeOffset;    /* storm control policer index          */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT policerMode; /* policer mode       */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */

    receivePortIdx = PRV_TGF_PORT_IDX_2_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_1_CNS;
    stormTypeOffset = 3;
    eOffset = 1;
    policerMode = PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E;
    policerWorkModeIsEvlan = GT_FALSE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 2;

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portIdx = receivePortIdx;
    }
    else
    {
        portIdx = sendPortIdx;
    }

#ifdef CHX_FAMILY
    eVlanPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 10);
#else
    eVlanPolicerIndex = 10;
#endif

    /* Set metering indexes, one matches Physical port and the other eVlan index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] =
        prvTgfPolicerPhyPortMeterIndexCalc(prvTgfDevNum, stage, &stormTypeOffset, portIdx);

    policerStageInfoArr[stage].meterEntryIdxArr[1] = eVlanPolicerIndex + eOffset;

    /* meter entry 0 */
    cir[0] = 1;
    cbs[0] = 1;
    ebs[0] = 10304;
    /* meter entry 1 */
    cir[1] = 1;
    cbs[1] = 1;
    ebs[1] = 1;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing entry for Port Metering, expected 64 yellow bytes */
        /* Billing entry for eVlan metering, expected all bytes 0 */
        expectBillingEntriesArr[stage][0].yellowCntr.l[0] = (ebs[0] - 10240);

    }

    /* management counters should count yellow packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].packetMngCntr = 1;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].duMngCntr.l[0] =
        (ebs[0] - 10240);
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_5_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst1Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, /* management counter set */
        policerMode, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_TRUE, /* enable/disable port metering */
        stormTypeOffset,/* stormTypeIndex */
        GT_FALSE, /* enable/disable eVlan */
        GT_FALSE, /* enable/disable ePort */
        eVlanPolicerIndex, /* eVlan policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_FALSE, /* enable/disable tti entry */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* tti policer index */
        GT_FALSE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 500:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Enable port index 1 for metering.
*         6. Configure index 3 for Unicast traffic for port 8 -> the index in
*         the Meter Table should be 200 (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to eVlan : cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 1 and ebs = 10304.
*         9. Send one unknown unicast packet from port index 1 with vlan 5.
*         10. Check management counters: one packet should be red.
*         11. Check Billing counters on entry matching to eVlan: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         12. Check Billing counters on matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanEnabledTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      eVlanPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      portIdx;            /* port index                           */
    GT_U32      stormTypeOffset;    /* storm control policer index          */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT policerMode; /* policer mode       */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */

    receivePortIdx = PRV_TGF_PORT_IDX_2_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_1_CNS;
    stormTypeOffset = 3;
    eOffset = 1;
    policerMode = PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E;
    policerWorkModeIsEvlan = GT_TRUE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 2;

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portIdx = receivePortIdx;
    }
    else
    {
        portIdx = sendPortIdx;
    }

#ifdef CHX_FAMILY
    eVlanPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 10);
#else
    eVlanPolicerIndex = 10;
#endif


    /* Set metering indexes, one matches Physical port and the other eVlan index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] =
        prvTgfPolicerPhyPortMeterIndexCalc(prvTgfDevNum, stage, &stormTypeOffset, portIdx);

    policerStageInfoArr[stage].meterEntryIdxArr[1] = eVlanPolicerIndex + eOffset;

    /* meter entry 0 */
    cir[0] = 1;
    cbs[0] = 1;
    ebs[0] = 10304;
    /* meter entry 1 */
    cir[1] = 1;
    cbs[1] = 1;
    ebs[1] = 1;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing counters entry 0, expected all bytes 0 */
        /* Billing counters entry 1, expected 64 red bytes */
        expectBillingEntriesArr[stage][1].redCntr.l[0] = 64;
    }

    /* management counters should count red packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].packetMngCntr = 1;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].duMngCntr.l[0] = 64;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_500_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst1Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET1_E, /* management counter set */
        policerMode, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_TRUE, /* enable/disable port metering */
        stormTypeOffset,/* stormTypeIndex */
        GT_TRUE, /* enable/disable eVlan */
        GT_FALSE, /* enable/disable ePort */
        eVlanPolicerIndex, /* eVlan policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_FALSE, /* enable/disable tti entry */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* tti policer index */
        GT_FALSE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Configure TTI action.
*         6. Configure TTI policer index to 50.
*         7. Configure Meter entry matching to eVlan : cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to TTI action: cir = 1, cbs = 10368 and ebs = 1.
*         9. Send two unknown unicast packets from port index 3 with vlan 5.
*         10. Check management counters: two packets should be green.
*         11. Check Billing counters on entry matching to TTI action: 128 bytes should be counted
*         for green packets and 128 bytes for other.
*         12. Check Billing counters on matching to eVlan: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabledTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      ttiPolicerIndex;    /* policer index in the TTI action      */
    GT_U32      eVlanPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */
    GT_STATUS   rc;

    receivePortIdx = PRV_TGF_PORT_IDX_0_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_3_CNS;
    eOffset = 6;
    policerWorkModeIsEvlan = GT_FALSE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 2;

    /* AUTODOC: set the Meter default mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* AUTODOC: set packet based meter resolution algorithm */
    rc = prvTgfPolicerMeterResolutionSet(stage,
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterResolutionSet: %d", prvTgfDevNum);

#ifdef CHX_FAMILY
    eVlanPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 10);
    ttiPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 20);
#else
    eVlanPolicerIndex = 10;
    ttiPolicerIndex   = 20;
#endif

    /* Set metering indexes, one matches Physical port and the other eVlan index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] = ttiPolicerIndex;
    policerStageInfoArr[stage].meterEntryIdxArr[1] = eVlanPolicerIndex + eOffset;

    /* meter entry 0, match TTI action*/
    cir[0] = 1;
    cbs[0] = 10368;
    ebs[0] = 1;
    /* meter entry 1, match eVlan Trigger entry */
    cir[1] = 1;
    cbs[1] = 1;
    ebs[1] = 1;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing counters entry 0, expected all bytes 0 */
        /* Billing counters entry 1, expected 64 red bytes */
        expectBillingEntriesArr[stage][0].greenCntr.l[0] = 128;
    }

    /* management counters should count red packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].packetMngCntr = 2;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].duMngCntr.l[0] = 128;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_5_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst2Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET1_E, /* management counter set */
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_FALSE, /* enable/disable port metering */
        0,/* stormTypeIndex */
        GT_TRUE, /* enable/disable eVlan */
        GT_FALSE, /* enable/disable ePort */
        eVlanPolicerIndex, /* eVlan policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_TRUE, /* enable/disable tti entry */
        ttiPolicerIndex, /* tti policer index */
        GT_FALSE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanPortModeEportEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to PORT .
*         2. Enable Billing.
*         3. Disable eVlan metering/counting.
*         4. Enable ePort metering/counting.
*         4. Configure ePort Trigger entry for ePort 300:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 40.
*         enable registered multicast
*         Configure registered multicast offset to 4.
*         5. Enable port index 2 for metering.
*         6. Configure index 3 for registered Multicast traffic for port index 2 -> the index in
*         the Meter Table should be (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to ePort: cir = 1, cbs = 1 and ebs = 10368 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 10368 and ebs = 1.
*         9. For Ingress, Configure TTI rule with ePort assignment: port index 2 to ePort 300.
*         10. For Egress, configure E2Phy table: 300 -> port index 0 (recieve port).
*         11. Send two registered multicast packets from port port index 2.
*         12. Check management counters: two packets should be yellow.
*         13. Check Billing counters on index matching to ePort: 128 bytes should be counted
*         for yellow packets and 0 bytes for other.
*         14. Check Billing counters matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanPortModeEportEnabledTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      ePortPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      ePortTrigerPortNum; /* ePort trigger number                 */
    GT_U32      portIdx;            /* port index                           */
    GT_U32      stormTypeOffset;    /* storm control policer index          */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT policerMode; /* policer mode       */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */

    receivePortIdx = PRV_TGF_PORT_IDX_0_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_2_CNS;
    stormTypeOffset = 3;
    eOffset = 4;
    ePortTrigerPortNum = 300;
    policerMode = PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E;
    policerWorkModeIsEvlan = GT_FALSE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 2;

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portIdx = receivePortIdx;
    }
    else
    {
        portIdx = sendPortIdx;
    }

#ifdef CHX_FAMILY
    ePortPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 40);
#else
    ePortPolicerIndex = 40;
#endif


    /* Set metering indexes, one matches Physical port and the other eVlan index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] =
        prvTgfPolicerPhyPortMeterIndexCalc(prvTgfDevNum, stage, &stormTypeOffset, portIdx);

    if (stage < PRV_TGF_POLICER_STAGE_NUM)
    {
        policerStageInfoArr[stage].meterEntryIdxArr[1] = ePortPolicerIndex;
    }
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
       policerStageInfoArr[stage].meterEntryIdxArr[1] += eOffset;
    }

    /* meter entry 0 */
    cir[0] = 1;
    cbs[0] = 10368;
    ebs[0] = 1;
    /* meter entry 1 */
    cir[1] = 1;
    cbs[1] = 1;
    ebs[1] = 10368;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing entry for Port Metering, expected 64 yellow bytes */
        /* Billing entry for eVlan metering, expected all bytes 0 */
        expectBillingEntriesArr[stage][1].yellowCntr.l[0] = (ebs[1] - 10240);

    }

    /* management counters should count yellow packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].packetMngCntr = 2;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].duMngCntr.l[0] =
        (ebs[1] - 10240);
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_YELLOW_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_5_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst2Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, /* management counter set */
        policerMode, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_TRUE, /* enable/disable port metering */
        stormTypeOffset,/* stormTypeIndex */
        GT_FALSE, /* enable/disable eVlan */
        GT_TRUE, /* enable/disable ePort */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* eVlan policer index */
        ePortPolicerIndex, /* ePort policer index */
        ePortTrigerPortNum, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_TRUE, /* enable/disable tti entry */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* tti policer index */
        GT_TRUE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanPortModeEportDisabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to PORT .
*         2. Enable Billing.
*         3. Disable eVlan metering/counting.
*         4. Disable ePort metering/counting.
*         4. Configure ePort Trigger entry for ePort 300:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 40.
*         enable registered multicast
*         Configure registered multicast offset to 4.
*         5. Enable port index 2 for metering.
*         6. Configure index 3 for registered Multicast traffic for port index 2 -> the index in
*         the Meter Table should be (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to ePort: cir = 1, cbs = 1 and ebs = 10368 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 10368 and ebs = 1.
*         9. For Ingress, Configure TTI rule with ePort assignment: port index 2 to ePort 300.
*         10. For Egress, configure E2Phy table: 300 -> port index 0 (recieve port).
*         11. Send two registered multicast packets from port port index 2.
*         12. Check management counters: two packets should be green.
*         13. Check Billing counters on index matching physical port: 128 bytes should be counted
*         for green packets and 0 bytes for other.
*         14. Check Billing counters matching ePort: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanPortModeEportDisabledTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      ePortPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      ePortTrigerPortNum; /* ePort trigger number                 */
    GT_U32      portIdx;            /* port index                           */
    GT_U32      stormTypeOffset;    /* storm control policer index          */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT policerMode; /* policer mode       */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */

    receivePortIdx = PRV_TGF_PORT_IDX_0_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_2_CNS;
    stormTypeOffset = 1;
    eOffset = 4; /* Disable */
    ePortTrigerPortNum = 300;
    policerMode = PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E;
    policerWorkModeIsEvlan = GT_FALSE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 2;


    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portIdx = receivePortIdx;
    }
    else
    {
        portIdx = sendPortIdx;
    }

#ifdef CHX_FAMILY
    ePortPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 40);
#else
    ePortPolicerIndex = 40;
#endif

    /* Set metering indexes, one matches Physical port + stormType offset
       and the other ePort index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] =
        prvTgfPolicerPhyPortMeterIndexCalc(prvTgfDevNum, stage, &stormTypeOffset, portIdx);

    policerStageInfoArr[stage].meterEntryIdxArr[1] = ePortPolicerIndex + eOffset;

    /* meter entry 0 */
    cir[0] = 1;
    cbs[0] = 10368;
    ebs[0] = 1;
    /* meter entry 1 */
    cir[1] = 1;
    cbs[1] = 1;
    ebs[1] = 10368;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing entry for Port Metering, expected 64 green bytes */
        /* Billing entry for eVlan metering, expected all bytes 0 */
        expectBillingEntriesArr[stage][0].greenCntr.l[0] = (cbs[0] - 10240);

    }

    /* management counters should count green packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].packetMngCntr = 2;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].duMngCntr.l[0] =
        (ebs[1] - 10240);
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_GREEN_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_5_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst2Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET2_E, /* management counter set */
        policerMode, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_TRUE, /* enable/disable port metering */
        stormTypeOffset,/* stormTypeIndex */
        GT_FALSE, /* enable/disable eVlan */
        GT_TRUE, /* enable/disable ePort */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* eVlan policer index */
        ePortPolicerIndex, /* ePort policer index */
        ePortTrigerPortNum, /* ePort trigger number */
        GT_FALSE, /* trigger entry counting enable */
        GT_FALSE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_TRUE, /* enable/disable tti entry */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* tti policer index */
        GT_TRUE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypesTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW .
*         2. Enable Billing.
*         3. Enable eVlan metering/counting.
*         4. Disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for eVlan 500:
*         Enable metering
*         Enable counting
*         Configure police Index.
*         Note: Each time all offsets are configured (they all differ),
*         so each one points to another metering entry.
*         5. Configure 6 metering entries, matching each offset, so that the
*         one matching givn packet type should color one packet red, where
*         others should color all packets green.
*         6. Send one specific packet type from port port index 3.
*         7. Check management counters: one packets should be red.
*         8. Check Billing counters on index matching policer index + given offset: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         9. Check Billing counters matching other metering entries: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
* @param[in] pktTypeToSend            - packet type to send
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypesTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT pktTypeToSend
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      eVlanPolicerIndex;  /* policer index in the Trigger entry   */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */
    GT_U32      expectedPacketLength;       /* length of single packet to send      */

    receivePortIdx = PRV_TGF_PORT_IDX_1_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_3_CNS;
    policerWorkModeIsEvlan = GT_TRUE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = (PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E + 1);


#ifdef CHX_FAMILY
    eVlanPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 10);
#else
    eVlanPolicerIndex = 10;
#endif

    if (pktTypeToSend == PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E)
    {
        expectedPacketLength = PRV_TGF_IPV4_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS;
    }
    else
    {
        expectedPacketLength = PRV_TGF_PACKET_TAG_CRC_LEN_CNS;
    }

    /* Set 6 metering indexes, each one matches one eVlan Trigger entry offset */
    for (eOffset = 0; eOffset < policerStageInfoArr[stage].numOfMeterEntries; eOffset++)
    {
        policerStageInfoArr[stage].meterEntryIdxArr[eOffset] =
            eVlanPolicerIndex + eOffset;

        if (((eOffset == (GT_U32)pktTypeToSend) &&
                (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)) ||
            (eOffset == 0 && (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)))
        {
            cir[eOffset] = 1;
            cbs[eOffset] = 1;
            ebs[eOffset] = 1;

            /* Configure expected metering/counting results */

            /* In Egress stage, billing is done after remarking (remarking should be configured) */
            if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
            {
                /* Billing entry for specific traffic type, eVlan Metering,
                   expected 64 red bytes */
                expectBillingEntriesArr[stage][eOffset].redCntr.l[0] =
                    (prvTgfBurst1Count * expectedPacketLength);
            }
        }
        else
        {
            cir[eOffset] = 1;
            cbs[eOffset] = 10368;
            ebs[eOffset] = 1;

            /* Billing entry for other traffic types, eVlan metering,
               expected all bytes 0 */
        }
    }

    /* management counters should count red packets */
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].packetMngCntr = prvTgfBurst1Count;
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].duMngCntr.l[0] =
        (prvTgfBurst1Count * expectedPacketLength);
    expectManagEntriesArr[stage][PRV_TGF_POLICER_MNG_CNTR_RED_E].duMngCntr.l[1] = 0;
    expectDropPkts = 0;

    /* set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_500_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst1Count, /* numOfPackets */
        pktTypeToSend, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_SET1_E, /* management counter set */
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_TRUE, /* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_FALSE, /* enable/disable port metering */
        0,/* stormTypeIndex */
        GT_TRUE, /* enable/disable eVlan */
        GT_FALSE, /* enable/disable ePort */
        eVlanPolicerIndex, /* eVlan policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort policer index */
        0, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        ALL_OFFSETS_IN_TRIGGER_ENTRY_CONFIG_CNS, /* all offsets should be enabled */
        GT_FALSE, /* enable/disable tti entry */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* tti policer index */
        GT_FALSE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);
}

/**
* @internal prvTgfPolicerEportEvlanGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerEportEvlanGlobalConfigurationRestore
(
    void
)
{
    GT_STATUS   rc;

    /****************/
    /* Tables Reset */
    /****************/

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_500_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_500_CNS);

    /* restore memory mode */
    rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum, defaultMemMode,
        orig_numEntriesIngressStage0,
        orig_numEntriesIngressStage1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d", prvTgfDevNum,
                                 defaultMemMode);

    rc = prvTgfPolicerPortModeAddressSelectSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, defaultAddrSelTypeStage_1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectGet: %d, %d", prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_1_E);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfPolicerEportEvlanConfigurationRestore function
* @endinternal
*
* @brief   Restore Policer per stage test configuration
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanConfigurationRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      ii;


    /* Restore the Meter default mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        defaultMeterMode[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage, defaultMeterMode[stage]);

    /* Enable metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* Set metered Packet Size Mode as CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E */
    rc = prvTgfPolicerPacketSizeModeSet(stage, CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", stage);

    /* Restore counting mode default mode*/
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                      defaultCountMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d %d",
                                 prvTgfDevNum, stage, defaultCountMode[stage]);

    /* Disable ePort/eVlan meter mode */
    rc = prvTgfPolicerEAttributesMeteringModeSet(prvTgfDevNum, stage,
                                 PRV_TGF_POLICER_E_ATTR_METER_MODE_DISABLED_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterModeEnableSet: %d, %d",
                                 prvTgfDevNum, stage);

    /* Disable Port Triggered counting */
    rc = prvTgfPolicerCountingTriggerByPortEnableSet(prvTgfDevNum, stage, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingTriggerByPortEnableSet: %d, %d",
                                 prvTgfDevNum, stage);

    /* Clear all management counters */
    for (ii=PRV_TGF_POLICER_MNG_CNTR_SET0_E; ii <= PRV_TGF_POLICER_MNG_CNTR_SET2_E; ii++)
    {
        prvTgfPolicerEportEvlanManagementCountersReset(stage, ii);
    }
}

/**
* @internal prvTgfPolicerPclEvlanMatchConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress PCL configuration
*/
static GT_VOID prvTgfPolicerPclEvlanMatchConfigurationIpclAdd
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U32                           i;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for send port 3: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

    /* Ingress rule */
    /* Forward */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_IPCL_RULE_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    /* configures both bits[11:0] in the key */
    /* and bits[15:12] in the extension      */
    mask.ruleStdNotIp.common.vid    = 0xFFF;
    pattern.ruleStdNotIp.common.vid = PRV_TGF_VLANID_5_CNS;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleStdNotIp.macDa.arEther[i] = 0xFF;
        pattern.ruleStdNotIp.macDa.arEther[i] =
            prvTgfUcastUnknownMac[i];
    }

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:00:00:00:11:AA, VID=5 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPolicerPclEvlanMatchIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPolicerPclEvlanMatchIpclRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_PORT_IDX_3_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_IPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPolicerEportEvlanFlowModeTtiActionBillingCountersTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Disable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Configure TTI action: meter disable, counter enable
*         6. Configure TTI policer index to 50.
*         7. Configure Meter entry matching to TTI action: cir = 1, cbs = 10368 and ebs = 1.
*         8. Send one unknown unicast packet from port index 3 with vlan 5.
*         9. Check management counters: no packets.
*         10. Check Billing counters on entry matching to TTI action: 64 bytes should be counted
*         for green packets and 0 bytes for other.
*         11. Check Billing counters on matching to eVlan: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeTtiActionBillingCountersTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_U32      receivePortIdx;     /* sending port index                   */
    GT_U32      sendPortIdx;        /* receiving port index                 */
    GT_U32      ttiPolicerIndex;    /* policer index in the TTI action      */
    GT_U32      eOffset;            /* policer offset in the Trigger entry  */
    GT_U32      cir[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cir values  */
    GT_U32      cbs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of cbs values  */
    GT_U32      ebs[MAX_NUM_OF_METER_ENTRIES_CNS];  /* array of ebs values  */
    GT_STATUS   rc;

    receivePortIdx = PRV_TGF_PORT_IDX_0_CNS;
    sendPortIdx = PRV_TGF_PORT_IDX_3_CNS;
    eOffset = 6;
    policerWorkModeIsEvlan = GT_FALSE;

    /* initialize arrays */
    prvTgfPolicerEportEvlanInitAllStages(stage);
    policerStageInfoArr[stage].numOfMeterEntries = 1;

    /* Set the Meter default mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* AUTODOC: set packet based meter resolution algorithm */
    rc = prvTgfPolicerMeterResolutionSet(stage,
                                         PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterResolutionSet: %d", prvTgfDevNum);

#ifdef CHX_FAMILY
    ttiPolicerIndex =
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[stage] - 20);
#else
    ttiPolicerIndex   = 20;
#endif

    /* Set metering indexes, one matches Physical port and the other eVlan index */
    policerStageInfoArr[stage].meterEntryIdxArr[0] = ttiPolicerIndex;

    /* meter entry 0, match TTI action*/
    cir[0] = 1;
    cbs[0] = 10368;
    ebs[0] = 1;

    /* Configure expected metering/counting results */

    /* In Egress stage, billing is done after remarking (remarking should be configured) */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Billing counters entry 0, expected 64 green bytes */
        expectBillingEntriesArr[stage][0].greenCntr.l[0] = 64;
    }

    /* AUTODOC: add PCL configuration */
    prvTgfPolicerPclEvlanMatchConfigurationIpclAdd();

    prvTgfPolicerEportEvlanCfgAndTrafficGenerate(
        stage,
        PRV_TGF_VLANID_5_CNS, /* vid */
        sendPortIdx,
        receivePortIdx,
        prvTgfBurst1Count, /* numOfPackets */
        PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E, /* packetTypeToSend */
        PRV_TGF_POLICER_MNG_CNTR_DISABLED_E, /* management counter set */
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E, /* policer mode */
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, /* count mode */
        GT_FALSE,/* enable/disable metering in meter entry */
        GT_TRUE, /* enable/disable counting in meter entry */
        GT_FALSE, /* enable/disable port metering */
        0,/* stormTypeIndex */
        GT_FALSE, /* enable/disable eVlan */
        GT_FALSE, /* enable/disable ePort */
        0, /* eVlan policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort policer index */
        PRV_TGF_NOT_EXIST_POLICER_INDEX_CNS, /* ePort trigger number */
        GT_TRUE, /* trigger entry counting enable */
        GT_TRUE, /* trigger entry metering enable */
        eOffset, /* trigger entry policer index offset */
        GT_TRUE, /* enable/disable tti entry */
        ttiPolicerIndex, /* tti policer index */
        GT_FALSE, /* enable/disable tti ePort assignment */
        cir,
        cbs,
        ebs);

    /* AUTODOC: restore PCL configuration */
    prvTgfPolicerPclEvlanMatchIpclRestore();
}


