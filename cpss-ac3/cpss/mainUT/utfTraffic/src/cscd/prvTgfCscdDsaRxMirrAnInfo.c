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
* @file prvTgfCscdDsaRxMirrAnInfo.c
*
* @brief check eDSA (TO_ANALYZER) fields inside an rx-mirrored packet
* - <Analyzer phy port is valid>
* - <Analyzer target device>
* - <Analyzer target physical port>
* - <Analyzer target ePort>
* Test check 3 cases in two modes HOP_BY_HOP and END_TO_END
* 1) rx port is a network port and the mirrored packet egresses
* to cascade port.
* 2) rx port is a cascade port and the mirrored packet egresses
* to cascade port.
* 3) rx port is a casccade port and the mirrored packet egresses to
* network port (target analyzer port).
* In 1st case we check three subcases: analyzer ePort is consequentially
* is mapped to local port, to remote port and to trunk (with a remote port
* as trunk's member)
*
*
* @version   4
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
#include <common/tgfTrunkGen.h>
#include <cscd/prvTgfCscdDsaRxMirrAnInfo.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define PRV_TGF_PAYLOAD_LEN_CNS   0x30 /* 48 */

/* PRV_TGF_PORTS_COUNT_CNS equal to variabe prvTgfPortsNum. I need the macro
   because a variable can't be used as array size in declaration of variables*/
#define PRV_TGF_PORTS_COUNT_CNS 4
/* macro to simplify setting a port to be a cascade. Direction can be RX, TX */
#define PRV_TGF_PORT_CSCD_SET_MAC(direction, portIndex, type)         \
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,                          \
                               CPSS_PORT_DIRECTION_##direction##_E,   \
                               prvTgfPortsArray[(portIndex)],         \
                               (type));                               \
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet"); \

/* a destination analyzer's device */
#define PRV_TGF_TARGET_DEV_CNS (prvTgfSavedHwDevNum < 2 ? \
                                2 : prvTgfSavedHwDevNum - 1)

/* device number different than the local device and the target device */
#define PRV_TGF_MIDDLE_DEV_CNS (prvTgfSavedHwDevNum < 2 ? \
                                3 : prvTgfSavedHwDevNum - 2)

/* the analyzer port on the remote device*/
#define PRV_TGF_REMOTE_PORT_CNS prvTgfPortsArray[prvTgfAnPortIx]

/* max trunk ID value. For random generation*/
#define PRV_TGF_MAX_TRUNK_ID        256

/* enum describing used analyzer interfaces types */
typedef enum
{
    PRV_TGF_AN_IFACE_LOCAL_EPORT_E,
    PRV_TGF_AN_IFACE_REMOTE_EPORT_E,
    PRV_TGF_AN_IFACE_TRUNK_E
} PRV_TGF_AN_IFACE_ENT;


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
    GT_TRUE,/* rxSniffer - egress mirroring */
    GT_TRUE,/* isTagged */

    {/* the sniffed port info - src (for rx mirror) or dest (for tx mirror) */
        0,/* hwDevNum */
        0,/* portNum */
        0 /* ePort */
    },/* devPort */

    CPSS_INTERFACE_PORT_E,/* analyzerTrgType */ /* will be modified later */
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
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag}, /* will be modified */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/************************** PACKET INFO STRUCTURES ****************************/

/* a network vlan tagged packet info */
static TGF_PACKET_STC prvTgfNetwPacketInfo = {
    PRV_TGF_NETW_TAG_PACKET_SIZE_CNS,
    sizeof(prvTgfNetwPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfNetwPacketPartsArray
};

/* a cascade vlan tagged packet info */
static TGF_PACKET_STC prvTgfCscdPacketInfo = {
    PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS,
    sizeof(prvTgfCscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfCscdPacketPartsArray
};


/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some values that will be initialized at run time */
static TGF_VLAN_ID prvTgfVlanId;         /* vlan id */
static GT_U32      prvTgfRxPortIx;       /* rx port index */
static GT_U32      prvTgfTxPortIx;       /* cascade port connected with target device */
static GT_U32      prvTgfAnPortIx;       /* an analyzer port index used in END_TO_END mode */
static GT_U32      prvTgfGlobalAnPortIx; /* an analyzer port index used in HOP_BY_HOP mode */
static GT_PORT_NUM prvTgfEport;          /* analyzer ePort */
static GT_TRUNK_ID prvTgfTrunkId;

/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedRxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedRxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedCscdPortType[PRV_TGF_PORTS_COUNT_CNS];
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static CPSS_INTERFACE_INFO_STC                        prvTgfSavedEportIface;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCascadeLink;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static GT_U32                                         prvTgfSavedAnalyzerIndex;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];


/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/*************************** PRIVATE DECLARATIONS *****************************/
/**
* @internal prvTgfCscdRxAnMirroringSet function
* @endinternal
*
* @brief   Setup a configuration related to mirroring.
*
* @param[in] mode                     - forwarding mode
* @param[in] index                    - analyzer index
* @param[in] devNum                   - an Analyzer interface's device id
* @param[in] ePort                    - an Analyzer interface's ePort
*                                       None
*/
static GT_VOID prvTgfCscdRxAnMirroringSet
(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode,
    IN GT_U32         index,
    IN GT_HW_DEV_NUM  devNum,
    IN GT_PORT_NUM    ePort
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC iface               = {{0}};
    GT_BOOL                               isGlobalIndexEn;
    GT_STATUS                             rc;

    /* setup forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(mode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    iface.interface.type             = CPSS_INTERFACE_PORT_E;
    iface.interface.devPort.hwDevNum = devNum;
    iface.interface.devPort.portNum  = ePort;

    isGlobalIndexEn =
        (mode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E) ?
        GT_TRUE : GT_FALSE;

    /* set the global rx Analyzer index */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(isGlobalIndexEn, index);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet");

    /* assign the analyzer index to an interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(index, &iface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                             "prvTgfMirrorAnalyzerInterfaceSet error %d", ePort);

    /* setup rx port to be mirrored. 3rd parameter is ignored in hop-by-hop  forwarding mode */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[prvTgfRxPortIx], GT_TRUE, index);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");
}


/**
* @internal prvTgfCscdRxAnConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdRxAnConfigSave
(
    GT_VOID
)
{
    GT_U32    i;
    GT_STATUS rc;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   prvTgfGlobalAnPortIx,
                                   prvTgfAnPortIx,
                                   prvTgfRxPortIx,
                                   prvTgfTxPortIx,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: save default Hw device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: save forwarding mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeGet");

    /* AUTODOC: save global tx Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfSavedAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to prvTgfSavedAnalyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          prvTgfSavedAnalyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of tx port */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[prvTgfRxPortIx],
                               &prvTgfSavedRxMirrorModeEnabled,
                               &prvTgfSavedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet error");

    /* AUTODOC: save a cascade configuration of used ports */
    for (i = 0; i <PRV_TGF_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[i],
                                   i == prvTgfRxPortIx ? CPSS_PORT_DIRECTION_RX_E
                                                       : CPSS_PORT_DIRECTION_TX_E,
                                   &prvTgfSavedCscdPortType[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");
    };

    /* AUTODOC: save interface assigned to prvTgfEport (E2PHY) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                         prvTgfEport, &prvTgfSavedEportIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");


    /* AUTODOC: save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                  PRV_TGF_TARGET_DEV_CNS,
                                  prvTgfPortsArray[prvTgfAnPortIx],
                                  &prvTgfSavedCascadeLink,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");
}


/**
* @internal prvTgfCscdRxAnInitArrayByRandom function
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
static GT_STATUS prvTgfCscdRxAnInitArrayByRandom
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
* @internal prvTgfCscdRxAnInitRandomValues function
* @endinternal
*
* @brief   Initialize some random values
*/
static GT_VOID prvTgfCscdRxAnInitRandomValues
(
    GT_VOID
)
{
    GT_U32 randArr[PRV_TGF_PORTS_COUNT_CNS];
    GT_STATUS rc;

    /* AUTODOC: initialize rx port, tx port, analyzer index, analyzer physical
       port, analyzer ePort (VIDX), vlanId and "fake" port by random values.
    */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* get unique random values for ports (rx, tx, analyzer) indexes */
    rc = prvTgfCscdRxAnInitArrayByRandom(randArr, 4, 0, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfCscdRxAnInitArrayByRandom");

    prvTgfRxPortIx       = randArr[0];
    prvTgfTxPortIx       = randArr[1];
    prvTgfAnPortIx       = randArr[2];
    prvTgfGlobalAnPortIx = randArr[3];

    prvTgfEport   = _2K + cpssOsRand() % _6K;
    prvTgfEport = ALIGN_RESOURCE_TO_DEV_MAC(prvTgfEport , _2K , UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum));

    /* random vlan id */
    prvTgfVlanId = 2 + cpssOsRand() % (_4K - 2);

    /* random trunkId */
    if (PRV_TGF_MAX_TRUNK_ID <= (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16))
    {
        prvTgfTrunkId = 1 + cpssOsRand() % (PRV_TGF_MAX_TRUNK_ID - 1);
    }
    else
    {
        prvTgfTrunkId = 1 + cpssOsRand() % ((GT_TRUNK_ID)(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16) - 1);
    }

    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(prvTgfTrunkId);

    /* display assigned random values */
    PRV_UTF_LOG0_MAC("\n\n***** RANDOMIZED VALUES *****:\n");
    PRV_UTF_LOG1_MAC("Rx port: %d\n", prvTgfPortsArray[prvTgfRxPortIx]);
    PRV_UTF_LOG1_MAC("Tx port: %d\n", prvTgfPortsArray[prvTgfTxPortIx]);
    PRV_UTF_LOG1_MAC("Analyzer physical port (END_TO_END mode): %d\n",
                     prvTgfPortsArray[prvTgfAnPortIx]);
    PRV_UTF_LOG1_MAC("Analyzer physical port (HOP_BY_HOP mode): %d\n",
                     prvTgfPortsArray[prvTgfGlobalAnPortIx]);
    PRV_UTF_LOG2_MAC("Analyzer ePort: %d(0x%X)\n",
                     prvTgfEport, prvTgfEport);
    PRV_UTF_LOG2_MAC("vlan id: %d(0x%X)\n", prvTgfVlanId, prvTgfVlanId);
    PRV_UTF_LOG0_MAC("\n");
}


/**
* @internal prvTgfCscdRxAnConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdRxAnConfigSet
(
    GT_VOID
)
{
    CPSS_TRUNK_MEMBER_STC       trunkMember     = {0};
    CPSS_CSCD_LINK_TYPE_STC     cascadeLink     = {0};
    GT_U8                       tag             = 1;
    GT_STATUS                   rc;

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */

    /* eDSA sniffed port's attributes */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.devPort.portNum  = prvTgfPortsArray[prvTgfRxPortIx];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort    = prvTgfPortsArray[prvTgfRxPortIx];

    /* set vlan id inside a packet's vlan tag and inside an eDSA tag */
    prvTgfPacketVlanTag0Part.vid       = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid      = prvTgfPacketVlanTag0Part.vid;

    /* assign an analyzer part of eDSA tag at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;


    /**************************** CREATE VLAN *********************************/

    /* AUTODOC: create vlan with tagged rx port */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanId,
                                           &prvTgfPortsArray[prvTgfRxPortIx],
                                           &prvTgfDevsArray[prvTgfRxPortIx],
                                           &tag, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* make our packet to be recognized as 'untagged' by the TGF and also
       the CPSS will not override the ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);


    /**************** CONFIGURE THE TX-PORT TO BE A CASCASE PORT **************/

    /* AUTODOC: configure the tx port to be a cascade port */
    PRV_TGF_PORT_CSCD_SET_MAC(TX, prvTgfTxPortIx,
                              CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);

    /* AUTODOC: map the tx port to a remote analyzer device */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum  = prvTgfPortsArray[prvTgfTxPortIx];

    prvTgfCscdDevMapTableSet(prvTgfDevNum,
                             PRV_TGF_TARGET_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /**************** CONFIGURE THE TRUNK **************/
    /* AUTODOC: add remote analyzer's physical port to the trunk */
    trunkMember.hwDevice = PRV_TGF_TARGET_DEV_CNS;
    trunkMember.port     = PRV_TGF_REMOTE_PORT_CNS;

    rc = prvTgfTrunkMembersSet(prvTgfTrunkId, 1, &trunkMember, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");
}


/**
* @internal prvTgfCscdRxAnConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdRxAnConfigRestore
(
    GT_VOID
)
{
    GT_U32                  i;
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

    /* AUTODOC: restore forwarding mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore global rx Analyzer index value */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(
               prvTgfSavedGlobalMirroringEn, prvTgfSavedAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet");

    /* AUTODOC: restore an old Analyzer bound to prvTgfAnalyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfSavedAnalyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of tx port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[prvTgfRxPortIx],
                               prvTgfSavedRxMirrorModeEnabled,
                               prvTgfSavedRxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");

    /* AUTODOC: restore a cascade configuration of used ports */
    for (i = 0; i < PRV_TGF_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                                   i == prvTgfRxPortIx ? CPSS_PORT_DIRECTION_RX_E
                                                       : CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[i],
                                   prvTgfSavedCscdPortType[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    }

    /* AUTODOC: restore physical interface assigned to prvTgfEport (E2PHY) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                        prvTgfEport, &prvTgfSavedEportIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                          "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: Clear trunk members */
    rc = prvTgfTrunkMembersSet(prvTgfTrunkId, 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");

    /* AUTODOC: restore the current cascade map table */
    prvTgfCscdDevMapTableSet(prvTgfDevNum,
                             PRV_TGF_TARGET_DEV_CNS,
                             PRV_TGF_REMOTE_PORT_CNS,
                             &prvTgfSavedCascadeLink,
                             prvTgfSavedSrcPortTrunkHashEn,
                             GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdRxAnSendPacket function
* @endinternal
*
* @brief   Send the packet
*
* @param[in] packetPtr                - (pointer to) th packet that should be sent
*                                       None
*/
static GT_VOID prvTgfCscdRxAnSendPacket
(
    TGF_PACKET_STC *packetPtr
)
{
    GT_U32    portIter;
    GT_STATUS rc;

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* reset ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* send the packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[prvTgfRxPortIx]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
}


/**
* @internal prvTgfCscdRxAnCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] expRxCntArr[]            - array of expected rx packet counters on the ports.
* @param[in] expTxCntArr[]            - array of expected tx packet counters on the ports.
* @param[in] expPktArr[]              - array of pointers to packet's expected to be captured
*                                      on the ports
*                                       None
*
* @note size of expRxCntArr, expTxCntArr, expPktArr is prvTgfPortsNum
*
*/
static GT_VOID prvTgfCscdRxAnCheckOnPorts
(
    IN GT_U32          expRxCntArr[],
    IN GT_U32          expTxCntArr[],
    IN TGF_PACKET_STC *expPktArr[]
)
{
    CPSS_INTERFACE_INFO_STC iface          = {0};
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;


    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < PRV_TGF_PORTS_COUNT_CNS; ++i)
    {
        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRxCntArr[i], expTxCntArr[i],
                                    0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i],
                                     expRxCntArr[i], expTxCntArr[i]);
    }

    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Check trigger counters (captured packets) */

    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    /*iface.devPort.portNum will be initialized in the cycle below */

    for (i = 0; i < PRV_TGF_PORTS_COUNT_CNS; i++)
    {
        if (expPktArr[i])
        {
            iface.devPort.portNum   = prvTgfPortsArray[i];
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                            &iface,
                            expPktArr[i],
                            expRxCntArr[i],/*numOfPackets*/
                            0/* vfdNum */,
                            NULL /* vfdArray */,
                            NULL, /* bytesNum's skip list */
                            0,    /* length of skip list */
                            &actualNumOfPackets,
                            NULL/* onFirstPacketNumTriggersBmpPtr */);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                   "tgfTrafficGeneratorPortTxEthCaptureCompare:"
                                   " port = %d, rc = 0x%02X\n",
                                   iface.devPort.portNum, rc);
        }
    }
}


/**
* @internal prvTgfCscdRxAnDsaDestSrcFieldsSet function
* @endinternal
*
* @brief   Modify eDSA tag fields related to a destination analyzer interface
*         and related to a source device number. This function is applicable
*         both for incoming and outcoming DSA-tagged packet.
*         the tag
* @param[in] frwdMode                 - forwarding mode
* @param[in] isRxCascade              - whether rx port is a cascade port
* @param[in] isEgrCascade             - whether the port (tx port or analyzer port) where the
*                                      packet should be transmitted is a cascade port
* @param[in] analyzerType             - an analyzer type
* @param[in] isEgressed               -  whether a tagged packet is incoming (GT_FALSE)
*                                      or egressed (GT_TRUE).
*
* @param[out] tagPtr                   - pointer to a DSA tag
*                                      None
*                                       None
*/
static GT_VOID prvTgfCscdRxAnDsaDestSrcFieldsSet
 (
     OUT  TGF_PACKET_DSA_TAG_STC                       *tagPtr,
     IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT  frwdMode,
     IN   GT_BOOL                                       isRxCascade,
     IN   GT_BOOL                                       isEgrCascade,
     IN   PRV_TGF_AN_IFACE_ENT                          analyzerType,
     IN   GT_BOOL                                       isEgressed
 )
 {
     GT_HW_DEV_NUM        srcDev;
     GT_HW_DEV_NUM        trgDev;
     GT_BOOL              tagIsTrgPortValid;
     GT_HW_DEV_NUM        tagHwTrgDev;
     GT_PHYSICAL_PORT_NUM tagTrgPort;
     GT_PHYSICAL_PORT_NUM anPort;

     srcDev = GT_FALSE == isRxCascade ? /* the local device is a sniffed device */
         prvTgfSavedHwDevNum : PRV_TGF_MIDDLE_DEV_CNS;

     trgDev = GT_FALSE == isEgrCascade ? /* the local device is a target */
         prvTgfSavedHwDevNum : PRV_TGF_TARGET_DEV_CNS;

     anPort = (PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E== frwdMode) ?
         prvTgfPortsArray[prvTgfGlobalAnPortIx]
         : prvTgfPortsArray[prvTgfAnPortIx];

     if (PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E ==frwdMode
         && GT_TRUE == isEgressed && GT_TRUE == isRxCascade)
     {
         /* An egress packet tag is the same as incoming packet except for
            HOP_BY_HOP mode cascade rx port */

         tagIsTrgPortValid = 1;
         tagHwTrgDev       = prvTgfSavedHwDevNum;
         tagTrgPort        = anPort;
     }
     else
     {
         switch (analyzerType)
         {
             case PRV_TGF_AN_IFACE_LOCAL_EPORT_E:
                 tagIsTrgPortValid = 1;
                 tagHwTrgDev       = srcDev;
                 tagTrgPort        = anPort;
                 break;

             case PRV_TGF_AN_IFACE_REMOTE_EPORT_E:
                 tagIsTrgPortValid = 0;
                 tagHwTrgDev       = trgDev;
                 tagTrgPort        = PRV_TGF_REMOTE_PORT_CNS;
                 break;

             case PRV_TGF_AN_IFACE_TRUNK_E:
                 tagIsTrgPortValid = 1;
                 tagHwTrgDev       = trgDev;
                 tagTrgPort        = PRV_TGF_REMOTE_PORT_CNS;
                 break;

             default:
                 UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Unknown an analyzer type");
                 return;
         }
     }

     tagPtr->dsaInfo.toAnalyzer.devPort.hwDevNum                           = srcDev;
     tagPtr->dsaInfo.toAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
     tagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerTrgEport       = prvTgfEport;
     tagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = tagIsTrgPortValid;
     tagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = tagHwTrgDev;
     tagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerTrgPort        = tagTrgPort;
 }


 /*******************************************************************************
 * prvTgfCscdRxAnPrepareSendAndCheck
 *
 * DESCRIPTION:
 *       Make all required preparations and generate traffic
 *
 * INPUTS:
 *       frwdMode     - forwarding mode
 *       isRxCascade  - whether rx port is a cascade port
 *       isEgrCascade - whether the port where the packet should be transmitted
 *                      (tx port or analyzer port) is a cascade port
 *       anType       - a kind of the analyzer interface
 *
 * OUTPUTS:
 *       None
 *
 * RETURNS:
 *       None
 *
 * COMMENTS:
 *
 *******************************************************************************/
 static GT_VOID prvTgfCscdRxAnPrepareSendAndCheck
 (
     IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT frwdMode,
     IN GT_U32                                         isRxCascade,
     IN GT_U32                                         isEgrCascade,
     IN PRV_TGF_AN_IFACE_ENT                           anType
 )
 {
     CPSS_INTERFACE_INFO_STC  iface;
     GT_U32                   expRxCntArr[PRV_TGF_PORTS_COUNT_CNS] = {0};
     GT_U32                   expTxCntArr[PRV_TGF_PORTS_COUNT_CNS] = {0};
     TGF_PACKET_STC          *expPktArr[PRV_TGF_PORTS_COUNT_CNS] = {NULL};
     CPSS_CSCD_PORT_TYPE_ENT  rxCscdType;
     CPSS_CSCD_PORT_TYPE_ENT  egrCscdType;
     GT_U32                   egrPortIx;
     GT_U32                   analyzerPortIx;
     GT_HW_DEV_NUM            targetDevNum;
     GT_STATUS                rc;
     GT_BOOL                  expectFail = GT_FALSE;

     cpssOsMemSet(&iface, 0, sizeof iface);

     /*rx port type - cascade or network */
     rxCscdType = GT_TRUE == isRxCascade ?
         CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E;

     /*egress port type - cascade or network */
     egrCscdType  = GT_TRUE == isEgrCascade ?
         CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E;

     /* analyzer port */
     analyzerPortIx = (frwdMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E) ?
         prvTgfGlobalAnPortIx : prvTgfAnPortIx;

     /* egress port */
     egrPortIx = ( (frwdMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E
                    && GT_TRUE ==isRxCascade)
                   || PRV_TGF_AN_IFACE_LOCAL_EPORT_E == anType
                   || GT_FALSE == isEgrCascade) ?
         analyzerPortIx : prvTgfTxPortIx;

     /* AUTODOC: setup rx, tx ports type (cascade or network) */
     PRV_TGF_PORT_CSCD_SET_MAC(RX, prvTgfRxPortIx, rxCscdType);
     PRV_TGF_PORT_CSCD_SET_MAC(TX, egrPortIx, egrCscdType);

     /* AUTODOC: map ePort to an analyzer device/interface. */
     if (GT_TRUE == isRxCascade &&
         PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E == frwdMode)

     {
         /* in HOP_BY_HOP mode we use the same analyzer in 2nd and 3rd cases */
         targetDevNum           = prvTgfDevNum;
         iface.type             = CPSS_INTERFACE_PORT_E;
         iface.devPort.hwDevNum = prvTgfDevNum;
         iface.devPort.portNum  = prvTgfPortsArray[prvTgfGlobalAnPortIx];
     }
     else
     {
         switch (anType)
         {
             case PRV_TGF_AN_IFACE_LOCAL_EPORT_E:
                 targetDevNum           = prvTgfDevNum;
                 iface.type             = CPSS_INTERFACE_PORT_E;
                 iface.devPort.hwDevNum = prvTgfDevNum;
                 iface.devPort.portNum  = prvTgfPortsArray[analyzerPortIx];
                 break;

             case PRV_TGF_AN_IFACE_TRUNK_E:
                 targetDevNum  = prvTgfDevNum;
                 iface.type    = CPSS_INTERFACE_TRUNK_E;
                 iface.trunkId = prvTgfTrunkId;

                 if(prvUtfIsDoublePhysicalPortsModeUsed() &&
                    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E == frwdMode &&
                    isRxCascade == GT_TRUE   &&
                    isEgrCascade == GT_FALSE &&
                    prvTgfPortsArray[prvTgfGlobalAnPortIx] >= 256)/* need more than 8 bits in eDSA */
                 {
                    /* not enough bits in the eDSA to support this check ! */
                    expectFail = GT_TRUE;
                 }

                 break;

             case PRV_TGF_AN_IFACE_REMOTE_EPORT_E:
                 targetDevNum = PRV_TGF_TARGET_DEV_CNS;

                 /* ePort will be mapped to the local physical port with the same
                    number as remote physical port */
                 iface.type             = CPSS_INTERFACE_PORT_E;
                 iface.devPort.hwDevNum = prvTgfDevNum;
                 iface.devPort.portNum  = prvTgfPortsArray[analyzerPortIx];
                 break;

             default:
                 UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Unknown an analyzer type");
                 return;
         }
     }

     rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                            prvTgfEport, &iface);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

     /* AUTODOC: enable mirroring to ePort (on the device targetDevNum) */
     prvTgfCscdRxAnMirroringSet(frwdMode, prvTgfSavedAnalyzerIndex,
                                targetDevNum, prvTgfEport);

     /* AUTODOC: send the packet and check the result */
     iface.type              = CPSS_INTERFACE_PORT_E;
     iface.devPort.hwDevNum  = prvTgfDevNum;
     iface.devPort.portNum   = prvTgfPortsArray[egrPortIx];

     rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

     /* initialize a DSA tag of the incoming packet */
     prvTgfCscdRxAnDsaDestSrcFieldsSet(&prvTgfDsaTag, frwdMode, isRxCascade,
                                      isEgrCascade, anType, GT_FALSE);

    prvTgfCscdRxAnSendPacket( GT_TRUE == isRxCascade ? &prvTgfCscdPacketInfo
                                                     : &prvTgfNetwPacketInfo);

    /* AUTODOC: check the result on egress ports */
    /* initialize expected counters and expected packets */
    expRxCntArr[prvTgfRxPortIx] = prvTgfBurstCount;
    expTxCntArr[prvTgfRxPortIx] = prvTgfBurstCount;

    if(expectFail == GT_TRUE)
    {
        expRxCntArr[egrPortIx] = 0;
        expTxCntArr[egrPortIx] = 0;

        expPktArr[egrPortIx] = NULL;
    }
    else
    {
        expRxCntArr[egrPortIx] = prvTgfBurstCount;
        expTxCntArr[egrPortIx] = prvTgfBurstCount;
        expPktArr[egrPortIx] = (GT_TRUE == isEgrCascade) ? &prvTgfCscdPacketInfo
                                                         : &prvTgfNetwPacketInfo;
    }

    /* initialize a DSA tag of the expected packet */
    prvTgfCscdRxAnDsaDestSrcFieldsSet(&prvTgfDsaTag, frwdMode, isRxCascade,
                                      isEgrCascade, anType, GT_TRUE);

    prvTgfCscdRxAnCheckOnPorts(expRxCntArr, expTxCntArr, expPktArr);

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet");
}


/**
* @internal prvTgfCscdRxAnTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] mode                     - forwarding  used for mirroring.
*                                       None
*/
static GT_VOID prvTgfCscdRxAnTrafficGenerate

(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode
)
{

    PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");
    PRV_UTF_LOG0_MAC("\nCASE 1.1 Mirrored to local physical port\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_FALSE, GT_TRUE,
                                    PRV_TGF_AN_IFACE_LOCAL_EPORT_E);

    PRV_UTF_LOG0_MAC("\nCASE 1.2 Mirrored to remote physical port\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_FALSE, GT_TRUE,
                                    PRV_TGF_AN_IFACE_REMOTE_EPORT_E);

    PRV_UTF_LOG0_MAC("\nCASE 1.3 Mirrored to trunk\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_FALSE, GT_TRUE,
                                    PRV_TGF_AN_IFACE_TRUNK_E);

    PRV_UTF_LOG0_MAC("\nCASE 2. From a cascade port to a cascade port\n");
    PRV_UTF_LOG0_MAC("\nCASE 2.1 Mirrored to remote physical port\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_TRUE, GT_TRUE,
                                    PRV_TGF_AN_IFACE_REMOTE_EPORT_E);

    PRV_UTF_LOG0_MAC("\nCASE 2.2 Mirrored to trunk\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_TRUE, GT_TRUE,
                                    PRV_TGF_AN_IFACE_TRUNK_E);

    PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");
    PRV_UTF_LOG0_MAC("\nCASE 3.1 Mirrored to remote physical port\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_TRUE, GT_FALSE,
                                    PRV_TGF_AN_IFACE_REMOTE_EPORT_E);

    PRV_UTF_LOG0_MAC("\nCASE 3.2 Mirrored to trunk\n");
    prvTgfCscdRxAnPrepareSendAndCheck(mode, GT_TRUE, GT_FALSE,
                                    PRV_TGF_AN_IFACE_TRUNK_E);
}


/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaRxMirrAnInfoTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaRxMirrAnInfoTest
(
    GT_VOID
)
{
    prvTgfCscdRxAnInitRandomValues();
    prvTgfCscdRxAnConfigSave();
    prvTgfCscdRxAnConfigSet();

    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: setup HOP-BY-HOP forwarding mode */
        PRV_UTF_LOG0_MAC("\n***** HOP-BY-HOP FORWARDING MODE *****\n\n");
        prvTgfCscdRxAnTrafficGenerate(
            PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    }

    /* AUTODOC: setup END-TO-END forwarding mode */
    PRV_UTF_LOG0_MAC("\n***** END-TO-END FORWARDING MODE *****\n\n");
    prvTgfCscdRxAnTrafficGenerate(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);

    prvTgfCscdRxAnConfigRestore();
}

