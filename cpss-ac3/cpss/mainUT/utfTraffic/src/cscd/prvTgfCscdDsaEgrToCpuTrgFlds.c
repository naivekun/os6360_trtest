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
* @file prvTgfCscdDsaEgrToCpuTrgFlds.c
*
* @brief Test checks target info for TO_CPU by Egress Pipe.
* The packet is sent:
* 1.1 from a network port to a local CPU
* 1.2 from a network port to a remote CPU
* 2  from a cascade port to a remote CPU
* 3  from a cascade port to a local CPU
* Following eDSA tag fields checked:
* Target Tagged / Untagged
* Target Device
* Target physical port
* Target ePort
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
#include <common/tgfTrunkGen.h>
#include <common/tgfNetIfGen.h>
#include <common/tgfCosGen.h>
#include <cscd/prvTgfCscdDsaEgrToCpuTrgFlds.h>

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
                               prvTgfPortsArray[(portIndex)], \
                               (type));                               \
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet"); \

/* a device where remote CPU port is placed */
#define PRV_TGF_TARGET_DEV_CNS (prvTgfSavedHwDevNum < 2 ? \
                                2 : prvTgfSavedHwDevNum - 1)

/* device number different than the local device and the target device */
#define PRV_TGF_MIDDLE_DEV_CNS (prvTgfSavedHwDevNum < 2 ? \
                                3 : prvTgfSavedHwDevNum - 2)

/* used CPU code*/
#define PRV_TGF_CPU_CODE_CNS CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E

/* CPU Codes table entry's field <Destination device pointer> (0..7) */
#define PRV_TGF_DEV_NUM_IX_CNS 5

/* PRV_TGF_ERRATUM_ENT:
 * known erratum related to the tx-mirroring to CPU (TO_CPU DSA tag)
 * - PRV_TGF_ERRATUM_SRC_TRG_E - wrong bit <srcTrg> inside DSA tag
 * - PRV_TGF_ERRATUM_SRC_TRG_TAGGED_A0_E - <srcTrg Tagged> =1 even if an
 *       incoming packet is TO_CPU DSA-tagged with <srcTrg Tagged>=0.
 *       If local CPU is the target the trapped packet is vlan tagged
 *       regardless of the bit <srcTrg Tagged> inside of incoming packet.
 */
typedef enum {
    PRV_TGF_ERRATUM_SRC_TRG_E,
    PRV_TGF_ERRATUM_SRC_TRG_TAGGED_A0_E
} PRV_TGF_ERRATUM_ENT;

/******************************* TEST PACKETS *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN Tag part */
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

/* EtherType part = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof prvTgfPayloadDataArr
};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfDsaTag = {
    TGF_DSA_CMD_TO_CPU_E, /*dsaCommand*/
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

static TGF_DSA_DSA_TO_CPU_STC prvTgfDsaInfo_toCpu = {
    GT_TRUE,  /*isEgressPipe*/
    GT_FALSE,   /*isTagged*/
    0,          /*hwDevNum*/
    GT_FALSE,   /* srcIsTrunk */
    {
        0,      /* srcTrunkId */
        0,      /* portNum */
       /* supported in extended, 4 words,  DSA tag */
        0       /* ePort */
    }, /* interface */

    CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E, /* cpuCode */
    GT_FALSE,   /* wasTruncated */
    0,          /* originByteCount */
    0,          /* timestamp */

    /* supported in extended, 4 words,  DSA tag */
    GT_FALSE,   /* packetIsTT */
    {
        0
    },          /* flowIdTtOffset */

    0           /* tag0TpidIndex */
};



/****************************** PACKETS LENGTH ********************************/
/* size of the tagged packet that came from network port  */
#define PRV_TGF_NETW_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_VLAN_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))
/* size of the untagged packet that came from network port  */
#define PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS         \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))


/* size of the tagged packet that came from cascade port */
#define PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_eDSA_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))

/**************************** PACKET PARTS ARRAY ******************************/

/* parts of the vlan tagged packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetwTaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part}, /* tag0 */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the vlan untagged packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetwUntaggedPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parts of the vlan tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag}, /* will be modified */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/************************** PACKET INFO STRUCTURES ****************************/

/* a network vlan tagged packet info */
static TGF_PACKET_STC prvTgfNetwTaggedPacketInfo = {
    PRV_TGF_NETW_TAG_PACKET_SIZE_CNS,
    sizeof(prvTgfNetwTaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfNetwTaggedPacketPartsArray
};

/* a network vlan untagged packet info */
static TGF_PACKET_STC prvTgfNetwUntaggedPacketInfo = {
    PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS,
    sizeof(prvTgfNetwUntaggedPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfNetwUntaggedPacketPartsArray
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
static TGF_VLAN_ID prvTgfVlanId;   /* vlan id */
static GT_U32      prvTgfRxPortIx;       /* rx port index */
static GT_U32      prvTgfTxPortIx;       /* tx port index */
static GT_U32      prvTgfCscdPortIx;     /* cascade port index mapped to remote device */
static GT_U32      prvTgfAnalyzerIndex;  /* Analyzer index */
static GT_PORT_NUM prvTgfEport;          /* ePort mapped to tx port */

/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedTxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedTxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedCscdPortType[PRV_TGF_PORTS_COUNT_CNS];
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static CPSS_INTERFACE_INFO_STC                        prvTgfSavedEportIface;
static CPSS_CSCD_LINK_TYPE_STC                        prvTgfSavedCascadeLink;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT               prvTgfSavedSrcPortTrunkHashEn;
static PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC           prvTgfSavedCpuCodeTableEntry;
static GT_U8                                          prvTgfSavedRxDefaultUp;
static GT_BOOL                                        prvTgfSavedForceNewDsaToCpuEn;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/*************************** PRIVATE DECLARATIONS *****************************/
/**
* @internal prvTgfCscdEgrToCpuErratum function
* @endinternal
*
* @brief   Function checks if an erratum is on the device.
*
* @param[in] err                      - erratum type
*
* @retval 1                        - erratum is in the device
* @retval 0                        - there is no erratum in the device
*/
static GT_U32 prvTgfCscdEgrToCpuErratum
(
    PRV_TGF_ERRATUM_ENT err
)
{
    GT_U32 result;
    GT_U32 isWm;
#if !defined(ASIC_SIMULATION) || defined(GM_USED)
    isWm =0;
#else
    isWm = 1;
#endif

    switch (err) {
        case PRV_TGF_ERRATUM_SRC_TRG_E: /* actual for HW, GM */
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
            {
                result = 0; /* Fixed in BC2 B0 */
            }
            else
            {
                result = !(isWm);
            }
            break;

        case PRV_TGF_ERRATUM_SRC_TRG_TAGGED_A0_E:
            /* actual for hardware device and for GM (both revision a0) */
            result = !(isWm) && UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum);
            break;

        default:                /* No erratum */
            result = 0;
    } /* switch */
    return result;
}


/**
* @internal prvTgfCscdEgrToCpuInitArrayByRandom function
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
static GT_STATUS prvTgfCscdEgrToCpuInitArrayByRandom
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
* @internal prvTgfCscdEgrToCpuInitRandomValues function
* @endinternal
*
* @brief   Initialize some random values
*/
static GT_VOID prvTgfCscdEgrToCpuInitRandomValues
(
    GT_VOID
)
{
    GT_U32 randArr[PRV_TGF_PORTS_COUNT_CNS];
    GT_STATUS rc;

    /* AUTODOC: initialize rx port, tx port, vlanId by random values */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* get unique random values for ports (rx, tx) indexes */
    rc = prvTgfCscdEgrToCpuInitArrayByRandom(randArr, PRV_TGF_PORTS_COUNT_CNS,
                                         0, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfCscdEgrToCpuInitArrayByRandom");

    prvTgfRxPortIx   = randArr[0];
    prvTgfTxPortIx   = randArr[1];
    prvTgfCscdPortIx = randArr[2];

    /* random ePorts > _2K to be different than any physical port number */
    prvTgfEport   = _2K + cpssOsRand() % _6K;
    prvTgfEport = ALIGN_RESOURCE_TO_DEV_MAC(prvTgfEport , _2K , UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum));

    /* random vlan id's */
    prvTgfVlanId = 2 + cpssOsRand() % (_4K - 2);

    /* display assigned random values */
    PRV_UTF_LOG0_MAC("\n\n***** RANDOMIZED VALUES *****:\n");
    PRV_UTF_LOG1_MAC("Rx port: %d\n", prvTgfPortsArray[prvTgfRxPortIx]);
    PRV_UTF_LOG1_MAC("Tx port: %d\n", prvTgfPortsArray[prvTgfTxPortIx]);
    PRV_UTF_LOG1_MAC("Cascade port (mapped to remote device): %d\n",
                     prvTgfPortsArray[prvTgfCscdPortIx]);
    PRV_UTF_LOG1_MAC("Tx ePort: %d\n", prvTgfEport);
    PRV_UTF_LOG2_MAC("tagged vlan id: %d(0x%X)\n", prvTgfVlanId, prvTgfVlanId);
    PRV_UTF_LOG0_MAC("\n");
}


/**
* @internal prvTgfCscdEgrToCpuWriteInDsaTag function
* @endinternal
*
* @brief   Modify some eDSA tag fields. This function is applicable
*         both for incoming and outcoming (captured on egress port)
*         DSA-tagged packet.
*
* @param[out] tagPtr                   - pointer to a DSA tag
*                                       None
*
* @note pr
*
*/
static GT_VOID prvTgfCscdEgrToCpuWriteInDsaTag
(
    OUT  TGF_PACKET_DSA_TAG_STC *tagPtr,
    IN   GT_BOOL                 isVlanTagged,
    IN   GT_BOOL                 isRxCascade
)
{
    tagPtr->dsaInfo.toCpu.isEgressPipe = GT_TRUE;
    tagPtr->dsaInfo.toCpu.isTagged     = isVlanTagged;

    tagPtr->dsaInfo.toCpu.originByteCount = TGF_CRC_LEN_CNS +
                 (GT_TRUE == isVlanTagged ? PRV_TGF_NETW_TAG_PACKET_SIZE_CNS :
                                            PRV_TGF_NETW_UNTAG_PACKET_SIZE_CNS);

    /* a device where original packet was egressed */
    tagPtr->dsaInfo.toCpu.hwDevNum = (GT_TRUE == isRxCascade) ?
                                          PRV_TGF_MIDDLE_DEV_CNS : prvTgfDevNum;
    tagPtr->dsaInfo.toCpu.interface.portNum = prvTgfPortsArray[prvTgfTxPortIx];
    tagPtr->dsaInfo.toCpu.interface.ePort = prvTgfEport;
}


/**
* @internal prvTgfCscdEgrToCpuConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdEgrToCpuConfigSave
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
                                   prvTgfRxPortIx,
                                   prvTgfCscdPortIx,
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

    /* AUTODOC: save an old Analyzer bound to prvTgfAnalyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          prvTgfAnalyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* AUTODOC: save a mirroring status of tx port */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[prvTgfTxPortIx],
                               &prvTgfSavedTxMirrorModeEnabled,
                               &prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortGet");

    /* AUTODOC: save a cascade configuration of used ports */
    for (i = 0; i < PRV_TGF_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[i],
                                   i == prvTgfRxPortIx ? CPSS_PORT_DIRECTION_RX_E
                                                       : CPSS_PORT_DIRECTION_TX_E,
                                   &prvTgfSavedCscdPortType[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");
    }

    /* AUTODOC: save interface assigned to prvTgfEport (E2PHY) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                         prvTgfEport, &prvTgfSavedEportIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");

    /* AUTODOC: save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                  PRV_TGF_TARGET_DEV_CNS,
                                  CPSS_CPU_PORT_NUM_CNS,
                                  &prvTgfSavedCascadeLink,
                                  &prvTgfSavedSrcPortTrunkHashEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet");

    /* AUTODOC: save the CPU Code table entry */
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, PRV_TGF_CPU_CODE_CNS,
                                    &prvTgfSavedCpuCodeTableEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableGet");

    /* get default user priority of rx port. This value will be placed by
       device into an eDSA tag for incoming untagged packets. */
    rc = prvTgfCosPortDefaultUpGet(prvTgfDevNum,
                                   prvTgfPortsArray[prvTgfRxPortIx],
                                   &prvTgfSavedRxDefaultUp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultUpGet");

    /* AUTODOC: save <Force new DSA to CPU> bit */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum,
                                                &prvTgfSavedForceNewDsaToCpuEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanForceNewDsaToCpuEnableGet");
}


/**
* @internal prvTgfCscdEgrToCpuConfigVlanSetup function
* @endinternal
*
* @brief   Setup vlan members
*
* @param[in] vid                      - vlan id
* @param[in] isTagged                 - whether a tx port is tagged in this vlan
*                                       None
*/
static GT_VOID prvTgfCscdEgrToCpuConfigVlanSetup
(
    IN TGF_VLAN_ID    vid,
    IN GT_BOOL        isTagged
)
{
    GT_U8 tagsArr[2];
    GT_U32 portsArr[2];
    GT_STATUS rc;

    portsArr[0] = prvTgfPortsArray[prvTgfRxPortIx];
    portsArr[1] = prvTgfPortsArray[prvTgfTxPortIx];


    tagsArr[0] = 1;
    tagsArr[1] = (GT_TRUE == isTagged? 1 : 0);

    rc = prvTgfBrgDefVlanEntryWithPortsSet(vid, portsArr, NULL, tagsArr,
                                           sizeof portsArr / sizeof portsArr[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");
}


/**
* @internal prvTgfCscdEgrToCpuConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdEgrToCpuConfigSet
(
    GT_VOID
)
{
    CPSS_CSCD_LINK_TYPE_STC               cascadeLink  = {0};
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceAn      = {{0}};
    CPSS_INTERFACE_INFO_STC               iface        = {0};
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC  cpuCodeEntry;
    GT_STATUS                             rc;

    /* make our packet to be recognized as 'untagged' by the TGF and also
       the CPSS will not override the ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: enable capturing on a cascade port mapped to remote CPU */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[prvTgfCscdPortIx];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface, TGF_CAPTURE_MODE_PCL_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /************************** MIRRORING SETTINGS ****************************/

    /* AUTODOC: setup END_TO_END forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
                        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: assign the analyzer index to a CPU port */
    ifaceAn.interface.type             = CPSS_INTERFACE_PORT_E;
    ifaceAn.interface.devPort.hwDevNum = prvTgfDevNum;
    ifaceAn.interface.devPort.portNum  = CPSS_CPU_PORT_NUM_CNS;

    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfAnalyzerIndex, &ifaceAn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: setup tx port to be mirrored  */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx],
                               GT_TRUE, prvTgfAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/
    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */

    /* eDSA sniffed port's attributes */
    prvTgfDsaInfo_toCpu.hwDevNum = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toCpu.interface.portNum  = prvTgfPortsArray[prvTgfTxPortIx];
    prvTgfDsaInfo_toCpu.interface.ePort    = prvTgfEport;

    /* set vlan id inside a packet's VLAN- and eDSA- tags */
    prvTgfPacketVlanTag0Part.vid       = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid      = prvTgfPacketVlanTag0Part.vid;

    /* setup User Priority fields inside expected eDSA tag and packet's vlan tag
       to be equal default UP of rx port */
    prvTgfDsaTag.commonParams.vpt      = prvTgfSavedRxDefaultUp;
    prvTgfPacketVlanTag0Part.pri       = prvTgfDsaTag.commonParams.vpt;


    /* assign a toCpu at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toCpu = prvTgfDsaInfo_toCpu;

    /***************************** FDB ENTRY **********************************/

    /* AUTODOC: create FDB entry to forward a packet to the egress ePort */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          prvTgfVlanId,
                                          prvTgfDevNum, prvTgfEport,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: map ePort to physical tx port */
    iface.type             = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum = prvTgfDevNum;
    iface.devPort.portNum  = prvTgfPortsArray[prvTgfTxPortIx];

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           prvTgfEport, &iface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                           "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /************ CONFIGURE A CASCASE PORT MAPPED TO REMOTE DEVICE ************/

    /* AUTODOC: configure the cascade port */
    PRV_TGF_PORT_CSCD_SET_MAC(TX, prvTgfCscdPortIx,
                              CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);

    /* AUTODOC: map the cascade port to a remote device */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum  = prvTgfPortsArray[prvTgfCscdPortIx];

    prvTgfCscdDevMapTableSet(prvTgfDevNum,
                             PRV_TGF_TARGET_DEV_CNS,
                             CPSS_CPU_PORT_NUM_CNS,
                             &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /*************************** CPU CODE TABLE *******************************/
    /* AUTODOC: set CPU code table entry for EGRESS_MIRRORED_TO_ANALYZER */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.designatedDevNumIndex = PRV_TGF_DEV_NUM_IX_CNS;

    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    PRV_TGF_CPU_CODE_CNS, &cpuCodeEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC: set <Force new DSA to CPU> to 0 */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanForceNewDsaToCpuEnableGet");
}


/**
* @internal prvTgfCscdEgrToCpuConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdEgrToCpuConfigRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC ifaceAn;
    GT_U32                  i;
    GT_STATUS               rc;

    cpssOsMemSet(&ifaceAn, 0, sizeof(ifaceAn));

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: disable packet capturing */
    ifaceAn.type              = CPSS_INTERFACE_PORT_E;
    ifaceAn.devPort.hwDevNum  = prvTgfDevNum;
    ifaceAn.devPort.portNum   = prvTgfPortsArray[prvTgfCscdPortIx];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ifaceAn,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                 ifaceAn.devPort.portNum);

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

    /* AUTODOC: restore forwaring mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: restore an old Analyzer bound to prvTgfAnalyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfAnalyzerIndex,
                                          &prvTgfSavedAnalyzerIface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore a mirroring status of tx port */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx],
                               prvTgfSavedTxMirrorModeEnabled,
                               prvTgfSavedTxMirrorAnalyzerIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

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

    /* AUTODOC: restore the current cascade map table */
    prvTgfCscdDevMapTableSet(prvTgfDevNum,
                             PRV_TGF_TARGET_DEV_CNS,
                             CPSS_CPU_PORT_NUM_CNS,
                             &prvTgfSavedCascadeLink,
                             prvTgfSavedSrcPortTrunkHashEn,
                             GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet");

    /* AUTODOC: restore the CPU Code table entry */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, PRV_TGF_CPU_CODE_CNS,
                                    &prvTgfSavedCpuCodeTableEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC: restore <Force new DSA to CPU> value */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(prvTgfSavedForceNewDsaToCpuEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanForceNewDsaToCpuEnableGet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdEgrToCpuSendPacket function
* @endinternal
*
* @brief   Send the packet
*
* @param[in] packetPtr                - (pointer to) th packet that should be sent
*                                       None
*/
static GT_VOID prvTgfCscdEgrToCpuSendPacket
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
* @internal prvTgfCscdEgrToCpuCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isVlanTagged             - whether the packet is vlan tagged
* @param[in] isRxCascade              - whether rx port is a cascade port
* @param[in] isCpuRemote              - whether a target CPU port is placed on the remote device
*                                       None
*/
static GT_VOID prvTgfCscdEgrToCpuCheckOnPorts
(
    IN GT_BOOL                    isVlanTagged,
    IN GT_BOOL                    isRxCascade,
    IN GT_BOOL                    isCpuRemote
)
{
    GT_U32          expRxCnt;
    GT_U32          expTxCnt;
    TGF_PACKET_STC *expPktPtr;
    CPSS_INTERFACE_INFO_STC iface = {0};
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < PRV_TGF_PORTS_COUNT_CNS; ++i)
    {
        /* init expected counter's values  */
        expRxCnt = 0;
        expTxCnt = 0;
        if (i ==  prvTgfRxPortIx)
        {
            expRxCnt = prvTgfBurstCount;
            expTxCnt = expRxCnt;
        }
        else if (i == prvTgfTxPortIx)
        {
            expTxCnt = (GT_TRUE == isRxCascade) ? 0 : prvTgfBurstCount;
        }
        else if (i == prvTgfCscdPortIx)
        {
            expRxCnt = (GT_TRUE == isCpuRemote) ? prvTgfBurstCount : 0;
            expTxCnt = expRxCnt;
        }

        /* check counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRxCnt, expTxCnt,
                                    0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRxCnt, expTxCnt);
    }

    tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Check trigger counters (captured packets) */
    prvTgfCscdEgrToCpuWriteInDsaTag(&prvTgfDsaTag, isVlanTagged, isRxCascade);

    /* init expected packet and interface values */
    if (GT_TRUE == isCpuRemote)
    {
        iface.type              = CPSS_INTERFACE_PORT_E;
        iface.devPort.hwDevNum  = prvTgfDevNum;
        iface.devPort.portNum   = prvTgfPortsArray[prvTgfCscdPortIx];
        expPktPtr = &prvTgfCscdPacketInfo;

        if (GT_FALSE == isRxCascade)
        {
            /* the device always use ingress packet size for build of
               originByteCount. test always use VLAN tagged packet as ingress one
               - erratum FEr 28. */
            prvTgfDsaTag.dsaInfo.toCpu.originByteCount = TGF_CRC_LEN_CNS +
                    PRV_TGF_NETW_TAG_PACKET_SIZE_CNS;
        }
    }
    else
    {
         /* a packet is intented for local CPU */
        iface.type              = CPSS_INTERFACE_PORT_E;
        iface.devPort.hwDevNum  = (GT_TRUE == isRxCascade ?
                                   PRV_TGF_MIDDLE_DEV_CNS : prvTgfDevNum);
        iface.devPort.portNum   = prvTgfPortsArray[prvTgfTxPortIx];

        expPktPtr = (GT_TRUE == isVlanTagged ?
                     &prvTgfNetwTaggedPacketInfo : &prvTgfNetwUntaggedPacketInfo);
    }

    /* erratum */
    if (prvTgfCscdEgrToCpuErratum(PRV_TGF_ERRATUM_SRC_TRG_E))
    {
        if (GT_FALSE == isRxCascade)
        {
            prvTgfDsaTag.dsaInfo.toCpu.isEgressPipe = GT_FALSE;

            if (GT_FALSE==isVlanTagged) /* case 1, vlan untagged */
            {
                /* The ingress packet is VLAN tagged. The pre-egress (EQ) unit
                   resets srcTrg bit to be Source. So device (HA unit) builds
                   output packet according to ingress packet tagging. */
                if (GT_TRUE == isCpuRemote) /* case 1.2 */
                {
                    prvTgfDsaTag.dsaInfo.toCpu.isTagged = GT_TRUE;
                }
                else
                {
                    expPktPtr = &prvTgfNetwTaggedPacketInfo;
                }
            }
        }
    }

    /* erratum */
    if (prvTgfCscdEgrToCpuErratum(PRV_TGF_ERRATUM_SRC_TRG_TAGGED_A0_E))
    {
        if (GT_TRUE == isRxCascade) /* cases 2,3 */
        {
            if (GT_TRUE == isCpuRemote) /* case 2 */
            {
                prvTgfDsaTag.dsaInfo.toCpu.isTagged = GT_FALSE;
            }
            else                /* case 3 */
            {
                expPktPtr =  &prvTgfNetwUntaggedPacketInfo;
            }
        }
    }

    if(prvUtfIsDoublePhysicalPortsModeUsed() && isRxCascade == GT_TRUE)
    {
        /* the TTI unlike the HA not recognize the 9 bits of srcPhysical port
            in the ingress port ... so it will not be at egress


            NOTE: when TTI will support like HA .... need to bypass this mask.
        */
        prvTgfDsaTag.dsaInfo.toCpu.interface.portNum &= 0xFF;


        if(isCpuRemote == GT_FALSE)
        {
            iface.devPort.portNum  &= 0xFF;
        }
    }

    /* check packet in CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                            &iface,
                            expPktPtr,
                            prvTgfBurstCount,
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


/**
* @internal prvTgfCscdEgrToCpuPrepareSendAndCheck function
* @endinternal
*
* @brief   Make all required preparations and generate traffic
*
* @param[in] isVlanTagged             - whether the packet is vlan tagged
* @param[in] isRxCascade              - whether rx port is a cascade port
* @param[in] isCpuRemote              - whether a target CPU port is placed on the remote device
*                                       None
*/
static GT_VOID prvTgfCscdEgrToCpuPrepareSendAndCheck
(
    IN GT_BOOL                    isVlanTagged,
    IN GT_BOOL                    isRxCascade,
    IN GT_BOOL                    isCpuRemote
)
{
    TGF_PACKET_STC          *pktPtr = NULL;
    CPSS_CSCD_PORT_TYPE_ENT  rxCscdType;
    GT_STATUS                rc;

    /* setup rx port type (cascade or network) */
    rxCscdType = GT_TRUE == isRxCascade ?
        CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E;
    PRV_TGF_PORT_CSCD_SET_MAC(RX, prvTgfRxPortIx, rxCscdType);

    /* AUTODOC: set a device where a target CPU is placed */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(
             prvTgfDevNum, PRV_TGF_DEV_NUM_IX_CNS, (GT_TRUE == isCpuRemote ?
                                                    PRV_TGF_TARGET_DEV_CNS :
                                                    prvTgfSavedHwDevNum));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    /* AUTODOC: initialize a DSA tag of the incoming packet */
    prvTgfCscdEgrToCpuWriteInDsaTag(&prvTgfDsaTag, isVlanTagged, isRxCascade);

    /* AUTODOC: send the packet */

    pktPtr = (GT_TRUE == isRxCascade ? &prvTgfCscdPacketInfo
        : &prvTgfNetwTaggedPacketInfo);

    prvTgfCscdEgrToCpuSendPacket(pktPtr);

    /* AUTODOC: check expected packets on the ports */
    prvTgfCscdEgrToCpuCheckOnPorts(isVlanTagged, isRxCascade, isCpuRemote);
}


/**
* @internal prvTgfCscdEgrToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isVlanTagged             - whether the packet is vlan tagged
*                                       None
*/
static GT_VOID prvTgfCscdEgrToCpuTrafficGenerate
(
    IN GT_BOOL isVlanTagged
)
{
    /* AUTODOC: Generate traffic */

    tgfTrafficGeneratorExpectTraficToCpu(GT_TRUE);

    /* AUTODOC: create vlan with  rx, tx port */
    prvTgfCscdEgrToCpuConfigVlanSetup(prvTgfVlanId, isVlanTagged);

    /* AUTODOC: egress mirrored to local CPU port */
    PRV_UTF_LOG0_MAC("\nCASE 1.1: from network port to local CPU port\n");
    prvTgfCscdEgrToCpuPrepareSendAndCheck(isVlanTagged, GT_FALSE, GT_FALSE);

    /* AUTODOC: egress mirrored to remote CPU port */
    PRV_UTF_LOG0_MAC("\nCASE 1.2: from network port to remote CPU port\n");
    prvTgfCscdEgrToCpuPrepareSendAndCheck(isVlanTagged, GT_FALSE, GT_TRUE);

    /* AUTODOC: from cascade port to remote CPU port */
    PRV_UTF_LOG0_MAC("\nCASE 2: from cascade port to remote CPU port\n");
    prvTgfCscdEgrToCpuPrepareSendAndCheck(isVlanTagged, GT_TRUE, GT_TRUE);

    /* AUTODOC: from cascade port to local CPU port */
    PRV_UTF_LOG0_MAC("\nCASE 3: from cascade port to local CPU port\n");
    prvTgfCscdEgrToCpuPrepareSendAndCheck(isVlanTagged, GT_TRUE, GT_FALSE);
}


/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaEgrToCpuTrgFldsTest function
* @endinternal
*
* @brief   Test checks target info for TO_CPU by Egress Pipe.
*         Following eDSA tag fields checked:
*         Target Tagged / Untagged
*         Target Device
*         Target physical port
*         Target ePort
*/
GT_VOID prvTgfCscdDsaEgrToCpuTrgFldsTest
(
    GT_VOID
)
{
    prvTgfCscdEgrToCpuInitRandomValues();
    prvTgfCscdEgrToCpuConfigSave();
    prvTgfCscdEgrToCpuConfigSet();

    PRV_UTF_LOG0_MAC("\na VLAN tagged outgoing packet\n");
    prvTgfCscdEgrToCpuTrafficGenerate(GT_TRUE);

    PRV_UTF_LOG0_MAC("\na VLAN untagged outgoing packet\n");
    prvTgfCscdEgrToCpuTrafficGenerate(GT_FALSE);

    prvTgfCscdEgrToCpuConfigRestore();
}

