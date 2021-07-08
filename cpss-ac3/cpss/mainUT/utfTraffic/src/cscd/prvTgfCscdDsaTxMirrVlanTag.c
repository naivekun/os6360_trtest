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
* @file prvTgfCscdDsaTxMirrVlanTag.c
*
* @brief Check correctness of eDSA fields <eVLAN>, <CFI>, <UP>, <TPID> of
* of a tx-mirrored packet. There are 3 cases.
* 1) Configure analyzer port to be a cascade port. Send a packets to
* a network rx port. Packets will be forwarded to remote device
* (via FDB entry, a Device Map table and a cascade tx-port). Both
* packets are vlan-tagged. Whether to remove tag will decide
* tx port.
* And will be tx-mirrored to Analyzer port.
* 2) Send a DSA-tagged TO_ANALYZER packets to a cascade rx port.
* Packet should be received on the cascade analyzer port the same
* as it come to rx-port.
* 3) Setup an analyzer port to be a network port and send a DSA-tagged
* TO_ANALYZER packets on cascade rx port. Packets should be received
* on the analyzer port the same as they were sent on rx port
* in case 1.
*
*
* @version   3
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
#include <cscd/prvTgfCscdDsaTxMirrVlanTag.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max vlan id value. Used for random generation of vlan id */
#define PRV_TGF_MAX_VLAN_ID      _4K

#define PRV_TGF_PAYLOAD_LEN_CNS   0x30 /* 48 */

/* amount of TPID values used in the test */
#define PRV_TGF_TPID_COUNT_CNS    8

/* sequential PRV_TGF_TPID_COUNT_CNS values staring from
   PRV_TGF_INITIAL_ETHERTYPE_CNS will be used to fill TPID table */
#define PRV_TGF_INITIAL_ETHERTYPE_CNS (TGF_ETHERTYPE_8100_VLAN_TAG_CNS + 1)


/******************************* TEST PACKETS *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    PRV_TGF_INITIAL_ETHERTYPE_CNS, /* etherType */
    0, 0, 0    /* pri, cfi, vid */ /* will be initialized later */
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

    CPSS_INTERFACE_PORT_E,/* analyzerTrgType */
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
    0 /* tag0TpidIndex */  /* will be filled later */
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
static GT_U32      prvTgfGlobalTxAnalyzerIndex;
static GT_U32      prvTgfTpidArray[PRV_TGF_TPID_COUNT_CNS]; /* TPID table values */

/* some original values prvTgfSaved to be restored at the end of the test. */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT prvTgfSavedMirrMode;
static GT_BOOL                                        prvTgfSavedGlobalMirroringEn;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          prvTgfSavedAnalyzerIface;
static GT_BOOL                                        prvTgfSavedTxMirrorModeEnabled;
static GT_U32                                         prvTgfSavedTxMirrorAnalyzerIdx;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedRxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT                        prvTgfSavedAnCscdPortType;
static GT_HW_DEV_NUM                                  prvTgfSavedHwDevNum;
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT              prvTgfSavedAnEgrTagState;
static PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      prvTgfSavedAnEgrTagStateMode;
static GT_U16                                         prvTgfSavedIngrTpidArray[PRV_TGF_TPID_COUNT_CNS];
static GT_U16                                         prvTgfSavedEgrTpidArray[PRV_TGF_TPID_COUNT_CNS];
static TGF_PRI                                        prvTgfVlanUp;
static TGF_CFI                                        prvTgfVlanCfi;
static GT_U32                                         prvTgfTpidIndex;
static GT_U32                                         prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

/* egress port analyzer index used by the test (lower than 'global' index (prvTgfGlobalTxAnalyzerIndex) */
#define PRV_TGF_EGRESS_PORT_ANALYZER_IDX_CNS 0
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   testMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
static GT_U32   final_analyzerIndex = 0;/*set in runtime*/

/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/*************************** PRIVATE DECLARATIONS *****************************/

/**
* @internal prvTgfCscdIsTxMirrorTpidErratum function
* @endinternal
*
* @brief   Function checks if TPID index not kept erratum is in the tested device.
*         The device with erratum ignores TPID index from incomming eDSA and
*         always takes TPID index from egress ePort.
*
* @retval 1                        - erratum is in the device
* @retval 0                        - there is no erratum in the device
*/
GT_U32 prvTgfCscdIsTxMirrorTpidErratum
(
    GT_U8 devNum
)
{
    GT_U32 isErratumExist = 0; /* state of the erratum */
    devNum = devNum; /* to avoid compilation warnings */

    /* erratum does not exist in simulations */
#ifndef ASIC_SIMULATION
    if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(devNum))
    {
        isErratumExist = 1;
    }
#endif
    return isErratumExist;
}
/**
* @internal prvTgfCscdDsaTxMirrVlanTagConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdDsaTxMirrVlanTagConfigSave
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
                                   prvTgfAnPortIx,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: configure TPID profile 7 to not match any etherType and use it for capturing */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_TRUE, 7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCaptureEnableUnmatchedTpidProfile");

    /* AUTODOC: save default Hw device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: save forwarding mode  */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,
                                                 &prvTgfSavedMirrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* AUTODOC: save global tx Analyzer index value */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
               &prvTgfSavedGlobalMirroringEn, &prvTgfGlobalTxAnalyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet");

    /* AUTODOC: save an old Analyzer bound to prvTgfGlobalTxAnalyzerIndex */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
                                          prvTgfGlobalTxAnalyzerIndex,
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

    /* AUTODOC: save TPID table */
    for (i=0; i < PRV_TGF_TPID_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E, i,
                                       &prvTgfSavedIngrTpidArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanTpidEntryGet, index=%d\n", i);

        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E, i,
                                       &prvTgfSavedEgrTpidArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanTpidEntryGet, index=%d\n", i);

    }

    /* AUTODOC: save <Egress Tag State mode> of the Analyzer port */
     rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                 prvTgfPortsArray[prvTgfAnPortIx],
                                 &prvTgfSavedAnEgrTagStateMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateModeGet");

    /* AUTODOC: save <Egress Tag State> of the Analyzer port */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            prvTgfPortsArray[prvTgfAnPortIx],
                                            &prvTgfSavedAnEgrTagState);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateGet");

}


/**
* @internal prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom function
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
static GT_STATUS prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom
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
        /* check whether the value "new" equal to any values created earlier */
        for (j = 0; j < i && arr[j] != new; j++);
        if (j == i || 0 == i)
        {
            arr[i] = new;
            i++;
        }
    }
    return GT_OK;
}


/**
* @internal prvTgfCscdDsaTxMirrVlanTagInitRandomValues function
* @endinternal
*
* @brief   Initialize tx port, rx port, analyzer port, vlan id, vlan CFI, vlan UP,
*         and TPID array by random values
*/
static GT_VOID prvTgfCscdDsaTxMirrVlanTagInitRandomValues
(
    GT_VOID
)
{
    GT_U32 portIxArr[3];
    GT_STATUS rc;

    /* AUTODOC: initialize rx port, tx port, analyzer port, vlanId and
       PVID (for rx-port) by random values */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* get unique random values for ports (rx, tx, analyzer) indexes */
    rc = prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom(portIxArr, 3, 0, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom");
    prvTgfRxPortIx = portIxArr[0];
    prvTgfTxPortIx = portIxArr[1];
    prvTgfAnPortIx = portIxArr[2];

    /* random vlan for tagged packets */
    prvTgfVlanId = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;

    /* random vlna UP. 3 bits length */
    prvTgfVlanUp = cpssOsRand() % 8;

    /* random vlan CFI. 1 bit length */
    prvTgfVlanCfi = cpssOsRand() % 2;

    /* random TPID index. To choose TPID for vlan tag. */
    prvTgfTpidIndex = cpssOsRand() % PRV_TGF_TPID_COUNT_CNS;

    /*  8 random TPID values. Will be used to fill TPID table */
    rc = prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom(
                         prvTgfTpidArray,
                         PRV_TGF_TPID_COUNT_CNS,
                         PRV_TGF_INITIAL_ETHERTYPE_CNS,
                         PRV_TGF_INITIAL_ETHERTYPE_CNS + PRV_TGF_TPID_COUNT_CNS - 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfCscdDsaTxMirrVlanTagInitArrayByRandom");

    /* display assigned random values */
    PRV_UTF_LOG0_MAC("\n\n***** RANDOMIZED VALUES *****:\n");
    PRV_UTF_LOG1_MAC("Rx port: %d\n", prvTgfPortsArray[prvTgfRxPortIx]);
    PRV_UTF_LOG1_MAC("Tx port: %d\n", prvTgfPortsArray[prvTgfTxPortIx]);
    PRV_UTF_LOG1_MAC("Analyzer port: %d\n",  prvTgfPortsArray[prvTgfAnPortIx]);
    PRV_UTF_LOG1_MAC("vlan id: %d\n", prvTgfVlanId);
    PRV_UTF_LOG1_MAC("vlan UP: %d\n", prvTgfVlanUp);
    PRV_UTF_LOG1_MAC("vlan CFI: %d\n", prvTgfVlanCfi);
    PRV_UTF_LOG2_MAC("TPID index: %d ( 0x%x )\n\n",
                     prvTgfTpidIndex,
                     prvTgfTpidArray[prvTgfTpidIndex]);
}

/**
* @internal prvTgfCscdDsaTxMirrVlanTagConfigureTpidSelectionMode function
* @endinternal
*
* @brief   Configure option <keep original tag0 TPID> of some ePort's
*
* @param[in] isSet                    - set(GT_TRUE) or reset (GT_TRUE) this option
*                                       None
*/
static GT_VOID prvTgfCscdDsaTxMirrVlanTagConfigureTpidSelectionMode
(
    GT_BOOL isSet
)
{
    GT_STATUS rc;
    GT_U32    i;
    PRV_TGF_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT mode;

    mode = GT_TRUE == isSet ?
        PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E:
        PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;

    /* configure all test ports to keep a tag0 TPID from packet's tag rather than
       configure it according to ePort TPID index */
    for (i = 0; i < prvTgfPortsNum; i++)
    {
        rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(
                                prvTgfDevNum, prvTgfPortsArray[i],  CPSS_VLAN_ETHERTYPE0_E, mode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "prvTgfBrgVlanEgressTagTpidSelectModeSet");
    };
}


/**
* @internal prvTgfCscdDsaTxMirrVlanTagConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdDsaTxMirrVlanTagConfigSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC ifaceMir;
    PRV_TGF_BRG_MAC_ENTRY_STC             macEntry;
    GT_U32                                i;
    GT_STATUS                             rc;

    cpssOsMemSet(&ifaceMir,    0, sizeof(ifaceMir));
    cpssOsMemSet(&macEntry,    0, sizeof(macEntry));

    prvTgfCscdDsaTxMirrVlanTagInitRandomValues();

    prvTgfCscdDsaTxMirrVlanTagConfigSave();

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
        /* prvTgfGlobalTxAnalyzerIndex is known after calling :
            prvTgfCscdDsaTxMirrVlanTagConfigSave() */
        testMirrorMode      = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
        final_analyzerIndex = prvTgfGlobalTxAnalyzerIndex;
    }

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */
    /* sniffer port's attributes */
    prvTgfDsaInfo_toAnalyzer.devPort.hwDevNum = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.devPort.portNum  = prvTgfPortsArray[prvTgfTxPortIx];
    prvTgfDsaInfo_toAnalyzer.devPort.ePort    = prvTgfPortsArray[prvTgfTxPortIx];

    /* analyzer port's attribures */
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_TRUE;
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = prvTgfSavedHwDevNum;
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerTrgPort  = prvTgfPortsArray[prvTgfAnPortIx];
    prvTgfDsaInfo_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = prvTgfPortsArray[prvTgfAnPortIx];
    prvTgfDsaInfo_toAnalyzer.tag0TpidIndex = prvTgfTpidIndex;

    /* setup User Priority fields inside expected eDSA tag and inside a
       packet's vlan tag to random value */

    prvTgfPacketVlanTag0Part.etherType = (TGF_ETHER_TYPE)prvTgfTpidArray[prvTgfTpidIndex];
    prvTgfPacketVlanTag0Part.vid       = prvTgfVlanId;
    prvTgfPacketVlanTag0Part.cfi       = prvTgfVlanCfi;
    prvTgfPacketVlanTag0Part.pri       = prvTgfVlanUp;

    prvTgfDsaTag.commonParams.vpt      = prvTgfPacketVlanTag0Part.pri;
    prvTgfDsaTag.commonParams.vid      = prvTgfPacketVlanTag0Part.vid;
    prvTgfDsaTag.commonParams.cfiBit   = prvTgfVlanCfi;
    /* bind the DSA tag ANALYZER part at run time because a union is used */
    prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_toAnalyzer;

    /* AUTODOC: create vlan with all ports with tagging command =
       PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E*/
    rc =   prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfVlanId,
                                 PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: setup a tx analyzer port. Use hop-by-hop forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testMirrorMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorToAnalyzerForwardingModeSet");

    /* assign the analyzer port to the tx analyzer index */
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
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[prvTgfTxPortIx], GT_TRUE, PRV_TGF_EGRESS_PORT_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet");

    /* AUTODOC: setup analyzer port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet",
                                 prvTgfPortsArray[prvTgfAnPortIx]);

    /*------------------------CREATE AN FDB ENTRIES---------------------------*/

    /* AUTODOC: set an FDB entry to send vlan's traffic to tx port */
    macEntry.dstInterface.type             = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[prvTgfTxPortIx];
    macEntry.key.key.macVlan.vlanId        = prvTgfVlanId;
    macEntry.key.entryType                 = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                      = GT_TRUE;
    macEntry.saCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: fill TPID table by random values */
    for (i=0; i < PRV_TGF_TPID_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E, i,
                                       (GT_U16)prvTgfTpidArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanTpidEntrySet, index=%d\n", i);

        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E, i,
                                       (GT_U16)prvTgfTpidArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanTpidEntrySet, index=%d\n", i);
    }

    /* make our packet to be recognized as 'untagged' by the TGF and also
       the CPSS will not override the ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: set ePorts attribute <keep original tag0 TPID> to 1. */
    prvTgfCscdDsaTxMirrVlanTagConfigureTpidSelectionMode(GT_TRUE);

     /* AUTODOC: set <Egress Tag State mode> of the Analyzer port
        to be taken from ePort */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                 prvTgfPortsArray[prvTgfAnPortIx],
                                 PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateModeSet");
    /* AUTODOC: set <Egress Tag State> of the Analyzer port  to be <Tag0 Tagged>*/
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                 prvTgfPortsArray[prvTgfAnPortIx],
                                 PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanEgressPortTagStateSet");
 }

 /*******************************************************************************
 * prvTgfCscdDsaTxMirrVlanTagConfigRestore
 *
 * DESCRIPTION:
 *       Restore configuration
 *
 * INPUTS:
 *       None
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
 static GT_VOID prvTgfCscdDsaTxMirrVlanTagConfigRestore
 (
     GT_VOID
 )
 {
     GT_U32    i;
     GT_STATUS rc;

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

     /* AUTODOC: restore global tx Analyzer index value */
     rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(
                prvTgfSavedGlobalMirroringEn, prvTgfGlobalTxAnalyzerIndex);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet");

     /* AUTODOC: restore an old Analyzer bound to prvTgfGlobalTxAnalyzerIndex */
     rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfGlobalTxAnalyzerIndex,
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

     /* AUTODOC: restore a cascade configuration of the Analyzer port */
     rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                                CPSS_PORT_DIRECTION_TX_E,
                                prvTgfPortsArray[prvTgfAnPortIx],
                                prvTgfSavedAnCscdPortType);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

     /* restore tx port's vlan tag state */

     /* AUTODOC: restore TPID table */
     for (i=0; i < PRV_TGF_TPID_COUNT_CNS; i++)
     {
         rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E, i,
                                        prvTgfSavedIngrTpidArray[i]);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                   "prvTgfBrgVlanTpidEntrySet, index=%d\n", i);

         rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E, i,
                                        prvTgfSavedEgrTpidArray[i]);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                   "prvTgfBrgVlanTpidEntrySet, index=%d\n", i);
     }

     /* AUTODOC: restore a ePorts TPID select mode */
     prvTgfCscdDsaTxMirrVlanTagConfigureTpidSelectionMode(GT_FALSE);

     /* AUTODOC: restore <Egress Tag State mode> of the Analyzer port */
     rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                               prvTgfPortsArray[prvTgfAnPortIx],
                                               prvTgfSavedAnEgrTagStateMode);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanEgressPortTagStateModeSet");

    /* AUTODOC: restore <Egress Tag State> of the Analyzer port */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            prvTgfPortsArray[prvTgfAnPortIx],
                                            prvTgfSavedAnEgrTagState);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                  "prvTgfBrgVlanEgressPortTagStateSet");

     /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: restore content of TPID profile 7 */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_FALSE, 7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCaptureEnableUnmatchedTpidProfile");
 }

 /*******************************************************************************
 * prvTgfCscdDsaTxMirrVlanTagSendPacket
 *
 * DESCRIPTION:
 *       Send the packet
 *
 * INPUTS:
 *       None
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
 static GT_VOID prvTgfCscdDsaTxMirrVlanTagSendPacket
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

 /*******************************************************************************
 * prvTgfCscdDsaTxMirrVlanTagCheckOnPorts
 *
 * DESCRIPTION:
 *       Check counters on the ports.
 *
 * INPUTS:
 *       isSentWithDsa - whether the packet was sent with a DSA-tag.
 *       isExpWithDsa  - whether the packet should be received on the an analyzer
 *                       port with DSA-tag.
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
 static GT_VOID prvTgfCscdDsaTxMirrVlanTagCheckOnPorts
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
     TGF_ETHER_TYPE          saveEtherType = 0;
     GT_U32                  saveTpid = 0;


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
     packetInfoPtr = (GT_TRUE == isExpWithDsa) ? &prvTgfCscdPacketInfo :
                                                 &prvTgfNetwPacketInfo;

     if (prvTgfCscdIsTxMirrorTpidErratum(prvTgfDevNum) && isSentWithDsa)
     {
         /* TPID index is from ePort configuration and it's 0 by default */
         if (GT_TRUE == isExpWithDsa)
         {
             saveTpid = prvTgfDsaTag.dsaInfo.toAnalyzer.tag0TpidIndex;
             prvTgfDsaTag.dsaInfo.toAnalyzer.tag0TpidIndex = 0;
         }
         else
         {
             saveEtherType = prvTgfPacketVlanTag0Part.etherType;
             prvTgfPacketVlanTag0Part.etherType = (TGF_ETHER_TYPE)prvTgfTpidArray[0];
         }
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

     if (prvTgfCscdIsTxMirrorTpidErratum(prvTgfDevNum) && isSentWithDsa)
     {
         /* restore expected results */
         if (GT_TRUE == isExpWithDsa)
         {
             prvTgfDsaTag.dsaInfo.toAnalyzer.tag0TpidIndex = saveTpid;
         }
         else
         {
             prvTgfPacketVlanTag0Part.etherType = saveEtherType;
         }
     }
 }

 /*******************************************************************************
 * prvTgfCscdDsaTxMirrVlanTagSendAndCheck
 *
 * DESCRIPTION:
 *       Send two packets (vlan tagged/untagged)
 *
 * INPUTS:
 *       isSentWithDsa - packet is sent with(GT_TRUE) or without(GT_FALSE)
 *                       DSA tag to rx port
 *       isExpWithDsa  - packet is expected with(GT_TRUE) or without(GT_FALSE)
 *                       DSA tag on the analyzer port
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
 static GT_VOID prvTgfCscdDsaTxMirrVlanTagSendAndCheck
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

     prvTgfCscdDsaTxMirrVlanTagSendPacket();

     prvTgfCscdDsaTxMirrVlanTagCheckOnPorts(isSentWithDsa, isExpWithDsa);
 }


 /*******************************************************************************
 * prvTgfCscdDsaTxMirrVlanTagTrafficGenerate
 *
 * DESCRIPTION:
 *       Generate traffic
 *
 * INPUTS:
 *       None
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
 static GT_VOID prvTgfCscdDsaTxMirrVlanTagTrafficGenerate
 (
     GT_VOID
 )
 {
     GT_STATUS rc;

     /**************************************************************************/
     /* AUTODOC: CASE 1. From network port to cascade port */
     PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");

     /* AUTODOC: send the packet */
     prvTgfCscdDsaTxMirrVlanTagSendAndCheck(GT_FALSE, GT_TRUE);

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

    /* AUTODOC: send the packet */
    prvTgfCscdDsaTxMirrVlanTagSendAndCheck(GT_TRUE, GT_TRUE);

    /**************************************************************************/
    /* AUTODOC: CASE 3 Send a packet from cascade port to network port */

    PRV_UTF_LOG0_MAC("\nCASE 3. From a cascade port to a network port\n");

    /* AUTODOC: setup analyzer port to be a network port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[prvTgfAnPortIx],
                               CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: send the packet */
    prvTgfCscdDsaTxMirrVlanTagSendAndCheck(GT_TRUE, GT_FALSE);
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdDsaTxMirrVlanTagTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrVlanTagTest
(
    GT_VOID
)
{
    prvTgfCscdDsaTxMirrVlanTagConfigSet();
    prvTgfCscdDsaTxMirrVlanTagTrafficGenerate();
    prvTgfCscdDsaTxMirrVlanTagConfigRestore();
}

