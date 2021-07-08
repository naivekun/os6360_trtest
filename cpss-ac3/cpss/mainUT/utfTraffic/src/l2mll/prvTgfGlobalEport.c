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
* @file prvTgfGlobalEport.c
*
* @brief L2 MLL Global ePort test
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfIpGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfLttEvidxRange.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0
/* target device */
#define PRV_TGF_TRG_DEV_CNS         1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* Parameters needed to be restored */
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCscdLinkType;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
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
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfGlobalEportConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC        lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;
    GT_U32                               portsArray[1];
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink  = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* save the current cascade map table entry */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");

    /* create eVidx 1 */
    portsArray[0] = prvTgfPortsArray[1];

    /* AUTODOC: create eVIDX 1 with ports [1] */
    rc = prvTgfBrgVidxEntrySet(1, portsArray, NULL, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, eVIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 1");

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector=PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum + 1;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC:   nextMllPointer=0, dev=1, port=0, last=True */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* AUTODOC: set link up interface map as the cascade trunk with trunk number 0 for egress filtering */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    cascadeLink.linkNum  = 0;

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

}

/**
* @internal prvTgfGlobalEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfGlobalEportTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  counters;
    PRV_TGF_CFG_GLOBAL_EPORT_STC ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC global;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

    /* clear global ePorts */
    ecmp.enable = GT_FALSE;
    ecmp.pattern = 0x0;
    ecmp.mask = 0x0;
    global.enable = GT_FALSE;
    global.pattern = 0x0;
    global.mask = 0x0;

    /* AUTODOC: clear Global ePorts */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);

    /* clear Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: check L2 MLL exception Skip counter is 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counters.skip,
                                 "Skip counter different then expected");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] as global ePort */
    ecmp.enable = GT_TRUE;
    ecmp.pattern = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ecmp.mask = (GT_U32) -1;
    global = ecmp;

    /* AUTODOC: configure Global ePorts: */
    /* AUTODOC:   ECMP primary ePorts port=1 */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: check L2 MLL exception Skip counter is 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, counters.skip,
                                 "Skip counter different then expected");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* clear global ePorts */
    ecmp.enable = GT_FALSE;
    global.enable = GT_FALSE;

    /* AUTODOC: clear Global ePorts */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                 prvTgfDevNum);
}

/**
* @internal prvTgfGlobalEportConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear eVidx 1 */
    rc = prvTgfBrgVidxEntrySet(1, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    lttIndexArray[0] = 0;
    mllIndexArray[0] = 0;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore the cascade map table's entry */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  prvTgfSavedSrcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

}


