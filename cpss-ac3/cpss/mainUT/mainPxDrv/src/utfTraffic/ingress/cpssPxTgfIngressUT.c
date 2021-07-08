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
* @file cpssPxTgfIngressUT.c
*
* @brief Ingress tests for tests with traffic for the Pipe devices.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <utfTraffic/ingress/cpssPxTgfIngressIpFiltering.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>

#define DUMMY_PORT                          0xFFFF
#define DUMMY_TYPE                          0xEEEE

#define EXTENDED_PORT_NUM                   2
#define UPLINK_PORT_NUM                     12
#define CASCADE_PORT_NUM                    4
#define CPU_PORT_NUM                        10
#define IPL_PORT_NUM                        8
#define IPL_SRC_PORT_NUM                    12
#define MC_LAG_SRC_PORT_NUM                 15

#define IPL_MC_TARGET_PORT_NUM              0x80000FF0
#define M4_TARGET_PORT_NUM                  0x8000021C
#define M8_TARGET_PORT_NUM                  0x800003FE
#define EXTENDED_EXCEPTION_PORT_NUM         7
#define DST_EXCEPTION_PORT_NUM              8
#define SRC_EXCEPTION_PORT_NUM              9

#define CPU_PACKET_TYPE                     16

#define UPSTREAM_CPU_PACKET_TYPE            28
#define UPSTREAM_CASCADE_PACKET_TYPE        29
#define UPSTREAM_IPV4_TYPE                  30
#define UPSTREAM_NON_IPV4_PACKET_TYPE       31
#define DOWNSTREAM_CASCADE_PACKET_TYPE      12
#define DOWNSTREAM_MC_PACKET_TYPE           13
#define DOWNSTREAM_CPU_PACKET_TYPE          14
#define DOWNSTREAM_UC_MAP_COS_PACKET_TYPE   15
#define DOWNSTREAM_UC_IPL_PACKET_TYPE       17
#define DOWNSTREAM_EVB_TYPE                 19

#define PORT_MAP_ENTRY_MAX_INDEX           _4K - 1
#define SRC_PORT_MAP_ENTRY_MAX_INDEX       _2K - 1
#define CPU_PORT_MAP_ENTRY_INDEX           _4K - 2
#define UPLINK_PORT_MAP_ENTRY_INDEX        _4K - 3
#define DISCARDED_TRAFFIC_ENTRY_INDEX      _4K - 4
#define COS_PORT_MAP_ENTRY_INDEX           _4K - 5
#define IPL_PORT_MAP_ENTRY_INDEX           _4K - 6

#define ETAG_TPID                           0x893f
#define EVB_TAG_TPID                        0x7940
#define VLAN3001 (3001)
#define VLAN3002 (3002)
#define VLAN3003 (3003)
#define VLAN3004 (3004)
#define ECID1001 (1001)
#define ECID1002 (1002)

#define UPSTREAM_PROFILE (64)
#define DOWNSTREAM_PROFILE (65)
#define VLAN_TPID (0x8100)
#define PACKET_TYPE_DOWNSTREAM (31)
#define PACKET_TYPE_UPSTREAM_TAGGED (30)
#define PORT_UPSTREAM  (12)
#define PORT_EXTENDED1 (0)
#define PORT_EXTENDED2 (4)
#define PORT_EXTENDED3 (5)
#define DUMMY_INGRESS_ECID_EXT     (0xab)
#define DUMMY_ECID_EXT             (0xcd)
#define DUMMY_ECID_BASE            (0x111)
#define SRC_FILTERING_TEST_ECID    (0x10000)



static GT_U32 prvTgfIngressLagMcSourcePortNum[] = {0,1,2,3,4,5,6,7,8};
static GT_U32 prvTgfIngressLagMcTargetPortNum[] = {0,0x800003FC, 0x800003FA, 0x800003F6, 0x800003EE, 0x800003DE, 0x800003BE, 0x8000037E, 0x800002FE};

#define PRV_TGF_MEMBERS_NUM_MAC(_x) (sizeof(_x) / sizeof(_x[0]))
#define PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(_PKT, _VLAN) \
do \
{\
    _PKT.partsInfo[3].partBytesPtr[2] = (_VLAN >> 8);\
    _PKT.partsInfo[3].partBytesPtr[3] = (_VLAN & 0xFF);\
} while(0);

#define PRV_TGF_SET_DOWNSTREAM_PKT_ECID(_PKT, _ECID) \
do \
{\
    _PKT.partsInfo[2].partBytesPtr[4] = (_ECID >> 8);\
    _PKT.partsInfo[2].partBytesPtr[5] = (_ECID & 0xFF);\
} while(0);

#define PRV_TGF_SET_UPSTREAM_PKT_VLAN(_PKT, _VLAN) \
do \
{\
    _PKT.partsInfo[2].partBytesPtr[2] = (_VLAN >> 8);\
    _PKT.partsInfo[2].partBytesPtr[3] = (_VLAN & 0xFF);\
} while(0);

#define PRV_TGF_SET_EXPECTED_COUNTERS4(_cntrs, _p1, _v1, _p2, _v2, _p3, _v3, _p4, _v4)\
do                                          \
{                                           \
    cpssOsMemSet(_cntrs, 0, sizeof(_cntrs));\
    _cntrs[0].port = _p1;                   \
    _cntrs[0].value = _v1;                  \
    _cntrs[1].port = _p2;                   \
    _cntrs[1].value = _v2;                  \
    _cntrs[2].port = _p3;                   \
    _cntrs[2].value = _v3;                  \
    _cntrs[3].port = _p4;                   \
    _cntrs[3].value = _v4;                  \
} while(0);

#define PRV_TGF_SET_EXPECTED_COUNTERS3(_cntrs, _p1, _v1, _p2, _v2, _p3, _v3)\
do                                          \
{                                           \
    cpssOsMemSet(_cntrs, 0, sizeof(_cntrs));\
    _cntrs[0].port = _p1;                   \
    _cntrs[0].value = _v1;                  \
    _cntrs[1].port = _p2;                   \
    _cntrs[1].value = _v2;                  \
    _cntrs[2].port = _p3;                   \
    _cntrs[2].value = _v3;                  \
} while(0);

#define PRV_TGF_SET_EXPECTED_COUNTERS2(_cntrs, _p1, _v1, _p2, _v2)\
do                                          \
{                                           \
    cpssOsMemSet(_cntrs, 0, sizeof(_cntrs));\
    _cntrs[0].port = _p1;                   \
    _cntrs[0].value = _v1;                  \
    _cntrs[1].port = _p2;                   \
    _cntrs[1].value = _v2;                  \
} while(0);

#define PRV_TGF_SET_EXPECTED_COUNTERS1(_cntrs, _p1, _v1)\
do                                          \
{                                           \
    cpssOsMemSet(_cntrs, 0, sizeof(_cntrs));\
    _cntrs[0].port = _p1;                   \
    _cntrs[0].value = _v1;                  \
} while(0);

static char buffer[120];

/* Unicast Mac Address Data */
static TGF_MAC_ADDR packetKeyUcMacDaData = {
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04
};

/* Mac Address Exception Data */
static TGF_MAC_ADDR packetKeyUcMacDaExceptionData = {
    0x01, 0x00, 0x11, 0x12, 0x13, 0x14
};

/* Unicast Mac Address Mask*/
static TGF_MAC_ADDR packetKeyUcMacDaMask = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static GT_U8 frameArr[] = {
      0x00, 0x00, 0x01, 0x02, 0x03, 0x04,          /*mac da 00:0x00:0x01:0x02:0x03:0x04*/
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01,          /*mac sa 00:00:00:00:00:01 */
      0x55, 0x55,                                  /*ethertype                */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static GT_U8 frameArr1[] = {
      0x00, 0x00, 0x01, 0x02, 0x03, 0x04,          /*mac da 00:0x00:0x01:0x02:0x03:0x04*/
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01,          /*mac sa 00:00:00:00:00:01 */
      0x55, 0x55,                                  /*ethertype                */
      0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static GT_U8 packetPayload[] =
     {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static GT_U8 macDa[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xEE};
static GT_U8 macSa[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

static GT_U8    eTag[8] = {
    /*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
    /*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
    /*byte 2*/    (GT_U8)(DUMMY_ECID_BASE >> 8),
    /*byte 3*/    (GT_U8)(DUMMY_ECID_BASE & 0xFF),
    /*byte 4*/    (GT_U8)(ECID1001 >> 8),
    /*byte 5*/    (GT_U8)(ECID1001 & 0xFF),
    /*byte 6*/    DUMMY_INGRESS_ECID_EXT,
    /*byte 7*/    DUMMY_ECID_EXT
};
static GT_U8 vlanTag[] = {0x81, 0x00, (VLAN3001 >> 8), VLAN3001 & 0xFF};
static GT_U8 ethType[] = {0x08, 0x00};

static TGF_BYTES_INFO_LIST_STC fromExtendedPort =
{
    5,/*numOfParts*/
    {
        {/* MAC DA */6, macDa},
        {/* MAC SA */6, macSa},
        {/* VLAN TAG */4, vlanTag},
        {/* Inner ethertype */2, ethType},
        {/*Payload*/ 64, packetPayload}
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC fromUpstreamPort =
{
    6,/*numOfParts*/
    {
        {/* MAC DA */6, macDa},
        {/* MAC SA */6, macSa},
        {/* E-TAG */8, eTag},
        {/* VLAN TAG */4, vlanTag},
        {/* Inner ethertype */2, ethType},
        {/*Payload*/ 64, packetPayload}
    }/*partsInfo*/
};

static TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC egressFromUpstream[] =
{
    {PORT_UPSTREAM, {0, {{0}}}} /* We do not capture packets */
};

static TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC egressFromExtended[] =
{
    {PORT_EXTENDED1, {1, {{0}}}}, /* We do not capture packets */
    {PORT_EXTENDED2, {1, {{0}}}},
    {PORT_EXTENDED3, {1, {{0}}}}
};

typedef void (INGRESS_TGF_CONFIG_FUNC)(IN GT_SW_DEV_NUM devNum, 
                                       IN CPSS_PX_PACKET_TYPE  packetType);

/* Ingress traffic info */
typedef struct {
    GT_PHYSICAL_PORT_NUM sourcePortNum;
    GT_PHYSICAL_PORT_NUM targetPortNum;
    CPSS_PX_PACKET_TYPE  packetType;
    INGRESS_TGF_CONFIG_FUNC *prvIngressConfigFuncPtr;
}PX_INGRESS_TGF_INFO_STC;

static GT_VOID prvTgfPxEvbIngressConfig
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType
);

/* Ingress processing information data base */
PX_INGRESS_TGF_INFO_STC prvTgfIngressInfoDb[CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E] =
{
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E */
    {UPLINK_PORT_NUM, EXTENDED_PORT_NUM, DOWNSTREAM_MC_PACKET_TYPE},        /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E */
    {UPLINK_PORT_NUM, CASCADE_PORT_NUM,  DOWNSTREAM_CASCADE_PACKET_TYPE},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E */
    {CASCADE_PORT_NUM, UPLINK_PORT_NUM,  UPSTREAM_CASCADE_PACKET_TYPE},     /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E */
    {UPLINK_PORT_NUM, CPU_PORT_NUM,      DOWNSTREAM_CPU_PACKET_TYPE},       /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E */
    {CPU_PORT_NUM, UPLINK_PORT_NUM,      UPSTREAM_CPU_PACKET_TYPE},         /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, 0},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, 0},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E */
    {UPLINK_PORT_NUM, EXTENDED_PORT_NUM, 0},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E */
    {UPLINK_PORT_NUM, EXTENDED_PORT_NUM, 0},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E */
    {CPU_PORT_NUM, UPLINK_PORT_NUM, 0},        /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, 0},   /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E */
    {UPLINK_PORT_NUM, IPL_PORT_NUM,      DOWNSTREAM_UC_IPL_PACKET_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E */
    {IPL_SRC_PORT_NUM, IPL_MC_TARGET_PORT_NUM,     DOWNSTREAM_MC_PACKET_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM,      UPSTREAM_IPV4_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E */
    {MC_LAG_SRC_PORT_NUM, M4_TARGET_PORT_NUM,     DOWNSTREAM_MC_PACKET_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E */
    {MC_LAG_SRC_PORT_NUM, M8_TARGET_PORT_NUM,     DOWNSTREAM_MC_PACKET_TYPE},    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E */
    {DUMMY_PORT, DUMMY_PORT, DUMMY_TYPE},                                        /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE},                            /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E */
    {UPLINK_PORT_NUM, EXTENDED_PORT_NUM, DOWNSTREAM_EVB_TYPE, prvTgfPxEvbIngressConfig},/* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E */
    {EXTENDED_PORT_NUM, UPLINK_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE}          /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E */

};

/* Ingress processing exceptions */
typedef enum {
    /* Packet type key exceptions */
    PX_INGRESS_EXCEPTION_UDB_E,
    PX_INGRESS_EXCEPTION_TPID_E,
    PX_INGRESS_EXCEPTION_MAC_DA_E,
    PX_INGRESS_EXCEPTION_SRC_PROFILE_E,
    PX_INGRESS_EXCEPTION_LLC_NON_SNAP_E,
    /* Source/Destination port map exception */
    PX_INGRESS_EXCEPTION_SOURCE_PORT_MAP_E,
    PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E,
    /* Extended port wrong packet type */
    PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E
}PX_INGRESS_EXCEPTION_ENT;

GT_STATUS prvTgfPxEgressProcess
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32               extParam,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN GT_PHYSICAL_PORT_NUM sourcePortNum,
    IN GT_PHYSICAL_PORT_NUM targetPortNum,
    IN GT_U8                *frameArrPtr,
    IN GT_U32               frameLength
);

/* Structure keeps original packet type data to restore */
typedef struct {
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyDataOrig[32];
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMaskOrig[32];
    GT_BOOL                                     packetTypeEnableOrig[32];
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC  packetTypeFormatOrig[32][2];
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portsOrig[PRV_CPSS_PX_GOP_PORTS_NUM_CNS];
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntryOrig[32];
/*    CPSS_PX_PORTS_BMP portsBmpOrig[_8K][2]; */
}INGRESS_PACKET_TYPE_ORIG_DATA_STC;

/* Data base with original packet type data */
INGRESS_PACKET_TYPE_ORIG_DATA_STC ingressOrigDataDb;

typedef struct {
    GT_PHYSICAL_PORT_NUM port;
    GT_U32 value;
} PRV_TGF_EXPECTED_COUNTERS_STC;

/**
* @internal prvTgfPxIngressEvbUplinkPortConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream EVB destination packet
*
* @param[in] devNum                   - the device number
* 
*/
static GT_VOID prvTgfPxIngressEvbUplinkPortConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_32  indexConstant;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for uplink port */
    portKey.srcPortProfile = UPLINK_PORT_NUM;
    /* Set UDBP0 */
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* EVB <TPID> */
    portKey.portUdbPairArr[0].udbByteOffset = 12;

    portKey.portUdbPairArr[1].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* EVB <VLAN ID> */
    portKey.portUdbPairArr[1].udbByteOffset = 14;

    st = cpssPxIngressPortPacketTypeKeySet(devNum, UPLINK_PORT_NUM, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


    /* Enable packet type key for EVB downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_EVB_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = EVB_TAG_TPID;
    keyMask.etherType = 0xFFFF;

    /* Set packet type key for EVB downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_EVB_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* EVB packet type destination port map */
    indexConstant = 100;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_EVB_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Signed constant will be added on calculation of destination index */
    entryIndex = indexConstant;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            1 << EXTENDED_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

}

/**
* @internal prvTgfPxEvbIngressExtConfig function
* @endinternal
*
* @brief   Aditional EVB Ingress Processing configuration
*
* @param[in] devNum                   - device number
* @param[in] packetType               - packet type 
* @param[in] resetConfig              - reset configuration flag
*/
static GT_VOID prvTgfPxEvbIngressConfig
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType
)
{
    if (packetType == DOWNSTREAM_EVB_TYPE)
    {
        /* Configure up-link port */
        prvTgfPxIngressEvbUplinkPortConfig(devNum);
    }

    return; 
}

/**
* @internal prvTgfPxIngressCascadePortConfig function
* @endinternal
*
* @brief   Ingress processing configuration for cascade port
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressCascadePortConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    GT_U32 indexConstant;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for cascade port */
    portKey.srcPortProfile = CASCADE_PORT_NUM;
    st = cpssPxIngressPortPacketTypeKeySet(devNum, CASCADE_PORT_NUM, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Enable packet type key for upstream traffic from cascade port*/
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, UPSTREAM_CASCADE_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = CASCADE_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;

    /* Set packet type key for cascade upstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            UPSTREAM_CASCADE_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = UPLINK_PORT_MAP_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    /* Set packet type destination format entry for upstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            UPSTREAM_CASCADE_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << UPLINK_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressCpuPortConfig function
* @endinternal
*
* @brief   Ingress processing configuration for CPU port
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressCpuPortConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    GT_U32 indexConstant;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for CPU port */
    portKey.srcPortProfile = CPU_PORT_NUM;
    st = cpssPxIngressPortPacketTypeKeySet(devNum, CPU_PORT_NUM, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Enable packet type key for upstream traffic from cascade port*/
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, UPSTREAM_CPU_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = CPU_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;

    /* Set packet type key for cascade upstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            UPSTREAM_CPU_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = UPLINK_PORT_MAP_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    /* Set packet type destination format entry for upstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            UPSTREAM_CPU_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << UPLINK_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressExtendedPortConfig function
* @endinternal
*
* @brief   Ingress processing configuration for extended port
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressExtendedPortConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    GT_U32 indexConstant;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Enable packet type key for non-ipv4 upstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, UPSTREAM_NON_IPV4_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = EXTENDED_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    /* Set packet type key for non-ipv4 upstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            UPSTREAM_NON_IPV4_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = UPLINK_PORT_MAP_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    /* Set packet type destination format entry for upstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            UPSTREAM_NON_IPV4_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << UPLINK_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressExceptionPortConfig function
* @endinternal
*
* @brief   Ingress processing configuration for exceptions
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
* @param[in] packetTypeFormatPtr      - (pointer to) packet type format
* @param[in] portKeyPtr               - (pointer to) port packet type key
* @param[in] keyDataPtr               - (pointer to) packet type key format data
* @param[in] keyMaskPtr               - (pointer to) packet type key format mask
*                                       None
*/
static GT_VOID prvTgfPxIngressExceptionPortConfig
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            port,
    IN CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr,
    IN CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr,
    IN CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr,
    IN GT_U32 indexConstant
)
{
    GT_STATUS st;

    st = cpssPxIngressPortPacketTypeKeySet(devNum, port, portKeyPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Enable packet type key for exception traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, port, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


    /* Set packet type key for non-ipv4 upstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            port,
                                            keyDataPtr, keyMaskPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set packet type destination format entry for upstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            port,
                                            packetTypeFormatPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << DST_EXCEPTION_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressUplinkPortDownMcConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream multi destination packet
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownMcConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_32  indexConstant;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    GT_U32  portNum;
    GT_U32  fullPortsBmp = 0;
    GT_U32  fullPortsBmp_excludeMe;
    GT_U32  fullPortsBmp_withCpu;

#define PORT_SKIP_CHECK(dev,port)                                             \
    {                                                                         \
        GT_BOOL     isValid;                                                  \
        st = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum , &isValid); \
        if(st != GT_OK || isValid == GT_FALSE)                                \
        {                                                                     \
            continue;                                                         \
        }                                                                     \
    }

    for (portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        PORT_SKIP_CHECK(devNum, portNum);
        fullPortsBmp |= 1<<portNum;
    }

    /* with the CPU port */
    fullPortsBmp_withCpu = fullPortsBmp | (1 << PRV_CPSS_PX_CPU_DMA_NUM_CNS);
    fullPortsBmp_excludeMe = fullPortsBmp & ~(1<<UPLINK_PORT_NUM);

    /* Enable packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_MC_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;
    keyData.udbPairsArr[1].udb[0] = 0x20;
    keyMask.udbPairsArr[1].udb[0] = 0xF0;

    /* Set packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type destination port map */
    indexConstant = -100;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    /* 2a bf ab */
    packetTypeFormat.bitFieldArr[2].byteOffset = 18;  /*byte value 0xab*/
    packetTypeFormat.bitFieldArr[2].startBit = 3;
    packetTypeFormat.bitFieldArr[2].numBits = 4; /*0x5*/
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;  /*byte value 0x2a*/
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;/*0xa*/
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;  /*byte value 0xBF*/
    packetTypeFormat.bitFieldArr[0].startBit = 3;
    packetTypeFormat.bitFieldArr[0].numBits = 5;/*0x17*/

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = ((0xBF >> 3) & 0x1f) << 0 |
                 ((0x2a >> 0) & 0x0f) << 5 |
                 ((0xab >> 3) & 0x0f) << (5+4);

    /* Signed constant will be added on calculation of destination index */
    entryIndex += indexConstant;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            fullPortsBmp_withCpu, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type source port map */
    indexConstant = 0;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = SRC_PORT_MAP_ENTRY_MAX_INDEX;
    packetTypeFormat.bitFieldArr[1].byteOffset = 14;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;
    packetTypeFormat.bitFieldArr[0].byteOffset = 15;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = DUMMY_ECID_BASE;
    PRV_TGF_LOG1_MAC("SRC entryIndex = 0x%x \n",entryIndex);
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            entryIndex,
                                            fullPortsBmp_excludeMe, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}
/**
* @internal prvTgfPxIngressUplinkPortDownMcConfigGen function
* @endinternal
*
* @brief   Up-link port configuration for downstream multi destination packet
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownMcConfigGen
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            uplinkPort
)
{
    GT_STATUS st;
    GT_32  indexConstant;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    GT_U32  portNum;
    GT_U32  fullPortsBmp = 0;
    GT_U32  fullPortsBmp_excludeMe;
    GT_U32  fullPortsBmp_withCpu;

#define PORT_SKIP_CHECK(dev,port)                                             \
    {                                                                         \
        GT_BOOL     isValid;                                                  \
        st = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum , &isValid); \
        if(st != GT_OK || isValid == GT_FALSE)                                \
        {                                                                     \
            continue;                                                         \
        }                                                                     \
    }

    for (portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        PORT_SKIP_CHECK(devNum, portNum);
        fullPortsBmp |= 1<<portNum;
    }

    /* with the CPU port */
    fullPortsBmp_withCpu = fullPortsBmp | (1 << PRV_CPSS_PX_CPU_DMA_NUM_CNS);
    fullPortsBmp_excludeMe = fullPortsBmp & ~(1<< uplinkPort);

    /* Enable packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_MC_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = uplinkPort;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;
    keyData.udbPairsArr[1].udb[0] = 0x20;
    keyMask.udbPairsArr[1].udb[0] = 0xF0;

    /* Set packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type destination port map */
    indexConstant = -100;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    /* 2a bf ab */
    packetTypeFormat.bitFieldArr[2].byteOffset = 18;  /*byte value 0xab*/
    packetTypeFormat.bitFieldArr[2].startBit = 3;
    packetTypeFormat.bitFieldArr[2].numBits = 4; /*0x5*/
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;  /*byte value 0x2a*/
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;/*0xa*/
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;  /*byte value 0xBF*/
    packetTypeFormat.bitFieldArr[0].startBit = 3;
    packetTypeFormat.bitFieldArr[0].numBits = 5;/*0x17*/

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = ((0xBF >> 3) & 0x1f) << 0 |
                 ((0x2a >> 0) & 0x0f) << 5 |
                 ((0xab >> 3) & 0x0f) << (5+4);

    /* Signed constant will be added on calculation of destination index */
    entryIndex += indexConstant;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            fullPortsBmp_withCpu, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type source port map */
    indexConstant = 0;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = SRC_PORT_MAP_ENTRY_MAX_INDEX;
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;
    packetTypeFormat.bitFieldArr[1].startBit = 1;
    packetTypeFormat.bitFieldArr[1].numBits = 2;
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x1bf;
    PRV_TGF_LOG1_MAC("SRC entryIndex = 0x%x \n",entryIndex);
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            entryIndex,
                                            fullPortsBmp_excludeMe, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}


/**
* @internal prvTgfPxIngressUplinkPortDownCscdConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream traffic to cascade port
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownCscdConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_U32 indexConstant;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    /* Enable packet type key for downstream traffic to cascade port */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_CASCADE_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;
    keyData.udbPairsArr[1].udb[0] = 0x00;
    keyMask.udbPairsArr[1].udb[0] = 0xF0;

    /* Set packet type key for downstream traffic to cascade port */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_CASCADE_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = 1;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;

    /* Set packet type destination format entry for downstream traffic to cascade port */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_CASCADE_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set packet type destination port map entry to cascade port  */
    entryIndex = 0x568;
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            1 << CASCADE_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set packet type destination port map entry to cascade port */
    entryIndex = 0x988;
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            1 << CASCADE_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

}
/**
* @internal prvTgfPxIngressUplinkPortDownUcIplConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream traffic to cascade port
*
* @param[in] devNum                   - the device number
*                                       None
*/
GT_VOID prvTgfPxIngressUplinkPortDownUcIplConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_U32 indexConstant;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    /* Enable packet type key for downstream traffic to cascade port */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_UC_IPL_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = ETAG_TPID;
    keyMask.etherType = 0xFFFF;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;
    keyData.udbPairsArr[1].udb[0] = 0x00;
    keyMask.udbPairsArr[1].udb[0] = 0xF0;

    /* Set packet type key for downstream traffic to cascade port */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_UC_IPL_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = 1;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;

    /* Set packet type destination format entry for downstream traffic to cascade port */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_UC_IPL_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set packet type destination port map entry to cascade port  */
    entryIndex = 0x375;
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            1 << IPL_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


}

/**
* @internal prvTgfPxIngressUplinkPortDownCpuConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream traffic to CPU port
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownCpuConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_U32 indexConstant;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    /* Enable packet type key for downstream traffic to CPU */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_CPU_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;
    keyData.udbPairsArr[1].udb[0] = 0x00;
    keyMask.udbPairsArr[1].udb[0] = 0xF0;

    cpssOsMemCpy(keyData.macDa.arEther, packetKeyUcMacDaData, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(keyMask.macDa.arEther, packetKeyUcMacDaMask, sizeof(TGF_MAC_ADDR));

    /* Set packet type key for downstream traffic to CPU */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_CPU_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = CPU_PORT_MAP_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_CPU_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* CPU packet type destination port map */
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << CPU_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressUplinkPortDownUcCosMapConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream UC CoS mapped traffic
*         to CPU port
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownUcCosMapConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_U32 indexConstant;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;

    /* Enable packet type key for downstream traffic to cascade port */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, DOWNSTREAM_UC_MAP_COS_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&cosFormatEntry, 0, sizeof(cosFormatEntry));

    keyData.profileIndex = UPLINK_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.udbPairsArr[0].udb[0] = 0x20;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;

    cpssOsMemCpy(keyData.macDa.arEther, packetKeyUcMacDaData, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(keyMask.macDa.arEther, packetKeyUcMacDaMask, sizeof(TGF_MAC_ADDR));

    /* Set packet type key for downstream traffic to CPU port */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            DOWNSTREAM_UC_MAP_COS_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = COS_PORT_MAP_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;

    /* Set packet type destination format entry for UC CoS to CPU traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            DOWNSTREAM_UC_MAP_COS_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* CPU packet type destination port map */
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            indexConstant,
                                            1 << CPU_PORT_NUM, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cosFormatEntry.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
    cosFormatEntry.cosByteOffset = 14; /* C-Tag<PCP, DEI> */
    cosFormatEntry.cosNumOfBits = 4;
    cosFormatEntry.cosBitOffset = 4;

    /* Set packet CoS format entry per packet type */
    st = cpssPxCosFormatEntrySet(devNum, DOWNSTREAM_UC_MAP_COS_PACKET_TYPE,
                                 &cosFormatEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressUplinkPortConfig function
* @endinternal
*
* @brief   Up-link port configuration
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for uplink port */
    portKey.srcPortProfile = UPLINK_PORT_NUM;
    /* Set UDBP0 */
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* E-Tag  <E-PCP/E_DEI > */
    portKey.portUdbPairArr[0].udbByteOffset = 14;

    portKey.portUdbPairArr[1].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* E-Tag <GRP> */
    portKey.portUdbPairArr[1].udbByteOffset = 16;

    st = cpssPxIngressPortPacketTypeKeySet(devNum, UPLINK_PORT_NUM, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* MC downstream */
    prvTgfPxIngressUplinkPortDownMcConfig(devNum);
    /* Cascade port downstream */
    prvTgfPxIngressUplinkPortDownCscdConfig(devNum);
    /* CPU port downstream */
    prvTgfPxIngressUplinkPortDownCpuConfig(devNum);
    /* UC CoS mapping downstream */
    prvTgfPxIngressUplinkPortDownUcCosMapConfig(devNum);
}

static GT_VOID prvTgfPxIngressUplinkPortConfigGen
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            uplinkPortNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for uplink port */
    portKey.srcPortProfile = uplinkPortNum;
    /* Set UDBP0 */
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* E-Tag  <E-PCP/E_DEI > */
    portKey.portUdbPairArr[0].udbByteOffset = 14;

    portKey.portUdbPairArr[1].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* E-Tag <GRP> */
    portKey.portUdbPairArr[1].udbByteOffset = 16;

    st = cpssPxIngressPortPacketTypeKeySet(devNum, uplinkPortNum, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* MC downstream */
    prvTgfPxIngressUplinkPortDownMcConfigGen(devNum,uplinkPortNum);

}

/**
* @internal prvTgfPxIngressConfig function
* @endinternal
*
* @brief   General Ingress Processing configuration
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    /* Configure extended ports */
    prvTgfPxIngressExtendedPortConfig(devNum);

    /* Configure up-link port */
    prvTgfPxIngressUplinkPortConfig(devNum);

    /* Configure cascade port */
    prvTgfPxIngressCascadePortConfig(devNum);

    /* Configure CPU port */
    prvTgfPxIngressCpuPortConfig(devNum);
}

/**
* @internal prvTgfPxIngressConfig function
* @endinternal
*
* @brief   General Ingress Processing configuration
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressConfigGen
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            uplinkPort
)
{
    /* Configure extended ports
    prvTgfPxIngressExtendedPortConfig(devNum);*/

    /* Configure up-link port */
    prvTgfPxIngressUplinkPortConfigGen(devNum,uplinkPort);

    /* Configure cascade port
    prvTgfPxIngressCascadePortConfig(devNum);*/

    /* Configure CPU port
    prvTgfPxIngressCpuPortConfig(devNum);*/
}


/**
* @internal prvTgfPxIngressSaveConfig function
* @endinternal
*
* @brief   Save common ingress configuration to allow restore
*
* @param[in] devNum                   - the device number
*                                       None
*/
GT_VOID prvTgfPxIngressSaveConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS           st;
    GT_U32              i;
    CPSS_PX_PACKET_TYPE packetType;
    GT_U32              portNum;

    if (enable == GT_FALSE)
    {
        /* No need to save */
        PRV_TGF_LOG0_MAC("Keep database unchanged\n");
        return;
    }

    /* Save the data */
    PRV_TGF_LOG0_MAC("Save the data in database before changes being made\n");

    for (portNum = 0; portNum < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; portNum++)
    {
        st = cpssPxIngressPortPacketTypeKeyGet(devNum,
                                                portNum,
                                                &ingressOrigDataDb.portsOrig[portNum]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    for (packetType = 0; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
    {
        st = cpssPxIngressPacketTypeKeyEntryGet(devNum,
                                                packetType,
                                                &ingressOrigDataDb.keyDataOrig[packetType], &ingressOrigDataDb.keyMaskOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        st = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,
                                                packetType,
                                                &ingressOrigDataDb.packetTypeEnableOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        for (i = 0; i < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; i++)
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum, i,
                                                packetType,
                                                &ingressOrigDataDb.packetTypeFormatOrig[packetType][i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
        st = cpssPxCosFormatEntryGet(devNum, packetType,
                                     &ingressOrigDataDb.cosFormatEntryOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    /* Disable all packet types */
    for (packetType = 0; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
    {
        if (packetType != CPU_PACKET_TYPE)
        {
            st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
    }
}

/**
* @internal prvTgfPxIngressRestoreConfig function
* @endinternal
*
* @brief   Restore common ingress configuration from database
*
* @param[in] devNum                   - the device number
*                                       None
*/
GT_VOID prvTgfPxIngressRestoreConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS           st;
    GT_U32              i;
    CPSS_PX_PACKET_TYPE packetType;
    GT_U32              portNum;

    if (enable == GT_FALSE)
    {
        /* No need to restore*/
        PRV_TGF_LOG0_MAC("The data need not to be restored\n");
        return;
    }

    /* Restore the data */
    PRV_TGF_LOG0_MAC("Restore the data from database\n");

    for (portNum = 0; portNum < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; portNum++)
    {
        st = cpssPxIngressPortPacketTypeKeySet(devNum,
                                                portNum,
                                                &ingressOrigDataDb.portsOrig[portNum]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    for (packetType = 0; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
    {
        st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                                packetType,
                                                &ingressOrigDataDb.keyDataOrig[packetType], &ingressOrigDataDb.keyMaskOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
                                                packetType,
                                                ingressOrigDataDb.packetTypeEnableOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        for (i = 0; i < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; i++)
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, i,
                                                packetType,
                                                &ingressOrigDataDb.packetTypeFormatOrig[packetType][i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }

        st = cpssPxCosFormatEntrySet(devNum, packetType,
                                                &ingressOrigDataDb.cosFormatEntryOrig[packetType]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    }
}
/**
* @internal prvTgfPxIngressProcess function
* @endinternal
*
* @brief   Generic Ingress Processing function used in tests
*
* @param[in] devNum                   - the device number
* @param[in] haAction                 - HA type used in egress processing
* @param[in] extParam                 - extended parameters used in egress processing
* @param[in] enableSave               - enable/disable save and restore the tests data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxIngressProcess
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32               extParam,
    IN GT_BOOL              enableSave
)
{
    GT_STATUS            rc;
    CPSS_PX_PACKET_TYPE  packetType;
    GT_PHYSICAL_PORT_NUM sourcePortNum;
    GT_PHYSICAL_PORT_NUM targetPortNum;
    GT_U8                *frameArrPtr;
    GT_U32               frameLength;


    /* Taken from DB per current HA action */
    sourcePortNum = prvTgfIngressInfoDb[haAction].sourcePortNum;
    targetPortNum = prvTgfIngressInfoDb[haAction].targetPortNum;
    packetType = prvTgfIngressInfoDb[haAction].packetType;

    /* save configuration to allow restore */
    prvTgfPxIngressSaveConfig(devNum, enableSave);

    if (prvTgfIngressInfoDb[haAction].prvIngressConfigFuncPtr)
    {
        /* Private ingress processing configurations */
        prvTgfIngressInfoDb[haAction].prvIngressConfigFuncPtr(devNum, packetType);
    }
    else
    {
        /* ingress processing basic configurations */
        prvTgfPxIngressConfig(devNum);
    }

    frameArrPtr = NULL; 
    frameLength = 0;

    if (haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E)
    {
        if (packetType == DOWNSTREAM_UC_MAP_COS_PACKET_TYPE)
        {
            frameArrPtr = frameArr1;
            frameLength = sizeof(frameArr1);
        }
        else
        {
            frameArrPtr = frameArr;
            frameLength = sizeof(frameArr);
        }
    }
    rc = prvTgfPxEgressProcess(devNum, haAction, extParam, packetType, sourcePortNum, targetPortNum, frameArrPtr, frameLength);

    /* restore configuration to allow restore */
    prvTgfPxIngressRestoreConfig(devNum, enableSave);

    return rc;
}
static GT_STATUS prvTgfPxIngressProcessLagMc
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32               extParam,
    IN GT_BOOL              enableSave
)
{
    GT_STATUS            rc;
    CPSS_PX_PACKET_TYPE  packetType;
    GT_PHYSICAL_PORT_NUM sourcePortNum;
    GT_PHYSICAL_PORT_NUM targetPortNum;
    GT_U8                *frameArrPtr;
    GT_U32               frameLength;


    /* Taken from DB per current HA action */
    sourcePortNum = prvTgfIngressLagMcSourcePortNum[(extParam & 0xFF)];
    targetPortNum = prvTgfIngressLagMcTargetPortNum[extParam  & 0xFF];
    packetType = prvTgfIngressInfoDb[haAction].packetType;

    /* save configuration to allow restore */
    prvTgfPxIngressSaveConfig(devNum, enableSave);

    /* ingress processing basic configurations */
    prvTgfPxIngressConfigGen(devNum, sourcePortNum);

    frameArrPtr = NULL;
    frameLength = 0;

    rc = prvTgfPxEgressProcess(devNum, haAction, extParam, packetType, sourcePortNum, targetPortNum, frameArrPtr, frameLength);

    /* restore configuration to allow restore */
    prvTgfPxIngressRestoreConfig(devNum, enableSave);

    return rc;
}


static GT_VOID prvTgfPxIngressProcessExtSave
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_BOOL              savePhase
)
{
    GT_STATUS   st;
    static PX_INGRESS_TGF_INFO_STC             ingressTgfInfoSave;
    static CPSS_PX_COS_ATTRIBUTES_STC          cosAttributesSave;
    static CPSS_PORT_EGRESS_CNT_MODE_ENT       setModeBmpSave;
    static GT_PHYSICAL_PORT_NUM                portNumSave;
    static GT_U32                              tcSave;
    static CPSS_DP_LEVEL_ENT                   dpLevelSave;

    if (savePhase)
    {
        /* Taken from DB per current HA action */
        ingressTgfInfoSave = prvTgfIngressInfoDb[haAction];

        /* Get CoS L2 Attributes mapping per Port */
        st = cpssPxCosPortL2MappingGet(devNum, UPLINK_PORT_NUM, 2, &cosAttributesSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* Get configuration of egress counters set 0 */
        st = cpssPxPortEgressCntrModeGet(devNum, 0, &setModeBmpSave,
                                         &portNumSave, &tcSave, &dpLevelSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
    else
    {
        /* Restore configuration of egress counters set 0 */
        st = cpssPxPortEgressCntrModeSet(devNum, 0, setModeBmpSave,
                                         portNumSave, tcSave, dpLevelSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* Restore configuration of CoS L2 Attributes mapping per Port */
        st = cpssPxCosPortL2MappingSet(devNum, UPLINK_PORT_NUM, 2, &cosAttributesSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* Restore ingress info entry */
        prvTgfIngressInfoDb[haAction] = ingressTgfInfoSave;
    }
}

/**
* @internal prvTgfPxIngressCosProcess function
* @endinternal
*
* @brief   Ingress Processing function for CoS attributes mapping
*
* @param[in] devNum                   - the device number
* @param[in] haAction                 - HA type used in egress processing
* @param[in] extParam                 - extended parameters used in egress processing
*                                      enableSave              - enable/disable save and restore the tests data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxIngressCosProcess
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32               extParam,
    IN CPSS_PX_PACKET_TYPE  packetType
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_PX_COS_ATTRIBUTES_STC          cosAttributes;
    CPSS_PORT_EGRESS_CNT_MODE_ENT       setModeBmp;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_U32                              tc;
    CPSS_DP_LEVEL_ENT                   dpLevel;
    CPSS_PX_PORT_EGRESS_CNTR_STC        egrCntr;

    /* Save configuration before starting test */
    prvTgfPxIngressProcessExtSave(devNum, haAction, GT_TRUE);

    /* Set packet type */
    prvTgfIngressInfoDb[haAction].packetType = packetType;

    cosAttributes.userPriority = 3;
    cosAttributes.dropEligibilityIndication = 1;
    cosAttributes.trafficClass = 3;
    /* Drop Precedence CPSS_DP_GREEN_E */
    cosAttributes.dropPrecedence = CPSS_DP_GREEN_E;

    /* Set CoS L2 Attributes mapping per Port */
    rc = cpssPxCosPortL2MappingSet(devNum, UPLINK_PORT_NUM, 2, &cosAttributes);
    if (rc != GT_OK)
    {
        return rc;
    }

    setModeBmp = CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E;
    portNum = CPU_PORT_NUM;
    tc = 3;
    dpLevel = CPSS_DP_GREEN_E;

    /* Set configuration of egress counters set 0  */
    rc = cpssPxPortEgressCntrModeSet(devNum, 0, setModeBmp, portNum, tc, dpLevel);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Call common ingress processing routine */
    rc = prvTgfPxIngressProcess(devNum, haAction, extParam, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read egress counters */
    rc = cpssPxPortEgressCntrsGet(devNum, 0, &egrCntr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Expected one output packet on port = CPU_PORT_NUM with tc = 3 , dpLevel = CPSS_DP_GREEN_E */
    if (egrCntr.outFrames == 0)
    {
        return GT_FAIL;
    }

    /* Drop Precedence CPSS_DP_RED_E */
    cosAttributes.dropPrecedence = CPSS_DP_RED_E;

    /* Set CoS L2 Attributes mapping per Port */
    rc = cpssPxCosPortL2MappingSet(devNum, UPLINK_PORT_NUM, 2, &cosAttributes);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Call common ingress processing routine */
    rc = prvTgfPxIngressProcess(devNum, haAction, extParam, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read egress counters */
    rc = cpssPxPortEgressCntrsGet(devNum, 0, &egrCntr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Expected no output packet on port = CPU_PORT_NUM with tc = 3 , dpLevel = CPSS_DP_RED_E */
    if (egrCntr.outFrames != 0)
    {
        return GT_FAIL;
    }

    /* Restore configuration after finishing test */
    prvTgfPxIngressProcessExtSave(devNum, haAction, GT_FALSE);

    return rc;
}

/**
* @internal prvTgfPxIngressExceptionConfig function
* @endinternal
*
* @brief   Exception configuration
*
* @param[in] devNum                   - the device number
* @param[in] haAction                 - HA type used in egress processing
* @param[in] exceptionType            - exception type to be configured
*                                       None
*/
static void prvTgfPxIngressExceptionConfig
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT    haAction,
    IN PX_INGRESS_EXCEPTION_ENT                     exceptionType
)
{
    GT_STATUS                                   st;
    CPSS_PX_PORTS_BMP                           exceptionPortsBmp;
    GT_U32                                      port;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    GT_U32 indexConstant;


    PX_INGRESS_TGF_INFO_STC exceptionExtendedPort = {EXTENDED_EXCEPTION_PORT_NUM, UPLINK_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE};
    PX_INGRESS_TGF_INFO_STC exceptionPacketType = {EXTENDED_EXCEPTION_PORT_NUM, CPU_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE};
    PX_INGRESS_TGF_INFO_STC exceptionDstIdx = {EXTENDED_EXCEPTION_PORT_NUM, DST_EXCEPTION_PORT_NUM, UPSTREAM_NON_IPV4_PACKET_TYPE};

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* Initial exception port configuration */
    port = EXTENDED_EXCEPTION_PORT_NUM;
    portKey.srcPortProfile = port;
    keyData.profileIndex = portKey.srcPortProfile;
    keyMask.profileIndex = 0x7F;
    indexConstant = DISCARDED_TRAFFIC_ENTRY_INDEX;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;

    switch (exceptionType)
    {
        case PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E:
            /* ETAG packet expected */
            keyData.etherType = ETAG_TPID;
            keyMask.etherType = 0xFFFF;

            /* Replace DB by exception entry */
            prvTgfIngressInfoDb[haAction] = exceptionExtendedPort;

            break;
        case PX_INGRESS_EXCEPTION_MAC_DA_E:
                        /* Wrong MAC DA */
            cpssOsMemCpy(keyData.macDa.arEther, packetKeyUcMacDaExceptionData, sizeof(TGF_MAC_ADDR));
            cpssOsMemCpy(keyMask.macDa.arEther, packetKeyUcMacDaMask, sizeof(TGF_MAC_ADDR));

            /* Replace DB by exception entry */
            prvTgfIngressInfoDb[haAction] = exceptionPacketType;
            break;
        case PX_INGRESS_EXCEPTION_SRC_PROFILE_E:
                        /* Wrong source profile */
            keyData.profileIndex = portKey.srcPortProfile + 1;
            keyMask.profileIndex = 0x7F;

            /* Replace DB by exception entry */
            prvTgfIngressInfoDb[haAction] = exceptionPacketType;
            break;
        case PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E:
                        /* Destination Index exceed maximal */
            packetTypeFormat.indexConst = PORT_MAP_ENTRY_MAX_INDEX - 1;
            packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX - 4;
            /* Destination exception port map */
            exceptionPortsBmp = 1 << DST_EXCEPTION_PORT_NUM;
            st = cpssPxIngressForwardingPortMapExceptionSet(devNum,
                                                        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                                        exceptionPortsBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* Replace DB by exception entry */
            prvTgfIngressInfoDb[haAction] = exceptionDstIdx;

            break;
        default:
            return;
    }

    prvTgfPxIngressExceptionPortConfig(devNum, port,
                                               &packetTypeFormat,
                                               &portKey,
                                               &keyData,
                                               &keyMask,
                                               indexConstant);
}

/**
* @internal prvTgfPxIngressExceptionCheck function
* @endinternal
*
* @brief   Exception results checking
*
* @param[in] devNum                   - the device number
* @param[in] exceptionType            - exception type to be configured
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxIngressExceptionCheck
(
    IN GT_SW_DEV_NUM                                devNum,
    IN PX_INGRESS_EXCEPTION_ENT                     exceptionType
)
{
    GT_STATUS   rc;
    GT_U32                                      regAddr;
    GT_U32                                      errorCounter;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  errorKey;

    cpssOsTimerWkAfter(100);

    if (exceptionType == PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E)
    {
        return GT_OK;
    }

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyLookUpMissCntr;

    switch (exceptionType)
    {
        case PX_INGRESS_EXCEPTION_MAC_DA_E:
            rc = cpssPxIngressPacketTypeErrorGet(devNum, &errorCounter, &errorKey);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (errorCounter == 0)
            {
                PRV_TGF_LOG1_MAC("cpssPxIngressPacketTypeErrorGet[%d] : exception counter is zero\n", exceptionType);
                return GT_FAIL;
            }
            cpssOsSprintf(buffer, "Packet Type Key errorCounter[%d]\nmacDa[%02x:%02x:%02x:%02x:%02x:%02x] - expected macDa[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                          errorCounter,
                          errorKey.macDa.arEther[0],
                          errorKey.macDa.arEther[1],
                          errorKey.macDa.arEther[2],
                          errorKey.macDa.arEther[3],
                          errorKey.macDa.arEther[4],
                          errorKey.macDa.arEther[5],
                          packetKeyUcMacDaExceptionData[0],
                          packetKeyUcMacDaExceptionData[1],
                          packetKeyUcMacDaExceptionData[2],
                          packetKeyUcMacDaExceptionData[3],
                          packetKeyUcMacDaExceptionData[4],
                          packetKeyUcMacDaExceptionData[5]);
            /* Clear counter after read */
            rc  = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PX_INGRESS_EXCEPTION_SRC_PROFILE_E:
            rc = cpssPxIngressPacketTypeErrorGet(devNum, &errorCounter, &errorKey);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (errorCounter == 0)
            {
                PRV_TGF_LOG1_MAC("cpssPxIngressPacketTypeErrorGet[%d] : exception counter is zero\n", exceptionType);
                return GT_FAIL;
            }
            cpssOsSprintf(buffer, "Packet Type Key errorCounter[%d]\nprofileIndex[%d] - expected profileIndex[%d]\n",
                                                  errorCounter,
                          errorKey.profileIndex, (errorKey.profileIndex + 1));
            /* Clear counter after read */
            rc  = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E:
            rc = cpssPxIngressPortMapExceptionCounterGet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, &errorCounter);
            if (errorCounter == 0)
            {
                PRV_TGF_LOG1_MAC("cpssPxIngressPortMapExceptionCounterGet[%d] : exception counter is zero\n", exceptionType);
                return GT_FAIL;
            }
            cpssOsSprintf(buffer, "Destination Idx errorCounter[%d]\n", errorCounter);
            break;
        case PX_INGRESS_EXCEPTION_SOURCE_PORT_MAP_E:
            rc = cpssPxIngressPortMapExceptionCounterGet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, &errorCounter);
            if (errorCounter == 0)
            {
                PRV_TGF_LOG1_MAC("cpssPxIngressPortMapExceptionCounterGet[%d] : exception counter is zero\n", exceptionType);
                return GT_FAIL;
            }
            cpssOsSprintf(buffer, "Source Idx errorCounter[%d]\n", errorCounter);
            break;
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_LOG1_MAC("%s", buffer);

    PRV_TGF_LOG1_MAC("prvTgfPxIngressException[%d] : Test PASSED \n", exceptionType);

    return rc;
}

/**
* @internal prvTgfPxIngressException function
* @endinternal
*
* @brief   Exception processing
*
* @param[in] devNum                   - the device number
* @param[in] haAction                 - HA type used in egress processing
* @param[in] extParam                 - extended parameters used in egress processing
* @param[in] exceptionType            - exception type to be configured
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxIngressException
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT    haAction,
    IN GT_U32                                       extParam,
    IN PX_INGRESS_EXCEPTION_ENT                     exceptionType
)
{
    GT_STATUS            rc;
    PX_INGRESS_TGF_INFO_STC ingressInfoDbOrigEntry;

    ingressInfoDbOrigEntry = prvTgfIngressInfoDb[haAction];

    prvTgfPxIngressSaveConfig(devNum, GT_TRUE);

    /* Configure exception type */
    prvTgfPxIngressExceptionConfig(devNum, haAction, exceptionType);

    /* Ingress processing */
    rc = prvTgfPxIngressProcess(devNum, haAction, extParam, GT_FALSE);
    if (rc != GT_OK)
    {
        rc = prvTgfPxIngressExceptionCheck(devNum, exceptionType);
    }
    else
    {
        PRV_TGF_LOG1_MAC("prvTgfPxIngressException[%d] : Test FAILED \n", exceptionType);
    }

    prvTgfPxIngressRestoreConfig(devNum, GT_TRUE);

    /* Restore DB original entry */
    prvTgfIngressInfoDb[haAction] = ingressInfoDbOrigEntry;

    return rc;
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for ingress processing (E2U - packet unmodified)
 * Based on HA in egress engine.
 *    check case : egress as ingress
 */
UTF_TEST_CASE_MAC(prvTgfPxIngressBasic)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for E2U ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : Extended Port to Upstream Port (802.1br-E2U)
 *    check that egress packet added with proper ETag.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_E2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2E ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : Upstream Port to Extended Port (802.1br-U2E)
 *    check that egress packet Popped the ETAG + vlan tag
 *    mode:The VLAN tag is removed only if the packet's VLAN-ID equal to the
 *    Target Port Table[27:16] - Port VLAN-ID (PVID).
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2E)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
                                    EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
                                    EXT_PARAM_802_1BR_U2E_EGRESS_TAGGED_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
                                    EXT_PARAM_802_1BR_U2E_EGRESS_ALL_UNTAGGED_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
                                    EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_TPID_NO_MATCH_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2C ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : Upstream Port to Cascade Port  (802.1br-U2C)
 *    check that egress packet pop the ETAG (ETAG represents 'target ECID' is CPU of the 'next PE')
 *
 *    NOTE: the 'target ECID' on the ETag is the one of the cascade port.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2C)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,
                                    EXT_PARAM_802_1BR_U2C_EGRESS_KEEP_ETAG_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,
                                    EXT_PARAM_802_1BR_U2C_EGRESS_POP_ETAG_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for C2U ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : Upstream Port to CPU Port  (802.1br-U2CPU)
 *    CPU expected to get packet without ETAG (as ingress from the cascade port)
 *    check that egress packet as ingress (packet egress unchanged - ingress without ETAG)
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_C2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2CPU ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : Upstream Port to CPU Port  (802.1br-U2CPU)
 *    CPU expected to get packet with ETAG that indicates the incoming port number
 *    check that egress packet added with proper ETag.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2CPU)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2CPU ingress processing
 * Based on HA in egress engine.
 *    check for 802.1br : CPU Port to Upstream Port (802.1br-CPU2U)
 *    uplink expected to get packet with/without ETAG from the CPU
 *    check case : uplink get packet with ETAG from the CPU. (egress as ingress)
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_CPU2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,
                                    EXT_PARAM_802_1BR_CPU2U_EGRESS_WITH_ETAG_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for IPL2IPL process
 * Based on HA in egress engine.
 *    check for 802.1br : IPL Port to IPL Port (802.1br-IPL2IPL)
 *    - check that IPL port gets packet form IPL port without any modification
 *    - check that IPL port that is marked in source port table for filtering - doesn't get the packet
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_IPL2IPL)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}


/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2E_M4 process
 * Based on HA in egress engine.
 *    check for 802.1br : multicast M4 Upstream Port to Extend Port (802.1br-U2E_M4)
 *    - check that that if Ingress-ECID Base in the packet e-tag is equal to PCID [1-3] in the HA table,
 *      the packet was not sent to LAG ports.
 *    - check that if Ingress-ECID Base in the packet e-tag is not equal to any LAG PCID
 *      the packet is sent to LAG port.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2E_M4)
{
    GT_STATUS   st;
    GT_U32 srcPortInd;
    GT_U8       dev = 0;


    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (srcPortInd = 1; srcPortInd <=4; srcPortInd++ )
        {
            st = prvTgfPxIngressProcessLagMc(dev,
                                        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E,
                                        srcPortInd,
                                        GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        for (srcPortInd = 1; srcPortInd <= 4; srcPortInd++ )
        {
            st = prvTgfPxIngressProcessLagMc(dev,
                                        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E,
                                        (srcPortInd + SRC_FILTERING_TEST_ECID),
                                        GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2E_M8 process
 * Based on HA in egress engine.
 *    check for 802.1br : multicast M4 Upstream Port to Extend Port (802.1br-U2E_M8)
 *    - check that that if Ingress-ECID Base in the packet e-tag is equal to PCID [1-7] in the HA table,
 *      the packet was not sent to LAG ports.
 *    - check that if Ingress-ECID Base in the packet e-tag is not equal to any LAG PCID
 *      the packet is sent to LAG port.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2E_M8)
{
    GT_STATUS   st;
    GT_U32      srcPortInd;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (srcPortInd = 1; srcPortInd <=8; srcPortInd++ )
        {
            st = prvTgfPxIngressProcessLagMc(dev,
                                        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E,
                                        srcPortInd,
                                        GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        for (srcPortInd = 1; srcPortInd <=8; srcPortInd++ )
        {
            st = prvTgfPxIngressProcessLagMc(dev,
                                        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E,
                                        (srcPortInd + SRC_FILTERING_TEST_ECID),
                                        GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for exceptions in ingress processing.
 * Based on HA in egress engine.
 * The following exceptions are checked:
 *      PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E
 *      PX_INGRESS_EXCEPTION_MAC_DA_E
 *      PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E
 *      PX_INGRESS_EXCEPTION_SRC_PROFILE_E
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_EXCEPTION)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E */
        st = prvTgfPxIngressException(dev,
                                      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
                                      EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS,
                                      PX_INGRESS_EXCEPTION_EXTENDED_PORT_PACKET_TYPE_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* PX_INGRESS_EXCEPTION_MAC_DA_E */
        st = prvTgfPxIngressException(dev,
                                      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E,
                                      EXT_PARAM_DEFAULT_CNS,
                                      PX_INGRESS_EXCEPTION_MAC_DA_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E */
        st = prvTgfPxIngressException(dev,
                                      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,
                                      EXT_PARAM_DEFAULT_CNS,
                                      PX_INGRESS_EXCEPTION_DESTINATION_PORT_MAP_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* PX_INGRESS_EXCEPTION_SRC_PROFILE_E */
        st = prvTgfPxIngressException(dev,
                                      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,
                                      EXT_PARAM_DEFAULT_CNS,
                                      PX_INGRESS_EXCEPTION_SRC_PROFILE_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: basic test for U2CPU ingress processing for CoS mapping packet
 * Based on HA in egress engine.
 *    check for 802.1br : Upstream Port to Cascade Port  (802.1br-U2E)
 *    check that packet egress extended port on proper TC
 *
 *    NOTE: the 'target ECID' on the ETag is the one of the cascade port.
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_U2CPU_COS)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressCosProcess(dev,
                                       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E,
                                       EXT_PARAM_802_1BR_U2C_EGRESS_MODIFIED_COS_ETAG_CNS /*EXT_PARAM_DEFAULT_CNS*/,
                                       DOWNSTREAM_UC_MAP_COS_PACKET_TYPE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/**
* @internal prvTgfPxVlanFilteringCountersCheck function
* @endinternal
*
* @brief   Counters check
*
* @param[in] countersArr[]            - ports and counter values
* @param[in] cntrNum                  - number of elements in cntrNum
*                                       None
*/
static void prvTgfPxVlanFilteringCountersCheck
(
    PRV_TGF_EXPECTED_COUNTERS_STC                     countersArr[],
    GT_U32                                            cntrNum
)
{
    CPSS_PX_PORT_MAC_COUNTER_ENT    counterType;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portCounters;
    GT_U32 ii;
    GT_U32 jj;
    GT_BOOL matched;
    GT_STATUS st;
    GT_SW_DEV_NUM dev;

    dev = 0;
    counterType = CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E;
    for(jj = 0; jj < 16; jj++)
    {
        st = cpssPxPortMacCountersOnPortGet(dev, jj, &portCounters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, jj);
        matched = GT_FALSE;
        for(ii = 0; ii < cntrNum; ii++)
        {
            if(countersArr[ii].port == jj)
            {
                matched = GT_TRUE;
                UTF_VERIFY_EQUAL1_PARAM_MAC(countersArr[ii].value,
                    portCounters.mibCounter[counterType].l[0],
                    countersArr[ii].port);
                break;
            }
        }
        if(GT_FALSE == matched)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(0, portCounters.mibCounter[counterType].l[0], jj);
        }
    }
}

/**
* @internal prvTgfPxVlanFilteringExecute function
* @endinternal
*
* @brief   Execute VLAN filtering test.
*
* @param[in] dev                      - device number
*                                       None
*/
static void prvTgfPxVlanFilteringExecute
(
    GT_U8 dev
)
{
    GT_STATUS   st;
    PRV_TGF_EXPECTED_COUNTERS_STC   expectedCntrsArr[16];

    /* AUTODOC: Packet ingress to extended port 1 with VLAN 3001
        Expect: Egress from upstream port */
    PRV_TGF_SET_UPSTREAM_PKT_VLAN(fromExtendedPort, VLAN3001);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED1,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED1, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to extended port 2 with VLAN 3001
        Expect: Egress from upstream port */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED2,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED2, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to extended port 3 with VLAN 3001
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED3,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED3, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    PRV_TGF_SET_UPSTREAM_PKT_VLAN(fromExtendedPort, VLAN3002);

    /* AUTODOC: Packet ingress to extended port 1 with VLAN 3002
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED1,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED1, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to extended port 2 with VLAN 3002
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED2,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED2, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to extended port 3 with VLAN 3002
        Expect: Egress from upstream port */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED3,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED3, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    PRV_TGF_SET_UPSTREAM_PKT_VLAN(fromExtendedPort, VLAN3003);

    /* AUTODOC: Packet ingress to extended port 1 with VLAN 3003
        Expect: Egress from upstream port */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED1,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED1, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to extended port 2 with VLAN 3003
        Expect: Egress from upstream port */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED2,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED2, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to extended port 3 with VLAN 3003
        Expect: Egress from upstream port */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED3,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_EXTENDED3, 1, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    PRV_TGF_SET_UPSTREAM_PKT_VLAN(fromExtendedPort, VLAN3004);

    /* AUTODOC: Packet ingress to extended port 1 with VLAN 3004
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED1,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED1, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to extended port 2 with VLAN 3004
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED2,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED2, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to extended port 3 with VLAN 3004
        Expect: Filtered */
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_EXTENDED3,
        &fromExtendedPort, 1, egressFromUpstream);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_EXTENDED3, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3001 and ECID 1001
        Expect: Egress from ext. port 1, ext. port 2 */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3001);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1001);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS3(expectedCntrsArr, PORT_UPSTREAM, 1,
        PORT_EXTENDED1, 1, PORT_EXTENDED2, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 3);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3001 and ECID 1002
        Expect: Filtered */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3001);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1002);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3002 and ECID 1001
        Expect: Filtered */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3002);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1001);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3002 and ECID 1002
        Expect: Egress from ext. port 3 */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3002);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1002);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_UPSTREAM, 1,
        PORT_EXTENDED3, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3003 and ECID 1001
        Expect: Egress from ext. port 1, ext. port 2 */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3003);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1001);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS3(expectedCntrsArr, PORT_UPSTREAM, 1,
        PORT_EXTENDED1, 1, PORT_EXTENDED2, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 3);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3003 and ECID 1002
        Expect: Egress from ext. port 3 */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3003);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1002);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS2(expectedCntrsArr, PORT_UPSTREAM, 1,
        PORT_EXTENDED3, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 2);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3004 and ECID 1001
        Expect: Filtered */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3004);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1001);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);

    /* AUTODOC: Packet ingress to upstream port with VLAN 3004 and ECID 1002
        Expect: Filtered */
    PRV_TGF_SET_DOWNSTREAM_PKT_VLAN(fromUpstreamPort, VLAN3004);
    PRV_TGF_SET_DOWNSTREAM_PKT_ECID(fromUpstreamPort, ECID1002);
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(dev, PORT_UPSTREAM,
        &fromUpstreamPort, 3, egressFromExtended);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    PRV_TGF_SET_EXPECTED_COUNTERS1(expectedCntrsArr, PORT_UPSTREAM, 1);
    prvTgfPxVlanFilteringCountersCheck(expectedCntrsArr, 1);
}

/**
* @internal prvTgfPxVlanFilteringConfig function
* @endinternal
*
* @brief   Save or restore VLAN filtering original configuration
*
* @param[in] dev                      - device number
* @param[in] configure                - GT_TRUE - save configuration
*                                      GT_FALSE - restore configuration
*                                       None
*/
static void prvTgfPxVlanFilteringConfig
(
    GT_U8   dev,
    GT_BOOL configure
)
{
    GT_STATUS   st;

    static CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
        upstreamPortPacketTypeUpstreamRestore;
    static CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
        upstreamPortPacketTypeDownstream1Restore;
    static CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
        upstreamPortPacketTypeDownstream2Restore;
    static CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
        upstreamPortPacketTypeDownstream3Restore;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC downstreamKeyDataRestore;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC downstreamKeyMaskRestore;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC upstreamTagKeyDataRestore;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC upstreamTagKeyMaskRestore;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC typeFormatUpstreamTagRestore;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC typeFormatUpstreamTagSrcRestore;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC typeFormatDownstreamRestore;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC typeFormatUpstreamTagDstRestore;
    static CPSS_PX_PORTS_BMP vlan3001BmpRestore;
    static CPSS_PX_PORTS_BMP vlan3002BmpRestore;
    static CPSS_PX_PORTS_BMP vlan3003BmpRestore;
    static CPSS_PX_PORTS_BMP vlan3004BmpRestore;
    static CPSS_PX_PORTS_BMP vlan1001BmpRestore;
    static CPSS_PX_PORTS_BMP vlan1002BmpRestore;
    static CPSS_PX_PORTS_BMP vlan4095BmpRestore;
    static GT_BOOL lagEn3001Restore;
    static GT_BOOL lagEn3002Restore;
    static GT_BOOL lagEn3003Restore;
    static GT_BOOL lagEn3004Restore;
    static GT_BOOL lagEn1001Restore;
    static GT_BOOL lagEn1002Restore;
    static GT_BOOL lagEn4095Restore;
    static GT_BOOL upstreamTaggedEnIngRestore;
    static GT_BOOL downstreamEnIngRestore;
    static GT_BOOL upstreamTaggedEnEgrRestore;
    static GT_BOOL downstreamEnEgrRestore;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC profileConfig;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyMask;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC typeFormat;

    if(GT_TRUE == configure)
    {
        /* AUTODOC: Set up NULL pointers to disable packet capturing */
        egressFromExtended[0].egressFrame.partsInfo[0].partBytesPtr = NULL;
        egressFromExtended[1].egressFrame.partsInfo[0].partBytesPtr = NULL;
        egressFromExtended[2].egressFrame.partsInfo[0].partBytesPtr = NULL;

        /* AUTODOC: Save packet type keys */
        st = cpssPxIngressPortPacketTypeKeyGet(dev, PORT_UPSTREAM,
            &upstreamPortPacketTypeUpstreamRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_UPSTREAM);
        st = cpssPxIngressPortPacketTypeKeyGet(dev, PORT_EXTENDED1,
            &upstreamPortPacketTypeDownstream1Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED1);
        st = cpssPxIngressPortPacketTypeKeyGet(dev, PORT_EXTENDED2,
            &upstreamPortPacketTypeDownstream2Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED2);
        st = cpssPxIngressPortPacketTypeKeyGet(dev, PORT_EXTENDED3,
            &upstreamPortPacketTypeDownstream3Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED3);

        /* AUTODOC: Configure packet type key with VLAN ethertype offset
                    in UDBP0 */
        cpssOsMemSet(&profileConfig, 0, sizeof(profileConfig));
        profileConfig.srcPortProfile = UPSTREAM_PROFILE;
        profileConfig.portUdbPairArr[0].udbAnchorType =
            CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
        profileConfig.portUdbPairArr[0].udbByteOffset = 20;
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_UPSTREAM, &profileConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_UPSTREAM);
        cpssOsMemSet(&profileConfig, 0, sizeof(profileConfig));

        cpssOsMemSet(&profileConfig, 0, sizeof(profileConfig));
        profileConfig.srcPortProfile = DOWNSTREAM_PROFILE;
        profileConfig.portUdbPairArr[0].udbAnchorType =
            CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
        profileConfig.portUdbPairArr[0].udbByteOffset = 12;
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED1, &profileConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED1);
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED2, &profileConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED2);
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED3, &profileConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED3);

        /* AUTODOC: Save packet type key entries */
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, PACKET_TYPE_UPSTREAM_TAGGED,
            &upstreamTagKeyDataRestore, &upstreamTagKeyMaskRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, PACKET_TYPE_DOWNSTREAM,
            &downstreamKeyDataRestore, &downstreamKeyMaskRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Configure packet type entries:
            Pkt type idx  | Src port prof. | Ethertype  | UDBP0
            ----------------------------------------------------------
            Downstream    | Upstream port  |0x893F(ETAG)| 0x8100(VLAN)
            Upstream tag  |Downstream port |0x8100(VLAN)| -
            Upstream notag|Downstream port | -          | -
        */
        cpssOsMemSet(&keyData, 0, sizeof(keyData));
        cpssOsMemSet(&keyMask, 0, sizeof(keyMask));
        keyData.etherType = VLAN_TPID;
        keyMask.etherType = 0xFFFF;
        keyData.profileIndex = DOWNSTREAM_PROFILE;
        keyMask.profileIndex = 0x7F;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, PACKET_TYPE_UPSTREAM_TAGGED,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_UPSTREAM_TAGGED);

        cpssOsMemSet(&keyData, 0, sizeof(keyData));
        cpssOsMemSet(&keyMask, 0, sizeof(keyMask));
        keyData.etherType = ETAG_TPID;
        keyMask.etherType = 0xFFFF;
        keyData.udbPairsArr[0].udb[0] = VLAN_TPID >> 8;
        keyMask.udbPairsArr[0].udb[0] = 0xFF;
        keyData.udbPairsArr[0].udb[1] = VLAN_TPID & 0xFF;
        keyMask.udbPairsArr[0].udb[1] = 0xFF;
        keyData.profileIndex = UPSTREAM_PROFILE;
        keyMask.profileIndex = 0x7F;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, PACKET_TYPE_DOWNSTREAM,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Save type format entry */
        st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormatUpstreamTagRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED);

        st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormatUpstreamTagSrcRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM);

        st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormatUpstreamTagDstRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED);

        st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormatDownstreamRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Configure to upstream direction filtering indexing as VLAN
            number */
        cpssOsMemSet(&typeFormat, 0, sizeof(typeFormat));
        typeFormat.indexMax = 4095;
        typeFormat.indexConst = 0;
        typeFormat.bitFieldArr[0].byteOffset = 15; /* Outer ethertype */
        typeFormat.bitFieldArr[0].startBit   = 0;
        typeFormat.bitFieldArr[0].numBits    = 8;
        typeFormat.bitFieldArr[1].byteOffset = 14;
        typeFormat.bitFieldArr[1].startBit   = 0;
        typeFormat.bitFieldArr[1].numBits    = 4;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormat);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED);

        /* AUTODOC: Configure to upstream direction destination port table
            indexing */
        cpssOsMemSet(&typeFormat, 0, sizeof(typeFormat));
        typeFormat.indexMax = 8191;
        typeFormat.indexConst = 4095;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormat);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED);

        /* AUTODOC: Configure from upstream direction destination map indexing
            as ECID number */
        cpssOsMemSet(&typeFormat, 0, sizeof(typeFormat));
        typeFormat.indexMax   = 4095;
        typeFormat.indexConst = 0;
        typeFormat.bitFieldArr[0].byteOffset = 17; /* E-CID */
        typeFormat.bitFieldArr[0].startBit   = 0;
        typeFormat.bitFieldArr[0].numBits    = 8;
        typeFormat.bitFieldArr[1].byteOffset = 16;
        typeFormat.bitFieldArr[1].startBit   = 0;
        typeFormat.bitFieldArr[1].numBits    = 8;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormat);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Configure from upstream direction port filtering indexing as
            VLAN number */
        cpssOsMemSet(&typeFormat, 0, sizeof(typeFormat));
        typeFormat.indexMax   = 4095;
        typeFormat.indexConst = 0;
        typeFormat.bitFieldArr[0].byteOffset = 23; /* VLAN */
        typeFormat.bitFieldArr[0].startBit   = 0;
        typeFormat.bitFieldArr[0].numBits    = 8;
        typeFormat.bitFieldArr[1].byteOffset = 22;
        typeFormat.bitFieldArr[1].startBit   = 0;
        typeFormat.bitFieldArr[1].numBits    = 8;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormat);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Save port filtering table entries */
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001,
            &vlan3001BmpRestore, &lagEn3001Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001);
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002,
            &vlan3002BmpRestore, &lagEn3002Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002);
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003,
            &vlan3003BmpRestore, &lagEn3003Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003);
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004,
            &vlan3004BmpRestore, &lagEn3004Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004);

        /* AUTODOC: Configure port filtering entries for VLANs 3001, 3002, 3003,
            3004:
           Idx(VLAN) | Port map
           ----------------------------------
           3001      | Port 1, Port 2
           3002      | Port 3
           3003      | Port 1, Port 2, Port 3
           3004      | No port
        */
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001,
            ((1 << PORT_UPSTREAM) | (1 << PORT_EXTENDED1) |
            (1 << PORT_EXTENDED2)), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002,
            ((1 << PORT_UPSTREAM) | (1 << PORT_EXTENDED3)), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003,
            ((1 << PORT_UPSTREAM) | (1 << PORT_EXTENDED1) | (1 << PORT_EXTENDED2) |
            (1 << PORT_EXTENDED3)), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004,
            (1 << PORT_UPSTREAM), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004);


        /* AUTODOC: Save destination port map entry table entries */
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001,
            &vlan1001BmpRestore, &lagEn1001Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001);
        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002,
            &vlan1002BmpRestore, &lagEn1002Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002);

        st = cpssPxIngressPortMapEntryGet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095,
            &vlan4095BmpRestore, &lagEn4095Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095);

        /* AUTODOC: Configure destination portmap entries for ECIDs 1001, 1002
           Idx(ECID) | Port map
           ----------------------------------
           1001      | Port 1, Port 2
           1002      | Port 3
           4095      | Upstream port
        */
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001,
            ((1 << PORT_EXTENDED1) | (1 << PORT_EXTENDED2)), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002,
            (1 << PORT_EXTENDED3), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095,
            (1 << PORT_UPSTREAM), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095);

        /* AUTODOC: Save filtering enable states */
        st = cpssPxIngressFilteringEnableGet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &upstreamTaggedEnIngRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableGet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &upstreamTaggedEnEgrRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_EGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableGet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_DOWNSTREAM, &downstreamEnIngRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);
        st = cpssPxIngressFilteringEnableGet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_DOWNSTREAM, &downstreamEnEgrRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Configure filtering enable states:
            Pkt type idx  | Enable egr. flt | Enable ing. flt
            -------------------------------------------------
            Downstream    | 1               | 0
            Upstream tag  | 0               | 1
            Upstream notag| 0               | 0
        */
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_EGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_DOWNSTREAM, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_DOWNSTREAM, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);
    }
    else
    {
        /* AUTODOC: Restore filtering enable states */
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, upstreamTaggedEnIngRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_UPSTREAM_TAGGED, upstreamTaggedEnEgrRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_EGRESS_E, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_INGRESS_E,
            PACKET_TYPE_DOWNSTREAM, downstreamEnIngRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);
        st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
            PACKET_TYPE_DOWNSTREAM, downstreamEnEgrRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_DIRECTION_INGRESS_E, PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Restore destination port map entry table entries */
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001,
            vlan1001BmpRestore, lagEn1001Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1001);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002,
            vlan1002BmpRestore, lagEn1002Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1002);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095,
            vlan4095BmpRestore, lagEn4095Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 4095);

        /* AUTODOC: Restore port filtering table entries */
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001,
            vlan3001BmpRestore, lagEn3001Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3001);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002,
            vlan3002BmpRestore, lagEn3002Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3002);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003,
            vlan3003BmpRestore, lagEn3003Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3003);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004,
            vlan3004BmpRestore, lagEn3004Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 3004);

        /* AUTODOC: Restore type format entry */
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormatDownstreamRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_DOWNSTREAM);
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormatUpstreamTagDstRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED, &typeFormatUpstreamTagRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM, &typeFormatUpstreamTagSrcRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
            PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Restore packet type key entries */
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, PACKET_TYPE_UPSTREAM_TAGGED,
            &upstreamTagKeyDataRestore, &upstreamTagKeyMaskRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_UPSTREAM_TAGGED);
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, PACKET_TYPE_DOWNSTREAM,
            &downstreamKeyDataRestore, &downstreamKeyMaskRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PACKET_TYPE_DOWNSTREAM);

        /* AUTODOC: Restore packet type keys */
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_UPSTREAM,
            &upstreamPortPacketTypeUpstreamRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_UPSTREAM);
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED1,
            &upstreamPortPacketTypeDownstream1Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED1);
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED2,
            &upstreamPortPacketTypeDownstream2Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED2);
        st = cpssPxIngressPortPacketTypeKeySet(dev, PORT_EXTENDED3,
            &upstreamPortPacketTypeDownstream3Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PORT_EXTENDED3);
    }

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test ingress and egress VLAN filtering.
 */
UTF_TEST_CASE_MAC(prvTgfPxVlanFiltering)
{
    GT_U8       dev = 0;
    GT_BOOL     skip = GT_TRUE;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(GT_FALSE == PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {
            continue;
        }
        skip = GT_FALSE;
        prvTgfPxVlanFilteringConfig(dev, GT_TRUE);
        prvTgfPxVlanFilteringExecute(dev);
        prvTgfPxVlanFilteringConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
    if(GT_TRUE == skip)
    {
        prvUtfSkipTestsSet();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test check ip address classification
 */
UTF_TEST_CASE_MAC(prvTgfPxIpClassification)
{
    GT_U8       dev = 0;
    GT_BOOL     skip = GT_TRUE;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(GT_FALSE == PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {
            continue;
        }
        skip = GT_FALSE;
        prvTgfPxIpClassificationConfig(dev, GT_TRUE);
        prvTgfPxIpClassificationExecute(dev);
        prvTgfPxIpClassificationConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
    if(GT_TRUE == skip)
    {
        prvUtfSkipTestsSet();
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: test for EVB E2U ingress processing
 * Based on HA in egress engine.
 *    check for EVB E2U: Extended Port to Upstream Port 
 *    check the proper VLAN tag is added to egress packet
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_EVB_E2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*-----------------------------------------------------------------------------------*/
/* AUTODOC: test for EVB U2E ingress processing
 * Based on HA in egress engine.
 *    check for EVB U2E: Upstream Port to Extended Port
 *    check the VLAN tag is removed from egress packet
 */
UTF_TEST_CASE_MAC(prvTgfPxIngress_EVB_U2E)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxIngressProcess(dev,
                                    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E,
                                    EXT_PARAM_DEFAULT_CNS,
                                    GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfIngress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfIngress)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressBasic)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_E2U)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2E)

    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2C)

    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_C2U)

    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2CPU)

    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_CPU2U)

    /* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E  */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_IPL2IPL)

 /* Ingress Processing Exceptions */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_EXCEPTION)

    /* Ingress Processing CoS assignment */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2CPU_COS)

    /* Ingress and Egress VLAN filtering */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxVlanFiltering)

    /* IP address classification */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIpClassification)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2E_M4)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_U2E_M8)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_EVB_E2U)

    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngress_EVB_U2E)


UTF_SUIT_END_TESTS_MAC(cpssPxTgfIngress)


