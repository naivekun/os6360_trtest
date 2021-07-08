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
* @file prvTgfCscdDsaTxMirrSrcFlds.c
*
* @brief Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
* <SrcTrg Tagged> of a tx-mirrored packet. There are 3 cases. In every
* case we check two packets: vlan-tagged and vlan-untagged.
* 1) Configure analyzer port to be a cascade port. Send a packets to
* a network rx port. Packets will be fowrarded to remote device
* (via FDB entry, a Device Map table and a cascade tx-port). Both
* packets are vlan-tagged. Whether to remove tag will decide
* tx port.
* And will be tx-mirrored to Analyzer port.
* 2) Snd a DSA-tagged TO_ANALYZER packets to a cascade rx port.
* Packet should be received on the cascade analyzer port the same
* as it come to rx-port.
* 3) Setup an analyzer port to be a network port and send a DSA-tagged
* TO_ANALYZER packets on cascade rx port. Packets should be received
* on the analyzer port the same as they were sent on rx port
* in case 1.
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
#include <cscd/prvTgfCscdDsaTxMirrSrcFlds.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max vlan id value. Used for random generation of vlan id */
#define PRV_TGF_MAX_VLAN_ID      _4K

#define PRV_TGF_PAYLOAD_LEN_CNS   0x30 /* 48 */

/* a remote device's number. */
#define PRV_TGF_REMOTE_DEV_CNS    15

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
    GT_FALSE,/*rxSniffer - ingress mirroring */
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
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))

/* size of the tagged packet that came from cascade port */
#define PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_eDSA_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdTaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
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
static TGF_VLAN_ID prvTgfVlanId;         /* vlan id */
static GT_U32      prvTgfRxPortIx;       /* rx port index */
static GT_U32      prvTgfTxPortIx;       /* tx port index */
static GT_U32      prvTgfAnPortIx;       /* analyzer port index*/
static GT_U8       prvTgfRxDefaultUp;      /* default user priority of rx port */
static GT_U32      prvTgfGlobalTxAnalazyerIndex;

/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedTxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedTxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedRxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedTxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedAnCscdPortType;
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCascadeLink;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT              prvTgfSavedTxVlanTagState;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

/* egress port analyzer index used by the test (lower than 'global' index (prvTgfGlobalTxAnalazyerIndex) */
#define PRV_TGF_EGRESS_PORT_ANALYZER_IDX_CNS 0
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   testMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
static GT_U32   final_analyzerIndex = 0;/*set in runtime*/


/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/*************************** PRIVATE DECLARATIONS *****************************/
/**
* @internal prvTgfCscdDsaTxMirrSrcFldsConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsConfigSave
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
                                   prvTgfTxPortIx,
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

    /* AUTODOC: save global tx Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfGlobalTxAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to prvTgfGlobalTxAnalazyerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          prvTgfGlobalTxAnalazyerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of tx port */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[prvTgfTxPortIx],
                               &prvTgfSavedTxMirrorModeEnabled,
                               &prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortGet error");

    /* AUTODOC: save a cascade configuration of the rx-port port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               CPSS_PORT_DIRECTION_RX_E,
                               &prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* AUTODOC: save a cascade configuration of the tx port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[prvTgfTxPortIx],
                               CPSS_PORT_DIRECTION_TX_E,
                               &prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* AUTODOC: save a cascade configuration of the analyzer port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_PORT_DIRECTION_TX_E,
                               &prvTgfSavedAnCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

 /* AUTODOC: save the current cascade map table entry */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                                  prvTgfPortsArray[prvTgfAnPortIx],
                                  &prvTgfSavedCascadeLink,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");

    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            prvTgfPortsArray[prvTgfTxPortIx],
                                            &prvTgfSavedTxVlanTagState);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateGet");

}

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsInitRandomValues function
* @endinternal
*
* @brief   Initialize used ports and vlan by random values.
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsInitRandomValues
(
    GT_VOID
)
{
    /* AUTODOC: initialize rx port, tx port, analyzer port, vlanId and
       PVID (for rx-port) by random values */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* random rx port index */
    prvTgfRxPortIx = cpssOsRand() % prvTgfPortsNum;

    /* random tx port index */
    do
    {
        prvTgfTxPortIx = cpssOsRand() % prvTgfPortsNum;
    }
    while (prvTgfTxPortIx == prvTgfRxPortIx);

    /* random analyzer port index */
    do
    {
        prvTgfAnPortIx = cpssOsRand() % prvTgfPortsNum;
    }
    while (prvTgfAnPortIx == prvTgfRxPortIx || prvTgfAnPortIx == prvTgfTxPortIx);

    /* random vlan for tagged packets */
    prvTgfVlanId = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;

    PRV_UTF_LOG0_MAC("\n\nRandomized values:\n");
    PRV_UTF_LOG1_MAC("Rx port: %d\n", prvTgfPortsArray[prvTgfRxPortIx]);
    PRV_UTF_LOG1_MAC("Tx port: %d\n", prvTgfPortsArray[prvTgfTxPortIx]);
    PRV_UTF_LOG1_MAC("Analyzer port: %d\n",  prvTgfPortsArray[prvTgfAnPortIx]);
    PRV_UTF_LOG1_MAC("vlan id: %d\n\n", prvTgfVlanId);
}

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsConfigSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceMir;
    CPSS_CSCD_LINK_TYPE_STC               cascadeLink;
    PRV_TGF_BRG_MAC_ENTRY_STC             macEntry;
    GT_STATUS                             rc;

    cpssOsMemSet(&ifaceMir,    0, sizeof(ifaceMir));
    cpssOsMemSet(&cascadeLink, 0, sizeof(cascadeLink));
    cpssOsMemSet(&macEntry,    0, sizeof(macEntry));

    prvTgfCscdDsaTxMirrSrcFldsInitRandomValues();

    prvTgfCscdDsaTxMirrSrcFldsConfigSave();

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* we run the test with 'end-to-end' !!!
        (the 'hop-by-hop' is ignored , and replaced by 'end-to-end') */
        testMirrorMode      = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

        /* in this mode the 'PRV_TGF_ANALYZER_IDX_CNS' is ignored and so the 'port' */
        /* so we need to set analyzer to get the packet */
        final_analyzerIndex = PRV_TGF_EGRESS_PORT_ANALYZER_IDX_CNS;
    }
    else
    {
        /* prvTgfGlobalTxAnalazyerIndex is known after calling :
            prvTgfCscdDsaTxMirrSrcFldsConfigSave() */
        testMirrorMode      = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
        final_analyzerIndex = prvTgfGlobalTxAnalazyerIndex;
    }

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */
    /* sniffer port's attributes */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.devPort.portNum  = prvTgfPortsArray[prvTgfTxPortIx];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort    = prvTgfPortsArray[prvTgfTxPortIx];

    /* analyzer port's attributes */
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
    prvTgfPacketVlanTag0Part.pri  = prvTgfRxDefaultUp;

    /* set a packet's vlan id */
    prvTgfDsaTag.commonParams.vid = prvTgfPacketVlanTag0Part.vid = prvTgfVlanId;

    /* bind the DSA tag ANALYZER part at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;

    /* AUTODOC: Create vlan with all ports with tagging command =
     PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E*/
    rc =   prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfVlanId,
                                 PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: setup an rx analyzer port. Use hop-by-hop forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testMirrorMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* assign the analyzer port to tx analyzer index */
    ifaceMir.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMir.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceMir.interface.devPort.portNum  = prvTgfPortsArray[prvTgfAnPortIx];

    rc = prvTgfMirrorAnalyzerInterfaceSet(final_analyzerIndex,
                                          &ifaceMir);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                 ifaceMir.interface.devPort.portNum);

    /* AUTODOC: setup tx port to be mirrored */

    /*3rd parameter is ignored in hop-by-hop forwarding mode */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx], GT_TRUE, final_analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

    /* AUTODOC: setup tx port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfTxPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet",
                                 prvTgfPortsArray[prvTgfTxPortIx]);

    /* AUTODOC: setup analyzer port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet",
                                 prvTgfPortsArray[prvTgfAnPortIx]);

    /* AUTODOC: assign remote device with local tx port (device map table) */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[prvTgfTxPortIx];

    prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                             prvTgfPortsArray[prvTgfAnPortIx],
                             &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /*------------------------CREATE AN FDB ENTRIES---------------------------*/

    /* AUTODOC: set an FDB entry to send vlan's traffic to remote device  */
    macEntry.dstInterface.type             = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = PRV_TGF_REMOTE_DEV_CNS;
    macEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[prvTgfAnPortIx];
    macEntry.key.key.macVlan.vlanId        = prvTgfVlanId;
    macEntry.key.entryType                 = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                      = GT_TRUE;
    macEntry.saCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d",
                                 prvTgfVlanId);
}

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");


    /* AUTODOC: invalidate used vlan entries */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                     "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfVlanId);

    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    /* AUTODOC: restore forwaringd mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore global tx Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(
               prvTgfSavedGlobalMirroringEn, prvTgfGlobalTxAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore an old Analyzer bound to prvTgfGlobalTxAnalazyerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfGlobalTxAnalazyerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of tx port */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx],
                               prvTgfSavedTxMirrorModeEnabled,
                               prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a cascade configuration of the tx port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfTxPortIx],
                               prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a cascade configuration of the Analyzer port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               prvTgfSavedAnCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore the cascade map table's entry */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_REMOTE_DEV_CNS,
                                  prvTgfPortsArray[prvTgfAnPortIx],
                                  &prvTgfSavedCascadeLink,
                                  prvTgfSavedSrcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* restore tx port's vlan tag state */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            prvTgfPortsArray[prvTgfTxPortIx],
                                            prvTgfSavedTxVlanTagState);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray,
                 sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsSendPacket function
* @endinternal
*
* @brief   Send the packet
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsSendPacket
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_U32    portIter;
    GT_STATUS rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

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

    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfAnPortIx];

    /* enable packet capturing on analyzer port. */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                               TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[prvTgfRxPortIx]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    /* disable packet capturing on analyzer port. */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet");

}

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isVlanTagged             - whether the mirrored packet has vlan tag.
* @param[in] isSentWithDsa            - whether the packet was sent with a DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received on the an analyzer
*                                      port with DSA-tag.
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsCheckOnPorts
(
    IN GT_BOOL isVlanTagged,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    CPSS_INTERFACE_INFO_STC iface          = {0};
    TGF_PACKET_STC          *packetInfoPtr = NULL;
    GT_U32                  expRx;
    GT_U32                  expTx;
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;


    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        if (prvTgfRxPortIx == i || prvTgfAnPortIx == i)
        {
            expRx = expTx = 1;
        }
        else if (prvTgfTxPortIx == i)
        {
            /* we expect a packet on tx port only in 1st case, when packet is
             received on the network rx port. */
            expTx = isSentWithDsa ? 0 : 1;
            expRx = 0;
        }
        else
        {
            expRx = expTx = 0;
        }

        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRx, expTx, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRx, expTx);
    }

    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Check trigger counters */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfAnPortIx];

    /* AUTODOC: check the captured packet */
    if (GT_TRUE == isExpWithDsa)
    {
        packetInfoPtr = (GT_TRUE == isVlanTagged) ?
            &prvTgfCscdTaggedPacketInfo :
            &prvTgfCscdUntaggedPacketInfo;
    }
    else
    {
        packetInfoPtr = (GT_TRUE == isVlanTagged) ?
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
* @internal prvTgfCscdDsaTxMirrSrcFldsSendAndCheck function
* @endinternal
*
* @brief   Send two packets (vlan tagged/untagged)
*
* @param[in] isVlanTagged             -    packet should be received on analyzer port with
*                                      (GT_TRUE) or without (GT_FALSE) vlan tag
* @param[in] isSentWithDsa            - packet is sent with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag to rx port
* @param[in] isExpWithDsa             - packet is expected with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag on the analyzer port
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsSendAndCheck
(
    IN GT_BOOL isVlanTagged,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    TGF_PACKET_STC *packet;
    GT_STATUS rc;

    prvTgfDsaTag.dsaInfo.toAnalyzer.isTagged = isVlanTagged;

    if (GT_TRUE == isVlanTagged)
    {
        PRV_UTF_LOG0_MAC("\nA packet should be egress mirrored with a vlan tag.\n\n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("\nA packet should be egress mirrored without a vlan tag.\n\n");
    }

    /* prepare the packet to be sent */
    if (GT_TRUE == isSentWithDsa)
    {
        packet = (GT_TRUE == isVlanTagged) ?
            &prvTgfCscdTaggedPacketInfo :
            &prvTgfCscdUntaggedPacketInfo;
    }
    else
    {
        packet = &prvTgfNetwTaggedPacketInfo;
    }
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packet, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    prvTgfCscdDsaTxMirrSrcFldsSendPacket();

    prvTgfCscdDsaTxMirrSrcFldsCheckOnPorts(isVlanTagged,
                                           isSentWithDsa, isExpWithDsa);
}


/**
* @internal prvTgfCscdDsaTxMirrSrcFldsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfCscdDsaTxMirrSrcFldsTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;

    /**************************************************************************/
    /* AUTODOC: CASE 1. From network port to cascade port */
    PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_TRUE,  GT_FALSE, GT_TRUE);

    /* configure vlan's taggind command to remove the tag from the packet.*/
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfVlanId,
                                          PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_FALSE, GT_FALSE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 2. Send a packet from cascade port to cascade port */

    PRV_UTF_LOG0_MAC("\nCASE 2. From a cascade port to a cascade port\n");

    /* AUTODOC: set sniffed device to be different than local device number */
    prvTgfDsaTag.dsaInfo.toAnalyzer.devPort.hwDevNum =
                       (0 == prvTgfSavedHwDevNum) ? 1 : prvTgfSavedHwDevNum - 1;

    /* AUTODOC: setup rx port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[prvTgfRxPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_TRUE,  GT_TRUE, GT_TRUE);
    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_FALSE, GT_TRUE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 3 Send a packet from cascade port to network port */

    PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");

    /* AUTODOC: setup analyzer port to be a network port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: send two packets (vlan tagged and untagged) */
    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_TRUE,  GT_TRUE, GT_FALSE);
    prvTgfCscdDsaTxMirrSrcFldsSendAndCheck(GT_FALSE, GT_TRUE, GT_FALSE);
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaTxMirrSrcFldsTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrSrcFldsTest
(
    GT_VOID
)
{
    prvTgfCscdDsaTxMirrSrcFldsConfigSet();
    prvTgfCscdDsaTxMirrSrcFldsTrafficGenerate();
    prvTgfCscdDsaTxMirrSrcFldsConfigRestore();
}

