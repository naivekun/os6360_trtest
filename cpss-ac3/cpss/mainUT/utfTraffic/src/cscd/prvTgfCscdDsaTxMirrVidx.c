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
* @file prvTgfCscdDsaTxMirrVidx.c
*
* @brief check DSA fields <Analyzer Use eVIDX>, <Analyzer eVIDX>
* of a tx-mirrored packet.
* 1) configure a tx-mirroring on ePort mapped to VIDX.
* 2) move a physical port (called here an "vidx-port") to this VIDX.
* 3) set HOP-BY-HOP forwarding mode. Assign ePort to a global tx Analyzer
* 4) generate traffic on rx port and check egressed packet on the
* vidx-port in three cases:
* 4.1) rx port is network port, analyzer port is cascade port
* 4.2) rx port is cascade port, analyzer port is cascade port
* 4.3) rx port is cascade port, analyzer port is network port
* 5) disable global tx Analyzer interface.
* 6) Setup END-TO-END forwarding mode. Make ePort to be an analyzer
* for tx port.
* 7) repeat steps 4.1 - 4.3. Before step 4.2 reconfigure analyzer for
* tx port to be a port!=vidx-port to be sure that mirroring is
* performed according to eDSA <VIDX>/<use eVIDX> settings
* rather than local setting. Packet should be received on vidx-port
*
*
* @version   5
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
#include <common/tgfMirror.h>
#include <common/tgfBridgeGen.h>
#include <cscd/prvTgfCscdDsaTxMirrVidx.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define PRV_TGF_PAYLOAD_LEN_CNS   0x30 /* 48 */

/* macro to simplify setting a port to be a cascade. Direction can be RX, TX */
#define PRV_TGF_PORT_CSCD_SET_MAC(direction, portIndex, type)         \
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,                          \
                               CPSS_PORT_DIRECTION_##direction##_E,   \
                               prvTgfPortsArray[(portIndex)],         \
                               (type));                               \
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet"); \


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
    GT_FALSE,/* rxSniffer - egress mirroring */
    GT_TRUE,/* isTagged */

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        0,/* hwDevNum */
        0,/* portNum */
        0 /* ePort */
    },/* devPort */

    CPSS_INTERFACE_VIDX_E,/* analyzerTrgType */
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
    {{0}}, /* will be filled later */
    0 /* tag0TpidIndex */
};



/****************************** PACKETS LENGTH ********************************/
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

/* parts of the  vlan tagged packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetwPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part}, /* tag0 */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/************************** PACKET INFO STRUCTURES ****************************/

/* a network  vlan tagged packet info */
static TGF_PACKET_STC prvTgfNetwPacketInfo = {
    PRV_TGF_NETW_TAG_PACKET_SIZE_CNS,                                /* totalLen */
    sizeof(prvTgfNetwPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetwPacketPartsArray                                       /* partsArray */
};

/* a cascade vlan tagged packet info */
static TGF_PACKET_STC prvTgfCscdPacketInfo = {
    PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS,                                /* totalLen */
    sizeof(prvTgfCscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCscdPacketPartsArray                                       /* partsArray */
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some values that will be initialized at run time */
static TGF_VLAN_ID prvTgfVlanId;         /* vlan id */
static GT_U32      prvTgfRxPortIx;       /* rx port index */
static GT_U32      prvTgfTxPortIx;       /* tx port index */
static GT_U32      prvTgfAnPortIx;       /* analyzer port index*/
static GT_U32      prvTgfAnalazyerIndex; /* analyzer index */
static GT_PORT_NUM prvTgfAnEport;        /* analyzer ePort (mapped to VIDX) */
static GT_PORT_NUM prvTgfNotVidxPortIx;  /* analayzer port not in VIDX*/
static GT_U16      prvTgfVidx;

/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedTxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedTxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedRxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedAnCscdPortType;
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static CPSS_INTERFACE_INFO_STC                        prvTgfSavedE2phyIface;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/**
* @internal prvTgfCscdDsaTxMirrVidxConfigMirroring function
* @endinternal
*
* @brief   Set or restore configuration related to mirroring.
*
* @param[in] mode                     - forwarding mode
* @param[in] index                    - analyzer index
* @param[in] ePort                    -  of Analyzer interface
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxConfigMirroring
(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode,
    IN GT_U32 index,
    IN GT_PORT_NUM ePort
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC iface = {{0}};
    GT_BOOL isGlobalIndexEn;
    GT_STATUS rc;
    /* setup forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(mode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    iface.interface.type             = CPSS_INTERFACE_PORT_E;
    iface.interface.devPort.hwDevNum = prvTgfDevNum;
    iface.interface.devPort.portNum  = ePort;

    isGlobalIndexEn =
        (mode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E) ?
        GT_TRUE : GT_FALSE;

    /* set global tx Analyzer index */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(isGlobalIndexEn, index);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

    /* assign an analyzer index to interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(index, &iface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                             "prvTgfMirrorAnalyzerInterfaceSet error %d", ePort);

    /* setup tx port to be mirrored. 3rd parameter is ignored in hop-by-hop  forwarding mode */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx], GT_TRUE, index);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");
}

/*************************** PRIVATE DECLARATIONS *****************************/
/**
* @internal prvTgfCscdDsaTxMirrVidxConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save prvTgfPortsArray and exclude ports that can't be configured as cascade */
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

    /* AUTODOC: save global tx Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to prvTgfAnalazyerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          prvTgfAnalazyerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of tx port */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[prvTgfTxPortIx],
                               &prvTgfSavedTxMirrorModeEnabled,
                               &prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortGet error");

    /* AUTODOC: save a cascade configuration of the rx port */
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

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateGet");

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                         prvTgfAnEport, &prvTgfSavedE2phyIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
}


/**
* @internal prvTgfCscdDsaTxMirrVidxInitArrayByRandom function
* @endinternal
*
* @brief   Initialize array by unique random values.
*
* @param[in,out] arr[]                    - ay that should be filled.
* @param[in] arrLen                   - length of array
* @param[in] min                      - a minimal value for array elements
* @param[in] max                      - a maximal value for array elements
*
* @retval GT_OK                    -    everything is good.
* @retval GT_BAD_PARAM             - unacceptable argument values
*/
static GT_STATUS prvTgfCscdDsaTxMirrVidxInitArrayByRandom
(
    INOUT GT_U32 arr[],
    IN    GT_U32 arrLen,
    IN    GT_U32 min,
    IN    GT_U32 max
)
{
    GT_U32 i;
    GT_U32 j;
    GT_U32 new;

    if (max < min || arrLen > max - min + 1)
    {
        return GT_BAD_PARAM;
    }

    i = 0;
    while (i < arrLen)
    {
        new = min + cpssOsRand() % (max - min + 1);
        /* check whether the value "new" was already generated earlier */
        for (j = 0; j < i && arr[j] != new; j++);

        if (j == i || 0 == i) /* the "new" value is unique */
        {
            arr[i] = new;
            i++;
        }
    }
    return GT_OK;
}


/**
* @internal prvTgfCscdDsaTxMirrVidxInitRandomValues function
* @endinternal
*
* @brief   Initialize some random values
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxInitRandomValues
(
    GT_VOID
)
{
    GT_U32 portIxArr[4];
    GT_STATUS rc;
    GT_U32  vidxNum;

    /* AUTODOC: initialize rx port, tx port, analyzer index, analyzer physical
       port, analyzer ePort (VIDX), vlanId and "fake" port by random values.
    */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* get unique random values for ports (rx, tx, analyzer) indexes */
    rc = prvTgfCscdDsaTxMirrVidxInitArrayByRandom(portIxArr, 4, 0, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfCscdDsaTxMirrVidxInitArrayByRandom");
    prvTgfRxPortIx      = portIxArr[0];
    prvTgfTxPortIx      = portIxArr[1];
    prvTgfAnPortIx      = portIxArr[2];
    prvTgfNotVidxPortIx = portIxArr[3];
    /* random ePort. Choose value > _2K to be different than any
       physical port number */
    prvTgfAnEport =  _2K  + cpssOsRand() % _6K;
    prvTgfAnEport = ALIGN_RESOURCE_TO_DEV_MAC(prvTgfAnEport , _2K , UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum));
    /* random Analyzer index {0..6}*/
    prvTgfAnalazyerIndex = cpssOsRand() % 7;

    /* random vlan for tagged packets */
    prvTgfVlanId = 2 + cpssOsRand() % (_4K - 2);

    /* random vidx in range {1, 0xFFF-1} */
    prvTgfVidx = 1 + cpssOsRand() % (0xFFF-1);

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,PRV_TGF_CFG_TABLE_VIDX_E,&vidxNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: PRV_TGF_CFG_TABLE_VIDX_E : devNum %d",
                                 prvTgfDevNum);

    /* support device with less than 4K vidx */
    prvTgfVidx %= vidxNum;


    /* display assigned random values */
    PRV_UTF_LOG0_MAC("\n\n***** RANDOMIZED VALUES *****:\n");
    PRV_UTF_LOG1_MAC("Rx port: %d\n", prvTgfPortsArray[prvTgfRxPortIx]);
    PRV_UTF_LOG1_MAC("Tx port: %d\n", prvTgfPortsArray[prvTgfTxPortIx]);
    PRV_UTF_LOG1_MAC("Analyzer ePort (VIDX): %d\n", prvTgfAnEport);
    PRV_UTF_LOG1_MAC("Analyzer physical port: %d\n", prvTgfPortsArray[prvTgfAnPortIx]);
    PRV_UTF_LOG1_MAC("vlan id: %d\n", prvTgfVlanId);
    PRV_UTF_LOG0_MAC("\n");
}


/**
* @internal prvTgfCscdDsaTxMirrVidxConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxConfigSet
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    CPSS_INTERFACE_INFO_STC     iface;
    GT_STATUS                   rc;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&iface,    0, sizeof(iface));

    prvTgfCscdDsaTxMirrVidxInitRandomValues();

    prvTgfCscdDsaTxMirrVidxConfigSave();

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */
    /* sniffer port's attributes */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.devPort.portNum  = prvTgfPortsArray[prvTgfTxPortIx];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort    = prvTgfPortsArray[prvTgfTxPortIx];
    /* analyzer port's attribures */
    cpssOsMemSet(&prvTgfDsaInfo_toAnalyzer.extDestInfo, 0,
                 sizeof(prvTgfDsaInfo_toAnalyzer.extDestInfo));
    prvTgfDsaInfo_toAnalyzer.extDestInfo.multiDest.analyzerEvidx = prvTgfVidx;

    /* setup vlan id inside a packet's vlan tag and inside an eDSA tag */

    prvTgfPacketVlanTag0Part.vid       = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid      = prvTgfPacketVlanTag0Part.vid;

    /* bind the DSA tag ANALYZER part at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;

    /* AUTODOC: create vlan with all ports with tagging command =
       PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E*/
    rc =   prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfVlanId,
                                 PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /*------------------------CREATE AN FDB ENTRIES---------------------------*/

    /* AUTODOC: set an FDB entry to send vlan's traffic to tx port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          prvTgfVlanId,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[prvTgfTxPortIx],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* make our packet to be recognized as 'untagged' by the TGF and also
       the CPSS will not override the ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: map ePort to VIDX */
    iface.type =  CPSS_INTERFACE_VIDX_E;
    iface.vidx = prvTgfVidx;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                         prvTgfAnEport, &iface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: create VIDX with the analyzer port */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, &prvTgfPortsArray[prvTgfAnPortIx],
                               NULL, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet");
}

/**
* @internal prvTgfCscdDsaTxMirrVidxConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxConfigRestore
(
    GT_VOID
)
{
    GT_STATUS               rc;

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

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
               prvTgfSavedGlobalMirroringEn, prvTgfAnalazyerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore an old Analyzer bound to prvTgfAnalazerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfAnalazyerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of tx port */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx],
                               prvTgfSavedTxMirrorModeEnabled,
                               prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

    /* AUTODOC: restore a cascade configuration of the rx-port */
    PRV_TGF_PORT_CSCD_SET_MAC(RX, prvTgfRxPortIx, prvTgfSavedRxCscdPortType);

    /* AUTODOC: restore a cascade configuration of the Analyzer port */
    PRV_TGF_PORT_CSCD_SET_MAC(TX, prvTgfAnPortIx, prvTgfSavedAnCscdPortType);

    /* AUTODOC: restore physical interface assigned to prvTgfAnEport (E2PHY) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                        prvTgfAnEport, &prvTgfSavedE2phyIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: Clear VIDX members list. That is default */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, NULL, NULL, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray,
                 sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaTxMirrVidxSendPacket function
* @endinternal
*
* @brief   Send the packet
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxSendPacket
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

    /* enable packet capturing on the analyzer port. */
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
* @internal prvTgfCscdDsaTxMirrVidxCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isSentWithDsa            - whether the packet was sent with a DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received on the an analyzer
*                                      port with DSA-tag.
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxCheckOnPorts
(
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

    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 100, NULL);

    /* AUTODOC: Check trigger counters */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfAnPortIx];

    /* AUTODOC: check the captured packet */
    packetInfoPtr = (GT_TRUE == isExpWithDsa) ? &prvTgfCscdPacketInfo :
                                                &prvTgfNetwPacketInfo;

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
* @internal prvTgfCscdDsaTxMirrVidxSendAndCheck function
* @endinternal
*
* @brief   Send two packets (vlan tagged/untagged)
*
* @param[in] isSentWithDsa            - packet is sent with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag to rx port
* @param[in] isExpWithDsa             - packet is expected with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag on the analyzer port
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxSendAndCheck
(
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    TGF_PACKET_STC *packet;
    GT_STATUS rc;

    /* prepare the packet to be sent */
    packet = (GT_TRUE == isSentWithDsa) ? &prvTgfCscdPacketInfo :
                                          &prvTgfNetwPacketInfo;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packet, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    prvTgfCscdDsaTxMirrVidxSendPacket();

    prvTgfCscdDsaTxMirrVidxCheckOnPorts(isSentWithDsa, isExpWithDsa);
}


/**
* @internal prvTgfCscdDsaTxMirrVidxTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] mode                     - forwarding  used for mirroring.
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrVidxTrafficGenerate
(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode
)
{
    GT_STATUS rc;

    /* AUTODOC: setup eDSA hwDevNum to be local device number */
    prvTgfDsaTag.dsaInfo.toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;

    /* AUTODOC: setup mirroring related settings */
    prvTgfCscdDsaTxMirrVidxConfigMirroring(mode, prvTgfAnalazyerIndex,
                                           prvTgfAnEport);

    /**************************************************************************/
    /* AUTODOC: CASE 1. From network port to cascade port */
    PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");

    /* AUTODOC: setup an rx port to be a network port */
    PRV_TGF_PORT_CSCD_SET_MAC(RX, prvTgfRxPortIx, CPSS_CSCD_PORT_NETWORK_E);

    /* AUTODOC: setup analyzer port to be a cascade port */
    PRV_TGF_PORT_CSCD_SET_MAC(TX, prvTgfAnPortIx, CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);

    /* AUTODOC: send the packet */
    prvTgfCscdDsaTxMirrVidxSendAndCheck(GT_FALSE, GT_TRUE);

    if (mode != PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        /* re-map analyzer index to port not in VIDX to ensure that a packet
           is mirrored according to eDSA info (<eVIDX>, <use eVIDX> fields)
           rather than local analyzer info */
        prvTgfCscdDsaTxMirrVidxConfigMirroring(mode, prvTgfAnalazyerIndex,
                                         prvTgfPortsArray[prvTgfNotVidxPortIx]);
    }

    /**************************************************************************/
    /* AUTODOC: CASE 2. Send a packet from cascade port to cascade port */
    PRV_UTF_LOG0_MAC("\nCASE 2. From a cascade port to a cascade port\n");

    /* AUTODOC: set sniffed device to be different than local device number */
    prvTgfDsaTag.dsaInfo.toAnalyzer.devPort.hwDevNum =
                       (0 == prvTgfSavedHwDevNum) ? 1 : prvTgfSavedHwDevNum - 1;

    /* AUTODOC: setup an rx port to be a cascade port */
    PRV_TGF_PORT_CSCD_SET_MAC(RX, prvTgfRxPortIx, CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);

    /* AUTODOC: send the packet */
    prvTgfCscdDsaTxMirrVidxSendAndCheck(GT_TRUE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 3 Send a packet from cascade port to network port */

    PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");

    /* AUTODOC: setup an analyzer port to be a network port */
    PRV_TGF_PORT_CSCD_SET_MAC(TX, prvTgfAnPortIx, CPSS_CSCD_PORT_NETWORK_E);

    /* AUTODOC: send the packet */
    prvTgfCscdDsaTxMirrVidxSendAndCheck(GT_TRUE, GT_FALSE);
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaTxMirrVidxTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrVidxTest
(
    GT_VOID
)
{
    prvTgfCscdDsaTxMirrVidxConfigSet();


    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: setup HOP-BY-HOP forwarding mode */
        PRV_UTF_LOG0_MAC("\n***** HOP-BY-HOP FORWARDING MODE *****\n\n");

        prvTgfCscdDsaTxMirrVidxTrafficGenerate(
            PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    }

    /* AUTODOC: setup END-TO-END forwarding mode */
    PRV_UTF_LOG0_MAC("\n***** END-TO-END FORWARDING MODE *****\n\n");

    prvTgfCscdDsaTxMirrVidxTrafficGenerate(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);

    prvTgfCscdDsaTxMirrVidxConfigRestore();
}

