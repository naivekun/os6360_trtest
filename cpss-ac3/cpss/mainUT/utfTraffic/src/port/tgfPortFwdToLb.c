/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfPortFwdToLb.c
*
* DESCRIPTION:
*       Check "forwarding to loopback/service port" (abbreviated as "FTL" below)
*       feature.
*       General idea of all tests is:
*       Send a packet - check the packet(s) egressed target port(s)
*       Enable forwarding to loopback for the target port - check the
*       packet egressed via loopback port instead of target port.
*       There are test cases:
*       - single-target FORWARD packet type
*       - multi-target FORWARD packet type
*       - FROM_CPU packet type
*       - TO_ANALYZER packet type
*       - TO_CPU packet type
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <port/tgfPortFwdToLb.h>
#include <common/tgfCosGen.h>
#include <common/tgfNetIfGen.h>

/* a source port index in prvTgfPortsArray */
#define FWDTOLB_RX_PORT_IX_CNS 0
/* a target port index in prvTgfPortsArray */
#define FWDTOLB_TX_PORT_IX_CNS 1
/* a loopback port index in prvTgfPortsArray */
#define FWDTOLB_LB_PORT_IX_CNS 2
/* analyzer port index in prvTgfPortsArray */
#define FWDTOLB_AN_PORT_IX_CNS 3
/* prvTgfPortsArray index approproate to a port a target port is mapped to
   after loopback eVIDX mapping */
#define FWDTOLB_NEW_TX_PORT_IX_CNS 3

/* source port */
#define FWDTOLB_RX_PORT_CNS (prvTgfPortsArray[FWDTOLB_RX_PORT_IX_CNS])
/* target port */
#define FWDTOLB_TX_PORT_CNS (prvTgfPortsArray[FWDTOLB_TX_PORT_IX_CNS])
/* loopback port  */
#define FWDTOLB_LB_PORT_CNS (prvTgfPortsArray[FWDTOLB_LB_PORT_IX_CNS])
/* analyzer port */
#define FWDTOLB_AN_PORT_CNS (prvTgfPortsArray[FWDTOLB_AN_PORT_IX_CNS])
/* port where a packet destined to FWDTOLB_TX_PORT_CNS goes after
   a Loopback eVIDX Mapping is applied */
#define FWDTOLB_NEW_TX_PORT_CNS (prvTgfPortsArray[FWDTOLB_NEW_TX_PORT_IX_CNS])


/*  FTL will be enabled for profiles pair:
   {rx: FWDTOLB_LB_PROFILE_1_CNS,
    tx: FWDTOLB_LB_PROFILE_2_CNS} */

/* loopback profile 1 */
#define FWDTOLB_LB_PROFILE_1_CNS 1
/* loopback profile 2 */
#define FWDTOLB_LB_PROFILE_2_CNS 2

/* ingress mirroring analyzer index */
#define FWDTOLB_AN_IX_CNS 3

/* traffic class of incoming packet */
#define FWDTOLB_TC_CNS 4

/* CPU code of packet forwarded to cpu */
#define FWDTOLB_CPU_CODE_CNS CPSS_NET_BRIDGED_PACKET_FORWARD_E

/* СPU code not used during forwarding to CPU */
#define FWDTOLB_UNUSED_CPU_CODE_CNS CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E

/* number of sent packets */
#define FWDTOLB_BURST_COUNT_CNS 1

/* convert rc= GT_BAD_PARAM to GT_OK if port can't be stored in device
 * as valid loopback port */
#define FWDTOLB_SET_GT_OK_IF_INVALID_LB_PORT_MAC(rc, dev, port) \
    if (rc == GT_BAD_PARAM &&                                   \
        GT_FALSE == portFwdToLbIsLoobackPortValid(dev, port))   \
    {                                                           \
        /* ignore this error */                                 \
        rc = GT_OK;                                             \
    }                                                           \


/*
 * Typedef: enum FWDTOLB_TEST_CASES_ENT
 *
 * Description:
 *      test cases
 * Fields:
 *     FWDTOLB_TEST_CASES_FWD_SINGLE_E -
 *                single-target FORWARD packet type,
 *                FTL is disabled
 *
 *     FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E -
 *                single-target FORWARD packet type,
 *                FTL is enabled
 *     FWDTOLB_TEST_CASES_FWD_SINGLE_LB_MIRR_E -
 *                single-target FORWARD packet type,
 *                FTL is enabled.
 *                Egress Mirroring for packets forwarded to
 *                a Loopback/Service port is enabled.
 *
 *     FWDTOLB_TEST_CASES_FWD_MULTI_E -
 *                multi-target FORWARD packet type,
 *                FTL is disabled
 *
 *     FWDTOLB_TEST_CASES_FWD_MULTI_LB_E -
 *                multi-target FORWARD packet type,
 *                FTL is enabled
 *     FWDTOLB_TEST_CASES_FWD_MULTI_LB_VIDX_MAP_E -
 *                multi-target FORWARD packet type,
 *                FTL is enabled. VIDX mapping is enabled.
 *
 *     FWDTOLB_TEST_CASES_FROM_CPU_SINGLE_LB_E -
 *                single-target FROM_CPU packet type
 *                FTL is enabled
 *     FWDTOLB_TEST_CASES_TO_ANALYZER_E -
 *                TO_ANALYZER packet type. FTL is disabled.
 *     FWDTOLB_TEST_CASES_TO_ANALYZER_LB_E -
 *                TO_ANALYZER packet type. FTL is enabled.
 *     FWDTOLB_TEST_CASES_TO_CPU_E -
 *                TO_CPU packet type. FTL is disabled.
 *     FWDTOLB_TEST_CASES_TO_CPU_LB_E -
 *                TO_CPU packet type. FTL is enabled.
 *     FWDTOLB_TEST_CASES_TO_CPU_LB_AN_E -
 *                TO_CPU packet type. FTL is enabled.
 *                Loopback port is enabled for egress mirroring
 *
 *  Comments:
 */
typedef enum {
    FWDTOLB_TEST_CASES_FWD_SINGLE_E,
    FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E,
    FWDTOLB_TEST_CASES_FWD_SINGLE_LB_MIRR_E,
    FWDTOLB_TEST_CASES_FWD_MULTI_E,
    FWDTOLB_TEST_CASES_FWD_MULTI_LB_E,
    FWDTOLB_TEST_CASES_FWD_MULTI_LB_VIDX_MAP_E,
    FWDTOLB_TEST_CASES_FROM_CPU_SINGLE_LB_E,
    FWDTOLB_TEST_CASES_TO_ANALYZER_E,
    FWDTOLB_TEST_CASES_TO_ANALYZER_LB_E,
    FWDTOLB_TEST_CASES_TO_CPU_E,
    FWDTOLB_TEST_CASES_TO_CPU_LB_E,
    FWDTOLB_TEST_CASES_TO_CPU_LB_AN_E
} FWDTOLB_TEST_CASES_ENT;


/******************************* TEST PACKETS *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0, 0, 0    /* pri, cfi, vid */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[48] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* EtherType part = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthertypePart =
{
    sizeof prvTgfPayloadDataArr
};


/**************************** PACKET PARTS ARRAY ******************************/
/* parts of the  vlan tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part}, /* tag0 */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/****************************** PACKETS LENGTH ********************************/
/* size of the tagged packet that came from network port  */
#define FWDTOLB_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS                 \
                                 + TGF_VLAN_TAG_SIZE_CNS                \
                                 + TGF_ETHERTYPE_SIZE_CNS               \
                                 + sizeof(prvTgfPayloadDataArr))

/************************** PACKET INFO STRUCTURES ****************************/
/* a vlan tagged packet info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    FWDTOLB_PACKET_SIZE_CNS,                              /* totalLen */
    sizeof(prvTgfPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketPartsArray                                       /* partsArray */
};





/* some values that will be initialized at run time */
static TGF_VLAN_ID prvTgfVlanId;         /* vlan id */
static GT_U16      prvTgfVidx;          /* vidx for multi-target packet */
static GT_U16      prvTgfVidxNew;       /* vidx after loopback */



/* ---------- some original values to be restored at the end of the test */

/* just a shortcut for existing constant*/
#define LB_PROF_NUM CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS
static GT_U32 savedSrcLbProfiles[PRV_TGF_MAX_PORTS_NUM_CNS+1]; /* +1 is CPU  port*/
static GT_U32 savedTrgLbProfiles[PRV_TGF_MAX_PORTS_NUM_CNS+1]; /* +1 is CPU  port*/
static GT_BOOL savedLbEnArr[LB_PROF_NUM][LB_PROF_NUM][CPSS_TC_RANGE_CNS];
static GT_PHYSICAL_PORT_NUM savedLbPortArr[PRV_TGF_MAX_PORTS_NUM_CNS+1];  /* +1 is CPU  port*/
static GT_BOOL                               savedFromCpuEn;
static GT_BOOL                               savedFwdSingleEn;
static GT_BOOL                               savedFwdMultiEn;
static CPSS_PORTS_BMP_STC                    savedVidxMembers;
static CPSS_PORTS_BMP_STC                    savedVidxNewMembers;
static GT_BOOL                               savedEvidxMappingEn;
static GT_U32                                savedEvidxMappingOffset;
static CPSS_CSCD_PORT_TYPE_ENT               savedLbPortCascadeType;
static GT_BOOL                               saveSrcDevFilterEn;
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT savedMirrMode;
static GT_BOOL                               savedRxGlobalMirroringEn;
static GT_U32                                savedRxAnalyzerIx;
static GT_BOOL                               savedTxGlobalMirroringEn;
static GT_U32                                savedTxAnalyzerIx;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC savedAnalyzerIface;
static GT_PHYSICAL_PORT_NUM                  savedAnalyzerLbPort;
static GT_BOOL                               savedAnalyzerLbEn;
static GT_PHYSICAL_PORT_NUM                  savedToCpuLbPort;
static GT_BOOL                               savedToCpuLbEn;
static GT_PHYSICAL_PORT_NUM                  savedToCpuLbPortUnused;
static GT_BOOL                               savedToCpuLbEnUnused;
static GT_BOOL                               savedRxPortMirrModeEn;
static GT_U32                                savedRxPortAnIx;
static GT_BOOL                               savedTxPortMirrModeEn;
static GT_U32                                savedTxPortAnIx;
static GT_BOOL                               savedEgrMirrOnLbEn;
static GT_U32                                savedRxPortQosProfileIx;
static GT_U32                                savedRxPortQosProfileIx;
static PRV_TGF_COS_PROFILE_STC               savedQosProfile;
static GT_U32                                savedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/**
* @internal portFwdToLbInitRandomValues
* @endinternal
*
* @brief    Initialize some random values used by all test cases.
*/
static GT_VOID portFwdToLbInitRandomValues
(
    GT_VOID
)
{
    GT_U32 count;
    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* init vlan with random value 2..4K-1 */
    prvTgfVlanId = 2 + cpssOsRand() % (_4K-2);

    /* init vidx value with random value 1..max */
    count = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.vidxNum;
    prvTgfVidx  = (1 + cpssOsRand() % (count -1)) & 0xFFF; /* limit to 4K */

    /* init vidx value with random value in range 1..max. This value should be
       different than prvTgfVidx and greater than FWDTOLB_TX_PORT_CNS because
       it should satisfy a condition
       prvTgfVidxNew = <Loopback eVIDX offset> + FWDTOLB_TX_PORT_CNS */
    if (count < FWDTOLB_TX_PORT_CNS)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_FAIL, "Port number is too big"
                                     " %d(%d). Test logic should be changed\n",
                                     FWDTOLB_TX_PORT_CNS, count);
    }

    do {
        prvTgfVidxNew  = (FWDTOLB_TX_PORT_CNS + (cpssOsRand() % (count - FWDTOLB_TX_PORT_CNS))) & 0xFFF; /* limit to 4K */
    }
    while (prvTgfVidxNew == prvTgfVidx || prvTgfVidxNew == 0);
}

/**
* @internal portFwdToLbInitRandomValues
* @endinternal
*
* @brief    Print random generated valued
*/
static GT_VOID portFwdToLbPrintRandomValues
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("========== Used random values: ==========\n");
    PRV_UTF_LOG2_MAC("prvTgfVlanId  = %d(0x%x)\n", prvTgfVlanId, prvTgfVlanId);
    PRV_UTF_LOG2_MAC("prvTgfVidx    = %d(0x%x)\n", prvTgfVidx, prvTgfVidx);
    PRV_UTF_LOG2_MAC("prvTgfVidxNew = %d(0x%x)\n", prvTgfVidxNew, prvTgfVidxNew);
}

/**
* @internal portFwdToLbIsLoobackPortValid
* @endinternal
* @brief    Check if specified port is valid to be written
*           in device's registers as loopback port for
*           any packet. This function is used as workaround
*           for situation described in Note.
*
* @note When a UT stores configuration calling one of APIs
*           cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
*           cpssDxChPortLoopbackPktTypeToCpuGet
*           cpssDxChPortLoopbackPktTypeToAnalyzerGet
*       a read loopback port value can be 0 - that is a default register value
*       But this value can't be successfully written back (when we restore
*       configuration) with
*           cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
*           cpssDxChPortLoopbackPktTypeToCpuSet
*           cpssDxChPortLoopbackPktTypeToAnalyzerSet
*       if the device doesn't have physical port 0. API's are failed
*       on checking if portNum is mapped to TxQ port (< SIP5.20).
*       In UT's logic such fail should be ignored.
* @param[in] devNum - device number
* @param[in] portNum - physical port number
* @retval    - GT_TRUE - if portNum is valid
*            - GT_FALSE - if portNum is not valid
*/
static GT_BOOL portFwdToLbIsLoobackPortValid
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_U32               txqPort;
    GT_STATUS            rc;

    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) && portNum == 0)
    {
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
            &txqPort);
        if (rc == GT_BAD_PARAM)
        {
            /* there are no TxQ port */
            return GT_FALSE;
        }
    }
    return GT_TRUE;
}


/**
* @internal portFwdToLbCommonConfigSave
* @endinternal
*
* @brief    Save configurations used by all tests.
*/
static GT_VOID portFwdToLbCommonConfigSave
(
    GT_VOID
)
{
    GT_U32               i;
    GT_U32               j;
    GT_U32               tc;
    GT_PHYSICAL_PORT_NUM port;
    GT_STATUS            rc;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(savedPortsArray, prvTgfPortsArray, sizeof(savedPortsArray));

    /* AUTODOC: save all port's loopback profiles and loopback ports */
    for (i = 0; i <= prvTgfPortsNum; i++)
    {
        port = (i == prvTgfPortsNum) ?
            CPSS_CPU_PORT_NUM_CNS : prvTgfPortsArray[i];
        rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
                                            port,
                                            CPSS_DIRECTION_INGRESS_E,
                                            &savedSrcLbProfiles[i]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfPortsArray[i]);

        rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
                                            port,
                                            CPSS_DIRECTION_EGRESS_E,
                                            &savedTrgLbProfiles[i]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,  port);

        rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
            prvTgfDevNum, port, &savedLbPortArr[i]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, port);
    }

    /* AUTODOC: save general FTL trigger status  */
    for (i = 0; i < CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS; i++)
    {
        for (j = 0; j < CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS; j++)
        {
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
            {
                rc = cpssDxChPortLoopbackEnableGet(
                    prvTgfDevNum, i, j, tc,
                    &savedLbEnArr[i][j][tc]);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, i,j,tc);
            }
        }
    }

    /* AUTODOC: save FTL FORWARD packet type  trigger status*/
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
        prvTgfDevNum, &savedFromCpuEn, &savedFwdSingleEn, &savedFwdMultiEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}



/**
* @internal portFwdToLbMirroringConfigSet
* @endinternal
*
* @brief    Make configuration related to mirroring
*           generic for all UT's using mirroring. I.e.:
*           - set HOP_BY_HOP forwarding mode
*           - set global rx/tx analyzer indexes to be a FWDTOLB_AN_IX_CNS
*           - assign this index with a port FWDTOLB_AN_PORT_CNS.
*/
static GT_VOID portFwdToLbCommonMirrorConfigSet
(
    GT_VOID
)
{
    GT_STATUS                             rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceMirr;

    cpssOsMemSet(&ifaceMirr, 0, sizeof(ifaceMirr));

    /* setup HOP-BY-HOP forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* setup Rx analyzer index */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
                                                       FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* setup Tx analyzer index */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
                                                       FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* assign analyzer index 3 with analyzer port 3 */
    ifaceMirr.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMirr.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceMirr.interface.devPort.portNum  = FWDTOLB_AN_PORT_CNS;

    rc = prvTgfMirrorAnalyzerInterfaceSet(FWDTOLB_AN_IX_CNS, &ifaceMirr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                 ifaceMirr.interface.devPort.portNum);
}

/**
* @internal portFwdToLbCommonMirrorConfigSave
* @endinternal
*
* @brief    Save configurations related to mirroring.
*
* @param[in] ixPortIsCpu - GT_TRUE  - tx port is CPU port.
*                          GT_FALSE - tx port is FWDTOLB_TX_PORT_CNS
*/
static GT_VOID portFwdToLbCommonMirrorConfigSave
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: save a device forwaring mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &savedMirrMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save global rx analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &savedRxGlobalMirroringEn, &savedRxAnalyzerIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save global tx analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &savedTxGlobalMirroringEn, &savedTxAnalyzerIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save an analyzer port bound to analyzer index 3 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          FWDTOLB_AN_IX_CNS,
                                          &savedAnalyzerIface);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save a mirroring status of rx port 0 */
    rc = prvTgfMirrorRxPortGet(FWDTOLB_RX_PORT_CNS,
                               &savedRxPortMirrModeEn,
                               &savedRxPortAnIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save a ePort Egress mirroring of tx port.
                A using of "ePort Egress mirroring" instead of "Physical Port
                Egress mirroing" is important because this two mirroring kinds
                behave different towards to packet destinied to an egress
                port with enabled egress mirroring but forwarded to
                loopback port instead. See description of
                cpssDxChPortLoopbackEnableEgressMirroringGet */
    rc = prvTgfMirrorTxPortTypeGet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                               &savedTxPortMirrModeEn,
                               &savedTxPortAnIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* AUTODOC: save FTL status of analyzer index 3 */
    rc =cpssDxChPortLoopbackPktTypeToAnalyzerGet(prvTgfDevNum,
                                                 FWDTOLB_AN_IX_CNS,
                                                 &savedAnalyzerLbPort,
                                                 &savedAnalyzerLbEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}


/**
* @internal portFwdToLbCommonMirrorConfigRestore
* @endinternal
*
* @brief    Restore configurations related to mirroring.
*
* @param[in] ixPortIsCpu - GT_TRUE  - tx port is CPU port.
*                          GT_FALSE - tx port is FWDTOLB_TX_PORT_CNS
*/
static GT_VOID portFwdToLbCommonMirrorConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: restore FTL status of analyzer index 3 */
    rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum,
                                                  FWDTOLB_AN_IX_CNS,
                                                  savedAnalyzerLbPort,
                                                  savedAnalyzerLbEn);
    FWDTOLB_SET_GT_OK_IF_INVALID_LB_PORT_MAC(rc, prvTgfDevNum, savedAnalyzerLbPort);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore a ePort Egress mirroring status of tx port 1 */
    rc = prvTgfMirrorTxPortTypeSet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                                   savedTxPortMirrModeEn,
                                   savedTxPortAnIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore a mirroring status of rx port 0 */
    rc = prvTgfMirrorRxPortSet(FWDTOLB_RX_PORT_CNS,
                               savedRxPortMirrModeEn,
                               savedRxPortAnIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore an analyzer port bound to analyzer index 3 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(FWDTOLB_AN_IX_CNS,
                                          &savedAnalyzerIface);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore global tx analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(savedTxGlobalMirroringEn,
                                                       savedTxAnalyzerIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore global rx analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(savedRxGlobalMirroringEn,
                                                       savedRxAnalyzerIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore a device forwaring mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(savedMirrMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal portFwdToLbCommonConfigSet
* @endinternal
*
* @brief    Configuration used by all tests
*
* @param[in]        addFdbEntry - create (GT_TRUE) or not (GT_FALSE) an FDB entry
*/
static GT_VOID portFwdToLbCommonConfigSet
(
    IN FWDTOLB_TEST_CASES_ENT testCase
)
{
    GT_STATUS            rc;
    GT_U32               i;
    GT_BOOL              addFdbEntry = GT_TRUE;
    GT_PHYSICAL_PORT_NUM trgPort     = FWDTOLB_TX_PORT_CNS;
    GT_U8                vlanTagsArr[PRV_TGF_MAX_PORTS_NUM_CNS];

    /* in most cases all ports should be tagged */
    cpssOsMemSet(vlanTagsArr, 1, sizeof(vlanTagsArr));

    switch (testCase)
    {
        case FWDTOLB_TEST_CASES_FWD_SINGLE_E:
        case FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E:
        case FWDTOLB_TEST_CASES_FROM_CPU_SINGLE_LB_E:
        case FWDTOLB_TEST_CASES_TO_ANALYZER_E:
        case FWDTOLB_TEST_CASES_TO_ANALYZER_LB_E:
            /* make tx port untagged to distinguish (by size)
             * rx-mirrored and tx-mirrored packets */
            vlanTagsArr[FWDTOLB_TX_PORT_IX_CNS] = 0;
            break;

        case FWDTOLB_TEST_CASES_FWD_MULTI_E:
        case FWDTOLB_TEST_CASES_FWD_MULTI_LB_E:
            addFdbEntry = GT_FALSE;
            break;

        case FWDTOLB_TEST_CASES_TO_CPU_E:
        case FWDTOLB_TEST_CASES_TO_CPU_LB_E:
            trgPort = CPSS_CPU_PORT_NUM_CNS;
            break;
        default:
            /* shouldn't happen */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FAIL, GT_OK);
            break;
    }

    /* AUTODOC: create random vlan prvTgfVlanId with ports 0,1,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanId,
                                           prvTgfPortsArray,
                                           prvTgfDevsArray,
                                           vlanTagsArr,
                                           prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfVlanId);

    /* AUTODOC: init packet's vlan id with prvTgfVlanId */
    prvTgfPacketVlanTag0Part.vid = prvTgfVlanId;

    if (addFdbEntry != GT_FALSE)
    {
        /* AUTODOC: set an FDB entry to send vlan's traffic to tx port 1 */
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                              prvTgfVlanId,
                                              prvTgfDevNum,
                                              trgPort,
                                              GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                    prvTgfVlanId, trgPort);
    }

    /* AUTODOC: set rx port 0 loopback profiles {source #1, target #2} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        FWDTOLB_RX_PORT_CNS,
                                        CPSS_DIRECTION_INGRESS_E,
                                        FWDTOLB_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);

    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        FWDTOLB_RX_PORT_CNS,
                                        CPSS_DIRECTION_EGRESS_E,
                                        FWDTOLB_LB_PROFILE_2_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);

    /* AUTODOC: enable FTL for loopback profiles {source #1, target #2}
       and for all traffic classes (general trigger)*/
    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           FWDTOLB_LB_PROFILE_1_CNS,
                                           FWDTOLB_LB_PROFILE_2_CNS,
                                           i,
                                           GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, i);
    }

}


/**
* @internal portFwdToLbCommonConfigRestore
* @endinternal
*
* @brief    Restore configuration
*/
static GT_VOID portFwdToLbCommonConfigRestore
(
    GT_VOID
)
{
    GT_U32               i;
    GT_U32               j;
    GT_U32               tc;
    GT_PHYSICAL_PORT_NUM port;
    GT_STATUS            rc;

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore all port's loopback profiles and loopback ports */
    for (i = 0; i <= prvTgfPortsNum; i++)
    {
        port = (i == prvTgfPortsNum) ?
            CPSS_CPU_PORT_NUM_CNS : prvTgfPortsArray[i];

        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                            port,
                                            CPSS_DIRECTION_INGRESS_E,
                                            savedSrcLbProfiles[i]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, i, prvTgfPortsArray[i]);

        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                            port,
                                            CPSS_DIRECTION_EGRESS_E,
                                            savedTrgLbProfiles[i]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, i, prvTgfPortsArray[i]);

        rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
            prvTgfDevNum, port, savedLbPortArr[i]);
        FWDTOLB_SET_GT_OK_IF_INVALID_LB_PORT_MAC(rc, prvTgfDevNum, savedLbPortArr[i]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, port, savedLbPortArr[i]);
    }

    /* AUTODOC: restore general FTL trigger status */
    for (i = 0; i < CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS; i++)
    {
        for (j = 0; j < CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS; j++)
        {
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
            {
                rc = cpssDxChPortLoopbackEnableSet(
                    prvTgfDevNum, i, j, tc,
                    savedLbEnArr[i][j][tc]);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, i,j,tc);
            }
        }
    }

    /* AUTODOC: restore FTL FORWARD packet type  trigger status*/
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, savedFromCpuEn, savedFwdSingleEn, savedFwdMultiEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: flush FDB table */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: invalidate vlan prfTgfVlanId */
    rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, savedPortsArray, sizeof(savedPortsArray));
}

/**
* @internal portFwdToLbFromCpuSendPacket
* @endinternal
*
* @brief    Send packet from CPU to port FWDTOLB_RX_PORT_CNS with traffic class
*       FWDTOLB_TC_CNS.
*/
static GT_VOID portFwdToLbFromCpuSendPacket
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    CPSS_INTERFACE_INFO_STC   iface;
    GT_U8                     *buf;
    GT_U32                    bufLen;
    PRV_TGF_NET_TX_PARAMS_STC txParams;
    TGF_DSA_DSA_FROM_CPU_STC  *dsaFromCpuPtr;
    GT_HW_DEV_NUM             hwDevNum;


    cpssOsMemSet(&iface, 0, sizeof(iface));
    cpssOsMemSet(&txParams, 0, sizeof(txParams));

    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");


    /* AUTODOC: prepare tx parameters: FROM_CPU packet, destination is rx port 0 */
    txParams.packetIsTagged                        = GT_TRUE;
    txParams.sdmaInfo.txQueue                      = 2;  /* any 0..7 */
    txParams.sdmaInfo.recalcCrc                    = GT_TRUE;
    txParams.dsaParam.dsaType                      = TGF_DSA_4_WORD_TYPE_E;
    txParams.dsaParam.dsaCommand                   = TGF_DSA_CMD_FROM_CPU_E;
    txParams.dsaParam.commonParams.vid             = (GT_U16)prvTgfVlanId;
    dsaFromCpuPtr = &txParams.dsaParam.dsaInfo.fromCpu;
    dsaFromCpuPtr->tc                              = FWDTOLB_TC_CNS;
    dsaFromCpuPtr->dstInterface.type               = CPSS_INTERFACE_PORT_E;
    dsaFromCpuPtr->dstInterface.devPort.hwDevNum   = hwDevNum;
    dsaFromCpuPtr->dstEport                        =
    dsaFromCpuPtr->dstInterface.devPort.portNum    = 0; /* not used because dstEport is used */
    dsaFromCpuPtr->dstEport                        = FWDTOLB_RX_PORT_CNS;
    dsaFromCpuPtr->extDestInfo.devPort.dstIsTagged = txParams.packetIsTagged;

    /* AUTODOC: allocate packet buffer from the pool and initialize it */


    /* AUTODOC: init pool of packet buffers  */
    rc = prvTgfTrafficGeneratorPoolInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrafficGeneratorPoolInit");

    bufLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    buf    = prvTgfTrafficGeneratorPoolGetBuffer();
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, (GT_UINTPTR)buf,
                                     "prvTgfTrafficGeneratorPoolGetBuffer");


    rc = tgfTrafficEnginePacketBuild(&prvTgfPacketInfo, buf, &bufLen,
                                     NULL, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficEnginePacketBuild");

    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = FWDTOLB_RX_PORT_CNS;

    /* AUTODOC: send the packet */
    rc = prvTgfNetIfSdmaTxPacketSend(prvTgfDevNum,
                                     &txParams,
                                     &buf, &bufLen,
                                     1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaTxPacketSend");

    /* AUTODOC: free pool buffer */
    rc = prvTgfTrafficGeneratorPoolFreeBuffer(buf);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfTrafficGeneratorPoolFreeBuffer");

    /* give a time to update MAC counters  */
    cpssOsTimerWkAfter(50);
}
/**
* @internal portFwdToLbTrafficGenerate
* @endinternal
*
* @brief    Restore configuration
*
* @param[in] isFromCpu - generate FROM_CPU packet type with specified traffic class
*/
static GT_VOID portFwdToLbTrafficGenerate
(
    GT_BOOL isFromCpu
)
{
    GT_U32    portIter;
    GT_STATUS rc;

    /* AUTODOC: prepare the packet to be sent */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, &prvTgfPacketInfo, FWDTOLB_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: reset ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d",
                                     prvTgfPortsArray[portIter]);
    }
    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: send packet */
    if (isFromCpu != GT_FALSE)
    {
        /* It is required to specify traffic class for such packet so other than
           prvTgfStartTransmitingEth function should be used. */
        portFwdToLbFromCpuSendPacket();
    }
    else
    {
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, FWDTOLB_RX_PORT_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
    }
}


/**
* @internal portFwdToLbCheckCounters
* @endinternal
*
* @brief    Check counters on ports for specified test case
*
* @param[in] testCase - test case.
*/
static GT_VOID portFwdToLbCheckCounters
(
    FWDTOLB_TEST_CASES_ENT testCase
)
{
    GT_U32    i;
    GT_U32    expRxArr[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32    expTxArr[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_STATUS rc;

    cpssOsMemSet(expRxArr, 0, sizeof(expRxArr));
    cpssOsMemSet(expTxArr, 0, sizeof(expTxArr));


    expRxArr[FWDTOLB_RX_PORT_IX_CNS] = 1;
    expTxArr[FWDTOLB_RX_PORT_IX_CNS] = 1;

    switch (testCase)
    {
        case FWDTOLB_TEST_CASES_FWD_SINGLE_E:
            expTxArr[FWDTOLB_TX_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E:
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_FWD_SINGLE_LB_MIRR_E:
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_AN_PORT_IX_CNS] = 1;
            break;
        case FWDTOLB_TEST_CASES_FWD_MULTI_E:
            expTxArr[FWDTOLB_TX_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_NEW_TX_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_FWD_MULTI_LB_E:
            expRxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;

            expTxArr[FWDTOLB_TX_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_NEW_TX_PORT_IX_CNS] = 2;
            break;

        case FWDTOLB_TEST_CASES_FWD_MULTI_LB_VIDX_MAP_E:
            expRxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;

            expTxArr[FWDTOLB_NEW_TX_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_FROM_CPU_SINGLE_LB_E:
            /* packet doesn't reach rx-port  */
            expRxArr[FWDTOLB_RX_PORT_IX_CNS] = 0;
            expTxArr[FWDTOLB_RX_PORT_IX_CNS] = 0;
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_TO_ANALYZER_E:
            expTxArr[FWDTOLB_TX_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_AN_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_TO_ANALYZER_LB_E:
            expTxArr[FWDTOLB_TX_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_TO_CPU_E:
            break;

        case FWDTOLB_TEST_CASES_TO_CPU_LB_E:
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            break;

        case FWDTOLB_TEST_CASES_TO_CPU_LB_AN_E:
            expTxArr[FWDTOLB_LB_PORT_IX_CNS] = 1;
            expTxArr[FWDTOLB_AN_PORT_IX_CNS] = 1;
            break;

        default:
            /* shouldn't happen */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FAIL, GT_OK);
            break;
    }

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRxArr[i], expTxArr[i], 0,
                                    FWDTOLB_BURST_COUNT_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRxArr[i], expTxArr[i]);
    }

    if (FWDTOLB_TEST_CASES_TO_CPU_E == testCase ||
        FWDTOLB_TEST_CASES_TO_CPU_LB_E == testCase)
    {
        /* check packets in CPU. Single packet is expected */
        GT_U8   pktBuf[0x100];
        GT_U32  bufLen = sizeof(pktBuf)/ sizeof(pktBuf[0]);
        GT_U32  actualPktLen;
        GT_U8   devNum;
        GT_U8   queue;
        TGF_NET_DSA_STC         rxParams;
        CPSS_INTERFACE_INFO_STC iface;

        tgfTrafficGeneratorRxInCpuNumWait(FWDTOLB_BURST_COUNT_CNS, 100, NULL);

        if (testCase == FWDTOLB_TEST_CASES_TO_CPU_E)
        {
            cpssOsMemSet(&iface, 0, sizeof(iface));
            iface.type = CPSS_INTERFACE_PORT_E;
            iface.devPort.hwDevNum = 0;
            iface.devPort.portNum = FWDTOLB_RX_PORT_CNS;

            /* single packet is expected in CPU  */
            rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(
                &iface, TGF_PACKET_TYPE_REGULAR_E, GT_TRUE, GT_TRUE, pktBuf,
                &bufLen, &actualPktLen, &devNum, &queue, &rxParams);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet\n");

            if (rc == GT_OK)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(FWDTOLB_CPU_CODE_CNS,
                                            rxParams.cpuCode);
            }

        }

        /* there are no more packets  in CPU */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_REGULAR_E, GT_FALSE, GT_TRUE, pktBuf,
            &bufLen, &actualPktLen, &devNum, &queue, &rxParams);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, rc);
    }
}

/**
* @internal portFwdToLbForwardSingleTargetConfigSet
* @endinternal
*
* @brief    Configuration specific for single-target FORWARD packet type
*/
static GT_VOID portFwdToLbForwardSingleTargetConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;
    PRV_TGF_COS_PROFILE_STC qosProfile;


    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    /* AUTODOC: make configuration generic for all FTL tests */
    portFwdToLbCommonConfigSet(FWDTOLB_TEST_CASES_FWD_SINGLE_E);

    /* AUTODOC: set target port 1 loopback profiles {source #2, target #2} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        FWDTOLB_TX_PORT_CNS,
                                        CPSS_DIRECTION_BOTH_E,
                                        FWDTOLB_LB_PROFILE_2_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);


    /* AUTODOC: enable FTL for loopback profiles {source #1, target #2}
       and for all traffic classes (general trigger)*/
    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           FWDTOLB_LB_PROFILE_1_CNS,
                                           FWDTOLB_LB_PROFILE_2_CNS,
                                           i,
                                           GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, i);
    }

    /* AUTODOC: assign target port 1 with loopback port 2 */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
        prvTgfDevNum, FWDTOLB_TX_PORT_CNS, FWDTOLB_LB_PORT_CNS);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                FWDTOLB_TX_PORT_CNS, FWDTOLB_LB_PORT_CNS);

    /* AUTODOC: update rx port QOS profile to assign a packet's traffic class */
    qosProfile              = savedQosProfile;
    qosProfile.trafficClass = FWDTOLB_TC_CNS;
    rc = prvTgfCosProfileEntrySet(savedRxPortQosProfileIx, &qosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}


/**
* @internal portFwdToLbForwardSingleTargetConfigSave
* @endinternal
*
* @brief    Save configurations used by all tests.
*/
static GT_VOID portFwdToLbForwardSingleTargetConfigSave
(
    GT_VOID
)
{
    CPSS_QOS_ENTRY_STC portQosCfg;
    GT_STATUS          rc;

    /* AUTODOC: save some generic configurations used by all FTL UT's */
    portFwdToLbCommonConfigSave();

    /* AUTODOC: save settings related to mirroring */
    portFwdToLbCommonMirrorConfigSave();

    /* AUTODOC: save rx port 0 QOS profile */
    rc = prvTgfCosPortQosConfigGet(prvTgfDevNum, FWDTOLB_RX_PORT_CNS, &portQosCfg);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    savedRxPortQosProfileIx =  portQosCfg.qosProfileId;

    rc = prvTgfCosProfileEntryGet(prvTgfDevNum, savedRxPortQosProfileIx,
                                  &savedQosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save an analyzer port bound to analyzer index 3 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          FWDTOLB_AN_IX_CNS,
                                          &savedAnalyzerIface);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save an ePort Egress mirroring status of tx port 1 */
    rc = prvTgfMirrorTxPortTypeGet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                                   &savedTxPortMirrModeEn,
                                   &savedTxPortAnIx);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save Egress Mirroring on Loopback status */
    rc = cpssDxChPortLoopbackEnableEgressMirroringGet(prvTgfDevNum,
                                                      &savedEgrMirrOnLbEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal portFwdToLbForwardSingleTargetConfigRestore
* @endinternal
*
* @brief    Restore configuration
*/
static GT_VOID portFwdToLbForwardSingleTargetConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: restore Egress Mirroring on Loopback status */
    rc = cpssDxChPortLoopbackEnableEgressMirroringSet(prvTgfDevNum,
                                                      savedEgrMirrOnLbEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* AUTODOC: restore rx port 0 QOS profile */
    rc = prvTgfCosProfileEntrySet(savedRxPortQosProfileIx, &savedQosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save settings related to mirroring */
    portFwdToLbCommonMirrorConfigRestore();

    /* AUTODOC: restore settings common for all FTL tests */
    portFwdToLbCommonConfigRestore();
}

/**
* @internal tgfPortFwdToLbForwardSingleTargetTest
* @endinternal
*
* @brief    Check FTL for single-target FORWARD and FROM_CPU packet types.
*/
GT_VOID tgfPortFwdToLbForwardSingleTargetTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;

    portFwdToLbInitRandomValues();
    portFwdToLbPrintRandomValues();
    /* AUTODOC: save device settings */
    portFwdToLbForwardSingleTargetConfigSave();

    /* AUTODOC: configure a device */
    portFwdToLbForwardSingleTargetConfigSet();

    /* --------------- CHECK SINGLE-TARGET FORWARD TRAFFIC ---------------- */
    PRV_UTF_LOG0_MAC ("========== FTL is disabled ==========\n");

    /* AUTODOC: send packet with disabled FTL.
                Packet is expected on the target port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_E);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for FORWARD multi-target"
                      " (all tc)\n");
    /* AUTODOC: enable FTL for multi-target FORWARD packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, GT_FALSE, GT_FALSE, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send a single-target packet.
                Packet is expected on the target port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_E);

    /* AUTODOC: enable FTL for single-target FORWARD packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, GT_FALSE, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for FORWARD single-target"
                      " (all tc)\n");
    /* AUTODOC: send a single-target packet.
                Packet is expected on the loopback port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E);

    if(!PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(prvTgfDevNum))
    {
        PRV_UTF_LOG1_MAC ("========== Enable FTL for FORWARD single-target (tc=%d)\n",
                          FWDTOLB_TC_CNS);
        /* AUTODOC: disable FTL for all traffic classes except FWDTOLB_TC_CNS */
        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            if (i != FWDTOLB_TC_CNS)
            {
                rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                                   FWDTOLB_LB_PROFILE_1_CNS,
                                                   FWDTOLB_LB_PROFILE_2_CNS,
                                                   i, GT_FALSE);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, i);
            }
        }
    }

    /* AUTODOC: send a single-target packet.
                Packet is expected on the loopback port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E);

    /**************************************************************************/
    /* AUTODOC: check 'Disable Egress Mirroring when transmitting to a
     * Loopback/Service Port */

    PRV_UTF_LOG0_MAC ("========== Check 'Disable Egress Mirroring' \n");

    /* AUTUOC: enable mirroring globally (generic trigger) */
    portFwdToLbCommonMirrorConfigSet();

    /* AUTODOC: enable ePort Egress Mirroring to analyzer index 3 for tx port 1 */
    rc = prvTgfMirrorTxPortTypeSet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                                   GT_TRUE, FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* enable Egress Mirroring for packets redirected to loopback port */
    rc = cpssDxChPortLoopbackEnableEgressMirroringSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send a single-target packet.
                Packet is expected on the loopback port and analyzer port */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_LB_MIRR_E);

    /* disable Egress Mirroring for packets redirected to loopback port */
    rc = cpssDxChPortLoopbackEnableEgressMirroringSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send a single-target packet.
                Packet is expected on the loopback port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_SINGLE_LB_E);

    /* AUTODOC: disable ePort Egress Mirroring for tx port 1 */
    rc = prvTgfMirrorTxPortTypeSet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                                   GT_TRUE, FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTUOC: Disable mirroring globally (generic trigger) */
    portFwdToLbCommonMirrorConfigRestore();
    /**************************************************************************/



    /* --------------- CHECK SINGLE-TARGET FROM-CPU TRAFFIC ---------------
       TGF creates FROM_CPU packet to generate traffic. So the idea is enable
       Forwarding to Loopback for pair "CPU port"-> "rx port 0", assign
       rx port 0 with loopback port 2 and traffic to port rx.
       If FTL is enabled packet will not achieve rx port, will be forwarded
       to port 2 instead.
     */
    PRV_UTF_LOG1_MAC ("========== Enable FTL for FROM_CPU single-target (tc=%d)\n",
                      FWDTOLB_TC_CNS);

    /* AUTODOC: assign CPU port with loopback profiles {source #1, target #1}.
       A pair {source #1, source #2} is enabled for FTL and a port 0 target
       profile is #2. So packet sent from CPU to port 0 satisfies
       general FTL trigger requirements */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        CPSS_CPU_PORT_NUM_CNS,
                                        CPSS_DIRECTION_BOTH_E,
                                        FWDTOLB_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);

    /* AUTODOC: enable FTL for single-target FROM_CPU packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, GT_TRUE, GT_TRUE, GT_FALSE);

    /* AUTODOC: assign rx port 0 with loopback port 2 */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
        prvTgfDevNum, FWDTOLB_RX_PORT_CNS, FWDTOLB_LB_PORT_CNS);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                FWDTOLB_TX_PORT_CNS, FWDTOLB_LB_PORT_CNS);

    /* AUTODOC: send a single-target FROM_CPU packet.
                Packet is expected on the loopback port only */
    portFwdToLbTrafficGenerate(GT_TRUE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FROM_CPU_SINGLE_LB_E);

    /* AUTODOC: restore device configuration */
    portFwdToLbForwardSingleTargetConfigRestore();
}


/**
* @internal portFwdToLbForwardMultiTargetConfigSet
* @endinternal
*
* @brief    Configuration specific for multi-target FORWARD packet type
*

*/
static GT_VOID portFwdToLbForwardMultiTargetConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    CPSS_INTERFACE_INFO_STC iface;
    GT_U32                  i;
    CPSS_PORTS_BMP_STC      portsBmp;
    PRV_TGF_COS_PROFILE_STC qosProfile;

    cpssOsMemSet(&iface,    0, sizeof(iface));
    cpssOsMemSet(&portsBmp, 0, sizeof(portsBmp));

    /* AUTODOC: make configuration generic for all FTL tests */
    portFwdToLbCommonConfigSet(FWDTOLB_TEST_CASES_FWD_MULTI_E);

    /* AUTODOC: create vidx prvTgfVidx with target ports 1,3 */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, FWDTOLB_TX_PORT_CNS);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, FWDTOLB_NEW_TX_PORT_CNS);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, prvTgfVidx, &portsBmp);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfVidx,
                                FWDTOLB_TX_PORT_CNS, FWDTOLB_NEW_TX_PORT_CNS);

    /* AUTODOC: create vidx prvTgfVidxNew with target port 3 */
    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsBmp, FWDTOLB_TX_PORT_CNS);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, prvTgfVidxNew, &portsBmp);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfVidxNew, FWDTOLB_TX_PORT_CNS);

    /* AUTODOC: create an FDB entry {prvTgfVlanId, MAC DA, prvTgfVidx} */
    rc =  prvTgfBrgDefFdbMacEntryOnVidxSet(
        prvTgfPacketL2Part.daMac, prvTgfVlanId, prvTgfVidx, GT_TRUE);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfVlanId, prvTgfVidx);

    /* AUTODOC: set target port 1 loopback profiles {source #2, target #2} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, FWDTOLB_TX_PORT_CNS,
                                        CPSS_DIRECTION_BOTH_E,
                                        FWDTOLB_LB_PROFILE_2_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);

    /* AUTODOC: assign target port 1 with loopback port 2 */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
        prvTgfDevNum, FWDTOLB_TX_PORT_CNS, FWDTOLB_LB_PORT_CNS);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                FWDTOLB_TX_PORT_CNS, FWDTOLB_LB_PORT_CNS);

    /* AUTODOC: configure port 2 as a loopback port */
    iface.type = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum = prvTgfDevNum;
    iface.devPort.portNum  = FWDTOLB_LB_PORT_CNS;
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&iface, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, iface.devPort.portNum);

    /* AUTODOC: configure port 2 as a cascade port with disabled filtering of
     * DSA tagged packets with srcDev = local device. */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_BOTH_E,
                               FWDTOLB_LB_PORT_CNS,
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_LB_PORT_CNS);

    rc = prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: enable FTL for loopback profiles {source #1, target #2}
       for all traffic classes  (general trigger)*/
    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           FWDTOLB_LB_PROFILE_1_CNS,
                                           FWDTOLB_LB_PROFILE_2_CNS,
                                           i,
                                           GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, i);
    }

    /* AUTODOC: update rx port QOS profile to assign a packet's traffic class */
    qosProfile              = savedQosProfile;
    qosProfile.trafficClass = FWDTOLB_TC_CNS;
    rc = prvTgfCosProfileEntrySet(savedRxPortQosProfileIx, &qosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}


/**
* @internal portFwdToLbForwardMultiTargetConfigSave
* @endinternal
*
* @brief    Save configurations used by tests for multi-target FORWARD packet type
*/
static GT_VOID portFwdToLbForwardMultiTargetConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save some generic configurations used by all FTL UT's */
    portFwdToLbCommonConfigSave();

    /* AUTODOC: save used VIDXs members */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, prvTgfVidx, &savedVidxMembers);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, prvTgfVidxNew, &savedVidxNewMembers);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save cascade status of loopback port 2 */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               FWDTOLB_LB_PORT_CNS,
                               CPSS_PORT_DIRECTION_BOTH_E,
                               &savedLbPortCascadeType);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save status of the filter "ingress DSA source device is local" */
    rc = prvTgfCscdDsaSrcDevFilterGet(&saveSrcDevFilterEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save ""loopback multi-target eVIDX mapping"" state */
    rc = cpssDxChPortLoopbackEvidxMappingGet(prvTgfDevNum,
                                             &savedEvidxMappingOffset,
                                             &savedEvidxMappingEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: save rx port 0 QOS profile */
    rc = prvTgfCosProfileEntryGet(prvTgfDevNum, savedRxPortQosProfileIx,
                                  &savedQosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}


/**
* @internal portFwdToLbForwardMultiTargetConfigRestore
* @endinternal
*
* @brief    Restore configuration
*/
static GT_VOID portFwdToLbForwardMultiTargetConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC iface;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: restore rx port 0 QOS profile */
    rc = prvTgfCosProfileEntrySet(savedRxPortQosProfileIx, &savedQosProfile);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* AUTODOC: restore ""loopback multi-target eVIDX mapping"" state */
    rc = cpssDxChPortLoopbackEvidxMappingSet(prvTgfDevNum,
                                             savedEvidxMappingOffset,
                                             savedEvidxMappingEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore status of the filter "ingress DSA source device is local" */
    rc = prvTgfCscdDsaSrcDevFilterSet(saveSrcDevFilterEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore cascade status of loopback port 2 */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_BOTH_E,
                               FWDTOLB_LB_PORT_CNS,
                               savedLbPortCascadeType);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore used VIDX members */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, prvTgfVidx, &savedVidxMembers);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, prvTgfVidxNew, &savedVidxNewMembers);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* AUTODOC: disable loopback mode on loopback port */
    iface.type             = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum = prvTgfDevNum;
    iface.devPort.portNum  = FWDTOLB_LB_PORT_CNS;
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&iface, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, iface.devPort.portNum);

    /* AUTODOC: restore settings common for all FTL tests */
    portFwdToLbCommonConfigRestore();
}

/**
* @internal tgfPortFwdToLbForwardMultiTargetTest
* @endinternal
*
* @brief    Check FTL feature for multi-target FORWARD packet type.
*/
GT_VOID tgfPortFwdToLbForwardMultiTargetTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 vidxOffset;
    CPSS_PORTS_BMP_STC portsBmp = {{0}};

    portFwdToLbInitRandomValues();
    portFwdToLbPrintRandomValues();
    portFwdToLbForwardMultiTargetConfigSave();
    /* AUTODOC: configure a device */
    portFwdToLbForwardMultiTargetConfigSet();

    PRV_UTF_LOG0_MAC ("========== FTL is disabled\n");

    /* AUTODOC: send packet. Packet is expected on the target vidx port only */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_MULTI_E);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for FORWARD single-target"
                      " (all tc)\n");

    /* AUTODOC: enable FTL for single-target FORWARD packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, GT_FALSE, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send a packet. It is expected on the target vidx ports 1,3 */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_MULTI_E);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for FORWARD multi-target"
                      " (all tc)\n");

    /* AUTODOC: enable FTL for multi-target FORWARD packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        prvTgfDevNum, GT_FALSE, GT_FALSE, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send a packet. It is expected on loopback port (instead of port 1),
       on target port 1 (after loopback) and on target port 3 twice:
       - before and after loopback. */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_MULTI_LB_E);

    /* AUTODOC: disable FTL for all traffic classes except FWDTOLB_TC_CNS */
    if(!PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(prvTgfDevNum))
    {
        PRV_UTF_LOG1_MAC ("========== FTL is enabled for FORWARD multi-target"
                          " (tc=%d)\n", FWDTOLB_TC_CNS);

        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            if (i != FWDTOLB_TC_CNS)
            {
                rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                                   FWDTOLB_LB_PROFILE_1_CNS,
                                                   FWDTOLB_LB_PROFILE_2_CNS,
                                                   i, GT_FALSE);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, i);
            }
        }

        /* AUTODOC: send a packet. It is expected on loopback port (instead of port 1),
           on target port 1 (after loopback) and on target port 3 twice:
           - before and after loopback. */
        portFwdToLbTrafficGenerate(GT_FALSE);
        portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_MULTI_LB_E);
    }

    /* AUTODOC: enable loopback multi-target eVIDX mapping. Offset is taken to
       satisfy an expression: offset + FWDTOLB_TX_PORT_CNS == prvTgfVidxNew" */
    vidxOffset = prvTgfVidxNew - FWDTOLB_TX_PORT_CNS;
    PRV_UTF_LOG3_MAC (
        "========== enable VIDX mapping, offset %d, old vidx %d, new vidx %d\n",
        vidxOffset, prvTgfVidx, prvTgfVidxNew);

    rc = cpssDxChPortLoopbackEvidxMappingSet(prvTgfDevNum, vidxOffset, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, vidxOffset);

    /* AUTODOC: reconfigure prvTgfVidx to contain port 1 only. It is enough
       enough to check loopback multi-target eVIDX mapping */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, FWDTOLB_TX_PORT_CNS);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, prvTgfVidx, &portsBmp);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfVidx, FWDTOLB_TX_PORT_CNS);

    /* AUTODOC: send a packet. It is expected on the loopback port 2 and on
                the port 3 (port 1 is mapped to it) */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_FWD_MULTI_LB_VIDX_MAP_E);

    /* AUTODOC: restore device configuration */
    portFwdToLbForwardMultiTargetConfigRestore();
}


/**
* @internal portFwdToLbToAnalyzerConfigSet
* @endinternal
*
* @brief    Configuration specific for TO_ANALYZER packet type
*
* @param[in]        isRxMirr - GT_TRUE:  enable rx mirroring , disable tx mirroring,
*                       GT_FALSE: disable rx mirroring, enable tx mirroring
*/
static GT_VOID portFwdToLbToAnalyzerConfigSet
(
    GT_BOOL isRxMirr
)
{
    GT_STATUS                             rc;
    GT_BOOL                               isTxMirr;

    isTxMirr = (GT_FALSE == isRxMirr? GT_TRUE: GT_FALSE);

    /* AUTODOC: make configuration generic for all FTL tests */
    portFwdToLbCommonConfigSet(FWDTOLB_TEST_CASES_TO_ANALYZER_E);


    /* AUTODOC: setup an analyzer index 3 bound to port 3 */

    portFwdToLbCommonMirrorConfigSet();

    /* AUTODOC: setup rx port 0 mirroring to analyzer index 3 */
    rc = prvTgfMirrorRxPortSet(FWDTOLB_RX_PORT_CNS, isRxMirr, FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTODOC: setup ePort Egress Mirroring to analyzer index 3 for tx port 1 */
    rc = prvTgfMirrorTxPortTypeSet(FWDTOLB_TX_PORT_CNS, GT_FALSE,
                                   isTxMirr, FWDTOLB_AN_IX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortTypeSet");


    /* AUTODOC: set analyzer port 3 loopback profiles {source 2, target 2} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, FWDTOLB_AN_PORT_CNS,
                                        CPSS_DIRECTION_BOTH_E,
                                        FWDTOLB_LB_PROFILE_2_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, FWDTOLB_RX_PORT_CNS);
}

/**
* @internal portFwdToLbToAnalyzerConfigSave
* @endinternal
*
* @brief    Save configurations used by test for TO_ANALYZER packet type
*/
static GT_VOID portFwdToLbToAnalyzerConfigSave
(
    GT_VOID
)
{
    /* AUTODOC: save some generic configurations used by all FTL UT's */
    portFwdToLbCommonConfigSave();

    /* AUTODOC: save settings related to mirroring */
    portFwdToLbCommonMirrorConfigSave();
}

/**
* @internal portFwdToLbToAnalyzerConfigRestore
* @endinternal
*
* @brief    Restore configuration
*/
static GT_VOID portFwdToLbToAnalyzerConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: save settings related to mirroring */
    portFwdToLbCommonMirrorConfigRestore();

    /* AUTODOC: restore settings common for all FTL tests */
    portFwdToLbCommonConfigRestore();
}


/**
* @internal tgfPortFwdToLbToAnalyzerTest
* @endinternal
*
* @brief    Check FTL feature for TO_ANALYER packet type.
*/
GT_VOID tgfPortFwdToLbToAnalyzerTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_BOOL isRx;
    GT_U32 i;

    portFwdToLbInitRandomValues();
    portFwdToLbPrintRandomValues();

    portFwdToLbToAnalyzerConfigSave();

    PRV_UTF_LOG4_MAC("Used ports:"
                     " rx = %d, tx = %d, loopback = %d, analyzer = %d\n",
                     FWDTOLB_RX_PORT_CNS,
                     FWDTOLB_TX_PORT_CNS,
                     FWDTOLB_LB_PORT_CNS,
                     FWDTOLB_AN_PORT_CNS);

    for (i = 0; i<2; i++)
    {
        isRx = BIT2BOOL_MAC(i);
        PRV_UTF_LOG1_MAC ("========== Check %s Mirroring ==========\n",
                          (GT_TRUE == isRx ? "Rx":"Tx"));

        /* AUTODOC: configure a device. Enable rx-mirroring on port 0 */
        portFwdToLbToAnalyzerConfigSet(isRx);

        /* AUTODOC: Disable FTL TO_ANALYZER trigger for analyzer index 3 */
        rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum,
                                                      FWDTOLB_AN_IX_CNS,
                                                      FWDTOLB_LB_PORT_CNS,
                                                      GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                    FWDTOLB_AN_IX_CNS, FWDTOLB_LB_PORT_CNS);

        PRV_UTF_LOG0_MAC ("========== FTL is disabled ==========\n");
        /* AUTODOC: send packet (FTL is not activated per packet type).
           Packet is expected on the target port 1, analyzer port 3 */
        portFwdToLbTrafficGenerate(GT_FALSE);
        portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_TO_ANALYZER_E);

        PRV_UTF_LOG0_MAC ("========== FTL is enabled for TO_ANALYZER (all tc)\n");

        /* AUTODOC: Enable forwarding to loopback port 2 for analyzer index 3 */
        rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum,
                                                      FWDTOLB_AN_IX_CNS,
                                                      FWDTOLB_LB_PORT_CNS,
                                                      GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc,
                                    FWDTOLB_AN_IX_CNS, FWDTOLB_LB_PORT_CNS);

        /* AUTODOC: send packet. Packet is expected on the target port 1,
           loopback port 2 */
        portFwdToLbTrafficGenerate(GT_FALSE);
        portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_TO_ANALYZER_LB_E);
    }

    /* AUTODOC: restore a device configuration */
    portFwdToLbToAnalyzerConfigRestore();
}


/**
* @internal portFwdToLbToCpuConfigSet
* @endinternal
*
* @brief    Configuration specific for TO_CPU packet type
*

*/
static GT_VOID portFwdToLbToCpuConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: make configuration generic for all FTL tests */
    portFwdToLbCommonConfigSet(FWDTOLB_TEST_CASES_TO_CPU_E);

   /* AUTODOC: set cpu port 63 loopback profiles {source 2, target 2}.  */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                                        CPSS_DIRECTION_BOTH_E,
                                        FWDTOLB_LB_PROFILE_2_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* if don't do this packets received in CPU are treated by TGF
       as unexpected */
    prvTgfTrafficTableRxStartCapture(GT_TRUE);
}

/**
* @internal portFwdToLbToCpuConfigSave
* @endinternal
*
* @brief    Save configuration specific for TO_CPU packet type
*/
static GT_VOID portFwdToLbToCpuConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save some generic configurations used by all FTL UT's */
    portFwdToLbCommonConfigSave();

    /* AUTODOC: save FTL status for TO_CPU packet type and for two used CPU codes */
    rc = cpssDxChPortLoopbackPktTypeToCpuGet(prvTgfDevNum,
                                             FWDTOLB_CPU_CODE_CNS,
                                             &savedToCpuLbPort,
                                             &savedToCpuLbEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = cpssDxChPortLoopbackPktTypeToCpuGet(prvTgfDevNum,
                                             FWDTOLB_UNUSED_CPU_CODE_CNS,
                                             &savedToCpuLbPortUnused,
                                             &savedToCpuLbEnUnused);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal portFwdToLbToCpuConfigRestore
* @endinternal
*
* @brief    Restore configuration
*/
static GT_VOID portFwdToLbToCpuConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* stop packets capturing */
    prvTgfTrafficTableRxStartCapture(GT_FALSE);

    /* AUTODOC: restore FTL status for TO_CPU packet type and for two used CPU codes */
    rc = cpssDxChPortLoopbackPktTypeToCpuSet(prvTgfDevNum,
                                             FWDTOLB_CPU_CODE_CNS,
                                             savedToCpuLbPort,
                                             savedToCpuLbEn);
    FWDTOLB_SET_GT_OK_IF_INVALID_LB_PORT_MAC(rc, prvTgfDevNum, savedToCpuLbPort);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = cpssDxChPortLoopbackPktTypeToCpuSet(prvTgfDevNum,
                                             FWDTOLB_UNUSED_CPU_CODE_CNS,
                                             savedToCpuLbPortUnused,
                                             savedToCpuLbEnUnused);
    FWDTOLB_SET_GT_OK_IF_INVALID_LB_PORT_MAC(rc, prvTgfDevNum, savedToCpuLbPortUnused);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore a device configurations */
    portFwdToLbCommonConfigRestore();
}


/**
* @internal tgfPortFwdToLbToCpuTest function
* @endinternal
*
* @brief    Check FTL feature for TO_CPU packet type.
*        Feature "Disable egress mirroring On loopback" is checked here too.
*/
GT_VOID tgfPortFwdToLbToCpuTest
(
    GT_VOID
)
{
    GT_STATUS rc;

    portFwdToLbInitRandomValues();
    portFwdToLbPrintRandomValues();

    portFwdToLbToCpuConfigSave();

    /* AUTODOC: configure a device */
    portFwdToLbToCpuConfigSet();

    PRV_UTF_LOG0_MAC ("========== FTL is disabled ==========\n");

    /* AUTODOC: send packet. Packet is expected in CPU */

    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_TO_CPU_E);

    /* AUTODOC: enable FTL for TO_CPU packet type and for 'wrong' CPU code */
    rc = cpssDxChPortLoopbackPktTypeToCpuSet(
        prvTgfDevNum, FWDTOLB_UNUSED_CPU_CODE_CNS, FWDTOLB_LB_PORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for TO_CPU with wrong"
                      " CPU code\n");
    /* AUTODOC: send packet. Packet is expected in CPU */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_TO_CPU_E);

    /* AUTODOC: enable FTL for TO_CPU packet type and for 'correct' CPU code */
    rc = cpssDxChPortLoopbackPktTypeToCpuSet(
        prvTgfDevNum, FWDTOLB_CPU_CODE_CNS, FWDTOLB_LB_PORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC ("========== FTL is enabled for TO_CPU with correct"
                      " CPU code\n");

    /* AUTODOC: send a packet. Packet is expected on loopback port 2. */
    portFwdToLbTrafficGenerate(GT_FALSE);
    portFwdToLbCheckCounters(FWDTOLB_TEST_CASES_TO_CPU_LB_E);

    /* AUTODOC: restore a device configuration */
    portFwdToLbToCpuConfigRestore();
}
