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
* @file prvTgfCscdDsaToCpuTrgFlds.c
*
* @brief Test checks target info for TO CPU by Ingress Pipe for Original is Trunk.
* The packet is sent:
* - from a network port to a cascade port
* - from a cascade port to a cascade port
* - from a cascade port to a CPU port
* Following eDSA tag fields checked:
* Source / Target packet data
* Source Tagged / Untagged
* Source Device
* Source physical port
* Orig is Trunk - should be 1
* Source ePort
* Source Trunk
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
#include <common/tgfTrunkGen.h>
#include <common/tgfNetIfGen.h>
#include <common/tgfPortGen.h>
#include <cscd/prvTgfCscdDsaToCpuTrgFlds.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_1_CNS        6

/* traffic generation sending port */
#define ING_NETWORK_PORT_IDX_CNS    0
#define ING_CASCADE_PORT_IDX_CNS    1

/* target port */
#define EGR_CASCADE_PORT_IDX_CNS    2
#define EGR_NETWORK_PORT_IDX_CNS    3
#define EGR_CPU_PORT_IDX_CNS        4

/* max vlan id value. For random generation */
#define PRV_TGF_MAX_VLAN_ID         _4K

/* max trunk ID value. For random generation*/
#define PRV_TGF_MAX_TRUNK_ID        256

/* trunk ID field. Will be initialized later */
static GT_TRUNK_ID prvTgfTrunkId;

/* vlan used by test. Will be initialized later */
static TGF_VLAN_ID prvTgfVlanId;

/* default user priority of Rx port */
static GT_U8       prvTgfRxDefaultUp;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* CPU port number */
#define PRV_TGF_CPU_PORT_NUM_CNS   63

/* Designated device number index */
#define PRV_TGF_DEVICE_NUM_INDEX_CNS    5
/* Designated device number */
#define PRV_TGF_HW_DEVICE_NUMBER_CNS    25

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 0                            /* pri, cfi, VlanId */
};

/********************** "to CPU" DSA tag parameters ***************************/
static TGF_DSA_DSA_TO_CPU_STC  prvTgfPacketDsaTagPart_to_CPU = {
    GT_FALSE ,  /*isEgressPipe*/
    GT_FALSE,   /*isTagged*/
    0,          /*hwDevNum*/
    GT_TRUE,    /* srcIsTrunk */
    {
        0,      /* srcTrunkId */
        0,      /* portNum */
       /* supported in extended, 4 words,  DSA tag */
        0       /* ePort */
    }, /* interface */

    0,         /* cpuCode */
    GT_FALSE,   /* wasTruncated */
    68 ,        /* originByteCount */
    0,          /* timestamp */

    /* supported in extended, 4 words,  DSA tag */
    GT_FALSE,   /* packetIsTT */
    {
        0
    },          /* flowIdTtOffset */

    0           /* tag0TpidIndex */

};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_TO_CPU_E ,/*dsaCommand*/
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

/* EtherType part (for untagged packet) = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof prvTgfPayloadDataArr
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
/* PARTS of network port untagged packet */
static TGF_PACKET_PART_STC prvTgfUntaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of network port untagged packet */
#define PRV_TGF_NETWORK_UNTAGGED_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of network untagged packet with CRC */
#define PRV_TGF_NETWORK_UNTAGGED_PACKET_CRC_LEN_CNS  \
    (PRV_TGF_NETWORK_UNTAGGED_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* UNTAGGED network packet to send */
static TGF_PACKET_STC prvTgfNetworkUntaggedPacketInfo =
{
    PRV_TGF_NETWORK_UNTAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUntaggedPacketPartArray                                        /* partsArray */
};


/* PARTS of network port tagged packet */
static TGF_PACKET_PART_STC prvTgfTagPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of network port tagged packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* TAGGED network packet to send */
static TGF_PACKET_STC prvTgfNetworkTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfTagPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTagPacketPartArray                                        /* partsArray */
};

/* PARTS of cascade port tagged packet */
static TGF_PACKET_PART_STC prvTgfCscdTagPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of cascade port tagged packet */
#define PRV_TGF_CASCADE_TAG_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of cascade tagged packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS  (PRV_TGF_CASCADE_TAG_PACKET_LEN_CNS \
                                                          + TGF_CRC_LEN_CNS)

/* CASCADE tagged packet to send info */
static TGF_PACKET_STC prvTgfCscdTagPacketInfo = {
    PRV_TGF_CASCADE_TAG_PACKET_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfCscdTagPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfCscdTagPacketPartArray                                        /* partsArray */
};

/* PARTS of untagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdUntaggedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH untagged packet came from cascade port */
#define PRV_TGF_CASCADE_UNTAGGED_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS \
  + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of cascade untagged packet with CRC */
#define PRV_TGF_CASCADE_UNTAGGED_PACKET_CRC_LEN_CNS  \
    (PRV_TGF_CASCADE_UNTAGGED_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* CASCADE untagged packet to send info */
static TGF_PACKET_STC prvTgfCscdUntaggedPacketInfo = {
    PRV_TGF_CASCADE_UNTAGGED_PACKET_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfCscdUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfCscdUntaggedPacketPartArray                                        /* partsArray */
};

/********************** "FORAWARD" changed to "to CPU" DSA tag parameters ***************************/
static TGF_DSA_DSA_TO_CPU_STC  prvTgfPacketDsaTagPart_to_CPU_from_FORWARD;


/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/*************************** Restore config ***********************************/
/* Parameters needed to be restored */
static GT_HW_DEV_NUM                     prvTgfSavedHwDevNum;
static TGF_DSA_DSA_TO_CPU_STC            prvTgfSavedPacketDsaTagPart_to_CPU;
static GT_U16                            prvTgfSavedRxPvid;
static GT_U32                            prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

static struct
{
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT     lookupMode;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT    srcPortTrunkHashEn;
} prvTgfDeviceMapTableRestoreCfg;
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
* @internal setPortAsTrunkMember function
* @endinternal
*
* @brief   set/unset port as member of trunk.
*
* @param[in] portNum                  - port index
* @param[in] trunkId                  - trunk ID
* @param[in] enable                   - enable/disable port as member of trunk
*                                       None
*/
static GT_VOID setPortAsTrunkMember
(
    IN GT_U32                   portNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable
)
{
    GT_STATUS               rc;
    CPSS_TRUNK_MEMBER_STC   trunkMembersArray[1] = {{0, 0}};

    /* set trunk members */
    trunkMembersArray[0].hwDevice = prvTgfDevNum;
    trunkMembersArray[0].port     = portNum;

    /* AUTODOC: create trunk 1 with ports [0,1] */

    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    if (GT_TRUE == enable)
    {
        rc = prvTgfTrunkMembersSet(trunkId, 1, trunkMembersArray, 0, NULL);
    }
    else
    {
        rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, trunkId);

}

/**
* @internal prvTgfCscdDsaToCpuTestPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] ingressPortIndex         - ingress port index
*                                       None
*/
static GT_VOID prvTgfCscdDsaToCpuTestPacketSend
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
                                                TGF_CAPTURE_MODE_MIRRORING_E,
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
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
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

static GT_BOOL  forward_To_cpu = GT_FALSE;

/**
* @internal prvTgfCscdDsaToCpuTestCpuCapure function
* @endinternal
*
* @brief   Function check trap packet to CPU while no capture on egress network port
*
* @param[in] packetInfoPtr            - (pointer to) send packet
* @param[in] isTagged                 - whether the packet has vlan tag(s).
*                                       None
*/
static GT_VOID prvTgfCscdDsaToCpuTestCpuCapure
(
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_BOOL                  isTagged
)
{
    GT_STATUS                       rc;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;
    GT_U8                           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                          packetActualLength = 0;
    GT_U8                           devNum;
    GT_U8                           queue;
    TGF_NET_DSA_STC                 rxParam;
    GT_U8                           expectedPacketBuff[256];
    GT_U32                          numOfBytesInBuff = 256;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    TGF_PACKET_DSA_TAG_STC          prvTgfPacketDsaTagPart;
    GT_U32                          ingressPortIdx;
    GT_U32                          expectedByteCount;

    if(forward_To_cpu)
    {
        ingressPortIdx = ING_NETWORK_PORT_IDX_CNS;

        prvTgfPacketDsaTagPart.dsaCommand = TGF_DSA_CMD_TO_CPU_E;
        prvTgfPacketDsaTagPart.dsaType = TGF_DSA_4_WORD_TYPE_E;
        prvTgfPacketDsaTagPart.dsaInfo.toCpu = prvTgfPacketDsaTagPart_to_CPU_from_FORWARD;

    }
    else
    {
        ingressPortIdx = ING_CASCADE_PORT_IDX_CNS;

        /* AUTODOC: save packet DSA tag info */
        cpssOsMemCpy(&prvTgfPacketDsaTagPart,
                     (TGF_PACKET_DSA_TAG_STC *)packetInfoPtr->partsArray[1].partPtr,
                     sizeof(TGF_PACKET_DSA_TAG_STC));

        if(prvUtfIsDoublePhysicalPortsModeUsed())
        {
            /* the TTI unlike the HA not recognize the 9 bits of srcPhysical port
                in the ingress port ... so it will not be at egress


                NOTE: when TTI will support like HA .... need to bypass this mask.
            */
            prvTgfPacketDsaTagPart.dsaInfo.toCpu.interface.portNum &= 0xFF;
        }
    }



    /* AUTODOC: check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (portIter != ingressPortIdx)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                        "Rx another goodPktsRcv counter %d, than expected %d, on port %d",
                                         portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter]);
        }
    }

    if(isTagged)
    {
        /* AUTODOC: check the tagged captured packet */
        packetInfoPtr = &prvTgfNetworkTaggedPacketInfo;
    }
    else
    {
        /* AUTODOC: check the untagged captured packet */
        packetInfoPtr = &prvTgfNetworkUntaggedPacketInfo;
    }

    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetInfoPtr, expectedPacketBuff, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rc,
                                 "tgfTrafficGeneratorRxInCpuGet %d");


    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorRxInCpuGet %d");

    if(rc != GT_OK)
    {
        return;
    }

    /* AUTODOC: check DSA tag  */
    if(rxParam.dsaCmdIsToCpu)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.cpuCode, rxParam.cpuCode,
                                     "DSA Tag CPU code is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.interface.ePort, rxParam.ePortNum,
                                     "DSA Tag ePort is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.interface.srcTrunkId, rxParam.trunkId,
                                     "DSA Tag srcTrunkId is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.interface.portNum, rxParam.portNum,
                                     "DSA Tag portNum is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.hwDevNum, rxParam.hwDevNum,
                                     "DSA Tag hwDevNum is wrong");
#ifdef CHX_FAMILY
        if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
               PRV_CPSS_DXCH_PACKET_ORIG_BYTE_COUNT_WA_E) == GT_TRUE)
        {
            if(isTagged)
            {
                if(forward_To_cpu)
                {
                    expectedByteCount = (prvTgfPacketDsaTagPart.dsaInfo.toCpu.originByteCount + 4);
                }
                else
                {
                    expectedByteCount = (prvTgfPacketDsaTagPart.dsaInfo.toCpu.originByteCount - TGF_eDSA_TAG_SIZE_CNS + 4);
                }
            }
            else
            {
                expectedByteCount = (prvTgfPacketDsaTagPart.dsaInfo.toCpu.originByteCount - TGF_eDSA_TAG_SIZE_CNS);
            }
        }
        else
        {
            expectedByteCount = prvTgfPacketDsaTagPart.dsaInfo.toCpu.originByteCount;
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(expectedByteCount, rxParam.originByteCount,
                                     "DSA Tag originByteCount is wrong");

#endif
        rc = (prvTgfPacketDsaTagPart.dsaInfo.toCpu.srcIsTrunk == rxParam.srcIsTrunk) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDsaTagPart.dsaInfo.toCpu.srcIsTrunk, rxParam.srcIsTrunk,
                                     "DSA Tag srcIsTrunk is wrong");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, rxParam.dsaCmdIsToCpu,
                                     "DSA Tag command is not TO_CPU (rxParam.dsaCmdIsToCpu != GT_TRUE) ");
    }
}

/**
* @internal prvTgfCscdDsaToCpuTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] egressPortIndex          - egress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
* @param[in] isTagged                 - whether the packet has vlan tag(s).
* @param[in] isExpected               - is packet expected on source port
*                                       None
*/
static GT_VOID prvTgfCscdDsaToCpuTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_BOOL                  isTagged,
    IN GT_BOOL                  isExpected
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

    if(forward_To_cpu)
    {
        /* do not change */
    }
    else
    {
        /* bind the DSA tag to CPU part */
        prvTgfPacketDsaTagPart.dsaInfo.toCpu = prvTgfPacketDsaTagPart_to_CPU;
    }

    /* AUTODOC: send packet. */
    prvTgfCscdDsaToCpuTestPacketSend(&portInterface, packetInfoPtr, ingressPortIndex);

    /* AUTODOC check cascade to CPU traffic */
    if(forward_To_cpu ||
      (ingressPortIndex == ING_CASCADE_PORT_IDX_CNS &&
        egressPortIndex == EGR_NETWORK_PORT_IDX_CNS))
    {
        prvTgfCscdDsaToCpuTestCpuCapure(packetInfoPtr, isTagged);
        return;
    }

    /* AUTODOC: check counters */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(egressPortIndex == portIter)
        {
            if(isTagged)
            {
                /* AUTODOC: check the tagged captured packet */
                packetInfoPtr = (EGR_CASCADE_PORT_IDX_CNS == egressPortIndex) ?
                                &prvTgfCscdTagPacketInfo : &prvTgfNetworkTaggedPacketInfo;
            }
            else
            {
                /* AUTODOC: check the untagged captured packet */
                packetInfoPtr = (EGR_CASCADE_PORT_IDX_CNS == egressPortIndex) ?
                                &prvTgfCscdUntaggedPacketInfo : &prvTgfNetworkUntaggedPacketInfo;
            }

            if(prvUtfIsDoublePhysicalPortsModeUsed() &&
               ingressPortIndex == ING_CASCADE_PORT_IDX_CNS)/*came from cascade*/
            {
                /* the TTI unlike the HA not recognize the 9 bits of srcPhysical port
                    in the ingress port ... so it will not be at egress


                    NOTE: when TTI will support like HA .... need to bypass this mask.
                */
                prvTgfPacketDsaTagPart.dsaInfo.toCpu.interface.portNum &= 0xFF;
            }

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
        else if(ING_NETWORK_PORT_IDX_CNS == portIter)
        {
            if(GT_TRUE == isExpected)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
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
}

/**
* @internal prvTgfCscdDsaToCpuTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaToCpuTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS        rc;
    GT_U32            vlanPortsArray[4];
    GT_U8            vlanTagArray[] = {1, 1, 1, 1};
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    CPSS_CSCD_LINK_TYPE_STC     cascadeLink;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   ING_NETWORK_PORT_IDX_CNS,
                                   EGR_CASCADE_PORT_IDX_CNS,
                                   ING_CASCADE_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* save Hardware device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* save DSA tag initial values */
    cpssOsMemCpy(&prvTgfSavedPacketDsaTagPart_to_CPU,
                 &prvTgfPacketDsaTagPart_to_CPU,
                 sizeof(prvTgfPacketDsaTagPart_to_CPU));

    /* AUTODOC: save a PVID of Rx port */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                 &prvTgfSavedRxPvid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet");

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: initialize vlan and trunk by random values */
    prvTgfVlanId  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;
    if (PRV_TGF_MAX_TRUNK_ID <= (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16))
    {
        prvTgfTrunkId = cpssOsRand() % (PRV_TGF_MAX_TRUNK_ID - 1) + 1;
    }
    else
    {
        prvTgfTrunkId = cpssOsRand() % ((GT_TRUNK_ID)(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16) - 1) + 1;
    }


    PRV_UTF_LOG2_MAC("We will use vlanId = %d, trunkId = %d ", prvTgfVlanId, prvTgfTrunkId);

    vlanPortsArray[0] = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    vlanPortsArray[1] = prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS];
    vlanPortsArray[2] = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    vlanPortsArray[3] = prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: create VLAN with ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanId, vlanPortsArray,
                                           NULL, vlanTagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_1_CNS);

    /* AUTODOC: set PVID for Rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                 prvTgfVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* Get default UP of Rx port. This value will be placed by
       device into an eDSA tag for incoming untagged packets. */
    rc = prvTgfCosPortDefaultUpGet(prvTgfDevNum,
                                   prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                   &prvTgfRxDefaultUp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultUpGet");

    prvTgfPacketDsaTagPart_to_CPU.hwDevNum  = prvTgfSavedHwDevNum;
    prvTgfPacketVlanTagPart.vid             = prvTgfVlanId;
    prvTgfPacketDsaTagPart.commonParams.vid = prvTgfVlanId;
    prvTgfPacketDsaTagPart.commonParams.vpt = prvTgfRxDefaultUp;
    prvTgfPacketVlanTagPart.pri             = prvTgfRxDefaultUp;

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_TRUE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_TRUE );

    /* AUTODOC set CPU code table - CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E */
    /* clear entry */
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    entryInfo.designatedDevNumIndex = PRV_TGF_DEVICE_NUM_INDEX_CNS;
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC set CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    PRV_TGF_HW_DEVICE_NUMBER_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                  PRV_TGF_HW_DEVICE_NUMBER_CNS,
                                  0,
                                  &(prvTgfDeviceMapTableRestoreCfg.cascadeLink),
                                  &(prvTgfDeviceMapTableRestoreCfg.srcPortTrunkHashEn));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_HW_DEVICE_NUMBER_CNS);

    /* AUTODOC: assign remote device with egress cascade port (device map table) */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                PRV_TGF_HW_DEVICE_NUMBER_CNS,
                                0,
                                &cascadeLink,
                                0,
                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCscdDevMapTableSet");
}

/**
* @internal prvTgfCscdDsaToCpuTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaToCpuTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore a PVID of Rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                 prvTgfSavedRxPvid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");
    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    /* AUTODOC restore DSA tag initial values */
    cpssOsMemCpy(&prvTgfPacketDsaTagPart_to_CPU,
                 &prvTgfSavedPacketDsaTagPart_to_CPU,
                 sizeof(prvTgfPacketDsaTagPart_to_CPU));

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_FALSE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_FALSE );

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfVlanId);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                  PRV_TGF_HW_DEVICE_NUMBER_CNS,
                                  0,
                                  &(prvTgfDeviceMapTableRestoreCfg.cascadeLink),
                                  prvTgfDeviceMapTableRestoreCfg.srcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_HW_DEVICE_NUMBER_CNS);

    /* clear entry */
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC Reset CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaToCpuFromNetworkToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (network to cascade phase)
*/
static GT_VOID prvTgfCscdDsaToCpuFromNetworkToCascade
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC create static FDB entry with daCmd == PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E */
    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    macEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId   = prvTgfVlanId;
    macEntry.isStatic                 = GT_TRUE;
    macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type        = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[EGR_CPU_PORT_IDX_CNS];

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* set DA command  */
    macEntry.daCommand = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    PRV_UTF_LOG2_MAC(
            "\n*** CASE 1: from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS], prvTgfTrunkId, GT_TRUE);

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.interface.srcTrunkId = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_to_CPU.interface.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_to_CPU.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;

    /* AUTODOC: Send  known network UC tagged packet */
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfNetworkTaggedPacketInfo,
                                       GT_TRUE,
                                       GT_TRUE);

    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.interface.ePort = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: invalidate trunk entry */
    setPortAsTrunkMember(prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS], prvTgfTrunkId, GT_FALSE);

    /* AUTODOC: Send known UC network tagged  packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfNetworkTaggedPacketInfo,
                                       GT_TRUE,
                                       GT_TRUE);

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.originByteCount =
        PRV_TGF_NETWORK_UNTAGGED_PACKET_CRC_LEN_CNS;

    /* AUTODOC: Send known UC network untagged  packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfNetworkUntaggedPacketInfo,
                                       GT_FALSE,
                                       GT_TRUE);


    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);
}

/**
* @internal prvTgfCscdDsaToCpuFromCascadeToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to cascade phase)
*/
static GT_VOID prvTgfCscdDsaToCpuFromCascadeToCascade
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_UTF_LOG2_MAC(
                    "\n*** CASE 2: from Cascade port %02d to Cascade port %02d ***\n",
                    prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                    prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* set Hw device number to be different from srcHwDev in the eDSA tag */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.interface.srcTrunkId = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_to_CPU.interface.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_to_CPU.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
    prvTgfPacketDsaTagPart_to_CPU.originByteCount = PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS;

    /* AUTODOC: Send  unregistered UC tagged packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfCscdTagPacketInfo,
                                       GT_TRUE,
                                       GT_FALSE);

    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.interface.ePort = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: Send unregistered tagged UC packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfCscdTagPacketInfo,
                                       GT_TRUE,
                                       GT_FALSE);

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.originByteCount =
        PRV_TGF_CASCADE_UNTAGGED_PACKET_CRC_LEN_CNS;

    /* AUTODOC: Send known UC untagged packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_CASCADE_PORT_IDX_CNS,
                                       &prvTgfCscdUntaggedPacketInfo,
                                       GT_FALSE,
                                       GT_FALSE);
}

/**
* @internal prvTgfCscdDsaToCpuFromCascadeToCpu function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to CPU port phase)
*/
static GT_VOID prvTgfCscdDsaToCpuFromCascadeToCpu
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    PRV_UTF_LOG2_MAC(
                    "\n*** CASE 3: from Cascade port %02d to CPU port %02d ***\n",
                    prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                    prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS]);

    /* In HIR mode, SW DevNum and Hw DevNum are same. For this test
       SW devNum and HW DevNum should be different.Below line is to
       set srcHwDev in eDSA tag to be different HwDevNum */
    prvTgfPacketDsaTagPart_to_CPU.hwDevNum  = 0x10;

    /* set Hw device number to be different from srcHwDev in the eDSA tag */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    /* AUTODOC: restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                  PRV_TGF_HW_DEVICE_NUMBER_CNS,
                                  0,
                                  &(prvTgfDeviceMapTableRestoreCfg.cascadeLink),
                                  prvTgfDeviceMapTableRestoreCfg.srcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_HW_DEVICE_NUMBER_CNS);

    /* clear entry */
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC Reset CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU.interface.srcTrunkId = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_to_CPU.interface.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_to_CPU.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
    prvTgfPacketDsaTagPart_to_CPU.originByteCount =
        PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS;

    /* AUTODOC: Send unregistered tagged UC packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_NETWORK_PORT_IDX_CNS,
                                       &prvTgfCscdTagPacketInfo,
                                       GT_TRUE,
                                       GT_FALSE);


    prvTgfPacketDsaTagPart_to_CPU.srcIsTrunk = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.interface.srcTrunkId = 0;
    prvTgfPacketDsaTagPart_to_CPU.interface.ePort = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    /* AUTODOC: Send unregistered tagged UC packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_NETWORK_PORT_IDX_CNS,
                                       &prvTgfCscdTagPacketInfo,
                                       GT_TRUE,
                                       GT_FALSE);

    prvTgfPacketDsaTagPart_to_CPU.isTagged = GT_FALSE;
    prvTgfPacketDsaTagPart_to_CPU.originByteCount =
        PRV_TGF_CASCADE_UNTAGGED_PACKET_CRC_LEN_CNS;

    /* AUTODOC: Send known UC untagged packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                       EGR_NETWORK_PORT_IDX_CNS,
                                       &prvTgfCscdUntaggedPacketInfo,
                                       GT_FALSE,
                                       GT_TRUE);
}


/**
* @internal prvTgfCscdDsaToCpuNetworkToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (network to cascade)
*/
GT_VOID prvTgfCscdDsaToCpuNetworkToCascade
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationSet();

    /* AUTODOC: CASE 1: from Network port to Cascade port */
    prvTgfCscdDsaToCpuFromNetworkToCascade();

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationRestore();
}

/**
* @internal prvTgfCscdDsaToCpuCascadeToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to cascade)
*/
GT_VOID prvTgfCscdDsaToCpuCascadeToCascade
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationSet();

    /* AUTODOC: CASE 2: from Cascade port to Cascade port */
    prvTgfCscdDsaToCpuFromCascadeToCascade();

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationRestore();
}

/**
* @internal prvTgfCscdDsaToCpuCascadeToCpu function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to CPU port)
*/
GT_VOID prvTgfCscdDsaToCpuCascadeToCpu
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationSet();

    /* AUTODOC: CASE 3: from Cascade port to CPU port */
    prvTgfCscdDsaToCpuFromCascadeToCpu();

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationRestore();
}

/* local physical port number of remote port.
   Must be in range 0..127 because SIP5 devices support 7 bits for source port in DSA tag */
#define REMOTE_TRUNK_MEMBER_CNS                     101

/********************** "to CPU" DSA tag parameters ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward_2_words = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    0,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        REMOTE_TRUNK_MEMBER_CNS/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0xA,/*devNum*/
            5/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    3,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        19/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};
static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart_for_FARWARD_2_words = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

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

/* PARTS of cascade port tagged packet */
static TGF_PACKET_PART_STC prvTgfCscdTagPacketPartArray_for_FARWARD_2_words [] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart_for_FARWARD_2_words},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of cascade port tagged packet */
#define PRV_TGF_CASCADE_TAG_PACKET_LEN_FARWARD_2_WORDS_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of cascade tagged packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_2_WORDS_LEN_CNS  (PRV_TGF_CASCADE_TAG_PACKET_LEN_FARWARD_2_WORDS_CNS \
                                                          + TGF_CRC_LEN_CNS)

/* CASCADE tagged packet to send info */
static TGF_PACKET_STC prvTgfCscdTagPacketInfo_for_FARWARD_2_words = {
    PRV_TGF_CASCADE_PACKET_CRC_2_WORDS_LEN_CNS,                         /* totalLen */
    sizeof(prvTgfCscdTagPacketPartArray_for_FARWARD_2_words) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfCscdTagPacketPartArray_for_FARWARD_2_words                                        /* partsArray */
};

/**
* @internal prvTgfCscdDsaForwardFromCascadeToCpu function
* @endinternal
*
* @brief   Test for to FORWARD dsa that trap to CPU eDsa tag (cascade to CPU port)
*         but with trunkId on the 'remap on tti'
*/
static GT_VOID prvTgfCscdDsaForwardFromCascadeToCpu
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    GT_U32  ingressPort = ING_NETWORK_PORT_IDX_CNS;
    GT_U32  egressPort  = EGR_NETWORK_PORT_IDX_CNS;
    GT_U32  logical_ingressPort = REMOTE_TRUNK_MEMBER_CNS;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_BOOL     orig_enableOwnDevFlr;
    GT_U32  orig_srcDevLsbAmount, orig_srcPortLsbAmount;

    forward_To_cpu = GT_TRUE;

    prvTgfCscdDsaSrcDevFilterGet(&orig_enableOwnDevFlr);
    prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);

    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum,
                                prvTgfPortsArray[ingressPort], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, prvTgfPortsArray[ingressPort],
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");

    rc = prvTgfPortRemotePhysicalMapAdd(prvTgfDevNum, logical_ingressPort, prvTgfPortsArray[ingressPort]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortRemotePhysicalMapAdd: %d, %d",
                             logical_ingressPort, prvTgfPortsArray[ingressPort]);

    /* set port mapping dsa source lsb amount */
    rc = prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet(
                  &orig_srcDevLsbAmount, &orig_srcPortLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet");

    rc = prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet(0,7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet");

    /* AUTODOC: set ingress port as cascade */
    setIngressPortCascade( prvTgfPortsArray[ingressPort], GT_TRUE );

    /* AUTODOC: disable bypass on cascade port */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,logical_ingressPort,GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set PVID for Rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 logical_ingressPort,
                                 prvTgfVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanId,
                                logical_ingressPort, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanMemberAdd: %d",
                                 prvTgfDevNum);

    /* AUTODOC create static FDB entry with daCmd == PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E */
    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    macEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId   = prvTgfVlanId;
    macEntry.isStatic                 = GT_TRUE;
    macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type        = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[EGR_CPU_PORT_IDX_CNS];

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* set DA command  */
    macEntry.daCommand = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    PRV_UTF_LOG2_MAC(
                    "\n*** CASE 1: from Cascade port %02d to CPU port %02d ***\n",
                    prvTgfPortsArray[ingressPort],
                    prvTgfPortsArray[egressPort]);


    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(logical_ingressPort, prvTgfTrunkId, GT_TRUE);

    prvTgfPacketDsaTagPart_forward_2_words.srcHwDev = (prvTgfDevNum - 1) & 0x3FF;
    prvTgfPacketDsaTagPart_forward_2_words.source.portNum = logical_ingressPort;

    /* bind extended DSA (2 words) to the sent packet */
    prvTgfPacketDsaTagPart_for_FARWARD_2_words.dsaInfo.forward =
        prvTgfPacketDsaTagPart_forward_2_words;


    /* set Hw device number to be different from srcHwDev in the eDSA tag */
/*    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");*/

    /* AUTODOC: restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                  PRV_TGF_HW_DEVICE_NUMBER_CNS,
                                  0,
                                  &(prvTgfDeviceMapTableRestoreCfg.cascadeLink),
                                  prvTgfDeviceMapTableRestoreCfg.srcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_HW_DEVICE_NUMBER_CNS);

    /* clear entry */
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC Reset CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD = prvTgfPacketDsaTagPart_to_CPU;

    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.hwDevNum = prvTgfDevNum;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.isTagged = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.srcIsTrunk = GT_TRUE;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.interface.srcTrunkId = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.interface.portNum = logical_ingressPort;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
    prvTgfPacketDsaTagPart_to_CPU_from_FORWARD.originByteCount =
        PRV_TGF_CASCADE_PACKET_CRC_2_WORDS_LEN_CNS;

    /* AUTODOC: Send unregistered tagged UC packet*/
    prvTgfCscdDsaToCpuTestSendAndCheck(ingressPort,
                                       egressPort,
                                       &prvTgfCscdTagPacketInfo_for_FARWARD_2_words,
                                       GT_TRUE,
                                       GT_FALSE);

    /* AUTODOC: unset Network port 0 as trunk member */
    setPortAsTrunkMember(logical_ingressPort, prvTgfTrunkId, GT_FALSE);

    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,logical_ingressPort,GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set PVID for Rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 logical_ingressPort,
                                 prvTgfSavedRxPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanId,
                                logical_ingressPort);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanMemberAdd: %d",
                                 prvTgfDevNum);

    /* AUTODOC: unset ingress port as cascade */
    setIngressPortCascade( prvTgfPortsArray[ingressPort], GT_FALSE );

    rc = prvTgfPortPhysicalMapDelete(prvTgfDevNum, logical_ingressPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortPhysicalMapDelete");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, prvTgfPortsArray[ingressPort],
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");

    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum,
                                prvTgfPortsArray[ingressPort], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet");

    prvTgfCscdDsaSrcDevFilterSet(orig_enableOwnDevFlr);

    forward_To_cpu = GT_FALSE;

}

/**
* @internal prvTgfCscdDsaForwardCascadeToCpu function
* @endinternal
*
* @brief   Test for to FORWARD dsa that trap to CPU eDsa tag (cascade to CPU port)
*         but with trunkId on the 'remap on tti'
*/
GT_VOID prvTgfCscdDsaForwardCascadeToCpu
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationSet();

    /* AUTODOC: from Cascade port to CPU port */
    prvTgfCscdDsaForwardFromCascadeToCpu();

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaToCpuTestConfigurationRestore();
}

