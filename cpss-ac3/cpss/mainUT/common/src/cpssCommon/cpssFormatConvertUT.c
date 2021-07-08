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
* @file cpssFormatConvertUT.c
*
* @brief Unit tests for Format Convertion utility
*
* @version   9
********************************************************************************
*/
/* includes */

#include <cpssCommon/cpssFormatConvert.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>

#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* Forward declarations */
static GT_STATUS prvTtiIpv4HwGet
(
    IN  CPSS_DXCH_TTI_IPV4_RULE_STC        *ipv4StcPtr,
    OUT GT_U32                             *hwFormatArray
);

/* the size of tti rule key/mask in words */
#define TTI_RULE_SIZE_CNS               8

/* the size of TCAM rule key/mask in words */
#define TCAM_RULE_SIZE_CNS              20

/*
CH2 HW format

Bits    Field name Description
6:0     {1'b0, SrcPort[5:0]/SrcTrunk[6:0] Source port or Source trunk-ID
7       SrcIsTrunk
        0 - Source is not a trunk
        1 - Source is a trunk
12:8    SrcDev[4:0] Source device
13      reserved reserved
25:14   VID[11:0] Packet VID assignment
73:26   MAC_DA[47:0] Tunnel header MAC DA
74      TunnelKeyType For MPLS TT key, this is 1
75      reserved reserved
78:76   EXP2 EXP in Label 2
79      Sbit2 S-bit in Label 2
99:80   Label2 Label 2
102:100 EXP1 EXP in Label 1
103     Sbit1 S-bit in Label 1
123:104 Label1 Label 1
126:124 reserved reserved
127     Ttkey Must be set to 1
191:128 reserved reserved

*/

#if 0
static GT_U32 ttMpls1Raw[4] = {
    0xa9554501,
    0x488d159e,
    0x11117404,
    0x80ffffd0};
#endif
/* Sample 2 */
/*
typedef struct
{
    GT_U32                  pclId;
    GT_BOOL                 srcIsTrunk;
    GT_U32                  srcPortTrunk;
    GT_ETHERADDR            mac;
    GT_U16                  vid;
    GT_BOOL                 isTagged;
    GT_BOOL                 dsaSrcIsTrunk;
    GT_U8                   dsaSrcPortTrunk;
    GT_U8                   dsaSrcDevice;

} CPSS_DXCH_TTI_RULE_COMMON_STC;

typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC common;
    GT_U32                  tunneltype;
    GT_IPADDR               srcIp;
    GT_IPADDR               destIp;
    GT_BOOL                 isArp;

} CPSS_DXCH_TTI_IPV4_RULE_STC;

  bits      width  name
9 : 0       10      PCL ID
17 : 10     8       LocalSrcPort/LocalSrcTrunkID
18 : 18     1       SrcIsTrunk
30 : 19     12      VLAN-ID
31 : 31     1       Packet Is Tagged
79 : 32     48      MAC_DA/MAC_SA
86 : 80     7       Source Device
89 : 87     3       Tunneling Protocol
121 : 90    32      SIP
153 : 122   32      DIP
154 : 154   1       IsARP
166 : 160   7       DSA Tag Source Port/Trunk
167 : 167   1       DSA SrcIsTrunk
190 : 168   23      Reserved
191 : 191   1       Must be 1. IPv4 / ARP TTI Key
159 : 155   5       Reserved

*/

#define TTI_CMN_STC CPSS_DXCH_TTI_RULE_COMMON_STC
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC ttiRuleCommon[] =
{
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC   (TTI_CMN_STC, pclId,          0, 10),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(TTI_CMN_STC, srcIsTrunk,    CPSS_FORMAT_CONVERT_FIELD_BOOL_E,  18, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC   (TTI_CMN_STC, srcPortTrunk,  10, 8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(TTI_CMN_STC, mac,           CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E,  32, 48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC   (TTI_CMN_STC, vid,           19, 12),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(TTI_CMN_STC, isTagged,      CPSS_FORMAT_CONVERT_FIELD_BOOL_E,  31, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(TTI_CMN_STC, dsaSrcIsTrunk, CPSS_FORMAT_CONVERT_FIELD_BOOL_E,  167, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC   (TTI_CMN_STC, dsaSrcPortTrunk, 160, 7),
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC   (TTI_CMN_STC, dsaSrcDevice,     80, 7),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

#define TTI_IPV4_STC CPSS_DXCH_TTI_IPV4_RULE_STC

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC ttiRuleIpv4[] =
{
    CPSS_FORMAT_CONVERT_FIELD_INFO_STRUCTURE_GEN_MAC(TTI_IPV4_STC, common, 0, ttiRuleCommon),
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC    (TTI_IPV4_STC, tunneltype,  87, 3),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC (TTI_IPV4_STC, srcIp,  CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E,  90, 32),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC (TTI_IPV4_STC, destIp, CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E,  122, 32),
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC (TTI_IPV4_STC, isArp,  CPSS_FORMAT_CONVERT_FIELD_BOOL_E,  154, 1),
    /* bit 191  - must be 1 */
    CPSS_FORMAT_CONVERT_FIELD_INFO_CONSTANT_MAC(1 , 191, 1, 0),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_DXCH_TTI_IPV4_RULE_STC  ttiRuleIpv4Stc1 = {
   {0x1 /* must be 1 */, GT_TRUE, 0x17, {{0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa}}, 0x11, GT_FALSE, GT_FALSE, 0x5, 0x10},
   0x1, {0x132887ff/*{0x13, 0x28, 0x87, 0xff}*/}, {0x001188d1/*{0x00, 0x11, 0x88, 0xd1}*/}, GT_TRUE
};

UTF_TEST_CASE_MAC(prvCpssFormat2)
{
    GT_U32                                  rawData[TCAM_RULE_SIZE_CNS];
    GT_U32                                  rawDataSample[TCAM_RULE_SIZE_CNS];
    CPSS_DXCH_TTI_IPV4_RULE_STC             stcData;
    GT_STATUS                               rc;
    GT_BOOL                                 isEqual   = GT_FALSE;
    /* convert STC to Raw */
    cpssOsMemSet(rawData, 0, sizeof(rawData));

    rc = cpssFormatConvertStcToRaw(ttiRuleIpv4,&ttiRuleIpv4Stc1, NULL, rawData);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssFormatConvertStcToRaw");

    /* compare raw data with sample */
    cpssOsMemSet(rawDataSample, 0, sizeof(rawDataSample));
    rc = prvTtiIpv4HwGet(&ttiRuleIpv4Stc1,rawDataSample);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTtiIpv4HwGet");

    /* compare Raw with Raw */
    isEqual = (0 == cpssOsMemCmp(&rawData, &rawDataSample,
                                 sizeof(rawDataSample))) ? GT_TRUE : GT_FALSE;

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                          "get another rawData than was set");

    cpssOsMemSet(&stcData, 0, sizeof(stcData));

    rc = cpssFormatConvertRawToStc(ttiRuleIpv4,rawData, &stcData, NULL);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssFormatConvertRawToStc");

    /* compare STC with STC */
    isEqual = (0 == cpssOsMemCmp(&stcData, &ttiRuleIpv4Stc1,
                                 sizeof(stcData))) ? GT_TRUE : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                          "get another stcData than was set");
}

/* the size of tti macToMe table entry in words */
#define TTI_MAC2ME_SIZE_CNS             4


/* maximum value for IPv4 tunnel type field */
#define IPV4_TUNNEL_TYPE_MAX_CNS        7

/* maximum value for MPLS label field */
#define MPLS_LABEL_MAX_CNS              1048575

/* maximum value for MPLS labels field */
#define MPLS_LABELS_MAX_CNS             3

/* maximum value for MPLS protocol above MPLS field */
#define MPLS_PROTOCOL_ABOVE_MAX_CNS     2

/* maximum value for ETH cfi field */
#define ETH_CFI_MAX_CNS                 1

static GT_STATUS ttiConfigLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             convertMask,
    IN  CPSS_DXCH_TTI_RULE_UNT             *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{
     /* zero out hw format */
    cpssOsMemSet(hwFormatArray,0,sizeof(GT_U32)*TTI_RULE_SIZE_CNS);

    /* to avoid warnings */
    devNum = devNum;

    /* check ipv4/mpls/eth configuration pararmeters only for config, not for mask */
    if (convertMask == GT_FALSE)
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
            if (logicFormatPtr->ipv4.common.pclId != 1)
                return GT_BAD_PARAM;
            if (logicFormatPtr->ipv4.tunneltype > IPV4_TUNNEL_TYPE_MAX_CNS)
                return GT_BAD_PARAM;
            break;

        case CPSS_DXCH_TTI_KEY_MPLS_E:
            if (logicFormatPtr->mpls.common.pclId != 2)
                return GT_BAD_PARAM;

            PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(logicFormatPtr->mpls.exp0);
            PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(logicFormatPtr->mpls.exp1);
            PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(logicFormatPtr->mpls.exp2);
            if (logicFormatPtr->mpls.label0 > MPLS_LABEL_MAX_CNS)
                return GT_BAD_PARAM;
            if (logicFormatPtr->mpls.label1 > MPLS_LABEL_MAX_CNS)
                return GT_BAD_PARAM;
            if (logicFormatPtr->mpls.label2 > MPLS_LABEL_MAX_CNS)
                return GT_BAD_PARAM;
            if (logicFormatPtr->mpls.numOfLabels > MPLS_LABELS_MAX_CNS)
                return GT_BAD_PARAM;
            if (logicFormatPtr->mpls.numOfLabels < MPLS_LABELS_MAX_CNS)
            {
                if (logicFormatPtr->mpls.protocolAboveMPLS > MPLS_PROTOCOL_ABOVE_MAX_CNS)
                    return GT_BAD_PARAM;
            }
            break;

        case CPSS_DXCH_TTI_KEY_ETH_E:
            if (logicFormatPtr->eth.common.pclId != 3)
                return GT_BAD_PARAM;
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->eth.common.vid);

            PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->eth.up0);
            if (logicFormatPtr->eth.cfi0 > ETH_CFI_MAX_CNS)
                return GT_BAD_PARAM;
            if (logicFormatPtr->eth.isVlan1Exists)
            {
                PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->eth.vid1);
                PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->eth.up1);
                if (logicFormatPtr->eth.cfi1 > ETH_CFI_MAX_CNS)
                    return GT_BAD_PARAM;
            }

            break;

        default:
            /* do nothing */
            break;
        }
    }

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:

        /* handle word 0 (bits 0-31) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],0,10,logicFormatPtr->ipv4.common.pclId);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],10,8,logicFormatPtr->ipv4.common.srcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],18,1,BOOL2BIT_MAC(logicFormatPtr->ipv4.common.srcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],19,12,logicFormatPtr->ipv4.common.vid);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,BOOL2BIT_MAC(logicFormatPtr->ipv4.common.isTagged));

        /* handle word 1 (bits 32-63) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,8,logicFormatPtr->ipv4.common.mac.arEther[5]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],8,8,logicFormatPtr->ipv4.common.mac.arEther[4]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],16,8,logicFormatPtr->ipv4.common.mac.arEther[3]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],24,8,logicFormatPtr->ipv4.common.mac.arEther[2]);

        /* handle word 2 (bits 64-95) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],0,8,logicFormatPtr->ipv4.common.mac.arEther[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],8,8,logicFormatPtr->ipv4.common.mac.arEther[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],16,7,logicFormatPtr->ipv4.common.dsaSrcDevice);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],23,3,logicFormatPtr->ipv4.tunneltype);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],26,6,logicFormatPtr->ipv4.srcIp.arIP[3]);

        /* handle word 3 (bits 96-127) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,2,(logicFormatPtr->ipv4.srcIp.arIP[3] >> 6));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],2,8,logicFormatPtr->ipv4.srcIp.arIP[2]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],10,8,logicFormatPtr->ipv4.srcIp.arIP[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],18,8,logicFormatPtr->ipv4.srcIp.arIP[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],26,6,logicFormatPtr->ipv4.destIp.arIP[3]);

        /* handle word 4 (bits 128-159) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],0,2,(logicFormatPtr->ipv4.destIp.arIP[3] >> 6));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],2,8,logicFormatPtr->ipv4.destIp.arIP[2]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],10,8,logicFormatPtr->ipv4.destIp.arIP[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],18,8,logicFormatPtr->ipv4.destIp.arIP[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],26,1,BOOL2BIT_MAC(logicFormatPtr->ipv4.isArp));

        /* handle word 5 (bits 160-191) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],0,7,logicFormatPtr->ipv4.common.dsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],7,1,BOOL2BIT_MAC(logicFormatPtr->ipv4.common.dsaSrcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */
        break;

    case CPSS_DXCH_TTI_KEY_MPLS_E:

        /* handle word 0 (bits 0-31) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],0,10,logicFormatPtr->mpls.common.pclId);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],10,8,logicFormatPtr->mpls.common.srcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],18,1,BOOL2BIT_MAC(logicFormatPtr->mpls.common.srcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],19,12,logicFormatPtr->mpls.common.vid);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,BOOL2BIT_MAC(logicFormatPtr->mpls.common.isTagged));

        /* handle word 1 (bits 32-63) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,8,logicFormatPtr->mpls.common.mac.arEther[5]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],8,8,logicFormatPtr->mpls.common.mac.arEther[4]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],16,8,logicFormatPtr->mpls.common.mac.arEther[3]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],24,8,logicFormatPtr->mpls.common.mac.arEther[2]);

        /* handle word 2 (bits 64-95) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],0,8,logicFormatPtr->mpls.common.mac.arEther[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],8,8,logicFormatPtr->mpls.common.mac.arEther[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],16,7,logicFormatPtr->mpls.common.dsaSrcDevice);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],23,3,logicFormatPtr->mpls.exp0);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],26,3,logicFormatPtr->mpls.exp1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],29,3,logicFormatPtr->mpls.label0);

        /* handle word 3 (bits 96-127) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,17,(logicFormatPtr->mpls.label0 >> 3));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],17,15,logicFormatPtr->mpls.label1);

        /* handle word 4 (bits 128-159) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],0,5,(logicFormatPtr->mpls.label1 >> 15));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],5,3,logicFormatPtr->mpls.exp2);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],8,20,logicFormatPtr->mpls.label2);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],28,2,logicFormatPtr->mpls.numOfLabels);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],30,2,logicFormatPtr->mpls.protocolAboveMPLS);

        /* handle word 5 (bits 160-191) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],0,7,logicFormatPtr->mpls.common.dsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],7,1,BOOL2BIT_MAC(logicFormatPtr->mpls.common.dsaSrcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */
        break;

    case CPSS_DXCH_TTI_KEY_ETH_E:

        /* handle word 0 (bits 0-31) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],0,10,logicFormatPtr->eth.common.pclId);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],10,8,logicFormatPtr->eth.common.srcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],18,1,BOOL2BIT_MAC(logicFormatPtr->eth.common.srcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],19,12,logicFormatPtr->eth.common.vid);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,BOOL2BIT_MAC(logicFormatPtr->eth.common.isTagged));

        /* handle word 1 (bits 32-63) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,8,logicFormatPtr->eth.common.mac.arEther[5]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],8,8,logicFormatPtr->eth.common.mac.arEther[4]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],16,8,logicFormatPtr->eth.common.mac.arEther[3]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],24,8,logicFormatPtr->eth.common.mac.arEther[2]);

        /* handle word 2 (bits 64-95) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],0,8,logicFormatPtr->eth.common.mac.arEther[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],8,8,logicFormatPtr->eth.common.mac.arEther[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],16,7,logicFormatPtr->eth.common.dsaSrcDevice);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],23,3,logicFormatPtr->eth.up0);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],26,1,logicFormatPtr->eth.cfi0);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],27,1,BOOL2BIT_MAC(logicFormatPtr->eth.isVlan1Exists));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],28,4,(logicFormatPtr->eth.vid1 >> 4));

        /* handle word 3 (bits 96-127) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,8,logicFormatPtr->eth.vid1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],8,3,logicFormatPtr->eth.up1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],11,1,logicFormatPtr->eth.cfi1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],12,16,logicFormatPtr->eth.etherType);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],28,1,BOOL2BIT_MAC(logicFormatPtr->eth.macToMe));
         /* handle word 4 (bits 128-159) */
        /* reserved */

        /* handle word 5 (bits 160-191) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],0,7,logicFormatPtr->eth.common.dsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],7,1,BOOL2BIT_MAC(logicFormatPtr->eth.common.dsaSrcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */
        break;

    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS prvTtiIpv4HwGet
(
    IN  CPSS_DXCH_TTI_IPV4_RULE_STC        *ipv4StcPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    CPSS_DXCH_TTI_RULE_UNT             logicFormat;
    GT_STATUS                           rc;

    logicFormat.ipv4 = *ipv4StcPtr;

    rc = ttiConfigLogic2HwFormat(0, CPSS_DXCH_TTI_KEY_IPV4_E, GT_FALSE,
                                    &logicFormat, hwFormatArray);

    return rc;
}

UTF_SUIT_BEGIN_TESTS_MAC(cpssFormatConvert)
UTF_SUIT_DECLARE_TEST_MAC(prvCpssFormat2)
UTF_SUIT_END_TESTS_MAC(cpssFormatConvert)


