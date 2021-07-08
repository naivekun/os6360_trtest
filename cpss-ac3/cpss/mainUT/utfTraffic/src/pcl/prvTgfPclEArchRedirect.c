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
* @file prvTgfPclEArchRedirect.c
*
* @brief redirect to ePort eVlan and eVidx
*
* @version   13
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfIpGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS  2
#define PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS   3
#define PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS  0

/* initial VIDs in two tags */
#define PRV_TGF_INIT_VID0_CNS  2

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1

#define SET_ARR_3_MAC(arr,a,b,c) \
    arr[0] = a;                \
    arr[1] = b;                \
    arr[2] = c

#define SET_ARR_2_MAC(arr,a,b) \
    arr[0] = a;                \
    arr[1] = b

/* Send ePort values */
#define PRV_TGF_SEND_EPORT_VALUES_MAC(arr)   \
    SET_ARR_3_MAC(arr ,   \
                0x12 ,  \
                0x64 ,  \
                0x1F4)

/* ePort values */
#define PRV_TGF_TRG_EPORT_VALUES_MAC(arr)           \
    SET_ARR_3_MAC(arr ,                               \
                ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x17FE) ,  \
                ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x11FF) ,  \
                3)

/* eVID values */
#define PRV_TGF_TRG_EVID_VALUES_MAC(arr)            \
    SET_ARR_3_MAC(arr ,                               \
                ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(0x1FFC) ,  \
                0xFF0 ,  \
                50)

/* eVIDX values */
#define PRV_TGF_TRG_EVIDX_VALUES_MAC(arr)  \
    SET_ARR_2_MAC(arr ,                               \
                ALIGN_EVIDX_TO_4K_TILL_MAX_DEV_MAC(0x1FFB) ,  \
                0x1000)

/* eVIDX used MLL index */
#define PRV_TGF_TRG_EVIDX_MLL_IDX_CNS  1

/* value to ignore the IPCL Cfg parameter */
#define PRV_IPCL_CFG_INVALID_EPORT_CNS      0xFFFFFF
#define PRV_IPCL_CFG_INVALID_EVID_CNS       0
#define PRV_IPCL_CFG_INVALID_EVIDX_CNS      0
#define PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS  0xFFFFFF

/* L2 part of packet 1*/
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0x8a, 0x5c, 0x71, 0x90, 0x36},                /* daMac */
    {0x00, 0xfe, 0x98, 0x76, 0x43, 0x55}                 /* saMac */
};
/* L2 part of packet 2*/
static TGF_PACKET_L2_STC packet2TgfL2Part = {
    {0x00, 0xfe, 0x98, 0x76, 0x43, 0x55},                /* daMac */
    {0x00, 0x9a, 0x55, 0x81, 0x93, 0x30}                 /* saMac */
};
static TGF_PACKET_VLAN_TAG_STC packet1VlanTag0 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0, /* tag0 UP */
    0, /* tag0 CFI*/
    PRV_TGF_INIT_VID0_CNS
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {0x3333};

/* DATA of packet */
static GT_U8 packet1TgfPayloadDataArr[] =
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

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC packet1TgfPayloadPart = {
    sizeof(packet1TgfPayloadDataArr),                 /* dataLength */
    packet1TgfPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet1 */
static TGF_PACKET_PART_STC packet1TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag0},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};
/* PARTS of packet2 */
static TGF_PACKET_PART_STC packet2TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet2TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag0},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};
/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                        /* totalLen */
    (sizeof(packet1TgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    packet1TgfPartArray                                          /* partsArray */
};
/* PACKET2 to send */
static TGF_PACKET_STC packet2TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                        /* totalLen */
    (sizeof(packet2TgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    packet2TgfPartArray                                          /* partsArray */
};
/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclEArchRedirectMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
*
* @param[out] savePhyInfoPtr           - (pointer to) inteface to save previous mapping
*                                      may be NULL to skip saving
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEArchRedirectMapEport2PhySet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_INTERFACE_INFO_STC          *phyInfoPtr,
    OUT CPSS_INTERFACE_INFO_STC          *savePhyInfoPtr
)
{
    GT_STATUS rc;

    if (savePhyInfoPtr != NULL)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            devNum, portNum, savePhyInfoPtr);
        PRV_UTF_VERIFY_GT_OK(
            rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
        devNum, portNum, phyInfoPtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclEArchRedirectMapEVidx2PhyPortSet function
* @endinternal
*
* @brief   Maps eVidx to physical port
*
* @param[in] devNum                   - devNum
* @param[in] eVidx                    -   number
* @param[in] portNum                  -  ePort
* @param[in] mllIndex                 -  index of MLL table entry to be used
*
* @param[out] allEvidxEnablePtr        - (pointer to) enable lookup for VIX 0-4095
*                                      saveMllModePtr    - (pointer to) saved MLL table mode
* @param[out] saveMllEntryPtr          - (pointer to) saved MLL table entry
* @param[out] lttEntryPtr              - (pointer to) saved LTT table entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEArchRedirectMapEVidx2PhyPortSet
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             eVidx,
    IN  GT_PORT_NUM                        portNum,
    IN  GT_U32                             mllIndex,
    OUT GT_BOOL                            *allEvidxEnablePtr,
    OUT PRV_TGF_L2_MLL_ENTRY_STC           *saveMllEntryPtr,
    OUT PRV_TGF_L2_MLL_LTT_ENTRY_STC       *lttEntryPtr
)
{
    GT_STATUS                      rc;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;
    PRV_TGF_L2_MLL_PAIR_STC              saveMllPairEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32 lttIndex;

    lttIndex = eVidx - 4096;

    /* save enable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableGet(devNum, allEvidxEnablePtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfL2MllLookupForAllEvidxEnableGet");

    /* disable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(devNum, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfL2MllLookupForAllEvidxEnableSet");

   /* save LTT entry */
    rc = prvTgfL2MllLttEntryGet(devNum, lttIndex, lttEntryPtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllLttEntryGet");

    /* set LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(lttEntry));
    lttEntry.mllPointer = mllIndex;
    lttEntry.entrySelector = 0;
    rc = prvTgfL2MllLttEntrySet(devNum, lttIndex, &lttEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllLttEntrySet");

    /* save L2 MLL entry */
    mllPairWriteForm=PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    rc = prvTgfL2MllPairRead(devNum, mllIndex, mllPairWriteForm, &saveMllPairEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllPairRead");

    cpssOsMemCpy(saveMllEntryPtr, &(saveMllPairEntry.firstMllNode) , sizeof(PRV_TGF_L2_MLL_ENTRY_STC));

    /* set L2 MLL entry */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = devNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = portNum;
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, mllIndex, mllPairWriteForm, &mllPairEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllPairWrite");

    return GT_OK;
}

/**
* @internal prvTgfPclEArchRedirectMapEVidx2PhyPortRestore function
* @endinternal
*
* @brief   Restore HW after Map eVidx to physical port
*
* @param[in] devNum                   - devNum
* @param[in] eVidx                    -   number
* @param[in] mllIndex                 -  index of MLL table entry to be used
* @param[in] allEvidxEnable           - enable lookup for VIX 0-4095
*                                      saveMllMode       - saved MLL table mode
* @param[in] saveMllEntryPtr          - (pointer to) saved MLL table entry
* @param[in] lttEntryPtr              - (pointer to) saved LTT table entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEArchRedirectMapEVidx2PhyPortRestore
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             eVidx,
    IN  GT_U32                             mllIndex,
    IN  GT_BOOL                            allEvidxEnable,
    IN  PRV_TGF_L2_MLL_ENTRY_STC           *saveMllEntryPtr,
    IN  PRV_TGF_L2_MLL_LTT_ENTRY_STC       *lttEntryPtr
)
{
    GT_STATUS                      rc;
    GT_U32 lttIndex;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    lttIndex = eVidx - 4096;

    /* restore L2 MLL entry */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    cpssOsMemCpy(&(mllPairEntry.firstMllNode), saveMllEntryPtr , sizeof(PRV_TGF_L2_MLL_ENTRY_STC));

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, mllIndex, mllPairWriteForm, &mllPairEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllPairWrite");


    /* restore LTT entry */
    rc = prvTgfL2MllLttEntrySet(devNum, lttIndex, lttEntryPtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfL2MllLttEntrySet");

    /* restore enable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(devNum, allEvidxEnable);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfL2MllLookupForAllEvidxEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclEArchRedirectMapEVid2PhyPortSet function
* @endinternal
*
* @brief   Maps eVid to physical port
*
* @param[in] devNum                   -  devNum
* @param[in] eVid                     -   number
* @param[in] phyPortNum               -  port
*
* @param[out] saveVlanRangePtr         (pointer to) saved vlan range
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEArchRedirectMapEVid2PhyPortSet
(
    IN  GT_U8                              devNum,
    IN  GT_U16                             eVid,
    IN  GT_U32                             phyPortNum,
    OUT GT_U16                             *saveVlanRangePtr
)
{
    GT_STATUS                      rc;
    GT_U32                          portsArray[1];
    GT_U8                          devArray[1];
    GT_U8                          tagArray[1];

    devArray[0]    = devNum;
    portsArray[0]  = phyPortNum;
    tagArray[0]    = 1; /*tagged*/

    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        eVid, portsArray, devArray, tagArray,
        1 /*numPorts*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    rc = prvTgfBrgVlanRangeGet(saveVlanRangePtr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanRangeGet");

    rc = prvTgfBrgVlanRangeSet((GT_U16)(eVid + 1));
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanRangeSet");

    return GT_OK;
}

/**
* @internal prvTgfPclEArchRedirectMapEVid2PhyPortRestore function
* @endinternal
*
* @brief   Restore eVid
*
* @param[in] eVid                     -   number
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEArchRedirectMapEVid2PhyPortRestore
(
    IN  GT_U16                             eVid,
    IN  GT_U16                             saveVlanRange
)
{
    GT_STATUS                      rc;

    rc = prvTgfBrgVlanRangeSet(saveVlanRange);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanRangeSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(eVid);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return GT_OK;
}

/**
* @internal prvTgfPclEArchRedirectConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclEArchRedirectConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_INIT_VID0_CNS);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 2 with all tagged OUTER_TAG0_INNER_TAG1 ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_INIT_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclEArchRedirectConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEArchRedirectConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
        prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_INIT_VID0_CNS);
}

/**
* @internal prvTgfPclEArchRedirectConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*
* @param[in] ePort                    -  to redirect to (PRV_IPCL_CFG_INVALID_EPORT_CNS - ignore)
* @param[in] eVid                     -   to redirect to (PRV_IPCL_CFG_INVALID_EVID_CNS  - ignore)
* @param[in] eVidx                    -  to redirect to (PRV_IPCL_CFG_INVALID_EVIDX_CNS - ignore)
* @param[in] eSrcPort                 -  to redirect to
*                                      (PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS - ignore)
*                                       None
*/
static GT_VOID prvTgfPclEArchRedirectConfigurationIpclAdd
(
    IN GT_PORT_NUM ePort,
    IN GT_U16      eVid,
    IN GT_U16      eVidx,
    IN GT_PORT_NUM eSrcPort
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

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Ingress rule */
    /* Forward, modify VID0  */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_IPCL_RULE_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleStdNotIp.macDa.arEther[i] = 0xFF;
        mask.ruleStdNotIp.macSa.arEther[i] = 0xFF;
        pattern.ruleStdNotIp.macDa.arEther[i] =
            packet1TgfL2Part.daMac[i];
        pattern.ruleStdNotIp.macSa.arEther[i] =
            packet1TgfL2Part.saMac[i];
    }
    mask.ruleStdNotIp.etherType    = 0xFFFF;
    pattern.ruleStdNotIp.etherType =
        prvTgfPacket1EtherTypePart.etherType;

    mask.ruleStdNotIp.common.isTagged    = 1;
    pattern.ruleStdNotIp.common.isTagged    = 1;

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    if (ePort != PRV_IPCL_CFG_INVALID_EPORT_CNS)
    {
        action.redirect.redirectCmd =
            PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */
        action.redirect.data.outIf.outlifType =
            PRV_TGF_OUTLIF_TYPE_LL_E;
        action.redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_PORT_E;
        action.redirect.data.outIf.outInterface.devPort.hwDevNum =
            prvTgfDevNum;
        action.redirect.data.outIf.outInterface.devPort.portNum =
            ePort;
    }
    else if (eVid != PRV_IPCL_CFG_INVALID_EVID_CNS)
    {
        /* redirection does not works to VID             */
        /* it can only flood in the current packets VLAN */
        /* Vlan assignment used                          */
        action.vlan.vlanId = eVid;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    }
    else if (eVidx != PRV_IPCL_CFG_INVALID_EVIDX_CNS)
    {
        action.redirect.redirectCmd =
            PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */
        action.redirect.data.outIf.outlifType =
            PRV_TGF_OUTLIF_TYPE_LL_E;
        action.redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_VIDX_E;
        action.redirect.data.outIf.outInterface.vidx = eVidx;
    }
    else if (eSrcPort != PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS)
    {
        action.sourcePort.assignSourcePortEnable = GT_TRUE;
        action.sourcePort.sourcePortValue = eSrcPort;
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_BAD_PARAM, "no ePort, eVid, eVidx, eSrcPort");
    }

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:8A:5C:71:90:36, SA=00:FE:98:76:43:55 */
    /* AUTODOC:   pattern pclId=1, EtherType=0x3333 */
    /* AUTODOC: add PCL action: */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEArchRedirectIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEArchRedirectIpclRestore
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
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

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
* @internal prvTgfPclEArchRedirectConfigurationIpclAdd1 function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration in ePort access mode
*
* @param[in] eSendPort                - ePort to send traffic
* @param[in] ePort                    -  to redirect to (PRV_IPCL_CFG_INVALID_EPORT_CNS - ignore)
* @param[in] eVid                     -   to redirect to (PRV_IPCL_CFG_INVALID_EVID_CNS  - ignore)
* @param[in] eVidx                    -  to redirect to (PRV_IPCL_CFG_INVALID_EVIDX_CNS - ignore)
* @param[in] eSrcPort                 -  to redirect to
*                                      (PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS - ignore)
*                                       None
*/
static GT_VOID prvTgfPclEArchRedirectConfigurationIpclAdd1
(
    IN GT_PORT_NUM eSendPort,
    IN GT_PORT_NUM ePort,
    IN GT_U16      eVid,
    IN GT_U16      eVidx,
    IN GT_PORT_NUM eSrcPort
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

    /* AUTODOC: init PCL Engine in ePort mode for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefEportInitExt(
        prvTgfDevNum,
        eSendPort,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d",
        prvTgfDevNum,
        eSendPort);

    /* Ingress rule */
    /* Forward, modify VID0  */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_IPCL_RULE_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleStdNotIp.macDa.arEther[i] = 0xFF;
        mask.ruleStdNotIp.macSa.arEther[i] = 0xFF;
        pattern.ruleStdNotIp.macDa.arEther[i] =
            packet1TgfL2Part.daMac[i];
        pattern.ruleStdNotIp.macSa.arEther[i] =
            packet1TgfL2Part.saMac[i];
    }
    mask.ruleStdNotIp.etherType    = 0xFFFF;
    pattern.ruleStdNotIp.etherType =
        prvTgfPacket1EtherTypePart.etherType;

    mask.ruleStdNotIp.common.isTagged    = 1;
    pattern.ruleStdNotIp.common.isTagged    = 1;

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    if (ePort != PRV_IPCL_CFG_INVALID_EPORT_CNS)
    {
        action.redirect.redirectCmd =
            PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */
        action.redirect.data.outIf.outlifType =
            PRV_TGF_OUTLIF_TYPE_LL_E;
        action.redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_PORT_E;
        action.redirect.data.outIf.outInterface.devPort.hwDevNum =
            prvTgfDevNum;
        action.redirect.data.outIf.outInterface.devPort.portNum =
            ePort;
    }
    else if (eVid != PRV_IPCL_CFG_INVALID_EVID_CNS)
    {
        /* redirection does not works to VID             */
        /* it can only flood in the current packets VLAN */
        /* Vlan assignment used                          */
        action.vlan.vlanId = eVid;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    }
    else if (eVidx != PRV_IPCL_CFG_INVALID_EVIDX_CNS)
    {
        action.redirect.redirectCmd =
            PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */
        action.redirect.data.outIf.outlifType =
            PRV_TGF_OUTLIF_TYPE_LL_E;
        action.redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_VIDX_E;
        action.redirect.data.outIf.outInterface.vidx = eVidx;
    }
    else if (eSrcPort != PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS)
    {
        action.sourcePort.assignSourcePortEnable = GT_TRUE;
        action.sourcePort.sourcePortValue = eSrcPort;
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_BAD_PARAM, "no ePort, eVid, eVidx, eSrcPort");
    }

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:8A:5C:71:90:36, SA=00:FE:98:76:43:55 */
    /* AUTODOC:   pattern pclId=1, EtherType=0x3333 */
    /* AUTODOC: add PCL action: */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);

    /* AUTODOC: Set new default source ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        eSendPort);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberSet: %d, %d, %d",
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], eSendPort);
}

/**
* @internal prvTgfPclEArchRedirectIpclRestore1 function
* @endinternal
*
* @brief   Restore test configuration for ePort mode access
*
* @param[in] eSendPort                - ePort to send traffic
*                                       None
*/
static GT_VOID prvTgfPclEArchRedirectIpclRestore1
(
    IN GT_PORT_NUM eSendPort
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;

    /* AUTODOC: Restore default access mode on port */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    interfaceInfo.type               = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index              = eSendPort;

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

    /* AUTODOC: Restore default ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberSet: %d, %d, %d",
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclEArchRedirectCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclEArchRedirectCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; (portIter < PRV_TGF_PORTS_NUM_CNS); portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(
            prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
}

/**
* @internal prvTgfPclEArchRedirectTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclEArchRedirectTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc         = GT_OK;

    prvTgfPclEArchRedirectCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &packet1TgfInfo, 1 /*burstCount*/, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:8A:5C:71:90:36, SA=00:FE:98:76:43:55 */
    /* AUTODOC:   VID=2, EtherType=0x3333 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}
/**
* @internal prvTgfPclEArchRedirectTrafficGenerate2 function
* @endinternal
*
* @brief   Generate traffic.
*         Traffic checks FDB that learned during previous stage of the test
*         DA of this traffic is equal SA of previous send packet (function
*         prvTgfPclEArchRedirectTrafficGenerate).
*/
static GT_VOID prvTgfPclEArchRedirectTrafficGenerate2
(
    GT_VOID
)
{
    GT_STATUS                       rc         = GT_OK;

    prvTgfPclEArchRedirectCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &packet2TgfInfo, 1 /*burstCount*/, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:FE:98:76:43:55, SA=00:8A:5C:71:90:36 */
    /* AUTODOC:   VID=2, EtherType=0x3333 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclEArchRedirectTrafficCheck function
* @endinternal
*
* @brief   Checks traffic egress VID0 in the Tag
*
* @param[in] sendPortIndex            - index of the port the packet sent from it,
*                                      must be skipped because the packed
*                                      sent from this port by CPU to be looped back.
* @param[in] receivePortIndex         - index of the port that receives the packet
*                                       None
*/
GT_VOID prvTgfPclEArchRedirectTrafficCheck
(
    GT_U32 sendPortIndex,
    GT_U32 receivePortIndex
)
{
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_STATUS                       rc;

    for (portIter = 0; (portIter < PRV_TGF_PORTS_NUM_CNS); portIter++)
    {
        if (sendPortIndex == portIter)
        {
            continue;
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portIter],
            prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfReadPortCountersEth");

        if (portIter == receivePortIndex)
        {
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
                0, portCntrs.goodPktsSent.l[0], "wrong counter value");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(
                0, portCntrs.goodPktsSent.l[0], "wrong counter value");
        }
    }
}

/**
* @internal prvTgfPclEArchFloodingCheck function
* @endinternal
*
* @brief   Checks flooding on all VLAN ports excluding sending port.
*
* @param[in] sendPortIndex            - index of the port the packet sent from it.
*                                       None
*
* @note In case of sending port counters should be 0
*
*/
GT_VOID prvTgfPclEArchFloodingCheck
(
    GT_U32 sendPortIndex
)
{
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_STATUS                       rc;

    for (portIter = 0; (portIter < PRV_TGF_PORTS_NUM_CNS); portIter++)
    {
        if (sendPortIndex == portIter)
        {
            continue;
        }
        /* read counters */
        rc = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portIter],
            prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfReadPortCountersEth");

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, portCntrs.goodPktsSent.l[0], "wrong counter value");
    }
}

/**
* @internal prvTgfPclEArchRedirectIpclEPortTest function
* @endinternal
*
* @brief   Ingress PCL redirection to ePort test
*/
GT_VOID prvTgfPclEArchRedirectIpclEPortTest
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC          phyInfo;
    CPSS_INTERFACE_INFO_STC          savePhyInfo;
    GT_U32                           valueArr[3];
    GT_U32                           valueAmount =
        sizeof(valueArr) / sizeof(valueArr[0]);
    GT_U32                           valueIdx;
    GT_PORT_NUM                      eSendPortArr[3];
    GT_U32                           eSendPortNum =
        sizeof(eSendPortArr) / sizeof(eSendPortArr[0]);
    GT_U32                           portIdx;

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);
    PRV_TGF_TRG_EPORT_VALUES_MAC(valueArr);

    /* AUTODOC: SETUP CONFIGURATION: */
    phyInfo.type = CPSS_INTERFACE_PORT_E;
    phyInfo.devPort.hwDevNum =
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS];
    phyInfo.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS];

    PRV_UTF_LOG0_MAC(
        "*** EArchRedirect IPCL ePort test  *** \n");

    prvTgfPclEArchRedirectConfigurationSet();

    /* AUTODOC: Iterate through ePorts [0x7FE, 0x1FF, 3]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {

        PRV_UTF_LOG1_MAC(
            "*** EPort = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet(
            prvTgfDevNum, valueArr[valueIdx],
            &phyInfo, &savePhyInfo);

        prvTgfPclEArchRedirectConfigurationIpclAdd(
                valueArr[valueIdx],
                PRV_IPCL_CFG_INVALID_EVID_CNS,
                PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
        /* AUTODOC:  redirect to ePort */

        /* AUTODOC: GENERATE TRAFFIC: */
        prvTgfPclEArchRedirectTrafficGenerate();

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged packet received on port 2 */
        prvTgfPclEArchRedirectTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS);

        prvTgfPclEArchRedirectIpclRestore();

        /* AUTODOC: restore mapping ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet(
            prvTgfDevNum, valueArr[valueIdx],
            &savePhyInfo, NULL);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: Iterate through ePorts [0x7FE, 0x1FF, 3]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {
        PRV_UTF_LOG1_MAC(
            "*** EPort = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet(
            prvTgfDevNum, valueArr[valueIdx],
            &phyInfo, &savePhyInfo);

        /* AUTODOC: Iterate through eSendPortArr [0x12, 0x64, 0x3E8]: */
        for(portIdx = 0; portIdx < eSendPortNum; portIdx++)
        {
            PRV_UTF_LOG1_MAC(
                "*** Send ePort = 0x%04X  *** \n", eSendPortArr[portIdx]);

            /* AUTODOC: configure IPCL in ePort access mode */
            prvTgfPclEArchRedirectConfigurationIpclAdd1(
                    eSendPortArr[portIdx],
                    valueArr[valueIdx],
                    PRV_IPCL_CFG_INVALID_EVID_CNS,
                    PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                    PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
            /* AUTODOC:  redirect to ePort */

            /* AUTODOC: GENERATE TRAFFIC: */
            prvTgfPclEArchRedirectTrafficGenerate();

            /* AUTODOC: verify traffic: */
            /* AUTODOC:   tagged packet received on port 2 */
            prvTgfPclEArchRedirectTrafficCheck(
                PRV_TGF_SEND_PORT_IDX_CNS,
                PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS);

            prvTgfPclEArchRedirectIpclRestore1(eSendPortArr[portIdx]);
        }
        /* AUTODOC: restore mapping ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet(
            prvTgfDevNum, valueArr[valueIdx],
            &savePhyInfo, NULL);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEArchRedirectConfigurationRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of EArchRedirect IPCL ePort test  *** \n");
}

/**
* @internal prvTgfPclEArchRedirectIpclEVidTest function
* @endinternal
*
* @brief   Ingress PCL redirection to eVid test
*/
GT_VOID prvTgfPclEArchRedirectIpclEVidTest
(
    GT_VOID
)
{
    GT_U16                           saveVlanRange;
    GT_U16                           valueArr[3];
    GT_U32                           valueAmount =
        sizeof(valueArr) / sizeof(valueArr[0]);
    GT_U32                           valueIdx;
    GT_PORT_NUM                      eSendPortArr[3];
    GT_U32                           eSendPortNum =
        sizeof(eSendPortArr) / sizeof(eSendPortArr[0]);
    GT_U32                           portIdx;

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);
    PRV_TGF_TRG_EVID_VALUES_MAC(valueArr);

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC(
        "*** EArchRedirect IPCL eVid test  *** \n");

    prvTgfPclEArchRedirectConfigurationSet();

    /* AUTODOC: Iterate through eVids [0x1FFC, 0xFF0, 50]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {
        PRV_UTF_LOG1_MAC(
            "*** EVid = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map eVid to physical port 3 */
        prvTgfPclEArchRedirectMapEVid2PhyPortSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS],
            valueArr[valueIdx],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS],
            &saveVlanRange);

        prvTgfPclEArchRedirectConfigurationIpclAdd(
                PRV_IPCL_CFG_INVALID_EPORT_CNS,
                valueArr[valueIdx],
                PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
        /* AUTODOC:  flood to Vlan, VlanId=eVid */

        /* AUTODOC: GENERATE TRAFFIC: */
        prvTgfPclEArchRedirectTrafficGenerate();

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged packet received on port 3 */
        prvTgfPclEArchRedirectTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS);

        prvTgfPclEArchRedirectIpclRestore();

        /* AUTODOC: restore mapping eVid to physical port 3 */
        prvTgfPclEArchRedirectMapEVid2PhyPortRestore(
            valueArr[valueIdx], saveVlanRange);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: Iterate through eVids [0x1FFC, 0xFF0, 50]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {
        PRV_UTF_LOG1_MAC(
            "*** EVid = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map eVid to physical port 3 */
        prvTgfPclEArchRedirectMapEVid2PhyPortSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS],
            valueArr[valueIdx],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS],
            &saveVlanRange);

        /* AUTODOC: Iterate through eSendPortArr [0x12, 0x64, 0x3E8]: */
        for(portIdx = 0; portIdx < eSendPortNum; portIdx++)
        {
            PRV_UTF_LOG1_MAC(
                "*** Send ePort = 0x%04X  *** \n", eSendPortArr[portIdx]);
            /* AUTODOC: configure IPCL in ePort access mode */
            prvTgfPclEArchRedirectConfigurationIpclAdd1(
                    eSendPortArr[portIdx],
                    PRV_IPCL_CFG_INVALID_EPORT_CNS,
                    valueArr[valueIdx],
                    PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                    PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
            /* AUTODOC:  flood to Vlan, VlanId=eVid */

            /* AUTODOC: GENERATE TRAFFIC: */
            prvTgfPclEArchRedirectTrafficGenerate();

            /* AUTODOC: verify traffic: */
            /* AUTODOC:   tagged packet received on port 3 */
            prvTgfPclEArchRedirectTrafficCheck(
                PRV_TGF_SEND_PORT_IDX_CNS,
                PRV_TGF_RECEIVE_PORT_IDX_EVID_CNS);

            prvTgfPclEArchRedirectIpclRestore1(eSendPortArr[portIdx]);
        }

        /* AUTODOC: restore mapping eVid to physical port 3 */
        prvTgfPclEArchRedirectMapEVid2PhyPortRestore(
            valueArr[valueIdx], saveVlanRange);
    }

    /* AUTODOC: End iteration. */
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEArchRedirectConfigurationRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of EArchRedirect IPCL eVid test  *** \n");
}

/**
* @internal prvTgfPclEArchRedirectIpclEVidxTest function
* @endinternal
*
* @brief   Ingress PCL redirection to eVidx test
*/
GT_VOID prvTgfPclEArchRedirectIpclEVidxTest
(
    GT_VOID
)
{
    GT_BOOL                            allEvidxEnable;
    PRV_TGF_L2_MLL_ENTRY_STC           saveMllEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC       lttEntry;
    GT_U16                             valueArr[2];
    GT_U32                             valueAmount =
        sizeof(valueArr) / sizeof(valueArr[0]);
    GT_U32                             valueIdx;
    GT_PORT_NUM                      eSendPortArr[3];
    GT_U32                           eSendPortNum =
        sizeof(eSendPortArr) / sizeof(eSendPortArr[0]);
    GT_U32                           portIdx;

    PRV_TGF_TRG_EVIDX_VALUES_MAC(valueArr);
    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC(
        "*** EArchRedirect IPCL eVidx test  *** \n");

    prvTgfPclEArchRedirectConfigurationSet();

    /* AUTODOC: Iterate through eVidxs [0x1FFB, 0x1000]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {
        PRV_UTF_LOG1_MAC(
            "*** EVidx = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map eVidx to physical port 0 */
        prvTgfPclEArchRedirectMapEVidx2PhyPortSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            valueArr[valueIdx],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            PRV_TGF_TRG_EVIDX_MLL_IDX_CNS,
            &allEvidxEnable, &saveMllEntry, &lttEntry);

        prvTgfPclEArchRedirectConfigurationIpclAdd(
                PRV_IPCL_CFG_INVALID_EPORT_CNS,
                PRV_IPCL_CFG_INVALID_EVID_CNS,
                valueArr[valueIdx],
                PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
        /* AUTODOC:  redirect to eVidx */

        /* AUTODOC: GENERATE TRAFFIC: */
        prvTgfPclEArchRedirectTrafficGenerate();

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged packet received on port 0 */
        prvTgfPclEArchRedirectTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS);

        prvTgfPclEArchRedirectIpclRestore();

        /* AUTODOC: restore mapping eVidx to physical port 0 */
        prvTgfPclEArchRedirectMapEVidx2PhyPortRestore(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            valueArr[valueIdx],
            PRV_TGF_TRG_EVIDX_MLL_IDX_CNS,
            allEvidxEnable, &saveMllEntry, &lttEntry);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: Iterate through eVidxs [0x1FFB, 0x1000]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {
        PRV_UTF_LOG1_MAC(
            "*** EVidx = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map eVidx to physical port 0 */
        prvTgfPclEArchRedirectMapEVidx2PhyPortSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            valueArr[valueIdx],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            PRV_TGF_TRG_EVIDX_MLL_IDX_CNS,
            &allEvidxEnable, &saveMllEntry, &lttEntry);

        /* AUTODOC: Iterate through eSendPortArr [0x12, 0x64, 0x3E8]: */
        for(portIdx = 0; portIdx < eSendPortNum; portIdx++)
        {
            PRV_UTF_LOG1_MAC(
                "*** Send ePort = 0x%04X  *** \n", eSendPortArr[portIdx]);

            /* AUTODOC: configure IPCL in ePort access mode */
            prvTgfPclEArchRedirectConfigurationIpclAdd1(
                    eSendPortArr[portIdx],
                    PRV_IPCL_CFG_INVALID_EPORT_CNS,
                    PRV_IPCL_CFG_INVALID_EVID_CNS,
                    valueArr[valueIdx],
                    PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS);
            /* AUTODOC:  redirect to eVidx */

            /* AUTODOC: GENERATE TRAFFIC: */
            prvTgfPclEArchRedirectTrafficGenerate();

            /* AUTODOC: verify traffic: */
            /* AUTODOC:   tagged packet received on port 0 */
            prvTgfPclEArchRedirectTrafficCheck(
                PRV_TGF_SEND_PORT_IDX_CNS,
                PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS);

            prvTgfPclEArchRedirectIpclRestore1(eSendPortArr[portIdx]);
        }

        /* AUTODOC: restore mapping eVidx to physical port 0 */
        prvTgfPclEArchRedirectMapEVidx2PhyPortRestore(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EVIDX_CNS],
            valueArr[valueIdx],
            PRV_TGF_TRG_EVIDX_MLL_IDX_CNS,
            allEvidxEnable, &saveMllEntry, &lttEntry);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEArchRedirectConfigurationRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of EArchRedirect IPCL eVidx test  *** \n");
}
/**
* @internal prvTgfPclEArchSourcePortTest function
* @endinternal
*
* @brief   PCL source port assignment test
*/
GT_VOID prvTgfPclEArchSourcePortTest
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC          phyInfo;
    CPSS_INTERFACE_INFO_STC          savePhyInfo;
    GT_U32                           valueArr[3];
    GT_U32                           valueAmount =
                                         sizeof(valueArr) / sizeof(valueArr[0]);
    GT_U32                           valueIdx;
    GT_BOOL                          ForwardingEnable;
    GT_PORT_NUM                      eSendPortArr[3];
    GT_U32                           eSendPortNum =
        sizeof(eSendPortArr) / sizeof(eSendPortArr[0]);
    GT_U32                           portIdx;

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);
    PRV_TGF_TRG_EPORT_VALUES_MAC(valueArr);

    /* AUTODOC: SETUP CONFIGURATION: */
    phyInfo.type = CPSS_INTERFACE_PORT_E;
    phyInfo.devPort.hwDevNum =
                           prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS];
    phyInfo.devPort.portNum =
                           prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS];

    PRV_UTF_LOG0_MAC( "*** EArch source ePort test  *** \n");

    prvTgfPclEArchRedirectConfigurationSet();

    /* AUTODOC: Iterate through ePorts [0x7FE, 0x1FF, 3]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {

        PRV_UTF_LOG1_MAC( "*** EPort = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet( prvTgfDevNum, valueArr[valueIdx],
                                               &phyInfo, &savePhyInfo);

        /* AUTODOC: configure PCL rule to assign source ePort*/
        prvTgfPclEArchRedirectConfigurationIpclAdd(
                PRV_IPCL_CFG_INVALID_EPORT_CNS,
                PRV_IPCL_CFG_INVALID_EVID_CNS,
                PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                valueArr[valueIdx]);

        /* AUTODOC: save current value of forwarding of NA to CPU enable */
        prvTgfBrgFdbNaToCpuPerPortGet(prvTgfDevNum, valueArr[valueIdx],
                                      &ForwardingEnable);
        /* AUTODOC: enable forwarding of NA to CPU on the new src eport */
        prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, valueArr[valueIdx],
                                      GT_TRUE);

        /* AUTODOC:  redirect to ePort */

        /* AUTODOC: GENERATE TRAFFIC: */
        prvTgfPclEArchRedirectTrafficGenerate();

        /* AUTODOC: verify traffic: */
        /* AUTODOC: tagged packet received on all ports */
        prvTgfPclEArchFloodingCheck(PRV_TGF_SEND_PORT_IDX_CNS);

        /* AUTODOC: GENERATE TRAFFIC: */
        prvTgfPclEArchRedirectTrafficGenerate2();

        /* AUTODOC: verify traffic: */
        /* AUTODOC: tagged packet received on port 2 */
        prvTgfPclEArchRedirectTrafficCheck( PRV_TGF_SEND_PORT_IDX_CNS,
                                            PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS);

        /* AUTODOC: restore forwarding value of NA to CPU on the new src eport*/
        prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, valueArr[valueIdx],
                                      ForwardingEnable);

        /* AUTODOC: restore PCL configuration*/
        prvTgfPclEArchRedirectIpclRestore();

        /* AUTODOC: restore mapping ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet( prvTgfDevNum, valueArr[valueIdx],
                                               &savePhyInfo, NULL);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: Iterate through ePorts [0x7FE, 0x1FF, 3]: */
    for (valueIdx = 0; (valueIdx < valueAmount); valueIdx++)
    {

        PRV_UTF_LOG1_MAC( "*** EPort = 0x%04X  *** \n", valueArr[valueIdx]);

        /* AUTODOC: map ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet( prvTgfDevNum, valueArr[valueIdx],
                                               &phyInfo, &savePhyInfo);

        /* AUTODOC: Iterate through eSendPortArr [0x12, 0x64, 0x3E8]: */
        for(portIdx = 0; portIdx < eSendPortNum; portIdx++)
        {
            PRV_UTF_LOG1_MAC(
                "*** Send ePort = 0x%04X  *** \n", eSendPortArr[portIdx]);
            /* AUTODOC: configure PCL rule to assign source ePort*/
            prvTgfPclEArchRedirectConfigurationIpclAdd1(
                    eSendPortArr[portIdx],
                    PRV_IPCL_CFG_INVALID_EPORT_CNS,
                    PRV_IPCL_CFG_INVALID_EVID_CNS,
                    PRV_IPCL_CFG_INVALID_EVIDX_CNS,
                    valueArr[valueIdx]);

            /* AUTODOC: save current value of forwarding of NA to CPU enable */
            prvTgfBrgFdbNaToCpuPerPortGet(prvTgfDevNum, valueArr[valueIdx],
                                          &ForwardingEnable);
            /* AUTODOC: enable forwarding of NA to CPU on the new src eport */
            prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, valueArr[valueIdx],
                                          GT_TRUE);

            /* AUTODOC:  redirect to ePort */

            /* AUTODOC: GENERATE TRAFFIC: */
            prvTgfPclEArchRedirectTrafficGenerate();

            /* AUTODOC: verify traffic: */
            /* AUTODOC: tagged packet received on all ports */
            prvTgfPclEArchFloodingCheck(PRV_TGF_SEND_PORT_IDX_CNS);

            /* AUTODOC: GENERATE TRAFFIC: */
            prvTgfPclEArchRedirectTrafficGenerate2();

            /* AUTODOC: verify traffic: */
            /* AUTODOC: tagged packet received on port 2 */
            prvTgfPclEArchRedirectTrafficCheck( PRV_TGF_SEND_PORT_IDX_CNS,
                                                PRV_TGF_RECEIVE_PORT_IDX_EPORT_CNS);

            /* AUTODOC: restore forwarding value of NA to CPU on the new src eport*/
            prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, valueArr[valueIdx],
                                          ForwardingEnable);

            /* AUTODOC: restore PCL configuration*/
            prvTgfPclEArchRedirectIpclRestore1(eSendPortArr[portIdx]);
        }

        /* AUTODOC: restore mapping ePort to physical port 2 */
        prvTgfPclEArchRedirectMapEport2PhySet( prvTgfDevNum, valueArr[valueIdx],
                                               &savePhyInfo, NULL);
    }
    /* AUTODOC: End iteration. */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEArchRedirectConfigurationRestore();

    PRV_UTF_LOG0_MAC( "*** End of EArch source ePort test  *** \n");
}

