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
* @file prvTgfCscdDsaFrwdSrcId.c
*
* @brief Test a source-ID in the eDsa tag (FORWARD command). The packet is sent:
* - from a network port to a cascade port
* - from a cascade port to a cascade port
* - from a cascade port to a network port
* Tx port is checked: it should or shouldn't filter the packet
* depending on the port is added to appropriate source-id group.
*
*
* @version   1
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
#include <common/tgfBridgeGen.h>
#include <cscd/prvTgfCscdDsaFrwdSrcId.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1

/* max vlan id value. Used for random generation of vlan id */
#define PRV_TGF_MAX_VLAN_ID         _4K

/* max source-id value. Used for random generation of source-id */
#define PRV_TGF_MAX_SRC_ID          (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(prvTgfDevNum) + 1)

/* eDSA tag source-ID field. Will be initialized later */
static GT_U32 prvTgfSrcId;

/* vlan used by test. Will be initialized later */
static TGF_VLAN_ID prvTgfVlanId;

/******************************* Test packets *********************************/
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

/* VLAN_TAG1 parts. */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,  /* etherType */
    0, 0, 6 /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[48] ={0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* size of packet that came from network port */
#define PRV_TGF_NETWORK_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + 2 *TGF_VLAN_TAG_SIZE_CNS    \
                                           + sizeof(prvTgfPayloadDataArr))

/* size of packet that came from cascade port */
#define PRV_TGF_CASCADE_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_eDSA_TAG_SIZE_CNS       \
                                           + TGF_VLAN_TAG_SIZE_CNS       \
                                           + sizeof(prvTgfPayloadDataArr))

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E, /*dsaCommand*/
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

static TGF_DSA_DSA_FORWARD_STC  prvTgfDsaInfo_forward = {
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E, /*srcIsTagged*/
    0,/*srcHwDev */
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/
    },/*source;*/

    0,/*srcId */ /* will be filled later */

    GT_FALSE,/*egrFilterRegistered - packet is unregistered UC */
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_VID_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            0/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport */
    0,/*tag0TpidIndex */
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum */
    },/*origSrcPhy*/
    GT_TRUE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/

};

/* parts of packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetworkPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a network packet info */
static TGF_PACKET_STC prvTgfNetworkPacketInfo = {
    PRV_TGF_NETWORK_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfNetworkPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetworkPacketPartsArray                                       /* partsArray */
};

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartsArray                                       /* partsArray */
};


/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some original values saved to be restored at the end of the test. */
CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedRxCscdPortType;
CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedTxCscdPortType;
GT_BOOL                           prvTgfSavedRxSrcIdForceEnable;
GT_BOOL                           prvTgfSavedTxSrcIdForceEnable;
GT_U32                            prvTgfSavedRxDefSrcId;
GT_HW_DEV_NUM                     prvTgfSavedHwDevNum;
static GT_U32                     prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
/* indication that xCat3x use rx NETWORK remote Port (from phy1690)
   sending to 16B DSA cascade port */
static GT_BOOL                    run_xCat3x_rx_from_remotePort = GT_FALSE;
/* indication that xCat3x use tx NETWORK remote Port (from phy1690)
   receiving from 16B DSA cascade port */
static GT_BOOL                    run_xCat3x_tx_to_remotePort = GT_FALSE;
/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/
extern GT_PHYSICAL_PORT_NUM prvTgfTrafficGeneratorPacketDsaTagGetSrcId
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_PHYSICAL_PORT_NUM           cascadePortNum,
    IN  GT_U32                         srcId
);

/************************** PRIVATE DECLARATIONS ****************************/
/**
* @internal prvTgfCscdDsaFrwdSrcIdConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdConfigSave
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
        if(run_xCat3x_rx_from_remotePort == GT_TRUE)
        {
            /*allow the network port to be 'remote port' --> to check SRC-ID erratum */
            rc = prvTgfDefPortsArrange(GT_TRUE,
                                       PRV_TGF_RX_PORT_IDX_CNS,
                                       -1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
        }
        else
        if(run_xCat3x_tx_to_remotePort == GT_TRUE)
        {
            /*allow the network port to be 'remote port' --> to check SRC-ID erratum */
            rc = prvTgfDefPortsArrange(GT_TRUE,
                                       PRV_TGF_TX_PORT_IDX_CNS,
                                       -1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
        }
        else
        {
            rc = prvTgfDefPortsArrange(GT_FALSE,
                                       PRV_TGF_RX_PORT_IDX_CNS,
                                       PRV_TGF_TX_PORT_IDX_CNS,
                                       -1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
        }
    }

    /* save a configuration of a rx-port to be a cascade port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               CPSS_PORT_DIRECTION_RX_E,
                               &prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    if(GT_FALSE == run_xCat3x_tx_to_remotePort)
    {
        /* save a configuration of a tx-port to be a cascade port */
        rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   CPSS_PORT_DIRECTION_TX_E,
                                   &prvTgfSavedTxCscdPortType);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");
    }

    /* save  default source id value of rx port */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      &prvTgfSavedRxDefSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdGet");

    /* save a ForceSrcId state of rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      &prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableGet");
    /* save Hardware device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");
}





/**
* @internal prvTgfCscdDsaFrwdSrcIdCreateVlan function
* @endinternal
*
* @brief   Create a vlan with tagged rx, tx ports. Tagging command
*         is PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E
* @param[in] vlanId                   - vlan id.
*
* @retval GT_OK                    - on success
*/

static GT_VOID prvTgfCscdDsaFrwdSrcIdCreateVlan
(
    IN TGF_VLAN_ID vlanId
)
{
    CPSS_PORTS_BMP_STC                 portsMembers;
    CPSS_PORTS_BMP_STC                 portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC          vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    GT_STATUS                          rc;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));
    cpssOsMemSet(&vlanInfo,        0, sizeof(vlanInfo));

    /* mark the ports to be added into vlan */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* mark the ports to be tagged */
    cpssOsMemCpy(&portsTagging, &portsMembers, sizeof(portsTagging));

    /* set tagging command for the ports */
    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]] =
                              PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;

    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]] =
                              PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 0;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,   vlanId,   &portsMembers,
                                 &portsTagging, &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite");
};



/**
* @internal prvTgfCscdDsaFrwdSrcIdConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdConfigSet
(
    GT_VOID
)
{
    GT_U16    floodVidx=0;
    GT_STATUS rc;
    /* temporary variables used to call prvTgfBrgVlanEntryRead */
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));

    prvTgfCscdDsaFrwdSrcIdConfigSave();

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: initialize vlan and source-id by random values */
    prvTgfVlanId  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;
    prvTgfSrcId = cpssOsRand() % (PRV_TGF_MAX_SRC_ID - 2) + 2;

    PRV_UTF_LOG2_MAC("We'll use vlanId=%d, srcId=%d ",
                     prvTgfVlanId, prvTgfSrcId);

    /* get flooding eVidx for vlan prvTgfVlanId */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, prvTgfVlanId, &portsMembers,
                          &portsTagging, &isValid, &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

    floodVidx = vlanInfo.floodVidx;

    /* AUTODOC: complete an initialization of packets structures  */
    prvTgfDsaInfo_forward.origSrcPhy.portNum  = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    prvTgfPacketVlanTag0Part.vid              = prvTgfVlanId;

    prvTgfPacketDsaTagPart.commonParams.vid   = prvTgfVlanId;
    prvTgfDsaInfo_forward.dstInterface.vlanId = prvTgfVlanId;

    prvTgfDsaInfo_forward.srcId               = prvTgfSrcId;
    prvTgfDsaInfo_forward.srcHwDev            = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_forward.source.portNum      = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    prvTgfDsaInfo_forward.dstInterface.vidx   = floodVidx;

    /* bind the DSA tag FORWARD part at run time because a union is used */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfDsaInfo_forward;


    /*-------------------------CREATE VLANS-----------------------------------*/
    prvTgfCscdDsaFrwdSrcIdCreateVlan(prvTgfVlanId);

    /*-------------------SETUP RX-PORT's SOURCE-ID VALUE----------------------*/

    /* AUTODOC: setup a source id that will be assigned to incoming packets */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], prvTgfSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* Enable ForceSrcId on rx port to avoid override of srcId by FDB */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableSet");

    if(GT_FALSE == run_xCat3x_tx_to_remotePort)
    {
        /* AUTODOC: setup tx port to be a cascade port */
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    }

    /* AUTODOC: add tx port to source-id group prvTgfSrcId */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, prvTgfSrcId,
                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortAdd(tx)");
}


/**
* @internal prvTgfCscdDsaFrwdSrcIdConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdConfigRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: invalidate used vlan entries */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfVlanId);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable packet capturing on tx port */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   =  prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                 iface.devPort.portNum);

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    if(GT_FALSE == run_xCat3x_tx_to_remotePort)
    {
        /* AUTODOC: restore a cascade configuration of the tx-port */
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                                   CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   prvTgfSavedTxCscdPortType);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    }

    /* AUTODOC: restore default source id value of rx port */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      prvTgfSavedRxDefSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* AUTODOC: restore a ForceSrcId state of rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableGet");

    /* AUTODOC: add tx port to source-id group prvTgfSrcId */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, prvTgfSrcId,
                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortAdd");

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(PRV_TGF_RX_PORT_IDX_CNS);
    }

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}


/**
* @internal prvTgfCscdDsaFrwdSrcIdSendPacket function
* @endinternal
*
* @brief   Send the packet
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdSendPacket
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
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d",
                                     prvTgfPortsArray[portIter]);
    }


    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficTableRxPcktTblClear error");


    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "StartTransmitting: %d\n",
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
}

/**
* @internal prvTgfCscdDsaFrwdSrcIdCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isExpected               - whether the packet should be found on the tx port.
* @param[in] isSentWithDsa            - whether the packet setn with DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received with DSA-tag.
*                                       None
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdCheckOnPorts
(
    IN GT_BOOL isExpected,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    CPSS_INTERFACE_INFO_STC iface          = {0};
    TGF_PACKET_STC          *packetInfoPtr = NULL;
    GT_U32                  expRx;
    GT_U32                  expTx;
    GT_U32                  packetSize;
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;


    if (GT_TRUE == isExpected)
    {
        /* wait the packets come to CPU */
        tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);
    }

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        if (PRV_TGF_RX_PORT_IDX_CNS == i)
        {
            packetSize = (GT_TRUE == isSentWithDsa) ?
                                     PRV_TGF_CASCADE_PACKET_SIZE_CNS
                                     : PRV_TGF_NETWORK_PACKET_SIZE_CNS;
            expRx = expTx = 1;
        }
        else if ( GT_TRUE == isExpected && PRV_TGF_TX_PORT_IDX_CNS == i)
        {
            packetSize = (GT_TRUE == isExpWithDsa) ?
                                     PRV_TGF_CASCADE_PACKET_SIZE_CNS
                                     : PRV_TGF_NETWORK_PACKET_SIZE_CNS;
            expRx = expTx = 1;
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

    /* AUTODOC: Check egress packet bytes are 'as expected' */
    if (GT_TRUE == isExpected)
    {
        iface.type              = CPSS_INTERFACE_PORT_E;
        iface.devPort.hwDevNum  = prvTgfDevNum;
        iface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

        /* AUTODOC: check the captured packet */
        packetInfoPtr = (GT_TRUE == isExpWithDsa) ?
                             &prvTgfCascadePacketInfo : &prvTgfNetworkPacketInfo;

        if(isExpWithDsa)
        {
            /* the srcID field may be different than we expect due to erratum for
               remote physical ports */
            prvTgfDsaInfo_forward.srcId               =
                prvTgfTrafficGeneratorPacketDsaTagGetSrcId(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                    prvTgfSrcId);

            prvTgfDsaInfo_forward.origSrcPhy.portNum = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                                prvTgfDevNum,
                                                                prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

            /* bind the DSA tag FORWARD part at run time because a union is used */
            prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfDsaInfo_forward;
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
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", iface.devPort.portNum, rc);

        UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, actualNumOfPackets,
                   "Triggers number: port #%d, expected - %d, received - %d\n",
                   iface.devPort.portNum, prvTgfBurstCount, actualNumOfPackets);


        /* RESTORE the origSrcPhy to 'default' */
        prvTgfDsaInfo_forward.origSrcPhy.portNum  = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
        /* RESTORE the srcId to 'default' */
        prvTgfDsaInfo_forward.srcId               = prvTgfSrcId;
        /* bind the DSA tag FORWARD part at run time because a union is used */
        prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfDsaInfo_forward;

    }
}


/**
* @internal prvTgfCscdDsaFrwdSrcIdSendTwoAndCheck function
* @endinternal
*
* @brief   Add/remove the tx port from source-id group, send 2 packet and check.
*
* @param[in] isSentWithDsa            - packet is sent with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
* @param[in] isExpWithDsa             - packet is expected with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
*                                       None
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdSendTwoAndCheck
(
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    GT_STATUS rc;

    /* AUTODOC: setup a packet to be sent (with or without eDSA tag) */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                   (GT_TRUE == isSentWithDsa ? &prvTgfCascadePacketInfo
                                             : &prvTgfNetworkPacketInfo),
                   prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: add tx port to source-id group prvTgfSrcId */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, prvTgfSrcId,
                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortAdd");

    /* AUTODOC: send the packet and check the tx port */
    PRV_UTF_LOG1_MAC("\nSend with a tx port added to a source-id group %d\n\n",
                     prvTgfSrcId);

    prvTgfCscdDsaFrwdSrcIdSendPacket();

    prvTgfCscdDsaFrwdSrcIdCheckOnPorts(GT_TRUE, isSentWithDsa, isExpWithDsa);

    /* AUTODOC: delete tx port from source-id group prvTgfSrcId */
    rc = prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum, prvTgfSrcId,
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortAdd(tx)");

    /* AUTODOC: send the packet and check the tx port */
    PRV_UTF_LOG1_MAC("\nSend with a tx port removed from a source-id group %d\n\n",
                     prvTgfSrcId);

    prvTgfCscdDsaFrwdSrcIdSendPacket();

    prvTgfCscdDsaFrwdSrcIdCheckOnPorts(GT_FALSE, isSentWithDsa, isExpWithDsa);
}


/**
* @internal prvTgfCscdDsaFrwdSrcIdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIdTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS               rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: generate traffic */

    /* AUTODOC: enable packet capturing on tx port. */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                               TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************************************************************************/
    /* AUTODOC: CASE 1. From network port to cascade port */

    if(run_xCat3x_rx_from_remotePort == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("\nCASE 4. From a (remote physical port) network port to a cascade port\n");
    }
    else
    if(run_xCat3x_tx_to_remotePort == GT_TRUE)
    {
        goto run_xCat3x_tx_to_remotePort_lbl;
    }
    else
    {
        PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");
    }


    /* AUTODOC: setup a packet without eDSA tag to be sent */
    prvTgfCscdDsaFrwdSrcIdSendTwoAndCheck(GT_FALSE, GT_TRUE);


    if(run_xCat3x_rx_from_remotePort == GT_TRUE)
    {
        /* no more cases on this mode */
        return;
    }

    /**************************************************************************/
    /* AUTODOC: CASE 2. Send a packet from cascade port to cascade port */

    PRV_UTF_LOG0_MAC("\nCASE 2. From a cascade port to a cascade port\n");

    /* set Hw device number to be different from srcHwDev in the eDSA tag */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(PRV_TGF_RX_PORT_IDX_CNS);
    }


    /* AUTODOC: setup rx port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    prvTgfCscdDsaFrwdSrcIdSendTwoAndCheck(GT_TRUE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 3 Send a packet from cascade port to network port */
run_xCat3x_tx_to_remotePort_lbl:
    if(run_xCat3x_tx_to_remotePort == GT_TRUE)
    {
        /* AUTODOC: setup rx port to be a cascade port */
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

        PRV_UTF_LOG0_MAC("\nCASE 5. From a cascade port to a network port (remote physical port) \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");
    }

    /* AUTODOC: setup tx port to be a network port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    prvTgfCscdDsaFrwdSrcIdSendTwoAndCheck(GT_TRUE, GT_FALSE);
}


/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaFrwdSrcIdTest function
* @endinternal
*
* @brief   Test a source-ID value in the eDsa tag with command FORWARD
*/
GT_VOID prvTgfCscdDsaFrwdSrcIdTest
(
    GT_VOID
)
{
    prvTgfCscdDsaFrwdSrcIdConfigSet();
    prvTgfCscdDsaFrwdSrcIdTrafficGenerate();
    prvTgfCscdDsaFrwdSrcIdConfigRestore();
}

/**
* @internal prvTgfCscdDsaFrwdSrcIdTest_xCat3x_Extra function
* @endinternal
*
* @brief   Test a source-ID value in the eDsa tag with command FORWARD
*         on remote physical ports
*/
GT_VOID prvTgfCscdDsaFrwdSrcIdTest_xCat3x_Extra
(
    GT_VOID
)
{
    run_xCat3x_rx_from_remotePort = GT_TRUE;
    prvTgfCscdDsaFrwdSrcIdTest();
    run_xCat3x_rx_from_remotePort = GT_FALSE;

    run_xCat3x_tx_to_remotePort = GT_TRUE;
    prvTgfCscdDsaFrwdSrcIdTest();
    run_xCat3x_tx_to_remotePort = GT_FALSE;
}


