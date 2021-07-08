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
* @file prvTgfCscdDsaToAnalyzerVidx.c
*
* @brief Analyzer Use eVIDX, Analyzer eVidx fields in To Analyzer eDsa tag test
* advanced UT.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfMirror.h>
#include <cscd/prvTgfCscdDsaToAnalyzerVidx.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS        5

/* traffic generation sending port */
#define ING_NETWORK_PORT_IDX_CNS    0
#define ING_CASCADE_PORT_IDX_CNS    1

/* target port */
#define EGR_NETWORK_PORT_IDX_CNS    3
#define EGR_CASCADE_PORT_IDX_CNS    2

/* analzer index used by the test */
#define PRV_TGF_ANALYZER_IDX_CNS 2

/* max vidx value. Used for random generation of vidx */
#define PRV_TGF_MAX_VIDX_CNS      PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(prvTgfDevNum)

/* ePort */
#define PRV_TGF_EPORT_CNS          (IS_AC3X_DEV_MAC(prvTgfDevNum)?768:5678)

/* target device */
#define PRV_TGF_TRG_DEV_CNS         2

/* Vidx field. Will be initialized later */
static GT_U16 prvTgfVidx;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* the number of vidx group members (two first indexes) */
static GT_U8  prvTgfMcMembersNum  = 1;

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/********************** eDSA tag  ***************************/
static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfDsaInfo_toAnalyzer = {
    GT_TRUE,/* rxSniffer - ingress mirroring */
    GT_FALSE,/* isTagged */

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

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_TO_ANALYZER_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        0,/*vid*/
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

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfCascasePacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_CASCADE_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS  (PRV_TGF_CASCADE_PACKET_LEN_CNS \
                                                          + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
        PRV_TGF_CASCADE_PACKET_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfCascasePacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascasePacketPartArray                                       /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static GT_U32                                         prvTgfSavedGlobalAnalyzerIndex;
static GT_BOOL                                        prvTgfSavedRxMirrorModeEnabled[2];
static GT_U32                                         prvTgfSavedRxMirrorAnalyzerIdx[2];
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static CPSS_INTERFACE_INFO_STC                        savePhyInfo;
static GT_U8                                          prvTgfRxDefaultUp;
static GT_U16                                         prvTgfDefVlanId[2];
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCscdLinkType;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal setEgressPortCascade function
* @endinternal
*
* @brief   set/unset the egress port as cascade with DSA tag .
*
* @param[in] egressPort               - egress port to be DSA tagged
* @param[in] enable                   - enable/disable the cascade port.
*                                       None
*/
static GT_VOID setEgressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     egressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        prvTgfDevNum, egressPort, enable,
        enable ? CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");
}
/**
* @internal setIngressPortCascade function
* @endinternal
*
* @brief   set/unset the ingress port as cascade with DSA tag .
*/
static GT_VOID setIngressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     ingressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the ingress port to be DSA tagged */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
                                             prvTgfDevNum, ingressPort, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorIngressCscdPortEnableSet: [%d] \n");
}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxRedirectMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
* @param[in] savePhyInfoPtr           -  (pointer to) save inteface
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCscdDsaToAnalyzerVidxRedirectMapEport2PhySet
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
* @internal setAnalyzerMode function
* @endinternal
*
* @brief   switch between Hop-by-hop and End-to-end modes.
*
* @param[in] mode                     - analyzer  select
*                                      GT_FALSE - Hop-by-hop forwarding mode
*                                      GT_TRUE  - End-to-end forwarding mode
* @param[in] portIdx                  - Rx mirrored port index
*                                       None
*/
static GT_VOID setAnalyzerMode
(
    IN GT_BOOL                  mode,
    IN GT_U32                   portIdx
)
{
    GT_STATUS                   rc;

    if (GT_TRUE == mode)
    {
        /* AUTODOC: setup analyzer port. Use end-to-end forwarding mode */
        rc = prvTgfMirrorToAnalyzerForwardingModeSet(
                            PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "prvTgfMirrorToAnalyzerForwardingModeSet");
        /* setup Rx analyzer index */
        rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_FALSE,
                                                      PRV_TGF_ANALYZER_IDX_CNS);
        /*3rd parameter is used in end-to-end forwarding mode */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[portIdx], GT_TRUE,
                                                      PRV_TGF_ANALYZER_IDX_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");
    }
    else
    {
        /* AUTODOC: setup analyzer port. Use hop-by-hop forwarding mode */
        rc = prvTgfMirrorToAnalyzerForwardingModeSet(
                            PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "prvTgfMirrorToAnalyzerForwardingModeSet");
        /* setup Rx analyzer index */
        rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
                                                      PRV_TGF_ANALYZER_IDX_CNS);

        /*3rd parameter is ignored in hop-by-hop forwarding mode */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[portIdx], GT_TRUE, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");
    }

}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxTestPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] ingressPortIndex         - ingress port index
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVidxTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_U32                   ingressPortIndex
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /*this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the
      ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_PCL_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_TRUE);

    /* AUTODOC: send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[ingressPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ingressPortIndex]);

    /* AUTODOC: wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 100, NULL);

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_PCL_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* AUTODOC: restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] egressPortIndex          - egress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;
    GT_U32                          actualNumOfPackets;

    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    prvTgfDsaInfo_toAnalyzer.devPort.portNum         = prvTgfPortsArray[ingressPortIndex];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort           = prvTgfDsaInfo_toAnalyzer.devPort.portNum;

    /* bind the DSA tag TO_ANALYZER part */
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;

    /* AUTODOC: send packet. */
    prvTgfCscdDsaToAnalyzerVidxTestPacketSend(&portInterface, packetInfoPtr, ingressPortIndex);

    /* AUTODOC: check counters */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (egressPortIndex == portIter)
        {
            /* AUTODOC: check the captured packet */
            packetInfoPtr = (EGR_CASCADE_PORT_IDX_CNS == egressPortIndex) ?
                                 &prvTgfCascadePacketInfo : &prvTgfPacketInfo;

            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                                          &portInterface,
                                          packetInfoPtr,
                                          prvTgfBurstCount,/*numOfPackets*/
                                          0/* vfdNum */,
                                          NULL /* vfdArray */,
                                          NULL, /* bytesNum's skip list */
                                          0,    /* length of skip list */
                                          &actualNumOfPackets,
                                          NULL/* onFirstPacketNumTriggersBmpPtr */);
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                         " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, actualNumOfPackets,
                       "Triggers number: port #%d, expected - %d, received - %d\n",
                       portInterface.devPort.portNum, prvTgfBurstCount, actualNumOfPackets);
        }
        else if (ingressPortIndex == portIter)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                        "Tx another goodPktsSent counter %d, than expected %d, on port %d",
                                         portCntrs.goodPktsSent.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                        "Tx another goodPktsSent counter %d, than expected %d, on port %d",
                                         portCntrs.goodPktsSent.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter]);
        }
    }

}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaToAnalyzerVidxTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                             rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceMir;
    CPSS_INTERFACE_INFO_STC               physicalInfo;
    GT_U32                                 portsArray[4];
    GT_U8                                 tagArray[] = {0,0,0,0};
    CPSS_CSCD_LINK_TYPE_STC               cascadeLink  = {0};

    cpssOsMemSet(&ifaceMir, 0, sizeof(ifaceMir));
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   ING_CASCADE_PORT_IDX_CNS,
                                   EGR_CASCADE_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* save the current cascade map table entry */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");

    /* save Hardware device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet( prvTgfDevNum,
                                  prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                  &prvTgfDefVlanId[ING_NETWORK_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet( prvTgfDevNum,
                                  prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                                  &prvTgfDefVlanId[ING_CASCADE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: save forwarding mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeGet");

    /* AUTODOC: save global Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfSavedGlobalAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to index PRV_TGF_ANALYZER_IDX_CNS  */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          PRV_TGF_ANALYZER_IDX_CNS,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of rx port 0 */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                     &prvTgfSavedRxMirrorModeEnabled[ING_NETWORK_PORT_IDX_CNS],
                     &prvTgfSavedRxMirrorAnalyzerIdx[ING_NETWORK_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet error");

    /* AUTODOC: save a mirroring status of rx port 1 */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                     &prvTgfSavedRxMirrorModeEnabled[ING_CASCADE_PORT_IDX_CNS],
                     &prvTgfSavedRxMirrorAnalyzerIdx[ING_CASCADE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet error");

    /* get default user priority of rx port. This value will be placed by
       device into an eDSA tag for incoming untagged packets. */
    rc = prvTgfCosPortDefaultUpGet(prvTgfDevNum,
                                   prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                   &prvTgfRxDefaultUp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultUpGet");

    /* AUTODOC: set link up interface map as the cascade trunk with trunk number 0 for egress filtering */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    cascadeLink.linkNum  = 0;

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: initialize vidx by random values */
    prvTgfVidx  = (GT_U16) (cpssOsRand() % (PRV_TGF_MAX_VIDX_CNS - 1) + 1);

    PRV_UTF_LOG1_MAC("We will use vidx = %d ", prvTgfVidx);

    portsArray[0] = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    portsArray[1] = prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS];
    portsArray[2] = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    portsArray[3] = prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: create VLAN with untagged port [0] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, portsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                                            PRV_TGF_VLANID_CNS);

    /* setup new rx port's PVID for port [0] */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* setup new rx port's PVID for port [1]*/
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_TRUE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_TRUE );

    /* assign analyzer port to rx-analyzer index */
    ifaceMir.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceMir.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceMir.interface.devPort.portNum  = PRV_TGF_EPORT_CNS;

    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS, &ifaceMir);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet error %d",
                                 ifaceMir.interface.devPort.portNum);

    /* AUTODOC: map ePort to vidx */
    physicalInfo.type = CPSS_INTERFACE_VIDX_E;
    physicalInfo.vidx = prvTgfVidx;

    prvTgfCscdDsaToAnalyzerVidxRedirectMapEport2PhySet( prvTgfDevNum, PRV_TGF_EPORT_CNS,
                                                   &physicalInfo, &savePhyInfo);

    /* AUTODOC: create VIDX with port [2] */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, prvTgfPortsArray + 2, NULL,
                                                            prvTgfMcMembersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                                      prvTgfDevNum, prvTgfVidx);


    /* AUTODOC: add FDB entry to avoid flooding */
    prvTgfBrgDefFdbMacEntryOnPortSet( prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
         PRV_TGF_TRG_DEV_CNS, prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_TRUE);

    prvTgfPacketDsaTagPart.commonParams.vpt          = prvTgfRxDefaultUp;
    prvTgfPacketDsaTagPart.commonParams.vid          = PRV_TGF_VLANID_CNS;
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum        = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.extDestInfo.multiDest.analyzerEvidx = prvTgfVidx;
    prvTgfDsaInfo_toAnalyzer.analyzerTrgType         = CPSS_INTERFACE_VIDX_E;
}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaToAnalyzerVidxTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore map ePort to physical port */
    prvTgfCscdDsaToAnalyzerVidxRedirectMapEport2PhySet( prvTgfDevNum,
                                         PRV_TGF_EPORT_CNS, &savePhyInfo, NULL);

    /* AUTODOC: restore forwarding mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore global Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(
               prvTgfSavedGlobalMirroringEn, prvTgfSavedGlobalAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore an old Analyzer bound to PRV_TGF_ANALYZER_IDX_CNS */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_IDX_CNS,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of rx port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                      prvTgfSavedRxMirrorModeEnabled[ING_NETWORK_PORT_IDX_CNS],
                      prvTgfSavedRxMirrorAnalyzerIdx[ING_NETWORK_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTODOC: restore a mirroring status of rx port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                      prvTgfSavedRxMirrorModeEnabled[ING_CASCADE_PORT_IDX_CNS],
                      prvTgfSavedRxMirrorAnalyzerIdx[ING_CASCADE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    setIngressPortCascade(prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_FALSE);
    setEgressPortCascade( prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: invalidate VIDX */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, NULL, NULL, 0);

    /* restore PVID for port [0] */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                 prvTgfDefVlanId[ING_NETWORK_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* restore PVID for port [1]*/
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                                 prvTgfDefVlanId[ING_CASCADE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore the cascade map table's entry */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_TRG_DEV_CNS,
                                  0,
                                  &prvTgfSavedCscdLinkType,
                                  prvTgfSavedSrcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaToAnalyzerVidxTest function
* @endinternal
*
* @brief   Test for Analyzer Use eVIDX, Analyzer eVidx fields in To Analyzer
*         eDsa tag
*/
GT_VOID prvTgfCscdDsaToAnalyzerVidxTest
(
    GT_VOID
)
{
    GT_STATUS               rc;

    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaToAnalyzerVidxTestConfigurationSet();

    /* AUTODOC: CASE 1: from Network port to Cascade port */
    PRV_UTF_LOG2_MAC(
        "\n*** CASE 1: from Network port %02d to Cascade port %02d ***\n",
        prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
        prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Set Hop-by-hop analyzer mode */
        setAnalyzerMode(GT_FALSE, ING_NETWORK_PORT_IDX_CNS);

        /* AUTODOC: Send registered UC packet*/
        prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                    EGR_CASCADE_PORT_IDX_CNS,
                                                    &prvTgfPacketInfo);
    }

    /* AUTODOC: Set End-to-end analyzer mode */
    setAnalyzerMode(GT_TRUE, ING_NETWORK_PORT_IDX_CNS);

    /* AUTODOC: Send registered UC packet*/
    prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfPacketInfo);

    /* AUTODOC: CASE 2: from Cascade port to Cascade port */
    PRV_UTF_LOG2_MAC(
        "\n*** CASE 2: from Cascade port %02d to Cascade port %02d ***\n",
        prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
        prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* set Hw device number in the eDSA tag to be different from srcHwDev  */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum  = prvTgfSavedHwDevNum + 1;

    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Set Hop-by-hop analyzer mode */
        setAnalyzerMode(GT_FALSE, ING_CASCADE_PORT_IDX_CNS);

        /* AUTODOC: Send  registered UC packet*/
        prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                    EGR_CASCADE_PORT_IDX_CNS,
                                                    &prvTgfCascadePacketInfo);
    }

    /* AUTODOC: Set End-to-end analyzer mode */
    setAnalyzerMode(GT_TRUE, ING_CASCADE_PORT_IDX_CNS);

    /* AUTODOC: Send registered UC packet*/
    prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo);

    /* AUTODOC: CASE 3: from Cascade port to Network port */
    PRV_UTF_LOG2_MAC(
        "\n*** CASE 3: from Cascade port %02d to Network port %02d ***\n",
        prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
        prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: invalidate VIDX */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, NULL, NULL, 0);

    /* AUTODOC: create VIDX with port [2] */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, prvTgfPortsArray + 3, NULL,
                               prvTgfMcMembersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, prvTgfVidx);

    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Set Hop-by-hop analyzer mode */
        setAnalyzerMode(GT_FALSE, ING_CASCADE_PORT_IDX_CNS);

        /* AUTODOC: Send  registered UC packet*/
        prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                    EGR_NETWORK_PORT_IDX_CNS,
                                                    &prvTgfCascadePacketInfo);
    }

    /* AUTODOC: Set End-to-end analyzer mode */
    setAnalyzerMode(GT_TRUE, ING_CASCADE_PORT_IDX_CNS);

    /* AUTODOC: Send  registered UC packet*/
    prvTgfCscdDsaToAnalyzerVidxTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_NETWORK_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo);

    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum  = prvTgfSavedHwDevNum;

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaToAnalyzerVidxTestConfigurationRestore();
}

