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
* @file prvTgfCscdDsaRxMirrSrcFlds.c
*
* @brief Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
* <SrcTrg Tagged> of a packet rx-mirrored to a remote port.
* vlan tagged and untagged packets are checked. The packets are sent:
* - from a network port to a cascade port
* - from a cascade port to a cascade port
* - from a cascade port to a network port
*
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
#include <common/tgfCscdGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfMirror.h>
#include <common/tgfBridgeGen.h>
#include <cscd/prvTgfCscdDsaRxMirrSrcFlds.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* global analyzer index used by the test */
#define PRV_TGF_ANALYZER_IDX_CNS 2
/* ingress port analyzer index used by the test (lower than 'global' index (PRV_TGF_ANALYZER_IDX_CNS) */
#define PRV_TGF_INGRESS_PORT_ANALYZER_IDX_CNS 0

/* max vlan id value. Used for random generation of vlan id */
#define PRV_TGF_MAX_VLAN_ID      _4K

#define PRV_TGF_PAYLOAD_LEN_CNS   0x30 /* 48 */

/******************************* TEST PACKETS *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0, 0, 0    /* pri, cfi, vid */ /* pri will be initialized later */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[PRV_TGF_PAYLOAD_LEN_CNS] =
                                           {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};


/* EtherType part (for untagged packet) = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof prvTgfPayloadDataArr
};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfDsaTag = {
    TGF_DSA_CMD_TO_ANALYZER_E, /*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E, /*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        0,/*vid*/ /* will be filled later */
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

static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfDsaInfo_toAnalyzer = {
    GT_TRUE,/*rxSniffer - ingress mirroring */
    GT_TRUE,/*isTagged. Will be filled later */

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        0,/*hwDevNum*/
        0,/*portNum*/
        0/*ePort*/
    },/*devPort*/

    CPSS_INTERFACE_PORT_E,/*analyzerTrgType*/
    /* union */
    /*
    union{
        struct{
            GT_U16                          analyzerEvidx;
        }multiDest;

        struct{
            GT_BOOL                         analyzerIsTrgPortValid;
            GT_HW_DEV_NUM                   analyzerHwTrgDev;
            GT_PHYSICAL_PORT_NUM            analyzerTrgPort;
            GT_PORT_NUM                     analyzerTrgEport;
        }devPort;
    } extDestInfo;
    */
    {{0}} /* will be filled later */
};

/****************************** PACKETS LENGTH ********************************/

/* size of the untagged packet that came from network port  */
#define PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                            + TGF_ETHERTYPE_SIZE_CNS         \
                                            + sizeof(prvTgfPayloadDataArr))

/* size of the untagged packet that came from cascade port */
#define PRV_TGF_CSCD_UNTAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                            + TGF_eDSA_TAG_SIZE_CNS          \
                                            + TGF_ETHERTYPE_SIZE_CNS         \
                                            + sizeof(prvTgfPayloadDataArr))

/* size of the tagged packet that came from network port  */
#define PRV_TGF_NETW_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_VLAN_TAG_SIZE_CNS          \
                                          + sizeof(prvTgfPayloadDataArr))

/* size of the tagged packet that came from cascade port */
#define PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_eDSA_TAG_SIZE_CNS          \
                                          + sizeof(prvTgfPayloadDataArr))


/**************************** PACKET PARTS ARRAY ******************************/

/* parts of untagged packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetwUntaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of untagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdUntaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the  tagged packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetwTaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part}, /* tag0 */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdTaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/************************** PACKET INFO STRUCTURES ****************************/

/* a network  tagged packet info */
static TGF_PACKET_STC prvTgfNetwTaggedPacketInfo = {
    PRV_TGF_NETW_TAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfNetwTaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetwTaggedPacketPartsArray                                       /* partsArray */
};

/* a cascade tagged packet info */
static TGF_PACKET_STC prvTgfCscdTaggedPacketInfo = {
    PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCscdTaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCscdTaggedPacketPartsArray                                       /* partsArray */
};

/* a network untagged packet info */
static TGF_PACKET_STC prvTgfNetwUntaggedPacketInfo = {
    PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfNetwUntaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetwUntaggedPacketPartsArray                                       /* partsArray */
};

/* a cascade untagged packet info */
static TGF_PACKET_STC prvTgfCscdUntaggedPacketInfo = {
    PRV_TGF_CSCD_UNTAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCscdUntaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCscdUntaggedPacketPartsArray                                       /* partsArray */
};


/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some values that will be initialized at run time */
static TGF_VLAN_ID prvTgfTaggedVlanId;         /* vlan id */
static TGF_VLAN_ID prvTgfUntaggedVlanId;      /* new PVID for rx port */
static GT_U32      prvTgfRxPortIx;       /* rx port index */
static GT_U32      prvTgfAnPortIx;       /* analyzer port index*/
static GT_U8       prvTgfRxDefaultUp;      /* default user priority of rx port */


/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static GT_U32                                         prvTgfSavedGlobalAnalazyerIndex;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedRxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedRxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedRxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedAnCscdPortType;
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static GT_U16                                         prvTgfSavedRxPvid;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];


static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   testMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
static GT_U32   final_analyzerIndex = PRV_TGF_ANALYZER_IDX_CNS;

/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/*************************** PRIVATE DECLARATIONS *****************************/
/**
* @internal prvTgfCscdDsaRxMirrSrcFldsConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   prvTgfRxPortIx,
                                   prvTgfAnPortIx,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: save default Hw device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: save forwaringd mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeGet");

    /* AUTODOC: save global Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfSavedGlobalAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to index final_analyzerIndex  */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          final_analyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of rx port */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[prvTgfRxPortIx],
                               &prvTgfSavedRxMirrorModeEnabled,
                               &prvTgfSavedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet error");

    /* AUTODOC: save a cascade configuration of the rx-port port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               CPSS_PORT_DIRECTION_RX_E,
                               &prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* AUTODOC: save a cascade configuration of the analyzer port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_PORT_DIRECTION_TX_E,
                               &prvTgfSavedAnCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* AUTODOC: save a PVID of rx port */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[prvTgfRxPortIx],
                                 &prvTgfSavedRxPvid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet");
}

/**
* @internal prvTgfCscdDsaRxMirrSrcFldsConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsConfigSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceMir;
    GT_U8                                 tag       = 1;
    GT_STATUS                             rc;

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* we run the test with 'end-to-end' !!!
        (the 'hop-by-hop' is ignored , and replaced by 'end-to-end') */
        testMirrorMode      = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

        /* in this mode the 'PRV_TGF_ANALYZER_IDX_CNS' is ignored and so the 'port' */
        /* so we need to set analyzer to get the packet */
        final_analyzerIndex = PRV_TGF_INGRESS_PORT_ANALYZER_IDX_CNS;
    }
    else
    {
        testMirrorMode      = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
        final_analyzerIndex = PRV_TGF_ANALYZER_IDX_CNS;
    }


    cpssOsMemSet(&ifaceMir, 0, sizeof(ifaceMir));

    /* AUTODOC: initialize rx port, analyzer port, vlanId and
       PVID (for rx-port) by random values */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    prvTgfRxPortIx = cpssOsRand() % prvTgfPortsNum;

    /* random analyzer port index (different than rx port index)  */
    do
    {
        prvTgfAnPortIx = cpssOsRand() % prvTgfPortsNum;
    }
    while (prvTgfAnPortIx == prvTgfRxPortIx);

    prvTgfTaggedVlanId   = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID -2) + 2;

    /* change PVID of rx port to new random value */
    do
    {
        prvTgfUntaggedVlanId = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID -2) + 2;
    }
    while (prvTgfUntaggedVlanId == prvTgfSavedRxPvid);


    /* AUTODOC: save configuration. */
    prvTgfCscdDsaRxMirrSrcFldsConfigSave();

    /* AUTODOC: initialize eDSA tag some fields that should be filled at run-time (unions). */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum  = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.devPort.portNum = prvTgfPortsArray[prvTgfRxPortIx];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort   = prvTgfPortsArray[prvTgfRxPortIx];

    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_TRUE;
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerTrgPort  = prvTgfPortsArray[prvTgfAnPortIx];
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = prvTgfPortsArray[prvTgfAnPortIx];

    /* get default user priority of rx port. This value will be placed by
       device into an eDSA tag for incoming untagged packets. */
    rc = prvTgfCosPortDefaultUpGet(prvTgfDevNum,
                                   prvTgfPortsArray[prvTgfRxPortIx],
                                   &prvTgfRxDefaultUp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultUpGet");


    /* setup User Priority fields inside expected eDSA tag and packet's vlan tag
       to be equal default UP of rx port */
    prvTgfDsaTag.commonParams.vpt = prvTgfRxDefaultUp;
    prvTgfPacketVlanTag0Part.pri            = prvTgfRxDefaultUp;

    /* bind the DSA tag FORWARD part at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;

    /* AUTODOC: create vlan and place the rx-port in this vlan tagged */
    tag = 1;
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfTaggedVlanId,
                                           &prvTgfPortsArray[prvTgfRxPortIx],
                                           NULL, &tag, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgDefVlanEntryWithPortsSet %d", prvTgfTaggedVlanId);

    /* AUTODOC: Create vlan with untagged rx port and set rx port's PVID equal
       to this vlan. This will avoid untagged packet's flooding on all ports. */
    tag = 0;
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfUntaggedVlanId,
                                           &prvTgfPortsArray[prvTgfRxPortIx],
                                           NULL, &tag, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                  "prvTgfBrgDefVlanEntryWithPortsSet %d", prvTgfUntaggedVlanId);
    /* setup new rx port's PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[prvTgfRxPortIx],
                                 prvTgfUntaggedVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: setup analyzer port. Use hop-by-hop forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testMirrorMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* setup Rx analyzer index */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
                                                      final_analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet");

    /* assign analyzer port to rx-analyzer index */
    ifaceMir.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMir.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceMir.interface.devPort.portNum  = prvTgfPortsArray[prvTgfAnPortIx];

    rc = prvTgfMirrorAnalyzerInterfaceSet(final_analyzerIndex, &ifaceMir);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                 ifaceMir.interface.devPort.portNum);

    /* AUTODOC: setup rx port to be mirrored */

    /*3rd parameter is ignored in hop-by-hop forwarding mode */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[prvTgfRxPortIx], GT_TRUE, PRV_TGF_INGRESS_PORT_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTODOC: setup analyzer port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
}

/**
* @internal prvTgfCscdDsaRxMirrSrcFldsConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsConfigRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable packet capturing on analyzer port */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   =  prvTgfPortsArray[prvTgfAnPortIx];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                 iface.devPort.portNum);

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");


    /* AUTODOC: invalidate used vlan entries */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfTaggedVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                     "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfTaggedVlanId);

    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfUntaggedVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                   "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfUntaggedVlanId);


    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    /* AUTODOC: restore forwaringd mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore global Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(
               prvTgfSavedGlobalMirroringEn, prvTgfSavedGlobalAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore an old Analyzer bound to final_analyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceSet(final_analyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of rx port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[prvTgfRxPortIx],
                               prvTgfSavedRxMirrorModeEnabled,
                               prvTgfSavedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a cascade configuration of the Analyzer port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               prvTgfSavedAnCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a PVID of rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[prvTgfRxPortIx],
                                 prvTgfSavedRxPvid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaRxMirrSrcFldsSendPacket function
* @endinternal
*
* @brief   Send the packet
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsSendPacket
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc;

    /* reset ethernet counters */
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

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[prvTgfRxPortIx]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
}

/**
* @internal prvTgfCscdDsaRxMirrSrcFldsCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isExpected               - whether the packet should be got on rx port.
* @param[in] isTagged                 - whether the packet has vlan tag(s).
* @param[in] isSentWithDsa            - whether the packet setn with DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received with DSA-tag.
*                                       None
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsCheckOnPorts
(
    IN GT_BOOL isExpected,
    IN GT_BOOL isTagged,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    CPSS_INTERFACE_INFO_STC iface          = {0};
    TGF_PACKET_STC          *packetInfoPtr = NULL;
    GT_U32                  expRx;
    GT_U32                  expTx;
    GT_U32                  packetSize;
    GT_U32                  sizesArr[2];
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;

    if (GT_TRUE == isTagged)
    {
        sizesArr[0] = PRV_TGF_NETW_TAG_PACKET_SIZE_CNS;
        sizesArr[1] = PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS;
    }
    else
    {
        sizesArr[0] = PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS;
        sizesArr[1] = PRV_TGF_CSCD_UNTAG_PACKET_SIZE_CNS;
    }

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        if (prvTgfRxPortIx == i)
        {
            packetSize = sizesArr[GT_TRUE == isSentWithDsa ? 1 : 0];
            expRx = expTx = 1;
        }
        else if (prvTgfAnPortIx == i)
        {
            packetSize = sizesArr[GT_TRUE == isExpWithDsa ? 1 : 0];
            expRx = expTx = GT_TRUE == isExpected ? 1 : 0;
        }
        else
        {
            expRx = expTx = 0;
            packetSize = 0;
        }

        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRx, expTx, packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRx, expTx);
    }

    if (GT_FALSE == isExpected)
    {
        return;
    };

    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Check trigger counters */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfAnPortIx];

    /* AUTODOC: check the captured packet */
    if (GT_TRUE == isExpWithDsa)
    {
        packetInfoPtr = (GT_TRUE == isTagged) ?
            &prvTgfCscdTaggedPacketInfo :
            &prvTgfCscdUntaggedPacketInfo;
    }
    else
    {
        packetInfoPtr = (GT_TRUE == isTagged) ?
            &prvTgfNetwTaggedPacketInfo :
            &prvTgfNetwUntaggedPacketInfo;
    }

    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
        &iface,
        packetInfoPtr,
        prvTgfBurstCount,/*numOfPackets*/
        0/* vfdNum */,
        NULL /* vfdArray */,
        NULL, /* bytesNum's skip list */
        0,    /* length of skip list */
        &actualNumOfPackets,
        NULL/* onFirstPacketNumTriggersBmpPtr */);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorPortTxEthCaptureCompare:"
                        " port = %d, rc = 0x%02X\n", iface.devPort.portNum, rc);
}


/**
* @internal prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck function
* @endinternal
*
* @brief   Send two packets (vlan tagged/untagged)
*
* @param[in] isExpected               - packet should (GT_TRUE) or shouldn't (GT_FALSE)
*                                      be received
* @param[in] isSentWithDsa            - packet is sent with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
* @param[in] isExpWithDsa             - packet is expected with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
*                                       None
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck
(
    IN GT_BOOL isExpected,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    GT_STATUS rc;

    /* AUTODOC: send a vlan-tagged packet (with and without eDSA tag) */

    prvTgfDsaTag.commonParams.vid = prvTgfPacketVlanTag0Part.vid = prvTgfTaggedVlanId;
    prvTgfDsaTag.dsaInfo.toAnalyzer.isTagged = GT_TRUE;

    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                   (GT_TRUE == isSentWithDsa ? &prvTgfCscdTaggedPacketInfo
                                             : &prvTgfNetwTaggedPacketInfo),
                   prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("\nSend tagged packet\n\n");

    prvTgfCscdDsaRxMirrSrcFldsSendPacket();

    prvTgfCscdDsaRxMirrSrcFldsCheckOnPorts(isExpected, GT_TRUE,
                                           isSentWithDsa, isExpWithDsa);

    if (GT_FALSE == isExpected)
    {
        /* one packet is enough. */
        return;
    }

    /* AUTODOC: send an untagged packet (with and without eDSA tag) */

    prvTgfDsaTag.commonParams.vid = prvTgfPacketVlanTag0Part.vid = prvTgfUntaggedVlanId;
    prvTgfDsaTag.dsaInfo.toAnalyzer.isTagged = GT_FALSE;

    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                   (GT_TRUE == isSentWithDsa ? &prvTgfCscdUntaggedPacketInfo
                                             : &prvTgfNetwUntaggedPacketInfo),
                   prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("\nSend untagged packet\n\n");

    prvTgfCscdDsaRxMirrSrcFldsSendPacket();

    prvTgfCscdDsaRxMirrSrcFldsCheckOnPorts(isExpected, GT_FALSE,
                                           isSentWithDsa, isExpWithDsa);
}


/**
* @internal prvTgfCscdDsaRxMirrSrcFldsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfCscdDsaRxMirrSrcFldsTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS               rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: generate traffic */

    /* AUTODOC: enable packet capturing on analyzer port. */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfAnPortIx];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                               TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************************************************************************/
    /* AUTODOC: CASE 1. From network port to cascade port */
    PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck(GT_TRUE, GT_FALSE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 2. Send a packet from cascade port to cascade port */

    PRV_UTF_LOG0_MAC("\nCASE 2. From a cascade port to a cascade port\n");

    /* AUTODOC: set sniffed device number inside eDSA to be different than
       local device number */
    prvTgfDsaTag.dsaInfo.toAnalyzer.devPort.hwDevNum =
                       (0 == prvTgfSavedHwDevNum) ? 1 : prvTgfSavedHwDevNum - 1;

    /* AUTODOC: setup rx port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck(GT_TRUE, GT_TRUE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 3 Send a packet from cascade port to network port */

    PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");

    /* AUTODOC: setup analyzer port to be a network port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck(GT_TRUE, GT_TRUE, GT_FALSE);

    /**************************************************************************/
    /* AUTODOC: CASE 4 Send a packet from cascade port to network port.
       Setup sniffed devNum inside eDSA-tag the same as local device number */

    PRV_UTF_LOG0_MAC("\nCASE 4. From a cascade port to a network port. "
                     "A source devNum inside eDSA-tag is the same as local"
                     " devNum.\nPacket should be filtered.\n");

    prvTgfDsaTag.dsaInfo.toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;

    /* Bobcat2 B0 has erratum (Jira TTI-1166) and own device filterring does
       not work for TO_ANALYZER DSA packets.
       Bobcat2 A0 works as expected. */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: send one packet. It should be filtered. */
        prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck(GT_FALSE, GT_TRUE, GT_FALSE);
    }
    else
    {
        /* AUTODOC: send two packets (vlan tagged and untagged) */
        prvTgfCscdDsaRxMirrSrcFldsSendTwoAndCheck(GT_TRUE, GT_TRUE, GT_FALSE);
    }
}

/************************** PUBLIC DECLARATIONS *******************************/

/**
* @internal prvTgfCscdDsaRxMirrSrcFldsTest function
* @endinternal
*
* @brief   Check correctness of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaRxMirrSrcFldsTest
(
    GT_VOID
)
{
    prvTgfCscdDsaRxMirrSrcFldsConfigSet();
    prvTgfCscdDsaRxMirrSrcFldsTrafficGenerate();
    prvTgfCscdDsaRxMirrSrcFldsConfigRestore();
}

