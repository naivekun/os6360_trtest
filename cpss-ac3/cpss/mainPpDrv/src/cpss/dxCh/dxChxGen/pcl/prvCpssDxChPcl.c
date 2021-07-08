/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChPcl.c
*
* @brief Private CPSS CH PCL API implementation
*
* @version   145
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssCommon/cpssFormatConvert.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>

#define TCAM_OPER_TRACE_MAC(x) /*cpssOsPrintf x*/
/* debug environment parameter to allow the 'redirect' action to have bypassIngressPipe = 0 and bypassBridge = 0
    NOTE: in non sip5 devices this flag can be used to debug 'redirect' action with
        bypassIngressPipe = 0 and bypassBridge = 0

    In sip5 devices the full control is with the application anyway !!!
*/
static GT_U32   debugBypassFullControl = 0;
/* number of bits in the BMP of ports */
#define SIP5_PORT_LIST_NUM_BITS_CNS     28

/**************************************************************************************/
/* help macros and data for prvCpssDxChPclTcamRuleActionHw2SwConvert                              */
/**************************************************************************************/
/* ingress keys # */
#define F_KEY0     CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
#define F_KEY1     CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
#define F_KEY2     CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
#define F_KEY3     CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
#define F_KEY4     CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
#define F_KEY5     CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
#define F_KEY6     CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
#define F_KEY7     CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
#define F_KEY8     CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
#define F_KEY9     CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
#define F_KEY10    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
#define F_KEY11    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
#define F_KEY12    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC

/* egress keys # */
#define F_EGR_KEY0  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC             /*F_RESNI*/
#define F_EGR_KEY1  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC          /*F_RESIL2*/
#define F_EGR_KEY2  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC            /*F_RES4L4*/
#define F_EGR_KEY3  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC           /*F_REEN6*/
#define F_EGR_KEY4  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC            /*F_REE6L2*/
#define F_EGR_KEY5  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC            /*F_REE6L4*/
#define F_EGR_KEY6  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC     /*F_EE_IPV4_RACL_VACL*/
#define F_EGR_KEY7  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC   /*F_EU_IPV6_RACL_VACL*/

/* UDB only keys */
#define F_INGR_UDB_REP    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC
#define F_INGR_UDB_60FIX  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC
#define F_EGR_UDB_REP     CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC
#define F_EGR_UDB_60FIX   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC
#define F_INGR_UDB_ONLY   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC
#define F_EGR_UDB_ONLY    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC

/*******************************************************************/

#define F_RISNI    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
#define F_RISIL2   CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
#define F_RIS4L4   CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
#define F_RIS6D    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
#define F_RISU     CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
#define F_RIEN6    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
#define F_RIE6L2   CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
#define F_RIE6L4   CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
#define F_RIEU     CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
#define F_RESNI    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
#define F_RESIL2   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
#define F_RES4L4   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
#define F_REEN6    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
#define F_REE6L2   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
#define F_REE6L4   CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC

#define F_CI       CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
#define F_CISI     CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
#define F_CIE      CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
#define F_CIU      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
#define F_CE       CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
#define F_CEE      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
#define F_CESI     CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC

#define  /*ICMN*/ F_CI_IP                    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
#define  /*ISIP*/ F_IS_IPV4_ROUTED_ACL_QOS   CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
#define  /*IEIP*/ F_IE_IPV4_PORT_VLAN_QOS    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
#define  /*IUL0*/ F_IU_IPV6_PORT_VLAN_QOS    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
#define  /*IUL1*/ F_IU_IPV6_ROUTED_ACL_QOS   CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
#define  /*ECMN*/ F_CE_IP                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
#define  /*EEIP*/ F_EE_IPV4_RACL_VACL        CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
#define  /*EUIP*/ F_EU_IPV6_RACL_VACL        CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC

/* unconditional sized-int field without conversion */
#define I_F_MAC(_structType, _member, _rawBitOffset, _rawBitLength)  \
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_MAC(                    \
        _structType, _member, _rawBitOffset, _rawBitLength)

/* unconditional splitted sized-int field without conversion */
#define I_FO_MAC(_structType, _member, _rawBitOffset,              \
    _rawBitLength, _stcBitOffset)                                  \
    CPSS_FORMAT_CONVERT_FIELD_INFO_SIZED_INT_SPLIT_MAC(            \
        _structType, _member, _rawBitOffset, _rawBitLength,        \
        _stcBitOffset)

/* unconditional subtree for subtructure */
#define I_T_MAC(_struct, _member, _subTreeP)                       \
    CPSS_FORMAT_CONVERT_FIELD_INFO_STRUCTURE_GEN_MAC(              \
    _struct, _member, 0/*_conditionDep*/, _subTreeP)

/* unconditional subtree for the same subtructure */
#define I_ST_MAC(_subTreeP)                                        \
    CPSS_FORMAT_CONVERT_FIELD_INFO_SAME_STRUCT_SUBTREE_MAC(        \
    0/*_conditionDep*/, _subTreeP)

/* unconditional subtree for subtructure */
#define I_THO_MAC(_struct, _member, _rawWordsOffset, _subTreeP)          \
    CPSS_FORMAT_CONVERT_FIELD_INFO_STRUCTURE_GEN_WITH_RAW_OFFSET_MAC(    \
    _struct, _member, _rawWordsOffset, 0/*_conditionDep*/, _subTreeP)

/* unconditional subtree for the same subtructure */
#define I_STHO_MAC(_rawWordsOffset, _subTreeP)                               \
    CPSS_FORMAT_CONVERT_FIELD_INFO_SAME_STRUCT_SUBTREE_WITH_RAW_OFFSET_MAC(  \
    _rawWordsOffset, 0/*_conditionDep*/, _subTreeP)

/* unconditional byte-array member field without conversion */
#define I_FA_MAC(_structType, _member, _rawBitOffset, _rawBitLength)  \
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(                  \
        _structType, _member, CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E, \
        _rawBitOffset, _rawBitLength)

/* unconditional swapped byte-array member field without conversion */
#define I_FAS_MAC(_structType, _member, _rawBitOffset, _rawBitLength)  \
    CPSS_FORMAT_CONVERT_FIELD_INFO_TYPED_MEMBER_MAC(                   \
        _structType, _member,                                          \
        CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_ASCENT_E,                 \
        _rawBitOffset, _rawBitLength)


/* seg1 starts at bit 206 of the key (out of 208 bits in HW for seg 0)*/
/* used for extended and ultra keys */
#define LION2_SEG1_BITS_OFFSET_CNS  206
/* seg2 starts at bit 412 of the key (out of 416 bits in HW for seg 0,1)*/
/* used for ultra keys */
#define LION2_SEG2_BITS_OFFSET_CNS  412

/* seg1 starts at word 7 (as words 0..6 hold 206 bits of key) */
/* used for extended and ultra keys */
#define LION2_SEG1_WORDS_OFFSET_CNS 7
/* seg2 starts at word 14 (as words 0..6 hold 206 bits of key , and 7..13 hold additional 206 bits of key) */
/* used for extended and ultra keys */
#define LION2_SEG2_WORDS_OFFSET_CNS 14

/* The CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_COMMON_STC fiels in all keys */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_xCat[] =
{
    I_F_MAC(F_CI,   pclId,          0,  10),
    I_F_MAC(F_CI,   macToMe,        10,  1),
    I_F_MAC(F_CI,   sourcePort,     11,  6),
    I_F_MAC(F_CI,   isTagged,       17,  1),
    I_F_MAC(F_CI,   vid,            18, 12),
    I_F_MAC(F_CI,   up,             30,  3),
    I_F_MAC(F_CI,   qosProfile,     33,  7),
    I_F_MAC(F_CI,   isIp,           41,  1),
    I_F_MAC(F_CI,   isL2Valid,      107, 1),
    I_F_MAC(F_CI,   isUdbValid,     106, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_xCat2[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_xCat),
    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_basic1_sip5[] =
{
    I_F_MAC(F_CI,   pclId,          0,  10),
    I_F_MAC(F_CI,   macToMe,        10,  1),
    /* split the field */
    I_FO_MAC(F_CI,  sourcePort,     11,    6, 0),
    I_FO_MAC(F_CI,  sourcePort,     192,   2, 6),

    I_F_MAC(F_CI,   isTagged,       17,  1),
    I_F_MAC(F_CI,   vid,            18, 12),
    I_F_MAC(F_CI,   up,             30,  3),
    I_F_MAC(F_CI,   qosProfile,     33,  7),
    I_F_MAC(F_CI,   isIp,           41,  1),

    I_F_MAC(F_CI,    sourceDevice,       194,  10),

    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_sip5[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_basic1_sip5),
    I_F_MAC(F_CI,   isUdbValid,     106, 1),
    I_F_MAC(F_CI,   isL2Valid,      107, 1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC fields in all keys */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_EXT_sip5[] =
{
    I_F_MAC(F_CIE,   isIpv6,           40,  1),
    I_F_MAC(F_CIE,   ipProtocol,       48,  8),
    I_F_MAC(F_CIE,   dscp,             42,  6),
    I_F_MAC(F_CIE,   isL4Valid,        104, 1),
    I_F_MAC(F_CIE,   l4Byte0,          80,  8),
    I_F_MAC(F_CIE,   l4Byte1,          88,  8),
    I_F_MAC(F_CIE,   l4Byte2,          72,  8),
    I_F_MAC(F_CIE,   l4Byte3,          64,  8),
    I_F_MAC(F_CIE,   l4Byte13,         96,  8),
    I_F_MAC(F_CIE,   ipHeaderOk,       109, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC fiels in all keys */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_STD_IP_sip5[] =
{
    I_F_MAC(F_CISI,   isIpv4,             40,  1),
    I_F_MAC(F_CISI,   ipProtocol,         48,  8),
    I_F_MAC(F_CISI,   dscp,               42,  6),
    I_F_MAC(F_CISI,   isL4Valid,          104, 1),
    I_F_MAC(F_CISI,   l4Byte2,            72,  8),
    I_F_MAC(F_CISI,   l4Byte3,            64,  8),
    I_F_MAC(F_CISI,   ipHeaderOk,         109, 1),
    I_F_MAC(F_CISI,   ipv4Fragmented,     108, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_UDB_sip5[] =
{
    I_F_MAC(F_CIU,   pclId,             0,  10),
    I_F_MAC(F_CIU,   macToMe,           10,  1),

    /* split the field */
    I_FO_MAC(F_CIU,   sourcePort,        11,  6, 0),
    I_FO_MAC(F_CIU,   sourcePort,       192,  2, 6),
    /* split the field */
    I_FO_MAC(F_CIU,  pktTagging,        17,  1, 0),
    I_FO_MAC(F_CIU,  pktTagging,        33,  1, 1),

    I_F_MAC(F_CIU,   vid,               18, 12),
    I_F_MAC(F_CIU,   up,                30,  3),
    I_F_MAC(F_CIU,   l3OffsetInvalid,   34,  1),
    I_F_MAC(F_CIU,   l4ProtocolType,    35,  2),
    /* split the field */
    /*Applicable Flow Sub Template*/
    I_FO_MAC(F_CIU,  pktType,           37,  2, 0),
    I_FO_MAC(F_CIU,  pktType,           83,  6, 2),

    I_F_MAC(F_CIU,   ipHeaderOk,        39,  1),
    I_F_MAC(F_CIU,   isIp,              41,  1),
    I_F_MAC(F_CIU,   dscpOrExp,         42,  6),
    I_F_MAC(F_CIU,   macDaType,         80,  2),
    I_F_MAC(F_CIU,   l4OffsetInvalid,   82,  1),
    I_F_MAC(F_CIU,   l2Encapsulation,   89,  2),
    I_F_MAC(F_CIU,   isIpv6HopByHop,    91,  1),
    I_F_MAC(F_CIU,   isIpv6Eh,          92,  1),
    I_F_MAC(F_CIU,   isUdbValid,        94,  1),
    I_F_MAC(F_CIU,   isL2Valid,         95,  1),

    I_F_MAC(F_CIU,    sourceDevice,       194,  10),

    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/*fmtFld_INGRESS_STD_NOT_IP_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key0_flds[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_sip5),

    I_F_MAC(F_KEY0,   isIpv4,            40,  1),
    I_F_MAC(F_KEY0,   etherType,         42,  16),
    /* split the field */
    I_FO_MAC(F_KEY0,  vid1,              58,  6, 0),
    /*udb15*/
    I_F_MAC(F_KEY0,   udb15_17[0],       64,  8),
    /*udb16*/
    I_F_MAC(F_KEY0,   udb15_17[1],       72,  8),

    /* split the field */
    I_FA_MAC(F_KEY0,  macSa.arEther[3],  80,  24),
    I_FA_MAC(F_KEY0,  macSa.arEther[0],  120, 24),

    /* split the field */
    I_FO_MAC(F_KEY0,  up1,               104, 1, 0),
    I_FO_MAC(F_KEY0,  up1,               110, 2, 1),

    I_F_MAC(F_KEY0,   isArp,             105, 1),
    I_F_MAC(F_KEY0,   l2Encap,           108, 2),
    /*udb17*/
    I_F_MAC(F_KEY0,   udb15_17[2],       112, 8),

    I_FA_MAC(F_KEY0,  macDa,             144, 48),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY0,   vrfId,            64,  8,  0),
    I_FO_MAC(F_KEY0,   vrfId,            72,  4,  8),

    I_FO_MAC(F_KEY0,   vid1,             72,  6,  6),

    I_F_MAC(F_KEY0,   cfi1,              78,  1),
    I_F_MAC(F_KEY0,   tag1Exist,         79,  1),
    I_F_MAC(F_KEY0,   trunkHash,         112, 6),

    /*udb23*/
    I_F_MAC(F_KEY0,   udb23_26[0],       208, 8),
    /*udb24*/
    I_F_MAC(F_KEY0,   udb23_26[1],       216, 8),
    /*udb25*/
    I_F_MAC(F_KEY0,   udb23_26[2],       224, 8),
    /*udb26*/
    I_F_MAC(F_KEY0,   udb23_26[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/*fmtFld_INGRESS_STD_IP_L2_QOS_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key1_flds[] =
{
    I_T_MAC(F_KEY1,    common,      fmtFld_INGRESS_COMMON_sip5),
    I_T_MAC(F_KEY1,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_sip5),

    I_F_MAC(F_KEY1,   isArp,              105,  1),
    I_F_MAC(F_KEY1,   isIpv6ExtHdrExist,  110,  1),
    I_F_MAC(F_KEY1,   isIpv6HopByHop,     111,  1),
    I_FA_MAC(F_KEY1,  macDa,              144,  48),
    /* split the field */
    I_FA_MAC(F_KEY1,  macSa.arEther[3],    80,  24),
    I_FA_MAC(F_KEY1,  macSa.arEther[0],   120,  24),

    /*udb 18*/
    I_F_MAC(F_KEY1,   udb18_19[0],         56,  8),
    /*udb 19*/
    I_F_MAC(F_KEY1,   udb18_19[1],        112,  8),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY1,  vrfId,               56,  8,  0),
    I_FO_MAC(F_KEY1,  vrfId,               112, 4,  8),

    /*udb27*/
    I_F_MAC(F_KEY1,   udb27_30[0],       208, 8),
    /*udb28*/
    I_F_MAC(F_KEY1,   udb27_30[1],       216, 8),
    /*udb29*/
    I_F_MAC(F_KEY1,   udb27_30[2],       224, 8),
    /*udb30*/
    I_F_MAC(F_KEY1,   udb27_30[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/*fmtFld_INGRESS_STD_IPV4_L4_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key2_flds[] =
{
    I_T_MAC(F_KEY2,    common,      fmtFld_INGRESS_COMMON_sip5),
    I_T_MAC(F_KEY2,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_sip5),

    I_F_MAC(F_KEY2,   isArp,              105,  1),
    I_F_MAC(F_KEY2,   isBc,               110,  1),
    I_FA_MAC(F_KEY2,  sip,                112,  32),
    I_FA_MAC(F_KEY2,  dip,                144,  32),
    I_F_MAC(F_KEY2,   l4Byte0,             80,  8),
    I_F_MAC(F_KEY2,   l4Byte1,             88,  8),
    I_F_MAC(F_KEY2,   l4Byte13,            96,  8),
    /*udb 20*/
    I_F_MAC(F_KEY2,   udb20_22[0],         56,  8),
    /*udb 21*/
    I_F_MAC(F_KEY2,   udb20_22[1],        176,  8),
    /*udb 22*/
    I_F_MAC(F_KEY2,   udb20_22[2],        184,  8),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY2,  vrfId,               56,  8,  0),
    I_FO_MAC(F_KEY2,  vrfId,              176,  4,  8),

    I_F_MAC(F_KEY2,   trunkHash,          184,  6),

    /*udb31*/
    I_F_MAC(F_KEY2,   udb31_34[0],       208, 8),
    /*udb32*/
    I_F_MAC(F_KEY2,   udb31_34[1],       216, 8),
    /*udb33*/
    I_F_MAC(F_KEY2,   udb31_34[2],       224, 8),
    /*udb34*/
    I_F_MAC(F_KEY2,   udb31_34[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/*fmtFld_INGRESS_STD_UDB_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key3_flds[] =
{
    I_T_MAC(F_KEY3,    commonIngrUdb, fmtFld_INGRESS_COMMON_UDB_sip5),

    I_F_MAC(F_KEY3,   isIpv4,            40,  1),

    /*udb 0*/
    I_F_MAC(F_KEY3, udb0_15[0],        48,  8),
    /*udb 1*/
    I_F_MAC(F_KEY3, udb0_15[1],        56,  8),
    /*udb 2*/
    I_F_MAC(F_KEY3, udb0_15[2],        64,  8),
    /*udb 5*/
    I_F_MAC(F_KEY3, udb0_15[5],        72,   8),
    /*udb 3*/
    I_F_MAC(F_KEY3, udb0_15[3],        96,  8),
    /*udb 4*/
    I_F_MAC(F_KEY3, udb0_15[4],       104,  8),
    /*udb 6*/
    I_F_MAC(F_KEY3, udb0_15[6],       112,  8),
    /*udb 7*/
    I_F_MAC(F_KEY3, udb0_15[7],       120,  8),
    /*udb 8*/
    I_F_MAC(F_KEY3, udb0_15[8],       128,  8),
    /*udb 9*/
    I_F_MAC(F_KEY3, udb0_15[9],       136,  8),
    /*udb 10*/
    I_F_MAC(F_KEY3, udb0_15[10],      144,  8),
    /*udb 11*/
    I_F_MAC(F_KEY3, udb0_15[11],      152,  8),
    /*udb 12*/
    I_F_MAC(F_KEY3, udb0_15[12],      160,  8),
    /*udb 13*/
    I_F_MAC(F_KEY3, udb0_15[13],      168,  8),
    /*udb 14*/
    I_F_MAC(F_KEY3, udb0_15[14],      176,  8),
    /*udb 15*/
    I_F_MAC(F_KEY3, udb0_15[15],      184,  8),

    /* split the field */
    I_FO_MAC(F_KEY3,  vrfId,              48,  8,  0),
    I_FO_MAC(F_KEY3,  vrfId,              56,  4,  8),

    I_F_MAC(F_KEY3,   qosProfile,         64,    7),

    I_F_MAC(F_KEY3,   trunkHash,          96,  6),

    /*udb35*/
    I_F_MAC(F_KEY3,   udb35_38[0],       208, 8),
    /*udb36*/
    I_F_MAC(F_KEY3,   udb35_38[1],       216, 8),
    /*udb37*/
    I_F_MAC(F_KEY3,   udb35_38[2],       224, 8),
    /*udb38*/
    I_F_MAC(F_KEY3,   udb35_38[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/*fmtFld_INGRESS_EXT_NOT_IPV6_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key4_flds[] =
{
    I_T_MAC(F_KEY4,    common,      fmtFld_INGRESS_COMMON_sip5),
    I_T_MAC(F_KEY4,    commonExt,   fmtFld_INGRESS_COMMON_EXT_sip5),

    /*udb5*/
    I_F_MAC(F_KEY4,    udb0_5[5],         56,   8),

    /* split the field */
    I_FO_MAC(F_KEY4,   l2Encap,          105,   1,  0),
    I_FO_MAC(F_KEY4,   l2Encap,          110,   1,  1),

    I_F_MAC(F_KEY4,   ipv4Fragmented,    108,   1),

    I_FA_MAC(F_KEY4,   sip,              112,  32),
    I_FA_MAC(F_KEY4,   dip,              144,  32),
    I_F_MAC(F_KEY4,    etherType,        176,  16),

    I_F_MAC(F_KEY4,   up1,               208,  3),
    I_FA_MAC(F_KEY4,   macSa,            264,  48),
    I_FA_MAC(F_KEY4,   macDa,            312,  48),
    /*udb1*/
    I_FAS_MAC(F_KEY4,  udb0_5[1],        360,  8),
    /*udb2*/
    I_FAS_MAC(F_KEY4,  udb0_5[2],        368,  8),
    /*udb3*/
    I_FAS_MAC(F_KEY4,  udb0_5[3],        376,  8),
    /*udb4*/
    I_FAS_MAC(F_KEY4,  udb0_5[4],        384,  8),
    /*udb0*/
    I_FAS_MAC(F_KEY4,  udb0_5[0],        392,  8),

    I_F_MAC(F_KEY4,   tag1Exist,         400,  1),
    I_F_MAC(F_KEY4,   vid1,              401,  12),
    I_F_MAC(F_KEY4,   cfi1,              413,  1),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY4,   vrfId,             56,   8,  0),
    I_FO_MAC(F_KEY4,   vrfId,             360,  4,  8),

    I_F_MAC(F_KEY4,   trunkHash,          376,  6),

    /*udb39*/
    I_F_MAC(F_KEY4,   udb39_46[0],       416, 8),
    /*udb40*/
    I_F_MAC(F_KEY4,   udb39_46[1],       424, 8),
    /*udb41*/
    I_F_MAC(F_KEY4,   udb39_46[2],       432, 8),
    /*udb42*/
    I_F_MAC(F_KEY4,   udb39_46[3],       440, 8),
    /*udb43*/
    I_F_MAC(F_KEY4,   udb39_46[4],       448, 8),
    /*udb44*/
    I_F_MAC(F_KEY4,   udb39_46[5],       456, 8),
    /*udb45*/
    I_F_MAC(F_KEY4,   udb39_46[6],       464, 8),
    /*udb46*/
    I_F_MAC(F_KEY4,   udb39_46[7],       472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/*fmtFld_INGRESS_EXT_IPV6_L2_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key5_flds[] =
{
    I_T_MAC(F_KEY5,    common,      fmtFld_INGRESS_COMMON_sip5),
    I_T_MAC(F_KEY5,    commonExt,   fmtFld_INGRESS_COMMON_EXT_sip5),

    /* udb 11 */
    I_F_MAC(F_KEY5,    udb6_11[5],           56,  8),

    I_F_MAC(F_KEY5,    isIpv6ExtHdrExist,    105,  1),
    I_F_MAC(F_KEY5,    isIpv6HopByHop,       108,  1),
    /*SIP[31:0],SIP[79:32]*/
    I_FA_MAC(F_KEY5,   sip.arIP[6],          112,  80),

    /* split the field */
    I_FO_MAC(F_KEY5,   up1,               110,  2,  0),
    I_FO_MAC(F_KEY5,   up1,               413,  1,  2),

    /*SIP[127:80]*/
    I_FA_MAC(F_KEY5,   sip.arIP[0],        208,  48),
    /*DIP[127:120]*/
    I_F_MAC(F_KEY5,    dipBits127to120,    256,   8),

    I_FA_MAC(F_KEY5,   macSa,              264,  48),
    I_FA_MAC(F_KEY5,   macDa,              312,  48),
    /*udb 6*/
    I_F_MAC(F_KEY5,  udb6_11[0],         360,  8),
    /*udb 7*/
    I_F_MAC(F_KEY5,  udb6_11[1],         368,  8),
    /*udb 8*/
    I_F_MAC(F_KEY5,  udb6_11[2],         376,  8),
    /*udb 9*/
    I_F_MAC(F_KEY5,  udb6_11[3],         384,  8),
    /*udb 10*/
    I_F_MAC(F_KEY5,  udb6_11[4],         392,  8),

    I_F_MAC(F_KEY5,    tag1Exist,         400,  1),
    I_F_MAC(F_KEY5,    vid1,              401,  12),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY5,   vrfId,               56,   8,  0),
    I_FO_MAC(F_KEY5,   vrfId,              360,   4,  8),

    I_F_MAC(F_KEY5,    trunkHash,          368,   6),

    /*udb47*/
    I_F_MAC(F_KEY5,   udb47_49[0],       416, 8),
    /*udb48*/
    I_F_MAC(F_KEY5,   udb47_49[1],       424, 8),
    /*udb49*/
    I_F_MAC(F_KEY5,   udb47_49[2],       432, 8),
    /*udb0*/
    I_F_MAC(F_KEY5,   udb0_4[0],         440, 8),
    /*udb2*/
    I_F_MAC(F_KEY5,   udb0_4[1],         448, 8),
    /*udb2*/
    I_F_MAC(F_KEY5,   udb0_4[2],         456, 8),
    /*udb3*/
    I_F_MAC(F_KEY5,   udb0_4[3],         464, 8),
    /*udb4*/
    I_F_MAC(F_KEY5,   udb0_4[4],         472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/*fmtFld_INGRESS_EXT_IPV6_L4_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key6_flds[] =
{
    I_T_MAC(F_KEY6,    common,      fmtFld_INGRESS_COMMON_sip5),
    I_T_MAC(F_KEY6,    commonExt,   fmtFld_INGRESS_COMMON_EXT_sip5),

    /*SIP[31:0]*/
    I_FA_MAC(F_KEY6,   sip.arIP[12],          112,  32),
    /*DIP[31:0]*/
    I_FA_MAC(F_KEY6,   dip.arIP[12],          144,  32),
    I_F_MAC(F_KEY6,    isIpv6ExtHdrExist,     105,   1),
    I_F_MAC(F_KEY6,    isIpv6HopByHop,        108,   1),
    /*SIP[47:32]*/
    I_FA_MAC(F_KEY6,   sip.arIP[10],          176,  16),
    /*udb 14*/
    I_F_MAC(F_KEY6,    udb12_14[2],            56,   8),

    /* split the field */
    I_FO_MAC(F_KEY6,   up1,               110,  2,  0),
    I_FO_MAC(F_KEY6,   up1,               413,  1,  2),

    /*SIP[127:48]*/
    I_FA_MAC(F_KEY6,   sip.arIP[0],        208,  80),
    /*DIP[127:120]*/
    I_FA_MAC(F_KEY6,   dip.arIP[0],        288,   8),
    /*DIP[119:32]*/
    I_FA_MAC(F_KEY6,   dip.arIP[1],        296,  88),

    /*udb 12*/
    I_F_MAC(F_KEY6,  udb12_14[0],        384,  8),
    /*udb 13*/
    I_F_MAC(F_KEY6,  udb12_14[1],        392,  8),

    I_F_MAC(F_KEY6,    tag1Exist,         400,  1),
    I_F_MAC(F_KEY6,    vid1,              401,  12),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY6,   vrfId,               56,   8,  0),
    I_FO_MAC(F_KEY6,   vrfId,              384,   4,  8),

    I_F_MAC(F_KEY6,    trunkHash,          392,   6),

    /*udb15*/
    I_F_MAC(F_KEY6,   udb15_22[0],       416, 8),
    /*udb16*/
    I_F_MAC(F_KEY6,   udb15_22[1],       424, 8),
    /*udb17*/
    I_F_MAC(F_KEY6,   udb15_22[2],       432, 8),
    /*udb18*/
    I_F_MAC(F_KEY6,   udb15_22[3],       440, 8),
    /*udb19*/
    I_F_MAC(F_KEY6,   udb15_22[4],       448, 8),
    /*udb20*/
    I_F_MAC(F_KEY6,   udb15_22[5],       456, 8),
    /*udb21*/
    I_F_MAC(F_KEY6,   udb15_22[6],       464, 8),
    /*udb22*/
    I_F_MAC(F_KEY6,   udb15_22[7],       472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/*fmtFld_INGRESS_EXT_UDB_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key7_flds[] =
{
    I_T_MAC(F_KEY7,    commonIngrUdb,      fmtFld_INGRESS_COMMON_UDB_sip5),

    I_F_MAC(F_KEY7,    isIpv6,                    40,    1),
    I_F_MAC(F_KEY7,    ipProtocol,                48,    8),
    /*udb 1*/
    I_F_MAC(F_KEY7,    udb0_15[1],                56,    8),
    /*udb 2*/
    I_F_MAC(F_KEY7,    udb0_15[2],                64,    8),
    /*udb 5*/
    I_F_MAC(F_KEY7,    udb0_15[5],                72,    8),
    /*udb 3*/
    I_F_MAC(F_KEY7,    udb0_15[3],                96,   8),
    /*udb 4*/
    I_F_MAC(F_KEY7,    udb0_15[4],               104,   8),
    /*udb 6*/
    I_F_MAC(F_KEY7,    udb0_15[6],               112,   8),
    /*udb 7*/
    I_F_MAC(F_KEY7,    udb0_15[7],               120,   8),
    /*udb 8*/
    I_F_MAC(F_KEY7,    udb0_15[8],               128,   8),
    /*udb 9*/
    I_F_MAC(F_KEY7,    udb0_15[9],               136,   8),

    /*SIP[31:0] / S_ID*/
    I_FA_MAC(F_KEY7,   sipBits31_0[0],           144,   32),
    /*MAC SA[15:0] / SIP[47:32]*/
    I_FA_MAC(F_KEY7,   sipBits79_32orMacSa[4],   176,   16),
    /*MAC SA[47:16] / SIP[79:48]*/
    I_FA_MAC(F_KEY7,   sipBits79_32orMacSa[0],   208,   32),
    /*MAC DA / SIP[127:80]*/
    I_FA_MAC(F_KEY7,   sipBits127_80orMacDa[0],  240,   48),
    /*DIP[127:112] */
    I_FA_MAC(F_KEY7,   dipBits127_112[0],        288,   16),
    /*DIP[31:0] / D_ID*/
    I_FA_MAC(F_KEY7,   dipBits31_0[0],           304,   32),

    /*udb 10*/
    I_F_MAC(F_KEY7,  udb0_15[10],              336,   8),
    /*udb 11*/
    I_F_MAC(F_KEY7,  udb0_15[11],              344,   8),
    /*udb 12*/
    I_F_MAC(F_KEY7,  udb0_15[12],              352,   8),
    /*udb 13*/
    I_F_MAC(F_KEY7,  udb0_15[13],              360,   8),
    /*udb 14*/
    I_F_MAC(F_KEY7,  udb0_15[14],              368,   8),
    /*udb 15*/
    I_F_MAC(F_KEY7,  udb0_15[15],              376,   8),
    /*udb 0*/
    I_F_MAC(F_KEY7,  udb0_15[0],               384,   8),

    I_F_MAC(F_KEY7,    up1,                    397,   3),
    I_F_MAC(F_KEY7,    tag1Exist,              400,   1),
    I_F_MAC(F_KEY7,    vid1,                   401,  12),
    I_F_MAC(F_KEY7,    cfi1,                   413,   1),

    /*udb23*/
    I_F_MAC(F_KEY7,   udb23_30[0],       416, 8),
    /*udb24*/
    I_F_MAC(F_KEY7,   udb23_30[1],       424, 8),
    /*udb25*/
    I_F_MAC(F_KEY7,   udb23_30[2],       432, 8),
    /*udb26*/
    I_F_MAC(F_KEY7,   udb23_30[3],       440, 8),
    /*udb27*/
    I_F_MAC(F_KEY7,   udb23_30[4],       448, 8),
    /*udb28*/
    I_F_MAC(F_KEY7,   udb23_30[5],       456, 8),
    /*udb29*/
    I_F_MAC(F_KEY7,   udb23_30[6],       464, 8),
    /*udb30*/
    I_F_MAC(F_KEY7,   udb23_30[7],       472, 8),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY7,   vrfId,               56,   8,  0),
    I_FO_MAC(F_KEY7,   vrfId,               64,   4,  8),

    I_F_MAC(F_KEY7,    qosProfile,          72,    7),

    I_F_MAC(F_KEY7,    trunkHash,           96,    6),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* conversion table for Bobcat2, Caelum, Bobcat3 Common.                                         */
/* The tos[1] field different for all keys therefore it converted separately */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_IP_COMMON_sip5[] =
{
    I_F_MAC(F_CI_IP,   pclId,       0,  10),
    I_F_MAC(F_CI_IP,   macToMe,     10,  1),

    /* split the field */
    I_FO_MAC(F_CI,  sourcePort,     11,    6, 0),
    I_FO_MAC(F_CI,  sourcePort,     192,   2, 6),

    I_F_MAC(F_CI_IP,   isTagged,    17,  1),
    I_F_MAC(F_CI_IP,   vid,         18, 12),
    I_F_MAC(F_CI_IP,   up,          30,  3),
    /* split the field */
    I_FO_MAC(F_CI_IP,  tos,         33,  6, 2),
    I_FO_MAC(F_CI_IP,  tos,         50,  1, 0),

    I_F_MAC(F_CI_IP,   ipProtocol,  42,  8),
    I_F_MAC(F_CI_IP,   isL4Valid,   56,   1),
    I_F_MAC(F_CI_IP,   l4Byte0,     80,   8),
    I_F_MAC(F_CI_IP,   l4Byte1,     88,   8),
    I_F_MAC(F_CI_IP,   l4Byte2,     72,   8),
    I_F_MAC(F_CI_IP,   l4Byte3,     64,   8),
    I_F_MAC(F_CI_IP,   l4Byte13,    96,   8),

    I_F_MAC(F_CI_IP,    sourceDevice,       194,  10),

    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* for Bobcat2, Caelum, Bobcat3 -- fmtFldLion2_INGRESS_EXT_IPV4_PORT_VLAN_QOS */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key8_flds[] =
{
    I_T_MAC(F_KEY8,   ingressIpCommon, fmtFld_INGRESS_IP_COMMON_sip5),
    I_FO_MAC(F_KEY8,  ingressIpCommon.tos,        58,   1,   1),
    I_F_MAC(F_KEY8,   isL2Valid,                  55,   1),
    I_F_MAC(F_KEY8,   isBc,                       40,   1),
    I_F_MAC(F_KEY8,   isIp,                       41,   1),
    I_F_MAC(F_KEY8,   isArp,                      39,   1),
    /*MAC SA[15:0]*/
    I_FA_MAC(F_KEY8,  macSa.arEther[4],           176,   16),
    I_F_MAC(F_KEY8,   ipFragmented,               51,   2),
    I_F_MAC(F_KEY8,   ipHeaderInfo,               53,   2),
    /*SIP[31:0] / S_ID*/
    I_FA_MAC(F_KEY8,  sip,                       112,   32),
    /*DIP[31:0] / D_ID*/
    I_FA_MAC(F_KEY8,  dip,                       144,   32),
    I_F_MAC(F_KEY8,   isUdbValid,                 57,   1),
    /*udb 13*/
    I_F_MAC(F_KEY8,   udb5_16[8],                104,   8),
    /*MAC SA[47:32]*/
    I_FA_MAC(F_KEY8,  macSa.arEther[0],         208,   32),
    I_FA_MAC(F_KEY8,  macDa,                    240,   48),
    I_F_MAC(F_KEY8,   etherType,                288,   16),
    /* split the field */
    I_FO_MAC(F_KEY8,  l2Encap,                  304,   1,   0),
    I_FO_MAC(F_KEY8,  l2Encap,                  329,   1,   1),
    /*udb 14*/
    I_F_MAC(F_KEY8,   udb5_16[9],               305,   8),
    /*udb 15*/
    I_F_MAC(F_KEY8,   udb5_16[10],              313,   8),
    /*udb 16*/
    I_F_MAC(F_KEY8,   udb5_16[11],              321,   8),

    I_F_MAC(F_KEY8,    up1,                    333,   3),
    I_F_MAC(F_KEY8,    tag1Exist,              336,   1),
    I_F_MAC(F_KEY8,    vid1,                   337,  12),
    I_F_MAC(F_KEY8,    cfi1,                   349,   1),

    /*udb 6*/
    I_F_MAC(F_KEY8,   udb5_16[1],               350,   8),
    /*udb 7*/
    I_F_MAC(F_KEY8,   udb5_16[2],               358,   8),
    /*udb 8*/
    I_F_MAC(F_KEY8,   udb5_16[3],               366,   8),
    /*udb 9*/
    I_F_MAC(F_KEY8,   udb5_16[4],               374,   8),
    /*udb 10*/
    I_F_MAC(F_KEY8,   udb5_16[5],               382,   8),
    /*udb 11*/
    I_F_MAC(F_KEY8,   udb5_16[6],               390,   8),
    /*udb 12*/
    I_F_MAC(F_KEY8,   udb5_16[7],               398,   8),
    /*udb 5*/
    I_F_MAC(F_KEY8,   udb5_16[0],               406,   8),

    /*udb31*/
    I_F_MAC(F_KEY8,   udb31_38[0],       416, 8),
    /*udb32*/
    I_F_MAC(F_KEY8,   udb31_38[1],       424, 8),
    /*udb33*/
    I_F_MAC(F_KEY8,   udb31_38[2],       432, 8),
    /*udb34*/
    I_F_MAC(F_KEY8,   udb31_38[3],       440, 8),
    /*udb35*/
    I_F_MAC(F_KEY8,   udb31_38[4],       448, 8),
    /*udb36*/
    I_F_MAC(F_KEY8,   udb31_38[5],       456, 8),
    /*udb37*/
    I_F_MAC(F_KEY8,   udb31_38[6],       464, 8),
    /*udb38*/
    I_F_MAC(F_KEY8,   udb31_38[7],       472, 8),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY8,   vrfId,               104,   8,  0),
    I_FO_MAC(F_KEY8,   vrfId,               305,   4,  8),

    I_F_MAC(F_KEY8,    trunkHash,           313,    6),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* for Bobcat2, Caelum, Bobcat3 -- fmtFldLion2_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key9_flds[] =
{
    I_T_MAC(F_KEY9,   ingressIpCommon, fmtFld_INGRESS_IP_COMMON_sip5),
    I_FO_MAC(F_KEY9,  ingressIpCommon.tos,        58,   1,   1),
    I_F_MAC(F_KEY9,   isL2Valid,                  55,   1),
    I_F_MAC(F_KEY9,   isNd,                       53,   1),
    I_F_MAC(F_KEY9,   isBc,                       40,   1),
    I_F_MAC(F_KEY9,   isIp,                       41,   1),

    /* split the field */
    I_FO_MAC(F_KEY9,  l2Encap,                    39,   1,   0),
    I_FO_MAC(F_KEY9,  l2Encap,                    59,   1,   1),

    /*udb 12*/
    I_F_MAC(F_KEY9,   udb12,                     104,   8),

    /*MAC SA[15:0]*/
    I_FA_MAC(F_KEY9,  macSa.arEther[4],          176,  16),
    /*SIP[31:0]*/
    I_FA_MAC(F_KEY9,  sip.arIP[12],              112,  32),
    /*DIP[31:0]*/
    I_FA_MAC(F_KEY9,  dip.arIP[12],              144,  32),
    I_F_MAC(F_KEY9,   isIpv6HopByHop,             51,   1),
    I_F_MAC(F_KEY9,   isIpv6ExtHdrExist,          52,   1),
    I_F_MAC(F_KEY9,   ipHeaderOk,                 54,   1),
    I_F_MAC(F_KEY9,   isUdbValid,                 57,   1),
    /*MAC SA[47:32]*/
    I_FA_MAC(F_KEY9,  macSa.arEther[0],          208,   32),
    /*MAC DA*/
    I_FA_MAC(F_KEY9,  macDa,                     240,   48),
    I_F_MAC(F_KEY9,   etherType,                 288,   16),
    /*SIP[127:32]*/
    I_FA_MAC(F_KEY9,  sip.arIP[0],               304,   96),
    /*DIP[127:32]*/
    I_FA_MAC(F_KEY9,  dip.arIP[0],               416,   96),

    /*udb 0*/
    I_F_MAC(F_KEY9,   udb0_11[0],                512,   8),
    /*udb 1*/
    I_F_MAC(F_KEY9,   udb0_11[1],                520,   8),
    /*udb 2*/
    I_F_MAC(F_KEY9,   udb0_11[2],                528,   8),
    /*udb 3*/
    I_F_MAC(F_KEY9,   udb0_11[3],                536,   8),
    /*udb 4*/
    I_F_MAC(F_KEY9,   udb0_11[4],                544,   8),
    /*udb 5*/
    I_F_MAC(F_KEY9,   udb0_11[5],                552,   8),
    /*udb 6*/
    I_F_MAC(F_KEY9,   udb0_11[6],                560,   8),
    /*udb 7*/
    I_F_MAC(F_KEY9,   udb0_11[7],                568,   8),
    /*udb 8*/
    I_F_MAC(F_KEY9,   udb0_11[8],                576,   8),
    /*udb 9*/
    I_F_MAC(F_KEY9,   udb0_11[9],                584,   8),
    /*udb 10*/
    I_F_MAC(F_KEY9,   udb0_11[10],               592,   8),
    /*udb 11*/
    I_F_MAC(F_KEY9,   udb0_11[11],               600,   8),

    /*udb 39*/
    I_F_MAC(F_KEY9,   udb39_40[0],               624,   8),
    /*udb 40*/
    I_F_MAC(F_KEY9,   udb39_40[1],               632,   8),

    /* split the field */
    I_FO_MAC(F_KEY9,  ingressIpCommon.vid,       409,   1,   12),

    /* split the field */
    /*SRCePort/Trunk*/
    I_FO_MAC(F_KEY9,  srcPortOrTrunk,             60,   4,    0),
    I_FO_MAC(F_KEY9,  srcPortOrTrunk,            400,   8,    4),

    I_F_MAC(F_KEY9,   srcIsTrunk,                408,   1),

    I_F_MAC(F_KEY9,   up1,                       411,   3),

    I_F_MAC(F_KEY9,   tag1Exist,                 608,   1),
    I_F_MAC(F_KEY9,   vid1,                      609,  12),
    I_F_MAC(F_KEY9,   cfi1,                      621,   1),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY9,  vrfId,                     104,   8,  0),
    I_FO_MAC(F_KEY9,  vrfId,                     512,   4,  8),

    I_F_MAC(F_KEY9,   trunkHash,                 520,    6),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* for Bobcat2, Caelum, Bobcat3 -- fmtFldLion2_INGRESS_STD_IPV4_ROUTED_ACL_QOS */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key10_flds[] =
{
    I_T_MAC(F_KEY10,   ingressIpCommon, fmtFld_INGRESS_IP_COMMON_sip5),
    I_FO_MAC(F_KEY10,  ingressIpCommon.tos,        63,   1, 1),
    I_F_MAC(F_KEY10,   pktType,                    39,   3),
    I_F_MAC(F_KEY10,   ipFragmented,               51,   2),
    I_F_MAC(F_KEY10,   ipHeaderInfo,               53,   2),
    /* split the field */
    I_FO_MAC(F_KEY10,  ipPacketLength,             57,   6,  0),
    I_FO_MAC(F_KEY10,  ipPacketLength,            176,   8,  6),

    I_F_MAC(F_KEY10,   ttl,                       184,   8),
    /*SIP / S_ID*/
    I_FA_MAC(F_KEY10,  sip,                       112,   32),
    /*DIP / D_ID*/
    I_FA_MAC(F_KEY10,  dip,                       144,   32),
    /* udb5 */
    I_F_MAC(F_KEY10,   udb5,                      104,   8),

    I_F_MAC(F_KEY10,   isUdbValid,                206,   1),

    /*udb41*/
    I_F_MAC(F_KEY10,   udb41_44[0],       208, 8),
    /*udb42*/
    I_F_MAC(F_KEY10,   udb41_44[1],       216, 8),
    /*udb43*/
    I_F_MAC(F_KEY10,   udb41_44[2],       224, 8),
    /*udb44*/
    I_F_MAC(F_KEY10,   udb41_44[3],       232, 8),

    /* overlapping field */

    /* 8LSBs of VRF ID */
    I_F_MAC(F_KEY10,   vrfId,               104,   8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* for Bobcat2, Caelum, Bobcat3 -- fmtFldLion2_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key11_flds[] =
{
    /* the xcat format */
    I_T_MAC(F_KEY11,   ingressIpCommon, fmtFld_INGRESS_IP_COMMON_sip5),
    I_F_MAC(F_KEY11,   pktType,               39,    3),
    I_F_MAC(F_KEY11,   isUdbValid,            57,    1),
    /* split the field */
    I_FO_MAC(F_KEY11,  ipPacketLength,       51,   4,  0),
    I_FO_MAC(F_KEY11,  ipPacketLength,      279,   8,  4),
    I_FO_MAC(F_KEY11,  ipPacketLength,      504,   4, 12),

    /*SIP[31:0]*/
    I_FA_MAC(F_KEY11,  sip.arIP[12],         112,   32),
    /*DIP[31:0]*/
    I_FA_MAC(F_KEY11,  dip.arIP[12],         144,   32),
    /*udb 18*/
    I_F_MAC(F_KEY11,   udb17_22[1],          104,    8),
    /*udb 19*/
    I_F_MAC(F_KEY11,   udb17_22[2],          176,    8),
    /*udb 20*/
    I_F_MAC(F_KEY11,   udb17_22[3],          184,    8),

    I_FO_MAC(F_KEY11,  ingressIpCommon.tos, 232,   1, 1),
    I_F_MAC(F_KEY11,   isNd,                233,   1),
    I_F_MAC(F_KEY11,   ipv6HdrFlowLabel,    234,  20),
    I_F_MAC(F_KEY11,   isIpv6LinkLocal,     254,   1),
    I_F_MAC(F_KEY11,   mplsOuterLabel,      255,  20),
    I_F_MAC(F_KEY11,   mplsOuterLabExp,     275,   3),
    I_F_MAC(F_KEY11,   mplsOuterLabSBit,    278,   1),
    I_F_MAC(F_KEY11,   ttl,                 287,   8),
    /*SIP[127:32]*/
    I_FA_MAC(F_KEY11,  sip.arIP[0],         295,  96),
    /*DIP[39:32]*/
    I_F_MAC(F_KEY11,  dip.arIP[11],        391,   8),
    /*DIP[127:40]*/
    I_FA_MAC(F_KEY11,  dip.arIP[0],         416,   88),
    /*udb 17*/
    I_F_MAC(F_KEY11,   udb17_22[0],         224,   8),
    /*udb 21*/
    I_F_MAC(F_KEY11,   udb17_22[4],         208,   8),
    /*udb 22*/
    I_F_MAC(F_KEY11,   udb17_22[5],         216,   8),

    I_F_MAC(F_KEY11,   isIpv6ExtHdrExist,   511,   1),
    I_F_MAC(F_KEY11,   isIpv6HopByHop,      510,   1),
    I_F_MAC(F_KEY11,   isIpv6Mld,           509,   1),
    I_F_MAC(F_KEY11,   ipHeaderOk,          512,   1),
    /*udb 0*/
    I_F_MAC(F_KEY11,   udb0_11[0],          513,   8),
    /*udb 1*/
    I_F_MAC(F_KEY11,   udb0_11[1],          521,   8),
    /*udb 2*/
    I_F_MAC(F_KEY11,   udb0_11[2],          529,   8),
    /*udb 3*/
    I_F_MAC(F_KEY11,   udb0_11[3],          537,   8),
    /*udb 4*/
    I_F_MAC(F_KEY11,   udb0_11[4],          545,   8),
    /*udb 5*/
    I_F_MAC(F_KEY11,   udb0_11[5],          553,   8),
    /*udb 6*/
    I_F_MAC(F_KEY11,   udb0_11[6],          561,   8),
    /*udb 7*/
    I_F_MAC(F_KEY11,   udb0_11[7],          569,   8),
    /*udb 8*/
    I_F_MAC(F_KEY11,   udb0_11[8],          577,   8),
    /*udb 9*/
    I_F_MAC(F_KEY11,   udb0_11[9],          585,   8),
    /*udb 10*/
    I_F_MAC(F_KEY11,   udb0_11[10],         593,   8),
    /*udb 11*/
    I_F_MAC(F_KEY11,   udb0_11[11],         601,   8),

    /*udb 45*/
    I_F_MAC(F_KEY11,   udb45_46[0],         624,   8),
    /*udb 46*/
    I_F_MAC(F_KEY11,   udb45_46[1],         632,   8),

    /* split the field */
    /*SRCePort/Trunk*/
    I_FO_MAC(F_KEY11,  srcPortOrTrunk,             58, 6, 0),
    I_FO_MAC(F_KEY11,  srcPortOrTrunk,            399, 7, 6),

    I_F_MAC(F_KEY11,  srcIsTrunk,                 407, 1),

    /* split the field */
    I_FO_MAC(F_KEY11,  ingressIpCommon.vid,       408,   1,   12),

    I_F_MAC(F_KEY11,   tag1Exist,               409,  1),
    I_F_MAC(F_KEY11,    up1,                    410,   3),

    I_F_MAC(F_KEY11,    vid1,                   609,  12),
    I_F_MAC(F_KEY11,    cfi1,                   621,   1),

    /* overlapping field */

    /* split the field */
    I_FO_MAC(F_KEY11,   vrfId,               104,   8,  0),
    I_FO_MAC(F_KEY11,   vrfId,               176,   4,  8),

    I_F_MAC(F_KEY11,    trunkHash,           184,    6),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_key12_sip5[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_basic1_sip5),
    I_F_MAC(F_CI,   isL2Valid,      189, 1),
    I_F_MAC(F_CI,   isUdbValid,     206, 1),


    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_IP_key12_sip5[] =
{
    I_F_MAC(F_CISI,   isIpv4,             40,  1),
    I_F_MAC(F_CISI,   ipProtocol,         42,  8),
    I_F_MAC(F_CISI,   dscp,               50,  6),
    I_F_MAC(F_CISI,   isL4Valid,          56,  1),
    I_F_MAC(F_CISI,   ipHeaderOk,         191, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/*fmtFld_INGRESS_STD_IPV6_DIP_sip5*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_sip5_key12_flds[] =
{
    I_T_MAC(F_KEY12,    common, fmtFld_INGRESS_COMMON_key12_sip5),
    I_T_MAC(F_KEY12,    commonStdIp, fmtFld_INGRESS_COMMON_IP_key12_sip5),
    /*DIP[15:0]*/
    I_FA_MAC(F_KEY12,  dip.arIP[14],       57,  16),
    I_F_MAC(F_KEY12,   isArp,              73,   1),
    I_F_MAC(F_KEY12,   isIpv6ExtHdrExist,  74,   1),
    /*DIP[31:16]*/
    I_FA_MAC(F_KEY12,  dip.arIP[12],       75,  16),
    I_F_MAC(F_KEY12,   isIpv6HopByHop,     91,   1),
    /*DIP[127:32]*/
    I_FA_MAC(F_KEY12,  dip.arIP[0],        92,  96),

    /*udb47*/
    I_F_MAC(F_KEY12,   udb47_49[0],       208, 8),
    /*udb48*/
    I_F_MAC(F_KEY12,   udb47_49[1],       216, 8),
    /*udb49*/
    I_F_MAC(F_KEY12,   udb47_49[2],       224, 8),
    /*udb0*/
    I_F_MAC(F_KEY12,   udb0,              232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};



/* The CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC fiels in all keys */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_EXT_xCat[] =
{
    I_F_MAC(F_CIE,   isIpv6,           40,  1),
    I_F_MAC(F_CIE,   ipProtocol,       48,  8),
    I_F_MAC(F_CIE,   dscp,             42,  6),
    I_F_MAC(F_CIE,   isL4Valid,        104, 1),
    I_F_MAC(F_CIE,   l4Byte0,          80,  8),
    I_F_MAC(F_CIE,   l4Byte1,          88,  8),
    I_F_MAC(F_CIE,   l4Byte2,          72,  8),
    I_F_MAC(F_CIE,   l4Byte3,          64,  8),
    I_F_MAC(F_CIE,   l4Byte13,         96,  8),
    I_F_MAC(F_CIE,   ipHeaderOk,       109, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC fiels in all keys */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_STD_IP_xCat[] =
{
    I_F_MAC(F_CISI,   isIpv4,             40,  1),
    I_F_MAC(F_CISI,   ipProtocol,         48,  8),
    I_F_MAC(F_CISI,   dscp,               42,  6),
    I_F_MAC(F_CISI,   isL4Valid,          104, 1),
    I_F_MAC(F_CISI,   l4Byte2,            72,  8),
    I_F_MAC(F_CISI,   l4Byte3,            64,  8),
    I_F_MAC(F_CISI,   ipHeaderOk,         109, 1),
    I_F_MAC(F_CISI,   ipv4Fragmented,     108, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_NOT_IP_xCat_flds[] =
{
    I_F_MAC(F_RISNI,   isIpv4,            40,  1),
    I_F_MAC(F_RISNI,   etherType,         42,  16),
    I_F_MAC(F_RISNI,   isArp,             105, 1),
    I_FO_MAC(F_RISNI,  l2Encap,           108, 1,  0),
    I_FA_MAC(F_RISNI,  macDa,             144, 48),
    I_FA_MAC(F_RISNI,  macSa.arEther[3],  80,  24),
    I_FA_MAC(F_RISNI,  macSa,            120, 24),
    I_F_MAC(F_RISNI,   udb15_17[0],       64,  8),
    I_F_MAC(F_RISNI,   udb15_17[1],       72,  8),
    I_F_MAC(F_RISNI,   udb15_17[2],       112, 8),
    I_F_MAC(F_RISNI,   vrfId,             64,  12),
    I_F_MAC(F_RISNI,   trunkHash,         112, 6),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_NOT_IP_xCat2_flds[] =
{
    I_ST_MAC(fmtFld_INGRESS_STD_NOT_IP_xCat_flds),
    I_FO_MAC(F_RISNI,  l2Encap,           109, 1,  1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_NOT_IP_xCat[] =
{
    I_T_MAC(F_RISNI,    common, fmtFld_INGRESS_COMMON_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_NOT_IP_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_NOT_IP_xCat2[] =
{
    I_T_MAC(F_RISNI,    common, fmtFld_INGRESS_COMMON_xCat2),
    I_ST_MAC(fmtFld_INGRESS_STD_NOT_IP_xCat2_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IP_L2_QOS_xCat_flds[] =
{
    I_F_MAC(F_RISIL2,   isArp,              105,  1),
    I_F_MAC(F_RISIL2,   isIpv6ExtHdrExist,  110,  1),
    I_F_MAC(F_RISIL2,   isIpv6HopByHop,     111,  1),
    I_FA_MAC(F_RISIL2,  macDa,              144,  48),
    I_FA_MAC(F_RISIL2,  macSa.arEther[3],    80,  24),
    I_FA_MAC(F_RISIL2,  macSa,              120,  24),
    I_F_MAC(F_RISIL2,   udb18_19[0],         56,  8),
    I_F_MAC(F_RISIL2,   udb18_19[1],        112,  8),
    I_FO_MAC(F_RISIL2,  vrfId,               56,  8,  0),
    I_FO_MAC(F_RISIL2,  vrfId,               112, 4,  8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IP_L2_QOS_xCat[] =
{
    I_T_MAC(F_RISIL2,    common,      fmtFld_INGRESS_COMMON_xCat),
    I_T_MAC(F_RISIL2,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IP_L2_QOS_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IP_L2_QOS_xCat2[] =
{
    I_T_MAC(F_RISIL2,    common,      fmtFld_INGRESS_COMMON_xCat2),
    I_T_MAC(F_RISIL2,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IP_L2_QOS_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV4_L4_xCat_flds[] =
{
    I_F_MAC(F_RIS4L4,   isArp,              105,  1),
    I_F_MAC(F_RIS4L4,   isBc,               110,  1),
    I_FA_MAC(F_RIS4L4,  sip,                112,  32),
    I_FA_MAC(F_RIS4L4,  dip,                144,  32),
    I_F_MAC(F_RIS4L4,   l4Byte0,             80,  8),
    I_F_MAC(F_RIS4L4,   l4Byte1,             88,  8),
    I_F_MAC(F_RIS4L4,   l4Byte13,            96,  8),
    I_F_MAC(F_RIS4L4,   udb20_22[0],         56,  8),
    I_F_MAC(F_RIS4L4,   udb20_22[1],        176,  8),
    I_F_MAC(F_RIS4L4,   udb20_22[2],        184,  8),
    I_FO_MAC(F_RIS4L4,  vrfId,               56,  8,  0),
    I_FO_MAC(F_RIS4L4,  vrfId,              176,  4,  8),
    I_F_MAC(F_RIS4L4,   trunkHash,          184,  6),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV4_L4_xCat[] =
{
    I_T_MAC(F_RIS4L4,    common,      fmtFld_INGRESS_COMMON_xCat),
    I_T_MAC(F_RIS4L4,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IPV4_L4_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV4_L4_xCat2[] =
{
    I_T_MAC(F_RIS4L4,    common,      fmtFld_INGRESS_COMMON_xCat2),
    I_T_MAC(F_RIS4L4,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IPV4_L4_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_NOT_IPV6_Seg0_xCat_flds[] =
{
    I_F_MAC(F_RIEN6,    udb0_5[5],         56,   8),
    I_FO_MAC(F_RIEN6,   vrfId,             56,   8,  0),
    I_F_MAC(F_RIEN6,   ipv4Fragmented,    108,   1),
    I_FO_MAC(F_RIEN6,   l2Encap,          105,   1,  0),
    I_FA_MAC(F_RIEN6,   sip,              112,  32),
    I_FA_MAC(F_RIEN6,   dip,              144,  32),
    I_F_MAC(F_RIEN6,    etherType,        176,  16),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_NOT_IPV6_Seg0_xCat2_flds[] =
{
    I_ST_MAC(fmtFld_INGRESS_EXT_NOT_IPV6_Seg0_xCat_flds),
    I_FO_MAC(F_RIEN6,   l2Encap,          110,   1,  1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_NOT_IPV6_Seg1_xCat_flds[] =
{
    I_FA_MAC(F_RIEN6,   macSa,               56,  48),
    I_FA_MAC(F_RIEN6,   macDa,              104,  48),
    I_FAS_MAC(F_RIEN6,  udb0_5[1],          152,  32),
    I_FAS_MAC(F_RIEN6,  udb0_5[0],          184,   8),
    I_FO_MAC(F_RIEN6,   vrfId,              152,   4,  8),
    I_F_MAC(F_RIEN6,    trunkHash,          168,   6),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_NOT_IPV6_xCat[] =
{
    I_T_MAC(F_RIEN6,    common,      fmtFld_INGRESS_COMMON_xCat),
    I_T_MAC(F_RIEN6,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_NOT_IPV6_Seg0_xCat_flds),
    I_STHO_MAC(6, fmtFld_INGRESS_EXT_NOT_IPV6_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_NOT_IPV6_xCat2[] =
{
    I_T_MAC(F_RIEN6,    common,      fmtFld_INGRESS_COMMON_xCat2),
    I_T_MAC(F_RIEN6,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_NOT_IPV6_Seg0_xCat2_flds),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_INGRESS_EXT_NOT_IPV6_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L2_Seg0_xCat_flds[] =
{
    I_F_MAC(F_RIE6L2,    udb6_11[5],           56,  8),
    I_FO_MAC(F_RIE6L2,   vrfId,                56,   8,  0),
    I_FA_MAC(F_RIE6L2,   sip.arIP[6],          112,  80),
    I_F_MAC(F_RIE6L2,    isIpv6ExtHdrExist,    105,  1),
    I_F_MAC(F_RIE6L2,    isIpv6HopByHop,       108,  1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L2_Seg1_xCat_flds[] =
{
    I_FA_MAC(F_RIE6L2,   sip.arIP[0],          0,  48),
    I_F_MAC(F_RIE6L2,    dipBits127to120,     48,   8),
    I_FA_MAC(F_RIE6L2,   macSa,               56,  48),
    I_FA_MAC(F_RIE6L2,   macDa,              104,  48),
    I_FAS_MAC(F_RIE6L2,  udb6_11[0],         152,  40),
    I_FO_MAC(F_RIE6L2,   vrfId,              152,   4,  8),
    I_F_MAC(F_RIE6L2,    trunkHash,          160,   6),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L2_xCat[] =
{
    I_T_MAC(F_RIE6L2,    common,      fmtFld_INGRESS_COMMON_xCat),
    I_T_MAC(F_RIE6L2,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_IPV6_L2_Seg0_xCat_flds),
    I_STHO_MAC(6, fmtFld_INGRESS_EXT_IPV6_L2_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L2_xCat2[] =
{
    I_T_MAC(F_RIE6L2,    common,      fmtFld_INGRESS_COMMON_xCat2),
    I_T_MAC(F_RIE6L2,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_IPV6_L2_Seg0_xCat_flds),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_INGRESS_EXT_IPV6_L2_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L4_Seg0_xCat_flds[] =
{
    I_FA_MAC(F_RIE6L4,   sip.arIP[12],          112,  32),
    I_FA_MAC(F_RIE6L4,   dip.arIP[12],          144,  32),
    I_F_MAC(F_RIE6L4,    isIpv6ExtHdrExist,     105,   1),
    I_F_MAC(F_RIE6L4,    isIpv6HopByHop,        108,   1),
    I_FA_MAC(F_RIE6L4,   sip.arIP[10],          176,  16),
    I_F_MAC(F_RIE6L4,    udb12_14[2],            56,   8),
    I_FO_MAC(F_RIE6L4,   vrfId,                  56,   8,  0),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L4_Seg1_xCat_flds[] =
{
    I_FA_MAC(F_RIE6L4,   sip.arIP[0],          0,  80),
    I_FA_MAC(F_RIE6L4,   dip.arIP[0],         80,   8),
    I_FA_MAC(F_RIE6L4,   dip.arIP[1],         88,  88),
    I_FAS_MAC(F_RIE6L4,  udb12_14[0],        176,  16),
    I_FO_MAC(F_RIE6L4,   vrfId,              176,   4,  8),
    I_F_MAC(F_RIE6L4,    trunkHash,          184,   6),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L4_xCat[] =
{
    I_T_MAC(F_RIE6L4,    common,      fmtFld_INGRESS_COMMON_xCat),
    I_T_MAC(F_RIE6L4,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_IPV6_L4_Seg0_xCat_flds),
    I_STHO_MAC(6, fmtFld_INGRESS_EXT_IPV6_L4_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_IPV6_L4_xCat2[] =
{
    I_T_MAC(F_RIE6L4,    common,      fmtFld_INGRESS_COMMON_xCat2),
    I_T_MAC(F_RIE6L4,    commonExt,   fmtFld_INGRESS_COMMON_EXT_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_IPV6_L4_Seg0_xCat_flds),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_INGRESS_EXT_IPV6_L4_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_UDB_xCat[] =
{
    I_F_MAC(F_CIU,   pclId,             0,  10),
    I_F_MAC(F_CIU,   macToMe,           10,  1),
    I_F_MAC(F_CIU,   sourcePort,        11,  6),
    I_FO_MAC(F_CIU,  pktTagging,        17,  1, 0),
    I_FO_MAC(F_CIU,  pktTagging,        33,  1, 1),
    I_F_MAC(F_CIU,   vid,               18, 12),
    I_F_MAC(F_CIU,   up,                30,  3),
    I_F_MAC(F_CIU,   l3OffsetInvalid,   34,  1),
    I_F_MAC(F_CIU,   l4ProtocolType,    35,  2),
    I_FO_MAC(F_CIU,  pktType,           37,  2, 0),
    I_FO_MAC(F_CIU,  pktType,           83,  6, 2),
    I_F_MAC(F_CIU,   ipHeaderOk,        39,  1),
    I_F_MAC(F_CIU,   isIp,              41,  1),
    I_F_MAC(F_CIU,   dscpOrExp,         42,  6),
    I_F_MAC(F_CIU,   macDaType,         80,  2),
    I_F_MAC(F_CIU,   l4OffsetInvalid,   82,  1),
    I_F_MAC(F_CIU,   l2Encapsulation,   89,  2),
    I_F_MAC(F_CIU,   isIpv6HopByHop,    91,  1),
    I_F_MAC(F_CIU,   isIpv6Eh,          92,  1),
    I_F_MAC(F_CIU,   isUdbValid,        94,  1),
    I_F_MAC(F_CIU,   isL2Valid,         95,  1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_UDB_xCat2[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_UDB_xCat),
    I_FO_MAC(F_CIU,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CIU,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CIU,   portListBmp.ports[0],         192,  14,  14),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_UDB_xCat_flds[] =
{
    I_F_MAC(F_RISU,   isIpv4,            40,  1),
    I_FAS_MAC(F_RISU, udb0_15[0],        48,  24),
    I_F_MAC(F_RISU,   vrfId,             48,  12),
    I_F_MAC(F_RISU,   udb0_15[5],        72,   8),
    I_FAS_MAC(F_RISU, udb0_15[3],        96,  16),
    I_F_MAC(F_RISU,   trunkHash,         96,   6),
    I_FAS_MAC(F_RISU, udb0_15[6],       112,  80),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_UDB_xCat[] =
{
    I_T_MAC(F_RISU,    commonIngrUdb, fmtFld_INGRESS_COMMON_UDB_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_UDB_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_UDB_xCat2[] =
{
    I_T_MAC(F_RISU,    commonIngrUdb, fmtFld_INGRESS_COMMON_UDB_xCat2),
    I_ST_MAC(fmtFld_INGRESS_STD_UDB_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC fiels */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_UDB_Seg0_xCat_flds[] =
{
    I_F_MAC(F_RIEU,    isIpv6,                    40,    1),
    I_F_MAC(F_RIEU,    ipProtocol,                48,    8),
    I_FAS_MAC(F_RIEU,  udb0_15[1],                56,   16),
    I_F_MAC(F_RIEU,    vrfId,                     56,   12),
    I_F_MAC(F_RIEU,    udb0_15[5],                72,    8),
    I_F_MAC(F_RIEU,    qosProfile,                72,    7),
    I_FAS_MAC(F_RIEU,  udb0_15[3],                96,   16),
    I_F_MAC(F_RIEU,    trunkHash,                 96,    6),
    I_FAS_MAC(F_RIEU,  udb0_15[6],               112,   32),
    I_FA_MAC(F_RIEU,   sipBits31_0[0],           144,   32),
    I_FA_MAC(F_RIEU,   sipBits79_32orMacSa[4],   176,   16),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_UDB_Seg1_xCat_flds[] =
{
    I_FA_MAC(F_RIEU,   sipBits79_32orMacSa[0],     0,   32),
    I_FA_MAC(F_RIEU,   sipBits127_80orMacDa[0],   32,   48),
    I_FA_MAC(F_RIEU,   dipBits127_112[0],         80,   16),
    I_FA_MAC(F_RIEU,   dipBits31_0[0],            96,   32),
    I_FAS_MAC(F_RIEU,  udb0_15[10],              128,   48),
    I_F_MAC(F_RIEU,    udb0_15[0],               176,    8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_UDB_xCat[] =
{
    I_T_MAC(F_RIEU,    commonIngrUdb,      fmtFld_INGRESS_COMMON_UDB_xCat),
    I_ST_MAC(fmtFld_INGRESS_EXT_UDB_Seg0_xCat_flds),
    I_STHO_MAC(6, fmtFld_INGRESS_EXT_UDB_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_EXT_UDB_xCat2[] =
{
    I_T_MAC(F_RIEU,    commonIngrUdb,      fmtFld_INGRESS_COMMON_UDB_xCat2),
    I_ST_MAC(fmtFld_INGRESS_EXT_UDB_Seg0_xCat_flds),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_INGRESS_EXT_UDB_Seg1_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC fiels */
/* reduced common and commonStdIp for *STD_IPV6_DIP     */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_IPV6_DIP_xCat[] =
{
    I_F_MAC(F_CI,   pclId,          0,  10),
    I_F_MAC(F_CI,   macToMe,        10,  1),
    I_F_MAC(F_CI,   sourcePort,     11,  6),
    I_F_MAC(F_CI,   isTagged,       17,  1),
    I_F_MAC(F_CI,   vid,            18, 12),
    I_F_MAC(F_CI,   up,             30,  3),
    I_F_MAC(F_CI,   qosProfile,     33,  7),
    I_F_MAC(F_CI,   isIp,           41,  1),
    I_F_MAC(F_CI,   isL2Valid,      189, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_IPV6_DIP_xCat2[] =
{
    I_ST_MAC(fmtFld_INGRESS_COMMON_IPV6_DIP_xCat),
    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_COMMON_STD_IP_IPV6_DIP_xCat[] =
{
    I_F_MAC(F_CISI,   isIpv4,             40,  1),
    I_F_MAC(F_CISI,   ipProtocol,         42,  8),
    I_F_MAC(F_CISI,   dscp,               50,  6),
    I_F_MAC(F_CISI,   isL4Valid,          56,  1),
    I_F_MAC(F_CISI,   ipHeaderOk,         191, 1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV6_DIP_xCat_flds[] =
{
    I_FA_MAC(F_RIS6D,  dip.arIP[14],       57,  16),
    I_F_MAC(F_RIS6D,   isArp,              73,   1),
    I_F_MAC(F_RIS6D,   isIpv6ExtHdrExist,  74,   1),
    I_FA_MAC(F_RIS6D,  dip.arIP[12],       75,  16),
    I_F_MAC(F_RIS6D,   isIpv6HopByHop,     91,   1),
    I_FA_MAC(F_RIS6D,  dip.arIP[0],        92,  96),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV6_DIP_xCat[] =
{
    I_T_MAC(F_RIS6D,    common,      fmtFld_INGRESS_COMMON_IPV6_DIP_xCat),
    I_T_MAC(F_RIS6D,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_IPV6_DIP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IPV6_DIP_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_INGRESS_STD_IPV6_DIP_xCat2[] =
{
    I_T_MAC(F_RIS6D,    common,      fmtFld_INGRESS_COMMON_IPV6_DIP_xCat2),
    I_T_MAC(F_RIS6D,    commonStdIp, fmtFld_INGRESS_COMMON_STD_IP_IPV6_DIP_xCat),
    I_ST_MAC(fmtFld_INGRESS_STD_IPV6_DIP_xCat_flds),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_xCat_valid[] =
{
    /* valid bit moved to bit 191                 */
    /* other bits moved xCa2 compatible positions */
    I_F_MAC(F_CE,   valid,              191,   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields that in the same positions for STD and EXT rules */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_xCat_base[] =
{
    I_F_MAC(F_CE,   pclId,                 0,   10),
    I_FO_MAC(F_CE,   sourcePort,          10,    6,  0),
    I_F_MAC(F_CE,   isTagged,             16,    1),
    I_F_MAC(F_CE,   vid,                  17,   12),
    I_F_MAC(F_CE,   up,                   29,    3),
    I_F_MAC(F_CE,   fwdData.qosProfile,   32,    7),
    I_FO_MAC(F_CE,  toCpu.cpuCode,        32,    7,  0),
    I_F_MAC(F_CE,   fromCpu.dp,           32,    2),
    I_F_MAC(F_CE,   fromCpu.tc,           34,    3),
    I_F_MAC(F_CE,   isIp,                 40,    1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields from SEG0 for STD rules */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_xCat_std_seg0[] =
{
    I_F_MAC(F_CE,   srcHwDev,                    74,    5),
    I_F_MAC(F_CE,   fwdData.srcTrunkId,        74,    7),
    I_F_MAC(F_CE,   fromCpu.egrFilterEnable,   79,    1),
    I_F_MAC(F_CE,   toAnalyzer.rxSniff,        80,    1),
    I_F_MAC(F_CE,   toCpu.srcTrg,              80,    1),
    I_F_MAC(F_CE,   fwdData.srcIsTrunk,        81,    1),
    I_F_MAC(F_CE,   sourceId,                  82,    5),
    I_F_MAC(F_CE,   fwdData.isRouted,          87,    1),
    I_F_MAC(F_CE,   fwdData.isUnknown,         88,    1),
    I_FO_MAC(F_CE,  toCpu.cpuCode,             88,    1,  7),
    I_F_MAC(F_CE,   isL2Valid,                 89,    1),
    I_F_MAC(F_CE,   egrPacketType,            187,    2),
    I_F_MAC(F_CE,   isVidx,                   190,    1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields from SEG1 for EXT rules */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_xCat_ext_seg1[] =
{
    I_F_MAC(F_CE,   srcHwDev,                   163,    5),
    I_F_MAC(F_CE,   fwdData.srcTrunkId,       163,    7),
    I_F_MAC(F_CE,   fromCpu.egrFilterEnable,  168,    1),
    I_F_MAC(F_CE,   toAnalyzer.rxSniff,       169,    1),
    I_F_MAC(F_CE,   toCpu.srcTrg,             169,    1),
    I_F_MAC(F_CE,   fwdData.srcIsTrunk,       170,    1),
    I_F_MAC(F_CE,   sourceId,                 171,    5),
    I_FO_MAC(F_CE,  toCpu.cpuCode,            177,    1,  7),
    I_F_MAC(F_CE,   fwdData.isRouted,         176,    1),
    I_F_MAC(F_CE,   fwdData.isUnknown,        177,    1),
    I_F_MAC(F_CE,   isL2Valid,                178,    1),
    I_F_MAC(F_CE,   egrPacketType,            189,    2),
    I_F_MAC(F_CE,   isVidx,                   191,    1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_xCat2_base[] =
{
    I_ST_MAC(fmtFld_EGRESS_COMMON_xCat_base),
    I_FO_MAC(F_CE,  portListBmp.ports[0],         0,   8,  0),
    I_FO_MAC(F_CE,  portListBmp.ports[0],        32,   6,  8),
    I_FO_MAC(F_CE,  portListBmp.ports[0],       192,  14,  14),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* common to egress keys 0,1,2,3,4,5 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_012345[] =
{
    I_F_MAC(F_CE,   pclId,                 0,   10),
    I_FO_MAC(F_CE,   sourcePort,          10,    6,  0),
    I_F_MAC(F_CE,   isTagged,             16,    1),
    I_F_MAC(F_CE,   vid,                  17,   12),
    I_F_MAC(F_CE,   up,                   29,    3),
    I_F_MAC(F_CE,   fwdData.qosProfile,   32,    7),
    I_FO_MAC(F_CE,  toCpu.cpuCode,        32,    7,  0),
    I_F_MAC(F_CE,   fromCpu.dp,           32,    2),
    I_F_MAC(F_CE,   fromCpu.tc,           34,    3),
    I_F_MAC(F_CE,   isIp,                 40,    1),

    I_F_MAC(F_CE,  tag1Exist,         191,   1),

    I_FO_MAC(F_CI,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI,   portListBmp.ports[0],          32,   6,   8),
    I_FO_MAC(F_CI,   portListBmp.ports[0],         192,  14,  14),


    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* common to egress keys 0,1,2 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_012[] =
{
    I_F_MAC(F_CE,   srcHwDev,                    74,    5),
    I_F_MAC(F_CE,   fwdData.srcTrunkId,        74,    7),
    I_F_MAC(F_CE,   fromCpu.egrFilterEnable,   79,    1),
    I_F_MAC(F_CE,   toAnalyzer.rxSniff,        80,    1),
    I_F_MAC(F_CE,   toCpu.srcTrg,              80,    1),
    I_F_MAC(F_CE,   fwdData.srcIsTrunk,        81,    1),
    I_F_MAC(F_CE,   sourceId,                  82,    5),
    I_F_MAC(F_CE,   fwdData.isRouted,          87,    1),
    I_F_MAC(F_CE,   fwdData.isUnknown,         88,    1),
    I_FO_MAC(F_CE,  toCpu.cpuCode,             88,    1,  7),
    I_F_MAC(F_CE,   isL2Valid,                 89,    1),
    I_F_MAC(F_CE,   egrPacketType,            187,    2),
    I_F_MAC(F_CE,   isVidx,                   190,    1),

    I_F_MAC(F_CE,   isUdbValid,               206,    1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC

};

/* common STD IP to egress keys 1,2 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_12_STD_IP[] =
{
    I_F_MAC(F_CESI,   isIpv4,           39,   1),
    I_F_MAC(F_CESI,   ipProtocol,       41,  8),
    I_F_MAC(F_CESI,   dscp,             49,  6),
    I_F_MAC(F_CESI,   isL4Valid,        55,  1),
    I_F_MAC(F_CESI,   l4Byte2,          64,  8),
    I_F_MAC(F_CESI,   l4Byte3,          56,  8),

    I_F_MAC(F_CESI,  egrTcpUdpPortComparator,     90,   8),
    I_F_MAC(F_CESI,  ipv4Fragmented,             189,   1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* common to egress keys 3,4,5 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_345[] =
{
    I_FO_MAC(F_CE,   srcHwDev,                   371,    5 , 0),
    I_FO_MAC(F_CE,   srcHwDev,                   400,    7 , 5),


    I_FO_MAC(F_CE,   fwdData.srcTrunkId,       371,    7 , 0),
    I_FO_MAC(F_CE,   fwdData.srcTrunkId,       402,    5 , 7),


    I_F_MAC(F_CE,   fromCpu.egrFilterEnable,  376,    1),
    I_F_MAC(F_CE,   toAnalyzer.rxSniff,       377,    1),
    I_F_MAC(F_CE,   toCpu.srcTrg,             377,    1),
    I_F_MAC(F_CE,   fwdData.srcIsTrunk,       378,    1),

    I_FO_MAC(F_CE,   sourceId,                 379,    5 , 0),
    I_FO_MAC(F_CE,   sourceId,                 407,    7 , 5),

    I_FO_MAC(F_CE,  toCpu.cpuCode,            385,    1,  7),
    I_F_MAC(F_CE,   fwdData.isRouted,         384,    1),
    I_F_MAC(F_CE,   fwdData.isUnknown,        385,    1),
    I_F_MAC(F_CE,   isL2Valid,                386,    1),
    I_FO_MAC(F_CE,   sourcePort,              395,    2,  6),
    I_F_MAC(F_CE,   egrPacketType,            397,    2),
    I_F_MAC(F_CE,   isVidx,                   399,    1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* common extended to egress keys 3,4,5 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_345_EXT[] =
{
    I_F_MAC(F_CEE,  isIpv6,           39,   1),
    I_F_MAC(F_CEE,  ipProtocol,       41,   8),
    I_F_MAC(F_CEE,  dscp,             49,   6),
    I_F_MAC(F_CEE,  isL4Valid,        55,   1),
    I_F_MAC(F_CEE,  l4Byte3,          56,   8),
    I_F_MAC(F_CEE,  l4Byte2,          64,   8),
    I_F_MAC(F_CEE,  l4Byte13,         72,   8),
    I_F_MAC(F_CEE,  l4Byte1,          80,   8),
    I_F_MAC(F_CEE,  l4Byte0,          88,   8),

    I_F_MAC(F_CEE,  egrTcpUdpPortComparator,     387,   8),

    I_F_MAC(F_CEE,  isUdbValid,      414,   1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* common (ip common) to egress keys 6,7 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_sip5_keys_67[] =
{
    I_F_MAC(F_CE_IP,   pclId,                    0,   10),
    I_FO_MAC(F_CE_IP,  sourcePort,              11,   6 , 0),
    I_FO_MAC(F_CE_IP,  sourcePort,              49,   2 , 6),
    I_F_MAC(F_CE_IP,   qosProfile,              17,   7),
    I_F_MAC(F_CE_IP,   cpuCode,                 17,   7),
    I_F_MAC(F_CE_IP,   tag1Exist,               24,   1),
    I_F_MAC(F_CE_IP,   sourceId ,               25,   12),
    I_FO_MAC(F_CE_IP,  originalVid,             37,   12, 0),

    I_FO_MAC(F_CE_IP,   srcDevOrTrunkId,         51,   7 ,0),/*for trunk*/
    I_FO_MAC(F_CE_IP,   srcHwDev,                  51,   5 ,0),
    I_FO_MAC(F_CE_IP,   srcHwDev,                  58,   2 ,5),
    I_F_MAC(F_CE_IP,    egrFilterEnable,         56,    1),
    I_F_MAC(F_CE_IP,    srcTrg,                  57,   1),
    I_F_MAC(F_CE_IP,    rxSniff,                 57,   1),

    I_F_MAC(F_CE_IP,   isSrcTrunk,              60,   1),
    I_FA_MAC(F_CE_IP,  macSa,                   61,   48),
    I_FA_MAC(F_CE_IP,  macDa,                  109,   48),
    I_F_MAC(F_CE_IP,   isL2Valid,              157,   1),
    I_F_MAC(F_CE_IP,   isIp,                   158,   1),
    I_F_MAC(F_CE_IP,   ipProtocol,             159,   8),
    I_FO_MAC(F_CE_IP,  tos,                    167,   6,  2),
    I_FO_MAC(F_CE_IP,  tos,                    173,   2,  0),
    I_F_MAC(F_CE_IP,   ttl,                    175,   8),
    I_F_MAC(F_CE_IP,   isArp,                  183,   1),

    I_F_MAC(F_CE_IP,   tcpUdpPortComparators,  264,   8),
    I_F_MAC(F_CE_IP,   l4Byte13,               272,   8),
    I_F_MAC(F_CE_IP,   l4Byte1,                280,   8),
    I_F_MAC(F_CE_IP,   l4Byte0,                288,   8),
    I_F_MAC(F_CE_IP,   l4Byte3,                296,   8),
    I_F_MAC(F_CE_IP,   l4Byte2,                304,   8),
    I_F_MAC(F_CE_IP,   isL4Valid,              312,   1),
    I_F_MAC(F_CE_IP,   tc,                     313,   3),
    I_F_MAC(F_CE_IP,   dp,                     316,   2),
    I_F_MAC(F_CE_IP,   egrPacketType,          318,   2),
    I_F_MAC(F_CE_IP,   srcTrgOrTxMirror,       320,   1),
    I_F_MAC(F_CE_IP,   assignedUp,             321,   3),
    I_FO_MAC(F_CE_IP,  trgPhysicalPort,        324,   6, 0),
    /*I_F_MAC(F_CE_IP,   rxSniff,                330,   1), - duplicate instance */
    I_F_MAC(F_CE_IP,   isRouted,               331,   1),
    I_F_MAC(F_CE_IP,   isIpv6,                 332,   1),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_EXT_xCat_seg0[] =
{
    I_F_MAC(F_CEE,  isIpv6,             39,   1),
    I_F_MAC(F_CEE,  ipProtocol,         41,   8),
    I_F_MAC(F_CEE,  dscp,               49,   6),
    I_F_MAC(F_CEE,  isL4Valid,          55,   1),
    I_F_MAC(F_CEE,  l4Byte3,            56,   8),
    I_F_MAC(F_CEE,  l4Byte2,            64,   8),
    I_F_MAC(F_CEE,  l4Byte13,           72,   8),
    I_F_MAC(F_CEE,  l4Byte1,            80,   8),
    I_F_MAC(F_CEE,  l4Byte0,            88,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_EXT_xCat_seg1[] =
{
    I_F_MAC(F_CEE,  egrTcpUdpPortComparator,     179,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_COMMON_STD_IP_xCat[] =
{
    I_F_MAC(F_CESI,  isIpv4,                      39,   1),
    I_F_MAC(F_CESI,  ipProtocol,                  41,   8),
    I_F_MAC(F_CESI,  dscp,                        49,   6),
    I_F_MAC(F_CESI,  isL4Valid,                   55,   1),
    I_F_MAC(F_CESI,  l4Byte3,                     56,   8),
    I_F_MAC(F_CESI,  l4Byte2,                     64,   8),
    I_F_MAC(F_CESI,  egrTcpUdpPortComparator,     90,   8),
    I_F_MAC(F_CESI,  ipv4Fragmented,             189,   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_NOT_IP_xCat_fields[] =
{
    I_F_MAC(F_RESNI,  isIpv4,                      39,    1),
    I_F_MAC(F_RESNI,  etherType,                   42,   16),
    I_F_MAC(F_RESNI,  isArp,                       72,    1),
    I_F_MAC(F_RESNI,  l2Encap,                     90,    1),
    I_FA_MAC(F_RESNI,  macSa,                      91,   48),
    I_FA_MAC(F_RESNI,  macDa,                     139,  48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_NOT_IP_xCat[] =
{
    I_T_MAC(F_RESNI,  common, fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_RESNI,  common, fmtFld_EGRESS_COMMON_xCat_base),
    I_T_MAC(F_RESNI,  common, fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_ST_MAC(fmtFld_EGRESS_STD_NOT_IP_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_NOT_IP_xCat2[] =
{
    I_T_MAC(F_RESNI,  common, fmtFld_EGRESS_COMMON_xCat2_base),
    I_T_MAC(F_RESNI,  common, fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_ST_MAC(fmtFld_EGRESS_STD_NOT_IP_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key0 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_NOT_IP_sip5[] =
{
    I_T_MAC(F_EGR_KEY0,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY0,  common, fmtFld_EGRESS_COMMON_sip5_keys_012),

    I_F_MAC(F_EGR_KEY0,  isIpv4,                      39,    1),
    I_F_MAC(F_EGR_KEY0,  etherType,                   42,   16),
    I_F_MAC(F_EGR_KEY0,  isArp,                       72,    1),
    I_F_MAC(F_EGR_KEY0,  l2Encap,                     90,    1),
    I_FA_MAC(F_EGR_KEY0, macSa,                       91,   48),
    I_FA_MAC(F_EGR_KEY0, macDa,                      139,   48),

    I_FO_MAC(F_EGR_KEY0,  up1,       41,    1,  2),
    I_FO_MAC(F_EGR_KEY0,  up1,       70,    2,  0),

    I_F_MAC(F_EGR_KEY0,  vid1,       58,   12),
    I_F_MAC(F_EGR_KEY0,  cfi1,       189,   1),

    /*udb0*/
    I_F_MAC(F_EGR_KEY0,   udb0_3[0],       208, 8),
    /*udb1*/
    I_F_MAC(F_EGR_KEY0,   udb0_3[1],       216, 8),
    /*udb2*/
    I_F_MAC(F_EGR_KEY0,   udb0_3[2],       224, 8),
    /*udb3*/
    I_F_MAC(F_EGR_KEY0,   udb0_3[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key1 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IP_L2_QOS_sip5[] =
{
    I_T_MAC(F_EGR_KEY1,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY1,  common, fmtFld_EGRESS_COMMON_sip5_keys_012),

    I_T_MAC(F_EGR_KEY1,  commonStdIp, fmtFld_EGRESS_COMMON_sip5_keys_12_STD_IP),

    I_F_MAC(F_EGR_KEY1,  isArp,          72,     1),
    I_FA_MAC(F_EGR_KEY1, dipBits0to31,   98,    32),
    I_F_MAC(F_EGR_KEY1,  l4Byte13,      130,     8),
    I_FA_MAC(F_EGR_KEY1, macDa,         139,    48),

    /*udb4*/
    I_F_MAC(F_EGR_KEY1,   udb4_7[0],       208, 8),
    /*udb5*/
    I_F_MAC(F_EGR_KEY1,   udb4_7[1],       216, 8),
    /*udb6*/
    I_F_MAC(F_EGR_KEY1,   udb4_7[2],       224, 8),
    /*udb7*/
    I_F_MAC(F_EGR_KEY1,   udb4_7[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key2 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IPV4_L4_sip5[] =
{
    I_T_MAC(F_EGR_KEY2,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY2,  common, fmtFld_EGRESS_COMMON_sip5_keys_012),

    I_T_MAC(F_EGR_KEY2,  commonStdIp, fmtFld_EGRESS_COMMON_sip5_keys_12_STD_IP),

    I_F_MAC(F_EGR_KEY2,  isArp,          72,      1),
    I_F_MAC(F_EGR_KEY2,  isBc,           73,      1),
    I_FA_MAC(F_EGR_KEY2, sip,            98,     32),
    I_FA_MAC(F_EGR_KEY2, dip,           130,     32),
    I_F_MAC(F_EGR_KEY2,  l4Byte13,      162,      8),
    I_F_MAC(F_EGR_KEY2,  l4Byte1,       170,      8),
    I_F_MAC(F_EGR_KEY2,  l4Byte0,       178,      8),

    /*udb8*/
    I_F_MAC(F_EGR_KEY2,   udb8_11[0],       208, 8),
    /*udb9*/
    I_F_MAC(F_EGR_KEY2,   udb8_11[1],       216, 8),
    /*udb10*/
    I_F_MAC(F_EGR_KEY2,   udb8_11[2],       224, 8),
    /*udb11*/
    I_F_MAC(F_EGR_KEY2,   udb8_11[3],       232, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key3 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_NOT_IPV6_sip5[] =
{
    I_T_MAC(F_EGR_KEY3,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY3,  common, fmtFld_EGRESS_COMMON_sip5_keys_345),

    I_T_MAC(F_EGR_KEY3,  commonExt, fmtFld_EGRESS_COMMON_sip5_keys_345_EXT),

    I_FA_MAC(F_EGR_KEY3, sip,                         96,   32),
    I_FA_MAC(F_EGR_KEY3, dip,                        128,   32),
    I_F_MAC(F_EGR_KEY3,  l2Encap,                    160,    1),
    I_F_MAC(F_EGR_KEY3,  etherType,                  161,   16),
    I_F_MAC(F_EGR_KEY3,  ipv4Fragmented,             177,    1),
    I_FA_MAC(F_EGR_KEY3,  macSa,                     251,   48),
    I_FA_MAC(F_EGR_KEY3,  macDa,                     299,   48),
    I_F_MAC(F_EGR_KEY3,  vid1,                       208,   12),
    I_F_MAC(F_EGR_KEY3,  cfi1,                       220,    1),
    I_F_MAC(F_EGR_KEY3,  up1,                        221,    3),
    I_F_MAC(F_EGR_KEY3,  numOfMplsLabels,            224,    2),
    I_F_MAC(F_EGR_KEY3,  protocolTypeAfterMpls,      226,    2),
    I_F_MAC(F_EGR_KEY3,  mplsLabel1,                 228,   20),
    I_F_MAC(F_EGR_KEY3,  mplsExp1,                   248,    3),
    I_F_MAC(F_EGR_KEY3,  isMpls,                     347,    1),
    I_F_MAC(F_EGR_KEY3,  mplsLabel0,                 348,   20),
    I_F_MAC(F_EGR_KEY3,  mplsExp0,                   368,    3),

    /*udb12*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[0],       416, 8),
    /*udb13*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[1],       424, 8),
    /*udb14*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[2],       432, 8),
    /*udb15*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[3],       440, 8),
    /*udb16*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[4],       448, 8),
    /*udb17*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[5],       456, 8),
    /*udb18*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[6],       464, 8),
    /*udb19*/
    I_F_MAC(F_EGR_KEY3,  udb12_19[7],       472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key4 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L2_sip5[] =
{
    I_T_MAC(F_EGR_KEY4,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY4,  common, fmtFld_EGRESS_COMMON_sip5_keys_345),

    I_T_MAC(F_EGR_KEY4,  commonExt, fmtFld_EGRESS_COMMON_sip5_keys_345_EXT),

    I_FA_MAC(F_EGR_KEY4,  sip.arIP[5],              96,     88),
    I_FO_MAC(F_EGR_KEY4,  sip.arIP[4],             184,       7,   0),
    I_FO_MAC(F_EGR_KEY4,  sip.arIP[4],             208,       1,   7),
    I_FA_MAC(F_EGR_KEY4,  sip.arIP[0],             209,     32),
    I_F_MAC(F_EGR_KEY4,   isIpv6ExtHdrExist,       241,      1),
    I_F_MAC(F_EGR_KEY4,   isIpv6HopByHop,          242,      1),
    I_F_MAC(F_EGR_KEY4,   dipBits127to120,         243,      8),
    I_FA_MAC(F_EGR_KEY4,  macSa,                   251,     48),
    I_FA_MAC(F_EGR_KEY4,  macDa,                   299,     48),
    I_F_MAC(F_EGR_KEY4,   vid1,                    347,     12),
    I_F_MAC(F_EGR_KEY4,   cfi1,                    359,      1),
    I_F_MAC(F_EGR_KEY4,   up1,                     360,      3),

    /*udb20*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[0],       416, 8),
    /*udb21*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[1],       424, 8),
    /*udb22*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[2],       432, 8),
    /*udb23*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[3],       440, 8),
    /*udb24*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[4],       448, 8),
    /*udb25*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[5],       456, 8),
    /*udb26*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[6],       464, 8),
    /*udb27*/
    I_F_MAC(F_EGR_KEY4,  udb20_27[7],       472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key5 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L4_sip5[] =
{
    I_T_MAC(F_EGR_KEY5,  common, fmtFld_EGRESS_COMMON_sip5_keys_012345),
    I_T_MAC(F_EGR_KEY5,  common, fmtFld_EGRESS_COMMON_sip5_keys_345),

    I_T_MAC(F_EGR_KEY5,  commonExt, fmtFld_EGRESS_COMMON_sip5_keys_345_EXT),

    I_FA_MAC(F_EGR_KEY5,  sip.arIP[5],              96,     88),
    I_FO_MAC(F_EGR_KEY5,  sip.arIP[4],             184,     7,   0),
    I_FO_MAC(F_EGR_KEY5,  sip.arIP[4],             208,     1,   7),
    I_FA_MAC(F_EGR_KEY5,  sip.arIP[0],             209,     32),
    I_F_MAC(F_EGR_KEY5,   isIpv6ExtHdrExist,       241,      1),
    I_F_MAC(F_EGR_KEY5,   isIpv6HopByHop,          242,      1),
    I_F_MAC(F_EGR_KEY5,   dip.arIP[0],             243,    8),
    I_FA_MAC(F_EGR_KEY5,  dip.arIP[1],             251,   88),
    I_FA_MAC(F_EGR_KEY5,  dip.arIP[12],            339,   32),

    /*udb28*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[0],       416, 8),
    /*udb29*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[1],       424, 8),
    /*udb30*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[2],       432, 8),
    /*udb31*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[3],       440, 8),
    /*udb32*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[4],       448, 8),
    /*udb33*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[5],       456, 8),
    /*udb34*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[6],       464, 8),
    /*udb35*/
    I_F_MAC(F_EGR_KEY5,  udb28_35[7],       472, 8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key6 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_EXT_IPV4_RACL_VACL_sip5[] =
{
    I_T_MAC(F_EGR_KEY6,   egressIpCommon, fmtFld_EGRESS_COMMON_sip5_keys_67),

    I_F_MAC(F_EGR_KEY6,   sip.arIP[3],  184,   8),
    I_FA_MAC(F_EGR_KEY6,   sip.arIP[0],  208,   24),
    I_FA_MAC(F_EGR_KEY6,   dip,          232,   32),
    I_F_MAC(F_EGR_KEY6,    ipv4Options,  333,   1),
    I_F_MAC(F_EGR_KEY6,    isVidx,       334,   1),
    I_F_MAC(F_EGR_KEY6,    vid1,         335,   12),
    I_F_MAC(F_EGR_KEY6,    cfi1,         347,   1),
    I_F_MAC(F_EGR_KEY6,    up1,          348,   3),
    /* extra bits from fields of egressIpCommon */
    I_FO_MAC(F_EGR_KEY6,   egressIpCommon.srcDevOrTrunkId,         351,   5, 7),
    I_FO_MAC(F_EGR_KEY6,   egressIpCommon.srcHwDev,                  351,   5, 7),
    I_FO_MAC(F_EGR_KEY6,   egressIpCommon.trgPhysicalPort,         356,   2, 6),

    I_F_MAC(F_EGR_KEY6,   isUdbValid,       358,   1),

    /*udb 0*/
    I_F_MAC(F_EGR_KEY6,   udb0,             472,   8),
    /*udb 36*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[0],      360,   8),
    /*udb 37*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[1],      368,   8),
    /*udb 38*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[2],      376,   8),
    /*udb 39*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[3],      384,   8),
    /*udb 40*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[4],      392,   8),
    /*udb 41*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[5],      400,   8),
    /*udb 42*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[6],      408,   8),
    /*udb 43*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[7],      416,   8),
    /*udb 44*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[8],      424,   8),
    /*udb 45*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[9],      432,   8),
    /*udb 46*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[10],     440,   8),
    /*udb 47*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[11],     448,   8),
    /*udb 48*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[12],     456,   8),
    /*udb 49*/
    I_F_MAC(F_EGR_KEY6,   udb36_49[13],     464,   8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress key7 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_ULTRA_IPV6_RACL_VACL_sip5[] =
{
    I_T_MAC(F_EGR_KEY7,   egressIpCommon, fmtFld_EGRESS_COMMON_sip5_keys_67),

    I_F_MAC(F_EGR_KEY7,    sip.arIP[15],      184,    8),
    I_FA_MAC(F_EGR_KEY7,   sip.arIP[12],      208,   24),
    I_FA_MAC(F_EGR_KEY7,   sip.arIP[4],       336,   64),
    I_FA_MAC(F_EGR_KEY7,   sip.arIP[0],       416,   32),
    I_FA_MAC(F_EGR_KEY7,   dip.arIP[12],      232,   32),
    I_FA_MAC(F_EGR_KEY7,   dip.arIP[0],       448,   96),
    I_F_MAC(F_EGR_KEY7,    isIpv6ExtHdrExist, 333,    1),
    I_F_MAC(F_EGR_KEY7,    isNd,              334,    1),
    I_F_MAC(F_EGR_KEY7,    isIpv6HopByHop,    335,    1),
    I_F_MAC(F_EGR_KEY7,    isVidx,            544,    1),
    I_F_MAC(F_EGR_KEY7,    vid1,              545,   12),
    I_F_MAC(F_EGR_KEY7,    cfi1,              557,    1),
    I_F_MAC(F_EGR_KEY7,    up1,               558,    3),

    I_F_MAC(F_EGR_KEY7,    srcPort,           566,    13),
    I_F_MAC(F_EGR_KEY7,    trgPort,           583,    13),

    /* extra bits from fields of egressIpCommon */
    I_FO_MAC(F_EGR_KEY7,   egressIpCommon.srcDevOrTrunkId,   561,   5, 7),
    I_FO_MAC(F_EGR_KEY7,   egressIpCommon.srcHwDev,            561,   5, 7),
    I_FO_MAC(F_EGR_KEY7,   egressIpCommon.originalVid,       600,   4, 12),
    I_FO_MAC(F_EGR_KEY7,   egressIpCommon.trgPhysicalPort,   604,   2, 6),

    I_F_MAC(F_EGR_KEY7,   isUdbValid,     606,   1),

    /*udb 1*/
    I_F_MAC(F_EGR_KEY7,   udb1_4[0],      608,   8),
    /*udb 2*/
    I_F_MAC(F_EGR_KEY7,   udb1_4[1],      616,   8),
    /*udb 3*/
    I_F_MAC(F_EGR_KEY7,   udb1_4[2],      624,   8),
    /*udb 4*/
    I_F_MAC(F_EGR_KEY7,   udb1_4[3],      632,   8),

    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key replace fields */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld[] =
{
    I_F_MAC(F_INGR_UDB_REP,    pclId,        0,    10),
    I_F_MAC(F_INGR_UDB_REP,    isUdbValid,  15,     1),
    I_F_MAC(F_INGR_UDB_REP,    vid,         16,    13),
    I_F_MAC(F_INGR_UDB_REP,    srcPort,     32,    13),
    I_F_MAC(F_INGR_UDB_REP,    srcDevIsOwn,  1,     1),
    I_F_MAC(F_INGR_UDB_REP,    vid1,        48,    12),
    I_F_MAC(F_INGR_UDB_REP,    up1,         60,     3),
    I_F_MAC(F_INGR_UDB_REP,    macToMe,     63,     1),
    I_F_MAC(F_INGR_UDB_REP,    qosProfile,  64,    10),
    I_F_MAC(F_INGR_UDB_REP,    flowId,      80,    12),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only 60 bytes key fixed fields */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_60_sip5_fixed_fld[] =
{
    I_F_MAC(F_INGR_UDB_60FIX,   isUdbValid,      400,    1),
    I_F_MAC(F_INGR_UDB_60FIX,   pclId,           401,    10),
    I_F_MAC(F_INGR_UDB_60FIX,   vid,             411,    13),
    I_F_MAC(F_INGR_UDB_60FIX,   srcPort,         424,    13),
    I_F_MAC(F_INGR_UDB_60FIX,   srcDevIsOwn,     437,     1),
    I_F_MAC(F_INGR_UDB_60FIX,   vid1,            438,    12),
    I_F_MAC(F_INGR_UDB_60FIX,   up1,             450,     3),
    I_F_MAC(F_INGR_UDB_60FIX,   macToMe,         453,     1),
    I_F_MAC(F_INGR_UDB_60FIX,   qosProfile,      454,     10),
    I_F_MAC(F_INGR_UDB_60FIX,   flowId,          464,     12),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 10 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_10_sip5[] =
{
    /*udb 0-9*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],      0,   80),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 20 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_20_sip5[] =
{
    /*udb 0-19*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],      0,   160),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 30 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_30_sip5[] =
{
    /*udb 0-29*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],      0,   240),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 40 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_40_sip5[] =
{
    /*udb 0-39*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[30],     240,    80),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 50 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_50_sip5[] =
{
    /*udb 0-49*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[30],     240,   160),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* ingress UDB only key 60 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_60_sip5[] =
{
    /*udb 0-49*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[30],     240,   160),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    I_T_MAC(F_INGR_UDB_ONLY,     udb60FixedFld, fmtFld_INGRESS_UDB_ONLY_60_sip5_fixed_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_UDB_ONLY_60_NO_FIXED_FIELDS_sip5[] =
{
    /*udb 0-59*/
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_INGR_UDB_ONLY,   udb[30],     240,   240),
    I_T_MAC(F_INGR_UDB_ONLY,     replacedFld, fmtFld_INGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key replace fields */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld[] =
{
    I_F_MAC(F_EGR_UDB_REP,    pclId,        0,    10),
    I_F_MAC(F_EGR_UDB_REP,    isUdbValid,  15,     1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only 60 bytes key fixed fields */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_60_sip5_fixed_fld[] =
{
    I_F_MAC(F_EGR_UDB_60FIX,   pclId,                400,    10),
    I_F_MAC(F_EGR_UDB_60FIX,   isUdbValid,           410,     1),
    I_F_MAC(F_EGR_UDB_60FIX,   vid,                  411,    13),
    I_F_MAC(F_EGR_UDB_60FIX,   srcPort,              424,    13),
    I_F_MAC(F_EGR_UDB_60FIX,   trgPort,              437,    13),
    I_F_MAC(F_EGR_UDB_60FIX,   srcDev,               450,    10),
    I_F_MAC(F_EGR_UDB_60FIX,   trgDev,               460,    10),
    I_F_MAC(F_EGR_UDB_60FIX,   localDevTrgPhyPort,   470,     8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 10 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_10_sip5[] =
{
    /*udb 0-9*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],      0,   80),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 20 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_20_sip5[] =
{
    /*udb 0-19*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],      0,   160),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 30 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_30_sip5[] =
{
    /*udb 0-29*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],      0,   240),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 40 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_40_sip5[] =
{
    /*udb 0-39*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[30],     240,    80),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 50 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_50_sip5[] =
{
    /*udb 0-49*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[30],     240,   160),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* egress UDB only key 60 bytes */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_UDB_ONLY_60_sip5[] =
{
    /*udb 0-49*/
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[0],        0,   240),
    I_FAS_MAC(F_EGR_UDB_ONLY,   udb[30],     240,   160),
    I_T_MAC(F_EGR_UDB_ONLY,     replacedFld, fmtFld_EGRESS_UDB_ONLY_sip5_replace_fld),
    I_T_MAC(F_EGR_UDB_ONLY,     udb60FixedFld, fmtFld_EGRESS_UDB_ONLY_60_sip5_fixed_fld),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IP_L2_QOS_xCat_fields[] =
{
    I_F_MAC(F_RESIL2,  isArp,          72,     1),
    I_FA_MAC(F_RESIL2, dipBits0to31,   98,    32),
    I_F_MAC(F_RESIL2,  l4Byte13,      130,     8),
    I_FA_MAC(F_RESIL2, macDa,         139,    48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IP_L2_QOS_xCat[] =
{
    I_T_MAC(F_RESIL2,  common,      fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_RESIL2,  common,      fmtFld_EGRESS_COMMON_xCat_base),
    I_T_MAC(F_RESIL2,  common,      fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_T_MAC(F_RESIL2,  commonStdIp, fmtFld_EGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_EGRESS_STD_IP_L2_QOS_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IP_L2_QOS_xCat2[] =
{
    I_T_MAC(F_RESIL2,  common,      fmtFld_EGRESS_COMMON_xCat2_base),
    I_T_MAC(F_RESIL2,  common,      fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_T_MAC(F_RESIL2,  commonStdIp, fmtFld_EGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_EGRESS_STD_IP_L2_QOS_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IPV4_L4_xCat_fields[] =
{
    I_F_MAC(F_RES4L4,  isArp,          72,      1),
    I_F_MAC(F_RES4L4,  isBc,           73,      1),
    I_FA_MAC(F_RES4L4, sip,            98,     32),
    I_FA_MAC(F_RES4L4, dip,           130,     32),
    I_F_MAC(F_RES4L4,  l4Byte13,      162,      8),
    I_F_MAC(F_RES4L4,  l4Byte1,       170,      8),
    I_F_MAC(F_RES4L4,  l4Byte0,       178,      8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IPV4_L4_xCat[] =
{
    I_T_MAC(F_RES4L4,  common,       fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_RES4L4,  common,       fmtFld_EGRESS_COMMON_xCat_base),
    I_T_MAC(F_RES4L4,  common,       fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_T_MAC(F_RES4L4,  commonStdIp,  fmtFld_EGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_EGRESS_STD_IPV4_L4_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_STD_IPV4_L4_xCat2[] =
{
    I_T_MAC(F_RES4L4,  common,       fmtFld_EGRESS_COMMON_xCat2_base),
    I_T_MAC(F_RES4L4,  common,       fmtFld_EGRESS_COMMON_xCat_std_seg0),
    I_T_MAC(F_RES4L4,  commonStdIp,  fmtFld_EGRESS_COMMON_STD_IP_xCat),
    I_ST_MAC(fmtFld_EGRESS_STD_IPV4_L4_xCat_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg0_fields[] =
{
    I_FA_MAC(F_REEN6, sip,                         96,   32),
    I_FA_MAC(F_REEN6, dip,                        128,   32),
    I_F_MAC(F_REEN6,  l2Encap,                    160,    1),
    I_F_MAC(F_REEN6,  etherType,                  161,   16),
    I_F_MAC(F_REEN6,  ipv4Fragmented,             177,    1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields in the second segment of extended rule              */
/* (extended rule consists of to standard-rule-sized segments */
/* offsets from the second segment origin                     */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg1_fields[] =
{
    I_FA_MAC(F_REEN6,  macSa,                        43,   48),
    I_FA_MAC(F_REEN6,  macDa,                        91,   48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_NOT_IPV6_xCat[] =
{
    I_T_MAC(F_REEN6,   common,       fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_REEN6,   common,       fmtFld_EGRESS_COMMON_xCat_base),
    I_THO_MAC(F_REEN6, common,    6, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REEN6,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REEN6, commonExt, 6, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg0_fields),
    I_STHO_MAC(6, fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_NOT_IPV6_xCat2[] =
{
    I_T_MAC(F_REEN6,   common,       fmtFld_EGRESS_COMMON_xCat2_base),
    I_THO_MAC(F_REEN6, common,    LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REEN6,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REEN6, commonExt, LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg0_fields),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_EXT_NOT_IPV6_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg0_fields[] =
{
    I_FA_MAC(F_REE6L2,  sip.arIP[5],       96,  88),
    I_FO_MAC(F_REE6L2,  sip.arIP[4],      184,   7,   0),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields in the second segment of extended rule              */
/* (extended rule consists of to standard-rule-sized segments */
/* offsets from the second segment origin                     */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg1_fields[] =
{
    I_FO_MAC(F_REE6L2,  sip.arIP[4],        0,    1,   7),
    I_FA_MAC(F_REE6L2,  sip.arIP[0],        1,   32),
    I_F_MAC(F_REE6L2,   dipBits127to120,   35,    8),
    I_FA_MAC(F_REE6L2,  macSa,             43,   48),
    I_FA_MAC(F_REE6L2,  macDa,             91,   48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L2_xCat[] =
{
    I_T_MAC(F_REE6L2,   common,       fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_REE6L2,   common,       fmtFld_EGRESS_COMMON_xCat_base),
    I_THO_MAC(F_REE6L2, common,    6, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REE6L2,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REE6L2, commonExt, 6, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg0_fields),
    I_STHO_MAC(6, fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L2_xCat2[] =
{
    I_T_MAC(F_REE6L2,   common,       fmtFld_EGRESS_COMMON_xCat2_base),
    I_THO_MAC(F_REE6L2, common,    LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REE6L2,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REE6L2, commonExt, LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg0_fields),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_EXT_IPV6_L2_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg0_fields[] =
{
    I_FA_MAC(F_REE6L4,  sip.arIP[5],       96,  88),
    I_FO_MAC(F_REE6L4,  sip.arIP[4],      184,   7,   0),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* fields in the second segment of extended rule              */
/* (extended rule consists of to standard-rule-sized segments */
/* offsets from the second segment origin                     */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg1_fields[] =
{
    I_FO_MAC(F_REE6L4,  sip.arIP[4],        0,    1,   7),
    I_FA_MAC(F_REE6L4,  sip.arIP[0],        1,   32),
    I_F_MAC(F_REE6L4,   dip.arIP[0],       35,    8),
    I_FA_MAC(F_REE6L4,  dip.arIP[1],       43,   88),
    I_FA_MAC(F_REE6L4,  dip.arIP[12],     131,   32),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L4_xCat[] =
{
    I_T_MAC(F_REE6L4,   common,       fmtFld_EGRESS_COMMON_xCat_valid),
    I_T_MAC(F_REE6L4,   common,       fmtFld_EGRESS_COMMON_xCat_base),
    I_THO_MAC(F_REE6L4, common,    6, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REE6L4,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REE6L4, commonExt, 6, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg0_fields),
    I_STHO_MAC(6, fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC
    fmtFld_EGRESS_EXT_IPV6_L4_xCat2[] =
{
    I_T_MAC(F_REE6L4,   common,       fmtFld_EGRESS_COMMON_xCat2_base),
    I_THO_MAC(F_REE6L4, common,    LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_xCat_ext_seg1),
    I_T_MAC(F_REE6L4,   commonExt,    fmtFld_EGRESS_COMMON_EXT_xCat_seg0),
    I_THO_MAC(F_REE6L4, commonExt, LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_COMMON_EXT_xCat_seg1),
    I_ST_MAC(fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg0_fields),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFld_EGRESS_EXT_IPV6_L4_xCat_seg1_fields),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* The CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_COMMON_STC fiels in all keys */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldCh3_INGRESS_IP_COMMON[] =
{
    I_F_MAC(F_CI_IP,   pclId,       0,  10),
    I_F_MAC(F_CI_IP,   macToMe,     10,  1),
    I_F_MAC(F_CI_IP,   sourcePort,  11,  6),
    I_F_MAC(F_CI_IP,   isTagged,    17,  1),
    I_F_MAC(F_CI_IP,   vid,         18, 12),
    I_F_MAC(F_CI_IP,   up,          30,  3),
    I_FO_MAC(F_CI_IP,  tos,         33,  6, 0),
    I_FO_MAC(F_CI_IP,  tos,         50,  2, 6),
    I_F_MAC(F_CI_IP,   ipProtocol,   42,  8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* conversion table for XCAT Common.                                         */
/* The tos[1] field different for all keys therefore it converted separately */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_IP_COMMON[] =
{
    I_F_MAC(F_CI_IP,   pclId,       0,  10),
    I_F_MAC(F_CI_IP,   macToMe,     10,  1),
    I_F_MAC(F_CI_IP,   sourcePort,  11,  6),
    I_F_MAC(F_CI_IP,   isTagged,    17,  1),
    I_F_MAC(F_CI_IP,   vid,         18, 12),
    I_F_MAC(F_CI_IP,   up,          30,  3),
    I_FO_MAC(F_CI_IP,  tos,         33,  6, 2),
    I_FO_MAC(F_CI_IP,  tos,         50,  1, 0),
    I_F_MAC(F_CI_IP,   ipProtocol,  42,  8),
    I_F_MAC(F_CI_IP,   isL4Valid,   56,   1),
    I_F_MAC(F_CI_IP,   l4Byte0,     80,   8),
    I_F_MAC(F_CI_IP,   l4Byte1,     88,   8),
    I_F_MAC(F_CI_IP,   l4Byte2,     72,   8),
    I_F_MAC(F_CI_IP,   l4Byte3,     64,   8),
    I_F_MAC(F_CI_IP,   l4Byte13,    96,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* conversion table for Lion2(and xcat2) Common.                                         */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_IP_COMMON[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_IP_COMMON),
    I_FO_MAC(F_CI_IP,   portListBmp.ports[0],           0,   8,   0),
    I_FO_MAC(F_CI_IP,   portListBmp.ports[0],          11,   6,   8),
    I_FO_MAC(F_CI_IP,   portListBmp.ports[0],         192,  14,  14),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* DxCh3 CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_STD_IPV4_ROUTED_ACL_QOS[] =
{
    I_T_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldCh3_INGRESS_IP_COMMON),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   pktType,               39,   3),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ipFragmented,               52,   2),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ipHeaderInfo,               54,   2),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ipPacketLength,             83,  13,   0),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ipPacketLength,             73,   1,  13),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ttl,                        96,   3,   0),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ttl,                       187,   5,   3),
    I_FA_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  sip,                        99,   32),
    I_FA_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  dip,                       131,   32),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.isL4Valid,         56,   1),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte0,          179,   8),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte1,          171,   8),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte2,           65,   8),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte3,           57,   8),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte13,         163,   8),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   tcpUdpPortComparators,      75,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxChXCat and above INGRESS_STD_IPV4_ROUTED_ACL_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_STD_IPV4_ROUTED_ACL_QOS[] =
{
    I_T_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldXCat_INGRESS_IP_COMMON),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ingressIpCommon.tos,        63,   1, 1),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   pktType,                    39,   3),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ipFragmented,               51,   2),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ipHeaderInfo,               53,   2),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ipPacketLength,             57,   6,   0),
    I_FO_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  ipPacketLength,            176,   8,  6),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ttl,                       184,   8),
    I_FA_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  sip,                       112,   32),
    I_FA_MAC(F_IS_IPV4_ROUTED_ACL_QOS,  dip,                       144,   32),
    I_F_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   udb5,                      104,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 and above INGRESS_STD_IPV4_ROUTED_ACL_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_STD_IPV4_ROUTED_ACL_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_STD_IPV4_ROUTED_ACL_QOS),
    I_T_MAC(F_IS_IPV4_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldLion2_INGRESS_IP_COMMON),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* DxCh3 INGRESS_EXT_IPV4_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_EXT_IPV4_PORT_VLAN_QOS[] =
{
    I_T_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon, fmtFldCh3_INGRESS_IP_COMMON),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isL2Valid,                 278,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isBc,                       40,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isIp,                       41,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isArp,                     279,   1),
    I_FO_MAC(F_IE_IPV4_PORT_VLAN_QOS,  l2Encap,                    39,   1,     0),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   etherType,                  58,   16),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  macDa,                     140,   48),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  macSa,                      92,   48),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ipFragmented,              276,   2),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ipHeaderInfo,               90,   2),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  sip,                       212,   32),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  dip,                       244,   32),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.l4Byte13,         188,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.l4Byte1,          196,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.l4Byte2,           82,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.l4Byte3,           74,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.l4Byte0,          204,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon.isL4Valid,        313,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isUdbValid,                280,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbStdIpL2Qos[0],          320,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbStdIpL2Qos[1],          328,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbStdIpV4L4[0],           336,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbStdIpV4L4[1],           344,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbStdIpV4L4[2],           352,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L2[0],           360,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L2[1],           368,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L2[2],           376,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L2[3],           281,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L4[0],           289,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L4[1],           297,   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   UdbExtIpv6L4[2],           305,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxChXCat INGRESS_EXT_IPV4_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg0[] =
{
    I_T_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ingressIpCommon, fmtFldXCat_INGRESS_IP_COMMON),
    I_FO_MAC(F_IE_IPV4_PORT_VLAN_QOS,  ingressIpCommon.tos,        58,   1,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isL2Valid,                  55,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isBc,                       40,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isIp,                       41,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isArp,                      39,   1),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  macSa.arEther[4],           176,   16),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ipFragmented,               51,   2),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   ipHeaderInfo,               53,   2),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  sip,                       112,   32),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  dip,                       144,   32),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   isUdbValid,                 57,   1),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[8],                104,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg1[] =
{
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  macSa.arEther[0],         0,             32),
    I_FA_MAC(F_IE_IPV4_PORT_VLAN_QOS,  macDa,                    (224 - 192),   48),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   etherType,                (272 - 192),   16),
    I_FO_MAC(F_IE_IPV4_PORT_VLAN_QOS,  l2Encap,                  (288 - 192),   1,   0),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[9],               (289 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[10],              (297 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[11],              (305 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[1],               (320 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[2],               (328 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[3],               (336 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[4],               (344 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[5],               (352 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[6],               (360 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[7],               (368 - 192),   8),
    I_F_MAC(F_IE_IPV4_PORT_VLAN_QOS,   udb5_16[0],               (376 - 192),   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg1[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg1),
    I_FO_MAC(F_IE_IPV4_PORT_VLAN_QOS,   l2Encap,                  (327 - LION2_SEG1_BITS_OFFSET_CNS),   1,   1),/*l2Encap[1]*/
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg0),
    I_STHO_MAC(6, fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 (and xCat2) and above INGRESS_EXT_IPV4_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_EXT_IPV4_PORT_VLAN_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFldLion2_INGRESS_EXT_IPV4_PORT_VLAN_QOS_seg1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3 INGRESS_ULTRA_IPV6_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS[] =
{
    I_T_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon, fmtFldCh3_INGRESS_IP_COMMON),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isL2Valid,                 477,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isNd,                       90,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isBc,                       91,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIp,                       41,   1),
    I_FO_MAC(F_IU_IPV6_PORT_VLAN_QOS,  l2Encap,                   476,   1,     0),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   etherType,                  58,   16),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  macDa,                     140,   48),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  macSa,                      92,   48),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  sip.arIP[12],              212,   32),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  sip.arIP[0],               276,   96),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  dip.arIP[12],              244,   32),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  dip.arIP[0],               372,   96),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIpv6ExtHdrExist,          40,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIpv6HopByHop,             39,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ipHeaderOk,                479,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.isL4Valid,        496,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.l4Byte13,         188,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.l4Byte1,          196,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.l4Byte2,           82,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.l4Byte3,           74,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon.l4Byte0,          204,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isUdbValid,                478,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtNotIpv6[0],          468,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtNotIpv6[1],          480,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtNotIpv6[2],          488,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtNotIpv6[3],          568,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L2[0],           504,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L2[1],           512,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L2[2],           520,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L2[3],           528,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L2[4],           536,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L4[0],           544,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L4[1],           552,   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   UdbExtIpv6L4[2],           560,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxChXCat INGRESS_ULTRA_IPV6_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg0[] =
{
    I_T_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon, fmtFldXCat_INGRESS_IP_COMMON),
    I_FO_MAC(F_IU_IPV6_PORT_VLAN_QOS,  ingressIpCommon.tos,        58,   1,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isL2Valid,                  55,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isNd,                       53,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isBc,                       40,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIp,                       41,   1),
    I_FO_MAC(F_IU_IPV6_PORT_VLAN_QOS,  l2Encap,                    39,   1,   0),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  macSa.arEther[4],          176,  16),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  sip.arIP[12],              112,  32),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  dip.arIP[12],              144,  32),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIpv6HopByHop,             51,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isIpv6ExtHdrExist,          52,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ipHeaderOk,                 54,   1),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   isUdbValid,                 57,   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg1[] =
{
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  macSa.arEther[0],                    0,   32),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  sip.arIP[0],               (288 - 192),   96),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   etherType,                 (272 - 192),   16),
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  macDa,                     (224 - 192),   48),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg2[] =
{
    I_FA_MAC(F_IU_IPV6_PORT_VLAN_QOS,  dip.arIP[0],               (384 - 384),   96),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[0],                (480 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[1],                (488 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[2],                (496 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[3],                (504 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[4],                (512 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[5],                (520 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[6],                (528 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[7],                (536 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[8],                (544 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[9],                (552 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[10],               (560 - 384),   8),
    I_F_MAC(F_IU_IPV6_PORT_VLAN_QOS,   udb0_11[11],               (568 - 384),   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg0),
    I_STHO_MAC(6, fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg1),
    I_STHO_MAC(12, fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg2),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 (and xCat2) and above INGRESS_ULTRA_IPV6_PORT_VLAN_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg1),
    I_STHO_MAC(LION2_SEG2_WORDS_OFFSET_CNS, fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_seg2),
    I_T_MAC(F_IU_IPV6_PORT_VLAN_QOS,   ingressIpCommon, fmtFldLion2_INGRESS_IP_COMMON),
    I_FO_MAC(F_IU_IPV6_PORT_VLAN_QOS,   l2Encap,                   59,   1,   1),/*l2Encap[1]*/
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3 INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS[] =
{
    I_T_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldCh3_INGRESS_IP_COMMON),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   pktType,               39,   3),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isNd,                       90,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabel,            112,   20),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabExp,           173,   3),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabSBit,          176,   1),
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ipPacketLength,             52,   13,  0),
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ipPacketLength,            177,    3,  13),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ipv6HdrFlowLabel,           92,   20),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ttl,                       180,   8),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  sip.arIP[12],              212,   32),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  sip.arIP[0],               276,   96),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[12],              244,   32),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[0],               372,   96),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6ExtHdrExist,         471,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6HopByHop,            470,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6LinkLocal,           172,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6Mld,                 469,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ipHeaderOk,                479,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.isL4Valid,         91,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte13,         188,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte1,          196,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte2,           82,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte3,           74,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon.l4Byte0,          204,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isUdbValid,                478,   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdNotIp[0],            480,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdNotIp[1],            488,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpL2Qos[0],          496,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpL2Qos[1],          504,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpL2Qos[2],          512,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpV4L4[0],           520,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpV4L4[1],           528,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbStdIpV4L4[2],           536,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtNotIpv6[0],          544,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtNotIpv6[1],          552,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtNotIpv6[2],          560,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtNotIpv6[3],          568,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L2[0],           132,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L2[1],           140,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L2[2],            65,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L4[0],           148,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L4[1],           156,   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   UdbExtIpv6L4[2],           164,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* DxChXCat and above INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key segment 1 - bits 0..191 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg0[] =
{
    I_T_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldXCat_INGRESS_IP_COMMON),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   pktType,               39,    3),
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ipPacketLength,        51,    4,  0),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  sip.arIP[12],         112,   32),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[12],         144,   32),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isUdbValid,            57,    1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[1],          104,    8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[2],          176,    8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[3],          184,    8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* DxChXCat and above INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key segment 2 - bits 192..383 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg1[] =
{
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ingressIpCommon.tos, (216 - 192),   1, 1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isNd,                (217 - 192),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabel,      (239 - 192),  20),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabExp,     (259 - 192),   3),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   mplsOuterLabSBit,    (262 - 192),   1),
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ipPacketLength,      (263 - 192),   8,  4),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ipv6HdrFlowLabel,    (218 - 192),  20),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ttl,                 (271 - 192),   8),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  sip.arIP[0],         (279 - 192),  96),
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[0],         (375 - 192),   8),    /* 8 bits of DIP  127:32*/
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[1],         (383 - 192),   1, 0), /* 9th bit of DIP 127:32 */
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6LinkLocal,     (238 - 192),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[0],         (208 - 192),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[4],         (192 - 192),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb17_22[5],         (200 - 192),   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxChXCat and above INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key segment 3 - bits 384..575*/
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg2[] =
{
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  ipPacketLength,      (471 - 384),   4, 12),
    I_FO_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[1],         (384 - 384),   7,  1), /* 7 bits of DIP  127:32, starting from bit#10 */
    I_FA_MAC(F_IU_IPV6_ROUTED_ACL_QOS,  dip.arIP[2],         (391 - 384),  80), /* rest of DIP  127:32 */
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6ExtHdrExist,   (478 - 384),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6HopByHop,      (477 - 384),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   isIpv6Mld,           (476 - 384),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ipHeaderOk,          (479 - 384),   1),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[0],          (480 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[1],          (488 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[2],          (496 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[3],          (504 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[4],          (512 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[5],          (520 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[6],          (528 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[7],          (536 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[8],          (544 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[9],          (552 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[10],         (560 - 384),   8),
    I_F_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   udb0_11[11],         (568 - 384),   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxChXCat and above INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS[] =
{
    I_ST_MAC(fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg0),
    I_STHO_MAC(6, fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg1),
    I_STHO_MAC(12, fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg2),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 (and xcat2) and above INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS key */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS[] =
{
    /* the xcat format */
    I_ST_MAC(fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg1),
    I_STHO_MAC(LION2_SEG2_WORDS_OFFSET_CNS, fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_seg2),
    /* lion2 specific fields */
    I_T_MAC(F_IU_IPV6_ROUTED_ACL_QOS,   ingressIpCommon, fmtFldLion2_INGRESS_IP_COMMON),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* DxCh3 and above, Egress IP Common structure segment 0 - bits 0..191 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldCh3_EGRESS_IP_COMMON_seg0[] =
{
    I_F_MAC(F_CE_IP,   pclId,                    0,   10),
    I_F_MAC(F_CE_IP,   sourcePort,              11,   6),
    I_F_MAC(F_CE_IP,   qosProfile,              17,   7),
    I_F_MAC(F_CE_IP,   isL2Valid,              157,   1),
    I_F_MAC(F_CE_IP,   originalVid,             37,   12),
    I_F_MAC(F_CE_IP,   isSrcTrunk,              60,   1),
    I_F_MAC(F_CE_IP,   srcDevOrTrunkId,         51,   7),
    I_F_MAC(F_CE_IP,   isIp,                   158,   1),
    I_F_MAC(F_CE_IP,   isArp,                  183,   1),
    I_FA_MAC(F_CE_IP,  macDa,                  109,   48),
    I_FA_MAC(F_CE_IP,  macSa,                   61,   48),
    I_FO_MAC(F_CE_IP,  tos,                    173,   2,  0),
    I_FO_MAC(F_CE_IP,  tos,                    167,   6,  2),
    I_F_MAC(F_CE_IP,   ipProtocol,             159,   8),
    I_F_MAC(F_CE_IP,   ttl,                    175,   8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3 and above, Egress IP Common structure segment 1 - bits 192..383 */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldCh3_EGRESS_IP_COMMON_seg1[] =
{
    I_F_MAC(F_CE_IP,   isL4Valid,              (296 - 192),   1),
    I_F_MAC(F_CE_IP,   l4Byte0,                (272 - 192),   8),
    I_F_MAC(F_CE_IP,   l4Byte1,                (264 - 192),   8),
    I_F_MAC(F_CE_IP,   l4Byte2,                (288 - 192),   8),
    I_F_MAC(F_CE_IP,   l4Byte3,                (280 - 192),   8),
    I_F_MAC(F_CE_IP,   l4Byte13,               (256 - 192),   8),
    I_F_MAC(F_CE_IP,   tcpUdpPortComparators,  (248 - 192),   8),
    I_F_MAC(F_CE_IP,   tc,                     (297 - 192),   3),
    I_F_MAC(F_CE_IP,   dp,                     (300 - 192),   2),
    I_F_MAC(F_CE_IP,   egrPacketType,          (302 - 192),   2),
    I_F_MAC(F_CE_IP,   srcTrgOrTxMirror,       (304 - 192),   1),
    I_F_MAC(F_CE_IP,   assignedUp,             (305 - 192),   3),
    I_F_MAC(F_CE_IP,   trgPhysicalPort,        (308 - 192),   6),
    I_F_MAC(F_CE_IP,   rxSniff,                (314 - 192),   1),
    I_F_MAC(F_CE_IP,   isRouted,               (315 - 192),   1),
    I_F_MAC(F_CE_IP,   isIpv6,                 (316 - 192),   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3, xCat, xCat3, Lion */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldCh3_EGRESS_IP_COMMON[] =
{
    I_ST_MAC(fmtFldCh3_EGRESS_IP_COMMON_seg0),
    I_STHO_MAC(6, fmtFldCh3_EGRESS_IP_COMMON_seg1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/* the Lion2 FS hold wrong values of the fields starting at bit 192 that should
   be moved by 14 bits to start at bit 206 */
#define LION2_RACL_VACL_KEY_SEGMENT1_OFFSET     14
/* lion2 and above */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_IP_COMMON_RACL_VACL_seg1[] =
{
    I_F_MAC(F_CE_IP,   isL4Valid,              (296 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_CE_IP,   l4Byte0,                (272 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   l4Byte1,                (264 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   l4Byte2,                (288 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   l4Byte3,                (280 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   l4Byte13,               (256 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   tcpUdpPortComparators,  (248 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   8),
    I_F_MAC(F_CE_IP,   tc,                     (297 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   3),
    I_F_MAC(F_CE_IP,   dp,                     (300 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   2),
    I_F_MAC(F_CE_IP,   egrPacketType,          (302 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   2),
    I_F_MAC(F_CE_IP,   srcTrgOrTxMirror,       (304 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_CE_IP,   assignedUp,             (305 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   3),
    I_F_MAC(F_CE_IP,   trgPhysicalPort,        (308 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   6),
    I_F_MAC(F_CE_IP,   rxSniff,                (314 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_CE_IP,   isRouted,               (315 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_CE_IP,   isIpv6,                 (316 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3 and above */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_EXT_IPV4_RACL_VACL[] =
{
    I_T_MAC(F_EE_IPV4_RACL_VACL,   egressIpCommon, fmtFldCh3_EGRESS_IP_COMMON),
    I_FA_MAC(F_EE_IPV4_RACL_VACL,   sip,               184,   32),
    I_FA_MAC(F_EE_IPV4_RACL_VACL,   dip,               216,   32),
    I_F_MAC(F_EE_IPV4_RACL_VACL,    ipv4Options,       317,   1),
    I_F_MAC(F_EE_IPV4_RACL_VACL,    isVidx,            318,   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};


/* lion2 and above */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_IP_COMMON_RACL_VACL[] =
{
    I_ST_MAC(fmtFldCh3_EGRESS_IP_COMMON_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFldLion2_EGRESS_IP_COMMON_RACL_VACL_seg1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 : first segment */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL_seg0[] =
{
    I_F_MAC(F_EE_IPV4_RACL_VACL,   sip.arIP[3],          184,    8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 : second segment */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL_seg1[] =
{
    I_FA_MAC(F_EE_IPV4_RACL_VACL,   sip.arIP[0],  (192 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET  - LION2_SEG1_BITS_OFFSET_CNS),   24),
    I_FA_MAC(F_EE_IPV4_RACL_VACL,   dip,          (216 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET  - LION2_SEG1_BITS_OFFSET_CNS),   32),
    I_F_MAC(F_EE_IPV4_RACL_VACL,    ipv4Options,  (317 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET  - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_EE_IPV4_RACL_VACL,    isVidx,       (318 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET  - LION2_SEG1_BITS_OFFSET_CNS),   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL[] =
{
    I_ST_MAC(fmtFldLion2_EGRESS_IP_COMMON_RACL_VACL),
    I_ST_MAC(fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS, fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL_seg1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* DxCh3, xCat and Lion */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFld_EGRESS_ULTRA_IPV6_RACL_VACL[] =
{
    I_T_MAC(F_EU_IPV6_RACL_VACL,   egressIpCommon, fmtFldCh3_EGRESS_IP_COMMON),
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[12],      184,   32),
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[0],       320,   96),
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   dip.arIP[12],      216,   32),
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   dip.arIP[0],       416,   96),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isNd,              318,   1),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isIpv6ExtHdrExist, 317,   1),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isIpv6HopByHop,    319,   1),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isVidx,            512,   1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 : first segment */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg0[] =
{
    /*sip[0..7]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[15],      184,    8),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Lion2 : second segment */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg1[] =
{
    /*sip[8..31]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[12],      (192 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   24),
    /*dip[0..31]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   dip.arIP[12],      (216 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   32),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isIpv6ExtHdrExist, (317 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isNd,              (318 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isIpv6HopByHop,    (319 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   1),
    /* sip[32..95]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[4],       (320 + LION2_RACL_VACL_KEY_SEGMENT1_OFFSET - LION2_SEG1_BITS_OFFSET_CNS),   64),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* the Lion2 FS hold wrong values of the fields starting at bit 192 that should
   be moved by 14 bits to start at bit 206

   and additional 14 bits stating from bit (192*2)=384
   */
#define LION2_RACL_VACL_KEY_SEGMENT2_OFFSET (2*LION2_RACL_VACL_KEY_SEGMENT1_OFFSET)

/* Lion2 : third segment */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg2[] =
{
    /* sip[96..127]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   sip.arIP[0],       (384 + LION2_RACL_VACL_KEY_SEGMENT2_OFFSET - LION2_SEG2_BITS_OFFSET_CNS ),    32),
    /* dip[32..127]*/
    I_FA_MAC(F_EU_IPV6_RACL_VACL,   dip.arIP[0],       (416 + LION2_RACL_VACL_KEY_SEGMENT2_OFFSET - LION2_SEG2_BITS_OFFSET_CNS),    96),
    I_F_MAC(F_EU_IPV6_RACL_VACL,    isVidx,            (512 + LION2_RACL_VACL_KEY_SEGMENT2_OFFSET - LION2_SEG2_BITS_OFFSET_CNS),    1),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};
/* Lion2 and above */
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL[] =
{
    I_T_MAC(F_EU_IPV6_RACL_VACL,    egressIpCommon, fmtFldLion2_EGRESS_IP_COMMON_RACL_VACL),
    I_ST_MAC(                                fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg0),
    I_STHO_MAC(LION2_SEG1_WORDS_OFFSET_CNS , fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg1),
    I_STHO_MAC(LION2_SEG2_WORDS_OFFSET_CNS , fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL_seg2),
    CPSS_FORMAT_CONVERT_FIELD_INFO_END_MAC
};

/* Bobcat2, Caelum, Bobcat3 : ipcl : start bits for the ports bmp parts */
static GT_U32  ingressPortsBmpRuleStartBitsArr[3] = {0,11,192};
/* Bobcat2, Caelum, Bobcat3 : epcl : start bits for the ports bmp parts */
static GT_U32  egressPortsBmpRuleStartBitsArr[3] = {0,32,192};
/* length for the ports bmp parts */
static GT_U32  portsBmpSubFiledLengthArr[3]       = {8, 6, 14};


/**************************************************************************************/

/*
 * typedef enum PRV_CPSS_DXCH_PCL_TCAM_OPERATION
 *
 * Description: commands of copying words and half-wors
 *              from TCAM address space to application memory
 *
 * Enumerations:
 *
 *    PRV_TCAM_REG_COPY_NONE_E        - "do nothing" command
 *    PRV_TCAM_REG_COPY_32_E          - copy all 32 bits as is
 *    PRV_TCAM_REG_COPY_16LOW2LOW_E   - copy low 16 bits to low 16 bits
 *    PRV_TCAM_REG_COPY_16LOW2HIGH_E  - copy low 16 bits to high 16 bits
 *    PRV_TCAM_REG_COPY_16HIGH2LOW_E  - copy high 16 bits to low 16 bits
 *
 */
typedef enum
{
    PRV_TCAM_REG_COPY_NONE_E,
    PRV_TCAM_REG_COPY_32_E,
    PRV_TCAM_REG_COPY_16LOW2LOW_E,
    PRV_TCAM_REG_COPY_16LOW2HIGH_E,
    PRV_TCAM_REG_COPY_16HIGH2LOW_E
} PRV_TCAM_REG_COPY_ENT;

/**
* @struct PRV_TCAM_REG_COPY_CMD_STC
 *
 * @brief Rule of word or half-word copy
 * from TCAM address space to application memory
*/
typedef struct{

    /** copy command */
    PRV_TCAM_REG_COPY_ENT copyRule;

    /** @brief index of word in application memory
     *  Comments:
     */
    GT_U8 dataWordIndex;

} PRV_TCAM_REG_COPY_CMD_STC;

/**
* @struct PRV_TCAM_REG_TO_DATA_STC
 *
 * @brief Rule word copy
 * from TCAM address space to application memory
*/
typedef struct{

    /** index of word in TCAM address space */
    GT_U8 tcamWordIndex;

    PRV_TCAM_REG_COPY_CMD_STC copyTo[2];

} PRV_TCAM_REG_TO_DATA_STC;

/* Cheetah1 */
static PRV_TCAM_REG_TO_DATA_STC prvCpssDxChPclCh1Reg2Data[] =
{
    {0, {{PRV_TCAM_REG_COPY_32_E, 0}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {1, {{PRV_TCAM_REG_COPY_32_E, 1}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {2, {{PRV_TCAM_REG_COPY_32_E, 2}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {4, {{PRV_TCAM_REG_COPY_32_E, 3}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {5, {{PRV_TCAM_REG_COPY_32_E, 4}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {6, {{PRV_TCAM_REG_COPY_32_E, 5}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
};

static GT_U8 prvCpssDxChPclCh1CopyNum =
    sizeof(prvCpssDxChPclCh1Reg2Data) / sizeof(prvCpssDxChPclCh1Reg2Data[0]) ;

/* Cheetah2 */

/* any 24 byte data and 24 byte mask of PCE                 */
/* may be direct read (and written) via                     */
/* 4 register sets: data and data-mask registers            */
/* and control and control-mask registers                   */
/* the 24 bytes represented inside HW as 5 "words"          */
/* that contain 32,48,48,32 and 32 bits                     */
/* 32 bit words reside in data and data-mask registers      */
/* the 32 low bits 48 bit words also reside in              */
/* data and data-mask registers and the high 16 bit of them */
/* reside in control and control-mask registers             */
/* the table below shows how to retrieve the 24 sequence    */
/* from these registers                                     */
/* the data registers - even, the contol - odd              */

/* Cheetah2 */
static PRV_TCAM_REG_TO_DATA_STC prvCpssDxChPclCh2Reg2Data[] =
{
    {0, {{PRV_TCAM_REG_COPY_32_E, 0}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {2, {{PRV_TCAM_REG_COPY_32_E, 1}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {3, {{PRV_TCAM_REG_COPY_16LOW2LOW_E, 2},
        {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {4, {{PRV_TCAM_REG_COPY_16LOW2HIGH_E, 2},
        {PRV_TCAM_REG_COPY_16HIGH2LOW_E, 3}}},
    {5, {{PRV_TCAM_REG_COPY_16LOW2HIGH_E, 3},
        {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {6, {{PRV_TCAM_REG_COPY_32_E, 4}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
    {8, {{PRV_TCAM_REG_COPY_32_E, 5}, {PRV_TCAM_REG_COPY_NONE_E, 0}}},
};

static GT_U8 prvCpssDxChPclCh2CopyNum =
    sizeof(prvCpssDxChPclCh2Reg2Data) / sizeof(prvCpssDxChPclCh2Reg2Data[0]) ;


/* DxCh only IPCL LOOKUP 0 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwChPlusIngrL0Pos =
{
    0,     1,     /*enableLookupOff/len   */
    2,     10,    /*pclIdOff/Len          */
    12,    1,     /*nonIpKeyTypeOff/Len   */
    1,     1,     /*ipv4KeyTypeOff/Len    */    /* L2/L4       */
    0xFF,  0,     /*ipv6KeyTypeOff/Len    */    /* not-relevant*/
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh only IPCL LOOKUP 1 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwChPlusIngrL1Pos =
{
    14,    1,     /*enableLookupOff/len   */
    16,   10,     /*pclIdOff/Len          */
    26,    1,     /*nonIpKeyTypeOff/Len   */
    15,    1,     /*ipv4KeyTypeOff/Len    */ /* L2/L4 */
    27,    1,     /*ipv6KeyTypeOff/Len    */ /* std-DIP */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};


/* DxCh2 only IPCL LOOKUP 0 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh2IngrL0Pos =
{
    0,     1,     /*enableLookupOff/len   */
    2,    10,     /*pclIdOff/Len          */
    1,     1,     /*nonIpKeyTypeOff/Len   */
    12,    2,     /*ipv4KeyTypeOff/Len    */
    14,    2,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh2 only IPCL LOOKUP 1 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh2IngrL1Pos =
{
    16,    1,     /*enableLookupOff/len   */
    18,   10,     /*pclIdOff/Len          */
    17,    1,     /*nonIpKeyTypeOff/Len   */
    28,    2,     /*ipv4KeyTypeOff/Len    */
    30,    2,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh2 only EPCL */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh2EgrPos =
{
    0,     1,     /*enableLookupOff/len   */
    2,     10,    /*pclIdOff/Len          */
    1,     1,     /*nonIpKeyTypeOff/Len   */
    12,    2,     /*ipv4KeyTypeOff/Len    */
    14,    2,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh3 only IPCL LOOKUP 0 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh3IngrL0Pos =
{
    0,     1,     /*enableLookupOff/len   */
    11,   10,     /*pclIdOff/Len          */
    1,     3,     /*nonIpKeyTypeOff/Len   */
    4,     3,     /*ipv4KeyTypeOff/Len    */
    7,     3,     /*ipv6KeyTypeOff/Len    */
    10,    1,     /*dualLookupOff/Len     */
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh3 only IPCL LOOKUP 1 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh3IngrL1Pos =
{
    21,    1,     /*enableLookupOff/len   */
    32,   10,     /*pclIdOff/Len          */
    22,    3,     /*nonIpKeyTypeOff/Len   */
    25,    3,     /*ipv4KeyTypeOff/Len    */
    28,    3,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* xCat A1 and above IPCL LOOKUP 0 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwXCatIngrL0Pos =
{
    0,     1,     /*enableLookupOff/len   */
    13,   10,     /*pclIdOff/Len          */
    1,     3,     /*nonIpKeyTypeOff/Len   */
    4,     3,     /*ipv4KeyTypeOff/Len    */
    7,     3,     /*ipv6KeyTypeOff/Len    */
    10,    1,     /*dualLookupOff/Len     */
    23,   10,     /*pclIdL01Off/Len       */
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* xCat A1 and above IPCL LOOKUP 0 */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwXCatIngrL1Pos =
{
    0,     1,     /*enableLookupOff/len   */
    13,   10,     /*pclIdOff/Len          */
    1,     3,     /*nonIpKeyTypeOff/Len   */
    4,     3,     /*ipv4KeyTypeOff/Len    */
    7,     3,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* DxCh3 and above EPCL */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwCh3EgrPos =
{
    0,     1,     /*enableLookupOff/len   */
    3,    10,     /*pclIdOff/Len          */
    1,     2,     /*nonIpKeyTypeOff/Len   */
    13,    2,     /*ipv4KeyTypeOff/Len    */
    15,    2,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    0xFF,  0,     /*udbKeyBitmapEnable/Len*/    /* not-relevant*/
    0xFF,  0,     /*tcamSegmentMode/Len   */    /* not-relevant*/
    0xFF,  0      /*exactMatchLookupSubProfileIdOff/Len*/ /* not-relevant*/
};

/* Bobcat2 and above IPCL */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwSip5IngrPos =
{
    0,     1,     /*enableLookupOff/len   */
    13,   10,     /*pclIdOff/Len          */
    1,     4,     /*nonIpKeyTypeOff/Len   */
    5,     4,     /*ipv4KeyTypeOff/Len    */
    9,     4,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    23,    1,     /*udbKeyBitmapEnable/Len*/
    24,    3,     /*tcamSegmentMode/Len   */
    27,    3      /*exactMatchLookupSubProfileIdOff/Len*/
};

/* Bobcat2 and above EPCL */
static PRV_PCL_CFG_LOOKUP_POS_STC prvCpssDxChPclCfgTblHwSip5EgrPos =
{
    0,     1,     /*enableLookupOff/len   */
    1,    10,     /*pclIdOff/Len          */
    11,    4,     /*nonIpKeyTypeOff/Len   */
    15,    4,     /*ipv4KeyTypeOff/Len    */
    19,    4,     /*ipv6KeyTypeOff/Len    */
    0xFF,  0,     /*dualLookupOff/Len     */    /* not-relevant*/
    0xFF,  0,     /*pclIdL01Off/Len       */    /* not-relevant*/
    26,    1,     /*udbKeyBitmapEnable/Len*/
    23,    3,     /*tcamSegmentMode/Len   */
    27,    3      /*exactMatchLookupSubProfileIdOff/Len*/
};

#if PRV_CPSS_DXCH_ERRATA_REFERENCE_CNS  /* macro for references purpose only -- must be 0 --> never 1 */
    /* note: the file uses 'direct read' from the PCL TCAM , so not needed
             the WA to FEr#2004 :
             Policy and Router TCAM Tables indirect read is not functional
       see --> PRV_CPSS_DXCH3_POLICY_AND_ROUTER_TCAM_TABLES_INDIRECT_READ_WA_E */
    PRV_CPSS_DXCH3_POLICY_AND_ROUTER_TCAM_TABLES_INDIRECT_READ_WA_E

    /* note: the file uses 'direct write' to the PCL TCAM , so not needed
             the WA to FEr#2744 :
             Indirect access to TCAM (Router and PCL) is not supported
       see --> PRV_CPSS_DXCH_LION_INDIRECT_ACCESS_TO_TCAM_IS_NOT_SUPPORTED_WA_E */
    PRV_CPSS_DXCH_LION_INDIRECT_ACCESS_TO_TCAM_IS_NOT_SUPPORTED_WA_E

#endif /* PRV_CPSS_DXCH_ERRATA_REFERENCE_CNS */

static CPSS_FORMAT_CONVERT_FIELD_INFO_STC* fmtFldArr_ch3[] = {
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP _E                  */   /*0*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E               */   /*1*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                 */   /*2*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                */   /*3*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                */   /*4*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                 */   /*5*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                 */   /*6*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                   */   /*7*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                */   /*8*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                  */   /*9*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                 */   /*10*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                  */   /*11*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                  */   /*12*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                     */   /*13*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                     */   /*14*/  ,
        fmtFld_INGRESS_STD_IPV4_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E     */   /*15*/  ,
        fmtFld_INGRESS_EXT_IPV4_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E      */   /*16*/  ,
        fmtFld_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E    */   /*17*/  ,
        fmtFld_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E   */   /*18*/  ,
        fmtFld_EGRESS_EXT_IPV4_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E           */   /*19*/  ,
        fmtFld_EGRESS_ULTRA_IPV6_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E         */   /*20*/  ,
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL/*reserved*/
};
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC* fmtFldArr_xcat[] = {
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                  */   /*0*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E               */   /*1*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                 */   /*2*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                */   /*3*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                */   /*4*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                 */   /*5*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                 */   /*6*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                   */   /*7*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                */   /*8*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                  */   /*9*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                 */   /*10*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                  */   /*11*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                  */   /*12*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                     */   /*13*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                     */   /*14*/  ,
        fmtFldXCat_INGRESS_STD_IPV4_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E     */   /*15*/  ,
        fmtFldXCat_INGRESS_EXT_IPV4_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E      */   /*16*/  ,
        fmtFldXCat_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E    */   /*17*/  ,
        fmtFldXCat_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E   */   /*18*/  ,
        fmtFld_EGRESS_EXT_IPV4_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E           */   /*19*/  ,
        fmtFld_EGRESS_ULTRA_IPV6_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E         */   /*20*/  ,
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL/*reserved*/
};
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC* fmtFldArr_Lion2[] = {
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                  */   /*0*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E               */   /*1*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                 */   /*2*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                */   /*3*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                */   /*4*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                 */   /*5*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                 */   /*6*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                   */   /*7*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                */   /*8*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                  */   /*9*/   ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                 */   /*10*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                  */   /*11*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                  */   /*12*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                     */   /*13*/  ,
        NULL/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                     */   /*14*/  ,
        fmtFldLion2_INGRESS_STD_IPV4_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E     */   /*15*/  ,
        fmtFldLion2_INGRESS_EXT_IPV4_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E      */   /*16*/  ,
        fmtFldLion2_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E    */   /*17*/  ,
        fmtFldLion2_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E   */   /*18*/  ,
        fmtFldLion2_EGRESS_EXT_IPV4_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E           */   /*19*/  ,
        fmtFldLion2_EGRESS_ULTRA_IPV6_RACL_VACL/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E         */   /*20*/  ,
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL,/*reserved*/
        NULL/*reserved*/
};
static CPSS_FORMAT_CONVERT_FIELD_INFO_STC* fmtFldArr_sip5[] = {
        fmtFld_INGRESS_sip5_key0_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                  */   /*0*/   ,
        fmtFld_INGRESS_sip5_key1_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E               */   /*1*/   ,
        fmtFld_INGRESS_sip5_key2_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                 */   /*2*/   ,
        fmtFld_INGRESS_sip5_key12_flds/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                */   /*3*/   ,
        fmtFld_INGRESS_sip5_key4_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                */   /*4*/   ,
        fmtFld_INGRESS_sip5_key5_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                 */   /*5*/   ,
        fmtFld_INGRESS_sip5_key6_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                 */   /*6*/   ,

/*EPCL - key0*/fmtFld_EGRESS_STD_NOT_IP_sip5   /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                 */   /*7*/   ,
/*EPCL - key1*/fmtFld_EGRESS_STD_IP_L2_QOS_sip5/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E              */   /*8*/   ,
/*EPCL - key2*/fmtFld_EGRESS_STD_IPV4_L4_sip5  /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                */   /*9*/   ,
/*EPCL - key3*/fmtFld_EGRESS_EXT_NOT_IPV6_sip5 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E               */   /*10*/  ,
/*EPCL - key4*/fmtFld_EGRESS_EXT_IPV6_L2_sip5  /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                */   /*11*/  ,
/*EPCL - key5*/fmtFld_EGRESS_EXT_IPV6_L4_sip5  /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                */   /*12*/  ,

        fmtFld_INGRESS_sip5_key3_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                     */   /*13*/  ,
        fmtFld_INGRESS_sip5_key7_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                     */   /*14*/  ,
        fmtFld_INGRESS_sip5_key10_flds/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E     */   /*15*/  ,
        fmtFld_INGRESS_sip5_key8_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E      */   /*16*/  ,
        fmtFld_INGRESS_sip5_key9_flds /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E    */   /*17*/  ,
        fmtFld_INGRESS_sip5_key11_flds/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E   */   /*18*/  ,
/*EPCL - key6*/fmtFld_EGRESS_EXT_IPV4_RACL_VACL_sip5/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E    */   /*19*/  ,
/*EPCL - key7*/fmtFld_EGRESS_ULTRA_IPV6_RACL_VACL_sip5/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E*/   /*20*/  ,
        fmtFld_INGRESS_UDB_ONLY_10_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E*/   /*21*/
        fmtFld_INGRESS_UDB_ONLY_20_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E*/   /*22*/
        fmtFld_INGRESS_UDB_ONLY_30_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E*/   /*23*/
        fmtFld_INGRESS_UDB_ONLY_40_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E*/   /*24*/
        fmtFld_INGRESS_UDB_ONLY_50_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E*/   /*25*/
        fmtFld_INGRESS_UDB_ONLY_60_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E*/   /*26*/
        fmtFld_EGRESS_UDB_ONLY_10_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */   /*27*/
        fmtFld_EGRESS_UDB_ONLY_20_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E */   /*28*/
        fmtFld_EGRESS_UDB_ONLY_30_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E */   /*29*/
        fmtFld_EGRESS_UDB_ONLY_40_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E */   /*30*/
        fmtFld_EGRESS_UDB_ONLY_50_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E */   /*31*/
        fmtFld_EGRESS_UDB_ONLY_60_sip5,/*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E */   /*32*/
        fmtFld_INGRESS_UDB_ONLY_60_NO_FIXED_FIELDS_sip5/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E*/   /*33*/
};

static GT_STATUS      ipclGetProperFieldValue(
    IN GT_U8                            devNum,
    IN GT_U32                           *memPtr,
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName,
    OUT GT_U32                           *valuePtr
);
static GT_STATUS   ipclSetProperFieldValue(
    IN GT_U8                            devNum,
    IN GT_U32                           *memPtr,
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName,
    IN GT_U32                           value
);

/* macro to set value to field of (IPCL) SIP5_IPCL_ACTION entry format in buffer */
/* _fieldName of type SIP5_IPCL_ACTION_TABLE_FIELDS_ENT */
#define SIP5_IPCL_ACTION_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)   \
    {                                                                        \
        GT_STATUS _rc;                                                       \
        _rc = ipclSetProperFieldValue(_dev,_hwDataArr,_fieldName,_value);    \
        if (_rc != GT_OK)                                                    \
        {                                                                    \
            return _rc;                                                      \
        }                                                                    \
    }


/* macro to set value to field of (IPCL) SIP_5_20_IPCL_ACTION entry format in buffer */
/* _fieldName of type SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT */
#define SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)


/* macro to get value to field of (IPCL) SIP5_IPCL_ACTION entry format from buffer */
/* _fieldName of type SIP5_IPCL_ACTION_TABLE_FIELDS_ENT */
#define SIP5_IPCL_ACTION_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    {                                                                         \
        GT_STATUS _rc;                                                        \
        _rc = ipclGetProperFieldValue(_dev,_hwDataArr,_fieldName,&_value);    \
        if (_rc != GT_OK)                                                     \
        {                                                                     \
            return _rc;                                                       \
        }                                                                     \
    }

/* macro to get value to field of (IPCL) SIP_5_20_IPCL_ACTION entry format from buffer */
/* _fieldName of type SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT */
#define SIP_5_20_IPCL_ACTION_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

/* macro to set value to field of (IPCL) SIP6_IPCL_ACTION entry format in buffer */
/* _fieldName of type SIP6_IPCL_ACTION_TABLE_FIELDS_ENT */
#define SIP6_IPCL_ACTION_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)       \
        U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                     \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr, \
        _fieldName,                                                             \
        _value)


/* macro to set value to field of (EPCL) SIP5_EPCL_ACTION entry format in buffer */
#define SIP5_EPCL_ACTION_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EPCL_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

/* macro to get value to field of (EPCL) SIP5_EPCL_ACTION entry format from buffer */
#define SIP5_EPCL_ACTION_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EPCL_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

static struct {
    SIP5_IPCL_ACTION_TABLE_FIELDS_ENT      sip5Name;   /*IN*/
    SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT    sip5_20Name;/*OUT*/
}
ipcl_action_convert_sip5_to_sip_5_20 [] =
{
     {SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E                      , SIP5_20_IPCL_ACTION_TABLE_FIELDS_COMMAND_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E                     , SIP5_20_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E             , SIP5_20_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E           , SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E          }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E                         , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VIDX_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E                     , SIP5_20_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E                     , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E                , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E                     , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E                     , SIP5_20_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E                 , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E               , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E      , SIP5_20_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E                       , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E                      }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E            , SIP5_20_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E               , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E      , SIP5_20_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E         , SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E                  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E                    , SIP5_20_IPCL_ACTION_TABLE_FIELDS_SST_ID_E                      }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E                , SIP5_20_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E              , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E             }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E               , SIP5_20_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E                 , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E                         , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID0_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E   , SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E               , SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E                    , SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E}
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E                , SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E                , SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E , SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E}
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E                    , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E                      , SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E                         , SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID1_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E                          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_E                         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E                      , SIP5_20_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E             , SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E         , SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E                   , SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E                  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E                 , SIP5_20_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E                 , SIP5_20_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E            , SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E           }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E                  , SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E          , SIP5_20_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E                    , SIP5_20_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E               , SIP5_20_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E              }
    ,{0xFFFFFFFF , 0xFFFFFFFF}
};

static struct {
    SIP5_IPCL_ACTION_TABLE_FIELDS_ENT      sip5Name;   /*IN*/
    SIP6_IPCL_ACTION_TABLE_FIELDS_ENT      sip6Name;   /*OUT*/
}
ipcl_action_convert_sip5_to_sip_6 [] =
{
     {SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E                      , SIP6_IPCL_ACTION_TABLE_FIELDS_COMMAND_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E                     , SIP6_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E             , SIP6_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E          , SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E          , SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E           , SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E          }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E                         , SIP6_IPCL_ACTION_TABLE_FIELDS_VIDX_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E                     , SIP6_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E                     , SIP6_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E                , SIP6_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E                     , SIP6_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E                     , SIP6_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E                    }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E                 , SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E               , SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E  , SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E      , SIP6_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E                       , SIP6_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E                      }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E          , SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E            , SIP6_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E               , SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E      , SIP6_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E         , SIP6_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E                  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E                    , SIP6_IPCL_ACTION_TABLE_FIELDS_SST_ID_E                      }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E                , SIP6_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E          , SIP6_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E              , SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E             }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E               , SIP6_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E                 , SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E                         , SIP6_IPCL_ACTION_TABLE_FIELDS_VID0_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E   , SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E               , SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E              }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E                    , SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E , SIP6_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E}
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E                , SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E                , SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E               }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E , SIP6_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E}
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E                    , SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E                      , SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E                         , SIP6_IPCL_ACTION_TABLE_FIELDS_VID1_E                        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E                          , SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_E                         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E                      , SIP6_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E                     }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E             , SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E            }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E         , SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E        }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E                   , SIP6_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E                  }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E                 , SIP6_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E                 , SIP6_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E                }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E            , SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E           }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E                  , SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E                 }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E          , SIP6_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E         }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E                    , SIP6_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E                   }
    ,{SIP5_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E               , SIP6_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E              }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E              , SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E             }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E        , SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E       }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E        , SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E       }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E         , SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E        }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_RESERVED_3_BITS_E              , SIP6_IPCL_ACTION_TABLE_FIELDS_RESERVED_3_BITS_E             }
    ,{SIP6_IPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E  , SIP6_IPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E }
    ,{0xFFFFFFFF , 0xFFFFFFFF}
};
/**
* @internal prvCpssDxChLion2PclTcamParityCalc function
* @endinternal
*
* @brief   Function calculates parity values for Policy TCAM standard rules.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] xDataArr[8]              - X data array.
* @param[in] yDataArr[8]              - Y data array.
*
* @param[out] xParityPtr               - (pointer to) calculated X parity
* @param[out] yParityPtr               - (pointer to) calculated Y parity
*                                       None.
*/
GT_VOID prvCpssDxChLion2PclTcamParityCalc
(
    IN  GT_U32 xDataArr[8],
    IN  GT_U32 yDataArr[8],
    OUT GT_U32 *xParityPtr,
    OUT GT_U32 *yParityPtr
)
{
    GT_U32 i;                   /* loop iterator */
    GT_U32 parityIdx;           /* parity bit index */
    GT_U32 globalIdx;           /* global array bit index */
    GT_U32 wordNumber;          /* number of word in array */
    GT_U32 wordOffset;          /* offset in word */
    GT_U32 tempParityX = 0;     /* temp local parity */
    GT_U32 tempParityY = 0;     /* temp local parity */
    GT_U32 parityBitsX[8];      /* temp local parity array */
    GT_U32 parityBitsY[8];      /* temp local parity array */


    cpssOsMemSet(&parityBitsX , 0, sizeof(parityBitsX));
    cpssOsMemSet(&parityBitsY , 0, sizeof(parityBitsY));

    /*
        The parity calculation is as follows:
        Each line in the ecc memory (x & y) is 8 bits long, that represents
        the parity of 208 tcam bits. Each parity bit is in jumps of 8 bits:

            Bit 0 = parity (entry[0],entry[8],entry[16],..)
            Bit 1 = parity (entry[1],entry[9],entry[17],..)
            .
            .
            Bit 7 = parity(entry[7],entry[15],entry[23],..)
    */
    for (parityIdx = 0; parityIdx < 8; parityIdx++)
    {
        for (i = 0 + parityIdx; i < 208; i += 8)
        {
            globalIdx = i % 52 + ((i / 52) << 6);
            wordNumber = globalIdx >> 5;
            wordOffset = globalIdx & 0x1F;

            parityBitsX[parityIdx] ^= U32_GET_FIELD_MAC(xDataArr[wordNumber], wordOffset, 1);
            parityBitsY[parityIdx] ^= U32_GET_FIELD_MAC(yDataArr[wordNumber], wordOffset, 1);
        }

        tempParityX |= parityBitsX[parityIdx] << parityIdx;
        tempParityY |= parityBitsY[parityIdx] << parityIdx;
    }

    *xParityPtr = tempParityX;
    *yParityPtr = tempParityY;

    return;
}

/**
* @internal prvCpssDxChPclWriteTabEntry function
* @endinternal
*
* @brief   Writes data entry triggered by the last word writing.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] baseAddr                 - The base register's address to write to.
*                                      incremented by 4 for each word.
* @param[in] dataWords                - The length of data array in words.
* @param[in] dataArr[]                - The data array to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPclWriteTabEntry
(
    IN    GT_U8                             devNum,
    IN    GT_U32                            portGroupId,
    IN    GT_U32                            baseAddr,
    IN    GT_U32                            dataWords,
    IN    GT_U32                            dataArr[]
)
{
    GT_STATUS rc; /* return code */
    GT_U32    i;  /* loop index  */

    for (i = dataWords; (i > 0); i--)
    {
        if (i == 1)
        {
            /* verify that all words were written before write last one.*/
            /* Because last one triggers write of whole entry.          */
            GT_SYNC;
        }

        rc = prvCpssHwPpPortGroupWriteRegister(
            devNum, portGroupId, baseAddr, *dataArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* increment register address and data pointer */
        baseAddr += 4;
        dataArr  ++;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclErrataTcamEccInterruptLock function
* @endinternal
*
* @brief   Locks/unlocks the task switching and disables/enables the TCAM TCC interrupt
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] lock                     - GT_TRUE - to  task switching and to disable interrupt
*                                      - GT_FALSE - to enable interrupt and to unlock task switching
* @param[in,out] savedStatePtr            - for "lock" - nothing, for "unlock" - saved state
* @param[in,out] savedStatePtr            - for "lock" - place to save state, for "unlock" - nothing
*
* @retval GT_OK                    - success
* @retval GT_NOT_INITIALIZED       - the internal DB for the 'WA' was not initialized
*/
static GT_STATUS prvCpssDxChPclErrataTcamEccInterruptLock
(
    IN    GT_U8                             devNum,
    IN    GT_U32                            portGroupId,
    IN    GT_BOOL                           lock,
    INOUT GT_U32                            *savedStatePtr
)
{
    GT_BOOL   eccIntEn;    /* TCAM TCC interrupt enable          */
    GT_U32    data;        /* work data                          */
    GT_STATUS rc;          /* return code                        */

    if (lock != GT_FALSE)
    {
        /* initialize for abnormal return case */
        *savedStatePtr = 0;
    }

    if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
        devNum, PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E))
    {
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.
        pclInterruptCauseRegAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lock == GT_FALSE)
    {
        /* UNLOCK case */

        eccIntEn = (GT_BOOL)*savedStatePtr;
        rc = GT_OK;

        /* ECC - interrupt enable */

        if (GT_FALSE != eccIntEn)
        {
            PRV_CPSS_INT_SCAN_LOCK();

            /* clear Policy Engine Interrupt Cause register by read */
            rc = prvCpssHwPpPortGroupReadRegister(
                devNum, portGroupId,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.
                    pclInterruptCauseRegAddr,
                &data);

            /* unmask - i.e. event enable.                */
            /* prvCpssEventsMask may return not GT_OK  */
            /* if interrupt was not binded by application */
            /* or interrupts are not used.                */
            /* Need to ignore this fail.                  */
            (void)prvCpssDrvEventsMask(
                devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.
                pclInterruptIndex,
                GT_FALSE);

            PRV_CPSS_INT_SCAN_UNLOCK();
        }

        PRV_CPSS_UNLOCK_BY_MUTEX;

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* LOCK case */
        PRV_CPSS_LOCK_BY_MUTEX;

        PRV_CPSS_INT_SCAN_LOCK();

        prvCpssDrvEventsMaskGet(
            devNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.
                pclInterruptIndex,
            &eccIntEn);

        if (GT_FALSE != eccIntEn)
        {
            /* ECC - interrupt disable                    */
            /* mask - i.e. event disable.                 */
            /* prvCpssEventsMask may return not GT_OK  */
            /* if interrupt was not binded by application */
            /* or interrupts are not used.                */
            /* Need to ignore this fail.                  */
            (void)prvCpssDrvEventsMask(
                devNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E.
                    pclInterruptIndex,
                GT_TRUE);
        }

        PRV_CPSS_INT_SCAN_UNLOCK();

        *savedStatePtr = (GT_U32)eccIntEn;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclLookupParamCheck function
* @endinternal
*
* @brief   Checks lookup paraqmeters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress lookup0
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclLookupParamCheck
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum
)
{
    CPSS_PCL_DIRECTION_ENT     maxDirection;          /* max direction                  */
    CPSS_PCL_LOOKUP_NUMBER_ENT maxIngrLookupNum;      /* max IPCL lookup num            */
    GT_U32                     maxIngrLookup0SubNum;  /* max IPCL lookup0 sublookup num */

    switch (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
            maxDirection = CPSS_PCL_DIRECTION_INGRESS_E;
            maxIngrLookupNum = CPSS_PCL_LOOKUP_1_E;
            maxIngrLookup0SubNum = 0xFFFFFFFF; /* don't care */
            break;
        case CPSS_PP_FAMILY_CHEETAH2_E:
        case CPSS_PP_FAMILY_CHEETAH3_E:
            maxDirection = CPSS_PCL_DIRECTION_EGRESS_E;
            maxIngrLookupNum = CPSS_PCL_LOOKUP_1_E;
            maxIngrLookup0SubNum = 0xFFFFFFFF; /* don't care */
            break;
        default:
            maxDirection = CPSS_PCL_DIRECTION_EGRESS_E;
            maxIngrLookupNum = CPSS_PCL_LOOKUP_0_1_E;
            maxIngrLookup0SubNum = 1;
            break;
    }

    if ((GT_U32)direction > (GT_U32)maxDirection)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        if ((GT_U32)lookupNum > (GT_U32)maxIngrLookupNum)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (lookupNum == CPSS_PCL_LOOKUP_0_E)
        {
            if (subLookupNum > maxIngrLookup0SubNum)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        if (lookupNum == CPSS_PCL_LOOKUP_1_E)
        {
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleAbsoluteIndexGet function
* @endinternal
*
* @brief   Gets absolute rule index, i.e. it's action index in action table.
*         It also is an index of the first standard-sized part of the rule.
*         The application rule index is an amount of rules with the same
*         size that can be placed before the given rule
*         In complicated TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule
* @param[in] ruleSegment              - the number of STD-rule sized segment of rule
*
* @param[out] ruleAbsIndexPtr          - (pointer to) rule absolute index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleAbsoluteIndexGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  GT_U32                            ruleIndex,
    IN  GT_U32                            ruleSegment,
    OUT GT_U32                            *ruleAbsIndexPtr
)
{
    GT_U32 tcamRaws; /* amount of rows in the TCAM */
    GT_U32  columnEntries=0;/* amount of entries in column */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,ruleIndex);

        if (ruleSegment != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *ruleAbsIndexPtr = ruleIndex;
        return GT_OK;

    }

    tcamRaws = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws;
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
        case CPSS_PP_FAMILY_CHEETAH2_E:
            break;
        case CPSS_PP_FAMILY_CHEETAH3_E:
            columnEntries = 3584/*3.5K*/;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        case CPSS_PP_FAMILY_DXCH_LION_E:
            columnEntries = 768 /*0.75K*/;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            columnEntries = 256 /*0.25K*/;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            columnEntries = 512 /*0.5K*/;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(tcamRaws == 0)
    {
        /* the device not support the TCAM */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* DXCH and DXCH2 devices */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        switch (ruleSize)
        {
            case 1: /* standard */
                if ((ruleIndex >= (tcamRaws * 2)) || (ruleSegment > 0))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr =
                    ((ruleIndex / tcamRaws) * 512) + ((ruleIndex % tcamRaws));
                return GT_OK;

            case 2: /* extended */
                if ((ruleIndex >= tcamRaws) || (ruleSegment > 1))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr = ruleIndex + (ruleSegment * 512);
                return GT_OK;

            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        == CPSS_PP_FAMILY_CHEETAH3_E)
    {
    /* the TCAM is a matrix of PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS rows */
    /* with 4 standard rules or 2 extended rules or one ultra rule.   */
    /* Rule indexes are increasing first in column (top to down) and  */
    /* then in row:                                                   */
    /*    **********************                                      */
    /*    *  0    *  n     * ...                                      */
    /*    **********************                                      */
    /*    *  1    *  n+1   * ...                                      */
    /*    **********************                                      */
    /*    *  2    *  n+2   * ...                                      */
    /*    **********************                                      */
    /*    *  .    *  .     * ...                                      */
    /*    **********************                                      */
    /*    *  .    *  .     * ...                                      */
    /*    **********************                                      */
    /*    *  n-1  *  2n-1  * ...                                      */
    /*    **********************                                      */

        switch (ruleSize)
        {
            case 1: /* standard */
                if ((ruleIndex >= (tcamRaws * 4)) || (ruleSegment > 0))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr =
                    ((ruleIndex / tcamRaws) * columnEntries)
                    + ((ruleIndex % tcamRaws));
                return GT_OK;

            case 2: /* extended */
                if ((ruleIndex >= (tcamRaws * 2)) || (ruleSegment > 1))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr =
                    /* skip columns pairs */
                    ((ruleIndex / tcamRaws)
                        * (2 * columnEntries))
                    /* offset in column that contains the origin */
                        + (ruleIndex % tcamRaws)
                    /* skip columns with the previous segments */
                        + (ruleSegment * columnEntries);
                return GT_OK;

            case 3: /* ultra */
                if ((ruleIndex >= tcamRaws) || (ruleSegment > 2))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr = ruleIndex +
                    (ruleSegment * columnEntries);
                return GT_OK;

            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else /* xCat, xCat3, Lion, xCat2 */
    {
    /* the TCAM is a matrix of PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS rows */
    /* with 4 standard rules or 2 extended rules or one ultra rule.   */
    /* Rule indexes are increasing first in row and then in column    */
    /* (top to down):                                                 */
    /*    *************************                                   */
    /*    *  0  *  1  *  2  *  3  *                                   */
    /*    *************************                                   */
    /*    *  4  *  5  *  6  *  7  *                                   */
    /*    *************************                                   */
    /*    *  .  *  .  *  .  *  .  *                                   */
    /*    *************************                                   */

        switch (ruleSize)
        {
            case 1: /* standard */
                if ((ruleIndex >= (tcamRaws * 4)) || (ruleSegment > 0))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr = ruleIndex;
                return GT_OK;

            case 2: /* extended */
                if ((ruleIndex >= (tcamRaws * 2)) || (ruleSegment > 1))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr = (ruleIndex * 2) + ruleSegment;
                return GT_OK;

            case 3: /* ultra */
                if ((ruleIndex >= tcamRaws) || (ruleSegment > 2))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *ruleAbsIndexPtr = (ruleIndex * 4) + ruleSegment;
                return GT_OK;

            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
}

/**
* @internal prvCpssDxChPclTcamOperationWait function
* @endinternal
*
* @brief   Waits for operation completion (if started)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamOperationWait
(
    IN  GT_U8                         devNum
)
{
    /* wait for bit 0 to clear */
    return prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamOperationTrigger,
        0,GT_FALSE);
}

/**
* @internal prvCpssDxChPclTcamOperationStart function
* @endinternal
*
* @brief   Starts Read/Write/Invalidate rule(standard and extended)
*         Mask/Pattern/Action
*         Does exchance between intermediate regsters set and TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule, (APPLICABLE RANGES: 0..1023), extended (APPLICABLE RANGES: 0..511)
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
* @param[in] tcamOperation            - read Action, write Mask/Pattern/Action, invalidate
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
 GT_STATUS prvCpssDxChPclTcamOperationStart
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  GT_U32                            ruleIndex,
    IN  GT_BOOL                           waitPrevReady,
    IN  PRV_CPSS_DXCH_PCL_TCAM_OPERATION  tcamOperation
)
{
    GT_U32       data;      /* register data               */
    GT_U32       absIndex;  /* absolute rule index         */
    GT_U32       valid;     /* valid PCE-1, invalid-0      */
    GT_U32       extended;  /* extended PCE-1, standard-0  */
    GT_U32       action;    /* action-1, TCAM-0            */
    GT_U32       mask;      /* type of TCAM data (pattern or mask) */
    GT_U32       write;     /* to write-1, to read-0       */
    GT_STATUS    rc;        /* return code                 */
    GT_U32       regAddr;   /* register address            */
    GT_U32       accParma1RegAddr;   /* access param1 register address */
    GT_U32       compMode;  /* Cheetah2 compare mode       */
    GT_U32       accParam1; /* indirect access param1 CH3  */
    GT_U32       indexInBank; /* action index in one of 4 banks CH3  */
    GT_U32       indexOfBank; /* index of one of 4 action banks CH3  */

    TCAM_OPER_TRACE_MAC((
        "TCAM_OPER: dev: %d, ruleIndex: %d, extRule: %d, operation: %d\n",
        devNum, ruleIndex, extRule, tcamOperation));

    if (waitPrevReady != GT_FALSE)
    {
        rc = prvCpssDxChPclTcamOperationWait(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamOperationTrigger;

    /* set valid bit, only relevant when a write transaction is performed
       to the TCAM Data field */
    valid = (tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_VALID) ? 1 : 0;

    /* set extended bit, only relevant when a write transaction is TCAM data
      or mask fields, */
    extended = (ruleSize != 1) ? 1 : 0;

    /* set table type bit, either Action table or TCAM one */
    action = ((tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_READ_ACTION) ||
              (tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION))
                ? 1 : 0;

    /* set operation type, either read or write.
      Only Action table is read by indirect access. */
    write = (tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_READ_ACTION) ? 0 : 1;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* Cheetah1 */

        /* The mask bit unlike Cheetah 2 means: 0 - mask, 1-pattern*/
        mask = (tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK) ? 0 : 1;
        data = (
              (valid    << 18)
            | (extended << 17)
            | (action   << 16)
            | (mask     << 15)
            | ((ruleIndex & 0x1FFF) << 2)
            | (write    << 1)
            | 1 /* trigger bit */);
    }

    else if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        == CPSS_PP_FAMILY_CHEETAH2_E)
    {
        /* Cheetah2 */
        compMode = 3; /* default: at write mask */
        if (tcamOperation != PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK)
        {
            /* 1 - short pattern, 2 - long pattern */
            compMode = (ruleSize != 1) ? 2 : 1;
        }

        /* The mask bit means: 1 - mask, 0-pattern*/
        mask = (tcamOperation == PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK) ? 1 : 0;
        data = (
              (compMode << 19)
            | (valid    << 18)
            | (extended << 17)
            | (action   << 16)
            | (mask     << 15)
            | ((ruleIndex & 0x1FFF) << 2)
            | (write    << 1)
            | 1 /* trigger bit */);
    }

    else
    {
        /* DXCH3 and above */
        if (tcamOperation != PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        indexInBank = (absIndex % PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS);
        indexOfBank = (absIndex / PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS);

        data = (
                ((indexInBank & 0x2000) << 5)
                | (1 /*action*/   << 16)
                | ((indexInBank & 0x1FFF) << 2)
                | (1 /*write*/    << 1)
                | 1 /* trigger bit */);


        /* 4 "enable bits" for 4 words in the needed bank (of 4 banks) */
        accParam1 = (0x0F << (indexOfBank * 4));

        accParma1RegAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamOperationParam1;

        rc = prvCpssHwPpWriteRegister(devNum, accParma1RegAddr, accParam1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* start operation*/
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, data);

    return rc;
}

/**
* @internal prvCpssDxChPclTcamOperDataPut function
* @endinternal
*
* @brief   Writes PCE(standard and extended) Mask/Pattern/Action data
*         to addressable regster set for use by TCAM operation
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] tcamOperation            - write Mask/Pattern/Action
* @param[in] dataPtr                  - data - 3 words for action,
*                                      6/12 bytes for pattern/mask
*
* @retval GT_OK                    - on success.
* @retval other                    - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamOperDataPut
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  PRV_CPSS_DXCH_PCL_TCAM_OPERATION  tcamOperation,
    IN  GT_U32                            *dataPtr
)
{
    GT_U32       i;         /* loop index                     */
    GT_STATUS    rc;        /* return code                    */
    GT_U32       amount;    /* amount of operation registers  */
    GT_U32       start;     /* first operation register index */
    GT_U32       *regAddr;  /* register address array[12]     */

    regAddr =
        &(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamWriteData[0]);

    /* check that previous operation was completed */
    rc = prvCpssDxChPclTcamOperationWait(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate number of registers and it's addresses */
    switch (tcamOperation)
    {

        case PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION:
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            <= CPSS_PP_FAMILY_CHEETAH2_E)
            {
                /* Cheetah1 & Cheetah2 */
                amount = 3;
                start  = 9;
            }
            else /* CPSS_PP_FAMILY_CHEETAH3_E and above */
            {
                /* Cheetah3 */
                amount = 4;
                start  = 6;
            }
            break;
        case PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_VALID:
        case PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_INVALID:
        case PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK:
            amount = (ruleSize != 1) ? 12 : 6;
            start  = (ruleSize != 1) ? 0 : 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* put action or mask or pattern to data registers */
    for (i = 0; (i < amount); i++)
    {
        rc = prvCpssHwPpWriteRegister(
            devNum, regAddr[start + i], dataPtr[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPclActionSizeAndAllignmentGet function
* @endinternal
*
* @brief   Gets action size and alignment.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] sizePtr                  - (pointer to) amount words in HW action
* @param[out] alignmentPtr             - (pointer to) alignment of HW action in words
*                                       None.
*/
GT_VOID prvCpssDxChPclActionSizeAndAllignmentGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                        *sizePtr,
    OUT GT_U32                        *alignmentPtr
)
{
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        *sizePtr      = BITS_TO_WORDS_MAC(240); /*see SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS*/
        *alignmentPtr = 8;/*32 bytes*/
        return;
    }

    /* other devices */
    *sizePtr      = 4;
    *alignmentPtr = 4;
}

/**
* @internal prvCpssDxCh1PclTcamActionGet function
* @endinternal
*
* @brief   Reads PCE(standard and extended) Action data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of PCE  - owner of action to read
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
*
* @param[out] dataPtr                  - data - 4 words for action
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS prvCpssDxCh1PclTcamActionGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       waitPrevReady,
    OUT GT_U32                        *dataPtr
)
{
    GT_U32       i;         /* loop index                     */
    GT_STATUS    rc;        /* return code                    */
    GT_U32       *regAddr;  /* register address array[12]     */
    GT_U32       regAddrDr; /* register address direct read   */

    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH2_E) &&
        (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
           PRV_CPSS_DXCH2_POLICY_ACTION_DIRECT_BURST_ONLY_READ_WA_E)
            == GT_FALSE))
    {
        /* Cheetah2 direct action table access address */
        regAddrDr = 0x0B8C0000;

        for (i = 0; (i < 3); i++)
        {
            rc = prvCpssHwPpReadRegister(
                devNum, (regAddrDr + (0x10 * ruleIndex) + (i * 4)),
                &(dataPtr[i]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* Cheetah && Cheetah2 indirect read */
    regAddr =
        &(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamWriteData[0]);

    /* write command to read Action entry and put it to data registers */
    rc = prvCpssDxChPclTcamOperationStart(
        devNum, ruleSize, ruleIndex, waitPrevReady,
        PRV_CPSS_DXCH_PCL_TCAM_READ_ACTION);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* wait that operation was completed */
    rc = prvCpssDxChPclTcamOperationWait(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* read action entry from data registers */
    for (i = 0; (i < 3); i++)
    {
        rc = prvCpssHwPpReadRegister(
            devNum, regAddr[i + 9], &(dataPtr[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamActionGet function
* @endinternal
*
* @brief   Reads PCE(standard and extended) Action data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      a. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of PCE  - owner of action to read
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
*                                      relevant only for DxCh1 and DxCh2 indirect read
*                                      ignored for DxCh3 and above devices
*
* @param[out] dataPtr                  - HW data for action
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamActionGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       waitPrevReady,
    OUT GT_U32                        *dataPtr
)
{
    GT_U32       i;         /* loop index                     */
    GT_STATUS    rc;        /* return code                    */
    GT_U32       regAddrDr; /* register address direct read   */
    GT_U32       absIndex;  /* action index in action table   */
    GT_U32       actionSize;/* size of action in words        */
    GT_U32       actionAlgn;/* alignment of action in words   */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* convert size,index --> to absIndex */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        return prvCpssDxChPortGroupReadTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                 absIndex,
                                                 dataPtr);

    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        return prvCpssDxCh1PclTcamActionGet(
            devNum, ruleSize, ruleIndex, waitPrevReady, dataPtr);
    }

    prvCpssDxChPclActionSizeAndAllignmentGet(
        devNum, &actionSize, &actionAlgn);

    /* DXCH3 and above - direct read */
    rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
        devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddrDr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.actionTableBase
        + (actionAlgn * 4 * absIndex);

    for (i = 0; (i < actionSize); i++)
    {
        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, (regAddrDr + (4 * i)), &(dataPtr[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamReadStdRuleAddrGet function
* @endinternal
*
* @brief   Calculate standard rule Mask/Pattern direct read address
*         both data and control registers
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] wordIndex                - index of word in the rule
* @param[in] pattern                  - GT_TRUE - pattern, else mask
*
* @param[out] addrPtr                  - address of:  6 short rule words for pattern/mask
*                                      the long rule represented as two shorts
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note For Cheetah line consists of 8 registers
*       0,1,2,4,5,6 - 32-bit data,
*       3,7 - control (bit1 - "valid", bit3 - "long")
*       For Cheetah2 line consists of 10 registers
*       called in the document "data0-4" and "control0-4"
*       the control1 and control2 also contain 16 bits of data.
*       The short rule contains 24 8 = 192 bits,
*       "control" means bit19-18 "compMode", bit17 "valid"
*       "compMode" is 1 - short, 2 - long in pattern and 3 in mask
*       The representation in this program is
*       word0 - data-bit31:0
*       word1 - "control"
*       word2 - data-bit63:32
*       word3 - "control" data-bit79:64
*       word4 - data-bit111:80
*       word5 - "control" data-bit127:112
*       word6 - data-bit159:128
*       word7 - "control"
*       word8 - data-bit191:160
*       word9 - "control"
*       For Cheetah3 line consists of 8 registers
*       called in the document "data0-3" and "control0-3
*       the control0-3 also contain 16 bits of data each (bits 15:0).
*       The short rule contains 24 8 = 192 bits,
*       "control" means bit19-18 "compMode", bit17 "valid" , bit16 "spare"
*       "compMode" is 1 - standard, 2 - extended, 3 - ultra
*       The representation in this program is
*       word0 - data-bit31:0
*       word1 - "control" data-bit47:32
*       word2 - data-bit79:48
*       word3 - "control" data-bit95:80
*       word4 - data-bit127:96
*       word5 - "control" data-bit143:128
*       word6 - data-bit175:144
*       word7 - "control" data-bit191:176
*
*/
GT_STATUS prvCpssDxChPclTcamReadStdRuleAddrGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        wordIndex,
    IN  GT_BOOL                       pattern,
    OUT GT_U32                        *addrPtr
)
{
    GT_U32       regAddr;  /* register address for direct read    */
    GT_U32       wordOff;  /* word address offset                 */
    GT_U32       maxWordIndex;   /* max Word Index                */
    GT_U32       *tcamReadWordOffsetPtr; /*offsets for words      */
    GT_U32       tcamReadEntryInc; /* address increment per entry */
    GT_U32       tcamReadMaskBase;    /* tcam Read Mask Base      */
    GT_U32       tcamReadPatternBase; /* tcam Read Pattern Base   */

    /* The adreses of read access to TCAM patterns/masks              */
    /* Both data and contol registers in common entry                 */
    /* The values for address calculation by such expression:         */
    /*    ((pattern ? tcamReadPatternBase : tcamReadMaskBase)         */
    /*    + (tcamReadEntryInc * entry_index)                          */
    /*    + tcamReadWordOffset[word_index]                            */

    /* offsets for words of the same TCAM READ entry */
    static GT_U32  ch1TcamReadWordOffset[8] =
        {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C};

    static GT_U32  ch2TcamReadWordOffset[10] =
        {0x00, 0x04, 0x4000, 0x4004, 0x8000, 0x8004,
            0xC000, 0xC004, 0x10000, 0x10004};

    static GT_U32  ch3TcamReadWordOffset[8] =
        {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C};

        switch (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
        {
            case CPSS_PP_FAMILY_CHEETAH_E:
                /* Cheetah1 */
                if (ruleIndex >= 1024)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                maxWordIndex          = 8;
                tcamReadWordOffsetPtr = ch1TcamReadWordOffset;
                tcamReadEntryInc      = 0x20;
                tcamReadMaskBase      = 0x0B810000;
                tcamReadPatternBase   = 0x0B818000;
                break;

            case CPSS_PP_FAMILY_CHEETAH2_E:
                /* Cheetah2 */
                if (ruleIndex >= 1024)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                maxWordIndex          = 10;
                tcamReadWordOffsetPtr = ch2TcamReadWordOffset;
                tcamReadEntryInc      = 0x10;
                tcamReadMaskBase      = 0x0B880008;
                tcamReadPatternBase   = 0x0B880000;
                break;

            case CPSS_PP_FAMILY_CHEETAH3_E:
                /* Cheetah3 */
                if (ruleIndex >= (PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS * 4))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                maxWordIndex          = 8;
                tcamReadWordOffsetPtr = ch3TcamReadWordOffset;
                tcamReadEntryInc      = 0x40;
                tcamReadMaskBase      = 0x0BA00020;
                tcamReadPatternBase   = 0x0BA00000;
                break;

            default:
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                {
                    if (ruleIndex >= _2K)
                    {
                        /* Sip5 */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    /* xCat and above                                     */
                    /* the TCAM sizes are maximal possible for the family */
                    /* the real TCAM size may be less                     */
                    /* switch below only checks TCAM sizes                */
                    switch (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_XCAT_E:
                        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                        case CPSS_PP_FAMILY_DXCH_LION_E:
                            if (ruleIndex >= _3K)
                            {
                                /* xCat, xCat3, Lion */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }
                            break;
                        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                            if (ruleIndex >= _1K)
                            {
                                /* xCat2 */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }
                            break;
                        case CPSS_PP_FAMILY_DXCH_LION2_E:
                            if (ruleIndex >= _2K)
                            {
                                /* Lion2 */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }
                            break;
                        default: break;
                    }
                }

                maxWordIndex          = 8;
                tcamReadWordOffsetPtr = ch3TcamReadWordOffset;
                tcamReadEntryInc      = 0x40;

                tcamReadMaskBase      = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadPatternBase + (tcamReadEntryInc/2);
                tcamReadPatternBase   = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadPatternBase;
                break;
        }

    if (wordIndex >= maxWordIndex)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr =
        (pattern != GT_FALSE) ? tcamReadPatternBase : tcamReadMaskBase;

    wordOff = tcamReadWordOffsetPtr[wordIndex];

    *addrPtr = regAddr + (tcamReadEntryInc * ruleIndex) + wordOff;

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamStdRuleDataGet function
* @endinternal
*
* @brief   Reads standard rule Mask/Pattern data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] pattern                  - GT_TRUE - pattern, else mask
*
* @param[out] dataPtr                  - data:  6 words for pattern/mask
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Cheetah and Cheetah2, not relevant for Cheetah3
*
*/
GT_STATUS prvCpssDxChPclTcamStdRuleDataGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       pattern,
    OUT GT_U32                        *dataPtr
)
{
    GT_U8        i,j;         /* loop index                              */
    GT_U32       data;        /* register data                           */
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddr;     /* register address                        */

    PRV_TCAM_REG_TO_DATA_STC *regCopyPtr;
    GT_U8                    regCopyNum;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH_E)
    {
        regCopyPtr = prvCpssDxChPclCh1Reg2Data;
        regCopyNum = prvCpssDxChPclCh1CopyNum;
    }
    else
    {
        regCopyPtr = prvCpssDxChPclCh2Reg2Data;
        regCopyNum = prvCpssDxChPclCh2CopyNum;
    }

    /* clear oputput data */
    for (i = 0; (i < 6); i++)
    {
        dataPtr[i] = 0;
    }

    for (i = 0; (i < regCopyNum); i++)
    {
        rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, regCopyPtr[i].tcamWordIndex, pattern, &regAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (j = 0; (j < 2); j++)
        {
            switch (regCopyPtr[i].copyTo[j].copyRule)
            {
                case PRV_TCAM_REG_COPY_NONE_E:
                    /* nothing need */
                    break;
                case PRV_TCAM_REG_COPY_32_E:
                    dataPtr[regCopyPtr[i].copyTo[j].dataWordIndex] |= data;
                    break;
                case PRV_TCAM_REG_COPY_16LOW2LOW_E:
                    dataPtr[regCopyPtr[i].copyTo[j].dataWordIndex] |=
                        (data & 0x0000FFFF);
                    break;
                case PRV_TCAM_REG_COPY_16LOW2HIGH_E:
                    dataPtr[regCopyPtr[i].copyTo[j].dataWordIndex] |=
                        ((data << 16) & 0xFFFF0000);
                    break;
                case PRV_TCAM_REG_COPY_16HIGH2LOW_E:
                    dataPtr[regCopyPtr[i].copyTo[j].dataWordIndex] |=
                        ((data >> 16) & 0x0000FFFF);
                    break;
                /* must never occur */
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxCh1PclTcamRuleDataGet function
* @endinternal
*
* @brief   Reads rule Mask/Pattern data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] maskPtr                  - data:  (ruleSize  6) words for mask
* @param[out] patternPtr               - data:  (ruleSize  6) words for pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxCh1PclTcamRuleDataGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *maskPtr,
    OUT GT_U32                        *patternPtr
)
{
    GT_STATUS    rc;          /* return code                             */
    GT_U32       i;           /* loop index                              */
    GT_U32       absIndex;    /* absolute rule index                     */

    for (i = 0; (i < ruleSize); i++)
    {
        /* the i-th part of the data */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, i/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPclTcamStdRuleDataGet(
            devNum, absIndex,
            GT_FALSE /*pattern*/, &(maskPtr[i * 6]));
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPclTcamStdRuleDataGet(
            devNum, absIndex,
            GT_TRUE /*pattern*/, &(patternPtr[i * 6]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleDataGet function
* @endinternal
*
* @brief   Reads rule Mask/Pattern data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] maskPtr                  - data:  (ruleSize  6) words for mask
* @param[out] patternPtr               - data:  (ruleSize  6) words for pattern
*                                      for xCat2 and above: (ruleSize  7) words
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *maskPtr,
    OUT GT_U32                        *patternPtr
)
{
    GT_STATUS    rc;          /* return code                  */
    GT_U32       i;           /* loop index                   */
    GT_U32       absIndex;    /* absolute rule index          */
    GT_U32       x,y;         /* work variables               */
    GT_U32       savedStatus; /* ECC interrupt enable/disable */
    GT_STATUS    rc1;         /* return code                  */
    GT_U32       stdRuleWords;/* words in the standard rule   */

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        return prvCpssDxCh1PclTcamRuleDataGet(
            devNum, ruleSize, ruleIndex, maskPtr, patternPtr);
    }

    if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 and above */
        stdRuleWords = 7;
    }
    else
    {
        /* CH devices, xCat, xCat3, Lion */
        stdRuleWords = 6;
    }

    rc = prvCpssDxChPclErrataTcamEccInterruptLock(
        devNum, portGroupId, GT_TRUE/*lock*/, &savedStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; (i < ruleSize); i++)
    {
        /* the i-th part of the data */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, i/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            break;
        }

        rc = prvCpssDxChPclTcamStdRuleDataXandYGet(
            devNum, portGroupId, absIndex,
            &(patternPtr[i * stdRuleWords]),
            &(maskPtr[i * stdRuleWords]));
        if (rc != GT_OK)
        {
            break;
        }
    }

    rc1 = prvCpssDxChPclErrataTcamEccInterruptLock(
        devNum, portGroupId, GT_FALSE/*lock*/, &savedStatus);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert from X/Y format to pattern/mask format */
    for (i = 0; (i < (ruleSize * stdRuleWords)); i++)
    {
        x = patternPtr[i];
        y = maskPtr[i];
        patternPtr[i] = PRV_CPSS_DXCH_PCL_XY_TO_P_MAC(x, y);
        maskPtr[i] =    PRV_CPSS_DXCH_PCL_XY_TO_M_MAC(x, y);
    }

    return rc1;
}

/**
* @internal prvCpssDxChPclTcamStdRuleStateGet function
* @endinternal
*
* @brief   Reads standard rule control and converts it to state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] validPtr                 -  GT_TRUE - valid
* @param[out] ruleSizePtr              -  sizeof rule, measured in standard rules
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Cheetah and Cheetah2, not relevant for Cheetah3
*
*/
GT_STATUS prvCpssDxChPclTcamStdRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT GT_U32                        *ruleSizePtr
)
{
    GT_U32       data;        /* register data                           */
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddr;     /* register address                        */
    GT_U32       wordOff;     /* control register offset                 */
    GT_U32       extendedBit; /* bit to check for extended rule          */
    GT_U32       validBit;    /* bit to check for long rule              */

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* Cheetah1 */
        wordOff     = 3;
        extendedBit = 3;
        validBit    = 1;
    }
    else
    {
        /* Cheetah2 */
        wordOff     = 1;
        extendedBit = 19;
        validBit    = 17;
    }

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
        devNum, ruleIndex, wordOff, GT_TRUE /*pattern*/, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *validPtr    = ((data & (1 << validBit)) != 0) ? GT_TRUE : GT_FALSE;
    *ruleSizePtr = ((data & (1 << extendedBit)) != 0) ? 2 : 1;

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleStateGet function
* @endinternal
*
* @brief   Reads standard rule control and converts it to state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] validPtr                 -  GT_TRUE - valid
* @param[out] ruleFoundSizePtr         -  sizeof rule, measured in standard rules
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT GT_U32                        *ruleFoundSizePtr
)
{
    GT_STATUS    rc;          /* return code                  */
    GT_U32       absIndex;    /* absolute rule index          */
    GT_U32       xValid;      /* xValid                       */
    GT_U32       yValid;      /* yValid                       */
    GT_U32       xSize;       /* xSize                        */
    GT_U32       ySize;       /* ySize                        */
    GT_U32       xSpare;      /* xSpare                       */
    GT_U32       ySpare;      /* ySpare                       */
    GT_U32       mValid;      /* mValid                       */
    GT_U32       pValid;      /* pValid                       */
    GT_U32       savedStatus; /* ECC interrupt enable/disable */
    GT_STATUS    rc1;         /* return code                  */
    GT_U32       xData;       /* X register data              */
    GT_U32       yData;       /* Y register data              */
    GT_U32       regAddr;     /* register address             */

    rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
        devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        return prvCpssDxChPclTcamStdRuleStateGet(
            devNum, absIndex, validPtr, ruleFoundSizePtr);
    }

    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* CH3, xCat, xCat3, Lion */

        rc = prvCpssDxChPclErrataTcamEccInterruptLock(
            devNum, portGroupId, GT_TRUE/*lock*/, &savedStatus);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPclTcamStdRuleStateXandYGet(
            devNum, portGroupId, absIndex,
            &xValid, &yValid, &xSize, &ySize, &xSpare, &ySpare);

        rc1 = prvCpssDxChPclErrataTcamEccInterruptLock(
            devNum, portGroupId, GT_FALSE/*lock*/, &savedStatus);

        if (rc != GT_OK)
        {
            return rc;
        }

        if (rc1 != GT_OK)
        {
            return rc1;
        }

        /* all 2 bits of the size must be checked in any state */
        if (PRV_CPSS_DXCH_PCL_XY_TO_M_MAC(xSize, ySize) != 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        mValid = PRV_CPSS_DXCH_PCL_XY_TO_M_MAC(xValid, yValid);
        pValid = PRV_CPSS_DXCH_PCL_XY_TO_P_MAC(xValid, yValid);

        switch (((mValid & 1) << 1) | (pValid & 1))
        {
            case 0: *validPtr = GT_TRUE;  break;  /* don't care */
            case 1: *validPtr = GT_FALSE; break;  /* never match */
            case 2: *validPtr = GT_FALSE; break;  /* match 0 only */
            case 3: *validPtr = GT_TRUE;  break;  /* match 1 only */
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch (PRV_CPSS_DXCH_PCL_XY_TO_P_MAC(xSize, ySize))
        {
            case 1: *ruleFoundSizePtr = 1; break;
            case 2: *ruleFoundSizePtr = 2; break;
            case 3: *ruleFoundSizePtr = 3; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        return GT_OK;
    }

    /* xCat2 and above */

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
        devNum, absIndex, 0, GT_TRUE /*X*/, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpPortGroupReadRegister(
        devNum, portGroupId, regAddr, &xData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
        devNum, absIndex, 0, GT_FALSE /*Y*/, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpPortGroupReadRegister(
        devNum, portGroupId, regAddr, &yData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* cut the rule size bits */
    xData &= 3;
    yData &= 3;

    switch (xData)
    {
        case 1: *ruleFoundSizePtr = 1; break;
        case 2: *ruleFoundSizePtr = 2; break;
        case 3: *ruleFoundSizePtr = 3; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if ((PRV_CPSS_DXCH_PCL_XY_TO_M_MAC(xData, yData) == 3)
        && (PRV_CPSS_DXCH_PCL_XY_TO_P_MAC(xData, yData) == xData))
    {
        *validPtr = GT_TRUE;
    }
    else
    {
        *validPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxCh1PclTcamRuleWrite function
* @endinternal
*
* @brief   Writes standard or extended rule Action, Mask and Pattern to TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] validRule                - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
* @param[in] actionPtr                - action
* @param[in] maskPtr                  - mask
* @param[in] patternPtr               - pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS prvCpssDxCh1PclTcamRuleWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       validRule,
    IN  GT_BOOL                       waitPrevReady,
    IN  GT_U32                        *actionPtr,
    IN  GT_U32                        *maskPtr,
    IN  GT_U32                        *patternPtr
)
{
    GT_STATUS                        rc;       /* return code            */
    PRV_CPSS_DXCH_PCL_TCAM_OPERATION tcamOper; /* write valid or invalid */

    /* CH and CH2 devices */
    tcamOper = (validRule == GT_FALSE)
        ? PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_INVALID
        : PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_VALID;

    /* invalidate rule */
    rc = prvCpssDxChPclTcamOperationStart(
        devNum, ruleSize, ruleIndex, waitPrevReady,
        PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_INVALID);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* put action data to buffer registers */
    rc = prvCpssDxChPclTcamOperDataPut(
        devNum, ruleSize,
        PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION, actionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* store action */
    rc = prvCpssDxChPclTcamOperationStart(
        devNum, ruleSize, ruleIndex, GT_FALSE /*waitPrevReady*/,
        PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* put mask data to buffer registers */
    rc = prvCpssDxChPclTcamOperDataPut(
        devNum, ruleSize,
        PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK, maskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* store mask */
    rc = prvCpssDxChPclTcamOperationStart(
        devNum, ruleSize, ruleIndex, GT_FALSE /*waitPrevReady*/,
        PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* put pattern data to buffer registers */
    rc = prvCpssDxChPclTcamOperDataPut(
        devNum, ruleSize,
        tcamOper, patternPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* store pattern */
    rc = prvCpssDxChPclTcamOperationStart(
        devNum, ruleSize, ruleIndex, GT_FALSE /*waitPrevReady*/,
        tcamOper);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleWrite function
* @endinternal
*
* @brief   Writes standard or extended rule Action, Mask and Pattern to TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] validRule                - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
* @param[in] actionPtr                - action
* @param[in] maskPtr                  - mask
* @param[in] patternPtr               - pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*
* @note For all devices the mask and pattern of rule
*       is a sequence standard sized segments.
*       For all CH devices, xCat and Lion such segment is
*       192 bits in 6 32-bit words.
*       For xCat2 and above devices such segment is
*       206 bits in 7 32-bit words, padded 18 bits unused.
*
*/
GT_STATUS prvCpssDxChPclTcamRuleWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       validRule,
    IN  GT_BOOL                       waitPrevReady,
    IN  GT_U32                        *actionPtr,
    IN  GT_U32                        *maskPtr,
    IN  GT_U32                        *patternPtr
)
{
    GT_STATUS    rc;                           /* return code            */
    GT_U32       x[7];                         /* TCAM X - data          */
    GT_U32       y[7];                         /* TCAM Y - data          */
    GT_U32       seg;                          /* rule segment index     */
    GT_U32       absIndex;                     /* absolute rule index    */
    GT_U32       i;                            /* loop index             */
    GT_U32       j;                            /* work index             */
    GT_BOOL      segValid;                     /* rule segment valid     */
    GT_U32       actionStartAddr;              /* action start address in action table */
    GT_U32       stdRuleWords;                 /* words in STD rule data */
    GT_U32       actionSize;                   /* size of action in words        */
    GT_U32       actionAlgn;                   /* alignment of action in words   */
    GT_U32       oldRuleSize;                  /* size of existing rule  for given rule index */
    GT_BOOL      oldRuleValid = GT_FALSE;      /* validity of existing rule for given rule index */
    GT_BOOL      isEqual = GT_FALSE;           /* result of memory compare */
    GT_BOOL      isRuleExist = GT_TRUE;        /* if rule is exist already in given rule index*/


    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         <= CPSS_PP_FAMILY_CHEETAH2_E)
    {
        /* DxCh1 and DxCh1 devices */
        return prvCpssDxCh1PclTcamRuleWrite(
            devNum, ruleSize, ruleIndex, validRule, waitPrevReady,
            actionPtr, maskPtr, patternPtr);
    }
    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* CH3, xCat, xCat3, Lion */
        stdRuleWords = 6;
    }
    else
    {
        /* xCat2 and above */
        stdRuleWords = 7;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable == GT_TRUE)
    {
        /* Make needed checks to verify that rule possibly configured before in given index is different
           from new one is going to be written. If both rules are the same - no write operation is performed*/
        /* get validity and size of previous rule */
        rc = prvCpssDxChPclTcamRuleStateGet(
            devNum, portGroupId, ruleSize, ruleIndex, &oldRuleValid, &oldRuleSize);
        if (rc == GT_BAD_STATE)
        {
            isRuleExist = GT_FALSE;
        }
        else
        {
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if ((isRuleExist == GT_TRUE) && (oldRuleValid == GT_TRUE) && (oldRuleSize == ruleSize ))
        {
            GT_U32 oldActionPtr[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS]; /* action of existing rule  for given rule index*/
            GT_U32 oldMaskPtr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];     /* mask of existing rule  for given rule index   */
            GT_U32 oldPatternPtr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];  /* pattern of existing rule  for given rule index */
            cpssOsMemSet(oldActionPtr,0,sizeof(oldActionPtr));
            /* check rule contents and action*/
            /* get action of previous rule */
            rc = prvCpssDxChPclTcamActionGet(devNum, portGroupId, ruleSize,
                                             ruleIndex,GT_TRUE,oldActionPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* compare new and old action */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) actionPtr,
                                         (GT_VOID*) oldActionPtr,
                                         sizeof(oldActionPtr))) ? GT_TRUE : GT_FALSE;
            if (isEqual == GT_TRUE)
            {
                 cpssOsMemSet(oldMaskPtr,0,sizeof(oldMaskPtr));
                 cpssOsMemSet(oldPatternPtr,0,sizeof(oldPatternPtr));
                /* now check mask and pattern */
                rc = prvCpssDxChPclTcamRuleDataGet(
                    devNum, portGroupId, ruleSize, ruleIndex, oldMaskPtr, oldPatternPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /* compare new and old pattern */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) patternPtr,
                                             (GT_VOID*) oldPatternPtr,
                                             (sizeof(GT_U32)*stdRuleWords*ruleSize))) ? GT_TRUE : GT_FALSE;
                if (isEqual == GT_TRUE)
                {
                    /* compare new and old mask */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) maskPtr,
                                                 (GT_VOID*) oldMaskPtr,
                                                 (sizeof(GT_U32)*stdRuleWords*ruleSize))) ? GT_TRUE : GT_FALSE;
                    if (isEqual == GT_TRUE)
                    {
                        /* old and new rule are completely identical */
                        return GT_OK;
                    }
                }
            }
        }

    }

    /* invalidate rule */
    rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
        devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* invalidate rule */
    rc = prvCpssDxChPclTcamStdRuleValidStateSet(
        devNum, portGroupId, absIndex, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* convert size,index --> to absIndex */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                 absIndex,
                                                 actionPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    else
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        <= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        /* for CH3 indirect action write */
        /* put action data to buffer registers */
        rc = prvCpssDxChPclTcamOperDataPut(
            devNum, ruleSize,
            PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION, actionPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* store action */
        rc = prvCpssDxChPclTcamOperationStart(
            devNum, ruleSize, ruleIndex, GT_FALSE /*waitPrevReady*/,
            PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else /* xCat and above */
    {
        prvCpssDxChPclActionSizeAndAllignmentGet(
            devNum, &actionSize, &actionAlgn);

        /* direct action write                       */
        actionStartAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.actionTableBase
            + (actionAlgn * 4 * absIndex);

        rc = prvCpssDxChPclWriteTabEntry(
            devNum, portGroupId,
            actionStartAddr, actionSize, actionPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
    }

  /* end of loop done by "break" */
    for (seg = (ruleSize - 1); (1) ; seg--)
    {
        /* convert pattern/mask to X/Y format */
        for (i = 0; (i < stdRuleWords); i++)
        {
            j = (seg * stdRuleWords) + i;
            x[i] = PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(maskPtr[j], patternPtr[j]);
            y[i] = PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(maskPtr[j], patternPtr[j]);
        }

        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, seg/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((validRule == GT_FALSE) && (seg == 0))
        {
            /* for long rule only first segment may be invalid */
            segValid = GT_FALSE;
        }
        else
        {
            segValid = GT_TRUE;
        }

        rc = prvCpssDxChPclTcamStdRuleSizeAndValidSet(
            devNum, portGroupId, absIndex, x, y,
            segValid, ruleSize);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* end of loop */
        if (seg == 0) break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleActionUpdate function
* @endinternal
*
* @brief   Writes standard or extended rule Action to TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] actionPtr                - action
*
* @retval GT_OK                    - -on success.
*                                       GT_FAIL     on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamRuleActionUpdate
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *actionPtr
)
{
    GT_STATUS    rc;                /* return code                          */
    GT_U32       absIndex;          /* absolute rule index                  */
    GT_U32       actionStartAddr;   /* action start address in action table */
    GT_U32       actionSize;        /* size of action in words        */
    GT_U32       actionAlgn;        /* alignment of action in words   */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* convert size,index --> to absIndex */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
            devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        return prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                 absIndex,
                                                 actionPtr);
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* DxCh1-3 indirect write              */
        /* put action data to buffer registers */
        rc = prvCpssDxChPclTcamOperDataPut(
            devNum, ruleSize,
            PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION, actionPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* store action */
        rc = prvCpssDxChPclTcamOperationStart(
            devNum, ruleSize, ruleIndex, GT_FALSE /*waitPrevReady*/,
            PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION);

        return rc;
    }
    else
    {
        /* direct action write                       */
        /* action table line alignment is 0x10 bytes */
        rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(
        devNum, ruleSize, ruleIndex, 0/*ruleSegment*/, &absIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvCpssDxChPclActionSizeAndAllignmentGet(
            devNum, &actionSize, &actionAlgn);

        actionStartAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.actionTableBase
            + (actionAlgn * 4 * absIndex);

        rc = prvCpssDxChPclWriteTabEntry(
            devNum, portGroupId,
            actionStartAddr, actionSize, actionPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleCopy function
* @endinternal
*
* @brief   The function dedicated for one of port groups of multi port group device or
*         for not multi port group device.
*         The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate should be used to invalidate
*         Rule in old position
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSizeVal              - size of Rule in STD rules.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPclRuleCopy
(
    IN GT_U8                              devNum,
    IN GT_U32                             portGroupId,
    IN GT_U32                             ruleSizeVal,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
)
{
    GT_STATUS    rc;          /* return code                 */
    GT_U32       action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS]; /* action */
    GT_U32       mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];     /* mask    */
    GT_U32       pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];  /* pattern */
    GT_BOOL      valid;       /* valid source rule           */
    GT_U32       ruleSizeFound; /* previously configured rule size by given rule index */

    /* get Validity and size of source rule */
    rc = prvCpssDxChPclTcamRuleStateGet(
        devNum, portGroupId, ruleSizeVal, ruleSrcIndex, &valid, &ruleSizeFound);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ruleSizeVal != ruleSizeFound)
    {
        /* source rule has another size */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* get action of source rule */
    rc = prvCpssDxChPclTcamActionGet(
        devNum, portGroupId, ruleSizeVal, ruleSrcIndex,
        GT_TRUE /*waitPrevReady*/, action);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get mask and pattern */
    rc = prvCpssDxChPclTcamRuleDataGet(
        devNum, portGroupId, ruleSizeVal, ruleSrcIndex, mask, pattern);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write destination rule */
    return prvCpssDxChPclTcamRuleWrite(
        devNum, portGroupId, ruleSizeVal, ruleDstIndex, valid,
        GT_TRUE /*waitPrevReady*/,
        action, mask, pattern);
}


/**
* @internal prvCpssDxChPclSip5RuleCopy function
* @endinternal
*
* @brief   This function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move TCAM
*         Rule from old position to new one at first prvCpssDxChTcamRuleCopy
*         should be called. And after this cpssDxChTcamGortGroupRuleValidStatusSet should
*         be used to invalidate rule in old position.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - size of rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvCpssDxChPclSip5RuleCopy
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSize,
    IN  GT_U32                        ruleSrcIndex,
    IN  GT_U32                        ruleDstIndex
)
{

    GT_STATUS    rc;                                             /* return code */
    GT_U32       action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS]; /* Size of action in Bobcat2; Caelum; Bobcat3 is 8 words for TTI and PCL */
    GT_U32       mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];     /* Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words for PCL. for TTI it's 8 words */
    GT_U32       pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];  /* Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words for PCL. for TTI it's 8 words */
    GT_BOOL      valid;                                          /* valid source rule */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSizeFound;                  /* previously configured rule size by given rule index */
    GT_U32       entryNumber;                                    /* Rule's index in TCAM */
    GT_U32       dummy;

    GT_PORT_GROUPS_BMP portGroupsBmp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    portGroupsBmp = 1 << portGroupId;

    /* get Validity and size of source rule */
    rc = cpssDxChTcamPortGroupRuleValidStatusGet(devNum,portGroupsBmp,ruleSrcIndex,&valid,&ruleSizeFound);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ruleSize != ruleSizeFound)
    {
        /* source rule has another size */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Converts rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,ruleSrcIndex,&entryNumber,&dummy);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get action of source rule */
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                            portGroupId,
                                            CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                            (entryNumber/2),
                                            action);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get mask and pattern */
    rc =  cpssDxChTcamPortGroupRuleRead(devNum,portGroupsBmp,ruleSrcIndex,&valid,&ruleSizeFound,pattern,mask);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Converts rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,ruleDstIndex,&entryNumber,&dummy);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* write action to destination rule */
    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                             (entryNumber/2),
                                             action);

    /* write destination rule */
    return cpssDxChTcamPortGroupRuleWrite(devNum,portGroupsBmp,ruleDstIndex,valid,ruleSize,pattern,mask);

}

/**
* @internal prvCpssDxChXCatPclTcamRuleIngressActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Ingress Action from SW to HW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (4 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
static GT_STATUS prvCpssDxChXCatPclTcamRuleIngressActionSw2HwConvert
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
)
{
    GT_STATUS                              rc;           /* return code            */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;   /* DSA  Cpu Code          */
    GT_U32                                 rawCpuCode;   /* raw  Cpu Code          */
    GT_U32                                 hwValue;      /* value for enums        */
    GT_U32                                 routerLttIndex = 0; /* router LTT Index       */
    GT_U32                                 ppVersion;    /* PP version             */
    GT_U32                                 tempRouterLttPtr = 0;
    GT_U32                                 maxMatchCounterIndex; /* max Match Counter Index */

    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;
    GT_U32                                 maxValue;     /* (max valid value + 1)*/
    GT_HW_DEV_NUM                          hwDev = 0;
    GT_PORT_NUM                            hwPort = 0;
    GT_BOOL                                supportRouteEngine = GT_TRUE;/*indication that Router engine supported (for LTT and VRF)*/

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            ppVersion = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_LION_E:
            ppVersion = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:/*lion2 format like xcat2*/
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            ppVersion = 2;
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                supportRouteEngine = GT_FALSE;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* clear */
    cpssOsMemSet(
        hwActionPtr, 0,
        (4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    if (swActionPtr->egressPolicy != GT_FALSE)
    {
        /* egress action must be translated by DxCh3 related code */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (swActionPtr->ipUcRoute.doIpUcRoute != GT_FALSE)
    {
        /* ingress Uc Rout to NextHop action must be translated by */
        /* DxCh3 related code                                      */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* ingress action */

    /* calculate HW_CPU_CODE if needed */

    if ((swActionPtr->pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E)
        || (swActionPtr->pktCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E))
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(
            swActionPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rawCpuCode = dsaCpuCode;
    }
    else
    {
        /* default */
        rawCpuCode = 0;
    }

    /* GENERAL */

    /* packet command - word0, bits 2:0 */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
        hwValue, swActionPtr->pktCmd);
    /* bits 2:0 */
    hwActionPtr[0] |= hwValue;


    /* prevent the next lookups */
    hwActionPtr[2] |= ((BOOL2BIT_MAC(swActionPtr->actionStop)) << 15);

    /* bypass bridge */
    hwActionPtr[2] |= ((BOOL2BIT_MAC(swActionPtr->bypassBridge)) << 16);

    /* bypass ingress pipe */
    hwActionPtr[2] |= ((BOOL2BIT_MAC(swActionPtr->bypassIngressPipe)) << 17);

    if ((swActionPtr->redirect.redirectCmd
        == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E) && (debugBypassFullControl == 0))
    {
        /* If specified redirection to egress interface      */
        /* one of bypassBridge or bypassIngressPipe          */
        /* must be specified - otherwise following ingress   */
        /* engines like bridge may to override output        */
        /* interface or forwarding command                   */
        if ((swActionPtr->bypassIngressPipe == 0)
            && (swActionPtr->bypassBridge == 0))
        {
            /* bypass bridge */
            hwActionPtr[2] |= (1 << 16);
        }
    }

    /* MIRROR */

    /* CPU code */
    hwActionPtr[0] |= ((rawCpuCode & 0x00FF) << 3);

    /* mirror to analyzer port */
    hwActionPtr[0] |=
        ((BOOL2BIT_MAC(swActionPtr->mirror.mirrorToRxAnalyzerPort)) << 11);

    /* mirror TCP disconnect messages to CPU */
    hwActionPtr[3] |=
        ((BOOL2BIT_MAC(swActionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu)) << 17);

    /* LOOKUP */

    /* IPCL0_1 override Cfg table access index */
    hwActionPtr[2] |=
        (((swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex
          == CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0) << 19);

    /* IPCL1 override Cfg table access index */
    hwActionPtr[2] |=
        (((swActionPtr->lookupConfig.pcl1OverrideConfigIndex ==
           CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0) << 18);

    /* IPCL Cfg table access index */
    if (swActionPtr->lookupConfig.ipclConfigIndex != 0)
    {
        if (swActionPtr->redirect.redirectCmd
            == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (swActionPtr->lookupConfig.ipclConfigIndex >= BIT_13)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwActionPtr[0] |= ((swActionPtr->lookupConfig.ipclConfigIndex & 1) << 31);
        hwActionPtr[1] |= (swActionPtr->lookupConfig.ipclConfigIndex >> 1);

    }

    /* REDIRECT */

    switch (swActionPtr->redirect.redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            /* bits 14:12 already contain 0 */;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            hwActionPtr[0] |= (1 << 12);
            /* redirect interface */
            switch (swActionPtr->redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:

                    PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                    hwDev = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                    hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);


                    /* device */
                    maxValue =
                        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(
                            devNum);

                    if(hwDev > maxValue)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }
                    /* dev[4:0] => bits[26:22] */
                    hwActionPtr[0] |=
                        ((hwDev & 0x1F) << 22);

                    /* dev[11:5] => bits[143:137] */
                    hwActionPtr[4] |=
                        ((hwDev & 0xFE0) << 4);

                    /* port */
                    maxValue =
                        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(
                            devNum);

                    if (hwPort > maxValue)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }

                    /* port[5:0] => bits 22:16 */
                    hwActionPtr[0] |=
                        ((hwPort & 0x3F) << 16);

                    /* port[19:6] => bits 157:144 */
                    hwActionPtr[4] |=
                        ((hwPort & 0xFFFC0) << 10);

                    break;

                case CPSS_INTERFACE_TRUNK_E:
                    /* TRUNK IF */
                    hwActionPtr[0] |= (1 << 15);

                    /* trunk ID */
                    maxValue =
                        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(
                            devNum);

                    if(swActionPtr->redirect.data.outIf.outInterface.trunkId
                       > maxValue)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }
                    /* trunkId[6:0] => bits 22:16 */
                    hwActionPtr[0] |=
                        ((swActionPtr->redirect.data.outIf.outInterface.trunkId
                          & 0x7F) << 16);

                    /* trunkId[11:7] => bits 141:137 */
                    hwActionPtr[4] |=
                        ((swActionPtr->redirect.data.outIf.outInterface.trunkId
                          & 0xF80) << 2);
                    break;

                case CPSS_INTERFACE_VIDX_E:
                    /* multicast IF */
                    hwActionPtr[0] |= (1 << 27);

                    /* VIDX */
                    maxValue =
                        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(
                            devNum);

                    if(swActionPtr->redirect.data.outIf.outInterface.vidx
                       > maxValue)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }

                    /* vidx[11:0] => bits 26:15 */
                    hwActionPtr[0] |=
                        ((swActionPtr->redirect.data.outIf.outInterface.vidx
                           & 0xFFF) << 15);

                    /* vidx[15:12] => bits 137:140 */
                    hwActionPtr[4] |=
                        ((swActionPtr->redirect.data.outIf.outInterface.vidx
                           & 0xF000) >> 3);

                    break;

                case CPSS_INTERFACE_VID_E:
                    /* multicast IF */
                    hwActionPtr[0] |= (1 << 27);
                    /* VID - use constant 0xFFF for send to VLAN */
                    /* vidx[11:0] => bits 26:15 */
                    hwActionPtr[0] |= (0x0FFF << 15);
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* tunnel start */
            if (swActionPtr->redirect.data.outIf.tunnelStart == GT_TRUE)
            {
                /* tunnel start */
                hwActionPtr[0] |= (1 << 28);

                maxValue =
                    PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TS_PTR_MAC(
                        devNum);

                if (swActionPtr->redirect.data.outIf.tunnelPtr > maxValue)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
                /* tunnel PTR[2:0] => bits 31:29 */
                hwActionPtr[0] |=
                    ((swActionPtr->redirect.data.outIf.tunnelPtr & 7) << 29);
                /* tunnel PTR[12:3] bits 41:32 */
                hwActionPtr[1] |=
                    ((swActionPtr->redirect.data.outIf.tunnelPtr
                      & 0x1FF8) >> 3);
                /* tunnel PTR[19:13] bits 135:129 */
                hwActionPtr[4] |=
                    ((swActionPtr->redirect.data.outIf.tunnelPtr
                      & 0xFE000) >> 12);
                /* tunnel type */
                switch (swActionPtr->redirect.data.outIf.tunnelType)
                {
                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                        /* the data already contains 0 */
                        break;
                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                        hwActionPtr[1] |= (1 << 10);
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

            }
            else
            {
                /* vntL2Echo */
                hwActionPtr[1] |=
                    ((BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.vntL2Echo))
                     << 11);
                if (ppVersion > 0)
                {
                    maxValue =
                        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(
                            devNum);

                    if (swActionPtr->redirect.data.outIf.arpPtr > maxValue)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }

                    /* arpPtr[2:0] => bits [31:29] */
                    hwActionPtr[0] |=
                        ((swActionPtr->redirect.data.outIf.arpPtr & 7) << 29);

                    /* arpPtr[13:3] => bits [42:32] */
                    hwActionPtr[1] |=
                        ((swActionPtr->redirect.data.outIf.arpPtr
                           & 0x3FF8) >> 3);

                    /* arpPtr[21:14] => bits [136:129] */
                    hwActionPtr[4] |=
                        ((swActionPtr->redirect.data.outIf.arpPtr
                           & 0x1FC000) >> 13);

                    hwActionPtr[3] |=
                        ((BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.modifyMacDa))
                         << 20);
                    hwActionPtr[3] |=
                        ((BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.modifyMacSa))
                         << 21);
                }
            }
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            if (supportRouteEngine == GT_FALSE)
            {
                /* xCat2 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |= (2 << 12);
             /* ROUTER */
            if(swActionPtr->redirect.data.routerLttIndex>= BIT_15)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }


            if (swActionPtr->redirect.data.routerLttIndex > (fineTuningPtr->tableSize.router * 4))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

            /* FEr#2018: Limited number of Policy-based routes */
            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH3_LIMITED_NUMBER_OF_POLICY_BASED_ROUTES_WA_E))
            {
                /* xCat devices support index 0,4,8,12...(max IP TCAM row * 4) only */
                if ((swActionPtr->redirect.data.routerLttIndex % 4) != 0)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            routerLttIndex = swActionPtr->redirect.data.routerLttIndex;

            /* LTT entry is row based however in this field the LTT is treated as column based.
            Bits [11:0] indicate row while bits [14:13] indicate column, bit 12 is not used.
            The Formula for translating the LTT entry to column based is as follow:
            [11:0] << 2 + [14:13]   (Note: bit 12 not used). */
                tempRouterLttPtr = (((routerLttIndex & 0x3FFC) >> 2) | ((routerLttIndex & 0x3) << 13));
                hwActionPtr[0] |= (tempRouterLttPtr << 15);

        break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            if (supportRouteEngine == GT_FALSE)
            {
                /* xCat2 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |= (4 << 12);
             /* VIRT_ROUTER */
            if(swActionPtr->redirect.data.vrfId>= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |=
                (swActionPtr->redirect.data.vrfId << 15);
            break;
         case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E:
            if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_FALSE)
            {
                /* Applicable only for VPLS support */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            if (swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.type
                != CPSS_INTERFACE_PORT_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* redirection command bits[14:12] - 5 */
            hwActionPtr[0] |= (5 << 12);
            /* sourceMeshIdSetEnable - bit[30] */
            hwActionPtr[0] |=
                (BOOL2BIT_MAC(
                    swActionPtr->redirect.data.logicalSourceInterface.sourceMeshIdSetEnable)
                    << 30);
            /* sourceMeshId - bits[29:28] */
            if (swActionPtr->redirect.data.logicalSourceInterface.sourceMeshId >= BIT_2)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |=
                ((swActionPtr->redirect.data.logicalSourceInterface.sourceMeshId & 3) << 28);

            /* userTagAcEnable - bit[27] */
            hwActionPtr[0] |=
                (BOOL2BIT_MAC(
                    swActionPtr->redirect.data.logicalSourceInterface.userTagAcEnable)
                    << 27);

            /* logical source devNum - bits[26:22]  */
            if (swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum
                >= BIT_5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |=
                ((swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum
                  & 0x1F) << 22);

            /* logical source portNum - bits[21:16]  */
            if (swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum
                >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[0] |=
                ((swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum
                  & 0x3F) << 16);

            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Match counter */

    /* bind to CNC counter */
    hwActionPtr[1] |=
        ((BOOL2BIT_MAC(swActionPtr->matchCounter.enableMatchCount))
         << 12);

    /* counter index */
     maxMatchCounterIndex =
        (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_FALSE)
        ? BIT_14 : BIT_13;
    if (swActionPtr->matchCounter.matchCounterIndex >= maxMatchCounterIndex)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[1] |= (swActionPtr->matchCounter.matchCounterIndex << 13);

    /* POLICER */
    switch (swActionPtr->policer.policerEnable)
    {
        case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
            /* assumed hwActionPtr[1] bis 28:27 are 0 */
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
            /* bind to POLICER counter */
            /* bind to POLICER meter */
            hwActionPtr[1] |= (3 << 27);
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
            /* bind to POLICER meter */
            hwActionPtr[1] |= (1 << 27);
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
            /* bind to POLICER counter */
            hwActionPtr[1] |= (1 << 28);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* counter index */
    if (swActionPtr->policer.policerId >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[1] |= ((swActionPtr->policer.policerId & 7) << 29);
    hwActionPtr[2] |= (swActionPtr->policer.policerId >> 3);

    /* SOURCE ID */

    /* source Id assignment enable */
    hwActionPtr[2] |=
        ((BOOL2BIT_MAC(swActionPtr->sourceId.assignSourceId)) << 9);

    /* source Id value */

    maxValue =
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(
            devNum);

    if (swActionPtr->sourceId.sourceIdValue > maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* sourceId[4:0] => bits [78:74] */
    hwActionPtr[2] |=
        ((swActionPtr->sourceId.sourceIdValue & 0x1F) << 10);

    /* sourceId[10:5] => bits [127:122] */
    hwActionPtr[3] |=
        ((swActionPtr->sourceId.sourceIdValue & 0x7E0) << 21);

    /* sourceId[11] => bits [128] */
    hwActionPtr[4] |=
        ((swActionPtr->sourceId.sourceIdValue & 0x800) >> 11);

    /* VLAN assignment */

    /* vlan precedence */
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
        hwValue, swActionPtr->vlan.ingress.precedence);
    hwActionPtr[2] |= (hwValue << 20);

    /* nested VLAN */
    hwActionPtr[2] |=
        ((BOOL2BIT_MAC(swActionPtr->vlan.ingress.nestedVlan)) << 21);

    /* modify vlan command */
    switch (swActionPtr->vlan.ingress.modifyVlan)
    {
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E:
            hwValue = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E:
            hwValue = 1;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E:
            hwValue = 2;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E:
            hwValue = 3;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwActionPtr[2] |= (hwValue << 22);

    /* VLAN id */

    maxValue =
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(
            devNum);

    if (swActionPtr->vlan.ingress.vlanId > maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    /* vlanId[7:0] => bits[95:88] */
    hwActionPtr[2] |=
        ((swActionPtr->vlan.ingress.vlanId & 0xFF) << 24);
    /* vlanId[11:8] => bits[99:96] */
    hwActionPtr[3] |=
        ((swActionPtr->vlan.ingress.vlanId & 0xF00) >> 8);
    /* vlanId[11:8] => bits[118:121] */
    hwActionPtr[3] |=
        ((swActionPtr->vlan.ingress.vlanId & 0xF000) << 10);

    /* QOS */

    /* qos profile assignment enable */
    hwActionPtr[3] |=
        ((BOOL2BIT_MAC(swActionPtr->qos.ingress.profileAssignIndex)) << 4);

    /* qos profile assignment precedence */
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.profilePrecedence)
    hwActionPtr[3] |= (hwValue << 5);

    /* qos profile */
    if (swActionPtr->qos.ingress.profileIndex >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[3] |= (swActionPtr->qos.ingress.profileIndex << 6);

    /* enable modify DSCP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.modifyDscp);
    hwActionPtr[3] |= (hwValue << 13);

    /* enable modify UP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.modifyUp);
    hwActionPtr[3] |= (hwValue << 15);

     if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) != GT_FALSE)
    {
        if (swActionPtr->unknownSaCommandEnable == GT_FALSE)
        {
            /* preserve previous setting */
            hwValue = 7;
        }
        else
        {
            switch (swActionPtr->unknownSaCommand)
            {
                case CPSS_PACKET_CMD_FORWARD_E:
                    hwValue = 0;
                    break;
                case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                    hwValue = 1;
                    break;
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                    hwValue = 2;
                    break;
                case CPSS_PACKET_CMD_DROP_HARD_E:
                    hwValue = 3;
                    break;
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    hwValue = 4;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        /* unknownSaCommand - bits [115:114,58] */
        hwActionPtr[3] |= ((hwValue & 6) << 17);
        hwActionPtr[1] |= ((hwValue & 1) << 26);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChXCatPclTcamRuleIpUcRoutingActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Ip Unicast Routing Action from SW to HW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (4 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
static GT_STATUS prvCpssDxChXCatPclTcamRuleIpUcRoutingActionSw2HwConvert
(
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
)
{
    GT_U32                                 hwValue;      /* value for enums        */
    GT_HW_DEV_NUM                          hwDev = 0;
    GT_PORT_NUM                            hwPort = 0;

    /* Ip Unicast Routing action                       */
    /* Assumed but not checked:                        */
    /* swActionPtr->egressPolicy          == GT_FALSE  */
    /* swActionPtr->ipUcRoute.doIpUcRoute == GT_TRUE   */

    /* clear */
    cpssOsMemSet(
        hwActionPtr, 0,
        (4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    /* GENERAL */

    /* packet command - word0, bits 2:0 */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
        hwValue, swActionPtr->pktCmd);
    /* bits 2:0 */
    hwActionPtr[0] |= hwValue;

    /* MIRROR */

    /* mirror to analyzer port */
    hwActionPtr[0] |=
        ((BOOL2BIT_MAC(swActionPtr->mirror.mirrorToRxAnalyzerPort)) << 11);

    /* REDIRECT */

    /* Action used as next hop entry                  */
    /* swActionPtr->redirect.redirectCmd ignored      */
    /* The data treared as interface                  */
    /* for CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E */
    /* All tunnel related data ignored                */
    hwActionPtr[0] |= (3 << 12);

    /* redirect interface */
    switch (swActionPtr->redirect.data.outIf.outInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
            hwDev = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
            hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
            /* device */
            if(hwDev >= BIT_5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[1] |=
                (hwDev << 9);
            /* port */
            if (hwPort >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[1] |=
                (hwPort << 3);
            break;
        case CPSS_INTERFACE_TRUNK_E:
            /* TRUNK IF */
            hwActionPtr[1] |= (1 << 2);
            /* trunk ID */
            if(swActionPtr->redirect.data.outIf.outInterface.trunkId
               >= BIT_7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[1] |=
                (swActionPtr->redirect.data.outIf.outInterface.trunkId
                 << 3);
            break;
        case CPSS_INTERFACE_VIDX_E:
            /* multicast IF */
            hwActionPtr[1] |= (1 << 14);
            /* VIDX */
            if(swActionPtr->redirect.data.outIf.outInterface.vidx
               >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            hwActionPtr[1] |=
                (swActionPtr->redirect.data.outIf.outInterface.vidx
                   << 2);
            break;
        case CPSS_INTERFACE_VID_E:
            /* multicast IF */
            hwActionPtr[1] |= (1 << 14);
            /* VID - use constant 0xFFF for send to VLAN */
            hwActionPtr[1] |= (0x0FFF << 2);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* Match counter */

    /* bind to CNC counter */
    hwActionPtr[0] |=
        ((BOOL2BIT_MAC(swActionPtr->matchCounter.enableMatchCount))
         << 28);

    /* counter index */
    if (swActionPtr->matchCounter.matchCounterIndex >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[0] |=
        ((swActionPtr->matchCounter.matchCounterIndex & 0x07) << 29);
    hwActionPtr[1] |=
        ((swActionPtr->matchCounter.matchCounterIndex >> 3) & 0x03);

    /* VLAN assignment */

    /* vlan precedence ignored      */
    /* nested VLAN ignored          */
    /* modify vlan command  ignored */
    /* VLAN id */
    if (swActionPtr->vlan.ingress.vlanId >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[1] |= (swActionPtr->vlan.ingress.vlanId << 19);

    /* QOS */

    /* qos profile assignment enable */
    hwActionPtr[0] |=
        ((BOOL2BIT_MAC(swActionPtr->qos.ingress.profileAssignIndex)) << 16);

    /* qos profile assignment precedence */
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.profilePrecedence)
    hwActionPtr[0] |= (hwValue << 15);

    /* qos profile */
    if (swActionPtr->qos.ingress.profileIndex >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[0] |= (swActionPtr->qos.ingress.profileIndex << 17);

    /* enable modify DSCP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.modifyDscp);
    hwActionPtr[0] |= (hwValue << 24);

    /* enable modify UP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        hwValue, swActionPtr->qos.ingress.modifyUp);
    hwActionPtr[0] |= (hwValue << 26);

    /* Ip Uc specific */

    /* arpDaIndex */
    if (swActionPtr->ipUcRoute.arpDaIndex >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    hwActionPtr[0] |= ((swActionPtr->ipUcRoute.arpDaIndex & 0x00FF) << 3);
    hwActionPtr[1] |= (((swActionPtr->ipUcRoute.arpDaIndex >> 8) & 0x03) << 17);

    /* icmpRedirectCheck */
    hwActionPtr[2] |= (BOOL2BIT_MAC(swActionPtr->ipUcRoute.icmpRedirectCheck) << 1);

    /* bypassTTLCheck */
    hwActionPtr[2] |= (BOOL2BIT_MAC(swActionPtr->ipUcRoute.bypassTTLCheck) << 2);

    /* decrementTTL */
    hwActionPtr[2] |= (BOOL2BIT_MAC(swActionPtr->ipUcRoute.decrementTTL) << 3);

    return GT_OK;
}
/**
* @internal sip5PclTcamRuleIngressActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Ingress Action from SW to HW format - Bobcat2, Caelum, Aldrin, AC3X, Bobcat3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (support SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS bits)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
static GT_STATUS sip5PclTcamRuleIngressActionSw2HwConvert
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
)
{
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_STATUS   rc;
    GT_U32  value;
    GT_U32  redirectCmd;
    GT_BOOL useIpclProfileIndexAndEnablers = GT_FALSE;
    GT_BOOL usePolicer = GT_TRUE;
    GT_U32  policerMeterEn , policerCounterEn;
    GT_U32  lpmIndex;

    /* clear */
    cpssOsMemSet(
        hwActionPtr, 0,
        (4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    /* convert packet command */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
        value, swActionPtr->pktCmd);

    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        value);

    if (swActionPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        /* in Bobcat2, Caelum, Bobcat3 the CPU code is also 'drop code' so relevant for drops too. */
        rc = prvCpssDxChNetIfCpuToDsaCode(
            swActionPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        value = dsaCpuCode;
    }
    else
    {
        /* for 'forward' this param is "don't care" , but from trap/drop it is relevant ! */
        value = 0;
    }

    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
        value);

    if(swActionPtr->mirror.mirrorToRxAnalyzerPort == GT_FALSE)
    {
        value = 0;
    }
    else
    {
        value = swActionPtr->mirror.ingressMirrorToAnalyzerIndex + 1;
    }

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_0_E,
            (value & 0x01));
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_2_1_E,
            (value >> 1));
    }
    else
    {
        SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_20_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
            value);
    }


    switch(swActionPtr->redirect.redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            redirectCmd = 0;
            useIpclProfileIndexAndEnablers = GT_TRUE;
            if (swActionPtr->redirect.data.noRedirect.modifyMacDa != GT_FALSE)
            {
                useIpclProfileIndexAndEnablers = GT_FALSE;

                value = swActionPtr->redirect.data.noRedirect.arpPtr;
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                    value);

                /*swActionPtr->redirect.data.noRedirect.modifyMacDa == GT_TRUE*/
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
                    1);
            }
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            redirectCmd = 1;

            switch (swActionPtr->redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    value = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        value);

                    value = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                        value);

                    value = swActionPtr->redirect.data.outIf.outInterface.devPort.portNum;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E,
                        value);

                    value = swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E,
                        value);

                    break;

                case  CPSS_INTERFACE_TRUNK_E:
                    value = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        value);

                    value = 1;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                        value);

                    value = swActionPtr->redirect.data.outIf.outInterface.trunkId;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E,
                        value);
                    break;
                case CPSS_INTERFACE_VIDX_E:
                case CPSS_INTERFACE_VID_E:
                    value = 1;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        value);

                    value = (swActionPtr->redirect.data.outIf.outInterface.type == CPSS_INTERFACE_VID_E) ?
                        0xFFF :/* indication to flood the vlan */
                        swActionPtr->redirect.data.outIf.outInterface.vidx;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E,
                        value);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            value = BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.vntL2Echo);
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E,
                value);

            value = BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.tunnelStart);
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E,
                value);

            if (value)/* tunnel start */
            {
                value = swActionPtr->redirect.data.outIf.tunnelPtr;
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E,
                    value);

                switch (swActionPtr->redirect.data.outIf.tunnelType)
                {
                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                        value = 0;
                        break;
                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                        value = 1;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E,
                    value);

            }
            else    /*ARP pointer*/
            {
                value = swActionPtr->redirect.data.outIf.arpPtr;
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                    value);
            }

            value = BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.modifyMacSa);
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E,
                value);

            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            redirectCmd = 2;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if(swActionPtr->redirect.data.routerLttIndex >=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
                        swActionPtr->redirect.data.routerLttIndex, PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
                }
                value = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(swActionPtr->redirect.data.routerLttIndex);
            }
            else
            {
                rc = prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex(devNum,swActionPtr->redirect.data.routerLttIndex,&lpmIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }
                value = lpmIndex - PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
            }
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                value);
            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E)){
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
                    1);
            }

            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            redirectCmd = 4;
            useIpclProfileIndexAndEnablers = GT_TRUE;
            if (swActionPtr->redirect.data.vrfId >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = swActionPtr->redirect.data.vrfId;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                value);
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
            redirectCmd = 6;
            useIpclProfileIndexAndEnablers = GT_FALSE;
            usePolicer = GT_FALSE;

            value = swActionPtr->redirect.data.modifyMacSa.arpPtr;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                value);

            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* MAC_SA_29_0 - 30 bits */
                value = 0x3FFFFFFF &
                    (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[5]
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[4] << 8)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[3] << 16)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] << 24));
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_29_0_E,
                    value);

                /* MAC_SA_47_30 - 18 bits */
                value = 0x0003FFFF &
                    ((swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2]
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[1] << 8)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[0] << 16)) >> 6);
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_30_E,
                    value);
            }
            else
            {
                /* MAC_SA_27_0 - 28 bits */
                value = 0x0FFFFFFF &
                      (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[5]
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[4] << 8)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[3] << 16)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] << 24));
                SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E,
                    value);

                /* MAC_SA_47_28 - 20 bits */
                value = 0x000FFFFF &
                     ((swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2]
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[1] << 8)
                    | (swActionPtr->redirect.data.modifyMacSa.macSa.arEther[0] << 16)) >> 4);
                SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                    SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E,
                    value);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E,
        redirectCmd);

    if(useIpclProfileIndexAndEnablers == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            value = (swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex ==
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0;

            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E,
                value);

            value = (swActionPtr->lookupConfig.pcl1OverrideConfigIndex ==
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0;

            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                value);
        }
        else
        {
            value = (swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex ==
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0;

            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                value);

            value = (swActionPtr->lookupConfig.pcl1OverrideConfigIndex ==
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E) ? 1 : 0;

            if (value != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        value = swActionPtr->lookupConfig.ipclConfigIndex;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E,
            value);
    }


    value = BOOL2BIT_MAC(swActionPtr->matchCounter.enableMatchCount);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);

    value = swActionPtr->matchCounter.matchCounterIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        value);

    if (usePolicer != GT_FALSE)
    {
        switch (swActionPtr->policer.policerEnable)
        {
            case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
                policerMeterEn   = 0;
                policerCounterEn = 0;
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
                policerMeterEn   = 1;
                policerCounterEn = 1;
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
                policerMeterEn   = 1;
                policerCounterEn = 0;
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
                policerMeterEn   = 0;
                policerCounterEn = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        value = policerMeterEn;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E,
            value);

        value = policerCounterEn;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
            value);

        value = swActionPtr->policer.policerId;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E,
            value);
    }

    value = BOOL2BIT_MAC(swActionPtr->sourceId.assignSourceId);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        value);

    value = swActionPtr->sourceId.sourceIdValue;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->actionStop);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->bypassBridge);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->bypassIngressPipe);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        value);

    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
        value, swActionPtr->vlan.ingress.precedence);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->vlan.ingress.nestedVlan);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E,
        value);

    /* modify vlan command */
    switch (swActionPtr->vlan.ingress.modifyVlan)
    {
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E:
            value = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E:
            value = 1;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E:
            value = 2;
            break;
        case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E,
        value);

    value = swActionPtr->vlan.ingress.vlanId;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->qos.ingress.profileAssignIndex);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E,
        value);

    /* qos profile assignment precedence */
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
        value, swActionPtr->qos.ingress.profilePrecedence);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        value);

    value = swActionPtr->qos.ingress.profileIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
        value);

    /* enable modify DSCP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        value, swActionPtr->qos.ingress.modifyDscp);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        value);

    /* enable modify UP */
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(
        value, swActionPtr->qos.ingress.modifyUp);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->redirect.data.outIf.modifyMacDa);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
        value);

    if(swActionPtr->redirect.redirectCmd != CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {

        CPSS_TBD_BOOKMARK /* no such field in the STC */
        /*value = 0;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
            value);*/
    }

    switch (swActionPtr->vlan.ingress.vlanId1Cmd)
    {
        case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E:
            value = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E:
            value = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E,
        value);

    switch (swActionPtr->qos.ingress.up1Cmd)
    {
        case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E:
            value = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E:
            value = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E:
            value = 2;
            break;
        case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E,
        value);

    value = swActionPtr->vlan.ingress.vlanId1;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E,
        value);

    value = swActionPtr->qos.ingress.up1;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E,
        value);

    value = swActionPtr->flowId;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->setMacToMe);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->oam.timeStampEnable);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E,
        value);

    value = swActionPtr->oam.offsetIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->oam.oamProcessEnable);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E,
        value);

    value = swActionPtr->oam.oamProfile;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->sourcePort.assignSourcePortEnable);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E,
        value);

    value = swActionPtr->sourcePort.sourcePortValue;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E,
        value);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        value = BOOL2BIT_MAC(swActionPtr->skipFdbSaLookup);
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E,
            value);

        value = BOOL2BIT_MAC(swActionPtr->triggerInterrupt);
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E,
            value);

        value = BOOL2BIT_MAC(swActionPtr->exactMatchOverPclEn);
        value ^= 1;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E,
            value);

        value = BOOL2BIT_MAC(swActionPtr->latencyMonitor.monitoringEnable);
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
            value);

        value = swActionPtr->latencyMonitor.latencyProfile;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
            value);
    }

    return GT_OK;
}

/**
* @internal sip5PclTcamRuleEgressActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Egress Action from SW to HW format - Bobcat2, Caelum, Aldrin, AC3X, Bobcat3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (support SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS bits)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
static GT_STATUS sip5PclTcamRuleEgressActionSw2HwConvert
(
    IN GT_U8                          devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
)
{
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_STATUS   rc;
    GT_U32  value,value2;
    GT_U32 policerMeterEn , policerCounterEn;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egress_mirroring_mode;

    /* Egress mirror/trap is supported from Falcon devices,
     *  devices prior to Falcon valid packet cmd supported is : FORWARD/DROP
    */

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch (swActionPtr->pktCmd)
        {
            case CPSS_PACKET_CMD_FORWARD_E:
            case CPSS_PACKET_CMD_DROP_HARD_E:
                break;/* the only 2 valid cases for EPCL */
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (swActionPtr->actionStop != GT_FALSE)
    {
        /* legacy behavior - to enforce this field to be 'not stop' (instead of 'ignore' the field)*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* clear */
    cpssOsMemSet(
        hwActionPtr, 0,
        (4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    /* convert packet command */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
        value, swActionPtr->pktCmd);

    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        value);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (swActionPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
            rc = prvCpssDxChNetIfCpuToDsaCode(
                swActionPtr->mirror.cpuCode, &dsaCpuCode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The action.pktCmd[%d] got invalid action.mirror.cpuCode[%d]",
                    swActionPtr->pktCmd ,
                    swActionPtr->mirror.cpuCode);
            }
            value = dsaCpuCode;
        }
        else
        {
            /* for 'forward' this param is "don't care" , but from trap/drop it is relevant ! */
            value = 0;
        }

        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
            value);

        if(swActionPtr->mirror.mirrorToTxAnalyzerPortEn == GT_TRUE)
        {
            value = swActionPtr->mirror.egressMirrorToAnalyzerIndex + 1;
        }
        else /* No mirroring for the ePort */
        {
            value = 0;
        }
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E,
            value);
        egress_mirroring_mode = swActionPtr->mirror.egressMirrorToAnalyzerMode;
        PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC(value, egress_mirroring_mode);
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
            value);

        switch(swActionPtr->epclPhaInfo.phaThreadIdAssignmentMode)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E:
                value  = 0;
                value2 = 0;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E:
                /******************************************/
                /* check that the PHA LIB was initialized */
                /******************************************/
                PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
                /* check the index to the table */
                PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,swActionPtr->epclPhaInfo.phaThreadId);

                value  = 1;
                value2 = swActionPtr->epclPhaInfo.phaThreadId;
                break;

            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E:
                /******************************************/
                /* check that the PHA LIB was initialized */
                /******************************************/
                PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
                value  = 1;
                value2 = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(swActionPtr->epclPhaInfo.phaThreadIdAssignmentMode);
        }

        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E,
            value);
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E,
            value2);/* use value2 !!! */

        switch(swActionPtr->epclPhaInfo.phaThreadType)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E:
                /* parameter ignored ! (added code just for 'references' for this field) */
                /* the 'Get' function will always return '0' for this case ! */
                swActionPtr->epclPhaInfo.phaThreadUnion.notNeeded =
                    swActionPtr->epclPhaInfo.phaThreadUnion.notNeeded;

                value  = 0;
                value2 = 0;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
                /******************************************/
                /* check that the PHA LIB was initialized */
                /******************************************/
                PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

                value  = 1;

                /* set the most significant bit */
                value2 = BOOL2BIT_MAC(swActionPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption) << 31;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(swActionPtr->epclPhaInfo.phaThreadType);
        }

        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E,
            value);

        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E,
            value2);/* use value2 !!! */

    }

    value = swActionPtr->qos.egress.dscp;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E,
        value);

    value = swActionPtr->qos.egress.up;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E,
        value);

    /*
    0x0 = Keep;
        Keep previous packet DSCP/EXP settings.;
    0x1 = Modify Outer;
        If packet is MPLS, modify EXP of packet's outer label to <DSCP EXP>.
        If the packet is tunneled, this refers to the tunnel header
        Else If packet is IP, modify the packet's DSCP to <DSCP EXP>.
        If the packet is tunneled, this refers to the tunnel header (BWC mode);
    0x2 = Modify Inner;
        If packet is MPLS, modify EXP of packet's outer label to <DSCP EXP>.
        If the packet is tunneled, this refers to the tunnel header (same as Mode 1)
        Else If packet is IP, modify the packet's DSCP to <DSCP EXP>.
        If the packet is tunneled, this refers to the passenger header.;
    */
    switch (swActionPtr->qos.egress.modifyDscp)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E:
            value = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E:
            value = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E,
        value);

    switch (swActionPtr->qos.egress.modifyUp)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
            value = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
            value = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->matchCounter.enableMatchCount);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);

    value = swActionPtr->matchCounter.matchCounterIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->vlan.egress.vlanId1ModifyEnable);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->qos.egress.up1ModifyEnable);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E,
        value);

    value = swActionPtr->vlan.egress.vlanId1;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E,
        value);

    value = swActionPtr->qos.egress.up1;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E,
        value);

    switch (swActionPtr->vlan.egress.vlanCmd)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
            value = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
            value = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
            value = 2;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E,
        value);

    value = swActionPtr->vlan.egress.vlanId;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E,
        value);

    switch (swActionPtr->policer.policerEnable)
    {
        case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
            policerMeterEn   = 0;
            policerCounterEn = 0;
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
            policerMeterEn   = 1;
            policerCounterEn = 1;
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
            policerMeterEn   = 1;
            policerCounterEn = 0;
            break;
        case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
            policerMeterEn   = 0;
            policerCounterEn = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = policerCounterEn;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        value);

    value = policerMeterEn;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        value);

    value = swActionPtr->policer.policerId;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
        value);

    value = swActionPtr->flowId;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->oam.oamProcessEnable);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        value);

    value = swActionPtr->oam.oamProfile;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->oam.timeStampEnable);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E,
        value);

    value = swActionPtr->oam.offsetIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E,
        value);

    value = BOOL2BIT_MAC(swActionPtr->channelTypeToOpcodeMapEnable);
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        value);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported)
    {
        value = swActionPtr->tmQueueId;
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E,
            value);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        value = BOOL2BIT_MAC(swActionPtr->terminateCutThroughMode);
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E,
            value);
        value = BOOL2BIT_MAC(swActionPtr->exactMatchOverPclEn);
        value ^= 1;
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E,
            value);
        value = BOOL2BIT_MAC(swActionPtr->latencyMonitor.monitoringEnable);
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
            value);
        value = swActionPtr->latencyMonitor.latencyProfile;
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
            value);
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPclTcamRuleActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Action from SW to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (4 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleActionSw2HwConvert
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
)
{
    GT_U32    cheetahVer;    /* version of Cheetah                  */
    GT_STATUS rc;            /* return code                         */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT   dsaCpuCode; /* cpuCode ih HW format */
    GT_BOOL   useCpuCode;    /* GT_TRUE for TRAP & mirror to CPU    */
    GT_U32    routerLttIndex = 0; /* router LTT index                    */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;
    GT_HW_DEV_NUM   hwDev = 0;
    GT_PORT_NUM     hwPort = 0;

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(swActionPtr->egressPolicy == GT_FALSE)
        {
            /*IPCL*/

            if (swActionPtr->ipUcRoute.doIpUcRoute == GT_TRUE)
            {
                /* the device not supports this format !!!  */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            return sip5PclTcamRuleIngressActionSw2HwConvert(devNum,
                        swActionPtr, hwActionPtr);
        }
        else
        {
            /* EPCL */
            return sip5PclTcamRuleEgressActionSw2HwConvert(devNum,
                        swActionPtr, hwActionPtr);
        }
    }


    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
            cheetahVer = 1;
            break;
        case CPSS_PP_FAMILY_CHEETAH2_E:
            cheetahVer = 2;
            break;
        case CPSS_PP_FAMILY_CHEETAH3_E:
            cheetahVer = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            if (swActionPtr->egressPolicy == GT_FALSE)
            {
                if (swActionPtr->ipUcRoute.doIpUcRoute == GT_FALSE)
                {
                    /* xCat and above INGRESS action */
                    return prvCpssDxChXCatPclTcamRuleIngressActionSw2HwConvert(
                        devNum, swActionPtr, hwActionPtr);
                }
                else
                {
                    /* xCat and above IP Uc routing action */
                    return prvCpssDxChXCatPclTcamRuleIpUcRoutingActionSw2HwConvert(
                        swActionPtr, hwActionPtr);
                }
            }

            /* EGRESS action is the same as for DxCh3        */
            cheetahVer = 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* validate Egress action                */
    /* Only fields that not ignored by code  */
    /* common for CH3 ingress and any Egress */
    if (swActionPtr->egressPolicy != GT_FALSE)
    {
        switch (swActionPtr->pktCmd)
        {
            case CPSS_PACKET_CMD_FORWARD_E: break;
            case CPSS_PACKET_CMD_DROP_HARD_E: break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (swActionPtr->actionStop != GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* clear */
    cpssOsMemSet(
        hwActionPtr, 0,
        (4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    useCpuCode = GT_FALSE;
    /* packet command - word0, bits 2:0 */
    switch (swActionPtr->pktCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            /* hwActionPtr[0] already 0 */
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            hwActionPtr[0] |= 1;
            useCpuCode = GT_TRUE;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            hwActionPtr[0] |= 2;
            useCpuCode = GT_TRUE;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            hwActionPtr[0] |= 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            hwActionPtr[0] |= 4;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (cheetahVer >= 3)
    {
        if ((swActionPtr->actionStop != GT_FALSE) &&
            (swActionPtr->ipUcRoute.doIpUcRoute == GT_FALSE))
        {
            /* prevent the next lookups */
            hwActionPtr[3] |= (1 << 1);
        }
    }

    /* mirror bit 11 - to-analyzer-port, bits 10:3 CpuCode */

    if (swActionPtr->mirror.mirrorToRxAnalyzerPort != GT_FALSE)
    {
        hwActionPtr[0] |= (1 << 11);
    }

    dsaCpuCode = PRV_CPSS_DXCH_NET_DSA_TAG_REGULAR_FORWARD_E;

    if (useCpuCode != GT_FALSE)
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(
            swActionPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(dsaCpuCode >= BIT_8)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    hwActionPtr[0] |= ((dsaCpuCode & 0xFF) << 3);

    /* match counter bit 25 - enable, bits 30:26 - index */
    if (swActionPtr->matchCounter.enableMatchCount == GT_TRUE)
    {
        if (swActionPtr->egressPolicy == GT_FALSE)
        {
            hwActionPtr[0] |= (1 << 25);
            hwActionPtr[0] |=
                ((swActionPtr->matchCounter.matchCounterIndex & 0x1F) << 26);
        }
        else
        {
            if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
                devNum, PRV_CPSS_DXCH2_PCL_EGRESS_ACTION_CNTR_VLAN_WRONG_BITS_POS_WA_E))
            {
                hwActionPtr[0] |= (1 << 30);
                hwActionPtr[0] |=
                    ((swActionPtr->matchCounter.matchCounterIndex & 0x1F) << 25);
            }
            else
            {
                hwActionPtr[0] |= (1 << 25);
                hwActionPtr[0] |=
                    ((swActionPtr->matchCounter.matchCounterIndex & 0x1F) << 26);
            }
        }

        if (cheetahVer >= 3)
        {
            if(swActionPtr->matchCounter.matchCounterIndex >= BIT_14)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

            /* counter index 5 */
            hwActionPtr[0] |=
                (((swActionPtr->matchCounter.matchCounterIndex >> 5) & 0x1) << 31);

            /* counter index 13:6 */
            hwActionPtr[3] |=
                (((swActionPtr->matchCounter.matchCounterIndex >> 6) & 0xFF) << 2);
        }
        else
        {
            if(swActionPtr->matchCounter.matchCounterIndex >= BIT_5)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }


    /* QOS */

    /* common for ingress and egress */

    if (swActionPtr->egressPolicy == GT_FALSE)
    {
        /* ingress */

        /* modifyDscp bits 22:21 */
        switch (swActionPtr->qos.ingress.modifyDscp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
                /* hwActionPtr[0] already 0 */
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                hwActionPtr[0] |= (2 << 21);
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                hwActionPtr[0] |= (1 << 21);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* modifyUp bits 24:23 */
        switch (swActionPtr->qos.ingress.modifyUp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
                /* hwActionPtr[0] already 0 */
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                hwActionPtr[0] |= (2 << 23);
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                hwActionPtr[0] |= (1 << 23);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (cheetahVer == 1)
        {
            /* QOS profile bits 20:14 */
            if(swActionPtr->qos.ingress.profileIndex >= PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        else
        {
            /* QOS profile bits 20:14 */
            if(swActionPtr->qos.ingress.profileIndex >= BIT_7)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwActionPtr[0] |=
            ((swActionPtr->qos.ingress.profileIndex & 0x7F) << 14);

        /* QOS profile marking enable bit 13 */
        if (swActionPtr->qos.ingress.profileAssignIndex == GT_TRUE)
        {
            hwActionPtr[0] |= (1 << 13);
        }

        /* QOS precedence bit 12 */
        switch (swActionPtr->qos.ingress.profilePrecedence)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
                /* hwActionPtr[0] already 0 */
                break;
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
                hwActionPtr[0] |= (1 << 12);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* egress */
        if (cheetahVer == 1)
        {
            /* the Cheetah not supports Egress Policy*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* modifyDscp bits 22:21 */
        switch (swActionPtr->qos.egress.modifyDscp)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                /* 0 - HW value */
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E:
                hwActionPtr[0] |= (1 << 21);
                break;
            /* CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E not supported */
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* modifyUp bits 24:23 */
        switch (swActionPtr->qos.egress.modifyUp)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                /* hwActionPtr[0] already 0 */
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                hwActionPtr[0] |= (1 << 23);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* UP bits 20:18 */
        if(swActionPtr->qos.egress.up >= BIT_3)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        hwActionPtr[0] |= ((swActionPtr->qos.egress.up & 0x07) << 18);

        /* DSCP bits 17:12 */
        if(swActionPtr->qos.egress.dscp >= BIT_6)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        hwActionPtr[0] |= ((swActionPtr->qos.egress.dscp & 0x3F) << 12);
    }

    /* redirect */

    /* command - word1, bits 1:0 */
    switch (swActionPtr->redirect.redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            /* hwActionPtr[0] already 0 */
            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:

            /* redirection command */
            hwActionPtr[1] |= 1;

            /* redirect interface */
            switch (swActionPtr->redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                    hwDev = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                    hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                    /* device */
                    if(hwDev >= BIT_5)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    hwActionPtr[1] |=
                        ((hwDev & 0x1F) << 9);
                    /* port */
                    if(hwPort >= BIT_6)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    hwActionPtr[1] |=
                        ((hwPort & 0x3F) << 3);
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    /* TRUNK IF */
                    hwActionPtr[1] |= (1 << 2);
                    /* trunk ID */
                    if(swActionPtr->redirect.data.outIf.
                          outInterface.trunkId >= BIT_7)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    hwActionPtr[1] |=
                        ((swActionPtr->redirect.data.outIf.
                          outInterface.trunkId & 0x7F) << 3);
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    /* multicast IF */
                    hwActionPtr[1] |= (1 << 14);
                    /* VIDX */
                    if(swActionPtr->redirect.data.outIf.outInterface.vidx >= BIT_12)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    hwActionPtr[1] |=
                        ((swActionPtr->redirect.data.outIf.outInterface.vidx
                          & 0x0FFF) << 2);
                    break;
                case CPSS_INTERFACE_VID_E:
                    /* multicast IF */
                    hwActionPtr[1] |= (1 << 14);
                    /* VID - use constant 0xFFF for send to VLAN */
                    hwActionPtr[1] |= (0x0FFF << 2);
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* tunnel start */
            if (swActionPtr->redirect.data.outIf.tunnelStart == GT_TRUE)
            {
                if (cheetahVer == 1)
                {
                    /* the Cheetah not supports tunnel */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                /* tunnel start */
                hwActionPtr[2] |= (1 << 10);

                /* tunnel PTR */
                hwActionPtr[2] |= ((swActionPtr->redirect.data.outIf.tunnelPtr
                                    & 0x03FF) << 11);

                if (cheetahVer >= 3)
                {
                    /* tunnel PTR bits 12:10 */
                    if(swActionPtr->redirect.data.outIf.tunnelPtr >= BIT_13)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    hwActionPtr[3] |=
                        (((swActionPtr->redirect.data.outIf.tunnelPtr
                                       >> 10) & 7) << 14);
                    /* tunnel type */
                    switch (swActionPtr->redirect.data.outIf.tunnelType)
                    {
                        case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                            /* the data already contains 0 */
                            break;
                        case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                            hwActionPtr[2] |= (1 << 21);
                            break;
                        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    if(swActionPtr->redirect.data.outIf.tunnelPtr >= BIT_10)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }

            }

            /* vntL2Echo */
            if (swActionPtr->redirect.data.outIf.vntL2Echo == GT_TRUE)
            {
                if (cheetahVer == 1)
                {
                    /* the Cheetah not supports VNT */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                /* vntL2Echo */
                hwActionPtr[2] |= (1 << 9);

            }

            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:

            if (cheetahVer == 1)
            {
                /* the Cheetah not supports redirect to Next Hop */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* redirection command */
            hwActionPtr[1] |= 2;

            /* routerLttIndex */
            if (cheetahVer >=  3)
            {
                /* routerLttIndex bits 14:13 */
                if(swActionPtr->redirect.data.routerLttIndex >= BIT_15)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            else
            {
                if(swActionPtr->redirect.data.routerLttIndex >= BIT_13)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            /* FEr#2018: Limited number of Policy-based routes */
            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH3_LIMITED_NUMBER_OF_POLICY_BASED_ROUTES_WA_E))
            {
                switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                /* DxCh3 devices support index 0,1,2,3...(max IP TCAM row) only */
                case CPSS_PP_FAMILY_CHEETAH3_E:
                        if (swActionPtr->redirect.data.routerLttIndex
                            >= fineTuningPtr->tableSize.router)
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        break;
                    default:
                        /* Do nothing */
                        break;
                }
            }
            routerLttIndex = swActionPtr->redirect.data.routerLttIndex;

            /* routerLttIndex */
            hwActionPtr[1] |= ((routerLttIndex & 0x1FFF) << 2);

            if (cheetahVer >=  3)
            {
                /* routerLttIndex bits 14:13 */
                hwActionPtr[3] |= (((routerLttIndex >> 13)& 3) << 24);
            }

            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            if (cheetahVer < 3)
            {
                /* the CH and CH2 not support redirect to virtual router */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* redirection command - 4 */
            /* bits 33:32 assumed to be 0 */
            /* bit 119 is redirection command bit 2 */
            hwActionPtr[3] |= (1 << 23);

            /* routerLttIndex */
            if(swActionPtr->redirect.data.vrfId >= BIT_12)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            hwActionPtr[1] |=
                ((swActionPtr->redirect.data.vrfId & 0x0FFF) << 2);

            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* policer Ch-Ch3 ingress only */
    if ((swActionPtr->policer.policerEnable != CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E)  &&
       (swActionPtr->egressPolicy == GT_FALSE)   &&
       (swActionPtr->ipUcRoute.doIpUcRoute == GT_FALSE) )
    {
        /* policer enable */
        hwActionPtr[2] |= 1;

        /* policer index */
        hwActionPtr[2] |= ((swActionPtr->policer.policerId & 0xFF) << 1);

        if (cheetahVer >= 3)
        {
            /* policer index bits 11:8 */
            if(swActionPtr->policer.policerId >= BIT_12)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            hwActionPtr[3] |= (((swActionPtr->policer.policerId >> 8) & 0x0F) << 10);
        }
        else
        {
            if(swActionPtr->policer.policerId >= BIT_8)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

    }

    /* Egress Policer xCat and above devices     */
    /* Ingress action for xCat in other function */
    if ((swActionPtr->egressPolicy != GT_FALSE)
        && (cheetahVer >= 4))
    {

        if (swActionPtr->policer.policerId >= BIT_12)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* Policer Index[7..0] bits 72..65 */
        hwActionPtr[2] |= ((swActionPtr->policer.policerId & 0x00FF) << 1);

        /* Policer Index[11..8] bits 109..106 */
        hwActionPtr[3] |= ((swActionPtr->policer.policerId & 0x0F00) << 2);

        switch (swActionPtr->policer.policerEnable)
        {
            case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
                /* bit 63  Bind To Policer Counter assumed 0 */
                /* bit 64  Bind To Policer Meter assumed 0   */
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
                /* bit 63  Bind To Policer Counter */
                hwActionPtr[1] |= (1 << 31);
                /* bit 64  Bind To Policer Meter   */
                hwActionPtr[2] |= 1;
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
                /* bit 64  Bind To Policer Meter */
                hwActionPtr[2] |= 1;
                break;
            case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
                /* bit 63  Bind To Policer Counter */
                hwActionPtr[1] |= (1 << 31);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* set bit 11 to be always 1 */
        hwActionPtr[0] |= (1 << 11);
    }

    /* vlan and OAM */

    /* vlan command word1 bits 18:17 */
    if (swActionPtr->egressPolicy == GT_FALSE)
    {
        /* Ingress Policy */
        if(swActionPtr->vlan.ingress.vlanId >= BIT_12)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        switch (swActionPtr->vlan.ingress.modifyVlan)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E:
                /* hwActionPtr[1] already 0 */
                break;
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E:
                hwActionPtr[1] |= (2 << 17);
                break;
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E:
                hwActionPtr[1] |= (1 << 17);
                break;
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E:
                hwActionPtr[1] |= (3 << 17);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* VLAN Id */
        hwActionPtr[1] |= ((swActionPtr->vlan.ingress.vlanId & 0x0FFF) << 19);

        /* nested VLAN */
        if (swActionPtr->vlan.ingress.nestedVlan == GT_TRUE)
        {
            hwActionPtr[1] |= (1 << 16);
        }


        /* VLAN command precedence */
        switch (swActionPtr->vlan.ingress.precedence)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
                /* hwActionPtr[0] already 0 */
                break;
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
                hwActionPtr[1] |= (1 << 15);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* Egress Policy */
        if(swActionPtr->vlan.egress.vlanId >= BIT_12)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
            devNum, PRV_CPSS_DXCH2_PCL_EGRESS_ACTION_CNTR_VLAN_WRONG_BITS_POS_WA_E))
        {
            switch (swActionPtr->vlan.egress.vlanCmd)
            {
                case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                    /* hwActionPtr[1] already 0 */
                    break;
                case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                    hwActionPtr[1] |= (1 << 18);
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* VLAN Id */
            hwActionPtr[1] |= ((swActionPtr->vlan.egress.vlanId & 0x07FF) << 20);
            hwActionPtr[2] |= ((swActionPtr->vlan.egress.vlanId >> 11) & 0x01);
        }
        else
        {
            switch (swActionPtr->vlan.egress.vlanCmd)
            {
                case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                    /* hwActionPtr[1] already 0 */
                    break;
                case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                    hwActionPtr[1] |= (1 << 17);
                    break;
                case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                    hwActionPtr[1] |= (2 << 17);
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* VLAN Id */
            hwActionPtr[1] |= ((swActionPtr->vlan.egress.vlanId & 0x0FFF) << 19);
        }
    }

    if ((cheetahVer >= 3)  &&
       (swActionPtr->egressPolicy == GT_FALSE)  &&
       (swActionPtr->ipUcRoute.doIpUcRoute == GT_FALSE))
    {
        /* MODIFY SOURCE ID */
        if (swActionPtr->sourceId.assignSourceId != GT_FALSE)
        {
            /* modify source id enable */
            hwActionPtr[3] |= (1 << 17);

            if(swActionPtr->sourceId.sourceIdValue >= BIT_5)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            hwActionPtr[3] |=
                ((swActionPtr->sourceId.sourceIdValue & 0x1F) << 18);
        }
    }

    /* IP Unicast routing for DxCh and xCat only (not for DxCh2 or DxCh3) */
    if (swActionPtr->ipUcRoute.doIpUcRoute == GT_TRUE)
    {
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH2_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E))
        {
            /* The DxCh2 and DxCh3 devices not support this feature */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* check required parameters from other parts of the action */
        /* assumed they contain the needed relevant data            */

        if (swActionPtr->redirect.redirectCmd
            != CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            /* the redirect command should be Redirect to Out Interface */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (swActionPtr->policer.policerEnable != CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E)
        {
            /* Policer must be disabled */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if ((swActionPtr->egressPolicy == GT_TRUE)
            || (swActionPtr->vlan.ingress.modifyVlan ==
                CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E)
            || (swActionPtr->vlan.ingress.nestedVlan == GT_TRUE))
        {
            /* VLAN modification should be for all types of packets
               Nested VLAN must be disabled  */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* update redirection command to be 3 - routing */
        hwActionPtr[1] &= (~ 3);
        hwActionPtr[1] |= 3;

        /* ARP DA index */
        if (swActionPtr->ipUcRoute.arpDaIndex >= BIT_10)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* 8 low bits */
        hwActionPtr[0] &= (~ (0x00FF << 3));
        hwActionPtr[0] |=
            (swActionPtr->ipUcRoute.arpDaIndex & 0x00FF) << 3;
        /* 2 high bits */
        hwActionPtr[1] &= (~ (0x03 << 17));
        hwActionPtr[1] |=
            ((swActionPtr->ipUcRoute.arpDaIndex >> 8) & 0x03) << 17;

        /* reset 15 - 16 bits of word 1 */
        hwActionPtr[1] &= (~ (0x03 << 15));

        /* reset word 2 */
        hwActionPtr[2] = 0;

        /* enable decrement TTL */
        if (swActionPtr->ipUcRoute.decrementTTL == GT_TRUE)
        {
            hwActionPtr[2] |= (1 << 3);
        }

        /* Enable TTL and Options check bypass. */
        if (swActionPtr->ipUcRoute.bypassTTLCheck == GT_TRUE)
        {
            hwActionPtr[2] |= (1 << 2);
        }

        /* ICMP Redirect Check Enable */
        if (swActionPtr->ipUcRoute.icmpRedirectCheck == GT_TRUE)
        {
            hwActionPtr[2] |= (1 << 1);
        }
    }

    return GT_OK;
}

/**
* @internal ipclAction2ReducedMask function
* @endinternal
*
* @brief  Converts a given IPCL action from logic format to
*         exact match reduced mask.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to pcl action in
*                                       logic format
* @param[in] pclExpandedActionOriginPtr - (pointer to) Whether
*                   to use the action attributes from the Exact
*                   Match rule action or from the profile
* @param[out] exactMatchReducedHwMaskArray  - pcl reduced action
*       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ipclAction2ReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *pclExpandedActionOriginPtr,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray
)
{
    GT_U32      value;
    GT_STATUS   rc=GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclExpandedActionOriginPtr);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchReducedHwMaskArray);

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_TRUE)
    {
        value = 0xFF; /*bits 0-7*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPktCmd==GT_TRUE)
    {
        value =0x7; /*bits 8-10*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_COMMAND_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_TRUE)
    {
        value = 0x7; /*bits 11-13 redirect command*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E,
                                   value);
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_TRUE)
    {
        value = 1; /*bits 16 useVidx*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                                   value);

        value = 1; /*bits 17 - is trunk*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                                   value);

        value = 0xFFFF; /*bits 17-32 eVIDX*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_VIDX_E,
                                   value);

        value = 0xFFF; /*bits 18-29 trunkID*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E,
                                   value);

        value = 0x3FFF; /*bits 18-31 Target ePort*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E,
                                   value);

        value = 0x3FF; /*bits 32-41 target device*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMacSa==GT_TRUE)
    {
        value = 0xFFFFFFF; /*bits 16-43 macSA[27:0]*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP6_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E,
                               value);
        value = 0xFFFFF; /*bits 62-81 MacSA[47:28]*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_TRUE)
    {
        value = 0x3FFFF; /*bits 33-50 IP Next Hop entry index*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_TRUE)
    {
        value = 1; /*bits 42 VNTL2Echo*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_TRUE)
    {
        value = 1; /*bits 43 tunnelStart*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E,
                                   value);

    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex==GT_TRUE)
    {
        value =0xFFFF; /*bits 44:59 TunnelIndex*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionArpPtr==GT_TRUE)
    {
        value = 0x3FFFF; /*bits 44-61 Arp index*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_TRUE)
    {
        value = 0xFFF; /*bits 52-63 VRF-ID */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_TRUE)
    {
        value = 1; /*bit 60 TS passenger Type*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacDA==GT_TRUE)
    {
        value = 1; /*bits 86 ModifyMacDA*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacSA==GT_TRUE)
    {
        value = 1; /*bits 87 ModifyMacSA*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                   SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E,
                                   value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_TRUE)
    {
        value = 1; /*bits 14*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_TRUE)
    {
        value = 1; /*bits 15*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL1OverrideConfigIndex==GT_TRUE)
    {
        value = 1; /*bits 16*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL2OverrideConfigIndex==GT_TRUE)
    {
         value = 1; /*bits 17*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionIPCLConfigurationIndex==GT_TRUE)
    {
        value = 0x1FFF; /*bits 18-30*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerEnable==GT_TRUE)
    {
        value = 1; /*bits 64 bindToPolicerMeter*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E,
                                       value);

        value = 1; /*bits 65 bindToPolicerCounter*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_TRUE)
    {
        value = 0x3FFF; /*bits 66-79 policerIndex*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu==GT_TRUE)
    {
        value = 1; /*bits  82 -Enable Mirror*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirror==GT_TRUE)
    {
        value = 0x7;/*bits  83-85 - Mirror To Analyzer Port*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassBridge==GT_TRUE)
    {
        value = 1; /*bits 88*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassIngressPipe==GT_TRUE)
    {
        value = 1; /*bits 89*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_TRUE)
    {
        value = 1; /*bits 90 Enable Nested*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP6_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlanPrecedence==GT_TRUE)
    {
        value = 1; /*bits 120 VLANPrecedence*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0Command==GT_TRUE)
    {
        value = 0x3; /*bits 121-122 VLANCommand*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0==GT_TRUE)
    {
        value = 0x1FFF; /*bits 123-135 VID0*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP6_IPCL_ACTION_TABLE_FIELDS_VID0_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1Command==GT_TRUE)
    {
        value = 0x3; /*bits 152-153 vlan1_cmd*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1==GT_TRUE)
    {
        value = 0xFFF; /*bits 156-167 vid1  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_VID1_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceIdEnable==GT_TRUE)
    {
        value = 1; /*bits 91 -Source-ID Assignment Enable*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceId==GT_TRUE)
    {
        value = 0xFFF; /*bits 92-103 Source-ID*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_SST_ID_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterEnable==GT_TRUE)
    {
        value = 1; /*bits 105 - Bind To CNC Counter*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterIndex==GT_TRUE)
    {
        value = 0x3FFF; /*bits 106-119 - CNC Counter Index */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosProfileMakingEnable==GT_TRUE)
    {
        value = 1; /*bits 136 Qos_Profile_Marking_Enable */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_TRUE)
    {
        value = 1; /*bits 137 QoSPrecedence  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSProfile==GT_TRUE)
    {
        value = 0x3FF; /*bits 138-147 QoS_Profile  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSModifyDSCP==GT_TRUE)
    {
        value = 0x3; /*bits 148-149 ModifyDSCP  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
                                       value);

    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp1==GT_TRUE)
    {
        value = 0x3; /*bits 150-151 ModifyUP  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1Command==GT_TRUE)
    {
        value = 0x3; /*bits 154-155 up1_cmd  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1==GT_TRUE)
    {

        value = 0x7; /*bits 168-170 up1  */
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamTimestampEnable==GT_TRUE)
    {
        value = 1;/*bits 192 - Timestamp Enable*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_TRUE)
    {
        value = 0x7F;/*bits 193-199 offset_index*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProcessingEnable==GT_TRUE)
    {
        value = 1;/*bits 200 oam_processing_enable*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_TRUE)
    {
        value = 1;/*bits 201 OAM Profile*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_TRUE)
    {
        value = 0x1FFF; /*bits 202-214*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePortEnable==GT_TRUE)
    {
        value = 1; /*bits 216 - Assign Source ePort Enabl*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePort==GT_TRUE)
    {
        value = 0x3FFF; /*bits 217-230 Source ePort*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitor==GT_TRUE)
    {
        value = 0x1FF; /*bits  231-239 -Latency Profile*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitorEnable==GT_TRUE)
    {
        value = 1; /*bits 240 Latency Monitoring En*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookup==GT_TRUE)
    {
        value = 1; /*bits 241*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTriggerInterrupt==GT_TRUE)
    {
        value = 1; /*bits 242*/
        SIP6_IPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E,
                                       value);
    }

    return rc;
}

/**
* @internal epclAction2ReducedMask function
* @endinternal
*
* @brief  Converts a given EPCL action from logic format to
*         exact match reduced mask.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to pcl action in
*                                       logic format
* @param[out] exactMatchReducedHwMaskArray  - pcl reduced action
*       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS epclAction2ReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *pclExpandedActionOriginPtr,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray
)
{
    GT_U32      value;
    GT_STATUS   rc=GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclExpandedActionOriginPtr);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchReducedHwMaskArray);

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/

    if (pclExpandedActionOriginPtr->overwriteExpandedActionPktCmd==GT_TRUE)
    {
        value =0x7; /*bits 0-2*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionDscpExp==GT_TRUE)
    {
        value = 0x3F; /*bits 3-8*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp0==GT_TRUE)
    {
        value = 0x7; /*bits 9-11*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSModifyDSCP==GT_TRUE)
    {
        value = 0x3; /*bits 12-13*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp0==GT_TRUE)
    {
        value = 0x3; /*bits 14-15*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E,
                                       value);
    }
      if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp1==GT_TRUE)
    {
        value = 1; /*bits 32*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1==GT_TRUE)
    {
        value = 0x7; /*bits 45-47*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterEnable==GT_TRUE)
    {
        value = 1; /*bits 16 - Bind To CNC Counter*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterIndex==GT_TRUE)
    {
        value = 0x3FFF; /*bits 17-30 - CNC Counter Index */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0Command==GT_TRUE)
    {
        value = 0x3; /*bits 48-49*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0==GT_TRUE)
    {
        value = 0xFFF; /*bits 50-61*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1Command==GT_TRUE)
    {
        value = 1; /*bits 31*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E,
                               value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1==GT_TRUE)
    {
        value = 0xFFF; /*bits 33-44*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                               SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E,
                               value);

    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerEnable==GT_TRUE)
    {
        value = 1; /*bits 62*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
                                       value);
        value = 1; /*bits 63*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_TRUE)
    {
        value = 0x3FFF; /*bits 64-77*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
                                       value);
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_TRUE)
    {
        value = 0x1FFF; /*bits 78-90*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProcessingEnable==GT_TRUE)
    {
        value = 1;/*bits 91*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_TRUE)
    {
        value = 1;/*bits 92*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamTimestampEnable==GT_TRUE)
    {
        value = 1;/*bits 93*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_TRUE)
    {

        value = 0x7F;/*bits 94-100*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable==GT_TRUE)
    {
        value = 1;/*bits 101*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
                                       value);
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadataAssignEnable==GT_TRUE)
    {
        value = 1; /*bits  123 */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadata==GT_TRUE)
    {
        value = 0xFFFFFFFF; /*bits 124-155 */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumberAssignEnable==GT_TRUE)
    {
        value = 1; /*bits  156 */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumber==GT_TRUE)
    {
        value = 0xFF; /*bits 157-164 */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E,
                                       value);
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorMode==GT_TRUE)
    {
        value = 0x3; /*bits  165-166 */
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirror==GT_TRUE)
    {
        value = 0x7; /*bits 167-169*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_TRUE)
    {
        value = 0xFF; /*bits 170-177*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitor==GT_TRUE)
    {
        value = 0x1FF; /*bits  178-186 -Latency Profile*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitorEnable==GT_TRUE)
    {
        value = 1; /*bits 187 Latency Monitoring En*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
                                       value);
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionCutThroughTerminateId==GT_TRUE)
    {
        value = 1; /*bits  188*/
        SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                       SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E,
                                       value);
    }

    /* SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E */

    return rc;
}

/**
* @internal prvCpssDxChPclActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                     - device number
* @param[in] logicFormatPtr             - points to pcl action in logic
*                                       format
* @param[in] pclExpandedActionOriginPtr - points to pcl use
*                                       Expander flags
* @param[out] hwFormatArray            - pcl action in hardware
*                                        format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - pcl reduced action
*                                       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPclActionLogic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *pclExpandedActionOriginPtr,
    OUT GT_U32                                               *hwFormatArray,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclExpandedActionOriginPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchReducedHwMaskArray);

    cpssOsMemSet(hwFormatArray, 0, sizeof(*hwFormatArray));
    cpssOsMemSet(exactMatchReducedHwMaskArray, 0, sizeof(*exactMatchReducedHwMaskArray));

    if(logicFormatPtr->egressPolicy == GT_FALSE)
    {
        /*IPCL*/

        if (logicFormatPtr->ipUcRoute.doIpUcRoute == GT_TRUE)
        {
            /* the device not supports this format !!!  */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = sip5PclTcamRuleIngressActionSw2HwConvert(devNum,
                                                        logicFormatPtr, hwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = ipclAction2ReducedMask(devNum, logicFormatPtr, pclExpandedActionOriginPtr, exactMatchReducedHwMaskArray);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* EPCL */
        rc = sip5PclTcamRuleEgressActionSw2HwConvert(devNum,logicFormatPtr, hwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = epclAction2ReducedMask(devNum, logicFormatPtr, pclExpandedActionOriginPtr, exactMatchReducedHwMaskArray);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChXCatPclTcamRuleIngressActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Ingress Action from HW to SW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] hwActionPtr              - action in HW format (4 words)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
static GT_STATUS prvCpssDxChXCatPclTcamRuleIngressActionHw2SwConvert
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
)
{
    GT_U32    hwValue;   /* HW value    */
    GT_STATUS rc;        /* return code */
    GT_U32    tempRouterLttPtr = 0;
    GT_U32    maxMatchCounterIndex; /* max Match Counter Index */

    /* default settings - forward only */
    cpssOsMemSet(swActionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* pktCmd */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
        swActionPtr->pktCmd,(hwActionPtr[0] & 0x07));

    swActionPtr->egressPolicy = GT_FALSE;
    swActionPtr->ipUcRoute.doIpUcRoute = GT_FALSE;

    swActionPtr->actionStop        = BIT2BOOL_MAC(((hwActionPtr[2] >> 15) & 1));
    swActionPtr->bypassBridge      = BIT2BOOL_MAC(((hwActionPtr[2] >> 16) & 1));
    swActionPtr->bypassIngressPipe = BIT2BOOL_MAC(((hwActionPtr[2] >> 17) & 1));

    /* Mirror */
    swActionPtr->mirror.mirrorToRxAnalyzerPort =
        BIT2BOOL_MAC(((hwActionPtr[0] >> 11) & 1));

    swActionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu =
        BIT2BOOL_MAC(((hwActionPtr[3] >> 17) & 1));

    /* CPU code */

    /* default */
    swActionPtr->mirror.cpuCode = CPSS_NET_CLASS_KEY_TRAP_E;

    if ((swActionPtr->pktCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
        || (swActionPtr->pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E))
    {
        hwValue = ((hwActionPtr[0] >> 3) & 0x00FF);

        rc = prvCpssDxChNetIfDsaToCpuCode(
            (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)hwValue,
            &swActionPtr->mirror.cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Match counting in CNC */

    swActionPtr->matchCounter.enableMatchCount =
        BIT2BOOL_MAC(((hwActionPtr[1] >> 12) & 1));

     maxMatchCounterIndex =
        (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_FALSE)
        ? BIT_14 : BIT_13;
    swActionPtr->matchCounter.matchCounterIndex =
        ((hwActionPtr[1] >> 13) & (maxMatchCounterIndex - 1));

    /* QOS */

    /* modifyDscp */
    hwValue = ((hwActionPtr[3] >> 13) & 3);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyDscp, hwValue);

    /* modifyUp */
    hwValue = ((hwActionPtr[3] >> 15) & 3);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyUp, hwValue);

    /* profileIndex */
    swActionPtr->qos.ingress.profileIndex =
        ((hwActionPtr[3] >> 6) & 0x7F);

    /* profileAssignIndex */
    swActionPtr->qos.ingress.profileAssignIndex =
        BIT2BOOL_MAC(((hwActionPtr[3] >> 4) & 1));

    /* profilePrecedence */
    hwValue = ((hwActionPtr[3] >> 5) & 1);
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(
        swActionPtr->qos.ingress.profilePrecedence, hwValue);

    /* redirect */

    switch ((hwActionPtr[0] >> 12) & 7)
    {
        case 0:
            swActionPtr->redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            break;
        case 1:
            swActionPtr->redirect.redirectCmd
                = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            /*bit 27 - IsVidx*/
            if ((hwActionPtr[0] >> 27) & 1)
            {
                swActionPtr->redirect.data.outIf.outInterface.vidx =
                    (GT_U16)((hwActionPtr[0] >> 15) & 0x0FFF);

                swActionPtr->redirect.data.outIf.outInterface.type =
                    (swActionPtr->redirect.data.outIf.outInterface.vidx
                     == 0xFFF)
                    /* the packet's VID flooding */
                    ? CPSS_INTERFACE_VID_E
                    /* redirect to specified MA group */
                    : CPSS_INTERFACE_VIDX_E;
            }
            /* bit 15 isTrunk*/
            else if ((hwActionPtr[0] >> 15) & 1)
            {
                swActionPtr->redirect.data.outIf.outInterface.type =
                    CPSS_INTERFACE_TRUNK_E;
                swActionPtr->redirect.data.outIf.outInterface.trunkId =
                    (GT_TRUNK_ID)((hwActionPtr[0] >> 16) & 0x007F);
            }
            /* port*/
            else
            {
                swActionPtr->redirect.data.outIf.outInterface.type =
                    CPSS_INTERFACE_PORT_E;

                swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
                    ((hwActionPtr[0] >> 22) & 0x001F);
                swActionPtr->redirect.data.outIf.outInterface.devPort.portNum =
                    ((hwActionPtr[0] >> 16) & 0x003F);
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(swActionPtr->redirect.data.outIf.outInterface));
            }

            swActionPtr->redirect.data.outIf.tunnelStart =
                BIT2BOOL_MAC((hwActionPtr[0] >> 28) & 1);
            swActionPtr->redirect.data.outIf.tunnelPtr =
                ((hwActionPtr[0] >> 29) & 7)
                | ((hwActionPtr[1] << 3) & 0x1FF8);
            swActionPtr->redirect.data.outIf.tunnelType =
                ((hwActionPtr[1] >> 10) & 1)
                ? CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E
                : CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
            swActionPtr->redirect.data.outIf.vntL2Echo =
                BIT2BOOL_MAC((hwActionPtr[1] >> 11) & 1);

            /* parse Lion and above fields */
            if ((PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)) &&
                (swActionPtr->redirect.data.outIf.tunnelStart == GT_FALSE))
            {
                swActionPtr->redirect.data.outIf.arpPtr =
                    ((hwActionPtr[0] >> 29) & 7)
                | ((hwActionPtr[1] << 3) & 0x3FF8);
            }
            break;
        case 2:
            swActionPtr->redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E;

            /* LTT entry is row based however in this field the LTT is treated as column based.
               Bits [11:0] indicate row while bits [14:13] indicate column, bit 12 is not used.
               The Formula for translating the LTT entry to column based is as follow:
               [11:0] << 2 + [14:13]   (Note: bit 12 not used). */
            tempRouterLttPtr = ((hwActionPtr[0] >> 15) & 0x7FFF);
            swActionPtr->redirect.data.routerLttIndex =
                (((tempRouterLttPtr & 0xFFF) << 2) |
                 ((tempRouterLttPtr & 0x6000) >> 13));

            break;
        case 4:
            swActionPtr->redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;

            swActionPtr->redirect.data.vrfId =
                ((hwActionPtr[0] >> 15) & 0x0FFF);
            break;
        case 5:
            if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            swActionPtr->redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E;
            /* sourceMeshIdSetEnable - bit[30] */
            swActionPtr->redirect.data.logicalSourceInterface.sourceMeshIdSetEnable
                = BIT2BOOL_MAC((hwActionPtr[0] >> 30) & 1);
            /* sourceMeshId - bits[29:28] */
            swActionPtr->redirect.data.logicalSourceInterface.sourceMeshId
                = ((hwActionPtr[0] >> 28) & 3);
            /* userTagAcEnable - bit[27] */
            swActionPtr->redirect.data.logicalSourceInterface.userTagAcEnable
                = BIT2BOOL_MAC((hwActionPtr[0] >> 27) & 1);
            /* source interface is port */
            swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.type
                = CPSS_INTERFACE_PORT_E;
            /* logical source devNum - bits[26:22]  */
            swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum
                = (GT_U8)((hwActionPtr[0] >> 22) & 0x1F);
            /* logical source portNum - bits[21:16]  */
            swActionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum
                = (GT_U8)((hwActionPtr[0] >> 16) & 0x3F);
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Lookup config */
    if (swActionPtr->redirect.redirectCmd
            != CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        swActionPtr->lookupConfig.ipclConfigIndex =
            ((hwActionPtr[0] >> 31) & 1) | ((hwActionPtr[1] & 0x0FFF) << 1);
    }

    swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex =
        ((hwActionPtr[2] >> 19) & 1)
        ? CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E
        : CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    swActionPtr->lookupConfig.pcl1OverrideConfigIndex =
        ((hwActionPtr[2] >> 18) & 1)
        ? CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E
        : CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    /* Policer */

    /* bind to meter (bit 59) and to counter (bit 60) */
    switch (((hwActionPtr[1] >> 26) & 2) | ((hwActionPtr[1] >> 28) & 1))
    {
        case 0:
            swActionPtr->policer.policerEnable =
                CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
            break;
        case 1:
            swActionPtr->policer.policerEnable =
                CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            break;
        case 2:
            swActionPtr->policer.policerEnable =
                CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
            break;
        case 3:
            swActionPtr->policer.policerEnable =
                CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            break;
        /* cannot occur */
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* policerId */
    swActionPtr->policer.policerId =
        ((hwActionPtr[1] >> 29) & 7)
        | ((hwActionPtr[2] << 3) & 0x0FF8);

    /* VLAN */

    /* modifyVlan */
    hwValue = ((hwActionPtr[2] >> 22) & 3);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->vlan.ingress.modifyVlan, hwValue);

    /* vlanId */
    swActionPtr->vlan.ingress.vlanId =
        ((hwActionPtr[2] >> 24) & 0x00FF)
        | ((hwActionPtr[3] << 8) & 0x0F00);

    /* precedence */
    hwValue = ((hwActionPtr[2] >> 20) & 1);
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(
        swActionPtr->vlan.ingress.precedence, hwValue);

    /* nestedVlan */
    swActionPtr->vlan.ingress.nestedVlan =
        BIT2BOOL_MAC((hwActionPtr[2] >> 21) & 1);

    /* Source Id */

    /* assignSourceId */
    swActionPtr->sourceId.assignSourceId =
        BIT2BOOL_MAC((hwActionPtr[2] >> 9) & 1);

    /* sourceIdValue */
    swActionPtr->sourceId.sourceIdValue =
        ((hwActionPtr[2] >> 10) & 0x1F);

    /* parse Lion and above fields */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        if (swActionPtr->redirect.redirectCmd
                == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            swActionPtr->redirect.data.outIf.modifyMacDa =
                BIT2BOOL_MAC((hwActionPtr[3] >> 20) & 1);
        }

        swActionPtr->redirect.data.outIf.modifyMacSa =
            BIT2BOOL_MAC((hwActionPtr[3] >> 21) & 1);
    }

     if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) != GT_FALSE)
    {
        /* unknownSaCommand - bits [115:114,58] */
        hwValue = (((hwActionPtr[3] >> 17) & 6) | ((hwActionPtr[1] >> 26) & 1));
        /* default */
        swActionPtr->unknownSaCommandEnable = GT_TRUE;

        switch (hwValue)
        {
            case 0:
                swActionPtr->unknownSaCommand = CPSS_PACKET_CMD_FORWARD_E;
                break;
            case 1:
                swActionPtr->unknownSaCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
                break;
            case 2:
                swActionPtr->unknownSaCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                break;
            case 3:
                swActionPtr->unknownSaCommand = CPSS_PACKET_CMD_DROP_HARD_E;
                break;
            case 4:
                swActionPtr->unknownSaCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                break;
            case 7:
                /* override variable */
                swActionPtr->unknownSaCommandEnable = GT_FALSE;
                swActionPtr->unknownSaCommand = 0;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChXCatPclTcamRuleIpUcRoutingActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Ip Unicast Routing Action from HW to SW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] hwActionPtr              - action in HW format (4 words)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
static GT_STATUS prvCpssDxChXCatPclTcamRuleIpUcRoutingActionHw2SwConvert
(
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
)
{
    GT_U32    hwValue; /* HW value */

    /* default settings - forward only */
    cpssOsMemSet(swActionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* pktCmd */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
        swActionPtr->pktCmd,(hwActionPtr[0] & 0x07));

    swActionPtr->egressPolicy = GT_FALSE;
    swActionPtr->ipUcRoute.doIpUcRoute = GT_TRUE;

    swActionPtr->actionStop        = GT_FALSE;
    swActionPtr->bypassBridge      = GT_FALSE;
    swActionPtr->bypassIngressPipe = GT_FALSE;

    /* Lookup config */
    swActionPtr->lookupConfig.ipclConfigIndex = 0;

    swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex =
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    swActionPtr->lookupConfig.pcl1OverrideConfigIndex =
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    /* Mirror */
    swActionPtr->mirror.mirrorToRxAnalyzerPort =
        BIT2BOOL_MAC(((hwActionPtr[0] >> 11) & 1));

    swActionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu = GT_FALSE;

    /* CPU code */

    /* default */
    swActionPtr->mirror.cpuCode = CPSS_NET_CLASS_KEY_TRAP_E;

    /* Match counting in CNC */

    swActionPtr->matchCounter.enableMatchCount =
        BIT2BOOL_MAC(((hwActionPtr[0] >> 28) & 1));

    swActionPtr->matchCounter.matchCounterIndex =
        ((hwActionPtr[0] >> 29) & 0x07) | ((hwActionPtr[1] & 0x03) << 3);

    /* QOS */

    /* modifyDscp */
    hwValue = ((hwActionPtr[0] >> 24) & 3);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyDscp, hwValue);

    /* modifyUp */
    hwValue = ((hwActionPtr[0] >> 26) & 3);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyUp, hwValue);

    /* profileIndex */
    swActionPtr->qos.ingress.profileIndex =
        ((hwActionPtr[0] >> 17) & 0x7F);

    /* profileAssignIndex */
    swActionPtr->qos.ingress.profileAssignIndex =
        BIT2BOOL_MAC(((hwActionPtr[0] >> 16) & 1));

    /* profilePrecedence */
    hwValue = ((hwActionPtr[0] >> 15) & 1);
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(
        swActionPtr->qos.ingress.profilePrecedence, hwValue);

    /* redirect */

    /* Only to explain SW data format */
    swActionPtr->redirect.redirectCmd
        = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;


    /* bit 46 - IsVidx */
    if ((hwActionPtr[1] >> 14) & 1)
    {
        swActionPtr->redirect.data.outIf.outInterface.vidx =
            (GT_U16)((hwActionPtr[1] >> 2) & 0x0FFF);

        swActionPtr->redirect.data.outIf.outInterface.type =
            (swActionPtr->redirect.data.outIf.outInterface.vidx == 0x0FFF)
        /* the packet's VID flooding */
        ? CPSS_INTERFACE_VID_E
        /* redirect to specified MA group */
        : CPSS_INTERFACE_VIDX_E;
    }
    /* bit 34 isTrunk*/
    else if ((hwActionPtr[1] >> 2) & 1)
    {
        swActionPtr->redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_TRUNK_E;
        swActionPtr->redirect.data.outIf.outInterface.trunkId =
            (GT_TRUNK_ID)((hwActionPtr[1] >> 3) & 0x007F);
    }
    /* port*/
    else
    {
        swActionPtr->redirect.data.outIf.outInterface.type =
            CPSS_INTERFACE_PORT_E;
        swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
            (GT_U8)((hwActionPtr[1] >> 9) & 0x001F);
        swActionPtr->redirect.data.outIf.outInterface.devPort.portNum =
            (GT_U8)((hwActionPtr[1] >> 3) & 0x003F);
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(swActionPtr->redirect.data.outIf.outInterface));
    }

    /* not relevant data */
    swActionPtr->redirect.data.outIf.tunnelStart = GT_FALSE;
    swActionPtr->redirect.data.outIf.tunnelPtr = 0;
    swActionPtr->redirect.data.outIf.tunnelType =
        CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
    swActionPtr->redirect.data.outIf.vntL2Echo = GT_FALSE;

    /* Policer */

    swActionPtr->policer.policerEnable =
        CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;

    /* policerId */
    swActionPtr->policer.policerId = 0;

    /* VLAN */

    /* modifyVlan */
    swActionPtr->vlan.ingress.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

    /* vlanId */
    swActionPtr->vlan.ingress.vlanId =
        (GT_U16)((hwActionPtr[1] >> 19) & 0x0FFF);

    /* precedence */
    swActionPtr->vlan.ingress.precedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* nestedVlan */
    swActionPtr->vlan.ingress.nestedVlan = GT_FALSE;

    /* Source Id */

    /* assignSourceId */
    swActionPtr->sourceId.assignSourceId = GT_FALSE;

    /* sourceIdValue */
    swActionPtr->sourceId.sourceIdValue = 0;

    /* Ip Uc specific */

    swActionPtr->ipUcRoute.arpDaIndex =
        (((hwActionPtr[0] >> 3) & 0x00FF)
        | ((hwActionPtr[1] >> 9) & 0x0300));

    swActionPtr->ipUcRoute.icmpRedirectCheck =
        BIT2BOOL_MAC(((hwActionPtr[2] >> 1) & 1));

    swActionPtr->ipUcRoute.bypassTTLCheck =
        BIT2BOOL_MAC(((hwActionPtr[2] >> 2) & 1));

    swActionPtr->ipUcRoute.decrementTTL =
        BIT2BOOL_MAC(((hwActionPtr[2] >> 3) & 1));

    return GT_OK;
}

/**
* @internal sip5PclTcamRuleIngressActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Ingress Action from HW to SW format - Bobcat2, Caelum, Aldrin, AC3X, Bobcat3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwActionPtr              - action in HW format (support SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS bits)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
static GT_STATUS sip5PclTcamRuleIngressActionHw2SwConvert
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
)
{
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_STATUS   rc;
    GT_U32  value,value1;
    GT_BOOL useIpclProfileIndexAndEnablers = GT_FALSE;
    GT_BOOL usePolicer = GT_TRUE;
    GT_U32  policerMeterEn , policerCounterEn;
    GT_U32  leafSwIndex;


    /* default settings - forward only */
    cpssOsMemSet(swActionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* convert packet command */
    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
        swActionPtr->pktCmd,value);

    if (swActionPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
            value);

        dsaCpuCode = value;
        /* in Bobcat2, Caelum, Bobcat3 the CPU code is also 'drop code' so relevant for drops too. */
        rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,
                &swActionPtr->mirror.cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* for 'forward' this param is "don't care" , but from trap/drop it is relevant ! */
        swActionPtr->mirror.cpuCode = CPSS_NET_CLASS_KEY_TRAP_E;
    }

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_0_E,
            value);
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_2_1_E,
            value1);
        value |= value1 << 1;
    }
    else
    {
        SIP_5_20_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_20_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
            value);
    }

    if(value)
    {
        swActionPtr->mirror.mirrorToRxAnalyzerPort = GT_TRUE;
        swActionPtr->mirror.ingressMirrorToAnalyzerIndex = value - 1;
    }
    else
    {
        swActionPtr->mirror.mirrorToRxAnalyzerPort = GT_FALSE;
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E,
        value);

    switch(value)
    {
        case 0:
            swActionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
                value);
            swActionPtr->redirect.data.noRedirect.modifyMacDa = BIT2BOOL_MAC(value);

            if (swActionPtr->redirect.data.noRedirect.modifyMacDa != GT_FALSE)
            {
                useIpclProfileIndexAndEnablers = GT_FALSE;

                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                    value);
                swActionPtr->redirect.data.noRedirect.arpPtr = value;
            }
            break;
        case 1:
            swActionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                value);
            if(value == 0)
            {
                /*port/trunk*/
                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                    value);

                if(value == 0)
                {/*port*/
                    swActionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;

                    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E,
                        value);
                    swActionPtr->redirect.data.outIf.outInterface.devPort.portNum = value;

                    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E,
                        value);
                    swActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum = value;
                }
                else
                { /*trunk-*/
                    swActionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_TRUNK_E;
                    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E,
                        value);
                    swActionPtr->redirect.data.outIf.outInterface.trunkId = (GT_TRUNK_ID)value;
                }
            }
            else
            {  /*vidx/vid*/
                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E,
                    value);
                if(value == 0xFFF)
                {
                    swActionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VID_E;
                }
                else
                {
                    swActionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VIDX_E;
                    swActionPtr->redirect.data.outIf.outInterface.vidx = (GT_U16)value;
                }
            }

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E,
                value);
            swActionPtr->redirect.data.outIf.vntL2Echo = BIT2BOOL_MAC(value);

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E,
                value);
            swActionPtr->redirect.data.outIf.tunnelStart = BIT2BOOL_MAC(value);

            if (value)/* tunnel start */
            {
                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E,
                    value);
                swActionPtr->redirect.data.outIf.tunnelPtr = value;

                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E,
                    value);
                swActionPtr->redirect.data.outIf.tunnelType = value ?
                    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E :
                    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
            }
            else    /*ARP pointer*/
            {
                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                    value);
                swActionPtr->redirect.data.outIf.arpPtr = value;

                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
                    value);
                swActionPtr->redirect.data.outIf.modifyMacDa = BIT2BOOL_MAC(value);
            }

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E,
                value);
            swActionPtr->redirect.data.outIf.modifyMacSa = BIT2BOOL_MAC(value);

            break;
        case 2:
            swActionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                value);
            /* get the HW index and translate it to SW index that take into consideration
               "holes" that we might have in the LPM RAM*/
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                leafSwIndex = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_HW_INDEX_TO_LEAF_SW_INDEX_MAC(value);
            }
            else
            {
                prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex(devNum, value, &leafSwIndex);
            }
            swActionPtr->redirect.data.routerLttIndex = leafSwIndex;

            break;
        case 4:
            swActionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                value);
            swActionPtr->redirect.data.vrfId = value;
            break;
        case 6:
            swActionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E;
            useIpclProfileIndexAndEnablers = GT_FALSE;
            usePolicer = GT_FALSE;

            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                value);
            swActionPtr->redirect.data.modifyMacSa.arpPtr = value;

            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_29_0_E,
                    value);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[5] = (GT_U8)(value & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[4] = (GT_U8)((value >> 8) & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[3] = (GT_U8)((value >> 16) & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] = (GT_U8)((value >> 24) & 0x3F);

                SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_30_E,
                    value);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] |= (GT_U8)((value & 0x03) << 6);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[1]  = (GT_U8)((value >> 2) & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[0]  = (GT_U8)((value >> 10) & 0xFF);
            }
            else
            {
                SIP_5_20_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E,
                    value);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[5] = (GT_U8) (value        & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[4] = (GT_U8)((value >> 8)  & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[3] = (GT_U8)((value >> 16) & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] = (GT_U8)((value >> 24) & 0x0F);

                SIP_5_20_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                    SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E,
                    value);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[2] |= (GT_U8)((value & 0x0F) << 4);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[1]  = (GT_U8)((value >>  4) & 0xFF);
                swActionPtr->redirect.data.modifyMacSa.macSa.arEther[0]  = (GT_U8)((value >> 12) & 0xFF);
            }


            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(useIpclProfileIndexAndEnablers == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E,
                value);
            swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex = value ?
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E :
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;


            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                value);
            swActionPtr->lookupConfig.pcl1OverrideConfigIndex = value ?
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E :
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        }
        else
        {
            SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                value);
            swActionPtr->lookupConfig.pcl0_1OverrideConfigIndex = value ?
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E :
                CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        }

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E,
            value);
        swActionPtr->lookupConfig.ipclConfigIndex = value;
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);
    swActionPtr->matchCounter.enableMatchCount = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        value);
    swActionPtr->matchCounter.matchCounterIndex = value;

    if (usePolicer != GT_FALSE)
    {
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E,
            value);
        policerMeterEn = value;

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
            value);
        policerCounterEn = value;

        if(policerCounterEn)
        {
            if(policerMeterEn)
            {
                swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            }
            else
            {
                swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            }
        }
        else
        {
            if(policerMeterEn)
            {
                swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
            }
            else
            {
                swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
            }
        }

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E,
            value);
        swActionPtr->policer.policerId = value;
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        value);
    swActionPtr->sourceId.assignSourceId = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E,
        value);
    swActionPtr->sourceId.sourceIdValue = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        value);
    swActionPtr->actionStop = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E,
        value);
    swActionPtr->bypassBridge = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        value);
    swActionPtr->bypassIngressPipe = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E,
        value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(
        swActionPtr->vlan.ingress.precedence, value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E,
        value);
    swActionPtr->vlan.ingress.nestedVlan = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E,
        value);

    switch (value)
    {
        case 0:
            swActionPtr->vlan.ingress.modifyVlan =
                CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
            break;
        case 1:
            swActionPtr->vlan.ingress.modifyVlan =
                CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
            break;
        case 2:
            swActionPtr->vlan.ingress.modifyVlan =
                CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
            break;
        case 3:
            swActionPtr->vlan.ingress.modifyVlan =
                CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            break;
        default:
            break;
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E,
        value);
    swActionPtr->vlan.ingress.vlanId = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E,
        value);
    swActionPtr->qos.ingress.profileAssignIndex = BIT2BOOL_MAC(value);

    /* qos profile assignment precedence */
    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(
        swActionPtr->qos.ingress.profilePrecedence, value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
        value);
    swActionPtr->qos.ingress.profileIndex = value;

    /* enable modify DSCP */
    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        value);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyDscp, value);

    /* enable modify UP */
    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        value);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
        swActionPtr->qos.ingress.modifyUp, value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E,
        value);
    swActionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu = BIT2BOOL_MAC(value);

    if(swActionPtr->redirect.redirectCmd != CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {

        CPSS_TBD_BOOKMARK /* no such field in the STC */
        /*
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
            value);*/
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E,
        value);
    switch (value)
    {
        case 0:
            swActionPtr->vlan.ingress.vlanId1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            break;
        case 1:
            swActionPtr->vlan.ingress.vlanId1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E;
            break;
        case 2:
            swActionPtr->vlan.ingress.vlanId1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
            break;
        case 3:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E,
        value);
    switch (value)
    {
        case 0:
            swActionPtr->qos.ingress.up1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            break;
        case 1:
            swActionPtr->qos.ingress.up1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E;
            break;
        case 2:
            swActionPtr->qos.ingress.up1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E;
            break;
        case 3:
            swActionPtr->qos.ingress.up1Cmd =
                CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E;
            break;
        default:
            break;
    }

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E,
        value);
    swActionPtr->vlan.ingress.vlanId1 = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E,
        value);
    swActionPtr->qos.ingress.up1 = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        value);
    swActionPtr->flowId = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        value);
    swActionPtr->setMacToMe = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E,
        value);
    swActionPtr->oam.timeStampEnable = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E,
        value);
    swActionPtr->oam.offsetIndex = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E,
        value);
    swActionPtr->oam.oamProcessEnable = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);
    swActionPtr->oam.oamProfile = value;

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E,
        value);
    swActionPtr->sourcePort.assignSourcePortEnable = BIT2BOOL_MAC(value);

    SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E,
        value);
    swActionPtr->sourcePort.sourcePortValue = value;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E,
            value);
        swActionPtr->skipFdbSaLookup = BIT2BOOL_MAC(value);

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E,
            value);
        swActionPtr->triggerInterrupt = BIT2BOOL_MAC(value);

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E,
            value);
        value ^= 1;
        swActionPtr->exactMatchOverPclEn = BIT2BOOL_MAC(value);

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
            value);
        swActionPtr->latencyMonitor.monitoringEnable = BIT2BOOL_MAC(value);

        SIP5_IPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
            value);
        swActionPtr->latencyMonitor.latencyProfile = value;

    }
    return GT_OK;
}

/**
* @internal sip5PclTcamRuleEgressActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Egress Action from HW to SW format - Bobcat2, Caelum, Aldrin, AC3X, Bobcat3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwActionPtr              - action in HW format (support SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS bits)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
static GT_STATUS sip5PclTcamRuleEgressActionHw2SwConvert
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
)
{
    GT_U32  value;
    GT_U32 policerMeterEn , policerCounterEn;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egress_mirroring_mode;
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT IN_phaThreadType;/* IN value of phaThreadType .. from the application */
    GT_STATUS   rc;

    /*
        IMPORTANT : this field filled by CPSS , to support 'unaware'
            applications to the 'PHA metadata'
    */
    /* 'save' the value , before the 'memset' */
    IN_phaThreadType = swActionPtr->epclPhaInfo.phaThreadType;

    /* clear */
    cpssOsMemSet(
        swActionPtr, 0,
        sizeof(*swActionPtr));

    swActionPtr->egressPolicy = GT_TRUE;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        value);
    /* convert packet command */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
        swActionPtr->pktCmd, value);

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (swActionPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E))
    {
        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
            value);

        dsaCpuCode = value;
        /* For SIP5 and above the CPU code is also 'drop code' so relevant for drops too. */
        rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,
                &swActionPtr->mirror.cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E,
            value);
        if(value)
        {
            swActionPtr->mirror.mirrorToTxAnalyzerPortEn = GT_TRUE;
            swActionPtr->mirror.egressMirrorToAnalyzerIndex = value - 1;
        }
        else
        {
            swActionPtr->mirror.mirrorToTxAnalyzerPortEn = GT_FALSE;
        }
        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
            value);
        PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_SW_VAL_MAC(value, egress_mirroring_mode);
        swActionPtr->mirror.egressMirrorToAnalyzerMode = egress_mirroring_mode;

        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E,
            value);

        if(value == 0)
        {
            swActionPtr->epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E;
            swActionPtr->epclPhaInfo.phaThreadId = 0;
        }
        else
        {
            SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E,
                value);

            if(value == 0)
            {
                swActionPtr->epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E;
                swActionPtr->epclPhaInfo.phaThreadId = 0;
            }
            else
            {
                swActionPtr->epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
                swActionPtr->epclPhaInfo.phaThreadId = value;
            }
        }

        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E,
            value);

        if(value == 0)
        {
            /*
                IMPORTANT : this field filled by CPSS , to support 'unaware'
                    applications to the 'PHA metadata'
                    (unlike in the case of value != 0)
            */
            swActionPtr->epclPhaInfo.phaThreadType = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E;
            swActionPtr->epclPhaInfo.phaThreadUnion.notNeeded = 0;
        }
        else
        {                                 
            SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
                SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E,
                value);

            /*
                IMPORTANT : this field is input !!! although we are in 'Get' function !!!
                the application gives us hint how to parse the info from the HW.
            */
            swActionPtr->epclPhaInfo.phaThreadType = IN_phaThreadType;

            switch(swActionPtr->epclPhaInfo.phaThreadType)
            {
                case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
                    swActionPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption =
                        BIT2BOOL_MAC((value >> 31) & 1);
                    break;
                default: /* unknown format */
                    CPSS_LOG_ERROR_MAC("epclPhaInfo.phaThreadType must hold value CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E when SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E = 'enabled'");
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(swActionPtr->epclPhaInfo.phaThreadType);
            }
        }
    }

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E,
        value);
    swActionPtr->qos.egress.dscp = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E,
        value);
    swActionPtr->qos.egress.up = value;

    CPSS_TBD_BOOKMARK/* 3 values field --> need new enum for modifyDscp */
    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E,
        value);
    switch (value)
    {
        case 0:
            swActionPtr->qos.egress.modifyDscp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            break;
        case 1:
            swActionPtr->qos.egress.modifyDscp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            break;
        case 2:
            swActionPtr->qos.egress.modifyDscp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E,
        value);
    switch (value)
    {
        case 0:
            swActionPtr->qos.egress.modifyUp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            break;
        case 1:
            swActionPtr->qos.egress.modifyUp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            break;
        case 2:
            swActionPtr->qos.egress.modifyUp = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);
    swActionPtr->matchCounter.enableMatchCount = BIT2BOOL_MAC(value);

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        value);
    swActionPtr->matchCounter.matchCounterIndex = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E,
        value);
    swActionPtr->vlan.egress.vlanId1ModifyEnable = BIT2BOOL_MAC(value);

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E,
        value);
    swActionPtr->qos.egress.up1ModifyEnable = BIT2BOOL_MAC(value);

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E,
        value);
    swActionPtr->vlan.egress.vlanId1 = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E,
        value);
    swActionPtr->qos.egress.up1 = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E,
        value);
    switch (value)
    {
        case 0:
            swActionPtr->vlan.egress.vlanCmd = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            break;
        case 1:
            swActionPtr->vlan.egress.vlanCmd = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            break;
        case 2:
            swActionPtr->vlan.egress.vlanCmd = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E,
        value);
    swActionPtr->vlan.egress.vlanId = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        value);
    policerCounterEn = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        value);
    policerMeterEn = value;

    if(policerCounterEn)
    {
        if(policerMeterEn)
        {
            swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
        }
        else
        {
            swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        }
    }
    else
    {
        if(policerMeterEn)
        {
            swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
        }
        else
        {
            swActionPtr->policer.policerEnable = CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
        }
    }

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
        value);
    swActionPtr->policer.policerId = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        value);
    swActionPtr->flowId = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        value);
    swActionPtr->oam.oamProcessEnable = BIT2BOOL_MAC(value);

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);
    swActionPtr->oam.oamProfile = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E,
        value);
    swActionPtr->oam.timeStampEnable = BIT2BOOL_MAC(value);

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E,
        value);
    swActionPtr->oam.offsetIndex = value;

    SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        value);
    swActionPtr->channelTypeToOpcodeMapEnable = BIT2BOOL_MAC(value);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported)
    {
        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E,
            value);
    }
    else
    {
        value = 0;
    }

    swActionPtr->tmQueueId = value;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* terminate Cut Through packet mode */
        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E,
            value);
        swActionPtr->terminateCutThroughMode = BOOL2BIT_MAC(value);

        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E,
            value);
        value ^= 1;
        swActionPtr->exactMatchOverPclEn = BOOL2BIT_MAC(value);

        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
            value);
        swActionPtr->latencyMonitor.monitoringEnable = BOOL2BIT_MAC(value);

        SIP5_EPCL_ACTION_FIELD_GET_MAC(devNum,hwActionPtr,
            SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
            value);
        swActionPtr->latencyMonitor.latencyProfile = value;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Action from HW to SW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
* @param[in] hwActionPtr              - action in HW format (4 words)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
GT_STATUS prvCpssDxChPclTcamRuleActionHw2SwConvert
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
)
{
    GT_U16 usedVidx;         /* used Vidx                           */
    GT_STATUS rc;            /* return code                         */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT   dsaCpuCode; /* cpuCode ih HW format */
    GT_U32    cheetahVer;    /* version of Cheetah                  */
    GT_U32    redirCmd;      /* redirect command                    */

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            /*IPCL*/
            return sip5PclTcamRuleIngressActionHw2SwConvert(devNum,
                        hwActionPtr, swActionPtr);
        }
        else
        {
            /*EPCL*/
            return sip5PclTcamRuleEgressActionHw2SwConvert(devNum,
                        hwActionPtr, swActionPtr);
        }
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
            cheetahVer = 1;
            break;
        case CPSS_PP_FAMILY_CHEETAH2_E:
            cheetahVer = 2;
            break;
        case CPSS_PP_FAMILY_CHEETAH3_E:
            cheetahVer = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
            {
                if  (((hwActionPtr[0] >> 12) & 3) != 3)
                {
                    /* xCat and above INGRESS action         */
                    /* but not special "Route to Next Hop action */
                    return prvCpssDxChXCatPclTcamRuleIngressActionHw2SwConvert(
                        devNum, hwActionPtr, swActionPtr);
                }
                else
                {
                    /* xCat and above Route to Next Hop action */
                    return prvCpssDxChXCatPclTcamRuleIpUcRoutingActionHw2SwConvert(
                        hwActionPtr, swActionPtr);
                }
            }
            /* xCat, xCat3, Lion, xCat2 */
            cheetahVer = 4;

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* default settings - forward only */
    cpssOsMemSet(swActionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* pktCmd */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
        swActionPtr->pktCmd,(hwActionPtr[0] & 0x07));

    swActionPtr->egressPolicy =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E) ? GT_TRUE : GT_FALSE;

    swActionPtr->policer.policerEnable = GT_FALSE;
    swActionPtr->ipUcRoute.doIpUcRoute = GT_FALSE;

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        /* determinate type of action: regular or 98DX2X3 IP UC routing */
        swActionPtr->ipUcRoute.doIpUcRoute =
            ((hwActionPtr[1] & 3) == 3) ? GT_TRUE : GT_FALSE;

        /* Parsing of multiplexed fields */
        if (swActionPtr->ipUcRoute.doIpUcRoute == GT_FALSE)
        {
            /* policer */
            swActionPtr->policer.policerEnable =
                ((hwActionPtr[2] & 1) != 0) ? GT_TRUE : GT_FALSE;

            swActionPtr->policer.policerId = (hwActionPtr[2] >> 1) & 0x00FF;

            if (cheetahVer >= 3)
            {
                /* policer bits 11:8 */
                swActionPtr->policer.policerId |= (((hwActionPtr[3] >> 10) & 0x0F) << 8);
            }

            /* CPU Code */
            dsaCpuCode = (GT_U8)((hwActionPtr[0] >> 3) & 0xFF);

            rc = prvCpssDxChNetIfDsaToCpuCode(
                dsaCpuCode, &swActionPtr->mirror.cpuCode);

            if (rc != GT_OK)
            {
                /* default */
                swActionPtr->mirror.cpuCode = CPSS_NET_CLASS_KEY_TRAP_E;
            }
        }
        else
        {
            /* policer not used for IP UC Router entries */
            swActionPtr->policer.policerEnable = GT_FALSE;

            /* ARP DA Index */
            swActionPtr->ipUcRoute.arpDaIndex =
                ((hwActionPtr[0] >> 3) & 0x00FF)
                | (((hwActionPtr[1] >> 17) & 3) << 8);

            /* bypass TTL */
            swActionPtr->ipUcRoute.bypassTTLCheck =
                (((hwActionPtr[2] >> 2) & 1) != 0) ? GT_TRUE : GT_FALSE;

            /* decrement TTL*/
            swActionPtr->ipUcRoute.decrementTTL =
                (((hwActionPtr[2] >> 3) & 1) != 0) ? GT_TRUE : GT_FALSE;

            /* ICMP check */
            swActionPtr->ipUcRoute.icmpRedirectCheck =
                (((hwActionPtr[2] >> 1) & 1) != 0) ? GT_TRUE : GT_FALSE;

        }

        swActionPtr->actionStop = GT_FALSE;
        if (cheetahVer >= 3)
        {
            /* bit 97 - action stop enable */
            if (hwActionPtr[3] & 2)
            {
                swActionPtr->actionStop = GT_TRUE;
            }
        }

        swActionPtr->sourceId.assignSourceId = GT_FALSE;
        if (cheetahVer >= 3)
        {
            /* assign source id enable: bit 113 */
            if ((hwActionPtr[3] >> 17) & 1)
            {
                swActionPtr->sourceId.assignSourceId = GT_TRUE;
                swActionPtr->sourceId.sourceIdValue =
                    ((hwActionPtr[3] >> 18) & 0x1F);
            }
        }

        /* mirror */
        swActionPtr->mirror.mirrorToRxAnalyzerPort =
            BIT2BOOL_MAC(((hwActionPtr[0] >> 11) & 1));
    }

    /* matchCounter */

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        swActionPtr->matchCounter.matchCounterIndex =
            (hwActionPtr[0] >> 26) & 0x1F;
        swActionPtr->matchCounter.enableMatchCount =
            (((hwActionPtr[0] >> 25) & 1) != 0) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
            devNum, PRV_CPSS_DXCH2_PCL_EGRESS_ACTION_CNTR_VLAN_WRONG_BITS_POS_WA_E))
        {
            swActionPtr->matchCounter.matchCounterIndex =
                (hwActionPtr[0] >> 25) & 0x1F;
            swActionPtr->matchCounter.enableMatchCount =
                (((hwActionPtr[0] >> 30) & 1) != 0) ? GT_TRUE : GT_FALSE;
        }
        else
        {
            swActionPtr->matchCounter.matchCounterIndex =
                (hwActionPtr[0] >> 26) & 0x1F;
            swActionPtr->matchCounter.enableMatchCount =
                (((hwActionPtr[0] >> 25) & 1) != 0) ? GT_TRUE : GT_FALSE;
        }
    }

    if (cheetahVer >= 3)
    {
        /* counter index's bit 5 - 31 */
        swActionPtr->matchCounter.matchCounterIndex |=
            (((hwActionPtr[0] >> 31) & 0x1) << 5);
        /* counter index's bits 13:6 - 105:98 */
        swActionPtr->matchCounter.matchCounterIndex |=
            (((hwActionPtr[3] >> 2) & 0x00FF) << 6);
    }

    /* vlan and OAM */

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        switch ((hwActionPtr[1] >> 17) & 0x03)
        {
            default: /* reserved values*/
            case 0:
                swActionPtr->vlan.ingress.modifyVlan =
                    CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
                break;
            case 1:
                swActionPtr->vlan.ingress.modifyVlan =
                    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
                break;
            case 2:
                swActionPtr->vlan.ingress.modifyVlan =
                    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
                break;
            case 3:
                swActionPtr->vlan.ingress.modifyVlan =
                    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
                break;
        }

        swActionPtr->vlan.ingress.vlanId = (GT_U16)((hwActionPtr[1] >> 19) & 0x0FFF);

        swActionPtr->vlan.ingress.nestedVlan =
            (((hwActionPtr[1] >> 16) & 1) != 0) ? GT_TRUE : GT_FALSE;

        swActionPtr->vlan.ingress.precedence =
            (((hwActionPtr[1] >> 15) & 1) != 0)
            ? CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
            : CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    }
    else
    {
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
            devNum, PRV_CPSS_DXCH2_PCL_EGRESS_ACTION_CNTR_VLAN_WRONG_BITS_POS_WA_E))
        {
            switch ((hwActionPtr[1] >> 18) & 0x03)
            {
                case 0:
                    swActionPtr->vlan.egress.vlanCmd
                        = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                    break;
                case 1:
                    swActionPtr->vlan.egress.vlanCmd
                        = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            swActionPtr->vlan.egress.vlanId =
                (GT_U16)((hwActionPtr[1] >> 20) & 0x07FF);
            swActionPtr->vlan.egress.vlanId |=
                (GT_U16)((hwActionPtr[2] & 1) << 11);
        }
        else
        {
            switch ((hwActionPtr[1] >> 17) & 0x03)
            {
                case 0:
                    swActionPtr->vlan.egress.vlanCmd
                        = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                    break;
                case 1:
                    swActionPtr->vlan.egress.vlanCmd
                        = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            swActionPtr->vlan.egress.vlanId =
                (GT_U16)((hwActionPtr[1] >> 19) & 0x0FFF);
        }
    }


    /* QOS */

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* egress */
        /* Modify DSCP */
        switch ((hwActionPtr[0] >> 21) & 3)
        {
            case 0:
                swActionPtr->qos.egress.modifyDscp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case 1:
                swActionPtr->qos.egress.modifyDscp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Modify UP */
        switch ((hwActionPtr[0] >> 23) & 3)
        {
            case 0:
                swActionPtr->qos.egress.modifyUp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case 1:
                swActionPtr->qos.egress.modifyUp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        swActionPtr->qos.egress.dscp =
            (GT_U8)((hwActionPtr[0] >> 12) & 0x3F);
        swActionPtr->qos.egress.up   =
            (GT_U8)((hwActionPtr[0] >> 18) & 0x07);
    }
    else
    {
        /* ingress */

        /* Modify DSCP */
        PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
            swActionPtr->qos.ingress.modifyDscp, ((hwActionPtr[0] >> 21) & 3));

        /* Modify UP */
        PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(
            swActionPtr->qos.ingress.modifyUp, ((hwActionPtr[0] >> 23) & 3));

        swActionPtr->qos.ingress.profileAssignIndex =
            (((hwActionPtr[0] >> 13) & 1) != 0) ? GT_TRUE : GT_FALSE;

        swActionPtr->qos.ingress.profileIndex =
            (hwActionPtr[0] >> 14) & 0x7F;

        swActionPtr->qos.ingress.profilePrecedence =
            (((hwActionPtr[0] >> 12) & 1) != 0)
            ? CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
            : CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    }

    /* REDIRECT */

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        redirCmd = hwActionPtr[1] & 3;
        if (cheetahVer >= 3)
        {
            /* redirCmd bit 2 - bit 119 */
            redirCmd |= (((hwActionPtr[3] >> 23) & 1) << 2);
        }

        switch (redirCmd)
        {
            default: /* must never occur      */
            case 0:  /* no redirection        */
                swActionPtr->redirect.redirectCmd =
                    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
                break;

            case 3:  /* 98DX2X3 IP UC routing */
            case 1:  /* redirect to interface */
                swActionPtr->redirect.redirectCmd =
                    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

                if (((hwActionPtr[1] >> 14) & 1) != 0)
                {
                    /* use vidx */
                    usedVidx = (GT_U16)((hwActionPtr[1] >> 2) & 0x0FFF);

                    /* the constant 0xFFF used for send packet to VLAN */
                    if (usedVidx != 0xFFF)
                    {
                        /* vidx */
                        swActionPtr->redirect.data.outIf.outInterface.type =
                            CPSS_INTERFACE_VIDX_E;

                        swActionPtr->redirect.data.outIf.outInterface.vidx =
                            usedVidx;
                    }
                    else
                    {
                        /* vid */
                        swActionPtr->redirect.data.outIf.outInterface.type =
                            CPSS_INTERFACE_VID_E;

                        swActionPtr->redirect.data.outIf.outInterface.vlanId =
                            usedVidx;
                    }
                }
                else
                {
                    /* port or trunk */
                    if (((hwActionPtr[1] >> 2) & 1) != 0)
                    {
                        /* trunk */
                        swActionPtr->redirect.data.outIf.outInterface.type =
                            CPSS_INTERFACE_TRUNK_E;

                        swActionPtr->redirect.data.outIf.outInterface.trunkId =
                            (GT_U16)((hwActionPtr[1] >> 3) & 0x07F);
                    }
                    else
                    {
                        /* port */
                        swActionPtr->redirect.data.outIf.outInterface.type =
                            CPSS_INTERFACE_PORT_E;

                        swActionPtr->redirect.data.outIf.outInterface.
                            devPort.hwDevNum = (GT_U8)((hwActionPtr[1] >> 9) & 0x1F);

                        swActionPtr->redirect.data.outIf.outInterface.
                            devPort.portNum = (GT_U8)((hwActionPtr[1] >> 3) & 0x3F);
                        PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(swActionPtr->redirect.data.outIf.outInterface));
                    }
                }

                /* tunnel and VNT support for DxCh2 and above devices */
                if (cheetahVer >= 2)
                {
                    swActionPtr->redirect.data.outIf.tunnelPtr =
                        (hwActionPtr[2] >> 11) & 0x03FF;
                    if (cheetahVer >= 3)
                    {
                        /* tunnelPtr bits 12:10 - bits 112:110 */
                        swActionPtr->redirect.data.outIf.tunnelPtr |=
                            (((hwActionPtr[3] >> 14) & 7) << 10);
                    }

                    swActionPtr->redirect.data.outIf.tunnelStart =
                        (((hwActionPtr[2] >> 10) & 1) != 0) ? GT_TRUE : GT_FALSE;

                    swActionPtr->redirect.data.outIf.vntL2Echo =
                        (((hwActionPtr[2] >> 9) & 1) != 0) ? GT_TRUE : GT_FALSE;

                    if (cheetahVer >= 3)
                    {
                        /* bit 85 : tunnel type */
                        swActionPtr->redirect.data.outIf.tunnelType =
                            ((hwActionPtr[2] >> 21) == 0)
                            ? CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E
                            : CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
                    }
                }

                break;


            case 2:  /* redirect to IP router */
                swActionPtr->redirect.redirectCmd =
                    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
                swActionPtr->redirect.data.routerLttIndex =
                    (hwActionPtr[1] >> 2) & 0x1FFF;
                if (cheetahVer >= 3)
                {
                    /* bits 121:120 - bits 14:13 of LttIndex */
                    swActionPtr->redirect.data.routerLttIndex |=
                        (((hwActionPtr[3] >> 24) & 3) << 13);
                }

                break;

            case 4: /* virtual router */
                swActionPtr->redirect.redirectCmd =
                    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;
                swActionPtr->redirect.data.vrfId =
                    (hwActionPtr[1] >> 2) & 0x0FFF;

                break;
        }
    }

    /* Egress Action Policer xCat and above devices */
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E)
        && (cheetahVer >= 4))
    {
        /* bind to meter and to counter */
        switch (((hwActionPtr[2] & 1) << 1) | ((hwActionPtr[1] >> 31) & 1))
        {
            case 0:
                swActionPtr->policer.policerEnable =
                    CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
                break;
            case 1:
                swActionPtr->policer.policerEnable =
                    CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
                break;
            case 2:
                swActionPtr->policer.policerEnable =
                    CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
                break;
            case 3:
                swActionPtr->policer.policerEnable =
                    CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
                break;
            /* cannot occur */
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        swActionPtr->policer.policerId =
            ((hwActionPtr[2] >> 1) & 0x00FF)
            | ((hwActionPtr[3] >> 2) & 0x0F00);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChXCatIngressPclTcamRuleDataSw2HwConvert function
* @endinternal
*
* @brief   Converts XCat TCAM pattern or mask from SW to HW Ingress ket format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] ruleFormat               - one of supported ingress key formats
* @param[in] swDataPtr                - rule pattern or mask in SW format
*
* @param[out] hwDataPtr                - rule pattern or mask in HW format (6 or 12 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
static GT_STATUS prvCpssDxChXCatIngressPclTcamRuleDataSw2HwConvert
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT      *swDataPtr,
    OUT GT_U32                             *hwDataPtr
)
{
    GT_U32  size;   /* size of heData in words    */
    GT_U32  i;      /* loop index                 */

    /* format depended pointers into swDataPtr */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC                 *commonPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC             *commonExtPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC          *commonStdIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC     *commonUdbPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC             *stdNotIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC          *stdIpL2QosPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC            *stdIpL4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC   *stdUdbPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC           *extNotIpv6Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC            *extIpv6L2Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC            *extIpv6L4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC   *extUdbPtr;

     /* clear all format depended pointers    */
    /* only relevant will get not null value */
    commonPtr         = NULL;
    commonExtPtr      = NULL;
    commonStdIpPtr    = NULL;
    commonUdbPtr      = NULL;
    stdNotIpPtr       = NULL;
    stdIpL2QosPtr     = NULL;
    stdIpL4Ptr        = NULL;
    stdUdbPtr         = NULL;
    extNotIpv6Ptr     = NULL;
    extIpv6L2Ptr      = NULL;
    extIpv6L4Ptr      = NULL;
    extUdbPtr         = NULL;

    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            stdNotIpPtr    = &(swDataPtr->ruleStdNotIp);
            commonPtr      = &(stdNotIpPtr->common);
            size           = 6;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            stdIpL2QosPtr  = &(swDataPtr->ruleStdIpL2Qos);
            commonPtr      = &(stdIpL2QosPtr->common);
            commonStdIpPtr = &(stdIpL2QosPtr->commonStdIp);
            size           = 6;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            stdIpL4Ptr     = &(swDataPtr->ruleStdIpv4L4);
            commonPtr      = &(stdIpL4Ptr->common);
            commonStdIpPtr = &(stdIpL4Ptr->commonStdIp);
            size           = 6;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            stdUdbPtr      = &(swDataPtr->ruleIngrStdUdb);
            commonUdbPtr   = &(stdUdbPtr->commonIngrUdb);
            size           = 6;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            extNotIpv6Ptr  = &(swDataPtr->ruleExtNotIpv6);
            commonPtr      = &(extNotIpv6Ptr->common);
            commonExtPtr   = &(extNotIpv6Ptr->commonExt);
            size           = 12;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            extIpv6L2Ptr   = &(swDataPtr->ruleExtIpv6L2);
            commonPtr      = &(extIpv6L2Ptr->common);
            commonExtPtr   = &(extIpv6L2Ptr->commonExt);
            size           = 12;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            extIpv6L4Ptr   = &(swDataPtr->ruleExtIpv6L4);
            commonPtr      = &(extIpv6L4Ptr->common);
            commonExtPtr   = &(extIpv6L4Ptr->commonExt);
            size           = 12;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            extUdbPtr      = &(swDataPtr->ruleIngrExtUdb);
            commonUdbPtr   = &(extUdbPtr->commonIngrUdb);
            size           = 12;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* clear the hwData before passing data from SW */
    for (i = 0; (i < size); i++)
    {
        hwDataPtr[i] = 0;
    }

    /* common for ingress formats */
    if (commonPtr      != NULL)
    {
        /* PCL Id*/
        hwDataPtr[0] |= (commonPtr->pclId & 0x03FF);

        /* macToMe */
        hwDataPtr[0] |= ((commonPtr->macToMe & 1) << 10);

        /* source port */
        hwDataPtr[0] |= ((commonPtr->sourcePort & 0x3F) << 11);

        /* isTagged */
        hwDataPtr[0] |= ((commonPtr->isTagged & 0x01) << 17);

        /* VID */
        hwDataPtr[0] |= ((commonPtr->vid & 0x0FFF) << 18);

        /* UP */
        hwDataPtr[0] |= ((commonPtr->up & 0x03) << 30);
        hwDataPtr[1] |= ((commonPtr->up & 0x04) >> 2);

        /* Qos Profile */
        hwDataPtr[1] |= ((commonPtr->qosProfile & 0x7F) << 1);

        /* is IP */
        hwDataPtr[1] |= ((commonPtr->isIp & 0x01) << 9);

        /* is L2 valid */
        hwDataPtr[3] |= ((commonPtr->isL2Valid & 0x01) << 11);

        /* is UDB Valid */
        hwDataPtr[3] |= ((commonPtr->isUdbValid & 0x01) << 10);

    }

    /* common for all extended ingress formats */
    if (commonExtPtr   != NULL)
    {
        /* is IPV6 */
        hwDataPtr[1] |= ((commonExtPtr->isIpv6 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonExtPtr->ipProtocol & 0xFF) << 16);

        /* DSCP */
        hwDataPtr[1] |= ((commonExtPtr->dscp & 0x3F) << 10);

        /* isL4Valid */
        hwDataPtr[3] |= ((commonExtPtr->isL4Valid & 0x01) << 8);

        /* l4Byte0 */
        hwDataPtr[2] |= (commonExtPtr->l4Byte0 << 16);

        /* l4Byte1 */
        hwDataPtr[2] |= (commonExtPtr->l4Byte1 << 24);

        /* l4Byte2 */
        hwDataPtr[2] |= (commonExtPtr->l4Byte2 << 8);

        /* l4Byte3 */
        hwDataPtr[2] |= commonExtPtr->l4Byte3;

        /* l4Byte13 */
        hwDataPtr[3] |= commonExtPtr->l4Byte13;

        /* ipHeaderOk */
        hwDataPtr[3] |= ((commonExtPtr->ipHeaderOk & 0x01) << 13);
    }

    if (commonStdIpPtr != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((commonStdIpPtr->isIpv4 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonStdIpPtr->ipProtocol & 0xFF) << 16);

        /* DSCP */
        hwDataPtr[1] |= ((commonStdIpPtr->dscp & 0x3F) << 10);

        /* isL4Valid */
        hwDataPtr[3] |= ((commonStdIpPtr->isL4Valid & 0x01) << 8);

        /* l4Byte2 */
        hwDataPtr[2] |= (commonStdIpPtr->l4Byte2 << 8);

        /* l4Byte3 */
        hwDataPtr[2] |= commonStdIpPtr->l4Byte3;

        /* ipHeaderOk */
        hwDataPtr[3] |= ((commonStdIpPtr->ipHeaderOk & 0x01) << 13);

        /* ipv4Fragmented */
        hwDataPtr[3] |= ((commonStdIpPtr->ipv4Fragmented & 0x01) << 12);
    }

    /* standard ingress not IP */
    if (stdNotIpPtr    != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((stdNotIpPtr->isIpv4 & 0x01) << 8);

        /* etherType */
        hwDataPtr[1] |= (stdNotIpPtr->etherType << 10);

        /* isArp */
        hwDataPtr[3] |= ((stdNotIpPtr->isArp & 0x01) << 9);

        /* l2Encap */
        hwDataPtr[3] |= ((stdNotIpPtr->l2Encap & 0x01) << 12);

        /* MAC DA */
        hwDataPtr[4] |= (stdNotIpPtr->macDa.arEther[5] << 16);
        hwDataPtr[4] |= (stdNotIpPtr->macDa.arEther[4] << 24);
        hwDataPtr[5] |=  stdNotIpPtr->macDa.arEther[3];
        hwDataPtr[5] |= (stdNotIpPtr->macDa.arEther[2] << 8);
        hwDataPtr[5] |= (stdNotIpPtr->macDa.arEther[1] << 16);
        hwDataPtr[5] |= (stdNotIpPtr->macDa.arEther[0] << 24);

        /* MAC SA */
        hwDataPtr[2] |= (stdNotIpPtr->macSa.arEther[5] << 16);
        hwDataPtr[2] |= (stdNotIpPtr->macSa.arEther[4] << 24);
        hwDataPtr[3] |=  stdNotIpPtr->macSa.arEther[3];
        hwDataPtr[3] |= (stdNotIpPtr->macSa.arEther[2] << 24);
        hwDataPtr[4] |=  stdNotIpPtr->macSa.arEther[1];
        hwDataPtr[4] |= (stdNotIpPtr->macSa.arEther[0] << 8);

        /* UDB15 */
        hwDataPtr[2] |=  stdNotIpPtr->udb15_17[0];
        /* UDB16 */
        hwDataPtr[2] |= (stdNotIpPtr->udb15_17[1] << 8);
        /* UDB17 */
        hwDataPtr[3] |= (stdNotIpPtr->udb15_17[2] << 16);
    }

    /* standard ingress IPV4 and IPV6 L2 Qos */
    if (stdIpL2QosPtr  != NULL)
    {
        /* isArp */
        hwDataPtr[3] |= ((stdIpL2QosPtr->isArp & 0x01) << 9);

        /* isIpv6ExtHdrExist */
        hwDataPtr[3] |= ((stdIpL2QosPtr->isIpv6ExtHdrExist & 0x01) << 14);

        /* isIpv6HopByHop */
        hwDataPtr[3] |= ((stdIpL2QosPtr->isIpv6HopByHop & 0x01) << 15);

        /* MAC DA */
        hwDataPtr[4] |= (stdIpL2QosPtr->macDa.arEther[5] << 16);
        hwDataPtr[4] |= (stdIpL2QosPtr->macDa.arEther[4] << 24);
        hwDataPtr[5] |=  stdIpL2QosPtr->macDa.arEther[3];
        hwDataPtr[5] |= (stdIpL2QosPtr->macDa.arEther[2] << 8);
        hwDataPtr[5] |= (stdIpL2QosPtr->macDa.arEther[1] << 16);
        hwDataPtr[5] |= (stdIpL2QosPtr->macDa.arEther[0] << 24);

        /* MAC SA */
        hwDataPtr[2] |= (stdIpL2QosPtr->macSa.arEther[5] << 16);
        hwDataPtr[2] |= (stdIpL2QosPtr->macSa.arEther[4] << 24);
        hwDataPtr[3] |=  stdIpL2QosPtr->macSa.arEther[3];
        hwDataPtr[3] |= (stdIpL2QosPtr->macSa.arEther[2] << 24);
        hwDataPtr[4] |=  stdIpL2QosPtr->macSa.arEther[1];
        hwDataPtr[4] |= (stdIpL2QosPtr->macSa.arEther[0] << 8);

        /* UDB18 */
        hwDataPtr[1] |= (stdIpL2QosPtr->udb18_19[0] << 24);
        /* UDB19 */
        hwDataPtr[3] |= (stdIpL2QosPtr->udb18_19[1] << 16);
    }

    /* standard ingress IPV4 L4 */
    if (stdIpL4Ptr     != NULL)
    {
        /* isArp */
        hwDataPtr[3] |= ((stdIpL4Ptr->isArp & 0x01) << 9);

        /* isBc */
        hwDataPtr[3] |= ((stdIpL4Ptr->isBc & 0x01) << 14);

        /* l4Byte0 */
        hwDataPtr[2] |= (stdIpL4Ptr->l4Byte0 << 16);

        /* l4Byte1 */
        hwDataPtr[2] |= (stdIpL4Ptr->l4Byte1 << 24);

        /* l4Byte13 */
        hwDataPtr[3] |=  stdIpL4Ptr->l4Byte13;

        /* SIP */
        hwDataPtr[3] |= (stdIpL4Ptr->sip.arIP[3] << 16);
        hwDataPtr[3] |= (stdIpL4Ptr->sip.arIP[2] << 24);
        hwDataPtr[4] |=  stdIpL4Ptr->sip.arIP[1];
        hwDataPtr[4] |= (stdIpL4Ptr->sip.arIP[0] << 8);

        /* DIP */
        hwDataPtr[4] |= (stdIpL4Ptr->dip.arIP[3] << 16);
        hwDataPtr[4] |= (stdIpL4Ptr->dip.arIP[2] << 24);
        hwDataPtr[5] |=  stdIpL4Ptr->dip.arIP[1];
        hwDataPtr[5] |= (stdIpL4Ptr->dip.arIP[0] << 8);

        /* UDB20 */
        hwDataPtr[1] |= (stdIpL4Ptr->udb20_22[0] << 24);
        /* UDB21 */
        hwDataPtr[5] |= (stdIpL4Ptr->udb20_22[1] << 16);
        /* UDB22 */
        hwDataPtr[5] |= (stdIpL4Ptr->udb20_22[2] << 24);
    }

    /* extended ingress Not IP and IPV4 */
    if (extNotIpv6Ptr  != NULL)
    {
        /* etherType */
        hwDataPtr[5] |= (extNotIpv6Ptr->etherType << 16);

        /* l2Encap */
        hwDataPtr[3] |= ((extNotIpv6Ptr->l2Encap & 0x01) << 9);

        /* ipv4Fragmented */
        hwDataPtr[3] |= ((extNotIpv6Ptr->ipv4Fragmented & 0x01) << 12);

        /* SIP */
        hwDataPtr[3] |= (extNotIpv6Ptr->sip.arIP[3] << 16);
        hwDataPtr[3] |= (extNotIpv6Ptr->sip.arIP[2] << 24);
        hwDataPtr[4] |=  extNotIpv6Ptr->sip.arIP[1];
        hwDataPtr[4] |= (extNotIpv6Ptr->sip.arIP[0] << 8);

        /* DIP */
        hwDataPtr[4] |= (extNotIpv6Ptr->dip.arIP[3] << 16);
        hwDataPtr[4] |= (extNotIpv6Ptr->dip.arIP[2] << 24);
        hwDataPtr[5] |=  extNotIpv6Ptr->dip.arIP[1];
        hwDataPtr[5] |= (extNotIpv6Ptr->dip.arIP[0] << 8);

        /* MAC DA */
        hwDataPtr[9]  |= (extNotIpv6Ptr->macDa.arEther[5] << 8);
        hwDataPtr[9]  |= (extNotIpv6Ptr->macDa.arEther[4] << 16);
        hwDataPtr[9]  |= (extNotIpv6Ptr->macDa.arEther[3] << 24);
        hwDataPtr[10] |=  extNotIpv6Ptr->macDa.arEther[2];
        hwDataPtr[10] |= (extNotIpv6Ptr->macDa.arEther[1] << 8);
        hwDataPtr[10] |= (extNotIpv6Ptr->macDa.arEther[0] << 16);

        /* MAC SA */
        hwDataPtr[7] |= (extNotIpv6Ptr->macSa.arEther[5] << 24);
        hwDataPtr[8] |=  extNotIpv6Ptr->macSa.arEther[4];
        hwDataPtr[8] |= (extNotIpv6Ptr->macSa.arEther[3] << 8);
        hwDataPtr[8] |= (extNotIpv6Ptr->macSa.arEther[2] << 16);
        hwDataPtr[8] |= (extNotIpv6Ptr->macSa.arEther[1] << 24);
        hwDataPtr[9] |=  extNotIpv6Ptr->macSa.arEther[0];

        /* UDB0 */
        hwDataPtr[11] |= (extNotIpv6Ptr->udb0_5[0] << 24);
        /* UDB1 */
        hwDataPtr[10] |= (extNotIpv6Ptr->udb0_5[1] << 24);
        /* UDB2 */
        hwDataPtr[11] |=  extNotIpv6Ptr->udb0_5[2];
        /* UDB3 */
        hwDataPtr[11] |= (extNotIpv6Ptr->udb0_5[3] << 8);
        /* UDB4 */
        hwDataPtr[11] |= (extNotIpv6Ptr->udb0_5[4] << 16);
        /* UDB5 */
        hwDataPtr[1]  |= (extNotIpv6Ptr->udb0_5[5] << 24);
    }

    /* extended ingress IPV6 L2 */
    if (extIpv6L2Ptr   != NULL)
    {
        /* SIP */
        hwDataPtr[3] |= (extIpv6L2Ptr->sip.arIP[15] << 16);
        hwDataPtr[3] |= (extIpv6L2Ptr->sip.arIP[14] << 24);
        hwDataPtr[4] |=  extIpv6L2Ptr->sip.arIP[13];
        hwDataPtr[4] |= (extIpv6L2Ptr->sip.arIP[12] << 8);
        hwDataPtr[4] |= (extIpv6L2Ptr->sip.arIP[11] << 16);
        hwDataPtr[4] |= (extIpv6L2Ptr->sip.arIP[10] << 24);
        hwDataPtr[5] |=  extIpv6L2Ptr->sip.arIP[9];
        hwDataPtr[5] |= (extIpv6L2Ptr->sip.arIP[8]  << 8);
        hwDataPtr[5] |= (extIpv6L2Ptr->sip.arIP[7]  << 16);
        hwDataPtr[5] |= (extIpv6L2Ptr->sip.arIP[6]  << 24);
        hwDataPtr[6] |=  extIpv6L2Ptr->sip.arIP[5];
        hwDataPtr[6] |= (extIpv6L2Ptr->sip.arIP[4]  << 8);
        hwDataPtr[6] |= (extIpv6L2Ptr->sip.arIP[3]  << 16);
        hwDataPtr[6] |= (extIpv6L2Ptr->sip.arIP[2]  << 24);
        hwDataPtr[7] |=  extIpv6L2Ptr->sip.arIP[1];
        hwDataPtr[7] |= (extIpv6L2Ptr->sip.arIP[0]  << 8);

        /* isIpv6ExtHdrExist */
        hwDataPtr[3] |= ((extIpv6L2Ptr->isIpv6ExtHdrExist & 0x01) << 9);

        /* isIpv6HopByHop */
        hwDataPtr[3] |= ((extIpv6L2Ptr->isIpv6HopByHop & 0x01) << 12);

        /* dipBits127to120 */
        hwDataPtr[7] |= (extIpv6L2Ptr->dipBits127to120 << 16);

        /* MAC DA */
        hwDataPtr[9]  |= (extIpv6L2Ptr->macDa.arEther[5] << 8);
        hwDataPtr[9]  |= (extIpv6L2Ptr->macDa.arEther[4] << 16);
        hwDataPtr[9]  |= (extIpv6L2Ptr->macDa.arEther[3] << 24);
        hwDataPtr[10] |=  extIpv6L2Ptr->macDa.arEther[2];
        hwDataPtr[10] |= (extIpv6L2Ptr->macDa.arEther[1] << 8);
        hwDataPtr[10] |= (extIpv6L2Ptr->macDa.arEther[0] << 16);

        /* MAC SA */
        hwDataPtr[7] |= (extIpv6L2Ptr->macSa.arEther[5] << 24);
        hwDataPtr[8] |=  extIpv6L2Ptr->macSa.arEther[4];
        hwDataPtr[8] |= (extIpv6L2Ptr->macSa.arEther[3] << 8);
        hwDataPtr[8] |= (extIpv6L2Ptr->macSa.arEther[2] << 16);
        hwDataPtr[8] |= (extIpv6L2Ptr->macSa.arEther[1] << 24);
        hwDataPtr[9] |=  extIpv6L2Ptr->macSa.arEther[0];

        /* UDB6 */
        hwDataPtr[10] |= (extIpv6L2Ptr->udb6_11[0] << 24);
        /* UDB7 */
        hwDataPtr[11] |=  extIpv6L2Ptr->udb6_11[1];
        /* UDB8 */
        hwDataPtr[11] |= (extIpv6L2Ptr->udb6_11[2] << 8);
        /* UDB9 */
        hwDataPtr[11] |= (extIpv6L2Ptr->udb6_11[3] << 16);
        /* UDB10 */
        hwDataPtr[11] |= (extIpv6L2Ptr->udb6_11[4] << 24);
        /* UDB11 */
        hwDataPtr[1]  |= (extIpv6L2Ptr->udb6_11[5] << 24);
    }

    /* extended ingress IPV6 L4 */
    if (extIpv6L4Ptr   != NULL)
    {
        /* SIP */
        /* sip 31:0  bits 143:112 */
        hwDataPtr[3] |= (extIpv6L4Ptr->sip.arIP[15] << 16);
        hwDataPtr[3] |= (extIpv6L4Ptr->sip.arIP[14] << 24);
        hwDataPtr[4] |=  extIpv6L4Ptr->sip.arIP[13];
        hwDataPtr[4] |= (extIpv6L4Ptr->sip.arIP[12] << 8);
        /* sip 127:32  bits 271:176 */
        hwDataPtr[5] |= (extIpv6L4Ptr->sip.arIP[11] << 16);
        hwDataPtr[5] |= (extIpv6L4Ptr->sip.arIP[10] << 24);
        hwDataPtr[6] |=  extIpv6L4Ptr->sip.arIP[9];
        hwDataPtr[6] |= (extIpv6L4Ptr->sip.arIP[8]  << 8);
        hwDataPtr[6] |= (extIpv6L4Ptr->sip.arIP[7]  << 16);
        hwDataPtr[6] |= (extIpv6L4Ptr->sip.arIP[6]  << 24);
        hwDataPtr[7] |=  extIpv6L4Ptr->sip.arIP[5];
        hwDataPtr[7] |= (extIpv6L4Ptr->sip.arIP[4]  << 8);
        hwDataPtr[7] |= (extIpv6L4Ptr->sip.arIP[3]  << 16);
        hwDataPtr[7] |= (extIpv6L4Ptr->sip.arIP[2]  << 24);
        hwDataPtr[8] |=  extIpv6L4Ptr->sip.arIP[1];
        hwDataPtr[8] |= (extIpv6L4Ptr->sip.arIP[0]  << 8);

        /* DIP */
        /* dip 31:0 */
        hwDataPtr[4]  |= (extIpv6L4Ptr->dip.arIP[15] << 16);
        hwDataPtr[4]  |= (extIpv6L4Ptr->dip.arIP[14] << 24);
        hwDataPtr[5]  |=  extIpv6L4Ptr->dip.arIP[13];
        hwDataPtr[5]  |= (extIpv6L4Ptr->dip.arIP[12] << 8);
        /* dip 127:120 */
        hwDataPtr[8]  |= (extIpv6L4Ptr->dip.arIP[0] << 16);
        /* dip 119:32 */
        hwDataPtr[8]  |= (extIpv6L4Ptr->dip.arIP[11] << 24);
        hwDataPtr[9]  |=  extIpv6L4Ptr->dip.arIP[10];
        hwDataPtr[9]  |= (extIpv6L4Ptr->dip.arIP[9] << 8);
        hwDataPtr[9]  |= (extIpv6L4Ptr->dip.arIP[8] << 16);
        hwDataPtr[9]  |= (extIpv6L4Ptr->dip.arIP[7] << 24);
        hwDataPtr[10] |=  extIpv6L4Ptr->dip.arIP[6];
        hwDataPtr[10] |= (extIpv6L4Ptr->dip.arIP[5] << 8);
        hwDataPtr[10] |= (extIpv6L4Ptr->dip.arIP[4] << 16);
        hwDataPtr[10] |= (extIpv6L4Ptr->dip.arIP[3] << 24);
        hwDataPtr[11] |=  extIpv6L4Ptr->dip.arIP[2];
        hwDataPtr[11] |= (extIpv6L4Ptr->dip.arIP[1] << 8);

        /* isIpv6ExtHdrExist */
        hwDataPtr[3] |= ((extIpv6L4Ptr->isIpv6ExtHdrExist & 0x01) << 9);

        /* isIpv6HopByHop */
        hwDataPtr[3] |= ((extIpv6L4Ptr->isIpv6HopByHop & 0x01) << 12);

        /* UDB12 */
        hwDataPtr[11] |= (extIpv6L4Ptr->udb12_14[0] << 16);
        /* UDB13 */
        hwDataPtr[11] |= (extIpv6L4Ptr->udb12_14[1] << 24);
        /* UDB14 */
        hwDataPtr[1]  |= (extIpv6L4Ptr->udb12_14[2] << 24);
    }

    if (commonUdbPtr!= NULL)
    {
        /* PCL Id*/
        hwDataPtr[0] |= (commonUdbPtr->pclId & 0x03FF);

        /* macToMe */
        hwDataPtr[0] |= ((commonUdbPtr->macToMe & 1) << 10);

        /* source port */
        hwDataPtr[0] |= ((commonUdbPtr->sourcePort & 0x3F) << 11);

        /* VID */
        hwDataPtr[0] |= ((commonUdbPtr->vid & 0x0FFF) << 18);

        /* UP */
        hwDataPtr[0] |= ((commonUdbPtr->up & 0x03) << 30);
        hwDataPtr[1] |= ((commonUdbPtr->up & 0x04) >> 2);

        /* is IP */
        hwDataPtr[1] |= ((commonUdbPtr->isIp & 0x01) << 9);

        /* is L2 valid */
        hwDataPtr[2] |= ((commonUdbPtr->isL2Valid & 0x01) << 31);

        /* is UDB Valid */
        hwDataPtr[2] |= ((commonUdbPtr->isUdbValid & 0x01) << 30);

        /* isIpv6ExtHdrExist */
        hwDataPtr[2] |= ((commonUdbPtr->isIpv6Eh & 0x01) << 28);

        /* isIpv6HopByHop */
        hwDataPtr[2] |= ((commonUdbPtr->isIpv6HopByHop & 0x01) << 27);

        /* DSCP or EXP */
        hwDataPtr[1] |= ((commonUdbPtr->dscpOrExp & 0x3F) << 10);

        /* pktTagging */
        hwDataPtr[0] |= ((commonUdbPtr->pktTagging & 0x01) << 17);
        hwDataPtr[1] |= (commonUdbPtr->pktTagging & 0x02);

        /* l3OffsetInvalid */
        hwDataPtr[1] |= ((commonUdbPtr->l3OffsetInvalid & 0x01) << 2);

        /* l4ProtocolType */
        hwDataPtr[1] |= ((commonUdbPtr->l4ProtocolType & 0x03) << 3);

        /* pktType */
        hwDataPtr[1] |= ((commonUdbPtr->pktType & 0x03) << 5);
        hwDataPtr[2] |= ((commonUdbPtr->pktType & 0xFC) << 17);

        /* ipHeaderOk */
        hwDataPtr[1] |= ((commonUdbPtr->ipHeaderOk & 0x01) << 7);

        /* macDaType */
        hwDataPtr[2] |= ((commonUdbPtr->macDaType & 0x03) << 16);

        /* l4OffsetInalid */
        hwDataPtr[2] |= ((commonUdbPtr->l4OffsetInvalid & 0x01) << 18);

        /* l2Encapsulation */
        hwDataPtr[2] |= ((commonUdbPtr->l2Encapsulation & 0x03) << 25);

    }

    if (stdUdbPtr != NULL)
    {
        /* isIpv4 */
        hwDataPtr[1] |= ((stdUdbPtr->isIpv4 & 0x01) << 8);

        /* UDB0 */
        hwDataPtr[1] |= (stdUdbPtr->udb0_15[0]  << 16);
        /* UDB1 */
        hwDataPtr[1] |= (stdUdbPtr->udb0_15[1]  << 24);
        /* UDB2 */
        hwDataPtr[2] |=  stdUdbPtr->udb0_15[2];
        /* UDB3 */
        hwDataPtr[3] |=  stdUdbPtr->udb0_15[3];
        /* UDB4 */
        hwDataPtr[3] |= (stdUdbPtr->udb0_15[4]  << 8);
        /* UDB5 */
        hwDataPtr[2] |= (stdUdbPtr->udb0_15[5]  << 8);
        /* UDB6 */
        hwDataPtr[3] |= (stdUdbPtr->udb0_15[6]  << 16);
        /* UDB7 */
        hwDataPtr[3] |= (stdUdbPtr->udb0_15[7]  << 24);
        /* UDB8 */
        hwDataPtr[4] |=  stdUdbPtr->udb0_15[8];
        /* UDB9 */
        hwDataPtr[4] |= (stdUdbPtr->udb0_15[9]  << 8);
        /* UDB10 */
        hwDataPtr[4] |= (stdUdbPtr->udb0_15[10] << 16);
        /* UDB11 */
        hwDataPtr[4] |= (stdUdbPtr->udb0_15[11] << 24);
        /* UDB12 */
        hwDataPtr[5] |=  stdUdbPtr->udb0_15[12];
        /* UDB12 */
        hwDataPtr[5] |= (stdUdbPtr->udb0_15[13] << 8);
        /* UDB14 */
        hwDataPtr[5] |= (stdUdbPtr->udb0_15[14] << 16);
        /* UDB15 */
        hwDataPtr[5] |= (stdUdbPtr->udb0_15[15] << 24);

    }

    if (extUdbPtr != NULL)
    {
        /* isIpv6 */
        hwDataPtr[1] |= ((extUdbPtr->isIpv6 & 0x01) << 8);

        /* ipProtocol */
        hwDataPtr[1] |= (extUdbPtr->ipProtocol << 16);

        /* sipBits31_0 */
        hwDataPtr[4] |= (extUdbPtr->sipBits31_0[3] << 16);
        hwDataPtr[4] |= (extUdbPtr->sipBits31_0[2] << 24);
        hwDataPtr[5] |=  extUdbPtr->sipBits31_0[1];
        hwDataPtr[5] |= (extUdbPtr->sipBits31_0[0] << 8);

        /* sipBits79_32orMacSa */
        hwDataPtr[5] |= (extUdbPtr->sipBits79_32orMacSa[5] << 16);
        hwDataPtr[5] |= (extUdbPtr->sipBits79_32orMacSa[4] << 24);
        hwDataPtr[6] |=  extUdbPtr->sipBits79_32orMacSa[3];
        hwDataPtr[6] |= (extUdbPtr->sipBits79_32orMacSa[2] << 8);
        hwDataPtr[6] |= (extUdbPtr->sipBits79_32orMacSa[1] << 16);
        hwDataPtr[6] |= (extUdbPtr->sipBits79_32orMacSa[0] << 24);

        /* sipBits127_80orMacDa */
        hwDataPtr[7] |=  extUdbPtr->sipBits127_80orMacDa[5];
        hwDataPtr[7] |= (extUdbPtr->sipBits127_80orMacDa[4] << 8);
        hwDataPtr[7] |= (extUdbPtr->sipBits127_80orMacDa[3] << 16);
        hwDataPtr[7] |= (extUdbPtr->sipBits127_80orMacDa[2] << 24);
        hwDataPtr[8] |=  extUdbPtr->sipBits127_80orMacDa[1];
        hwDataPtr[8] |= (extUdbPtr->sipBits127_80orMacDa[0] << 8);

        /* dipBits127_112 */
        hwDataPtr[8] |= (extUdbPtr->dipBits127_112[1] << 16);
        hwDataPtr[8] |= (extUdbPtr->dipBits127_112[0] << 24);

        /* dipBits31_0 */
        hwDataPtr[9] |=  extUdbPtr->dipBits31_0[3];
        hwDataPtr[9] |= (extUdbPtr->dipBits31_0[2] << 8);
        hwDataPtr[9] |= (extUdbPtr->dipBits31_0[1] << 16);
        hwDataPtr[9] |= (extUdbPtr->dipBits31_0[0] << 24);

        /* UDB0 */
        hwDataPtr[11] |= (extUdbPtr->udb0_15[0]  << 16);
        /* UDB1 */
        hwDataPtr[1]  |= (extUdbPtr->udb0_15[1]  << 24);
        /* UDB2 */
        hwDataPtr[2]  |=  extUdbPtr->udb0_15[2];
        /* UDB3 */
        hwDataPtr[3]  |=  extUdbPtr->udb0_15[3];
        /* UDB4 */
        hwDataPtr[3]  |= (extUdbPtr->udb0_15[4]  << 8);
        /* UDB5 */
        hwDataPtr[2]  |= (extUdbPtr->udb0_15[5]  << 8);
        /* UDB6 */
        hwDataPtr[3]  |= (extUdbPtr->udb0_15[6]  << 16);
        /* UDB7 */
        hwDataPtr[3]  |= (extUdbPtr->udb0_15[7]  << 24);
        /* UDB8 */
        hwDataPtr[4]  |=  extUdbPtr->udb0_15[8];
        /* UDB9 */
        hwDataPtr[4]  |= (extUdbPtr->udb0_15[9]  << 8);
        /* UDB10 */
        hwDataPtr[10] |= extUdbPtr->udb0_15[10];
        /* UDB11 */
        hwDataPtr[10] |= (extUdbPtr->udb0_15[11] << 8);
        /* UDB12 */
        hwDataPtr[10] |= (extUdbPtr->udb0_15[12] << 16);
        /* UDB13 */
        hwDataPtr[10] |= (extUdbPtr->udb0_15[13] << 24);
        /* UDB14 */
        hwDataPtr[11] |=  extUdbPtr->udb0_15[14];
        /* UDB15 */
        hwDataPtr[11] |= (extUdbPtr->udb0_15[15] << 8);
    }

    return GT_OK;
}

/**
* @internal sip5TcamRuleDataSw2HwConvert_checkSplittedFields function
* @endinternal
*
* @brief   the generic convert of STC to HW format , corrently not know to check
*         field overflow , for 'split fields'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ruleFormat               - one of supported key formats
* @param[in] swDataPtr                - action in SW format
*
* @retval GT_OK                    - on success.
* @retval other                    - on failure.
*/
static GT_STATUS sip5TcamRuleDataSw2HwConvert_checkSplittedFields
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT       *swDataPtr
)
{
    GT_U32 vrfId = 0; /* VRF ID is in the ingress rules excluding STD IPV6 DIP
                         and STD Routed ACL (usual not splitted 8 LSBs in use) */
    GT_U32 srcPortOrTrunk = 0; /* Source ePort or Trunk is in the Ultra keys */

    /* check field swDataPtr->ruleExtNotIpv6.common.sourcePort; */
    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E    :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E   :
            if(swDataPtr->ruleExtNotIpv6.common.sourcePort >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            vrfId = swDataPtr->ruleStdNotIp.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            vrfId = swDataPtr->ruleStdIpL2Qos.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            vrfId = swDataPtr->ruleStdIpv4L4.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            vrfId = swDataPtr->ruleExtNotIpv6.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            vrfId = swDataPtr->ruleExtIpv6L2.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            vrfId = swDataPtr->ruleExtIpv6L4.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            vrfId = swDataPtr->ruleIngrStdUdb.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            vrfId = swDataPtr->ruleIngrExtUdb.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            vrfId = swDataPtr->ruleExtIpv4PortVlanQos.vrfId;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            vrfId = swDataPtr->ruleUltraIpv6PortVlanQos.vrfId;
            srcPortOrTrunk = swDataPtr->ruleUltraIpv6PortVlanQos.srcPortOrTrunk;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            vrfId = swDataPtr->ruleUltraIpv6RoutedAclQos.vrfId;
            srcPortOrTrunk = swDataPtr->ruleUltraIpv6RoutedAclQos.srcPortOrTrunk;
            break;
        default:
            break;
    }

    /* VRF supports 12 bits only */
    if(vrfId >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* source ePort or trunk 13 bits only */
    if(srcPortOrTrunk >= BIT_13)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;

}


/**
* @internal tcamRuleDataSw2HwConvert_NewKeys function
* @endinternal
*
* @brief   Converts TCAM pattern or mask from SW to HW format
*         for Bobcat2; Caelum; Bobcat3; Aldrin - supports all keys.
*         for other devices: Only for next formats supported
*         CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
*         CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
*         CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,
*         CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,
*         CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
*         CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - one of 6 supported key formats
* @param[in] swDataPtr                - action in SW format
*
* @param[out] hwDataPtr                - action in HW format (3 words)
*
* @retval GT_OK                    - on success.
* @retval other                    - on failure.
*/
static GT_STATUS tcamRuleDataSw2HwConvert_NewKeys
(
    IN GT_U8                               devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT       *swDataPtr,
    OUT GT_U32                             *hwDataPtr
)
{
    GT_STATUS   rc;
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC* fmtFldPtr;   /* conversion rules array */
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC**fmtFldArrPtr;  /* pointer to array of SW convert for the keys*/
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       tmpSwFormat;/* needed for Lion2 for 'port' validity check*/
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       *tmpSwDataPtr = swDataPtr;/* pointer to the SW format */
    GT_PHYSICAL_PORT_NUM                *sourcePortPtr = NULL;/*pointer to the src port */

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        fmtFldArrPtr = fmtFldArr_sip5;
        /* do extra split fields check */
        rc = sip5TcamRuleDataSw2HwConvert_checkSplittedFields(ruleFormat,swDataPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                /* this device not supports those 6 keys */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

            case CPSS_PP_FAMILY_DXCH_LION2_E:
                fmtFldArrPtr = fmtFldArr_Lion2;
                switch (ruleFormat)
                {
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E        /*15*/  :
                        sourcePortPtr = &tmpSwFormat.ruleStdIpv4RoutedAclQos.ingressIpCommon.sourcePort;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E         /*16*/  :
                        sourcePortPtr = &tmpSwFormat.ruleExtIpv4PortVlanQos.ingressIpCommon.sourcePort;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E       /*17*/  :
                        sourcePortPtr = &tmpSwFormat.ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E      /*18*/  :
                        sourcePortPtr = &tmpSwFormat.ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourcePort;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E              /*19*/  :
                        sourcePortPtr = &tmpSwFormat.ruleEgrExtIpv4RaclVacl.egressIpCommon.sourcePort;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E            /*20*/  :
                        sourcePortPtr = &tmpSwFormat.ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourcePort;
                        break;
                    default:
                        break;
                }
                break;
            case CPSS_PP_FAMILY_DXCH_LION_E:
            case CPSS_PP_FAMILY_DXCH_XCAT_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                fmtFldArrPtr = fmtFldArr_xcat;
                break;
            case CPSS_PP_FAMILY_CHEETAH3_E:
                fmtFldArrPtr = fmtFldArr_ch3;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* needed for Lion2 for 'srcPort' validity check*/
        if(sourcePortPtr)
        {
            cpssOsMemCpy(&tmpSwFormat,swDataPtr,sizeof(tmpSwFormat));
            tmpSwDataPtr = &tmpSwFormat;

            /* bit 6 is not supported by the HW , but for multi-port
               group device :
                1. for ingress - we can support by splitting the ports between the port groups
                2. for egress  - we can use the 'dual device id' with the <srcDevOrTrunkId>
            */
            (*sourcePortPtr) &= ~(BIT_6);
        }
    }

    fmtFldPtr = fmtFldArrPtr[ruleFormat];
    if(fmtFldPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* clean HW key */
    cpssOsMemSet(
        hwDataPtr, 0,
        (CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS * sizeof(GT_U32)));

    return cpssFormatConvertStcToRaw(
        fmtFldPtr, (GT_VOID*)tmpSwDataPtr, NULL, hwDataPtr);
}


/**
* @internal prvCpssDxChPclTcamRuleDataSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM pattern or mask from SW to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - one of 13 supported key formats
* @param[in] swDataPtr                - rule pattern or mask in SW format
*
* @param[out] hwDataPtr                - rule pattern or mask in HW format (6 or 12 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataSw2HwConvert
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT      *swDataPtr,
    OUT GT_U32                             *hwDataPtr
)
{
    GT_U32  size;   /* size of heData in words    */
    GT_U32  i;      /* loop index                 */
    GT_U32  bitPos; /* position of bit in the key */
    GT_BOOL hasUDB; /* GT_TRUE - the key has UDBs */

    /* format depended pointers into swDataPtr */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC            *commonPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC        *commonExtPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC     *commonStdIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC        *stdNotIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC     *stdIpL2QosPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC       *stdIpL4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC      *stdIpv6DipPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC      *extNotIpv6Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC       *extIpv6L2Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC       *extIpv6L4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC        *commonEgrPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC    *commonEgrExtPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC *commonEgrStdIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC    *stdEgrNotIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC *stdEgrIpL2QosPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC   *stdEgrIpL4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC  *extEgrNotIpv6Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC   *extEgrIpv6L2Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC   *extEgrIpv6L4Ptr;

    if(ruleFormat >= CPSS_DXCH_PCL_RULE_FORMAT_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        return tcamRuleDataSw2HwConvert_NewKeys(devNum,ruleFormat,swDataPtr,hwDataPtr);
    }

    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
             return tcamRuleDataSw2HwConvert_NewKeys(devNum,ruleFormat,swDataPtr,hwDataPtr);
        default:
            break;
    }

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above ingress keys special conversion */
        switch (ruleFormat)
        {
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                return prvCpssDxChXCatIngressPclTcamRuleDataSw2HwConvert(
                    ruleFormat, swDataPtr, hwDataPtr);
            default: break;
        }
    }

    /* clear all format depended pointers    */
    /* only relevant will get not null value */
    commonPtr         = NULL;
    commonExtPtr      = NULL;
    commonStdIpPtr    = NULL;
    stdNotIpPtr       = NULL;
    stdIpL2QosPtr     = NULL;
    stdIpL4Ptr        = NULL;
    stdIpv6DipPtr     = NULL;
    extNotIpv6Ptr     = NULL;
    extIpv6L2Ptr      = NULL;
    extIpv6L4Ptr      = NULL;
    commonEgrPtr      = NULL;
    commonEgrExtPtr   = NULL;
    commonEgrStdIpPtr = NULL;
    stdEgrNotIpPtr    = NULL;
    stdEgrIpL2QosPtr  = NULL;
    stdEgrIpL4Ptr     = NULL;
    extEgrNotIpv6Ptr  = NULL;
    extEgrIpv6L2Ptr   = NULL;
    extEgrIpv6L4Ptr   = NULL;

    hasUDB = GT_FALSE;

    switch (ruleFormat)
    {
        /* ingress formats */
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            stdNotIpPtr    = &(swDataPtr->ruleStdNotIp);
            commonPtr      = &(stdNotIpPtr->common);
            size           = 6;
            hasUDB         = GT_TRUE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            stdIpL2QosPtr  = &(swDataPtr->ruleStdIpL2Qos);
            commonPtr      = &(stdIpL2QosPtr->common);
            commonStdIpPtr = &(stdIpL2QosPtr->commonStdIp);
            size           = 6;
            hasUDB         = GT_TRUE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            stdIpL4Ptr     = &(swDataPtr->ruleStdIpv4L4);
            commonPtr      = &(stdIpL4Ptr->common);
            commonStdIpPtr = &(stdIpL4Ptr->commonStdIp);
            size           = 6;
            hasUDB         = GT_TRUE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            stdIpv6DipPtr  = &(swDataPtr->ruleStdIpv6Dip);
            commonPtr      = &(stdIpv6DipPtr->common);
            commonStdIpPtr = &(stdIpv6DipPtr->commonStdIp);
            size           = 6;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            extNotIpv6Ptr  = &(swDataPtr->ruleExtNotIpv6);
            commonPtr      = &(extNotIpv6Ptr->common);
            commonExtPtr   = &(extNotIpv6Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_TRUE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            extIpv6L2Ptr   = &(swDataPtr->ruleExtIpv6L2);
            commonPtr      = &(extIpv6L2Ptr->common);
            commonExtPtr   = &(extIpv6L2Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_TRUE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            extIpv6L4Ptr   = &(swDataPtr->ruleExtIpv6L4);
            commonPtr      = &(extIpv6L4Ptr->common);
            commonExtPtr   = &(extIpv6L4Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_TRUE;
            break;
        /* egress formats */
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            stdEgrNotIpPtr    = &(swDataPtr->ruleEgrStdNotIp);
            commonEgrPtr      = &(stdEgrNotIpPtr->common);
            size           = 6;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            stdEgrIpL2QosPtr  = &(swDataPtr->ruleEgrStdIpL2Qos);
            commonEgrPtr      = &(stdEgrIpL2QosPtr->common);
            commonEgrStdIpPtr = &(stdEgrIpL2QosPtr->commonStdIp);
            size           = 6;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            stdEgrIpL4Ptr     = &(swDataPtr->ruleEgrStdIpv4L4);
            commonEgrPtr      = &(stdEgrIpL4Ptr->common);
            commonEgrStdIpPtr = &(stdEgrIpL4Ptr->commonStdIp);
            size           = 6;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            extEgrNotIpv6Ptr  = &(swDataPtr->ruleEgrExtNotIpv6);
            commonEgrPtr      = &(extEgrNotIpv6Ptr->common);
            commonEgrExtPtr   = &(extEgrNotIpv6Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            extEgrIpv6L2Ptr   = &(swDataPtr->ruleEgrExtIpv6L2);
            commonEgrPtr      = &(extEgrIpv6L2Ptr->common);
            commonEgrExtPtr   = &(extEgrIpv6L2Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_FALSE;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            extEgrIpv6L4Ptr   = &(swDataPtr->ruleEgrExtIpv6L4);
            commonEgrPtr      = &(extEgrIpv6L4Ptr->common);
            commonEgrExtPtr   = &(extEgrIpv6L4Ptr->commonExt);
            size           = 12;
            hasUDB         = GT_FALSE;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* clear the hwData before passing data from SW */
    for (i = 0; (i < size); i++)
    {
        hwDataPtr[i] = 0;
    }

    /* common for ingress formats */
    if (commonPtr      != NULL)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            <= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            /* CH and CH2 */
            /* PCL Id*/
            hwDataPtr[0] |= ((commonPtr->pclId & 0x03FF) << 1);
        }
        else
        {
            /* CH3 and above */
            /* PCL Id*/
            hwDataPtr[0] |= (commonPtr->pclId & 0x03FF);
            hwDataPtr[0] |= ((commonPtr->macToMe & 1) << 10);
        }

        /* source port */
        hwDataPtr[0] |= ((commonPtr->sourcePort & 0x3F) << 11);

        /* isTagged */
        hwDataPtr[0] |= ((commonPtr->isTagged & 0x01) << 17);

        /* VID */
        hwDataPtr[0] |= ((commonPtr->vid & 0x0FFF) << 18);

        /* UP */
        hwDataPtr[0] |= ((commonPtr->up & 0x03) << 30);
        hwDataPtr[1] |= ((commonPtr->up & 0x04) >> 2);

        /* Qos Profile */
        hwDataPtr[1] |= ((commonPtr->qosProfile & 0x7F) << 1);

        /* is IP */
        hwDataPtr[1] |= ((commonPtr->isIp & 0x01) << 9);

        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            /* is L2 valid. Supported by CH2 and above */
            bitPos = (size <= 6) ? 189 : 381;
            hwDataPtr[bitPos / 32] |=
                ((commonPtr->isL2Valid & 0x01) << (bitPos % 32));
        }

        /* is UDB Valid */
        if (hasUDB != GT_FALSE)
        {
            bitPos = (size <= 6) ? 188 : 382;
            hwDataPtr[bitPos / 32] |=
                ((commonPtr->isUdbValid & 0x01) << (bitPos % 32));
        }

    }

    /* common for egress formats */
    if (commonEgrPtr      != NULL)
    {
        /* valid */
        hwDataPtr[0] |= (commonEgrPtr->valid & 0x01);

        /* PCL Id*/
        hwDataPtr[0] |= ((commonEgrPtr->pclId & 0x03FF) << 1);

        /* source port */
        hwDataPtr[0] |= ((commonEgrPtr->sourcePort & 0x3F) << 11);

        /* isTagged */
        hwDataPtr[0] |= ((commonEgrPtr->isTagged & 0x01) << 17);

        /* VID */
        hwDataPtr[0] |= ((commonEgrPtr->vid & 0x0FFF) << 18);

        /* UP */
        hwDataPtr[0] |= ((commonEgrPtr->up & 0x03) << 30);
        hwDataPtr[1] |= ((commonEgrPtr->up & 0x04) >> 2);

        /* is IP */
        hwDataPtr[1] |= ((commonEgrPtr->isIp & 0x01) << 9);

        /* is L2 valid */
        bitPos = (size <= 6) ? 90 : 370;
        hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->isL2Valid & 0x01) << (bitPos % 32));

        /* egress Packet Type */
        bitPos = (size <= 6) ? 188 : 381;
        hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->egrPacketType & 0x03) << (bitPos % 32));

        /* packet type dependend data */

        /* packet to CPU */
                /* cpuCode bits6-0 and the bit7 */
                hwDataPtr[1] |=
            ((commonEgrPtr->toCpu.cpuCode & 0x7F) << 1);
                bitPos = (size <= 6) ? 89 : 369;
                hwDataPtr[bitPos / 32] |=
            (((commonEgrPtr->toCpu.cpuCode >> 7) & 0x01)
                     << (bitPos % 32));

                /* SrcTrg */
                bitPos = (size <= 6) ? 81 : 361;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->toCpu.srcTrg & 0x01)
                     << (bitPos % 32));

        /* packet from CPU */
                /* traffic class */
                hwDataPtr[1] |=
            ((commonEgrPtr->fromCpu.tc & 0x07) << 3);

                /* drop priority */
                hwDataPtr[1] |=
            ((commonEgrPtr->fromCpu.dp & 0x03) << 1);

                /* egress filtering enable */
                bitPos = (size <= 6) ? 80 : 360;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->fromCpu.egrFilterEnable & 0x01)
                     << (bitPos % 32));

        /* packet to ANALYZER */
                /* to Rx sniffer */
                bitPos = (size <= 6) ? 81 : 361;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->toAnalyzer.rxSniff & 0x01)
                     << (bitPos % 32));

        /* forward DATA packet */
                /* Qos Profile */
                hwDataPtr[1] |=
            ((commonEgrPtr->fwdData.qosProfile & 0x7F) << 1);

                /* source is trunk Id */
                bitPos = (size <= 6) ? 75 : 355;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->fwdData.srcTrunkId & 0x7F)
                     << (bitPos % 32));

                /* source is trunk */
                bitPos = (size <= 6) ? 82 : 362;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->fwdData.srcIsTrunk & 0x01)
                     << (bitPos % 32));

                /* MAC DA is Unknown */
                bitPos = (size <= 6) ? 89 : 369;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->fwdData.isUnknown & 0x01)
                     << (bitPos % 32));

                /* is Routed */
                bitPos = (size <= 6) ? 88 : 368;
                hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->fwdData.isRouted & 0x01)
                     << (bitPos % 32));


        /* source Device (if not forwarding from trunk) */
        bitPos = (size <= 6) ? 75 : 355;
        hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->srcHwDev & 0x1F) << (bitPos % 32));

        /* source Id */
        bitPos = (size <= 6) ? 83 : 363;
        hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->sourceId & 0x1F) << (bitPos % 32));

        /* is Vidx */
        bitPos = (size <= 6) ? 191 : 383;
        hwDataPtr[bitPos / 32] |=
            ((commonEgrPtr->isVidx & 0x01) << (bitPos % 32));
    }

    /* common for all extended ingress formats */
    if (commonExtPtr   != NULL)
    {
        /* is IPV6 */
        hwDataPtr[1] |= ((commonExtPtr->isIpv6 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonExtPtr->ipProtocol & 0x00FF) << 10);

        /* DSCP */
        hwDataPtr[1] |= ((commonExtPtr->dscp & 0x3F) << 18);

        /* isL4Valid */
        hwDataPtr[1] |= ((commonExtPtr->isL4Valid & 0x01) << 24);

        /* l4Byte3 */
        hwDataPtr[1] |= ((commonExtPtr->l4Byte3 & 0x007F) << 25);
        hwDataPtr[2] |= ((commonExtPtr->l4Byte3 & 0x0080) >> 7);

        /* l4Byte2 */
        hwDataPtr[2] |= ((commonExtPtr->l4Byte2 & 0x00FF) << 1);

        /* l4Byte13 */
        hwDataPtr[2] |= ((commonExtPtr->l4Byte13 & 0x00FF) << 9);

        /* l4Byte1 */
        hwDataPtr[2] |= ((commonExtPtr->l4Byte1 & 0x00FF) << 17);

        /* l4Byte0 */
        hwDataPtr[2] |= ((commonExtPtr->l4Byte0 & 0x007F) << 25);
        hwDataPtr[3] |= ((commonExtPtr->l4Byte0 & 0x0080) >> 7);

        /* ipHeaderOk */
        hwDataPtr[11] |= ((commonExtPtr->ipHeaderOk & 0x01) << 31);
    }

    /* common for all extended egress formats */
    if (commonEgrExtPtr   != NULL)
    {
        /* is IPV6 */
        hwDataPtr[1] |= ((commonEgrExtPtr->isIpv6 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonEgrExtPtr->ipProtocol & 0x00FF) << 10);

        /* DSCP */
        hwDataPtr[1] |= ((commonEgrExtPtr->dscp & 0x3F) << 18);

        /* isL4Valid */
        hwDataPtr[1] |= ((commonEgrExtPtr->isL4Valid & 0x01) << 24);

        /* l4Byte3 */
        hwDataPtr[1] |= ((commonEgrExtPtr->l4Byte3 & 0x007F) << 25);
        hwDataPtr[2] |= ((commonEgrExtPtr->l4Byte3 & 0x0080) >> 7);

        /* l4Byte2 */
        hwDataPtr[2] |= ((commonEgrExtPtr->l4Byte2 & 0x00FF) << 1);

        /* l4Byte13 */
        hwDataPtr[2] |= ((commonEgrExtPtr->l4Byte13 & 0x00FF) << 9);

        /* l4Byte1 */
        hwDataPtr[2] |= ((commonEgrExtPtr->l4Byte1 & 0x00FF) << 17);

        /* l4Byte0 */
        hwDataPtr[2] |= ((commonEgrExtPtr->l4Byte0 & 0x007F) << 25);
        hwDataPtr[3] |= ((commonEgrExtPtr->l4Byte0 & 0x0080) >> 7);

        /* egrTcpUdpPortComparator */
        hwDataPtr[11] |=
            ((commonEgrExtPtr->egrTcpUdpPortComparator & 0xFF) << 19);
    }

    /* common for all standard ingress IP formats */
    if (commonStdIpPtr != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((commonStdIpPtr->isIpv4 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonStdIpPtr->ipProtocol & 0x00FF) << 10);

        /* DSCP */
        hwDataPtr[1] |= ((commonStdIpPtr->dscp & 0x3F) << 18);

        /* isL4Valid */
        hwDataPtr[1] |= ((commonStdIpPtr->isL4Valid & 0x01) << 24);

        /* l4Byte3 */
        hwDataPtr[1] |= ((commonStdIpPtr->l4Byte3 & 0x007F) << 25);
        hwDataPtr[2] |= ((commonStdIpPtr->l4Byte3 & 0x0080) >> 7);

        /* l4Byte2 */
        hwDataPtr[2] |= ((commonStdIpPtr->l4Byte2 & 0x00FF) << 1);

        /* ipv4Fragmented */
        hwDataPtr[5] |= ((commonStdIpPtr->ipv4Fragmented & 0x01) << 30);

        /* ipHeaderOk */
        hwDataPtr[5] |= ((commonStdIpPtr->ipHeaderOk & 0x01) << 31);
    }

    /* common for all standard egress IP formats */
    if (commonEgrStdIpPtr != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((commonEgrStdIpPtr->isIpv4 & 0x01) << 8);

        /* ip Protocol */
        hwDataPtr[1] |= ((commonEgrStdIpPtr->ipProtocol & 0x00FF) << 10);

        /* DSCP */
        hwDataPtr[1] |= ((commonEgrStdIpPtr->dscp & 0x3F) << 18);

        /* isL4Valid */
        hwDataPtr[1] |= ((commonEgrStdIpPtr->isL4Valid & 0x01) << 24);

        /* l4Byte3 */
        hwDataPtr[1] |= ((commonEgrStdIpPtr->l4Byte3 & 0x007F) << 25);
        hwDataPtr[2] |= ((commonEgrStdIpPtr->l4Byte3 & 0x0080) >> 7);

        /* l4Byte2 */
        hwDataPtr[2] |= ((commonEgrStdIpPtr->l4Byte2 & 0x00FF) << 1);

        /* ipv4Fragmented */
        hwDataPtr[5] |= ((commonEgrStdIpPtr->ipv4Fragmented & 0x01) << 30);

        /* egrTcpUdpPortComparator */
        hwDataPtr[2] |=
            ((commonEgrStdIpPtr->egrTcpUdpPortComparator & 0x1F) << 27);
        hwDataPtr[3] |=
            ((commonEgrStdIpPtr->egrTcpUdpPortComparator & 0xE0) >> 5);
    }

    /* standard ingress not IP */
    if (stdNotIpPtr    != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((stdNotIpPtr->isIpv4 & 0x01) << 8);

        /* etherType */
        hwDataPtr[1] |= ((stdNotIpPtr->etherType & 0xFFFF) << 11);

        /* isArp */
        hwDataPtr[2] |= ((stdNotIpPtr->isArp & 0x01) << 9);

        /* l2Encap */
        hwDataPtr[2] |= ((stdNotIpPtr->l2Encap & 0x01) << 27);

        /* MAC DA */
        hwDataPtr[4] |= ((stdNotIpPtr->macDa.arEther[5] & 0xFF) << 12);
        hwDataPtr[4] |= ((stdNotIpPtr->macDa.arEther[4] & 0xFF) << 20);
        hwDataPtr[4] |= ((stdNotIpPtr->macDa.arEther[3] & 0x0F) << 28);
        hwDataPtr[5] |= ((stdNotIpPtr->macDa.arEther[3] & 0xF0) >> 4);
        hwDataPtr[5] |= ((stdNotIpPtr->macDa.arEther[2] & 0xFF) << 4);
        hwDataPtr[5] |= ((stdNotIpPtr->macDa.arEther[1] & 0xFF) << 12);
        hwDataPtr[5] |= ((stdNotIpPtr->macDa.arEther[0] & 0xFF) << 20);

        /* MAC SA */
        hwDataPtr[2] |= ((stdNotIpPtr->macSa.arEther[5] & 0x0F) << 28);
        hwDataPtr[3] |= ((stdNotIpPtr->macSa.arEther[5] & 0xF0) >> 4);
        hwDataPtr[3] |= ((stdNotIpPtr->macSa.arEther[4] & 0xFF) << 4);
        hwDataPtr[3] |= ((stdNotIpPtr->macSa.arEther[3] & 0xFF) << 12);
        hwDataPtr[3] |= ((stdNotIpPtr->macSa.arEther[2] & 0xFF) << 20);
        hwDataPtr[3] |= ((stdNotIpPtr->macSa.arEther[1] & 0x0F) << 28);
        hwDataPtr[4] |= ((stdNotIpPtr->macSa.arEther[1] & 0xF0) >> 4);
        hwDataPtr[4] |= ((stdNotIpPtr->macSa.arEther[0] & 0xFF) << 4);

        /* UDB */
        hwDataPtr[2] |= ((stdNotIpPtr->udb15_17[0] & 0xFF) << 11);
        hwDataPtr[2] |= ((stdNotIpPtr->udb15_17[1] & 0xFF) << 19);
        hwDataPtr[1] |= ((stdNotIpPtr->udb15_17[2] & 0x1F) << 27);
        hwDataPtr[2] |= ((stdNotIpPtr->udb15_17[2] & 0xE0) >> 5);
    }

    /* standard egress not IP */
    if (stdEgrNotIpPtr    != NULL)
    {
        /* is IPV4 */
        hwDataPtr[1] |= ((stdEgrNotIpPtr->isIpv4 & 0x01) << 8);

        /* etherType */
        hwDataPtr[1] |= ((stdEgrNotIpPtr->etherType & 0xFFFF) << 11);

        /* isArp */
        hwDataPtr[2] |= ((stdEgrNotIpPtr->isArp & 0x01) << 9);

        /* l2Encap */
        hwDataPtr[2] |= ((stdEgrNotIpPtr->l2Encap & 0x01) << 27);

        /* MAC DA */
        hwDataPtr[4] |= ((stdEgrNotIpPtr->macDa.arEther[5] & 0xFF) << 12);
        hwDataPtr[4] |= ((stdEgrNotIpPtr->macDa.arEther[4] & 0xFF) << 20);
        hwDataPtr[4] |= ((stdEgrNotIpPtr->macDa.arEther[3] & 0x0F) << 28);
        hwDataPtr[5] |= ((stdEgrNotIpPtr->macDa.arEther[3] & 0xF0) >> 4);
        hwDataPtr[5] |= ((stdEgrNotIpPtr->macDa.arEther[2] & 0xFF) << 4);
        hwDataPtr[5] |= ((stdEgrNotIpPtr->macDa.arEther[1] & 0xFF) << 12);
        hwDataPtr[5] |= ((stdEgrNotIpPtr->macDa.arEther[0] & 0xFF) << 20);

        /* MAC SA */
        hwDataPtr[2] |= ((stdEgrNotIpPtr->macSa.arEther[5] & 0x0F) << 28);
        hwDataPtr[3] |= ((stdEgrNotIpPtr->macSa.arEther[5] & 0xF0) >> 4);
        hwDataPtr[3] |= ((stdEgrNotIpPtr->macSa.arEther[4] & 0xFF) << 4);
        hwDataPtr[3] |= ((stdEgrNotIpPtr->macSa.arEther[3] & 0xFF) << 12);
        hwDataPtr[3] |= ((stdEgrNotIpPtr->macSa.arEther[2] & 0xFF) << 20);
        hwDataPtr[3] |= ((stdEgrNotIpPtr->macSa.arEther[1] & 0x0F) << 28);
        hwDataPtr[4] |= ((stdEgrNotIpPtr->macSa.arEther[1] & 0xF0) >> 4);
        hwDataPtr[4] |= ((stdEgrNotIpPtr->macSa.arEther[0] & 0xFF) << 4);
    }

    /* standard ingress IPV4 and IPV6 L2 Qos */
    if (stdIpL2QosPtr  != NULL)
    {
        /* isArp */
        hwDataPtr[2] |= ((stdIpL2QosPtr->isArp & 0x01) << 9);

        /* isIpv6ExtHdrExist */
        hwDataPtr[2] |= ((stdIpL2QosPtr->isIpv6ExtHdrExist & 0x01) << 10);

        /* isIpv6HopByHop */
        hwDataPtr[2] |= ((stdIpL2QosPtr->isIpv6HopByHop & 0x01) << 27);

        /* MAC DA */
        hwDataPtr[4] |= ((stdIpL2QosPtr->macDa.arEther[5] & 0xFF) << 12);
        hwDataPtr[4] |= ((stdIpL2QosPtr->macDa.arEther[4] & 0xFF) << 20);
        hwDataPtr[4] |= ((stdIpL2QosPtr->macDa.arEther[3] & 0x0F) << 28);
        hwDataPtr[5] |= ((stdIpL2QosPtr->macDa.arEther[3] & 0xF0) >> 4);
        hwDataPtr[5] |= ((stdIpL2QosPtr->macDa.arEther[2] & 0xFF) << 4);
        hwDataPtr[5] |= ((stdIpL2QosPtr->macDa.arEther[1] & 0xFF) << 12);
        hwDataPtr[5] |= ((stdIpL2QosPtr->macDa.arEther[0] & 0xFF) << 20);

        /* MAC SA */
        hwDataPtr[2] |= ((stdIpL2QosPtr->macSa.arEther[5] & 0x0F) << 28);
        hwDataPtr[3] |= ((stdIpL2QosPtr->macSa.arEther[5] & 0xF0) >> 4);
        hwDataPtr[3] |= ((stdIpL2QosPtr->macSa.arEther[4] & 0xFF) << 4);
        hwDataPtr[3] |= ((stdIpL2QosPtr->macSa.arEther[3] & 0xFF) << 12);
        hwDataPtr[3] |= ((stdIpL2QosPtr->macSa.arEther[2] & 0xFF) << 20);
        hwDataPtr[3] |= ((stdIpL2QosPtr->macSa.arEther[1] & 0x0F) << 28);
        hwDataPtr[4] |= ((stdIpL2QosPtr->macSa.arEther[1] & 0xF0) >> 4);
        hwDataPtr[4] |= ((stdIpL2QosPtr->macSa.arEther[0] & 0xFF) << 4);

        /* UDB */
        hwDataPtr[2] |= ((stdIpL2QosPtr->udb18_19[0] & 0xFF) << 11);
        hwDataPtr[2] |= ((stdIpL2QosPtr->udb18_19[1] & 0xFF) << 19);
    }

    /* standard egress IPV4 and IPV6 L2 Qos */
    if (stdEgrIpL2QosPtr  != NULL)
    {
        /* isArp */
        hwDataPtr[2] |= ((stdEgrIpL2QosPtr->isArp & 0x01) << 9);

        /* dipBits0to31 */
        hwDataPtr[3] |= ((stdEgrIpL2QosPtr->dipBits0to31[3] & 0xFF) <<  3);
        hwDataPtr[3] |= ((stdEgrIpL2QosPtr->dipBits0to31[2] & 0xFF) << 11);
        hwDataPtr[3] |= ((stdEgrIpL2QosPtr->dipBits0to31[1] & 0xFF) << 19);
        hwDataPtr[3] |= ((stdEgrIpL2QosPtr->dipBits0to31[0] & 0x1F) << 27);
        hwDataPtr[4] |= ((stdEgrIpL2QosPtr->dipBits0to31[0] & 0xE0) >>  5);

        /* l4Byte13 */
        hwDataPtr[4] |= ((stdEgrIpL2QosPtr->l4Byte13 & 0xFF) <<  3);

        /* MAC DA */
        hwDataPtr[4] |= ((stdEgrIpL2QosPtr->macDa.arEther[5] & 0xFF) << 12);
        hwDataPtr[4] |= ((stdEgrIpL2QosPtr->macDa.arEther[4] & 0xFF) << 20);
        hwDataPtr[4] |= ((stdEgrIpL2QosPtr->macDa.arEther[3] & 0x0F) << 28);
        hwDataPtr[5] |= ((stdEgrIpL2QosPtr->macDa.arEther[3] & 0xF0) >> 4);
        hwDataPtr[5] |= ((stdEgrIpL2QosPtr->macDa.arEther[2] & 0xFF) << 4);
        hwDataPtr[5] |= ((stdEgrIpL2QosPtr->macDa.arEther[1] & 0xFF) << 12);
        hwDataPtr[5] |= ((stdEgrIpL2QosPtr->macDa.arEther[0] & 0xFF) << 20);
    }

    /* standard ingress IPV4 L4 */
    if (stdIpL4Ptr     != NULL)
    {
        /* isArp */
        hwDataPtr[2] |= ((stdIpL4Ptr->isArp & 0x01) << 9);

        /* isBc */
        hwDataPtr[2] |= ((stdIpL4Ptr->isBc & 0x01) << 10);

        /* l4Byte13 */
        hwDataPtr[5] |= ((stdIpL4Ptr->l4Byte13 & 0xFF) << 3);

        /* l4Byte1 */
        hwDataPtr[5] |= ((stdIpL4Ptr->l4Byte1 & 0xFF) << 11);

        /* l4Byte0 */
        hwDataPtr[5] |= ((stdIpL4Ptr->l4Byte0 & 0xFF) << 19);

        /* SIP */
        hwDataPtr[3] |= ((stdIpL4Ptr->sip.arIP[3] & 0xFF) << 3);
        hwDataPtr[3] |= ((stdIpL4Ptr->sip.arIP[2] & 0xFF) << 11);
        hwDataPtr[3] |= ((stdIpL4Ptr->sip.arIP[1] & 0xFF) << 19);
        hwDataPtr[3] |= ((stdIpL4Ptr->sip.arIP[0] & 0x1F) << 27);
        hwDataPtr[4] |= ((stdIpL4Ptr->sip.arIP[0] & 0xE0) >> 5);

        /* DIP */
        hwDataPtr[4] |= ((stdIpL4Ptr->dip.arIP[3] & 0xFF) << 3);
        hwDataPtr[4] |= ((stdIpL4Ptr->dip.arIP[2] & 0xFF) << 11);
        hwDataPtr[4] |= ((stdIpL4Ptr->dip.arIP[1] & 0xFF) << 19);
        hwDataPtr[4] |= ((stdIpL4Ptr->dip.arIP[0] & 0x1F) << 27);
        hwDataPtr[5] |= ((stdIpL4Ptr->dip.arIP[0] & 0xE0) >> 5);

        /* UDB */
        hwDataPtr[2] |= ((stdIpL4Ptr->udb20_22[0] & 0xFF) << 11);
        hwDataPtr[2] |= ((stdIpL4Ptr->udb20_22[1] & 0xFF) << 19);
        hwDataPtr[2] |= ((stdIpL4Ptr->udb20_22[2] & 0x1F) << 27);
        hwDataPtr[3] |= ((stdIpL4Ptr->udb20_22[2] & 0xE0) >> 5);
    }

    /* standard egress IPV4 L4 */
    if (stdEgrIpL4Ptr     != NULL)
    {
        /* isArp */
        hwDataPtr[2] |= ((stdEgrIpL4Ptr->isArp & 0x01) << 9);

        /* isBc */
        hwDataPtr[2] |= ((stdEgrIpL4Ptr->isBc & 0x01) << 10);

        /* l4Byte13 */
        hwDataPtr[5] |= ((stdEgrIpL4Ptr->l4Byte13 & 0xFF) << 3);

        /* l4Byte1 */
        hwDataPtr[5] |= ((stdEgrIpL4Ptr->l4Byte1 & 0xFF) << 11);

        /* l4Byte0 */
        hwDataPtr[5] |= ((stdEgrIpL4Ptr->l4Byte0 & 0xFF) << 19);

        /* SIP */
        hwDataPtr[3] |= ((stdEgrIpL4Ptr->sip.arIP[3] & 0xFF) << 3);
        hwDataPtr[3] |= ((stdEgrIpL4Ptr->sip.arIP[2] & 0xFF) << 11);
        hwDataPtr[3] |= ((stdEgrIpL4Ptr->sip.arIP[1] & 0xFF) << 19);
        hwDataPtr[3] |= ((stdEgrIpL4Ptr->sip.arIP[0] & 0x1F) << 27);
        hwDataPtr[4] |= ((stdEgrIpL4Ptr->sip.arIP[0] & 0xE0) >> 5);

        /* DIP */
        hwDataPtr[4] |= ((stdEgrIpL4Ptr->dip.arIP[3] & 0xFF) << 3);
        hwDataPtr[4] |= ((stdEgrIpL4Ptr->dip.arIP[2] & 0xFF) << 11);
        hwDataPtr[4] |= ((stdEgrIpL4Ptr->dip.arIP[1] & 0xFF) << 19);
        hwDataPtr[4] |= ((stdEgrIpL4Ptr->dip.arIP[0] & 0x1F) << 27);
        hwDataPtr[5] |= ((stdEgrIpL4Ptr->dip.arIP[0] & 0xE0) >> 5);
    }

    /* standard ingress IPV6 DIP */
    if (stdIpv6DipPtr  != NULL)
    {
        /* isArp */
        hwDataPtr[2] |= ((stdIpv6DipPtr->isArp & 0x01) << 9);

        /* isIpv6ExtHdrExist */
        hwDataPtr[2] |= ((stdIpv6DipPtr->isIpv6ExtHdrExist & 0x01) << 10);

        /* isIpv6HopByHop */
        hwDataPtr[2] |= ((stdIpv6DipPtr->isIpv6HopByHop & 0x01) << 27);

        /* DIP */
        /* dip 15:0 */
        hwDataPtr[1] |= ((stdIpv6DipPtr->dip.arIP[15] & 0x7F) << 25);
        hwDataPtr[2] |= ((stdIpv6DipPtr->dip.arIP[15] & 0x80) >> 7);
        hwDataPtr[2] |= ((stdIpv6DipPtr->dip.arIP[14] & 0xFF) << 1);
        /* dip 31:16 */
        hwDataPtr[2] |= ((stdIpv6DipPtr->dip.arIP[13] & 0xFF) << 11);
        hwDataPtr[2] |= ((stdIpv6DipPtr->dip.arIP[12] & 0xFF) << 19);
        /* dip 127:32 */
        hwDataPtr[2] |= ((stdIpv6DipPtr->dip.arIP[11] & 0x0F) << 28);
        hwDataPtr[3] |= ((stdIpv6DipPtr->dip.arIP[11] & 0xF0) >> 4);
        hwDataPtr[3] |= ((stdIpv6DipPtr->dip.arIP[10] & 0xFF) << 4);
        hwDataPtr[3] |= ((stdIpv6DipPtr->dip.arIP[9] & 0xFF) << 12);
        hwDataPtr[3] |= ((stdIpv6DipPtr->dip.arIP[8] & 0xFF) << 20);
        hwDataPtr[3] |= ((stdIpv6DipPtr->dip.arIP[7] & 0x0F) << 28);
        hwDataPtr[4] |= ((stdIpv6DipPtr->dip.arIP[7] & 0xF0) >> 4);
        hwDataPtr[4] |= ((stdIpv6DipPtr->dip.arIP[6] & 0xFF) << 4);
        hwDataPtr[4] |= ((stdIpv6DipPtr->dip.arIP[5] & 0xFF) << 12);
        hwDataPtr[4] |= ((stdIpv6DipPtr->dip.arIP[4] & 0xFF) << 20);
        hwDataPtr[4] |= ((stdIpv6DipPtr->dip.arIP[3] & 0x0F) << 28);
        hwDataPtr[5] |= ((stdIpv6DipPtr->dip.arIP[3] & 0xF0) >> 4);
        hwDataPtr[5] |= ((stdIpv6DipPtr->dip.arIP[2] & 0xFF) << 4);
        hwDataPtr[5] |= ((stdIpv6DipPtr->dip.arIP[1] & 0xFF) << 12);
        hwDataPtr[5] |= ((stdIpv6DipPtr->dip.arIP[0] & 0xFF) << 20);

    }

    /* extended ingress Not IP and IPV4 */
    if (extNotIpv6Ptr  != NULL)
    {
        /* etherType */
        hwDataPtr[5] |= ((extNotIpv6Ptr->etherType & 0xFFFF) << 2);

        /* l2Encap */
        hwDataPtr[5] |= ((extNotIpv6Ptr->l2Encap & 0x01) << 1);

        /* ipv4Fragmented */
        hwDataPtr[5] |= ((extNotIpv6Ptr->ipv4Fragmented & 0x01) << 18);

        /* SIP */
        hwDataPtr[3] |= ((extNotIpv6Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[3] |= ((extNotIpv6Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[3] |= ((extNotIpv6Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[3] |= ((extNotIpv6Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[4] |= ((extNotIpv6Ptr->sip.arIP[0] & 0x80) >> 7);

        /* DIP */
        hwDataPtr[4] |= ((extNotIpv6Ptr->dip.arIP[3] & 0xFF) << 1);
        hwDataPtr[4] |= ((extNotIpv6Ptr->dip.arIP[2] & 0xFF) << 9);
        hwDataPtr[4] |= ((extNotIpv6Ptr->dip.arIP[1] & 0xFF) << 17);
        hwDataPtr[4] |= ((extNotIpv6Ptr->dip.arIP[0] & 0x7F) << 25);
        hwDataPtr[5] |= ((extNotIpv6Ptr->dip.arIP[0] & 0x80) >> 7);

        /* MAC DA */
        hwDataPtr[8] |= ((extNotIpv6Ptr->macDa.arEther[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extNotIpv6Ptr->macDa.arEther[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extNotIpv6Ptr->macDa.arEther[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extNotIpv6Ptr->macDa.arEther[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extNotIpv6Ptr->macDa.arEther[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extNotIpv6Ptr->macDa.arEther[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extNotIpv6Ptr->macDa.arEther[1] & 0xE0) >> 5);
        hwDataPtr[10] |= ((extNotIpv6Ptr->macDa.arEther[0] & 0xFF) << 3);

        /* MAC SA */
        hwDataPtr[7] |= ((extNotIpv6Ptr->macSa.arEther[5] & 0xFF) << 11);
        hwDataPtr[7] |= ((extNotIpv6Ptr->macSa.arEther[4] & 0xFF) << 19);
        hwDataPtr[7] |= ((extNotIpv6Ptr->macSa.arEther[3] & 0x1F) << 27);
        hwDataPtr[8] |= ((extNotIpv6Ptr->macSa.arEther[3] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extNotIpv6Ptr->macSa.arEther[2] & 0xFF) << 3);
        hwDataPtr[8] |= ((extNotIpv6Ptr->macSa.arEther[1] & 0xFF) << 11);
        hwDataPtr[8] |= ((extNotIpv6Ptr->macSa.arEther[0] & 0xFF) << 19);

        /* UDB */
        hwDataPtr[10] |= ((extNotIpv6Ptr->udb0_5[3] & 0xFF) << 11);
        hwDataPtr[10] |= ((extNotIpv6Ptr->udb0_5[4] & 0xFF) << 19);
        hwDataPtr[10] |= ((extNotIpv6Ptr->udb0_5[5] & 0x1F) << 27);
        hwDataPtr[11] |= ((extNotIpv6Ptr->udb0_5[5] & 0xE0) >> 5);
        hwDataPtr[11] |= ((extNotIpv6Ptr->udb0_5[0] & 0xFF) << 3);
        hwDataPtr[11] |= ((extNotIpv6Ptr->udb0_5[1] & 0xFF) << 11);
        hwDataPtr[11] |= ((extNotIpv6Ptr->udb0_5[2] & 0xFF) << 19);
    }

    /* extended egress Not IP and IPV4 */
    if (extEgrNotIpv6Ptr  != NULL)
    {
        /* etherType */
        hwDataPtr[5] |= ((extEgrNotIpv6Ptr->etherType & 0xFFFF) << 2);

        /* l2Encap */
        hwDataPtr[5] |= ((extEgrNotIpv6Ptr->l2Encap & 0x01) << 1);

        /* ipv4Fragmented */
        hwDataPtr[5] |= ((extEgrNotIpv6Ptr->ipv4Fragmented & 0x01) << 18);

        /* SIP */
        hwDataPtr[3] |= ((extEgrNotIpv6Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[3] |= ((extEgrNotIpv6Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[3] |= ((extEgrNotIpv6Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[3] |= ((extEgrNotIpv6Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[4] |= ((extEgrNotIpv6Ptr->sip.arIP[0] & 0x80) >> 7);

        /* DIP */
        hwDataPtr[4] |= ((extEgrNotIpv6Ptr->dip.arIP[3] & 0xFF) << 1);
        hwDataPtr[4] |= ((extEgrNotIpv6Ptr->dip.arIP[2] & 0xFF) << 9);
        hwDataPtr[4] |= ((extEgrNotIpv6Ptr->dip.arIP[1] & 0xFF) << 17);
        hwDataPtr[4] |= ((extEgrNotIpv6Ptr->dip.arIP[0] & 0x7F) << 25);
        hwDataPtr[5] |= ((extEgrNotIpv6Ptr->dip.arIP[0] & 0x80) >> 7);

        /* MAC SA */
        hwDataPtr[7] |= ((extEgrNotIpv6Ptr->macSa.arEther[5] & 0xFF) << 11);
        hwDataPtr[7] |= ((extEgrNotIpv6Ptr->macSa.arEther[4] & 0xFF) << 19);
        hwDataPtr[7] |= ((extEgrNotIpv6Ptr->macSa.arEther[3] & 0x1F) << 27);
        hwDataPtr[8] |= ((extEgrNotIpv6Ptr->macSa.arEther[3] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extEgrNotIpv6Ptr->macSa.arEther[2] & 0xFF) << 3);
        hwDataPtr[8] |= ((extEgrNotIpv6Ptr->macSa.arEther[1] & 0xFF) << 11);
        hwDataPtr[8] |= ((extEgrNotIpv6Ptr->macSa.arEther[0] & 0xFF) << 19);

        /* MAC DA */
        hwDataPtr[8] |= ((extEgrNotIpv6Ptr->macDa.arEther[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extEgrNotIpv6Ptr->macDa.arEther[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extEgrNotIpv6Ptr->macDa.arEther[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extEgrNotIpv6Ptr->macDa.arEther[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extEgrNotIpv6Ptr->macDa.arEther[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extEgrNotIpv6Ptr->macDa.arEther[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extEgrNotIpv6Ptr->macDa.arEther[1] & 0xE0) >> 5);
        hwDataPtr[10] |= ((extEgrNotIpv6Ptr->macDa.arEther[0] & 0xFF) << 3);
    }

    /* extended ingress IPV6 L2 */
    if (extIpv6L2Ptr   != NULL)
    {
        /* SIP */
        /* sip 31:0*/
        hwDataPtr[3] |= ((extIpv6L2Ptr->sip.arIP[15] & 0xFF) << 1);
        hwDataPtr[3] |= ((extIpv6L2Ptr->sip.arIP[14] & 0xFF) << 9);
        hwDataPtr[3] |= ((extIpv6L2Ptr->sip.arIP[13] & 0xFF) << 17);
        hwDataPtr[3] |= ((extIpv6L2Ptr->sip.arIP[12] & 0x7F) << 25);
        hwDataPtr[4] |= ((extIpv6L2Ptr->sip.arIP[12] & 0x80) >> 7);
        /* sip 127:32 */
        hwDataPtr[4] |= ((extIpv6L2Ptr->sip.arIP[11] & 0xFF) << 1);
        hwDataPtr[4] |= ((extIpv6L2Ptr->sip.arIP[10] & 0xFF) << 9);
        hwDataPtr[4] |= ((extIpv6L2Ptr->sip.arIP[9] & 0xFF) << 17);
        hwDataPtr[4] |= ((extIpv6L2Ptr->sip.arIP[8] & 0x7F) << 25);
        hwDataPtr[5] |= ((extIpv6L2Ptr->sip.arIP[8] & 0x80) >> 7);
        hwDataPtr[5] |= ((extIpv6L2Ptr->sip.arIP[7] & 0xFF) << 1);
        hwDataPtr[5] |= ((extIpv6L2Ptr->sip.arIP[6] & 0xFF) << 9);
        hwDataPtr[5] |= ((extIpv6L2Ptr->sip.arIP[5] & 0xFF) << 17);
        hwDataPtr[5] |= ((extIpv6L2Ptr->sip.arIP[4] & 0x7F) << 25);
        hwDataPtr[6] |= ((extIpv6L2Ptr->sip.arIP[4] & 0x80) >> 7);
        hwDataPtr[6] |= ((extIpv6L2Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[6] |= ((extIpv6L2Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[6] |= ((extIpv6L2Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[6] |= ((extIpv6L2Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[7] |= ((extIpv6L2Ptr->sip.arIP[0] & 0x80) >> 7);

        /* isIpv6ExtHdrExist */
        hwDataPtr[7] |= ((extIpv6L2Ptr->isIpv6ExtHdrExist & 0x01) << 1);

        /* isIpv6HopByHop */
        hwDataPtr[7] |= ((extIpv6L2Ptr->isIpv6HopByHop & 0x01) << 2);

        /* dipBits127to120 */
        hwDataPtr[7] |= ((extIpv6L2Ptr->dipBits127to120 & 0xFF) << 3);

        /* MAC DA */
        hwDataPtr[8] |= ((extIpv6L2Ptr->macDa.arEther[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extIpv6L2Ptr->macDa.arEther[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extIpv6L2Ptr->macDa.arEther[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extIpv6L2Ptr->macDa.arEther[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extIpv6L2Ptr->macDa.arEther[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extIpv6L2Ptr->macDa.arEther[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extIpv6L2Ptr->macDa.arEther[1] & 0xE0) >> 5);
        hwDataPtr[10] |= ((extIpv6L2Ptr->macDa.arEther[0] & 0xFF) << 3);

        /* MAC SA */
        hwDataPtr[7] |= ((extIpv6L2Ptr->macSa.arEther[5] & 0xFF) << 11);
        hwDataPtr[7] |= ((extIpv6L2Ptr->macSa.arEther[4] & 0xFF) << 19);
        hwDataPtr[7] |= ((extIpv6L2Ptr->macSa.arEther[3] & 0x1F) << 27);
        hwDataPtr[8] |= ((extIpv6L2Ptr->macSa.arEther[3] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extIpv6L2Ptr->macSa.arEther[2] & 0xFF) << 3);
        hwDataPtr[8] |= ((extIpv6L2Ptr->macSa.arEther[1] & 0xFF) << 11);
        hwDataPtr[8] |= ((extIpv6L2Ptr->macSa.arEther[0] & 0xFF) << 19);

        /* UDB */
        hwDataPtr[10] |= ((extIpv6L2Ptr->udb6_11[3] & 0xFF) << 11);
        hwDataPtr[10] |= ((extIpv6L2Ptr->udb6_11[4] & 0xFF) << 19);
        hwDataPtr[10] |= ((extIpv6L2Ptr->udb6_11[5] & 0x1F) << 27);
        hwDataPtr[11] |= ((extIpv6L2Ptr->udb6_11[5] & 0xE0) >> 5);
        hwDataPtr[11] |= ((extIpv6L2Ptr->udb6_11[0] & 0xFF) << 3);
        hwDataPtr[11] |= ((extIpv6L2Ptr->udb6_11[1] & 0xFF) << 11);
        hwDataPtr[11] |= ((extIpv6L2Ptr->udb6_11[2] & 0xFF) << 19);
    }

    /* extended egress IPV6 L2 */
    if (extEgrIpv6L2Ptr   != NULL)
    {
        /* SIP */
        /* sip 31:0*/
        hwDataPtr[3] |= ((extEgrIpv6L2Ptr->sip.arIP[15] & 0xFF) << 1);
        hwDataPtr[3] |= ((extEgrIpv6L2Ptr->sip.arIP[14] & 0xFF) << 9);
        hwDataPtr[3] |= ((extEgrIpv6L2Ptr->sip.arIP[13] & 0xFF) << 17);
        hwDataPtr[3] |= ((extEgrIpv6L2Ptr->sip.arIP[12] & 0x7F) << 25);
        hwDataPtr[4] |= ((extEgrIpv6L2Ptr->sip.arIP[12] & 0x80) >> 7);
        /* sip 127:32 */
        hwDataPtr[4] |= ((extEgrIpv6L2Ptr->sip.arIP[11] & 0xFF) << 1);
        hwDataPtr[4] |= ((extEgrIpv6L2Ptr->sip.arIP[10] & 0xFF) << 9);
        hwDataPtr[4] |= ((extEgrIpv6L2Ptr->sip.arIP[9] & 0xFF) << 17);
        hwDataPtr[4] |= ((extEgrIpv6L2Ptr->sip.arIP[8] & 0x7F) << 25);
        hwDataPtr[5] |= ((extEgrIpv6L2Ptr->sip.arIP[8] & 0x80) >> 7);
        hwDataPtr[5] |= ((extEgrIpv6L2Ptr->sip.arIP[7] & 0xFF) << 1);
        hwDataPtr[5] |= ((extEgrIpv6L2Ptr->sip.arIP[6] & 0xFF) << 9);
        hwDataPtr[5] |= ((extEgrIpv6L2Ptr->sip.arIP[5] & 0xFF) << 17);
        hwDataPtr[5] |= ((extEgrIpv6L2Ptr->sip.arIP[4] & 0x7F) << 25);
        hwDataPtr[6] |= ((extEgrIpv6L2Ptr->sip.arIP[4] & 0x80) >> 7);
        hwDataPtr[6] |= ((extEgrIpv6L2Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[6] |= ((extEgrIpv6L2Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[6] |= ((extEgrIpv6L2Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[6] |= ((extEgrIpv6L2Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[7] |= ((extEgrIpv6L2Ptr->sip.arIP[0] & 0x80) >> 7);

        /* dipBits127to120 */
        hwDataPtr[7] |= ((extEgrIpv6L2Ptr->dipBits127to120 & 0xFF) << 3);

        /* MAC SA */
        hwDataPtr[7] |= ((extEgrIpv6L2Ptr->macSa.arEther[5] & 0xFF) << 11);
        hwDataPtr[7] |= ((extEgrIpv6L2Ptr->macSa.arEther[4] & 0xFF) << 19);
        hwDataPtr[7] |= ((extEgrIpv6L2Ptr->macSa.arEther[3] & 0x1F) << 27);
        hwDataPtr[8] |= ((extEgrIpv6L2Ptr->macSa.arEther[3] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extEgrIpv6L2Ptr->macSa.arEther[2] & 0xFF) << 3);
        hwDataPtr[8] |= ((extEgrIpv6L2Ptr->macSa.arEther[1] & 0xFF) << 11);
        hwDataPtr[8] |= ((extEgrIpv6L2Ptr->macSa.arEther[0] & 0xFF) << 19);

        /* MAC DA */
        hwDataPtr[8] |= ((extEgrIpv6L2Ptr->macDa.arEther[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extEgrIpv6L2Ptr->macDa.arEther[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extEgrIpv6L2Ptr->macDa.arEther[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extEgrIpv6L2Ptr->macDa.arEther[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extEgrIpv6L2Ptr->macDa.arEther[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extEgrIpv6L2Ptr->macDa.arEther[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extEgrIpv6L2Ptr->macDa.arEther[1] & 0xE0) >> 5);
        hwDataPtr[10] |= ((extEgrIpv6L2Ptr->macDa.arEther[0] & 0xFF) << 3);
    }

    /* extended ingress IPV6 L4 */
    if (extIpv6L4Ptr   != NULL)
    {
        /* SIP */
        /* sip 31:0*/
        hwDataPtr[3] |= ((extIpv6L4Ptr->sip.arIP[15] & 0xFF) << 1);
        hwDataPtr[3] |= ((extIpv6L4Ptr->sip.arIP[14] & 0xFF) << 9);
        hwDataPtr[3] |= ((extIpv6L4Ptr->sip.arIP[13] & 0xFF) << 17);
        hwDataPtr[3] |= ((extIpv6L4Ptr->sip.arIP[12] & 0x7F) << 25);
        hwDataPtr[4] |= ((extIpv6L4Ptr->sip.arIP[12] & 0x80) >> 7);
        /* sip 127:32 */
        hwDataPtr[4] |= ((extIpv6L4Ptr->sip.arIP[11] & 0xFF) << 1);
        hwDataPtr[4] |= ((extIpv6L4Ptr->sip.arIP[10] & 0xFF) << 9);
        hwDataPtr[4] |= ((extIpv6L4Ptr->sip.arIP[9] & 0xFF) << 17);
        hwDataPtr[4] |= ((extIpv6L4Ptr->sip.arIP[8] & 0x7F) << 25);
        hwDataPtr[5] |= ((extIpv6L4Ptr->sip.arIP[8] & 0x80) >> 7);
        hwDataPtr[5] |= ((extIpv6L4Ptr->sip.arIP[7] & 0xFF) << 1);
        hwDataPtr[5] |= ((extIpv6L4Ptr->sip.arIP[6] & 0xFF) << 9);
        hwDataPtr[5] |= ((extIpv6L4Ptr->sip.arIP[5] & 0xFF) << 17);
        hwDataPtr[5] |= ((extIpv6L4Ptr->sip.arIP[4] & 0x7F) << 25);
        hwDataPtr[6] |= ((extIpv6L4Ptr->sip.arIP[4] & 0x80) >> 7);
        hwDataPtr[6] |= ((extIpv6L4Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[6] |= ((extIpv6L4Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[6] |= ((extIpv6L4Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[6] |= ((extIpv6L4Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[7] |= ((extIpv6L4Ptr->sip.arIP[0] & 0x80) >> 7);

        /* isIpv6ExtHdrExist */
        hwDataPtr[7] |= ((extIpv6L4Ptr->isIpv6ExtHdrExist & 0x01) << 1);

        /* isIpv6HopByHop */
        hwDataPtr[7] |= ((extIpv6L4Ptr->isIpv6HopByHop & 0x01) << 2);

        /* DIP */
        /* dip 127:120 */
        hwDataPtr[7] |= ((extIpv6L4Ptr->dip.arIP[0] & 0xFF) << 3);
        /* dip 119:32 */
        hwDataPtr[7] |= ((extIpv6L4Ptr->dip.arIP[11] & 0xFF) << 11);
        hwDataPtr[7] |= ((extIpv6L4Ptr->dip.arIP[10] & 0xFF) << 19);
        hwDataPtr[7] |= ((extIpv6L4Ptr->dip.arIP[9] & 0x1F) << 27);
        hwDataPtr[8] |= ((extIpv6L4Ptr->dip.arIP[9] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extIpv6L4Ptr->dip.arIP[8] & 0xFF) << 3);
        hwDataPtr[8] |= ((extIpv6L4Ptr->dip.arIP[7] & 0xFF) << 11);
        hwDataPtr[8] |= ((extIpv6L4Ptr->dip.arIP[6] & 0xFF) << 19);
        hwDataPtr[8] |= ((extIpv6L4Ptr->dip.arIP[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extIpv6L4Ptr->dip.arIP[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extIpv6L4Ptr->dip.arIP[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extIpv6L4Ptr->dip.arIP[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extIpv6L4Ptr->dip.arIP[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extIpv6L4Ptr->dip.arIP[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extIpv6L4Ptr->dip.arIP[1] & 0xE0) >> 5);

        /* dip 31:0 */
        hwDataPtr[10] |= ((extIpv6L4Ptr->dip.arIP[15] & 0xFF) << 3);
        hwDataPtr[10] |= ((extIpv6L4Ptr->dip.arIP[14] & 0xFF) << 11);
        hwDataPtr[10] |= ((extIpv6L4Ptr->dip.arIP[13] & 0xFF) << 19);
        hwDataPtr[10] |= ((extIpv6L4Ptr->dip.arIP[12] & 0x1F) << 27);
        hwDataPtr[11] |= ((extIpv6L4Ptr->dip.arIP[12] & 0xE0) >> 5);

        /* UDB */
        hwDataPtr[11] |= ((extIpv6L4Ptr->udb12_14[0] & 0xFF) << 3);
        hwDataPtr[11] |= ((extIpv6L4Ptr->udb12_14[1] & 0xFF) << 11);
        hwDataPtr[11] |= ((extIpv6L4Ptr->udb12_14[2] & 0xFF) << 19);
    }

    /* extended egress IPV6 L4 */
    if (extEgrIpv6L4Ptr   != NULL)
    {
        /* SIP */
        /* sip 31:0*/
        hwDataPtr[3] |= ((extEgrIpv6L4Ptr->sip.arIP[15] & 0xFF) << 1);
        hwDataPtr[3] |= ((extEgrIpv6L4Ptr->sip.arIP[14] & 0xFF) << 9);
        hwDataPtr[3] |= ((extEgrIpv6L4Ptr->sip.arIP[13] & 0xFF) << 17);
        hwDataPtr[3] |= ((extEgrIpv6L4Ptr->sip.arIP[12] & 0x7F) << 25);
        hwDataPtr[4] |= ((extEgrIpv6L4Ptr->sip.arIP[12] & 0x80) >> 7);
        /* sip 127:32 */
        hwDataPtr[4] |= ((extEgrIpv6L4Ptr->sip.arIP[11] & 0xFF) << 1);
        hwDataPtr[4] |= ((extEgrIpv6L4Ptr->sip.arIP[10] & 0xFF) << 9);
        hwDataPtr[4] |= ((extEgrIpv6L4Ptr->sip.arIP[9] & 0xFF) << 17);
        hwDataPtr[4] |= ((extEgrIpv6L4Ptr->sip.arIP[8] & 0x7F) << 25);
        hwDataPtr[5] |= ((extEgrIpv6L4Ptr->sip.arIP[8] & 0x80) >> 7);
        hwDataPtr[5] |= ((extEgrIpv6L4Ptr->sip.arIP[7] & 0xFF) << 1);
        hwDataPtr[5] |= ((extEgrIpv6L4Ptr->sip.arIP[6] & 0xFF) << 9);
        hwDataPtr[5] |= ((extEgrIpv6L4Ptr->sip.arIP[5] & 0xFF) << 17);
        hwDataPtr[5] |= ((extEgrIpv6L4Ptr->sip.arIP[4] & 0x7F) << 25);
        hwDataPtr[6] |= ((extEgrIpv6L4Ptr->sip.arIP[4] & 0x80) >> 7);
        hwDataPtr[6] |= ((extEgrIpv6L4Ptr->sip.arIP[3] & 0xFF) << 1);
        hwDataPtr[6] |= ((extEgrIpv6L4Ptr->sip.arIP[2] & 0xFF) << 9);
        hwDataPtr[6] |= ((extEgrIpv6L4Ptr->sip.arIP[1] & 0xFF) << 17);
        hwDataPtr[6] |= ((extEgrIpv6L4Ptr->sip.arIP[0] & 0x7F) << 25);
        hwDataPtr[7] |= ((extEgrIpv6L4Ptr->sip.arIP[0] & 0x80) >> 7);

        /* DIP */
        /* dip 127:120 */
        hwDataPtr[7] |= ((extEgrIpv6L4Ptr->dip.arIP[0] & 0xFF) << 3);

        /* dip 119:32 */
        hwDataPtr[7] |= ((extEgrIpv6L4Ptr->dip.arIP[11] & 0xFF) << 11);
        hwDataPtr[7] |= ((extEgrIpv6L4Ptr->dip.arIP[10] & 0xFF) << 19);
        hwDataPtr[7] |= ((extEgrIpv6L4Ptr->dip.arIP[9] & 0x1F) << 27);
        hwDataPtr[8] |= ((extEgrIpv6L4Ptr->dip.arIP[9] & 0xE0) >> 5);
        hwDataPtr[8] |= ((extEgrIpv6L4Ptr->dip.arIP[8] & 0xFF) << 3);
        hwDataPtr[8] |= ((extEgrIpv6L4Ptr->dip.arIP[7] & 0xFF) << 11);
        hwDataPtr[8] |= ((extEgrIpv6L4Ptr->dip.arIP[6] & 0xFF) << 19);
        hwDataPtr[8] |= ((extEgrIpv6L4Ptr->dip.arIP[5] & 0x1F) << 27);
        hwDataPtr[9] |= ((extEgrIpv6L4Ptr->dip.arIP[5] & 0xE0) >> 5);
        hwDataPtr[9] |= ((extEgrIpv6L4Ptr->dip.arIP[4] & 0xFF) << 3);
        hwDataPtr[9] |= ((extEgrIpv6L4Ptr->dip.arIP[3] & 0xFF) << 11);
        hwDataPtr[9] |= ((extEgrIpv6L4Ptr->dip.arIP[2] & 0xFF) << 19);
        hwDataPtr[9] |= ((extEgrIpv6L4Ptr->dip.arIP[1] & 0x1F) << 27);
        hwDataPtr[10] |= ((extEgrIpv6L4Ptr->dip.arIP[1] & 0xE0) >> 5);

        /* dip 31:0 */
        hwDataPtr[10] |= ((extEgrIpv6L4Ptr->dip.arIP[15] & 0xFF) << 3);
        hwDataPtr[10] |= ((extEgrIpv6L4Ptr->dip.arIP[14] & 0xFF) << 11);
        hwDataPtr[10] |= ((extEgrIpv6L4Ptr->dip.arIP[13] & 0xFF) << 19);
        hwDataPtr[10] |= ((extEgrIpv6L4Ptr->dip.arIP[12] & 0x1F) << 27);
        hwDataPtr[11] |= ((extEgrIpv6L4Ptr->dip.arIP[12] & 0xE0) >> 5);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPclTcamRuleDataHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM pattern or mask from HW to SW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] hwDataPtr                - rule pattern or mask in HW format.
*
* @param[out] swDataPtr                - rule pattern or mask in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataHw2SwConvert
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  GT_U32                             *hwDataPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT      *swDataPtr
)
{
    GT_STATUS                          rc;          /* return code           */
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC *ruleCnvPtr=NULL; /* rule conversion table */
    GT_U32                             ruleVer;     /* rule version          */
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC**fmtFldArrPtr=NULL;  /* pointer to array of SW convert for the keys*/

    /* clear output buffer */
    cpssOsMemSet(swDataPtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        fmtFldArrPtr = fmtFldArr_sip5;
    }
    else
    {
        switch (ruleFormat)
        {
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E        /*15*/  :
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E         /*16*/  :
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E       /*17*/  :
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E      /*18*/  :
            case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E              /*19*/  :
            case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E            /*20*/  :
                switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    case CPSS_PP_FAMILY_DXCH_LION2_E:
                        fmtFldArrPtr = fmtFldArr_Lion2;
                        break;
                    case CPSS_PP_FAMILY_DXCH_LION_E:
                    case CPSS_PP_FAMILY_DXCH_XCAT_E:
                    case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                        fmtFldArrPtr = fmtFldArr_xcat;
                        break;
                    case CPSS_PP_FAMILY_CHEETAH3_E:
                        fmtFldArrPtr = fmtFldArr_ch3;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    if(fmtFldArrPtr == NULL)
    {
        /* default value - xCat and Lion rule */
        ruleVer = 0;
        if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat2 and Lion2 rule */
            ruleVer = 1;
        }

        switch (ruleVer)
        {
            case 0: /* xCat and Lion */
                switch (ruleFormat)
                {
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_NOT_IP_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IP_L2_QOS_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IPV4_L4_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IPV6_DIP_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_NOT_IPV6_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_IPV6_L2_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_IPV6_L4_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_NOT_IP_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_IP_L2_QOS_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_IPV4_L4_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_NOT_IPV6_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_IPV6_L2_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_IPV6_L4_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_UDB_xCat;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_UDB_xCat;
                        break;
                    default:
                        break;
                }
                break;
            default:
            case 1: /* xCat2 and Lion2 */
                switch (ruleFormat)
                {
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_NOT_IP_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IP_L2_QOS_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IPV4_L4_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_IPV6_DIP_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_NOT_IPV6_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_IPV6_L2_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_IPV6_L4_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_NOT_IP_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_IP_L2_QOS_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
                        ruleCnvPtr = fmtFld_EGRESS_STD_IPV4_L4_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_NOT_IPV6_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_IPV6_L2_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
                        ruleCnvPtr = fmtFld_EGRESS_EXT_IPV6_L4_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                        ruleCnvPtr = fmtFld_INGRESS_STD_UDB_xCat2;
                        break;
                    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                        ruleCnvPtr = fmtFld_INGRESS_EXT_UDB_xCat2;
                        break;
                    default:
                        break;
                }
                break;
        }
    }

    if(fmtFldArrPtr)
    {
        ruleCnvPtr = fmtFldArrPtr[ruleFormat];
    }

    if(ruleCnvPtr == NULL)
    {
        /* the key type for this device is not supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssFormatConvertRawToStc(
        ruleCnvPtr, hwDataPtr, swDataPtr,
        NULL /*extMemoPtr*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclCfgTblHwSelectTblAndPos function
* @endinternal
*
* @brief   Selects the PCL Configuration table HW id and the field positions descriptor.
*         The position descriptor is a SW structure that contains offsets lengts
*         of the relevasnt subfields of the table entry
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
*
* @param[out] tableIdPtr               - (pointer to) HW table Id
* @param[out] posPtrPtr                - (pointer to) (pointer to) position descriptor
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
*/
GT_STATUS prvCpssDxChPclCfgTblHwSelectTblAndPos
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PCL_DIRECTION_ENT                 direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                 subLookupNum,
    OUT CPSS_DXCH_TABLE_ENT                *tableIdPtr,
    OUT PRV_PCL_CFG_LOOKUP_POS_STC             **posPtrPtr
)
{
    switch (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
            if (direction != CPSS_PCL_DIRECTION_INGRESS_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            switch (lookupNum)
            {
                case CPSS_PCL_LOOKUP_0_E:
                    *posPtrPtr = &prvCpssDxChPclCfgTblHwChPlusIngrL0Pos;
                    break;
                case CPSS_PCL_LOOKUP_1_E:
                    *posPtrPtr = &prvCpssDxChPclCfgTblHwChPlusIngrL1Pos;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PP_FAMILY_CHEETAH2_E:
            switch (direction)
            {
                case CPSS_PCL_DIRECTION_INGRESS_E:
                    switch (lookupNum)
                    {
                        case CPSS_PCL_LOOKUP_0_E:
                            *posPtrPtr = &prvCpssDxChPclCfgTblHwCh2IngrL0Pos;
                            break;
                        case CPSS_PCL_LOOKUP_1_E:
                            *posPtrPtr = &prvCpssDxChPclCfgTblHwCh2IngrL1Pos;
                            break;
                        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    break;
                case CPSS_PCL_DIRECTION_EGRESS_E:
                    /* ignore lookup number */
                    *posPtrPtr = &prvCpssDxChPclCfgTblHwCh2EgrPos;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                /* Lion2 and above */
                switch (direction)
                {
                    case CPSS_PCL_DIRECTION_INGRESS_E:
                        /* the same for 3 lookups 0, 1 and 0_1 */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwSip5IngrPos;
                        break;
                    case CPSS_PCL_DIRECTION_EGRESS_E:
                        /* ignore lookup number */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwSip5EgrPos;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
            {
                /* xCat2 and above */
                switch (direction)
                {
                    case CPSS_PCL_DIRECTION_INGRESS_E:
                        /* the same for 3 lookups 0, 1 and 0_1 */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwXCatIngrL1Pos;
                        break;
                    case CPSS_PCL_DIRECTION_EGRESS_E:
                        /* ignore lookup number */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwCh3EgrPos;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            {
                /* xCat, xCat3 and Lion */
                switch (direction)
                {
                    case CPSS_PCL_DIRECTION_INGRESS_E:
                        switch (lookupNum)
                        {
                            case CPSS_PCL_LOOKUP_0_E:
                                *posPtrPtr = &prvCpssDxChPclCfgTblHwXCatIngrL0Pos;
                                break;
                            case CPSS_PCL_LOOKUP_1_E:
                                *posPtrPtr = &prvCpssDxChPclCfgTblHwXCatIngrL1Pos;
                                break;
                            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                        break;
                    case CPSS_PCL_DIRECTION_EGRESS_E:
                        /* ignore lookup number */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwCh3EgrPos;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* DxCh3 */
                switch (direction)
                {
                    case CPSS_PCL_DIRECTION_INGRESS_E:
                        switch (lookupNum)
                        {
                            case CPSS_PCL_LOOKUP_0_E:
                                *posPtrPtr = &prvCpssDxChPclCfgTblHwCh3IngrL0Pos;
                                break;
                            case CPSS_PCL_LOOKUP_1_E:
                                *posPtrPtr = &prvCpssDxChPclCfgTblHwCh3IngrL1Pos;
                                break;
                            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                        break;
                    case CPSS_PCL_DIRECTION_EGRESS_E:
                        /* ignore lookup number */
                        *posPtrPtr = &prvCpssDxChPclCfgTblHwCh3EgrPos;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            break;
    }

    switch (direction)
    {
        case CPSS_PCL_DIRECTION_INGRESS_E:
            if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
            {
                /* xCat2 and above */
                switch (lookupNum)
                {
                    case CPSS_PCL_LOOKUP_0_E:
                        *tableIdPtr =
                            (subLookupNum == 0)
                            ? CPSS_DXCH_TABLE_PCL_CONFIG_E
                            : CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E;
                        break;
                    case CPSS_PCL_LOOKUP_1_E:
                        /* xCat2 IPCL Lookup1 Cfg Table */
                        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                        *tableIdPtr = CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E;
                        break;
                    case CPSS_PCL_LOOKUP_0_0_E:
                        /* xCat2 IPCL Lookup00 Cfg Table */
                        *tableIdPtr = CPSS_DXCH_TABLE_PCL_CONFIG_E;
                        break;
                    case CPSS_PCL_LOOKUP_0_1_E:
                        /* xCat2 IPCL Lookup01 Cfg Table */
                        *tableIdPtr = CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)
                && (lookupNum == CPSS_PCL_LOOKUP_1_E))
            {
                /* xCat, xCat3 and Lion IPCL Lookup1 Cfg Table */
                *tableIdPtr = CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E;
            }
            else
            {
                /* DxCh1-3 any lookup                   */
                /* DxCat and Lion lookup00 and lookup01 */
                *tableIdPtr = CPSS_DXCH_TABLE_PCL_CONFIG_E;
            }

            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            *tableIdPtr = CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return  GT_OK;
}

/**
* @internal prvCpssDxChPclCfgTblEntryIndexGet function
* @endinternal
*
* @brief   Calculates the index of PCL Configuration table entry's for given interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: either port or VLAN
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*
* @param[out] indexPtr                 - (pointer to) calculated index
* @param[out] portGroupIdPtr           - (pointer to) the port group Id that the index refers to.
*                                      needed to support multi-port-groups device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on HW error
*/
GT_STATUS prvCpssDxChPclCfgTblEntryIndexGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_INTERFACE_INFO_STC         *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    OUT GT_U32                          *indexPtr,
    OUT GT_U32                          *portGroupIdPtr
)
{
    GT_U32           regAddr;       /* register address                    */
    GT_STATUS        res;           /* return code                         */
    GT_U32           entryIndex;    /* pcl configuration table entry Index */
    GT_U32           gloRegData;    /* data from an appropriate global register */
    GT_BOOL          nonLocalAccess;/* GT_TRUE - non Local Access          */
    GT_U32           nonLocalBase;  /* base 0,1024,2048,3072               */
    GT_U32           portsPerDev;   /* amount of ports Per Device          */
    GT_U32           localPort;     /* local port - support multi-port-groups device */
    GT_HW_DEV_NUM    hwDevNum;      /* HW device number                    */
    GT_PORT_NUM      hwGlobalPort;  /* global port - support double device */
    GT_U32    EPCLPortInterfacePCLBlockBit = 0;/* EPCL : bit index of field <EPCL Port Interface PCL Block>*/
    GT_U32    EPCLMaxPortPerDevBit = 0;/* EPCL : bit index of field <EPCL Max Port Per Dev>*/

    /* state that the index belongs to all port groups -- for multi-port-groups device */
    *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    switch (direction)
    {
        case CPSS_PCL_DIRECTION_INGRESS_E:
            if (interfaceInfoPtr->type == CPSS_INTERFACE_INDEX_E)
            {
                if (interfaceInfoPtr->index < PRV_DXCH_IPCL_CONF_TBL_ROW_NUMBER_CNS(devNum))
                {
                    *indexPtr = interfaceInfoPtr->index;
                    return GT_OK;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).ingrPolicyGlobalConfig;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
            }
            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
                 == CPSS_PP_FAMILY_CHEETAH_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (interfaceInfoPtr->type == CPSS_INTERFACE_INDEX_E)
            {
                if (interfaceInfoPtr->index < PRV_DXCH_EPCL_CONF_TBL_ROW_NUMBER_CNS(devNum))
                {
                    *indexPtr = interfaceInfoPtr->index;
                    return GT_OK;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;
                EPCLPortInterfacePCLBlockBit = 6;
                EPCLMaxPortPerDevBit = 8;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.epclGlobalConfig;
                EPCLPortInterfacePCLBlockBit = 16;
                EPCLMaxPortPerDevBit = 18;
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for ingress and egress used the same configuration bites 1 and 18-16 */
    /* in ingress and in egress global configuration registers              */
    res = prvCpssHwPpPortGroupReadRegister(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &gloRegData);
    if (res != GT_OK)
    {
        return res;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* not as in Cheetah2 - only this bit relevant */
        nonLocalAccess = ((gloRegData & 2) == 0) ? GT_TRUE : GT_FALSE;
        switch (interfaceInfoPtr->type)
        {
            case CPSS_INTERFACE_PORT_E:
                if (nonLocalAccess == GT_FALSE)
                {
                    entryIndex = 1024 + interfaceInfoPtr->devPort.portNum;
                }
                else
                {
                    entryIndex = (interfaceInfoPtr->devPort.hwDevNum * 32)
                        + interfaceInfoPtr->devPort.portNum;
                }
                break;
            case CPSS_INTERFACE_VID_E:
                if (interfaceInfoPtr->vlanId >= 1024)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                entryIndex = interfaceInfoPtr->vlanId;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                if (nonLocalAccess == GT_FALSE)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                entryIndex = 1024 + interfaceInfoPtr->trunkId;
                break;
            /* all other types not relevant */
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        *indexPtr = entryIndex;
        return GT_OK;
    }

    /* DXCH2 and above */
    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* XCAT and above Ingress */
        if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            nonLocalAccess = ((gloRegData & 4) != 0) ? GT_TRUE : GT_FALSE;
            portsPerDev    = ((gloRegData & (1 << 7)) != 0) ? 64 : 32;
            nonLocalBase   = (portsPerDev == 32)
                ? (1024 * ((gloRegData >> 3) & 3))
                : (2048 * ((gloRegData >> 3) & 1));
        }
        else
        {
            /* XCAT and above Egress */
            nonLocalAccess = ((gloRegData & 2) != 0) ? GT_TRUE : GT_FALSE;
            portsPerDev    = ((gloRegData & (1 << EPCLMaxPortPerDevBit)) != 0) ? 64 : 32;
            nonLocalBase   = (2048 * ((gloRegData >> EPCLPortInterfacePCLBlockBit) & 1));
        }

    }
    else
    {
        /* CH1-3 Ingess and Egress */
        nonLocalAccess = ((gloRegData & 2) != 0) ? GT_TRUE : GT_FALSE;
        portsPerDev    = ((gloRegData & (1 << 18)) != 0) ? 64 : 32;
        nonLocalBase   = (portsPerDev == 32)
            ? (1024 * ((gloRegData >> 16) & 3))
            : (2048 * ((gloRegData >> 16) & 1));
    }

    switch (interfaceInfoPtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            if (nonLocalAccess == GT_FALSE)
            {
                if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    /* no conversion needed and we can write to all port groups the same value */
                    *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                    localPort = interfaceInfoPtr->devPort.portNum;
                }
                else
                {
                    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
                    *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,interfaceInfoPtr->devPort.portNum);
                    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,interfaceInfoPtr->devPort.portNum);
                }
                entryIndex = 4096 + localPort;
            }
            else
            {
                /* devNum+portNum mode */

                hwDevNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                    interfaceInfoPtr->devPort.hwDevNum,
                    interfaceInfoPtr->devPort.portNum);

                if (interfaceInfoPtr->devPort.hwDevNum != PRV_CPSS_HW_DEV_NUM_MAC(devNum))
                {
                    /* remote device, devNum and PortNum from DSA tag */

                    hwGlobalPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                        interfaceInfoPtr->devPort.hwDevNum,
                        interfaceInfoPtr->devPort.portNum);

                    entryIndex = nonLocalBase
                        + (hwDevNum * portsPerDev) + hwGlobalPort;
                }
                else
                {
                    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                    {
                        /* no conversion needed and we can write to all port groups the same value */
                        *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                        localPort = (interfaceInfoPtr->devPort.portNum & (portsPerDev-1));
                    }
                    else
                    {
                        /* local port and dual device support */
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(
                            devNum, interfaceInfoPtr->devPort.portNum);
                    }

                    entryIndex = nonLocalBase
                        + (hwDevNum * portsPerDev) + localPort;
                }
            }
            break;
        case CPSS_INTERFACE_VID_E:
            if (interfaceInfoPtr->vlanId >= 4096)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            entryIndex = interfaceInfoPtr->vlanId;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            if (direction != CPSS_PCL_DIRECTION_INGRESS_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if (nonLocalAccess == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = 4096 + interfaceInfoPtr->trunkId;
            break;
        /* all other types not relevant */
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *indexPtr = entryIndex;
    return GT_OK;
}

/**
* @internal prvCpssDxChPclCfgTblHwWrite function
* @endinternal
*
* @brief   Write PCL Configuration table entry (fields in HW format)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group Id , to support multi-port-groups device
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] entryIndex               - entry Index
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
* @param[in] lookupCfgPtr             - HW lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclCfgTblHwWrite
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN CPSS_PCL_DIRECTION_ENT                 direction,
    IN GT_U32                                 entryIndex,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                subLookupNum,
    IN PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC    *lookupCfgPtr
)
{
    GT_STATUS                  res;            /* return code          */
    PRV_PCL_CFG_LOOKUP_POS_STC *posPtr;        /* field layouts        */
    CPSS_DXCH_TABLE_ENT    tableId;        /* HW table Id          */
    GT_U32                     cfgEntry[2];    /* HW table entry data  */
    GT_U32                     fieldAmount;    /* amount of HW fields  */

    /* field value          */
    GT_U32  valueArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];
    /* field offset         */
    GT_U32  offsetArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];
    /* field length         */
    GT_U32  lenArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];

    res = prvCpssDxChPclCfgTblHwSelectTblAndPos(
        devNum, direction, lookupNum, subLookupNum, &tableId, &posPtr);
    if (res != GT_OK)
    {
        return res;
    }

    fieldAmount = 0;

    valueArray[fieldAmount]  = lookupCfgPtr->enableLookup;
    offsetArray[fieldAmount] = posPtr->enableLookupOff;
    lenArray[fieldAmount]    = posPtr->enableLookupLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->nonIpKeyType;
    offsetArray[fieldAmount] = posPtr->nonIpKeyTypeOff;
    lenArray[fieldAmount]    = posPtr->nonIpKeyTypeLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->ipv4KeyType;
    offsetArray[fieldAmount] = posPtr->ipv4KeyTypeOff;
    lenArray[fieldAmount]    = posPtr->ipv4KeyTypeLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->ipv6KeyType;
    offsetArray[fieldAmount] = posPtr->ipv6KeyTypeOff;
    lenArray[fieldAmount]    = posPtr->ipv6KeyTypeLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->pclId;
    offsetArray[fieldAmount] = posPtr->pclIdOff;
    lenArray[fieldAmount]    = posPtr->pclIdLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->dualLookup;
    offsetArray[fieldAmount] = posPtr->dualLookupOff;
    lenArray[fieldAmount]    = posPtr->dualLookupLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->pclIdL01;
    offsetArray[fieldAmount] = posPtr->pclIdL01Off;
    lenArray[fieldAmount]    = posPtr->pclIdL01Len;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->udbKeyBitmapEnable;
    offsetArray[fieldAmount] = posPtr->udbKeyBitmapEnableOff;
    lenArray[fieldAmount]    = posPtr->udbKeyBitmapEnableLen;
    fieldAmount ++;

    valueArray[fieldAmount]  = lookupCfgPtr->tcamSegmentMode;
    offsetArray[fieldAmount] = posPtr->tcamSegmentModeOff;
    lenArray[fieldAmount]    = posPtr->tcamSegmentModeLen;
    fieldAmount ++;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* enable the entry */
        valueArray[fieldAmount]  = 1;
        offsetArray[fieldAmount] = 28;
        lenArray[fieldAmount]    = 1;
        fieldAmount ++;
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        valueArray[fieldAmount]  = lookupCfgPtr->exactMatchLookupSubProfileId;
        offsetArray[fieldAmount] = posPtr->exactMatchLookupSubProfileIdOff;
        lenArray[fieldAmount]    = posPtr->exactMatchLookupSubProfileIdLen;
        fieldAmount ++;
    }

    return prvCpssDxChPortGroupWriteTableEntryFieldList(
        devNum, portGroupId, tableId, entryIndex, cfgEntry,
        fieldAmount, offsetArray, lenArray, valueArray);
}

/**
* @internal prvCpssDxChPclCfgTblHwRead function
* @endinternal
*
* @brief   Read PCL Configuration table entry (fields in HW format)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group Id , to support multi-port-groups device
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] entryIndex               - entry Index
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
*
* @param[out] lookupCfgPtr             - HW lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclCfgTblHwRead
(
    IN  GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN  CPSS_PCL_DIRECTION_ENT                 direction,
    IN  GT_U32                                 entryIndex,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                 subLookupNum,
    OUT PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC    *lookupCfgPtr
)
{
    GT_STATUS                  res;            /* return code          */
    PRV_PCL_CFG_LOOKUP_POS_STC *posPtr;        /* field layouts        */
    CPSS_DXCH_TABLE_ENT    tableId;        /* HW table Id          */
    GT_U32                     cfgEntry[2];    /* HW table entry data  */
    GT_U32                     fieldAmount;    /* amount of HW fields  */

    /* field value          */
    GT_U32  valueArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];
    /* field offset         */
    GT_U32  offsetArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];
    /* field length         */
    GT_U32  lenArray[PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS];

    cpssOsMemSet(valueArray, 0, sizeof(valueArray));

    res = prvCpssDxChPclCfgTblHwSelectTblAndPos(
        devNum, direction, lookupNum, subLookupNum, &tableId, &posPtr);
    if (res != GT_OK)
    {
        return res;
    }

    fieldAmount = 0;

    offsetArray[fieldAmount] = posPtr->enableLookupOff;
    lenArray[fieldAmount]    = posPtr->enableLookupLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->nonIpKeyTypeOff;
    lenArray[fieldAmount]    = posPtr->nonIpKeyTypeLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->ipv4KeyTypeOff;
    lenArray[fieldAmount]    = posPtr->ipv4KeyTypeLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->ipv6KeyTypeOff;
    lenArray[fieldAmount]    = posPtr->ipv6KeyTypeLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->pclIdOff;
    lenArray[fieldAmount]    = posPtr->pclIdLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->dualLookupOff;
    lenArray[fieldAmount]    = posPtr->dualLookupLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->pclIdL01Off;
    lenArray[fieldAmount]    = posPtr->pclIdL01Len;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->udbKeyBitmapEnableOff;
    lenArray[fieldAmount]    = posPtr->udbKeyBitmapEnableLen;
    fieldAmount ++;

    offsetArray[fieldAmount] = posPtr->tcamSegmentModeOff;
    lenArray[fieldAmount]    = posPtr->tcamSegmentModeLen;
    fieldAmount ++;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* Cheetah Plus "entry-valid" bit */
        offsetArray[fieldAmount] = 28;
        lenArray[fieldAmount]    = 1;
        fieldAmount ++;
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        offsetArray[fieldAmount] = posPtr->exactMatchLookupSubProfileIdOff;
        lenArray[fieldAmount]    = posPtr->exactMatchLookupSubProfileIdLen;
        fieldAmount ++;
    }

    res = prvCpssDxChPortGroupReadTableEntryFieldList(
        devNum, portGroupId , tableId, entryIndex, cfgEntry,
        fieldAmount, offsetArray, lenArray, valueArray);
    if (res != GT_OK)
    {
        return res;
    }

    /* erase all, not relevant fields will not been updated */
    cpssOsMemSet(lookupCfgPtr, 0, sizeof(PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC));

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
         == CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* check Cheetah Plus "entry-valid" bit */
        if (valueArray[fieldAmount - 1] == 0)
        {
            return GT_OK;
        }
    }

    lookupCfgPtr->enableLookup         = (GT_U8)valueArray[0];
    lookupCfgPtr->nonIpKeyType         = (GT_U8)valueArray[1];
    lookupCfgPtr->ipv4KeyType          = (GT_U8)valueArray[2];
    lookupCfgPtr->ipv6KeyType          = (GT_U8)valueArray[3];
    lookupCfgPtr->pclId                = (GT_U32)valueArray[4];
    lookupCfgPtr->dualLookup           = (GT_U8)valueArray[5];
    lookupCfgPtr->pclIdL01             = (GT_U32)valueArray[6];
    lookupCfgPtr->udbKeyBitmapEnable   = (GT_U8)valueArray[7];
    lookupCfgPtr->tcamSegmentMode      = (GT_U32)valueArray[8];
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        lookupCfgPtr->exactMatchLookupSubProfileId = (GT_U32)valueArray[9];
    }
    return  GT_OK;
}

/* CH3 specific utils */

/**
* @internal prvCpssDxChPclTcamStdRuleDataXorYGet function
* @endinternal
*
* @brief   Reads standard rule X or Y data
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xORy                     - GT_TRUE for X, GT_FALSE for Y.
*
* @param[out] dataPtr                  - data:  6 words for X or Y
*                                      For xCat2 and above devices - 7 words.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS prvCpssDxChPclTcamStdRuleDataXorYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       xORy,
    OUT GT_U32                        *dataPtr
)
{
    GT_U8        i;           /* loop index                              */
    GT_U32       data;        /* register data                           */
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddr;     /* register address                        */

    for (i = 0; (i < 8); i++)
    {
        rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, i, xORy, &regAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (0 != PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat2 and above */
            switch (i)
            {
                case 0: /* 30 bits */
                    dataPtr[0] =  ((data >> 2) & 0x3FFFFFFF);
                    break;
                case 1: /* 2 + 18 bits */
                    dataPtr[0] |= ((data & 3) << 30);
                    dataPtr[1] =  ((data >> 2) & 0x3FFFF);
                    break;
                case 2: /* 14 + 18 bits */
                    dataPtr[1] |= ((data & 0x3FFF) << 18);
                    dataPtr[2] =  ((data >> 14) & 0x3FFFF);
                    break;
                case 3: /* 14 + 6 bits */
                    dataPtr[2] |= ((data  & 0x3FFF) << 18);
                    dataPtr[3] =  ((data >> 14) & 0x3F);
                    break;
                case 4: /* 26 + 6 bits */
                    dataPtr[3] |=  ((data & 0x3FFFFFF) << 6);
                    dataPtr[4] =  ((data >> 26) & 0x3F);
                    break;
                case 5: /* 20 bits */
                    dataPtr[4] |=  ((data & 0xFFFFF) << 6);
                    break;
                case 6: /* 6 + 26 bits */
                    dataPtr[4] |= ((data & 0x3F) << 26);
                    dataPtr[5] =  ((data >> 6) & 0x3FFFFFF);
                    break;
                case 7: /* 6 + 14 bits*/
                    dataPtr[5] |= ((data & 0x3F) << 26);
                    dataPtr[6]  = ((data >> 6) & 0x3FFF);
                    break;

                default:
                        break;
            }
        }
        else
        {
            /* CH3, xCat, xCat3, Lion */
        switch (i)
        {
            case 0: dataPtr[0] =  data; break;
            case 1: dataPtr[1] =  (data & 0x0000FFFF); break;
            case 2: dataPtr[1] |= (data << 16) & 0xFFFF0000;
                    dataPtr[2] =  (data >> 16) & 0x0000FFFF;
                    break;
            case 3: dataPtr[2] |= (data << 16) & 0xFFFF0000; break;
            case 4: dataPtr[3] =  data; break;
            case 5: dataPtr[4] =  (data & 0x0000FFFF);break;
            case 6: dataPtr[4] |= (data << 16) & 0xFFFF0000;
                    dataPtr[5] =  (data >> 16) & 0x0000FFFF;
                    break;
            case 7: dataPtr[5] |= (data << 16) & 0xFFFF0000; break;

            default:
                    break;
        }
    }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamStdRuleDataXandYGet function
* @endinternal
*
* @brief   Reads standard rule X and Y data
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
*
* @param[out] xPtr                     - data:  6 words for X
* @param[out] yPtr                     - data:  6 words for Y
*                                      For xCat2 and above devices - 7 words.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleDataXandYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *xPtr,
    OUT GT_U32                        *yPtr
)
{
    GT_STATUS    rc;          /* return code                             */

    rc = prvCpssDxChPclTcamStdRuleDataXorYGet(
            devNum, portGroupId, ruleIndex, GT_TRUE, xPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamStdRuleDataXorYGet(
            devNum, portGroupId, ruleIndex, GT_FALSE, yPtr);

    return rc;
}

/**
* @internal prvCpssDxChPclTcamStdRuleStateXorYGet function
* @endinternal
*
* @brief   Reads standard rule X or Y control bits of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xORy                     - GT_TRUE for X, GT_FALSE for Y.
*
* @param[out] dValidPtr                -  xValidPtr or yValidPtr
* @param[out] dSizePtr                 -  xSizePtr or ySizePtr
* @param[out] dSparePtr                -  xSparePtr or ySparePtr
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS prvCpssDxChPclTcamStdRuleStateXorYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       xORy,
    OUT GT_U32                        *dValidPtr,
    OUT GT_U32                        *dSizePtr,
    OUT GT_U32                        *dSparePtr
)
{
    GT_U32       data;        /* register data                           */
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddr;     /* register address                        */
    GT_U32       validBitOffset; /* valid bit offset in control word     */
    GT_U32       spareBitOffset; /* spare bit offset in control word     */

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, 1, xORy, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpPortGroupReadRegister(
        devNum, portGroupId, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
             == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        validBitOffset = 17;
        spareBitOffset = 16;
    }
    else /* xCat & xCat3 */
    {
        validBitOffset = 16;
        spareBitOffset = 17;
    }

    *dSizePtr  = (data >> 18) & 3;
    *dValidPtr = (data >> validBitOffset) & 1;
    *dSparePtr = (data >> spareBitOffset) & 1;

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamStdRuleStateXandYGet function
* @endinternal
*
* @brief   Reads standard rule X and Y control bits of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
*
* @param[out] xValidPtr                -  xValidPtr
* @param[out] yValidPtr                -  yValidPtr
* @param[out] xSizePtr                 -  xSizePtr
* @param[out] ySizePtr                 -  ySizePtr
* @param[out] xSparePtr                -  xSparePtr
* @param[out] ySparePtr                -  ySparePtr
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleStateXandYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *xValidPtr,
    OUT GT_U32                        *yValidPtr,
    OUT GT_U32                        *xSizePtr,
    OUT GT_U32                        *ySizePtr,
    OUT GT_U32                        *xSparePtr,
    OUT GT_U32                        *ySparePtr
)
{
    GT_STATUS    rc;          /* return code                             */

    rc = prvCpssDxChPclTcamStdRuleStateXorYGet(
            devNum, portGroupId, ruleIndex, GT_TRUE,
            xValidPtr, xSizePtr, xSparePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamStdRuleStateXorYGet(
            devNum, portGroupId, ruleIndex, GT_FALSE,
            yValidPtr, ySizePtr, ySparePtr);

    return rc;
}

/**
* @internal prvCpssDxChPclTcamStdRuleValidStateXorYSet function
* @endinternal
*
* @brief   Writes standard rule X or Y valid bit of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      does not support value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xORy                     - GT_TRUE for X, GT_FALSE for Y.
* @param[in] dValid                   - xValid or yValid
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS prvCpssDxChPclTcamStdRuleValidStateXorYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       xORy,
    IN  GT_U32                        dValid
)
{
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddrData; /* register address                        */
    GT_U32       regAddrCtrl; /* register address                        */
    GT_U32       validBitOffset; /* valid bit offset in control word     */
    GT_U32       regData;     /* register data                           */
    GT_U32       regCtrl;     /* register control                        */

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, 0, xORy, &regAddrData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, 1, xORy, &regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
             == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        validBitOffset = 17;
    }
    else /* xCat & xCat3 */
    {
        validBitOffset = 16;
    }

    rc = prvCpssHwPpPortGroupReadRegister(
        devNum, portGroupId, regAddrData, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpPortGroupReadRegister(
        devNum, portGroupId, regAddrCtrl, &regCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    regCtrl &= (~ (1 << validBitOffset));
    regCtrl |= (dValid << validBitOffset);

    /* rewrite the same data without change           */
    /* must be done before the control register write */
    /* that triggers the TCAM update                  */

    rc = prvCpssHwPpPortGroupWriteRegister(
        devNum, portGroupId, regAddrData, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* verify that all words were written before write last one.
        because last one triggers write of whole entry */
    GT_SYNC;

    rc = prvCpssHwPpPortGroupWriteRegister(
        devNum, portGroupId, regAddrCtrl, regCtrl);

    return rc;
}

/**
* @internal prvCpssDxChPclTcamStdRuleValidStateXandYSet function
* @endinternal
*
* @brief   Writes standard rule X and Y valid bits of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xValid                   -  xValid
* @param[in] yValid                   -  yValid
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleValidStateXandYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        xValid,
    IN  GT_U32                        yValid
)
{
    GT_STATUS    rc, rc1;     /* return code                  */
    GT_U32       savedStatus; /* ECC interrupt enable/disable */

    /* disable interrupts */
    rc = prvCpssDxChPclErrataTcamEccInterruptLock(
        devNum, portGroupId, GT_TRUE/*lock*/, &savedStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamStdRuleValidStateXorYSet(
            devNum, portGroupId, ruleIndex, GT_TRUE, xValid);
    if (rc != GT_OK)
    {
        /* clean and enable interrupts */
        rc1 = prvCpssDxChPclErrataTcamEccInterruptLock(
            devNum, portGroupId, GT_FALSE/*lock*/, &savedStatus);
        return rc;
    }

    rc = prvCpssDxChPclTcamStdRuleValidStateXorYSet(
            devNum, portGroupId, ruleIndex, GT_FALSE, yValid);

    /* clean and enable interrupts */
    rc1 = prvCpssDxChPclErrataTcamEccInterruptLock(
        devNum, portGroupId, GT_FALSE/*lock*/, &savedStatus);

    if (rc != GT_OK)
    {
        return rc;
    }

    return rc1;
}

/**
* @internal prvCpssDxChPclTcamStdRuleValidStateSet function
* @endinternal
*
* @brief   Writes standard rule X and Y valid bits
*         The algorithms for xCat2 and above and
*         the algorithm for DxCh3, xCat, xCat3, Lion are different.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] valid                    -  valid
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleValidStateSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       valid
)
{
    GT_STATUS    rc;          /* return code         */
    GT_U32       xValid;      /* xValid              */
    GT_U32       yValid;      /* yValid              */
    GT_U32       regAddr;     /* register address    */
    GT_U32       regData[2];  /* register data       */
    GT_U32       i;           /* loop index          */
    GT_U32       yBitsWrite;  /* Y bits to write     */
    GT_BOOL      enable = GT_FALSE;  /* state of daemon */
    GT_U32       tmpValue = 0;/* temp value for calculations */
    GT_U32       yParity;     /* Y parity */

    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* DxCh3; xCat; xCat3; Lion; */
        if (valid == GT_FALSE)
        {
            /* any "valid bit" value will not be matched in the key */
            xValid = PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(0, 1);
            yValid = PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(0, 1);
        }
        else
        {
            /* "valid bit" value 1 only will be matched in the key */
            xValid = PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(1, 1);
            yValid = PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(1, 1);
        }

        return prvCpssDxChPclTcamStdRuleValidStateXandYSet(
            devNum, portGroupId, ruleIndex, xValid, yValid);
    }

    /* xCat2 and above */

    /* calculate new value for Y-bits for rule size (2 bits) */
    /* the X-bits of the rule never updated                  */

    /* The yBitsWrite here gets all 32 bits                  */
    /* The proper 2 bits that contain rule size will be      */
    /* cut off it below - at bit-replacing algorithm         */

    if (valid == GT_FALSE)
    {
        /* For the rule written to TCAM by CPSS         */
        /* X bits of rule size can contain 1,2 or 3     */
        /* in any case at least one pare (x==1 && y==1) */
        /* i.e. never-match pair will be created.       */
        /* Even in the case of the garbage in TCAM if   */
        /* X-bits of the rule size contain 0, the       */
        /* updated rule will match only keys that       */
        /* contain 0 in rule size bits - that cannot be.*/

        yBitsWrite = 0xFFFFFFFF;
    }
    else
    {
        rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
                devNum, ruleIndex, 0, GT_TRUE, &regAddr);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr, &(regData[0]));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* exact matching of the first 32 X - bits */
        yBitsWrite = (~ regData[0]);
    }

    /* Update the Y-bits of rule size                            */
    /* Two words must be written to TCAM.                        */
    /* The second word not changed but triggers the TCAM update. */

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, 0, GT_FALSE, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; (i < 2); i++)
    {
        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, (regAddr + (4 * i)), &(regData[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Disable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        /* store current state */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, &enable);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* store current state */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }

        tmpValue = regData[0];
    }

    /* Replacing two Y bits at the position of the size of rule */
    regData[0] &= (~ 3);
    regData[0] |= (yBitsWrite & 3);

    rc = prvCpssDxChPclWriteTabEntry(
        devNum, portGroupId,
        regAddr, 2, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Restore TCAM Parity Daemon and update Y Parity */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadMaskParityBase;

        /* Read Parity_Y table */
        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, (regAddr + (ruleIndex * 4)), &yParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        tmpValue = (tmpValue & 0x3) ^ (yParity & 0x3);
        U32_SET_FIELD_MAC(yParity, 0, 2, tmpValue);
        tmpValue = (regData[0] & 0x3) ^ (yParity & 0x3);
        U32_SET_FIELD_MAC(yParity, 0, 2, tmpValue);

        /* Write Parity_Y table */
        rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, (regAddr + (ruleIndex * 4)), yParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Restore Previous State */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamStdRuleXorYSet function
* @endinternal
*
* @brief   Writes standard rule X or Y data and control bits
*         The first 48-bit column control bits supposed to be invalid and will
*         be overwritten last
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xORy                     - GT_TRUE for X, GT_FALSE for Y.
* @param[in] dPtr                     - data:  6 words for X or Y
* @param[in] dValid                   - xValid or yValid bit of the word0
* @param[in] dValid                   - xValid or yValid of words 1-5
* @param[in] dSize                    - xSize or ySize
* @param[in] dSpare                   - xSpare or ySpare
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS prvCpssDxChPclTcamStdRuleXorYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       xORy,
    IN  GT_U32                        *dPtr,
    IN  GT_U32                        dValid0,
    IN  GT_U32                        dValid,
    IN  GT_U32                        dSize,
    IN  GT_U32                        dSpare
)
{
    GT_32        i;           /* loop index                              */
    GT_32        wordIndex;   /* word index derived from the loop index  */
    GT_U32       data;        /* word register data                      */
    GT_U32       ctrl;        /* control register data                   */
    GT_STATUS    rc;          /* return code                             */
    GT_U32       regAddrData; /* data register address                   */
    GT_U32       regAddrCtrl; /* data register address                   */
    GT_U32       validBitOffset; /* valid bit offset in control word     */
    GT_U32       spareBitOffset; /* spare bit offset in control word     */

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
             == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        validBitOffset = 17;
        spareBitOffset = 16;
    }
    else /* xCat & xCat3 */
    {
        validBitOffset = 16;
        spareBitOffset = 17;
    }

    /* size and spare control bits are the same for all segments. */
    /* valid control bit is the same for all segments when setting rule to valid status, when setting   */
    /* rule to invalid status it is enough to invalid one segment, in our case the first one is choosen */
    ctrl = ((dSize & 3) << 18) | ((dSpare & 1) << spareBitOffset) | ((dValid0 & 1) << validBitOffset);

    /* during each period two words are written. The first contains only data, while */
    /* the second contains data, control bits and triggers the TCAM write operation. */
    for (i = 0; (i < 8) ; i+=2)
    {
        /* descending or ascending order for valid or invalid operation respectively. */
        /* When setting valid status the first segment is written last.               */
        /* When setting invalid status the first segment is written first.            */
        /* Setting invalid status is done by setting invalid for the first segment only, */
        /* therefore when all valid bits have the same value we are in the case of       */
        /* setting valid status to the rule, when invalid status is requested the valid  */
        /* bit value of the first segment is opposite to the other segments valid bits   */
        /* value.                                                                        */
        wordIndex = ( dValid0 == dValid ) ? ( 6 - i ) : i ;
        ctrl &= 0xFFFF0000;

        switch(wordIndex)
        {
            case 0: data =  dPtr[0];
                    ctrl |= (dPtr[1] & 0x000FFFF);
                    break;
            case 2: /* In ascending order, we previously set the valid bit of the first */
                    /* segment to invalid status, and from now on valid status is set   */
                    /* for the other segments.                                          */
                    /* In descending order, used when valid status is set for all       */
                    /* segments valid bit setting has no influence since                */
                    /* dValid0 == dValid.                                               */
                    data =  (dPtr[1] >> 16) & 0x0000FFFF;
                    data |= (dPtr[2] << 16) & 0xFFFF0000;
                    ctrl |= (dPtr[2] >> 16) & 0x0000FFFF;
                    ctrl &= (~(1 << validBitOffset));
                    ctrl |= ((dValid & 1) << validBitOffset);
                    break;
            case 4: data =  dPtr[3];
                    ctrl |= (dPtr[4] & 0x0000FFFF);
                    break;
            case 6: data =  (dPtr[4] >> 16) & 0x0000FFFF;
                    data |= (dPtr[5] << 16) & 0xFFFF0000;
                    ctrl |= (dPtr[5] >> 16) & 0x0000FFFF;
                    break;

            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* this code should never be executed */
        }

        rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, wordIndex, xORy, &regAddrData);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
            devNum, ruleIndex, (wordIndex + 1), xORy, &regAddrCtrl);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpPortGroupWriteRegister(
            devNum, portGroupId, regAddrData, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* verify that all words were written before write last one.
            because last one triggers write of whole entry */
        GT_SYNC;

        rc = prvCpssHwPpPortGroupWriteRegister(
            devNum, portGroupId, regAddrCtrl, ctrl);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamStdRuleXandYSet function
* @endinternal
*
* @brief   Writes standard rule X and Y data and control bits
*         The first 48-bit column control bits supposed to be invalid and will
*         be overwritten last
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xPtr                     - data:  6 words for X
* @param[in] yPtr                     - data:  6 words for Y
* @param[in] xValid                   -  bit of the word0
* @param[in] yValid                   -  bit of the word0
* @param[in] xValid                   -   bit of the words 1-5
* @param[in] yValid                   -   bit of the words 1-5
* @param[in] xSize                    - xSize
* @param[in] ySize                    - ySize
* @param[in] xSpare                   - xSpare
* @param[in] ySpare                   - ySpare
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleXandYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *xPtr,
    IN  GT_U32                        *yPtr,
    IN  GT_U32                        xValid0,
    IN  GT_U32                        yValid0,
    IN  GT_U32                        xValid,
    IN  GT_U32                        yValid,
    IN  GT_U32                        xSize,
    IN  GT_U32                        ySize,
    IN  GT_U32                        xSpare,
    IN  GT_U32                        ySpare
)
{
    GT_STATUS    rc;          /* return code                             */

    rc = prvCpssDxChPclTcamStdRuleXorYSet(
            devNum, portGroupId, ruleIndex, GT_TRUE,
            xPtr, xValid0, xValid, xSize, xSpare);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamStdRuleXorYSet(
            devNum, portGroupId, ruleIndex, GT_FALSE,
            yPtr, yValid0, yValid, ySize, ySpare);

    return rc;
}

/**
* @internal prvCpssDxChPclTcamStdRuleSizeAndValidSet function
* @endinternal
*
* @brief   Writes standard rule X and Y data and control bits
*         The previous TCAM rule state supposed to be invalid and will
*         be overwritten last.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xPtr                     - data:  6 words for X
* @param[in] yPtr                     - data:  6 words for Y
* @param[in] xPtr                     and yPtr for xCat2 and above devices are 7 words
*                                      The contain 206-bit data starting with pclId and
*                                      not contain the 2-bit rule size.
* @param[in] validRule                - valid
* @param[in] ruleSize                 - rule Size (APPLICABLE RANGES: 1..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleSizeAndValidSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *xPtr,
    IN  GT_U32                        *yPtr,
    IN  GT_BOOL                       validRule,
    IN  GT_U32                        ruleSize
)
{
    GT_STATUS    rc;           /* return cose                 */
    GT_U32       regAddr;      /* register address            */
    GT_U32       hwRuleSize;   /* hw Rule Size                */
    GT_U32       xValid;       /* xValid                      */
    GT_U32       yValid;       /* yValid                      */
    GT_U32       yRuleSize;    /* 2-bit Y-value for rule size */
    GT_U32       i;            /* loop index                  */
    GT_U32       xData[2];     /* X data to write             */
    GT_U32       yData[2];     /* Y data to write             */
    GT_U32       xRegBase;     /* regiser address base fo X   */
    GT_U32       yRegBase;     /* regiser address base fo Y   */
    GT_U32       xDataArr[8];  /* buffer accumulator for X */
    GT_U32       yDataArr[8];  /* buffer accumulator for Y */
    GT_U32       xParity;      /* calculated parity X */
    GT_U32       yParity;      /* calculated parity X */
    GT_BOOL      enable = GT_FALSE; /* for storing daemon status */

    switch (ruleSize)
    {
        case 1: hwRuleSize = 1; break;
        case 2: hwRuleSize = 2; break;
        case 3: hwRuleSize = 3; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* CH3, xCat, xCat3, Lion */
        if (validRule == GT_FALSE)
        {
            /* any "valid bit" value will not be matched in the key */
            xValid = PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(0, 1);
            yValid = PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(0, 1);
        }
        else
        {
            /* "valid bit" value 1 only will be matched in the key */
            xValid = PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(1, 1);
            yValid = PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(1, 1);
        }

        return prvCpssDxChPclTcamStdRuleXandYSet(
            devNum, portGroupId, ruleIndex, xPtr, yPtr,
            xValid /*xValid0*/, yValid /*yValid0*/,
            PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(1, 1) /*xValid*/,
            PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(1, 1) /*yValid*/,
            PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(3, hwRuleSize) /*xSize*/,
            PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(3, hwRuleSize) /*ySize*/,
            PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(0, 0) /*xSpare*/,
            PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(0, 0) /*xSpare*/);
    }

    /* xCat2 and above */

    /* if Y==3 provides at least of one pair <x==1&&y==1> i.e. never match */
    /* the hwRuleSize always written to X rule size bits                   */
    /* hwRuleSize == PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(3, hwRuleSize)          */
    yRuleSize = (validRule == GT_FALSE) ? 3
        : PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(3, hwRuleSize);

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
        devNum, ruleIndex, 0, GT_TRUE, &xRegBase);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPclTcamReadStdRuleAddrGet(
        devNum, ruleIndex, 0, GT_FALSE, &yRegBase);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* writing 52-bit groups 3,2,1,0 */
    /* 7 words contain 32+32+32+32+32+32+14 == 206 bits */

    /* Disable TCAM Parity Daemon */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        /* store current state */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, &enable);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* store current state */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    for (i = 4; (i > 0); i--)
    {
        switch (i)
        {
            case 4:
                /* the 3-th 52 bit - 6+26 + 6+14 */
                xData[0] = ((xPtr[4] >> 26) & 0x3F) | ((xPtr[5] & 0x3FFFFFF) << 6);
                yData[0] = ((yPtr[4] >> 26) & 0x3F) | ((yPtr[5] & 0x3FFFFFF) << 6);
                xData[1] = ((xPtr[5] >> 26) & 0x3F) | ((xPtr[6] & 0x3FFF) << 6);
                yData[1] = ((yPtr[5] >> 26) & 0x3F) | ((yPtr[6] & 0x3FFF) << 6);
                break;
            case 3:
                /* the 2-th 52 bit - 26+6 + 20 */
                xData[0] = ((xPtr[3] >> 6) & 0x3FFFFFF) | ((xPtr[4] & 0x3F) << 26);
                yData[0] = ((yPtr[3] >> 6) & 0x3FFFFFF) | ((yPtr[4] & 0x3F) << 26);
                xData[1] = ((xPtr[4] >> 6) & 0xFFFFF);
                yData[1] = ((yPtr[4] >> 6) & 0xFFFFF);
                break;
            case 2:
                /* the 1-th 52 bit - 14+18 + 14+6 */
                xData[0] = ((xPtr[1] >> 18) & 0x3FFF) | ((xPtr[2] & 0x3FFFF) << 14);
                yData[0] = ((yPtr[1] >> 18) & 0x3FFF) | ((yPtr[2] & 0x3FFFF) << 14);
                xData[1] = ((xPtr[2] >> 18) & 0x3FFF) | ((xPtr[3] & 0x3F) << 14);
                yData[1] = ((yPtr[2] >> 18) & 0x3FFF) | ((yPtr[3] & 0x3F) << 14);
                break;
            case 1:
                /* the 0-th 52 bit - 2+30 && 2+18 */
                xData[0] = hwRuleSize | (xPtr[0] << 2);
                yData[0] = yRuleSize  | (yPtr[0] << 2);
                xData[1] = ((xPtr[0] >> 30) & 3) | ((xPtr[1] & 0x3FFFF) << 2);
                yData[1] = ((yPtr[0] >> 30) & 3) | ((yPtr[1] & 0x3FFFF) << 2);
                break;
            default: break;
        }


        /* Parity Daemon Table support */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
        {
            xDataArr[(i-1)*2] = xData[0];
            xDataArr[(i-1)*2 + 1] = xData[1];

            yDataArr[(i-1)*2] = yData[0];
            yDataArr[(i-1)*2 + 1] = yData[1];
        }

        /* write 52 X-bits */
        rc = prvCpssDxChPclWriteTabEntry(
            devNum, portGroupId,
            (xRegBase + ((2 * (i - 1)) * 4)), 2, xData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write 52 Y-bits */
        rc = prvCpssDxChPclWriteTabEntry(
            devNum, portGroupId,
            (yRegBase + ((2 * (i - 1)) * 4)), 2, yData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
    {
        prvCpssDxChLion2PclTcamParityCalc(xDataArr, yDataArr, &xParity, &yParity);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadPatternParityBase;

        /* Write Parity_X table */
        rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, (regAddr + (ruleIndex * 4)), xParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadMaskParityBase;

        /* Write Parity_Y table */
        rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, (regAddr + (ruleIndex * 4)), yParity);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Restore Previous State */
        rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataTo7WordsAdjust function
* @endinternal
*
* @brief   The function adjusts the data converted from SW to HW format
*         to 7 words per standard rule format - devices that use only 2 first
*         bits as control bits (from 52 4 bits of standard rule).
*         1) Drugging high rule segments by such way that each 6-word segment
*         will be padded by the 7-th word contains zero.
*         2) For egress rules the first segment of the rule left shifted to 1
*         because the "bit0 == valid" not exists for xCat2 and above devices.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] isEgress                 - GT_TRUE egress format, GT_FALSE ingress format.
* @param[in] ruleSize                 - amount of Standard segments of rule.
* @param[in,out] hwMaskArr[]              - array with  pattern converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] hwMaskArr[]              - array with  pattern converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
*                                       None.
*
* @note The xCat2 specific "key size" bits are added at low level write
*       function prvCpssDxChPclTcamStdRuleSizeAndValidSet.
*
*/
GT_VOID prvCpssDxChPclRuleDataTo7WordsAdjust
(
    IN     GT_BOOL                                  isEgress,
    IN     GT_U32                                   ruleSize,
    INOUT  GT_U32                                   hwMaskArr[],
    INOUT  GT_U32                                   hwPatternArr[]
)
{
    GT_U32 segIdx;       /* index of rule segment                          */
    GT_U32 wordIdx;      /* index of word                                  */
    GT_U32 segOldBase;   /* index of the old base word of the rule segment */
    GT_U32 segNewBase;   /* index of the new base word of the rule segment */
    GT_U32 i, j;         /* loop index                                     */

    /* move all segments with index more then 0, the 0-th not moved */
    for (i = 1; (i < ruleSize); i++)
    {
        segIdx = ruleSize - i;
        segOldBase = segIdx * 6;
        segNewBase = segIdx * 7;
        for (j = 0; (j <= 5); j++)
        {
            wordIdx = 5 - j;
            hwMaskArr[segNewBase + wordIdx] =
                hwMaskArr[segOldBase + wordIdx];
            hwPatternArr[segNewBase + wordIdx] =
                hwPatternArr[segOldBase + wordIdx];
        }
        /* clear the additional 7-th word */
        hwMaskArr[segNewBase + 6] = 0;
        hwPatternArr[segNewBase + 6] = 0;
    }
    /* clear the additional 7-th word of the first segment */
    hwMaskArr[6] = 0;
    hwPatternArr[6] = 0;

    /* shift the first segment of egress rule*/
    if (isEgress != GT_FALSE)
    {
        for (j = 0; (j <= 5); j++)
        {
            hwMaskArr[j] =
                (((hwMaskArr[j] >> 1) & 0x7FFFFFFF)
                | ((hwMaskArr[j + 1] & 1) << 31));
            hwPatternArr[j] =
                (((hwPatternArr[j] >> 1) & 0x7FFFFFFF)
                | ((hwPatternArr[j + 1] & 1) << 31));
        }
    }
}

/**
* @internal prvCpssDxChPclRuleGetDirAndSize function
* @endinternal
*
* @brief   The function Gets Direction and size of rule format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ruleFormat               - format of the Rule.
*
* @param[out] isEgressPtr              - (pointer to) GT_TRUE - egress, GT_FALSE - ingress
* @param[out] ruleSizePtr              - (pointer to) size of the rule in standard rules
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleGetDirAndSize
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    OUT  GT_BOOL                            *isEgressPtr,
    OUT  GT_U32                             *ruleSizePtr
)
{
    switch (ruleFormat)
    {
        /* ingress STD */
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
            *isEgressPtr = GT_FALSE;
            *ruleSizePtr = 1;
            break;
        /* ingress EXT */
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E:
            *isEgressPtr = GT_FALSE;
            *ruleSizePtr = 2;
            break;
        /* ingress Ultra */
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            *isEgressPtr = GT_FALSE;
            *ruleSizePtr = 3;
            break;
        /* egress STD */
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E:
            *isEgressPtr = GT_TRUE;
            *ruleSizePtr = 1;
            break;
        /* egress EXT */
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E:
            *isEgressPtr = GT_TRUE;
            *ruleSizePtr = 2;
            break;
        /* egress Ultra */
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
            *isEgressPtr = GT_TRUE;
            *ruleSizePtr = 3;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
            *isEgressPtr = GT_FALSE;
            *ruleSizePtr = 0;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E:
            *isEgressPtr = GT_TRUE;
            *ruleSizePtr = 0;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataXCat2NewFieldsAdd function
* @endinternal
*
* @brief   The function adds xCat2 specific fields to mask and pattern
*         already converted to HW format.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] ruleFormat               - format of the Rule.
* @param[in] maskPtr                  - rule mask.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in,out] hwMaskArr[]              - array with mask
* @param[in,out] hwPatternArr[]           - array with pattern
* @param[in,out] hwMaskArr[]              - array with mask adjusted for xCat2
* @param[in,out] hwPatternArr[]           - array with pattern adjusted for xCat2
*                                       None.
*
* @note Not treats port list bitmap.
*
*/
static GT_VOID prvCpssDxChPclRuleDataXCat2NewFieldsAdd
(
    IN     CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN     CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN     CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    INOUT  GT_U32                             hwMaskArr[],
    INOUT  GT_U32                             hwPatternArr[]
)
{
    GT_U32       encapHighBitMask;     /* the mask of high bit of encapsulation     */
    GT_U32       encapHighBitPattern;  /* the pattern of high bit of encapsulation  */
    GT_U32       encapHighBitWord;     /* the word num of high bit of encapsulation */
    GT_U32       encapHighBitPos;      /* the position of high bit of encapsulation */
    GT_U32       workMask;             /* the mask of high bit of encapsulation     */
    GT_U32       workPattern;          /* the pattern of high bit of encapsulation  */

    encapHighBitMask    = 0;
    encapHighBitPattern = 0;
    encapHighBitPos     = 0;
    encapHighBitWord    = 0;

    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            encapHighBitMask =
                ((maskPtr->ruleStdNotIp.l2Encap >> 1) & 1);
            encapHighBitPattern =
                ((patternPtr->ruleStdNotIp.l2Encap >> 1) & 1);
            encapHighBitPos  = 13;
            encapHighBitWord = 3;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            encapHighBitMask =
                ((maskPtr->ruleExtNotIpv6.l2Encap >> 1) & 1);
            encapHighBitPattern =
                ((patternPtr->ruleExtNotIpv6.l2Encap >> 1) & 1);
            encapHighBitPos  = 14;
            encapHighBitWord = 3;
            break;
        default: break;
    }

    if (encapHighBitMask != 0)
    {
        workMask = (1 << encapHighBitPos);
        /* update mask */
        workPattern = (encapHighBitMask << encapHighBitPos);
        hwMaskArr[encapHighBitWord] &= (~ workMask);
        hwMaskArr[encapHighBitWord] |= workPattern;
        /* update pattern */
        workPattern = (encapHighBitPattern << encapHighBitPos);
        hwPatternArr[encapHighBitWord] &= (~ workMask);
        hwPatternArr[encapHighBitWord] |= workPattern;
    }
}


/**
* @internal prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers function
* @endinternal
*
* @brief   The function Gets pointers of source fields modified for port group id.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] ruleFormat               - format of the Rule.
* @param[in] rulePtr                  - (pointer to) rule.
*
* @param[out] fldSrcPtr                - (pointer to) structure to get pointers into rulePtr .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT                      ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                           *rulePtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC *fldSrcPtr
)
{
    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleStdNotIp.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleStdNotIp.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleStdIpL2Qos.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleStdIpL2Qos.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleStdIpv4L4.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleStdIpv4L4.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleStdIpv6Dip.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleStdIpv6Dip.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleExtNotIpv6.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleExtNotIpv6.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleExtIpv6L2.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleExtIpv6L2.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleExtIpv6L4.common.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleExtIpv6L4.common.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrStdNotIp.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrStdIpL2Qos.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrStdIpv4L4.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrExtNotIpv6.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrExtIpv6L2.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleEgrExtIpv6L4.common.portListBmp);
            fldSrcPtr->srcPortPtr = NULL;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleIngrStdUdb.commonIngrUdb.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleIngrStdUdb.commonIngrUdb.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleIngrExtUdb.commonIngrUdb.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleIngrExtUdb.commonIngrUdb.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleStdIpv4RoutedAclQos.ingressIpCommon.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleExtIpv4PortVlanQos.ingressIpCommon.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            fldSrcPtr->portListBmpPtr =
                &(rulePtr->ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
            fldSrcPtr->srcPortPtr =
                &(rulePtr->ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourcePort);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E:
            fldSrcPtr->portListBmpPtr = NULL;/* not support ports bmp */
            fldSrcPtr->srcPortPtr = NULL;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleGetModifiedFieldsSources function
* @endinternal
*
* @brief   The function Gets source of fields modified for port group id.
*         and the port list bmp needed from xCat2.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] ruleFormat               - format of the Rule.
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattren
*
* @param[out] fldSrcPtr                - (pointer to) structure to get results in it.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
* @note The function used also for xCat2 becase the port list bitmap
*       treated only via this function.
*
*/
GT_STATUS prvCpssDxChPclRuleGetModifiedFieldsSources
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT                       ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                            *maskPtr,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                            *patternPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC *fldSrcPtr
)
{
    GT_STATUS rc; /* return code                                       */
    PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC fldSrcPointers;
                  /* pointers to srcPort ans portListBmp is rule union */

    rc = prvCpssDxChPclRuleGetDirAndSize(
        ruleFormat, &(fldSrcPtr->isEgress), &(fldSrcPtr->ruleSize));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* mask related data */
    rc = prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers(
        ruleFormat, maskPtr, &fldSrcPointers);
    if (rc != GT_OK)
    {
        return rc;
    }

    fldSrcPtr->srcPortMask        =
        (fldSrcPointers.srcPortPtr != NULL)
            ? *(fldSrcPointers.srcPortPtr) : 0;
    fldSrcPtr->portListBmpMaskPtr = fldSrcPointers.portListBmpPtr;

    /* pattern related data */
    rc = prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers(
        ruleFormat, patternPtr, &fldSrcPointers);
    if (rc != GT_OK)
    {
        return rc;
    }

    fldSrcPtr->srcPortPattern        =
        (fldSrcPointers.srcPortPtr != NULL)
            ? *(fldSrcPointers.srcPortPtr) : 0;
    fldSrcPtr->portListBmpPatternPtr = fldSrcPointers.portListBmpPtr;

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataConvertForPortGroup function
* @endinternal
*
* @brief   The function converts source port and port bitmap fields values
*         according to port-group Id.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port Groups Id for multi port group devices
* @param[in] fldSrcPtr                - source values of the fields
*
* @param[out] storeFldPtr              - converted values of the fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleDataConvertForPortGroup
(
    IN   GT_U8                                                     devNum,
    IN   GT_U32                                                    portGroupId,
    IN   PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC *fldSrcPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC  *storeFldPtr
)
{
    GT_STATUS               rc;                   /* return code             */
    GT_U32                  portGroupIdMask;      /* port group Id Mask      */
    GT_U32                  portGroupIdPattern;   /* port group Id  Pattern  */
    CPSS_PP_FAMILY_TYPE_ENT family;               /* device family           */
    GT_U32                  localBmp;             /* local port bitmap       */
    GT_U32                  startBit;             /* start bit in word       */
    GT_U32                  wordIdx;              /* index of word           */
    GT_BOOL                 isLion2;              /* GT_TRUE for Lion2 devs  */
    GT_U32                  srcPortSwMask;        /* mask for MAX SW srcPort */
    GT_U32                  srcPortHwMask;        /* mask for MAX HW srcPort */

    /* default, overriden below */
    isLion2 = GT_FALSE;
    family   = PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily;
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* treat Lion2 as lion */
        isLion2 = GT_TRUE;
    }

    storeFldPtr->isEgress  = fldSrcPtr->isEgress;
    storeFldPtr->ruleSize  = fldSrcPtr->ruleSize;
    /* no conversion, just copy to be stored */
    storeFldPtr->srcPortMask = fldSrcPtr->srcPortMask;
    storeFldPtr->srcPortPattern = fldSrcPtr->srcPortPattern;
    storeFldPtr->portListBmpMask = 0;
    storeFldPtr->portListBmpPattern = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if( NULL != fldSrcPtr->portListBmpMaskPtr )
        {
            /* make sure that application understand that only 28 bits are used */
            rc = prvCpssDxChTablePortsBmpLimitedNumCheck(devNum,
                    SIP5_PORT_LIST_NUM_BITS_CNS,/*28*/
                    fldSrcPtr->portListBmpMaskPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The mask <portList> bitmap supports only 28 LSBits");
            }

            storeFldPtr->portListBmpMask =
                fldSrcPtr->portListBmpMaskPtr->ports[0];
        }

        if( NULL != fldSrcPtr->portListBmpPatternPtr )
        {
            /* make sure that application understand that only 28 bits are used */
            rc = prvCpssDxChTablePortsBmpLimitedNumCheck(devNum,
                    SIP5_PORT_LIST_NUM_BITS_CNS,/*28*/
                    fldSrcPtr->portListBmpPatternPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The pattern <portList> bitmap supports only 28 LSBits");
            }

            storeFldPtr->portListBmpPattern =
                fldSrcPtr->portListBmpPatternPtr->ports[0];
        }

        return GT_OK;
    }


    switch (family)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            /* portlist not relevant to Lion devices         */
            /* For Lion2 must be treated converting portNum */
            /* to local port                                 */
            storeFldPtr->portListBmpMask = 0;
            storeFldPtr->portListBmpPattern = 0;
            if (isLion2 == GT_TRUE &&
                fldSrcPtr->portListBmpMaskPtr &&
                fldSrcPtr->portListBmpPatternPtr)
            {
                /* 16 bit per port group */
                startBit     = (portGroupId & 1) ? 16 : 0;
                wordIdx      = (portGroupId >> 1);

                /* mask conversion */

                /* local ports */
                localBmp =
                    (fldSrcPtr->portListBmpMaskPtr->ports[wordIdx]
                      >> startBit);
                storeFldPtr->portListBmpMask |= (localBmp & 0x0FFF);

                /* NULL port and CPU port */
                localBmp =
                    (fldSrcPtr->portListBmpMaskPtr->ports
                      [CPSS_CPU_PORT_NUM_CNS >> 5]
                      >> ((CPSS_CPU_PORT_NUM_CNS & 0x10) ? 16 : 0));
                storeFldPtr->portListBmpMask |= (localBmp & 0xF000);

                /* pattern conversion */

                /* local ports */
                localBmp =
                    (fldSrcPtr->portListBmpPatternPtr->ports[wordIdx]
                      >> startBit);
                storeFldPtr->portListBmpPattern |= (localBmp & 0x0FFF);

                /* NULL port and CPU port */
                localBmp =
                    (fldSrcPtr->portListBmpPatternPtr->ports
                      [CPSS_CPU_PORT_NUM_CNS >> 5]
                      >> ((CPSS_CPU_PORT_NUM_CNS & 0x10) ? 16 : 0));
                storeFldPtr->portListBmpPattern |= (localBmp & 0xF000);
            }

            /* source port conversion */

            srcPortSwMask = (isLion2 != GT_TRUE) ? 0x3F : 0x7F;
            srcPortHwMask = 0x3F;

            if ((fldSrcPtr->srcPortMask & fldSrcPtr->srcPortPattern)
                != fldSrcPtr->srcPortPattern)
            {
                /* invalid condition - require NULL port */
                storeFldPtr->srcPortMask = srcPortHwMask;
                storeFldPtr->srcPortPattern =
                    PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
            }
            else if ((storeFldPtr->isEgress == GT_FALSE)
                /* exclude CPU port and some other big values exact matching */
                && ((fldSrcPtr->srcPortPattern & (srcPortSwMask & (~ 3))) != 0x3C)
                && (fldSrcPtr->srcPortMask != 0))
            {
                /* ingress - local port */
                portGroupIdMask     =
                    ((fldSrcPtr->srcPortMask & srcPortSwMask) >> 4);
                portGroupIdPattern  =
                    ((fldSrcPtr->srcPortPattern & srcPortSwMask) >> 4);
                if ((portGroupIdMask & portGroupId) == portGroupIdPattern)
                {
                    /* match local port number only */
                    storeFldPtr->srcPortMask =
                        (fldSrcPtr->srcPortMask & 0x0F) | (srcPortHwMask & (~ 0x0F));
                    storeFldPtr->srcPortPattern =
                        fldSrcPtr->srcPortPattern & 0x0F;
                }
                else
                {
                    /* not compatible with port group - require NULL port */
                    storeFldPtr->srcPortMask = srcPortHwMask;
                    storeFldPtr->srcPortPattern =
                        PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
                }
            }
            else
            {
                /* no srcPort conversion    */
                /* cases are:               */
                /* egress                   */
                /* big port 0x3C-0x3F       */
                /* srcPort mask==pattern==0 */
                storeFldPtr->srcPortMask = fldSrcPtr->srcPortMask;
                storeFldPtr->srcPortPattern = fldSrcPtr->srcPortPattern;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            if(fldSrcPtr->portListBmpMaskPtr &&
               fldSrcPtr->portListBmpPatternPtr)
            {
                storeFldPtr->portListBmpMask =
                    fldSrcPtr->portListBmpMaskPtr->ports[0];
                storeFldPtr->portListBmpPattern =
                    fldSrcPtr->portListBmpPatternPtr->ports[0];
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataStorePortGroupModifiedFields function
* @endinternal
*
* @brief   The function stores such fields that values of them
*         depend of port-group owning the TCAM
*         For Bobcat2; Caelum; Bobcat3; Aldrin they are source port and port list bitmap.
*         The field value calculation done out of this function.
*         This function just adds the values to rule in HW format.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] addedFldPtr              - fields with port-group depended values
* @param[in,out] hwMaskArr[]              - array with mask converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] hwMaskArr[]              - array with mask adjusted for xCat2
* @param[in,out] hwPatternArr[]           - array with pattern adjusted for xCat2
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong addedFldPtr data
*/
GT_STATUS prvCpssDxChPclRuleDataStorePortGroupModifiedFields
(
    IN     GT_U8                                                    devNum,
    IN     PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC *addedFldPtr,
    INOUT  GT_U32                                                   hwMaskArr[],
    INOUT  GT_U32                                                   hwPatternArr[]
)
{
    GT_BOOL srcPortNoMatch; /* srcPort mask/pattern never match     */
    GT_BOOL portBmpNoMatch; /* portListBmp mask/pattern never match */
    GT_U32  numBits;/* number of bit in current sub field*/
    GT_U32  ruleStartBit;/*start bit in hwMaskArr ,hwPatternArr */
    GT_U32  portsBmpStartBit;/* start bit in addedFldPtr->portListBmpMask, addedFldPtr->portListBmpPattern */
    GT_U32  *ruleStartBitsPtr;/* pointer to : (IPLC/EPCL) start bits for the ports bmp parts */
    GT_U32  value;/*temp value*/
    GT_U32  ii;/*iterator*/


    /* check source port pattern and mask.
       pattern must comprize same bits as mask. */
    srcPortNoMatch =
        ((addedFldPtr->srcPortMask & addedFldPtr->srcPortPattern)
         != addedFldPtr->srcPortPattern)
        ? GT_TRUE : GT_FALSE;
    if (srcPortNoMatch != GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((addedFldPtr->isEgress == GT_FALSE)
        && (addedFldPtr->srcPortMask != 0) &&
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* ingress PCL */
        /* srcPort[5:0] to rule[16:11] */
        hwMaskArr[0]    &= (~ (0x3F << 11));
        hwPatternArr[0] &= (~ (0x3F << 11));
        hwMaskArr[0] |=
            ((addedFldPtr->srcPortMask & 0x3F) << 11);
        hwPatternArr[0] |=
            ((addedFldPtr->srcPortPattern & 0x3F) << 11);
    }

    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* Lion devices - portListBmp not relevant */
        return GT_OK;
    }

    /* check port list bitmap pattern and mask.
       pattern must comprize same bits as mask. */
    portBmpNoMatch =
        ((addedFldPtr->portListBmpMask & addedFldPtr->portListBmpPattern)
         != addedFldPtr->portListBmpPattern)
        ? GT_TRUE : GT_FALSE;
    if (portBmpNoMatch != GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((addedFldPtr->isEgress == GT_FALSE)
        && (addedFldPtr->srcPortMask != 0)
        && (addedFldPtr->portListBmpMask != 0))
    {
        /* muxed fields - no more the one of them must be used */
        /* These fields muxed for ingress formats only         */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (addedFldPtr->portListBmpMask != 0)
    {
        /* OR portListBmp with the data */

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            if (addedFldPtr->isEgress == GT_FALSE)
            {
                /*IPCL*/
                ruleStartBitsPtr = ingressPortsBmpRuleStartBitsArr;
            }
            else
            {
                /*EPCL*/
                ruleStartBitsPtr = egressPortsBmpRuleStartBitsArr;
            }

            portsBmpStartBit = 0;
            for(ii = 0 ; ii < 3 ; ii++)
            {
                numBits = portsBmpSubFiledLengthArr[ii];
                ruleStartBit = ruleStartBitsPtr[ii];

                U32_GET_FIELD_IN_ENTRY_MAC(&addedFldPtr->portListBmpMask,portsBmpStartBit,numBits,value);
                /* update mask */
                U32_SET_FIELD_IN_ENTRY_MAC(hwMaskArr,ruleStartBit,numBits,value);

                U32_GET_FIELD_IN_ENTRY_MAC(&addedFldPtr->portListBmpPattern,portsBmpStartBit,numBits,value);
                /* update pattern */
                U32_SET_FIELD_IN_ENTRY_MAC(hwPatternArr,ruleStartBit,numBits,value);

                /* accumulate the number of bits already done */
                portsBmpStartBit += numBits;
            }

        }
        else
        {

            /* portListBmp[7:0] to rule[7:0] */
            hwMaskArr[0]    &= (~ 0xFF);
            hwPatternArr[0] &= (~ 0xFF);
            hwMaskArr[0]    |= (addedFldPtr->portListBmpMask & 0xFF);
            hwPatternArr[0] |= (addedFldPtr->portListBmpPattern & 0xFF);

            if (addedFldPtr->isEgress == GT_FALSE)
            {
                /* ingress PCL */
                /* portListBmp[13:8] to rule[16:11] */
                hwMaskArr[0]    &= (~ (0x3F << 11));
                hwPatternArr[0] &= (~ (0x3F << 11));
                hwMaskArr[0] |=
                    ((addedFldPtr->portListBmpMask & 0x3F00) << 3);
                hwPatternArr[0] |=
                    ((addedFldPtr->portListBmpPattern & 0x3F00) << 3);
            }
            else
            {
                /* egress PCL */
                /* portListBmp[13:8] to rule[37:32] */
                hwMaskArr[1]    &= (~ 0x3F);
                hwPatternArr[1] &= (~ 0x3F);
                hwMaskArr[1] |=
                    ((addedFldPtr->portListBmpMask & 0x3F00) >> 8);
                hwPatternArr[1] |=
                    ((addedFldPtr->portListBmpPattern & 0x3F00) >> 8);
            }

            /* portListBmp[27:14] to rule[205:192] */
            hwMaskArr[6]    &= (~ 0x3FFF);
            hwPatternArr[6] &= (~ 0x3FFF);
            hwMaskArr[6] |=
                ((addedFldPtr->portListBmpMask >> 14) & 0x3FFF);
            hwPatternArr[6] |=
                ((addedFldPtr->portListBmpPattern >> 14) & 0x3FFF);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataLoadPortGroupModifiedFields function
* @endinternal
*
* @brief   The function loads such fields that values of them
*         depend of port-group owning the TCAM
*         For Bobcat2; Caelum; Bobcat3; Aldrin they are source port and port list bitmap.
*         This function just loads the values from rule in HW format.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] hwMaskArr[]              - array with mask converted to HW format
* @param[in] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] addedFldPtr              -  isEgress and ruleSize are input fields
* @param[in,out] addedFldPtr              - fields with port-group depended values
*                                       None.
*/
GT_STATUS prvCpssDxChPclRuleDataLoadPortGroupModifiedFields
(
    IN     GT_U8                                                    devNum,
    IN     GT_U32                                                   hwMaskArr[],
    IN     GT_U32                                                   hwPatternArr[],
    INOUT  PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC *addedFldPtr
)
{
    GT_U32  numBits;/* number of bit in current sub field*/
    GT_U32  ruleStartBit;/*start bit in hwMaskArr ,hwPatternArr */
    GT_U32  portsBmpStartBit;/* start bit in addedFldPtr->portListBmpMask, addedFldPtr->portListBmpPattern */
    GT_U32  *ruleStartBitsPtr;/* pointer to : (IPLC/EPCL) start bits for the ports bmp parts */
    GT_U32  value;/*temp value*/
    GT_U32  ii;/*iterator*/

    /* default values */
    addedFldPtr->portListBmpMask     = 0;
    addedFldPtr->portListBmpPattern  = 0;
    addedFldPtr->srcPortMask         = 0;
    addedFldPtr->srcPortPattern      = 0;

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (addedFldPtr->isEgress == GT_FALSE)
        {
            /*IPCL*/
            ruleStartBitsPtr = ingressPortsBmpRuleStartBitsArr;
        }
        else
        {
            /*EPCL*/
            ruleStartBitsPtr = egressPortsBmpRuleStartBitsArr;
        }

        portsBmpStartBit = 0;
        for(ii = 0 ; ii < 3 ; ii++)
        {
            numBits = portsBmpSubFiledLengthArr[ii];
            ruleStartBit = ruleStartBitsPtr[ii];

            U32_GET_FIELD_IN_ENTRY_MAC(hwMaskArr,  ruleStartBit    ,numBits,value);
            /* update bmp */
            U32_SET_FIELD_IN_ENTRY_MAC(&addedFldPtr->portListBmpMask,portsBmpStartBit,numBits,value);


            U32_GET_FIELD_IN_ENTRY_MAC(hwPatternArr,ruleStartBit    ,numBits,value);
            /* update bmp*/
            U32_SET_FIELD_IN_ENTRY_MAC(&addedFldPtr->portListBmpPattern,portsBmpStartBit,numBits,value);

            /* accumulate the number of bits already done */
            portsBmpStartBit += numBits;
        }

        return GT_OK;
    }

    if (addedFldPtr->isEgress == GT_FALSE)
    {
        /* ingress PCL */
        /* srcPort[5:0] from rule[16:11] */
        addedFldPtr->srcPortMask         = ((hwMaskArr[0] >> 11) & 0x3F);
        addedFldPtr->srcPortPattern      = ((hwPatternArr[0] >> 11) & 0x3F);
    }

    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* Lion devices - portListBmp not relevant */
        return GT_OK;
    }

    /* portListBmp[7:0] to rule[7:0] */
    addedFldPtr->portListBmpMask    = (hwMaskArr[0] & 0xFF);
    addedFldPtr->portListBmpPattern = (hwPatternArr[0] & 0xFF);

    if (addedFldPtr->isEgress == GT_FALSE)
    {
        /* ingress PCL */
        /* portListBmp[13:8] to rule[16:11] */
        addedFldPtr->portListBmpMask    |= ((hwMaskArr[0] & 0x1F800) >> 3);
        addedFldPtr->portListBmpPattern |= ((hwPatternArr[0] & 0x1F800) >> 3);
    }
    else
    {
        /* egress PCL */
        /* portListBmp[13:8] to rule[37:32] */
        addedFldPtr->portListBmpMask    |= ((hwMaskArr[1] & 0x3F) << 8);
        addedFldPtr->portListBmpPattern |= ((hwPatternArr[1] & 0x3F) << 8);
    }

    /* portListBmp[27:14] to rule[205:192] */
    addedFldPtr->portListBmpMask    |= ((hwMaskArr[6] & 0x3FFF) << 14);
    addedFldPtr->portListBmpPattern |= ((hwPatternArr[6] & 0x3FFF) << 14);

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataAccumulateFromPortGroup function
* @endinternal
*
* @brief   The function accumulates source port and port bitmap fields values
*         using values retrieved to port-group Id.
*         Conversion done if needed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port Groups Id for multi port group devices
* @param[in] storeFldPtr              - (pointer to)HW values of the fields
*
* @param[out] maskSrcPtrsPtr           - (pointer to)pointers into the mask rule structure
* @param[out] patternSrcPtrsPtr        - (pointer to)pointers into the pattern rule structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleDataAccumulateFromPortGroup
(
    IN   GT_U8                                                      devNum,
    IN   GT_U32                                                     portGroupId,
    IN   PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC   *storeFldPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC    *maskSrcPtrsPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC    *patternSrcPtrsPtr
)
{
    CPSS_PP_FAMILY_TYPE_ENT family;                 /* device family              */
    GT_U32                  localBmp;               /* local port bitmap          */
    GT_U32                  startBit;               /* start bit in word          */
    GT_U32                  wordIdx;                /* index of word              */
    GT_BOOL                 isLion2;                /* GT_TRUE for Lion2 devs     */
    GT_U32                  workMask;               /* work Mask                  */
    GT_U32                  workPattern1;           /* work Pattern               */
    GT_U32                  workPattern0;           /* work Pattern               */
    GT_U32                  swPortGroupBitsInPort;  /* SW port group bits in port */
    GT_U32                  hwPortGroupBitsInPort;  /* HW port group bits in port */
    GT_U32                  swPortOutOfHwBits;      /* SW out of HW port bits     */
    GT_U32                  hwPortMaxBitMask;       /* HW max port bitmask        */

    /* default, overriden below */
    isLion2 = GT_FALSE;
    family   = PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily;
    hwPortGroupBitsInPort = 0x30;
    swPortGroupBitsInPort = 0x30;
    hwPortMaxBitMask = 0x3F;
    swPortOutOfHwBits = 0;
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        isLion2 = GT_TRUE;
        swPortGroupBitsInPort = 0x70;
        swPortOutOfHwBits = 0x40;
    }

    switch (family)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            /* portlist not relevant to Lion devices.         */
            /* For Lion2 must be treated converting portNum   */
            /* to local port.                                 */
            /* Some egress keys do not support port group.    */
            if ((isLion2 == GT_TRUE) &&
                (maskSrcPtrsPtr->portListBmpPtr != NULL))
            {
                /* 16 bit per port group */

                /* local ports */
                startBit     = (portGroupId & 1) ? 16 : 0;
                wordIdx      = (portGroupId >> 1);

                localBmp = (storeFldPtr->portListBmpMask & 0x0FFF);
                maskSrcPtrsPtr->portListBmpPtr->ports[wordIdx] |=
                    (localBmp << startBit);

                localBmp = (storeFldPtr->portListBmpPattern & 0x0FFF);
                patternSrcPtrsPtr->portListBmpPtr->ports[wordIdx] |=
                    (localBmp << startBit);

                /* NULL port and CPU port */
                startBit     = (CPSS_CPU_PORT_NUM_CNS & 0x10) ? 16 : 0;
                wordIdx      = (CPSS_CPU_PORT_NUM_CNS >> 5);

                localBmp = (storeFldPtr->portListBmpMask & 0xF000);
                maskSrcPtrsPtr->portListBmpPtr->ports[wordIdx] |=
                    (localBmp << startBit);

                localBmp = (storeFldPtr->portListBmpPattern & 0xF000);
                patternSrcPtrsPtr->portListBmpPtr->ports[wordIdx] |=
                    (localBmp << startBit);
            }

            /* source port conversion */
            if (((patternSrcPtrsPtr->srcPortPtr == NULL)
                 || (maskSrcPtrsPtr->srcPortPtr == NULL)))
            {
                /* source port data must be ignored for: */
                /* egress key                            */
            }
            else if ((storeFldPtr->srcPortMask & storeFldPtr->srcPortPattern)
                    != storeFldPtr->srcPortPattern)
            {
                /* incorrect rule                        */
                /* skipped                               */
            }
            else if (storeFldPtr->srcPortPattern
                    == PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
            {
                /* null-port stamp rule                  */
                /* kepp the data - may be overriden      */
                if (*(maskSrcPtrsPtr->srcPortPtr) == 0)
                {
                    *(maskSrcPtrsPtr->srcPortPtr)    =
                        (swPortGroupBitsInPort | hwPortMaxBitMask);
                    *(patternSrcPtrsPtr->srcPortPtr) =
                        PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
                }
            }
            else if ((storeFldPtr->srcPortMask  & hwPortGroupBitsInPort)
                     != hwPortGroupBitsInPort)
            {
                /* Rule Set does not generate such rules  */
                /* replicating srcPort rule to portgroups */
                /* skipped                                */
            }
            else if ((storeFldPtr->srcPortPattern & hwPortGroupBitsInPort) == 0)
            {
                /* local port */
                if (*(patternSrcPtrsPtr->srcPortPtr) == PRV_CPSS_DXCH_NULL_PORT_NUM_CNS)
                {
                    /* NULL port rule was kept and will be overriden now */
                    *(maskSrcPtrsPtr->srcPortPtr)    = 0;
                    *(patternSrcPtrsPtr->srcPortPtr) = 0;
                }

                if (*(maskSrcPtrsPtr->srcPortPtr) == 0)
                {
                    /* first obtained data */
                    *(maskSrcPtrsPtr->srcPortPtr)    =
                        (swPortGroupBitsInPort | storeFldPtr->srcPortMask);
                    *(patternSrcPtrsPtr->srcPortPtr) =
                        ((portGroupId << 4) | storeFldPtr->srcPortPattern);
                }
                else if (((*(maskSrcPtrsPtr->srcPortPtr) & 0xF)
                         == storeFldPtr->portListBmpMask)
                         && ((*(patternSrcPtrsPtr->srcPortPtr) & 0xF)
                         == storeFldPtr->srcPortPattern))
                {
                    /* probably same port condition                   */
                    /* drop from  mask and pattern the bits           */
                    /* different in accumulated and new pattern       */

                    workPattern0 = *(patternSrcPtrsPtr->srcPortPtr);
                    workPattern1 =
                        ((portGroupId << 4) | storeFldPtr->srcPortPattern);

                    workMask = (~ (workPattern0 ^ workPattern1));
                    *(maskSrcPtrsPtr->srcPortPtr) &= workMask;
                    *(patternSrcPtrsPtr->srcPortPtr) &= workMask;
                }
            }
            else
            {
                /* glogal port (as CPU port)                       */
                /* or other data not generated by RuleSet function */
                if (*(maskSrcPtrsPtr->srcPortPtr) == 0)
                {
                    /* first obtained data */
                    *(maskSrcPtrsPtr->srcPortPtr)    =
                        (swPortOutOfHwBits | storeFldPtr->srcPortMask);
                    *(patternSrcPtrsPtr->srcPortPtr) =
                        ((portGroupId << 4) | storeFldPtr->srcPortPattern);
                }
                else if (((*(maskSrcPtrsPtr->srcPortPtr) & hwPortMaxBitMask)
                         == storeFldPtr->portListBmpMask)
                         && ((*(patternSrcPtrsPtr->srcPortPtr) & hwPortMaxBitMask)
                         == storeFldPtr->srcPortPattern))
                {
                    /* probably same port condition                   */
                    /* drop from  mask and pattern the bits           */
                    /* different in accumulated and new pattern       */

                    workPattern0 = *(patternSrcPtrsPtr->srcPortPtr);
                    workPattern1 =
                        ((portGroupId << 4) | storeFldPtr->srcPortPattern);

                    workMask = (~ (workPattern0 ^ workPattern1));
                    *(maskSrcPtrsPtr->srcPortPtr) &= workMask;
                    *(patternSrcPtrsPtr->srcPortPtr) &= workMask;
                }
            }

            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPclRuleDataSw2HwConvert function
* @endinternal
*
* @brief   The function Converts the Policy Rule Mask, Pattern and Action
*         to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @param[out] hwMaskArr[]              - array for mask converted to HW format
* @param[out] hwPatternArr[]           - array for pattern converted to HW format
* @param[out] hwActionArr[]            - array for action converted to HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPclRuleDataSw2HwConvert
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN   CPSS_DXCH_PCL_ACTION_STC           *actionPtr,
    OUT  GT_U32                             hwMaskArr[],
    OUT  GT_U32                             hwPatternArr[],
    OUT  GT_U32                             hwActionArr[]
)
{
    GT_STATUS    rc;                   /* return code                 */
    GT_U32       ruleSize;             /* ruleSize in standard rules                */
    GT_BOOL      isEgress;             /* GT_TRUE - egress, GT_FALSE - ingress      */

    /* convert action to HW format */
    rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(
        devNum, actionPtr, hwActionArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert Mask to HW format */
    rc = prvCpssDxChPclTcamRuleDataSw2HwConvert(
        devNum, ruleFormat, maskPtr, hwMaskArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert Pattern to HW format */
    rc = prvCpssDxChPclTcamRuleDataSw2HwConvert(
        devNum, ruleFormat, patternPtr, hwPatternArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Up to here done conversion for PPs from DxCh1 up to Lion */
    if (0 == PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        /* no more manipulations on the format */
        return GT_OK;
    }
    else
    {
        /* These formats implemented using conversion utility  */
        /* Not needed adjustment done below                    */
        switch (ruleFormat)
        {
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
            case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
                return GT_OK;
            default: break;
        }
    }

    rc = prvCpssDxChPclRuleGetDirAndSize(
        ruleFormat, &isEgress, &ruleSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* adjust rule HW format to xCat2 compatible - 7 words per  */
    /* standard rule                                            */

    prvCpssDxChPclRuleDataTo7WordsAdjust(
        isEgress, ruleSize, hwMaskArr, hwPatternArr);

    prvCpssDxChPclRuleDataXCat2NewFieldsAdd(
        ruleFormat, maskPtr, patternPtr,
        hwMaskArr, hwPatternArr);

    return GT_OK;
}

/**
* @internal prvCpssDxChPclTcamRuleModifiedWrite function
* @endinternal
*
* @brief   Writes rule Action, Mask and Pattern to TCAM.
*         Supports rules of any sizes.
*         Supports SrcPort field modification for multi port group devices
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] egressRule               - GT_TRUE - egress Rule, GT_FALSE ingress
* @param[in] validRule                - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
*                                      relevant only for DxCh1 and DxCh2 devices,
*                                      ignored for DxCh3 and above devices
* @param[in] actionPtr                - action
* @param[in] maskPtr                  - mask
* @param[in] patternPtr               - pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamRuleModifiedWrite
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       egressRule,
    IN  GT_BOOL                       validRule,
    IN  GT_BOOL                       waitPrevReady,
    IN  GT_U32                        *actionPtr,
    IN  GT_U32                        *maskPtr,
    IN  GT_U32                        *patternPtr
)
{
    GT_STATUS    rc;            /* return code                        */
    GT_BOOL      multiPortGroupModify;/* to modify rule for different port groups*/
    GT_U32       srcPortMask;      /* src Port Mask                   */
    GT_U32       srcPortPattern;   /* src Port Pattern                */
    GT_U32       portGroupIdMask;       /* port group Id Mask                    */
    GT_U32       portGroupIdPattern;    /* port group Id  Pattern                */
    GT_U32       portGroupId;           /* port group Id                         */

    /* initialization to avoid compiler warnings */
    srcPortMask    = 0;
    srcPortPattern = 0;

    /* default value - write without multi port group modify */
    multiPortGroupModify = GT_FALSE;

    if ((PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        && (egressRule == GT_FALSE))
    {
        /* default value - write with multi port group modify */
        /* overridden due several source port           */
        /* mask anf pattern conditions                  */
        multiPortGroupModify = GT_TRUE;

        srcPortMask    = ((maskPtr[0] >> 11) & 0x3F);
        srcPortPattern = ((patternPtr[0] >> 11) & 0x3F);

        if ((srcPortPattern & srcPortMask) != srcPortPattern)
        {
            /* if there is a bit (in this or any other field) */
            /* that it's mask == 0 and it's pattern == 1      */
            /* this rule does not match any key               */
            /* Must be replaced by "srcPort==NULL_PORT" rule  */

            /* clear srcPort bits in mask and pattern */
            maskPtr[0]    &= (~ (0x3F << 11));
            patternPtr[0] &= (~ (0x3F << 11));
            /* require srcPort to be NULL port */
            maskPtr[0]    |= (0x3F << 11);
            patternPtr[0] |= (PRV_CPSS_DXCH_NULL_PORT_NUM_CNS << 11);

            /* write port group unaware */
            multiPortGroupModify = GT_FALSE;
        }

        if ((srcPortPattern & 0x3C) == 0x3C)
        {
            /* big srcPort bits matched - CPU port or NULL port */
            multiPortGroupModify = GT_FALSE;
        }

        if (srcPortMask == 0)
        {
            /* source port field - "don't care" */
            multiPortGroupModify = GT_FALSE;
        }
    }

    if (multiPortGroupModify == GT_FALSE)
    {
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            /* write rule to all port groups without modification */
            rc = prvCpssDxChPclTcamRuleWrite(
                devNum, portGroupId,
                ruleSize, ruleIndex, validRule,
                waitPrevReady, actionPtr, maskPtr, patternPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }

    /* Multi port group modifying write */

    portGroupIdMask     = (srcPortMask >> 4) & 0x03;
    portGroupIdPattern  = (srcPortPattern >> 4) & 0x03;
    /* add high bits to require srcPort to be port-group-local */
    srcPortMask    |= 0x30;
    /* truncate pattern to port-group-local port value */
    srcPortPattern &= 0x0F;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* clear srcPort bits in mask and pattern */
        maskPtr[0]    &= (~ (0x3F << 11));
        patternPtr[0] &= (~ (0x3F << 11));

        if ((portGroupIdMask & portGroupId) == portGroupIdPattern)
        {
            /* put mask and pattern */
            maskPtr[0]    |= (srcPortMask << 11);
            patternPtr[0] |= (srcPortPattern << 11);
        }
        else
        {
            /* require srcPort to be NULL port */
            maskPtr[0]    |= (0x3F << 11);
            patternPtr[0] |= (PRV_CPSS_DXCH_NULL_PORT_NUM_CNS << 11);
        }

        rc = prvCpssDxChPclTcamRuleWrite(
            devNum, portGroupId, ruleSize, ruleIndex, validRule,
            waitPrevReady, actionPtr, maskPtr, patternPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}


/**
* @internal prvCpssDxChPclBypassFullControl_debug function
* @endinternal
*
* @brief   debug function to allow the 'redirect' action to have bypassIngressPipe = 0 and bypassBridge = 0
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] fullControl              - full control to the action by the application
*                                      0 - the application NOT have full control
*                                      1 - the application have full control (set have 'redirect'
*                                      action with bypassIngressPipe = 0 and bypassBridge = 0)
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChPclBypassFullControl_debug(IN GT_U32   fullControl)
{
    debugBypassFullControl = fullControl;
    return GT_OK;
}

/**
* @internal ipclActionConvertSip5ToSip5_20FieldName function
* @endinternal
*
* @brief   get field value :
*         function to allow sip5 and sip5_20 to use the same field names although
*         the order of the fields changed.
*         for sip5 device it not do any convert of the field name.
*         but for sip5_20 device it converts the field name to one of :
*         SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT names.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] fieldName                - the name of field as in SIP5_IPCL_ACTION_TABLE_FIELDS_ENT
*                                       converted sip5_20 field name
*/
static SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT      ipclActionConvertSip5ToSip5_20FieldName(
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName
)
{
    GT_U32  ii;
    SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT sip5_20_fieldName = 0xFFFFFFFF;

    for(ii = 0 ; ipcl_action_convert_sip5_to_sip_5_20[ii].sip5Name != 0xFFFFFFFF ; ii++)
    {
        if(ipcl_action_convert_sip5_to_sip_5_20[ii].sip5Name == fieldName)
        {
            /* found the proper convert */
            sip5_20_fieldName = ipcl_action_convert_sip5_to_sip_5_20[ii].sip5_20Name;
            break;
        }
    }

    return sip5_20_fieldName;
}

/**
* @internal ipclActionConvertSip5ToSip6FieldName function
* @endinternal
*
* @brief  function to allow sip5 and sip6 to use the same field
*         names although the order of the fields changed. for
*         sip5 device it not do any convert of the field name.
*         but for sip6 device it converts the field name to one
*         of : SIP6_IPCL_ACTION_TABLE_FIELDS_ENT names.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*                                  DxCh3; xCat; xCat3; Lion;
*                                  xCat2; Lion2; Bobcat2;
*                                  Caelum; Aldrin; AC3X;
*                                  Bobcat3; Aldrin2.
*
* @param[in] fieldName                - the name of field as in SIP5_IPCL_ACTION_TABLE_FIELDS_ENT
*                                       converted sip6 field
*                                       name
*/
static SIP6_IPCL_ACTION_TABLE_FIELDS_ENT  ipclActionConvertSip5ToSip6FieldName
(
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName
)
{
    GT_U32  ii;
    SIP6_IPCL_ACTION_TABLE_FIELDS_ENT sip6_fieldName = 0xFFFFFFFF;

    for(ii = 0 ; ipcl_action_convert_sip5_to_sip_6[ii].sip5Name != 0xFFFFFFFF ; ii++)
    {
        if(ipcl_action_convert_sip5_to_sip_6[ii].sip5Name == fieldName)
        {
            /* found the proper convert */
            sip6_fieldName = ipcl_action_convert_sip5_to_sip_6[ii].sip6Name;
            break;
        }
    }

    return sip6_fieldName;
}

/**
* @internal ipclGetProperFieldValue function
* @endinternal
*
* @brief  get field value :
*         function to allow sip5, sip5_20 and sip6 to use the
*         same field names although the order of the fields
*         changed. for sip5 device it not do any convert of the
*         field name. but for sip5_20 and sip6 devices it
*         converts the field name to one of :
*         SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT names.
*         or SIP6_IPCL_ACTION_TABLE_FIELDS_ENT names.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] memPtr                   - pointer to memory of the IPCL action.
* @param[in] fieldName                - the name of field as in SIP5_IPCL_ACTION_TABLE_FIELDS_ENT
*
* @param[out] valuePtr                 - (pointer to) value parsed from memPtr
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter : numBits > 32 or numBits == 0
*/
static GT_STATUS      ipclGetProperFieldValue(
    IN GT_U8                            devNum,
    IN GT_U32                           *memPtr,
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName,
    OUT GT_U32                           *valuePtr
)
{
    GT_U32  value;

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        U32_GET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            fieldName,
            value);
    }
    else /*convert the field*/
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))/*sip6*/
        {
            U32_GET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            ipclActionConvertSip5ToSip6FieldName(fieldName),
            value);
        }
        else  /*sip5_20*/
        {
            U32_GET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            ipclActionConvertSip5ToSip5_20FieldName(fieldName),
            value);
        }
    }

    *valuePtr = value;
    return GT_OK;
}

/**
* @internal ipclSetProperFieldValue function
* @endinternal
*
* @brief   set field value :
*         function to allow sip5, sip5_20 and sip6 to use the
*         same field names although the order of the fields
*         changed. for sip5 device it not do any convert of the
*         field name. but for sip5_20 and sip6 devices it
*         converts the field name to one of :
*         SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT names.
*         or SIP6_IPCL_ACTION_TABLE_FIELDS_ENT names.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] memPtr                   - pointer to memory of the IPCL action.
* @param[in] fieldName                - the name of field as in SIP5_IPCL_ACTION_TABLE_FIELDS_ENT
* @param[in] value                    -  to set to memPtr
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter : length > 32 or length == 0
*                                       or value > maxValue
*/
static GT_STATUS   ipclSetProperFieldValue(
    IN GT_U8                            devNum,
    IN GT_U32                           *memPtr,
    IN SIP5_IPCL_ACTION_TABLE_FIELDS_ENT  fieldName,
    IN GT_U32                           value
)
{
    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        U32_SET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            fieldName,
            value);
    }
    else /*convert the field*/
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) /*sip6*/
        {
            U32_SET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            ipclActionConvertSip5ToSip6FieldName(fieldName),
            value);
        }
        else /*sip5_20 */
        {
            U32_SET_FIELD_BY_ID_MAC(memPtr,
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E].fieldsInfoPtr,
            ipclActionConvertSip5ToSip5_20FieldName(fieldName),
            value);
        }

    }

    return GT_OK;
}

