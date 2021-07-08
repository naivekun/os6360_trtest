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
* @file prvTgfCncVlanL2L3.c
*
* @brief CPSS DXCH Centralized counters (CNC) Technology facility implementation.
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCosGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

static GT_U16 prvTgfDefVlanId;

/******************************* Test packet **********************************/

/* common parts */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/******************************* IPv4 packet **********************************/

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C9D,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* L2 LENGTH of IP packet 1 */
#define PRV_TGF_IP_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr))

/* L3 LENGTH of IP packet 1 */
#define PRV_TGF_IP_PACKET_L3_LEN_CNS \
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr))

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfIpPacket1Info = {
    PRV_TGF_IP_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/******************************* common payload *******************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

#if PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS
/* VLAN tag1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_CNC_TEST_UP0  /*pri*/,
    PRV_TGF_CNC_TEST_CFI0 /*cfi*/,
    PRV_TGF_CNC_TEST_VID0 /*vid*/
};
#endif /*PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS*/

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
#if PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
#endif /*PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + PRV_TGF_CNC_TEST_VLAN_TAG_SIZE_CNS \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCncTestVidDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfCncTestVidDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: create VLAN 5 with untagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_CNC_TEST_VID0);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], PRV_TGF_CNC_TEST_VID0);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfCncTestVidConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCncTestVidConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_CNC_TEST_VID0);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfCncTestCncBlockConfigure function
* @endinternal
*
* @brief   This function sets configuration of Cnc Block
*
* @param[in] blockNum                 - CNC block number
* @param[in] client                   - CNC client
*                                      valid range see in datasheet of specific device.
* @param[in] enable                   - the client  to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
* @param[in] indexRangesBmp[]         - the counter index ranges bitmap
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestCncBlockConfigure
(
    IN GT_U32                            blockNum,
    IN PRV_TGF_CNC_CLIENT_ENT            client,
    IN GT_BOOL                           enable,
    IN GT_U32                            indexRangesBmp[],
    IN PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
)
{
    GT_STATUS                   rc;
    rc = prvTgfCncBlockClientEnableSet(
        blockNum, client, enable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientEnableSet");

    rc = prvTgfCncBlockClientRangesSet(
        blockNum, client, indexRangesBmp);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");

    /* Lion B and above */
    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E))
    {
        rc = prvTgfCncCounterFormatSet(
            blockNum, format);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterFormatSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestCosReset function
* @endinternal
*
* @brief   This function resets configuration of Cos
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
static GT_STATUS prvTgfCncTestCosReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_QOS_ENTRY_STC             cosEntry;
    PRV_TGF_COS_PROFILE_STC        qosProfile;

    /* Reset packets DP to 0, TC to 0 */

    qosProfile.dropPrecedence = 0;
    qosProfile.trafficClass   = 0;
    qosProfile.dscp           = 0;
    qosProfile.exp            = 0;
    qosProfile.userPriority   = 0;

    if (GT_FALSE != prvUtfDeviceTestNotSupport(
        prvTgfDevNum, UTF_PUMA_E ))
    {
        /* AUTODOC: reset Cos profile table entry 0 */
        rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_TEST_QOS_PROFILE0, &qosProfile);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosProfileEntrySet");

        /* map send port to Qos Profile0 */
        cosEntry.qosProfileId     = 0;
        cosEntry.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        cosEntry.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        cosEntry.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

        /* AUTODOC: reset QoS attributes for port 1 */
        rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], &cosEntry);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosConfigSet");
    }
    else
    {
        /* Puma version */
        rc = prvTgfCosPortDefaultTcSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
            0);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortDefaultTcSet");

        rc = prvTgfCosPortDefaultDpSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
            CPSS_DP_GREEN_E);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortDefaultDpSet");
    }

    /* AUTODOC: reset UP Trust mode for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        CPSS_QOS_PORT_NO_TRUST_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosTrustModeSet");

    if (GT_FALSE != prvUtfDeviceTestNotSupport(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E ))
    {
            /* AUTODOC: reset EXP Trust mode for port 1 */
        rc = prvTgfCosTrustExpModeSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], GT_FALSE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosTrustExpModeSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestCommonReset function
* @endinternal
*
* @brief   This function resets configuration of Cnc
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestCommonReset
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      blockIdx;
    GT_U32                      blockAmount;
    GT_U32                      blockSize;
    GT_U32                      clientIdx;
    PRV_TGF_CNC_CLIENT_ENT      client;
    GT_U32                      clientRange[4];
    GT_U32                      counterIdx;
    PRV_TGF_CNC_COUNTER_STC     counter;
    static PRV_TGF_CNC_CLIENT_ENT      clientArr[] =
    {
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E,
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E,
        PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
        PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
        PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
        PRV_TGF_CNC_CLIENT_EGRESS_PCL_E,
        PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E,
        PRV_TGF_CNC_CLIENT_TUNNEL_START_E,
        PRV_TGF_CNC_CLIENT_TTI_E
    };
    static GT_U32 clientArrSize =
        sizeof(clientArr) / sizeof(clientArr[0]);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfCncTestVidConfigurationRestore();

    prvTgfCncTestCosReset();

    blockAmount = PRV_TGF_CNC_BLOCKS_NUM_MAC();
    blockSize   = PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC();
    clientRange[0] = 0;
    clientRange[1] = 0;
    clientRange[2] = 0;
    clientRange[3] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    for (blockIdx = 0; (blockIdx < blockAmount); blockIdx++)
    {
        for (clientIdx = 0; (clientIdx < clientArrSize); clientIdx++)
        {
            client = clientArr[clientIdx];

            if ((clientIdx >= 10) &&
                (GT_FALSE == prvUtfDeviceTestNotSupport(
                prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E)))
            {
                /* CH3 and Lion A0 */
                break;
            }
            else
            if ((client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E) &&
                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass))
            {
                /* PCL stage is bypassed */
                continue;
            }

            /* AUTODOC: unbind all CNC clients from all counter blocks */
            rc = prvTgfCncBlockClientEnableSet(
                blockIdx, client, GT_FALSE);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientEnableSet");

            /* AUTODOC: reset index ranges for all CNC clients and blocks */
            rc = prvTgfCncBlockClientRangesSet(
                blockIdx, client, clientRange);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");
        }

    }

    /* clear only the bloch used in all tests */
    /* clearing all blocks is very slow       */
    blockIdx = PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC();
    for (counterIdx = 0; (counterIdx < blockSize); counterIdx ++)
    {
        /* AUTODOC: reset CNC counters for all index and blocks */
        rc = prvTgfCncCounterSet(
            blockIdx, counterIdx,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestNotZeroCountersDump function
* @endinternal
*
* @brief   This function Dumps all non zero counters
*/
GT_STATUS prvTgfCncTestNotZeroCountersDump
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      blockIdx;
    GT_U32                      blockAmount;
    GT_U32                      blockSize;
    GT_U32                      counterIdx;
    PRV_TGF_CNC_COUNTER_STC     counter;

    blockAmount = PRV_TGF_CNC_BLOCKS_NUM_MAC();
    blockSize   = PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC();

    for (blockIdx = 0; (blockIdx < blockAmount); blockIdx++)
    {
        for (counterIdx = 0; (counterIdx < blockSize); counterIdx ++)
        {
                /* AUTODOC: reset CNC counters for all index and blocks */
            rc = prvTgfCncCounterGet(
                blockIdx, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterGet");

            if ((counter.packetCount.l[0] != 0) || (counter.byteCount.l[0] != 0))
            {
                PRV_UTF_LOG4_MAC(
                    "CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockIdx, counterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfCncTestVlanL2L3Configure function
* @endinternal
*
* @brief   This function configures Cnc VlanL2L3 tests
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestVlanL2L3Configure
(
    GT_VOID
)
{
    GT_STATUS                      rc;
    GT_U32                         indexRangesBmp[4];
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfCncTestVidDefConfigurationSet();

    /* all ranges relevant to VID map into the block    */
    /* correct both for 512 and for 2048 counter blocks */
    indexRangesBmp[0] = 0xFF;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    /* AUTODOC: bind L2L3_INGRESS_VLAN client to counter block */
    /* AUTODOC: for L2L3_INGRESS_VLAN client set index range to 0xFF */
    /* AUTODOC: set FORMAT_MODE_0 format of CNC counter */
    rc = prvTgfCncTestCncBlockConfigure(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterClearByReadValueSet");

    /* AUTODOC: enable counting for L2L3_INGRESS_VLAN client for port 1 */
    rc = prvTgfCncPortClientEnableSet(
        PRV_TGF_CNC_SEND_PORT_INDEX_CNS,
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncPortClientEnableSet");

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncClientByteCountModeSet");

    return GT_OK;
}

/**
* @internal prvTgfCncTestSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check that counter's packet count is equail to burstCount
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] blockNum                 -  CNC block number
* @param[in] counterIdx               -  index of counter to check
* @param[in] byteCountPattern         -  expecteded byte counter value
*                                      if 0xFFFFFFFF - the check skipped
* @param[in] cncIncrPerPacket         - number of CNC increments per packet
*                                       None
*/
GT_VOID prvTgfCncTestSendPacketAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            burstCount,
    IN GT_U32            blockNum,
    IN GT_U32            counterIdx,
    IN GT_U32            byteCountPattern,
    IN GT_U32            cncIncrPerPacket
)
{
    GT_STATUS                       rc           = GT_OK;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;

    rc = prvTgfCncCounterSet(
        blockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: prvTgfCncCounterSet FAILED, rc = [%d]",
                                     rc);
        return;
    }

    /* reset counters and force links UP */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], burstCount, 0, 0);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);

    rc = prvTgfCncCounterGet(
        blockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(
        (burstCount * cncIncrPerPacket), counter.packetCount.l[0],
        "packet counter blockNum: %d counterIdx %d",
        blockNum, counterIdx);

    if (byteCountPattern != 0xFFFFFFFF)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(
            (byteCountPattern * cncIncrPerPacket), counter.byteCount.l[0],
            "byte counter blockNum: %d counterIdx %d",
            blockNum, counterIdx);
    }
}

/**
* @internal prvTgfCncTestVlanL2L3TrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          burstCount;

    burstCount = 5;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send 5 IPv4 packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfCncTestSendPacketAndCheck(
        &prvTgfIpPacket1Info,
        burstCount,
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_CNC_TEST_VID0,
        /* counting all packet bytes with CRC */
        ((PRV_TGF_IP_PACKET_LEN_CNS + 4) * burstCount), 1);
    /* AUTODOC: verify CNC counters with counterIdx=5 : */
    /* AUTODOC:   byteCount = 320 */
    /* AUTODOC:   packetCount = 5 */

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L3 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncClientByteCountModeSet");

    burstCount = 2;

    /* AUTODOC: send 2 IPv4 packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfCncTestSendPacketAndCheck(
        &prvTgfIpPacket1Info,
        burstCount,
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_CNC_TEST_VID0,
        /* counting L3 packet bytes only */
        (PRV_TGF_IP_PACKET_L3_LEN_CNS * burstCount), 1);
    /* AUTODOC: verify CNC counters with counterIdx=5 : */
    /* AUTODOC:   byteCount = 92 */
    /* AUTODOC:   packetCount = 2 */
}

/**
* @internal prvTgfCncTestSendPacketAndCheckEx function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] blockNum                 -  CNC block number
* @param[in] format                   -  counter format
* @param[in] counterIdx               -  index of counter to check
* @param[in] expectedCounterPtr       -  expecteded counter value
* @param[in] expectedWraparound       - GT_TRUE - expected wraparound
*                                      of the given counter
*                                      GT_FALSE - no wraparound expected
*                                       None
*/
GT_VOID prvTgfCncTestSendPacketAndCheckEx
(
    IN TGF_PACKET_STC                 *packetInfoPtr,
    IN GT_U32                         burstCount,
    IN GT_U32                         blockNum,
    IN PRV_TGF_CNC_COUNTER_FORMAT_ENT format,
    IN GT_U32                         counterIdx,
    IN PRV_TGF_CNC_COUNTER_STC        *initCounterPtr,
    IN PRV_TGF_CNC_COUNTER_STC        *expectedCounterPtr,
    IN GT_BOOL                        expectedWraparound
)
{
    GT_STATUS                       rc           = GT_OK;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32                          indexesArr[32];
    GT_U32                          indexesNum;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* crear wraparond indexes array by read */
    indexesNum = sizeof(indexesArr) / sizeof(indexesArr[0]);
    rc = prvTgfCncCounterWraparoundIndexesGet(
        blockNum, &indexesNum, indexesArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterWraparoundIndexesGet");

    /* Lion B and above */
    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E))
    {
        rc = prvTgfCncCounterFormatSet(
            blockNum, format);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterFormatSet");
    }

    rc = prvTgfCncCounterSet(
        blockNum, counterIdx, format, initCounterPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet");

    /* reset counters and force links UP */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], burstCount, 0, 0);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
        prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS]);

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);

    rc = prvTgfCncCounterGet(
        blockNum, counterIdx, format, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    indexesNum = sizeof(indexesArr) / sizeof(indexesArr[0]);
    rc = prvTgfCncCounterWraparoundIndexesGet(
        blockNum, &indexesNum, indexesArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterWraparoundIndexesGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(
        expectedCounterPtr->packetCount.l[0], counter.packetCount.l[0],
        "packet counter[0] expected: %d received %d",
        expectedCounterPtr->packetCount.l[0], counter.packetCount.l[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC(
        expectedCounterPtr->packetCount.l[1], counter.packetCount.l[1],
        "packet counter[1] expected: %d received %d",
        expectedCounterPtr->packetCount.l[1], counter.packetCount.l[1]);

    UTF_VERIFY_EQUAL2_STRING_MAC(
        expectedCounterPtr->byteCount.l[0], counter.byteCount.l[0],
        "byte counter[0] expected: %d received %d",
        expectedCounterPtr->byteCount.l[0], counter.byteCount.l[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC(
        expectedCounterPtr->byteCount.l[1], counter.byteCount.l[1],
        "byte counter[1] expected: %d received %d",
        expectedCounterPtr->byteCount.l[1], counter.byteCount.l[1]);

    if (expectedWraparound == GT_FALSE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            0, indexesNum,
            "indexesNum expected: 0 received %d", indexesNum);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            1, indexesNum,
            "indexesNum expected: 1 received %d", indexesNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            counterIdx, indexesArr[0],
            "indexesArr[0] expected: %d received %d",
            counterIdx, indexesArr[0]);
    }

}

/**
* @internal prvTgfCncTestVlanL2L3TrafficTestFormats function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficTestFormats
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          i;
    GT_U32                          blockNum;
    GT_U32                          burstCount;
    PRV_TGF_CNC_COUNTER_STC         initCounter[3];
    PRV_TGF_CNC_COUNTER_STC         expectedCounter;
    PRV_TGF_CNC_COUNTER_FORMAT_ENT  format[3] =
    {
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E
    };
    GT_U32  numOfShifts = 0;

    /* AUTODOC: GENERATE TRAFFIC: */
    blockNum = PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC();

    burstCount = 2;

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncClientByteCountModeSet");

    /* counter values (1 << (numOfBits - 1))                    */
    /* For Multi-Port Group devices:                            */
    /* numOfBits by format + 2 to be dispersed on 4 port groups */

    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        numOfShifts = 2;
    }

    /* PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E */
    /* AUTODOC: init CNC counter for formats mode 0 to: */
    /* AUTODOC:   byteCount = 0x10 */
    /* AUTODOC:   packetCount = 0x40000000 */
    initCounter[0].packetCount.l[0] = (1 << (29 - 1 + numOfShifts));
    initCounter[0].packetCount.l[1] = 0;
    initCounter[0].byteCount.l[0] = 0;
    initCounter[0].byteCount.l[1] = (1 << (35 - 32 - 1 + numOfShifts));

    /* PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E */
    /* AUTODOC: init CNC counter for formats mode 1 to: */
    /* AUTODOC:   byteCount = 0x40 */
    /* AUTODOC:   packetCount = 0x10000000 */
    initCounter[1].packetCount.l[0] = (1 << (27 - 1 + numOfShifts));
    initCounter[1].packetCount.l[1] = 0;
    initCounter[1].byteCount.l[0] = 0;
    initCounter[1].byteCount.l[1] = (1 << (37 - 32 - 1 + numOfShifts));

    /* PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E */
    /* AUTODOC: init CNC counter for formats mode 2 to: */
    /* AUTODOC:   byteCount = 0x10000000 */
    /* AUTODOC:   packetCount = 0x40 */
    initCounter[2].packetCount.l[0] = 0;
    initCounter[2].packetCount.l[1] = (1 << (37 - 32 - 1 + numOfShifts));
    initCounter[2].byteCount.l[0] = (1 << (27 - 1 + numOfShifts));
    initCounter[2].byteCount.l[1] = 0;

    for (i = 0; (i < 3); i++)
    {
        expectedCounter = initCounter[i];
        expectedCounter.packetCount.l[0] += burstCount;
        expectedCounter.byteCount.l[0] +=
            ((prvTgfEthOtherPacketInfo.totalLen + 4) * burstCount);

        /* AUTODOC: For CNC counter formats modes 0,1,2 */
        /* AUTODOC: send 2 Ethernet packets from port 1 with: */
        /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
        /* AUTODOC:   EtherType=0x3456 */
        prvTgfCncTestSendPacketAndCheckEx(
            &prvTgfEthOtherPacketInfo,
            burstCount,
            blockNum,
            format[i],
            PRV_TGF_CNC_TEST_VID0,
            &(initCounter[i]),
            &expectedCounter,
            GT_FALSE /*expectedWraparound*/);
        /* AUTODOC: verify CNC counters for CNC format mode 0: */
        /* AUTODOC:   byteCount[0] = 164, byteCount[1] = 16 */
        /* AUTODOC:   packetCount = 0x40000002 */
        /* AUTODOC: verify CNC counters for CNC format mode 1: */
        /* AUTODOC:   byteCount[0] = 164, byteCount[1] = 64 */
        /* AUTODOC:   packetCount = 0x10000002 */
        /* AUTODOC: verify CNC counters for CNC format mode 2: */
        /* AUTODOC:   byteCount[0] = 0x100000A4 */
        /* AUTODOC:   packetCount[0] = 2, packetCount[0] = 0x40 */
    }
}

/**
* @internal prvTgfCncTestVlanL2L3TrafficTestWraparound function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficTestWraparound
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          i;
    GT_U32                          blockNum;
    GT_U32                          burstCount;
    PRV_TGF_CNC_COUNTER_STC         fullCounter;
    PRV_TGF_CNC_COUNTER_STC         initCounter;
    PRV_TGF_CNC_COUNTER_STC         expectedCounter;
    GT_U16                          portGroupNum;

    /* AUTODOC: GENERATE TRAFFIC: */
    blockNum = PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC();

    burstCount = 2;

    /* AUTODOC: for L2L3_INGRESS_VLAN CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncClientByteCountModeSet");

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,PRV_CPSS_DXCH_UNIT_PCL_E,&portGroupNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
               "prvCpssPortGroupsNumActivePortGroupsInBmpGet: %d", prvTgfDevNum);

    /* counter values (1 << numOfBits) - 1) - rize all bits     */
    /* for multi port group devices left shifteed by 2 to be    */
    /* dispersed on 4 port groups by API                        */

    /* AUTODOC: init CNC counter for formats mode 0 to: */
    /* AUTODOC:   byteCount[0] = 0xFFFFFFFF, byteCount[1] = 0x1F */
    /* AUTODOC:   packetCount[0] = 0x7FFFFFFC */

    /* PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E */
    fullCounter.packetCount.l[0] = ((1 << 29) - 1);
    fullCounter.packetCount.l[1] = 0;
    fullCounter.byteCount.l[0] = 0xFFFFFFFF;
    fullCounter.byteCount.l[1] = ((1 << (35 - 32)) - 1);

    /* overlappwed value */
    expectedCounter.packetCount.l[0] = (burstCount - 1);
    expectedCounter.packetCount.l[1] = 0;
    expectedCounter.byteCount.l[0] =
        (((prvTgfEthOtherPacketInfo.totalLen + 4) * burstCount) - 1);
    expectedCounter.byteCount.l[1] = 0;

    initCounter = fullCounter;

    if (portGroupNum > 1)
    {
        /* (portGroupNum - 1) times loop */
        for (i = 1; (i < portGroupNum); i++)
        {
            initCounter.packetCount =
                prvCpssMathAdd64(
                    initCounter.packetCount, fullCounter.packetCount);
            initCounter.byteCount =
                prvCpssMathAdd64(
                    initCounter.byteCount, fullCounter.byteCount);
            expectedCounter.packetCount =
                prvCpssMathAdd64(
                    expectedCounter.packetCount, fullCounter.packetCount);
            expectedCounter.byteCount =
                prvCpssMathAdd64(
                    expectedCounter.byteCount, fullCounter.byteCount);
        }
    }

    /* AUTODOC: send 2 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x3456 */
    prvTgfCncTestSendPacketAndCheckEx(
        &prvTgfEthOtherPacketInfo,
        burstCount,
        blockNum,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
        PRV_TGF_CNC_TEST_VID0,
        &initCounter,
        &expectedCounter,
        GT_TRUE /*expectedWraparound*/);
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount[0] = 160, byteCount[1] = 24 */
    /* AUTODOC:   packetCount[0] = 0x5FFFFFFE */
    /* AUTODOC: verify counter Wrap Around last 8 indexes: */
    /* AUTODOC:   indexNum = 1 */
    /* AUTODOC:   indexesArr = 5 */
}

