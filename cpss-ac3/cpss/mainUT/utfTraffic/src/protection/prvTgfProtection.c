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
* @file prvTgfProtection.c
*
* @brief Protection switching tests functions
*
* @version   6
********************************************************************************
*/
#include <common/tgfProtectionGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfConfigGen.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/* number of ports */
#define PRV_TGF_TX_SIDE_PORTS_COUNT_CNS                     3
#define PRV_TGF_RX_SIDE_PORTS_COUNT_CNS                     4

/* Global ePort */
#define PRV_TGF_GLOBAL_EPORT_IDX_CNS                        1018

/* Multi-target ePort value and mask */
#define PRV_TGF_GLOBAL_EPORT_VALUE_CNS                      PRV_TGF_GLOBAL_EPORT_IDX_CNS
#define PRV_TGF_GLOBAL_EPORT_MASK_CNS                       PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(prvTgfDevNum)

/* MLL pair entry index */
#define PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS                    12

/* TX ports */
#define PRV_TGF_TX_WORKING_EPORT_IDX_CNS                    1000
#define PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS                 (PRV_TGF_TX_WORKING_EPORT_IDX_CNS + 1)

/* RX ports */
#define PRV_TGF_RX_EPORT_IDX_CNS                            1002

/* TX tests */
#define PRV_TGF_INJECTION_PORT_IDX_CNS                      0
#define PRV_TGF_TX_WORKING_PORT_IDX_CNS                     1
#define PRV_TGF_TX_PROTECTION_PORT_IDX_CNS                  2

/* RX test */
#define PRV_TGF_RX_WORKING_PORT_IDX_CNS                     0
#define PRV_TGF_RX_PROTECTION_PORT_IDX_CNS                  1
#define PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS      2
#define PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS   3

/* Rule index for working path */
#define PRV_TGF_WORKING_PATH_RULE_INDEX_CNS                 80

/* Rule index for protection path */
#define PRV_TGF_PROTECTION_PATH_RULE_INDEX_CNS              81

/* LOC table index for the TX working ePort */
#define PRV_TGF_TX_WORKING_EPORT_LOC_TABLE_IDX_CNS          (1987 % PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesProtectionLoc)

/* LOC table index for the RX ePort */
#define PRV_TGF_RX_EPORT_LOC_TABLE_IDX_CNS                  (591 % PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesProtectionLoc)

/* Multi-target ePort base */
#define PRV_TGF_MULTI_TARGET_EPORT_BASE_CNS                 50

/* Multi-target eVidx base */
#define PRV_TGF_MULTI_TARGET_EVIDX_BASE_CNS                 4226

/* Multi-target protection switching eVidx */
#define PRV_TGF_PROTECTION_TX_EVIDX_CNS                                        \
    (PRV_TGF_MULTI_TARGET_EVIDX_BASE_CNS + PRV_TGF_GLOBAL_EPORT_IDX_CNS -      \
     PRV_TGF_MULTI_TARGET_EPORT_BASE_CNS)

#define PRV_TGF_MLL_LTT_INDEX_CNS       (PRV_TGF_PROTECTION_TX_EVIDX_CNS - 4096)

/* VLAN id */
#define PRV_TGF_VLAN_ID_CNS                                 2

/* Burst packets number */
static GT_U32 prvTgfBurstCount = 1;

/* FDB entry MAC address */
static TGF_MAC_ADDR prvTgfFdbEntryMacAddr = {0x00, 0x00, 0x00, 0x00, 0x34, 0x02};

static CPSS_NET_RX_CPU_CODE_ENT     rxExpectedCpuCode;

static struct
{
    /* Relevant for all the tests */
    GT_BOOL                             protectionEnabled;

    /* Relevant only for TX tests */
    CPSS_INTERFACE_INFO_STC             txWorkingEportMapping;
    CPSS_INTERFACE_INFO_STC             txProtectionEportMapping;

    /* Relevant only for TX 1:1 tests*/
    GT_BOOL                             txProtectionEnabled;
    GT_U32                              locTableIndexForTxWorkingEport;

    /* Relevant only for TX 1+1 tests*/
    PRV_TGF_CFG_GLOBAL_EPORT_STC        ecmpEportCfg;
    PRV_TGF_CFG_GLOBAL_EPORT_STC        trunkEportCfg;
    PRV_TGF_CFG_GLOBAL_EPORT_STC        globalEportCfg;
    GT_BOOL                             multiTargetPortEnable;
    GT_U32                              globalEportValue;
    GT_U32                              globalEportMask;
    GT_U32                              eportBase;
    GT_U32                              vidxBase;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC        lttEntry;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;

    /* Relevant only for TX 1:1 and RX tests */
    PRV_TGF_PROTECTION_LOC_STATUS_ENT   status;

    /* Relevant only for RX tests */
    CPSS_PACKET_CMD_ENT                 command;
    CPSS_NET_RX_CPU_CODE_ENT            cpuCode;
    GT_U32                              locTableIndexForRxEport;
    GT_BOOL                             lookupEnableForWorking;
    GT_BOOL                             lookupEnableForProtection;
    PRV_TGF_TTI_MAC_MODE_ENT            macMode;
} prvTgfRestoreCfg;

/*********************************** Packet ***********************************/

/* L2 */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* DA MAC */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* SA MAC */
};

/* VLAN tag */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_ID_CNS                           /* pri, cfi, VlanId */
};

/* Ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x73CD*/
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* Data */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* Parts of the packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/************************************************************************************************************************************************************
*                                                                                                                                                           *
*   TX test                                                                                     RX test                                                     *
*   -------                                                                                     -------                                                     *
*                                                                                                                                                           *
*   *********************************************                                                            *********************************************  *
*   *                                           *                                                    port 18 *                                           *  *
*   *                                           *                                           <----------------------                                      *  *
*   *                                           *                                                            *     |                                     *  *
*   *  *******              *********           * ePort 1000 (physical port 8)                       port 0  *     |                                     *  *
*   *  *     *              *       * ------------------>------------ working path -------->-----------------------                                      *  *
*   *  *     *              *       *           *                                                            *                                           *  *
*   *  *     *              *       *           *                                                ePort 1002  *                                           *  *
*   *  * FDB * -----------> * eVidx *           *                                                            *                                           *  *
*   *  *     *    global    *  98   * ------------------>---------- protection path ------->-----------------------                                      *  *
*   *  *     *     ePort    *       *           * ePort 1001 (physical port 18)                      port 8  *     |                                     *  *
*   *  *     *     1018     *       *           *                                                            *     |                                     *  *
*   *  *******              *********           *                                           <----------------------                                      *  *
*   *    /|\                                    *                                                    port 23 *                                           *  *
*   *     |                                     *                                                            *                                           *  *
*   *     |                                     *                                                            *                                           *  *
*   *     |                                     * port 0                                                     *                                           *  *
*   *     |-------------------------------------------------                                                 *                                           *  *
*   *                                           *                                                            *                                           *  *
*   *********************************************                                                            *********************************************  *
*                                                                                                                                                           *
************************************************************************************************************************************************************/

/**
* @internal prvTgfProtectionTxBridgeConfigurationSet function
* @endinternal
*
* @brief   TX protection switching bridge configurations:
*         - Set VLAN entry
*         - Add ports to VLAN
*         - Set MAC entry
*         - Set E2Phy entries
* @param[in] oneToOne                 - GT_TRUE:  1:1 configuration test
*                                      GT_FALSE: 1+1 configuration test
*                                       None
*/
GT_VOID prvTgfProtectionTxBridgeConfigurationSet
(
    GT_BOOL     oneToOne
)
{
    CPSS_PORTS_BMP_STC                          portsMembers;
    CPSS_PORTS_BMP_STC                          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC                   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC          portsTaggingCmd;
    GT_PORT_NUM                                 dstPortNum;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    GT_U32                                      i;
    GT_STATUS                                   rc;

    /* AUTODOC: Set VLAN entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(PRV_TGF_BRG_VLAN_INFO_STC));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    cpssOsMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 PRV_TGF_VLAN_ID_CNS,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", PRV_TGF_VLAN_ID_CNS);

    /* AUTODOC: Add ports to VLAN */
    for (i = 0; i < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                    prvTgfPortsArray[i], GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                     prvTgfPortsArray[i], GT_TRUE);
    }

    if (oneToOne)
    {
        dstPortNum = PRV_TGF_TX_WORKING_EPORT_IDX_CNS;
    }
    else
    {
        dstPortNum = PRV_TGF_GLOBAL_EPORT_IDX_CNS;
    }

    /* AUTODOC: Set MAC entry */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfFdbEntryMacAddr,
                                          PRV_TGF_VLAN_ID_CNS,
                                          prvTgfDevNum,
                                          dstPortNum,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefFdbMacEntryOnPortSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store E2Phy entries */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                                           &prvTgfRestoreCfg.txWorkingEportMapping);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS,
                                                           &prvTgfRestoreCfg.txProtectionEportMapping);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS);

    /* AUTODOC: Set E2Phy entries */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS);
}

/**
* @internal prvTgfProtectionRxBridgeConfigurationSet function
* @endinternal
*
* @brief   RX protection switching bridge configurations
*/
GT_VOID prvTgfProtectionRxBridgeConfigurationSet
(
    GT_VOID
)
{
    CPSS_PORTS_BMP_STC                          portsMembers;
    CPSS_PORTS_BMP_STC                          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC                   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC          portsTaggingCmd;
    GT_U32                                      i;
    GT_STATUS                                   rc;

    /* AUTODOC: Set VLAN entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(PRV_TGF_BRG_VLAN_INFO_STC));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    cpssOsMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 PRV_TGF_VLAN_ID_CNS,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", PRV_TGF_VLAN_ID_CNS);

    /* AUTODOC: Add ports to VLAN */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                    prvTgfPortsArray[i], GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                     prvTgfPortsArray[i], GT_TRUE);
    }
}

/**
* @internal prvTgfProtectionEnableConfigSet function
* @endinternal
*
* @brief   Enable or disable protection switching
*
* @param[in] enable                   - enable/disable protection switching
* @param[in] store                    -  the original enabling status
*                                       None
*/
GT_VOID prvTgfProtectionEnableConfigSet
(
    GT_BOOL         enable,
    GT_BOOL         store
)
{
    GT_STATUS       rc;

    if (store == GT_TRUE)
    {
        /* AUTODOC: Store protection switching enabling status */
        rc = prvTgfProtectionEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.protectionEnabled);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfProtectionEnableGet: %d",
                                     prvTgfDevNum);
    }

    /* AUTODOC: Set protection switching enabling status */
    rc = prvTgfProtectionEnableSet(prvTgfDevNum, enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionEnableSet: %d, %d",
                                 prvTgfDevNum, enable);
}

/**
* @internal prvTgfProtectionLocStatusConfigSet function
* @endinternal
*
* @brief   Set the LOC table status bits
*
* @param[in] direction                - CPSS_DIRECTION_INGRESS_E: for RX protection switching
*                                      CPSS_DIRECTION_EGRESS_E:  for TX protection switching
* @param[in] workingPathStatus        - LOC status on the working port
* @param[in] store                    -  the original enabling status
*                                       None
*/
GT_VOID prvTgfProtectionLocStatusConfigSet
(
    CPSS_DIRECTION_ENT                      direction,
    PRV_TGF_PROTECTION_LOC_STATUS_ENT       workingPathStatus,
    GT_BOOL                                 store
)
{
    GT_STATUS                           rc;
    GT_U32                              index;

    if (direction == CPSS_DIRECTION_INGRESS_E)
    {
        index = PRV_TGF_RX_EPORT_LOC_TABLE_IDX_CNS;
    }
    else    /* CPSS_DIRECTION_EGRESS_E */
    {
        index = PRV_TGF_TX_WORKING_EPORT_LOC_TABLE_IDX_CNS;
    }

    if (store == GT_TRUE)
    {
        /* AUTODOC: Store the LOC status */
        rc = prvTgfProtectionLocStatusGet(prvTgfDevNum, index, &prvTgfRestoreCfg.status);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfProtectionLocStatusGet: %d",
                                     prvTgfDevNum);
    }

    rc = prvTgfProtectionLocStatusSet(prvTgfDevNum, index, workingPathStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfProtectionLocStatusSet");
}

/**
* @internal prvTgfProtectionRxExceptionConfigSet function
* @endinternal
*
* @brief   Set protection RX exception packet command and CPU/drop code
*
* @param[in] command                  - the protection RX exception command
* @param[in] cpuCode                  - the CPU/drop code. relevant if command is not
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                       None
*/
GT_VOID prvTgfProtectionRxExceptionConfigSet
(
    CPSS_PACKET_CMD_ENT                     command,
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode
)
{
    GT_STATUS   rc;

    /* No need to store the packet command, because it's saved in
       prvTgfProtectionRxConfigurationSet */

    /* AUTODOC: Set protection RX exception packet command */
    rc = prvTgfProtectionRxExceptionPacketCommandSet(prvTgfDevNum, command);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionPacketCommandSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store protection RX exception CPU/drop code */
    rc = prvTgfProtectionRxExceptionCpuCodeGet(prvTgfDevNum, &prvTgfRestoreCfg.cpuCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionCpuCodeGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set protection RX exception CPU/drop code */
    rc = prvTgfProtectionRxExceptionCpuCodeSet(prvTgfDevNum, cpuCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionCpuCodeSet: %d",
                                 prvTgfDevNum);

    rxExpectedCpuCode = cpuCode;
}

/**
* @internal prvTgfProtectionOnePlusOneTxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1+1 TX configurations:
*         - Set global ePort configuration
*         - Enable multi-target port mapping
*         - Set the multi-target port value and mask
*         - Set the multi-target port base
*         - Set the port to VIDX base
*         - Configure MLL LTT entry
*         - Configure L2 MLL entry with 1+1 protection enabled
*/
GT_VOID prvTgfProtectionOnePlusOneTxConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc;
    PRV_TGF_L2_MLL_PAIR_STC                     mllPairEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                ecmpEportCfg;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                globalEportCfg;

    /* AUTODOC: Store ECMP ePort and global ePort configuration */
    rc = prvTgfCfgGlobalEportGet(prvTgfDevNum,
                                 &prvTgfRestoreCfg.globalEportCfg,
                                 &prvTgfRestoreCfg.ecmpEportCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfCfgGlobalEportGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set global ePort configuration */
    ecmpEportCfg.enable = GT_FALSE;
    globalEportCfg.enable = GT_TRUE;
    globalEportCfg.pattern = PRV_TGF_GLOBAL_EPORT_IDX_CNS;
    globalEportCfg.mask = (GT_U32) -1;
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &globalEportCfg , &ecmpEportCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfCfgGlobalEportSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store the multi-target port mapping enabling status */
    rc = prvTgfL2MllMultiTargetPortEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.multiTargetPortEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortEnableGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Enable multi-target port mapping */
    rc = prvTgfL2MllMultiTargetPortEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store the multi-target port value and mask */
    rc = prvTgfL2MllMultiTargetPortGet(prvTgfDevNum,
                                       &prvTgfRestoreCfg.globalEportValue,
                                       &prvTgfRestoreCfg.globalEportMask);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set the multi-target port value and mask */
    rc = prvTgfL2MllMultiTargetPortSet(prvTgfDevNum,
                                       PRV_TGF_GLOBAL_EPORT_VALUE_CNS,
                                       PRV_TGF_GLOBAL_EPORT_MASK_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store the multi-target port base */
    rc = prvTgfL2MllMultiTargetPortBaseGet(prvTgfDevNum, &prvTgfRestoreCfg.eportBase);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortBaseGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set the multi-target port base */
    rc = prvTgfL2MllMultiTargetPortBaseSet(prvTgfDevNum, PRV_TGF_MULTI_TARGET_EPORT_BASE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortBaseSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store the port to VIDX base */
    rc = prvTgfL2MllPortToVidxBaseGet(prvTgfDevNum, &prvTgfRestoreCfg.vidxBase);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllPortToVidxBaseGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set the port to VIDX base */
    rc = prvTgfL2MllPortToVidxBaseSet(prvTgfDevNum, PRV_TGF_MULTI_TARGET_EVIDX_BASE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllPortToVidxBaseSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store MLL LTT entry */
    rc = prvTgfL2MllLttEntryGet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS,
                                &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntryGet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Configure MLL LTT entry */
    lttEntry.mllPointer = PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS;
    lttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    lttEntry.mllMaskProfileEnable = GT_FALSE;
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Store L2 MLL entry */
    rc = prvTgfL2MllPairRead(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: %d", prvTgfDevNum);

    /* AUTODOC: Configure L2 MLL entry with 1+1 protection enabled */
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    cpssOsMemSet(&mllPairEntry.firstMllNode, 0, sizeof(PRV_TGF_L2_MLL_ENTRY_STC));
    mllPairEntry.firstMllNode.unknownUcFilterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_TGF_TX_WORKING_EPORT_IDX_CNS;
    mllPairEntry.firstMllNode.mcLocalSwitchingEnable = GT_FALSE;
    mllPairEntry.firstMllNode.onePlusOneFilteringEnable = GT_TRUE;
    mllPairEntry.firstMllNode.last= GT_FALSE;
    cpssOsMemSet(&mllPairEntry.secondMllNode, 0, sizeof(PRV_TGF_L2_MLL_ENTRY_STC));
    mllPairEntry.secondMllNode.unknownUcFilterEnable = GT_TRUE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS;
    mllPairEntry.secondMllNode.mcLocalSwitchingEnable = GT_FALSE;
    mllPairEntry.secondMllNode.onePlusOneFilteringEnable = GT_TRUE;
    mllPairEntry.secondMllNode.last= GT_TRUE;
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfProtectionOneToOneTxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1:1 TX configurations:
*         - Enable TX protection switching for the target eport
*         - Map between the target eport to the LOC table
*/
GT_VOID prvTgfProtectionOneToOneTxConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc;

    /* AUTODOC: Store TX protection enabling status */
    rc = prvTgfProtectionTxEnableGet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                     &prvTgfRestoreCfg.txProtectionEnabled);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionTxEnableGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    /* AUTODOC: Enable TX protection switching */
    rc = prvTgfProtectionTxEnableSet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfProtectionTxEnableSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS, GT_TRUE);

    /* AUTODOC: Store the mapping between the target eport to the LOC table */
    rc = prvTgfProtectionPortToLocMappingGet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                             &prvTgfRestoreCfg.locTableIndexForTxWorkingEport);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    /* AUTODOC: Map between the target eport to the LOC table */
    rc = prvTgfProtectionPortToLocMappingSet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                             PRV_TGF_TX_WORKING_EPORT_LOC_TABLE_IDX_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                 PRV_TGF_TX_WORKING_EPORT_LOC_TABLE_IDX_CNS);
}

/**
* @internal prvTgfProtectionRxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1+1 RX configurations:
*         - Enable TTI lookup for TTI_KEY_IPV4 on the RX ports
*         - Set MAC mode for IPv4 key
*         - Configure 2 TTI actions - one for working path and one for protection
*         path
*         - Set packet command for RX protection to hard drop
*         - Map between source ePort in the TTI action to Protection LOC table
*/
GT_VOID prvTgfProtectionRxConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc;
    PRV_TGF_TTI_ACTION_2_STC                    ttiAction;
    PRV_TGF_TTI_RULE_UNT                        ttiPattern;
    PRV_TGF_TTI_RULE_UNT                        ttiMask;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: Store the TTI lookup enabling status on the RX working port for
                PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E,
                                      &prvTgfRestoreCfg.lookupEnableForWorking);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    /* AUTODOC: Store the TTI lookup enabling status on the RX protection port for
                PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E,
                                      &prvTgfRestoreCfg.lookupEnableForProtection);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    /* AUTODOC: Enable TTI lookup for PRV_TGF_TTI_KEY_IPV4_E on the RX working port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Enable TTI lookup for PRV_TGF_TTI_KEY_IPV4_E on the RX protection port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Store MAC mode for IPv4 key */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    /* AUTODOC: Set MAC mode for IPv4 key */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: Configure TTI pattern and mask for the working path*/
    ttiPattern.ipv4.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS];
    ttiMask.ipv4.common.srcPortTrunk = BIT_7 - 1;

    /* AUTODOC: Configure TTI action for the working path */
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS];
    ttiAction.rxProtectionSwitchEnable          = GT_TRUE;
    ttiAction.rxIsProtectionPath                = GT_FALSE;
    ttiAction.sourceEPortAssignmentEnable       = GT_TRUE;
    ttiAction.sourceEPort                       = PRV_TGF_RX_EPORT_IDX_CNS;

    rc = prvTgfTtiRule2Set(PRV_TGF_WORKING_PATH_RULE_INDEX_CNS,
                           PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: Configure TTI pattern and mask for the protection path*/
    ttiPattern.ipv4.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS];

    /* AUTODOC: Configure TTI action for the protection path */
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS];
    ttiAction.rxIsProtectionPath                = GT_TRUE;

    rc = prvTgfTtiRule2Set(PRV_TGF_PROTECTION_PATH_RULE_INDEX_CNS,
                           PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: Store protection RX exception packet command */
    rc = prvTgfProtectionRxExceptionPacketCommandGet(prvTgfDevNum, &prvTgfRestoreCfg.command);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionPacketCommandGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Set packet command for RX protection to hard drop */
    rc = prvTgfProtectionRxExceptionPacketCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionPacketCommandSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Store the mapping between source ePort in the TTI action to
                Protection LOC table */
    rc = prvTgfProtectionPortToLocMappingGet(prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS,
                                             &prvTgfRestoreCfg.locTableIndexForRxEport);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS);

    /* AUTODOC: Map between source ePort in the TTI action to Protection LOC table */
    rc = prvTgfProtectionPortToLocMappingSet(prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS,
                                             PRV_TGF_RX_EPORT_LOC_TABLE_IDX_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS,
                                 PRV_TGF_RX_EPORT_LOC_TABLE_IDX_CNS);
}

/**
* @internal prvTgfProtectionTxOneToOneTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] workingPath              - GT_TRUE: the traffic is sent over the working path
*                                      - GT_FALSE: the traffic is sent over the protection path
*                                       None
*/
GT_VOID prvTgfProtectionTxOneToOneTrafficGenerate
(
    GT_BOOL workingPath
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_INTERFACE_INFO_STC         portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters
     */

    for (portIter = 0; portIter < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: enable capture on ports */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    packetSize += TGF_CRC_LEN_CNS;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from injection port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INJECTION_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INJECTION_PORT_IDX_CNS]);

    /* AUTODOC: disable capture ports */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify traffic */
    for (portIter = 0; portIter < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_INJECTION_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            case PRV_TGF_TX_WORKING_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = ((workingPath == GT_TRUE) ? (packetSize * prvTgfBurstCount) : 0);
                expectedCntrs.goodPktsSent.l[0]   = ((workingPath == GT_TRUE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.ucPktsSent.l[0]     = ((workingPath == GT_TRUE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = ((workingPath == GT_TRUE) ? (packetSize * prvTgfBurstCount) : 0);
                expectedCntrs.goodPktsRcv.l[0]    = ((workingPath == GT_TRUE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.ucPktsRcv.l[0]      = ((workingPath == GT_TRUE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;


            case PRV_TGF_TX_PROTECTION_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = ((workingPath == GT_FALSE) ? (packetSize * prvTgfBurstCount) : 0);
                expectedCntrs.goodPktsSent.l[0]   = ((workingPath == GT_FALSE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.ucPktsSent.l[0]     = ((workingPath == GT_FALSE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = ((workingPath == GT_FALSE) ? (packetSize * prvTgfBurstCount) : 0);
                expectedCntrs.goodPktsRcv.l[0]    = ((workingPath == GT_FALSE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.ucPktsRcv.l[0]      = ((workingPath == GT_FALSE) ? (prvTgfBurstCount) : 0);
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            default:
                /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
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
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfProtectionTxOnePlusOneTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfProtectionTxOnePlusOneTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_INTERFACE_INFO_STC         portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters
     */

    for (portIter = 0; portIter < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: enable capture on ports */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    packetSize += TGF_CRC_LEN_CNS;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from injection port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INJECTION_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INJECTION_PORT_IDX_CNS]);

    /* AUTODOC: disable capture ports */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_WORKING_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PROTECTION_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify traffic */
    for (portIter = 0; portIter < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_INJECTION_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            case PRV_TGF_TX_WORKING_PORT_IDX_CNS:
            case PRV_TGF_TX_PROTECTION_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            default:
                /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
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
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfProtectionRxTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] workingCmd               - command on the working path
* @param[in] protectionCmd            - command on the protection path
*                                       None
*/
GT_VOID prvTgfProtectionRxTrafficGenerate
(
    CPSS_PACKET_CMD_ENT     workingCmd,
    CPSS_PACKET_CMD_ENT     protectionCmd
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          workingNumOfExpectedRxPackets;
    GT_U32                          protectionNumOfExpectedRxPackets;
    GT_U32                          numOfExpectedTrappedPackets = 0;
    GT_BOOL                         getFirst;
    static GT_U8                    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                          buffLen;
    GT_U32                          packetActualLength = 0;
    TGF_NET_DSA_STC                 rxParam;
    GT_U8                           dev = 0;
    GT_U8                           queue = 0;
    GT_U32                          cpuCode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    switch (workingCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            workingNumOfExpectedRxPackets = prvTgfBurstCount;
            break;

        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            workingNumOfExpectedRxPackets = 0;
            numOfExpectedTrappedPackets += prvTgfBurstCount;
            break;

        case CPSS_PACKET_CMD_DROP_HARD_E:
        default:
            workingNumOfExpectedRxPackets = 0;
            break;
    }

    switch (protectionCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            protectionNumOfExpectedRxPackets = prvTgfBurstCount;
            break;

        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            protectionNumOfExpectedRxPackets = 0;
            numOfExpectedTrappedPackets += prvTgfBurstCount;
            break;

        case CPSS_PACKET_CMD_DROP_HARD_E:
        default:
            protectionNumOfExpectedRxPackets = 0;
            break;
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters
     */

    for (portIter = 0; portIter < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: enable capture on the egress ports */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS]);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    packetSize += TGF_CRC_LEN_CNS;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet to the RX working port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS]);

    /* AUTODOC: send tunneled packet to the RX protection port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on the egress ports */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS]);
    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify traffic */
    for (portIter = 0; portIter < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_RX_WORKING_PORT_IDX_CNS:
            case PRV_TGF_RX_PROTECTION_PORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = packetSize;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = packetSize;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            case PRV_TGF_REDIRECTION_FROM_WORKING_EPORT_IDX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = (packetSize - TGF_IPV4_HEADER_SIZE_CNS) * workingNumOfExpectedRxPackets;
                expectedCntrs.goodPktsSent.l[0]   = workingNumOfExpectedRxPackets;
                expectedCntrs.ucPktsSent.l[0]     = workingNumOfExpectedRxPackets;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize - TGF_IPV4_HEADER_SIZE_CNS) * workingNumOfExpectedRxPackets;
                expectedCntrs.goodPktsRcv.l[0]    = workingNumOfExpectedRxPackets;
                expectedCntrs.ucPktsRcv.l[0]      = workingNumOfExpectedRxPackets;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            case PRV_TGF_REDIRECTION_FROM_PROTECTION_EPORT_IDX_CNS:
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize - TGF_IPV4_HEADER_SIZE_CNS) * protectionNumOfExpectedRxPackets;
                    expectedCntrs.goodPktsSent.l[0]   = protectionNumOfExpectedRxPackets;
                    expectedCntrs.ucPktsSent.l[0]     = protectionNumOfExpectedRxPackets;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize - TGF_IPV4_HEADER_SIZE_CNS) * protectionNumOfExpectedRxPackets;
                    expectedCntrs.goodPktsRcv.l[0]    = protectionNumOfExpectedRxPackets;
                    expectedCntrs.ucPktsRcv.l[0]      = protectionNumOfExpectedRxPackets;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                    break;

            default:
                /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
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
        }
    }

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    getFirst = GT_TRUE;
    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    for (; numOfExpectedTrappedPackets > 0; numOfExpectedTrappedPackets--)
    {
        /* AUTODOC: get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet\n");

        /* AUTODOC: check the CPU code */
        cpuCode = (GT_U32)rxParam.cpuCode;
        UTF_VERIFY_EQUAL2_STRING_MAC(rxExpectedCpuCode, cpuCode,
                           "Wrong CPU code, expected: %d received %d", rxExpectedCpuCode, cpuCode);
        getFirst = GT_FALSE;
    }
    /* AUTODOC: get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfProtectionRestoreConfiguration function
* @endinternal
*
* @brief   Restore to default configuration
*/
static GT_VOID prvTgfProtectionRestoreConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Flush FDB */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: Invalidate VLAN */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLAN_ID_CNS);

    /* AUTODOC: Restore protection switching enabling status */
    rc = prvTgfProtectionEnableSet(prvTgfDevNum, prvTgfRestoreCfg.protectionEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfProtectionEnableSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal prvTgfProtectionTxRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the RX test
*/
static GT_VOID prvTgfProtectionTxRestoreConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;

    /* AUTODOC: Remove VLAN members */
    for (i = 0; i < PRV_TGF_TX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Restore E2Phy entries */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                                           &prvTgfRestoreCfg.txWorkingEportMapping);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS,
                                                           &prvTgfRestoreCfg.txProtectionEportMapping);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_PROTECTION_EPORT_IDX_CNS);

    prvTgfProtectionRestoreConfiguration();
}

/**
* @internal prvTgfProtectionTxOnePlusOneRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the 1+1 test
*/
GT_VOID prvTgfProtectionTxOnePlusOneRestoreConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Restore ECMP ePort and global ePort configuration */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum,
                                 &prvTgfRestoreCfg.globalEportCfg,
                                 &prvTgfRestoreCfg.ecmpEportCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfCfgGlobalEportSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the multi-target port mapping enabling status */
    rc = prvTgfL2MllMultiTargetPortEnableSet(prvTgfDevNum, prvTgfRestoreCfg.multiTargetPortEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the multi-target port value and mask */
    rc = prvTgfL2MllMultiTargetPortSet(prvTgfDevNum,
                                       prvTgfRestoreCfg.globalEportValue,
                                       prvTgfRestoreCfg.globalEportMask);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the multi-target port base */
    rc = prvTgfL2MllMultiTargetPortBaseSet(prvTgfDevNum, prvTgfRestoreCfg.eportBase);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllMultiTargetPortBaseSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the port to VIDX base */
    rc = prvTgfL2MllPortToVidxBaseSet(prvTgfDevNum, prvTgfRestoreCfg.vidxBase);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfL2MllPortToVidxBaseSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the MLL LTT entry */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS,
                                &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Restore the L2 MLL entry */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);

    prvTgfProtectionTxRestoreConfiguration();
}

/**
* @internal prvTgfProtectionTxOneToOneRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the 1:1 test
*/
GT_VOID prvTgfProtectionTxOneToOneRestoreConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Restore the LOC status */
    rc = prvTgfProtectionLocStatusSet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_LOC_TABLE_IDX_CNS,
                                      prvTgfRestoreCfg.status);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfProtectionLocStatusSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore TX protection enabling status */
    rc = prvTgfProtectionTxEnableSet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                     prvTgfRestoreCfg.txProtectionEnabled);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionTxEnableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    /* AUTODOC: Restore the mapping between the target eport to the LOC table */
    rc = prvTgfProtectionPortToLocMappingSet(prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS,
                                             prvTgfRestoreCfg.locTableIndexForTxWorkingEport);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_WORKING_EPORT_IDX_CNS);

    prvTgfProtectionTxRestoreConfiguration();
}

/**
* @internal prvTgfProtectionRxRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the RX test
*/
GT_VOID prvTgfProtectionRxRestoreConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;

    /* AUTODOC: Remove VLAN members */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLAN_ID_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Restore the LOC status */
    rc = prvTgfProtectionLocStatusSet(prvTgfDevNum, PRV_TGF_RX_EPORT_LOC_TABLE_IDX_CNS,
                                      prvTgfRestoreCfg.status);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfProtectionLocStatusSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore protection RX exception packet command */
    rc = prvTgfProtectionRxExceptionPacketCommandSet(prvTgfDevNum, prvTgfRestoreCfg.command);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionPacketCommandSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore protection RX exception CPU/drop code */
    rc = prvTgfProtectionRxExceptionCpuCodeSet(prvTgfDevNum, prvTgfRestoreCfg.cpuCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfProtectionRxExceptionCpuCodeSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Restore the mapping between source ePort in the TTI action to
                Protection LOC table */
    rc = prvTgfProtectionPortToLocMappingSet(prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS,
                                             prvTgfRestoreCfg.locTableIndexForRxEport);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfProtectionPortToLocMappingSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_RX_EPORT_IDX_CNS);

    /* AUTODOC: Restore the TTI lookup enabling status on the RX working port for
                PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_WORKING_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E,
                                      prvTgfRestoreCfg.lookupEnableForWorking);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Restore the TTI lookup enabling status on the RX protection port for
                PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_PROTECTION_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E,
                                      prvTgfRestoreCfg.lookupEnableForProtection);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Restore MAC mode for IPv4 key */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: Invalidate the TTI rule for the working path */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_WORKING_PATH_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: Invalidate the TTI rule for the protection path */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_PROTECTION_PATH_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    prvTgfProtectionRestoreConfiguration();
}


