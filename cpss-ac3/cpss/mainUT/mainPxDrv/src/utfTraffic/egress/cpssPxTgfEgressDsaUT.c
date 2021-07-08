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
* @file cpssPxTgfEgressDsaUT.c
*
* @brief Related to DSA systems :
* Egress tests for tests with traffic for the Pipe devices.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/ingress/cpssPxIngress.h>

#define INGRESS_PORT_NUM            7
#define UPLINK_PORT_NUM             12
#define EGRESS_DELAY_CNS        0xFEDCB/*20 bits*/

#define UNTAGGED_PVID_CNS       0 /* HARD CODED in the FW */
#define SRC_PORT_PVID_IGNORED_CNS   0xa74

#define EXTENDED_PORT_PVID_CNS      0xb9f
#define PORT_TPID_CNS      TGF_ETHERTYPE_9100_TAG_CNS


#define FRAME_VID_CNS      0x789
#define FRAME_TPID_CNS      (1 + TGF_ETHERTYPE_9100_TAG_CNS)

static GT_U8 frameVlanTagArr[] = {
/*byte 0*/    (GT_U8)(FRAME_TPID_CNS >> 8),
/*byte 1*/    (GT_U8)(FRAME_TPID_CNS & 0xFF),
/*byte 2*/    (GT_U8)((FRAME_VID_CNS >> 8) & 0x0F),
/*byte 3*/    (GT_U8)(FRAME_VID_CNS & 0xFF)
};

#define DSA_FROM_CPU    1
#define DSA_FRW         3
#define HW_SRC_DEV_DSA  0x1b /*5 bits*/
#define SRC_PORT_DSA    0x1a /*5 bits*/
#define UP_CNS          0
#define CFI_CNS         0


#define DSA_REPLACE_VLAN_TAG_WORD_CNS           \
   ((DSA_FRW << 30        ) |                   \
    (1 << 29              ) | /*srcTagged*/     \
    (HW_SRC_DEV_DSA  << 24) |                   \
    ((SRC_PORT_DSA & 0x1f)<< 19) |/*the FW  allow all 5 bits */ \
    (0 << 18              ) | /*srcIsTrunk*/    \
    (CFI_CNS << 16        ) |                   \
    (UP_CNS << 13         ) |                   \
    (0 << 12              ) | /*extended*/      \
    FRAME_VID_CNS)

static GT_U8    dsaReplaceVlanTagArr[] = {
/*byte 0*/    (GT_U8)((DSA_REPLACE_VLAN_TAG_WORD_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((DSA_REPLACE_VLAN_TAG_WORD_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((DSA_REPLACE_VLAN_TAG_WORD_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((DSA_REPLACE_VLAN_TAG_WORD_CNS >>  0) & 0xFF)
};

static CPSS_PX_REGULAR_DSA_FORWARD_STC dsaReplaceVlanTagInfo = {
/*srcTagged;      */  GT_TRUE,
/*hwSrcDev;       */  HW_SRC_DEV_DSA,
/*srcPortOrTrunk; */  0x1f, /* overridden by 4 LSBits CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC::srcPortNum */
/*srcIsTrunk;     */  GT_FALSE,
/*cfi;            */  CFI_CNS,
/*up;             */  UP_CNS,
/*vid;            */  0x666 /* dummy --> overridden by descriptor->vid */
};


static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E =
{
    1,/*vlan tag*/ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E =
{
    1,/*DSA tag*/ /*numOfParts*/
    {
        {
            4,
            dsaReplaceVlanTagArr
        }
    }/*partsInfo*/
};

#define DSA_ADDED_TAG_WORD_CNS           \
   ((DSA_FRW << 30        ) |                   \
    (0 << 29              ) | /*srcTagged*/     \
    (HW_SRC_DEV_DSA  << 24) |                   \
    ((SRC_PORT_DSA & 0x1f)<< 19) |/*the FW  allow all 5 bits */ \
    (0 << 18              ) | /*srcIsTrunk*/    \
    (CFI_CNS << 16        ) |                   \
    (UP_CNS << 13         ) |                   \
    (0 << 12              ) | /*extended*/      \
    UNTAGGED_PVID_CNS)

static GT_U8    dsaAddTagArr[] = {
/*byte 0*/    (GT_U8)((DSA_ADDED_TAG_WORD_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((DSA_ADDED_TAG_WORD_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((DSA_ADDED_TAG_WORD_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((DSA_ADDED_TAG_WORD_CNS >>  0) & 0xFF)
};

static CPSS_PX_REGULAR_DSA_FORWARD_STC dsaAddTagInfo = {
/*srcTagged;      */  GT_FALSE,
/*hwSrcDev;       */  HW_SRC_DEV_DSA,
/*srcPortOrTrunk; */  0x1f, /* overridden by 4 LSBits CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC::srcPortNum */
/*srcIsTrunk;     */  GT_FALSE,
/*cfi;            */  CFI_CNS,
/*up;             */  UP_CNS,
/*vid;            */  0x666 /* dummy --> overridden by CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC::pvid */
};
static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E =
{
    1,/*DSA tag*/ /*numOfParts*/
    {
        {
            4,
            dsaAddTagArr
        }
    }/*partsInfo*/
};


#define   EXT_DSA_TAG_WORD_0_CNS                \
   ((DSA_FRW << 30        ) |                   \
    (0 << 29              ) | /*srcTagged*/     \
    (HW_SRC_DEV_DSA  << 24) |                   \
    ((SRC_PORT_DSA & 0xf)<< 19) |/*the FW reset last bit !!!*/ \
    (0 << 18              ) | /*srcIsTrunk*/    \
    (CFI_CNS << 16        ) |                   \
    (UP_CNS << 13         ) |                   \
    (1 << 12              ) | /*extended*/      \
    EXTENDED_PORT_PVID_CNS)

#define EXT_DSA_TAG_WORD_1_CNS                          \
   ((0<<31              ) |/*extended*/                 \
    (0<<29              ) |/*SrcPort[6:5]*/             \
    (0<<28              ) |/*EgressFilterRegistered*/   \
    (0<<27              ) |/*DropOnSource*/             \
    (0<<26              ) |/*PacketIsLooped*/           \
    (0<<25              ) |/*Routed*/                   \
    (0<<20              ) |/*Src-ID*/                   \
    (0<<13              ) |/*QoSProfile*/               \
    (0<<12              ) |/*use_vidx*/                 \
    (0<<11              ) |/*Reserved*/                 \
    (0<<5               ) |/*TrgPort*/                  \
    (0<<0               ) )/*TrgDev*/

#define EXT_DSA_TAG_INVALID_WORD_0_CNS  \
    (EXT_DSA_TAG_WORD_0_CNS & (~(1<<12))/*extended*/)

#define EXT_DSA_TAG_INVALID_WORD_1_CNS  \
    (EXT_DSA_TAG_WORD_1_CNS | (1<<31)/*extended*/)


/* use format of invalid word 0 (extended bit is 0 in word 0) */
static GT_U8    invalidDsaW0ExtArr[] = {
/*byte 0*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_0_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_0_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_0_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_0_CNS >>  0) & 0xFF),
/*byte 4*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >> 24) & 0xFF),
/*byte 5*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >> 16) & 0xFF),
/*byte 6*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >>  8) & 0xFF),
/*byte 7*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >>  0) & 0xFF),
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_invalidDsaW0Ext0_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/*DSA tag*/ /*numOfParts*/
    {
        {
            8,
            invalidDsaW0ExtArr
        }
    }/*partsInfo*/
};

/* use format of DSA with valid word 0 (extended bit is 1 in word 0) */
/* but extended bit is 1 in word 1 --> 'not Extended-DSA' */
static GT_U8    invalidDsaW1ExtArr[] = {
/*byte 0*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >>  0) & 0xFF),
/*byte 4*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_1_CNS >> 24) & 0xFF),
/*byte 5*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_1_CNS >> 16) & 0xFF),
/*byte 6*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_1_CNS >>  8) & 0xFF),
/*byte 7*/    (GT_U8)((EXT_DSA_TAG_INVALID_WORD_1_CNS >>  0) & 0xFF),
};
static TGF_BYTES_INFO_LIST_STC    ingressTags_invalidDsaW1Ext1_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/*DSA tag*/ /*numOfParts*/
    {
        {
            8,
            invalidDsaW1ExtArr
        }
    }/*partsInfo*/
};

/* use format of DSA with valid word 0 (extended bit is 1 in word 0) */
/* but extended bit is 1 in word 1 --> 'not Extended-DSA' */
static GT_U8    dsaExtFromUplinkArr[] = {
/*byte 0*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((EXT_DSA_TAG_WORD_0_CNS >>  0) & 0xFF),
/*byte 4*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >> 24) & 0xFF),
/*byte 5*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >> 16) & 0xFF),
/*byte 6*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >>  8) & 0xFF),
/*byte 7*/    (GT_U8)((EXT_DSA_TAG_WORD_1_CNS >>  0) & 0xFF),
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/*DSA tag*/ /*numOfParts*/
    {
        {
            8,
            dsaExtFromUplinkArr
        }
    }/*partsInfo*/
};

/* vlan tag replace DSA -->
    take : vid,cfi,up form DSA ,
    take : tpid from 'target port' entry */
#define VLAN_TAG_REPLACE_EXT_DSA_WORD           \
   ((PORT_TPID_CNS << 16  ) |                   \
    (UP_CNS << 13         ) |                   \
    (CFI_CNS << 12        ) |                   \
    EXTENDED_PORT_PVID_CNS)


static GT_U8 vlanTagReplaceExtDsaArr[] = {
/*byte 0*/    (GT_U8)((VLAN_TAG_REPLACE_EXT_DSA_WORD >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((VLAN_TAG_REPLACE_EXT_DSA_WORD >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((VLAN_TAG_REPLACE_EXT_DSA_WORD >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((VLAN_TAG_REPLACE_EXT_DSA_WORD >>  0) & 0xFF)
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/*vlan tag*/ /*numOfParts*/
    {
        {
            4,
            vlanTagReplaceExtDsaArr
        }
    }/*partsInfo*/
};

#define DSA_FROM_CPU_WORD_CNS                   \
   ((DSA_FROM_CPU << 30   ) |                   \
    (1 << 29              ) | /*TrgTagged*/     \
    (HW_SRC_DEV_DSA  << 24) | /*TrgDev*/        \
    ((SRC_PORT_DSA & 0x1f)<< 19) |/*Trg Port[4:0]*/ \
    (0 << 18              ) | /*use_vidx*/      \
    (0 << 17              ) | /*TC[0]*/         \
    (CFI_CNS << 16        ) |                   \
    (UP_CNS << 13         ) |                   \
    (0 << 12              ) | /*extended*/      \
    FRAME_VID_CNS)

#define DSA_FROM_CPU_INVALID_WORD_CNS   \
    (DSA_FROM_CPU_WORD_CNS | (1<<12)/*extended*/)

static GT_U8 dsaFromCpuInvalidArr[] = {
/*byte 0*/    (GT_U8)((DSA_FROM_CPU_INVALID_WORD_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((DSA_FROM_CPU_INVALID_WORD_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((DSA_FROM_CPU_INVALID_WORD_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((DSA_FROM_CPU_INVALID_WORD_CNS >>  0) & 0xFF)
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_invalid_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E =
{
    2,/*dsa tag , vlan tag*/ /*numOfParts*/
    {
        {
            4,
            dsaFromCpuInvalidArr
        }
        ,
        {
            4,
            frameVlanTagArr
        }
    }/*partsInfo*/
};

static GT_U8 dsaFromCpuArr[] = {
/*byte 0*/    (GT_U8)((DSA_FROM_CPU_WORD_CNS >> 24) & 0xFF),
/*byte 1*/    (GT_U8)((DSA_FROM_CPU_WORD_CNS >> 16) & 0xFF),
/*byte 2*/    (GT_U8)((DSA_FROM_CPU_WORD_CNS >>  8) & 0xFF),
/*byte 3*/    (GT_U8)((DSA_FROM_CPU_WORD_CNS >>  0) & 0xFF)
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E =
{
    2,/*dsa tag , vlan tag*/ /*numOfParts*/
    {
        {
            4,
            dsaFromCpuArr
        }
        ,
        {
            4,
            frameVlanTagArr
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E =
{
    1,/* vlan tag*/ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr
        }
    }/*partsInfo*/
};


static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E =
{
    2,/*dsa tag , vlan tag*/ /*numOfParts*/
    {
        {
            4,
            dsaFromCpuArr
        }
        ,
        {
            4,
            frameVlanTagArr
        }
    }/*partsInfo*/
};



static CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC dsa_srcPortInfo = {
/*srcPortNum; */ (SRC_PORT_DSA & 0xF)  /*only 4 bits*/
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_dsa_uplink_trgPortInfo =
{
    0x524,             /* tpid -- dummy -- not used by uplink */
    0,                 /* pcid -- dummy -- not used by uplink DSA */
    EGRESS_DELAY_CNS   /* egressDelay */
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_dsa_extended_port_trgPortInfo =
{
    PORT_TPID_CNS,          /* tpid -- used to build VLAN tag */
    0,                 /* pcid -- dummy -- not used by extended port */
    EGRESS_DELAY_CNS   /*egressDelay*/
};

#define EXT_PARAM_DEFAULT_CNS                           (1<<0)

#define EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD0_EXTENDED_ZERO_CNS (1<<1)
#define EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD1_EXTENDED_ONE_CNS  (1<<2)
#define EXT_PARAM_DSA_U2E_EGRESS_UNTAGGED_CNS                          (1<<3)
#define EXT_PARAM_DSA_U2E_EGRESS_TAGGED_CNS                            (1<<4)
#define EXT_PARAM_DSA_U2E_EGRESS_ALL_UNTAGGED_CNS                      (1<<5)

#define EXT_PARAM_DSA_MRR2E_INVALID_DSA_WORD0_EXTENDED_ONE_CNS         (1<<1)



typedef struct{
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction;
    GT_U32                                     extParam;
    TGF_BYTES_INFO_LIST_STC                      *ingressTagsPtr;
    TGF_BYTES_INFO_LIST_STC                      *egressTagsPtr;

    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC          *dsa_srcPortInfoPtr;
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC       *info_common_trgPortInfoPtr;
}HA_ACTION_INFO;

static HA_ACTION_INFO ha_action_info[] =
{
     {CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E /*egressTagsPtr*/
     ,&dsa_srcPortInfo /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_uplink_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E /*egressTagsPtr*/
     ,&dsa_srcPortInfo /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_uplink_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD0_EXTENDED_ZERO_CNS
     ,&ingressTags_invalidDsaW0Ext0_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD1_EXTENDED_ONE_CNS
     ,&ingressTags_invalidDsaW1Ext1_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DSA_U2E_EGRESS_UNTAGGED_CNS |
      EXT_PARAM_DSA_U2E_EGRESS_ALL_UNTAGGED_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DSA_U2E_EGRESS_TAGGED_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DSA_MRR2E_INVALID_DSA_WORD0_EXTENDED_ONE_CNS
     ,&ingressTags_invalid_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E /*egressTagsPtr*/
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_extended_port_trgPortInfo
     }

     ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E /*egressTagsPtr*/ /* same as ingress !!! */
     ,NULL /*dsa_srcPortInfoPtr*/
     ,&info_common_dsa_uplink_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

    /*must be last*/
    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL  /*ingressTagsPtr*/
     ,NULL  /*egressTagsPtr*/
     ,NULL  /*dsa_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }
};

static GT_U8 frameArr[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,          /*mac da FF:FF:FF:FF:FF:FF */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01,          /*mac sa 00:00:00:00:00:01 */
      0x55, 0x55,                                  /*ethertype                */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   orig_haPerationType;
static CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  orig_haPerationInfo;
static CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   orig_sourcePortType;
static CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        orig_sourcePortInfo;
static CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   orig_targetPortType;
static CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        orig_targetPortInfo;
static GT_U32                                      orig_targetPortsBmpEnabled = 0;

/* save configuration to allow restore */
static void saveConfig(
    IN GT_SW_DEV_NUM     devNum,
    CPSS_PX_PACKET_TYPE  packetType,
    GT_PHYSICAL_PORT_NUM sourcePortNum,
    GT_PHYSICAL_PORT_NUM targetPortNum
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_BOOL     enabled;

    st = cpssPxEgressHeaderAlterationEntryGet(devNum,targetPortNum,packetType,
            &orig_haPerationType,&orig_haPerationInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressSourcePortEntryGet(devNum,sourcePortNum,
        &orig_sourcePortType,&orig_sourcePortInfo);
    if(st != GT_NOT_INITIALIZED)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
    else
    {
        orig_sourcePortType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E;
        orig_sourcePortInfo.notNeeded = 0;
    }

    st = cpssPxEgressTargetPortEntryGet(devNum,targetPortNum,
        &orig_targetPortType,&orig_targetPortInfo);
    if(st != GT_NOT_INITIALIZED)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
    else
    {
        orig_targetPortType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;
        orig_targetPortInfo.notNeeded = 0;
    }

    orig_targetPortsBmpEnabled = 0;
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }
        st = cpssPxIngressPortTargetEnableGet(devNum,ii,&enabled);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        if(enabled == GT_TRUE)
        {
            orig_targetPortsBmpEnabled |= 1 << ii;
        }
    }

}

/* restore configuration */
static void restoreConfig(
    IN GT_SW_DEV_NUM     devNum,
    CPSS_PX_PACKET_TYPE  packetType,
    GT_PHYSICAL_PORT_NUM sourcePortNum,
    GT_PHYSICAL_PORT_NUM targetPortNum
)
{
    GT_STATUS   st;
    GT_BOOL     enable;
    GT_U32      ii;

    st = cpssPxEgressHeaderAlterationEntrySet(devNum,targetPortNum,packetType,
            orig_haPerationType,&orig_haPerationInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressSourcePortEntrySet(devNum,sourcePortNum,
        orig_sourcePortType,&orig_sourcePortInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressTargetPortEntrySet(devNum,targetPortNum,
        orig_targetPortType,&orig_targetPortInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        enable = BIT2BOOL_MAC(((orig_targetPortsBmpEnabled>>ii)&1));

        st = cpssPxIngressPortTargetEnableSet(devNum,ii,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    /* Set default VLAN tag state entry for target ports */
    st = cpssPxEgressVlanTagStateEntrySet(devNum, EXTENDED_PORT_PVID_CNS, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    return;
}

/* disable all ports. (we skip CPU port)
    those that need traffic will be enabled by the engine.
    main purpose : to reduce replications in the simulation LOG.

    anyway we test only single egress port at a test.
*/
static void disabledAllPorts(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS   st;
    GT_U32      ii;

    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        st = cpssPxIngressPortTargetEnableSet(devNum,ii,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    return;
}

static GT_STATUS prvTgfPxEgressBasic(
    IN GT_SW_DEV_NUM     devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32           extParam
)
{
    GT_STATUS           rc;
    GT_U32              ii;
    GT_U32              frameLength;
    GT_U32              numOfEgressPortsInfo;
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC simpleList_egressPortsArr[1];
    TGF_BYTES_INFO_LIST_STC ingressFrame;
    TGF_BYTES_INFO_LIST_STC *egressFramePtr;
    GT_U32               egressPorts[1]={UPLINK_PORT_NUM};/* set at runtime */
    GT_PHYSICAL_PORT_NUM sourcePortNum = INGRESS_PORT_NUM;
    GT_PHYSICAL_PORT_NUM targetPortNum;
    HA_ACTION_INFO      *haInfoPtr;
    CPSS_PX_PACKET_TYPE  packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  haPerationInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   sourceInfoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        sourcePortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   targetInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        targetPortInfo;
    GT_BOOL                                     dropExpected = GT_FALSE;
    CPSS_PX_PORTS_BMP                           portsTagging;

    packetType = 7;/* as the ingress processing assigned */
    targetPortNum = egressPorts[0];

    /* save configuration to allow restore */
    saveConfig(devNum,packetType,sourcePortNum,targetPortNum);
    /* disable all ports. (we skip CPU port)
        those that need traffic will be enabled by the engine.
        main purpose : to reduce replications in the simulation LOG.

        anyway we test only single egress port at a test.
    */
    disabledAllPorts(devNum);

    frameLength = sizeof(frameArr);

    /* look for our case in the table */
    for(ii = 0 ;
        ha_action_info[ii].haAction != CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E ;
        ii++)
    {
        if((ha_action_info[ii].haAction == haAction) &&
           (ha_action_info[ii].extParam & extParam))
        {
            break;
        }
    }

    if(ha_action_info[ii].haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E)
    {
        /*case not found*/
        return GT_NOT_SUPPORTED;
    }

    haInfoPtr = &ha_action_info[ii];

    /**************************/
    /* prepare ingress packet */
    /**************************/
    ingressFrame.numOfParts = 4;
    ingressFrame.partsInfo[0].numOfBytes   = 12;/*mac DA,SA*/
    ingressFrame.partsInfo[0].partBytesPtr = &frameArr[0];
    ingressFrame.partsInfo[1].numOfBytes   = 0;   /*skip*/
    ingressFrame.partsInfo[1].partBytesPtr = NULL;/*skip*/
    ingressFrame.partsInfo[2].numOfBytes   = 0;   /*skip*/
    ingressFrame.partsInfo[2].partBytesPtr = NULL;/*skip*/

    if(haInfoPtr->ingressTagsPtr)
    {
        if(haInfoPtr->ingressTagsPtr->numOfParts >= 1)
        {
            ingressFrame.partsInfo[1] = haInfoPtr->ingressTagsPtr->partsInfo[0];
        }

        if(haInfoPtr->ingressTagsPtr->numOfParts == 2)
        {
            ingressFrame.partsInfo[2] = haInfoPtr->ingressTagsPtr->partsInfo[1];
        }

        if(haInfoPtr->ingressTagsPtr->numOfParts > 2)
        {
            /*case not supported*/
            return GT_NOT_SUPPORTED;
        }
    }

    ingressFrame.partsInfo[3].numOfBytes   = frameLength - ingressFrame.partsInfo[0].numOfBytes;
    ingressFrame.partsInfo[3].partBytesPtr = &frameArr[ingressFrame.partsInfo[0].numOfBytes];


    /*************************/
    /* prepare egress packet */
    /*************************/
    simpleList_egressPortsArr[0].portNum = targetPortNum;
    egressFramePtr = &simpleList_egressPortsArr[0].egressFrame;

    egressFramePtr->numOfParts = 4;
    egressFramePtr->partsInfo[0].numOfBytes   = 12;/*mac DA,SA*/
    egressFramePtr->partsInfo[0].partBytesPtr = &frameArr[0];
    egressFramePtr->partsInfo[1].numOfBytes   = 0;   /*skip*/
    egressFramePtr->partsInfo[1].partBytesPtr = NULL;/*skip*/
    egressFramePtr->partsInfo[2].numOfBytes   = 0;   /*skip*/
    egressFramePtr->partsInfo[2].partBytesPtr = NULL;/*skip*/

    if(haInfoPtr->egressTagsPtr)
    {
        if(haInfoPtr->egressTagsPtr->numOfParts >= 1)
        {
            egressFramePtr->partsInfo[1] = haInfoPtr->egressTagsPtr->partsInfo[0];
        }

        if(haInfoPtr->egressTagsPtr->numOfParts == 2)
        {
            egressFramePtr->partsInfo[2] = haInfoPtr->egressTagsPtr->partsInfo[1];
        }

        if(haInfoPtr->egressTagsPtr->numOfParts > 2)
        {
            /*case not supported*/
            return GT_NOT_SUPPORTED;
        }
    }

    /* the ingress packet hold 'hidden CRC bytes' */
    egressFramePtr->partsInfo[3].numOfBytes   = frameLength - egressFramePtr->partsInfo[0].numOfBytes + TGF_CRC_LEN_CNS;
    egressFramePtr->partsInfo[3].partBytesPtr = &frameArr[egressFramePtr->partsInfo[0].numOfBytes];


    /*************************/
    /* prepare egress tables */
    /*************************/
    haPerationInfo.notNeeded = 0;
    sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E;
    targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E;
    sourcePortInfo.notNeeded = 0;
    targetPortInfo.notNeeded = 0;


    if(haInfoPtr->dsa_srcPortInfoPtr)
    {
        sourcePortInfo.info_dsa = *haInfoPtr->dsa_srcPortInfoPtr;
    }

    if(haInfoPtr->info_common_trgPortInfoPtr)
    {
        targetPortInfo.info_common = *haInfoPtr->info_common_trgPortInfoPtr;
    }

    switch(haAction)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> Ingress tagged expected to : egress at 'Uplink Port' [%d] with DSA replace vlan tag \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_dsa_ET2U.dsaForward = dsaReplaceVlanTagInfo;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E;

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> Ingress untagged expected to : egress at 'Uplink Port' [%d] with DSA added \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_dsa_EU2U.dsaForward = dsaAddTagInfo;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            /* default mode :
               The VLAN tag replace the DSA only if the packet's extended DSA <VLAN-ID> equal to the
                <pvid> (of this entry)
               (1. egress without DSA and without VLAN tag --> 8 bytes removed
                 or
                2. egress with VLAN that replaces DSA --> 4 bytes removed)
            */
            portsTagging = 1 << targetPortNum;
            if(extParam == EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD0_EXTENDED_ZERO_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : NOT egress at 'Extended Port' [%d] because 'Regular' DSA (expected 'Extended') \n",
                    sourcePortNum,
                    targetPortNum);

                /* packet expected to be dropped */
                dropExpected = GT_TRUE;
            }
            else
            if(extParam == EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD1_EXTENDED_ONE_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : NOT egress at 'Extended Port' [%d] because larger than 'Extended' DSA (expected 'Extended') \n",
                    sourcePortNum,
                    targetPortNum);

                /* packet expected to be dropped */
                dropExpected = GT_TRUE;
            }
            else
            if(extParam == EXT_PARAM_DSA_U2E_EGRESS_UNTAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 8 bytes Ext-DSA \n",
                    sourcePortNum,
                    targetPortNum);
                portsTagging = 0;
            }
            else
            if(extParam == EXT_PARAM_DSA_U2E_EGRESS_TAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with 4 bytes vlan tag (replacing 8 bytes Ext-DSA) \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            if(extParam == EXT_PARAM_DSA_U2E_EGRESS_ALL_UNTAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : (ALL) egress at 'Extended Port' [%d] with removed 8 bytes Ext-DSA \n",
                    sourcePortNum,
                    targetPortNum);

                /* mode : The VLAN tag will not replace the DSA for all the packets egressing the port.
                  (egress without DSA and without VLAN tag --> 8 bytes removed)
                */
                portsTagging = 0;
            }
            else
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }

            if (dropExpected == GT_FALSE)
            {
                /* Set VLAN tag state entry for target ports */
                rc = cpssPxEgressVlanTagStateEntrySet(devNum, EXTENDED_PORT_PVID_CNS, portsTagging);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E:
            if(extParam == EXT_PARAM_DSA_MRR2E_INVALID_DSA_WORD0_EXTENDED_ONE_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : NOT egress at 'Extended Port' [%d] because NOT 'Regular' DSA (expected 'Extended' == 0) \n",
                    sourcePortNum,
                    targetPortNum);

                /* packet expected to be dropped */
                dropExpected = GT_TRUE;
            }
            else
            if(extParam == EXT_PARAM_DEFAULT_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 4 bytes DSA 'FROM_CPU' \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'CPU Port' [%d] --> expected to : egress at 'Uplink Port' [%d] (as ingress) --> with DSA \n",
                sourcePortNum,
                targetPortNum);

            break;

        default :
            /* not implemented or not needed */
            break;
    }

    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,targetPortNum,packetType,
            haAction,&haPerationInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxEgressSourcePortEntrySet(devNum,sourcePortNum,
        sourceInfoType,&sourcePortInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxEgressTargetPortEntrySet(devNum,targetPortNum,
        targetInfoType,&targetPortInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(dropExpected == GT_TRUE)
    {
        /*********************************************************************************/
        /* indication that only needs to check byte count counter and not packet content */
        /*********************************************************************************/
        simpleList_egressPortsArr[0].egressFrame.numOfParts = 1;
        simpleList_egressPortsArr[0].egressFrame.partsInfo[0].partBytesPtr = NULL;

        /* we need to expect 0 bytes on the egress port */
        simpleList_egressPortsArr[0].egressFrame.partsInfo[0].numOfBytes = 0;
    }


    numOfEgressPortsInfo = 1;

    rc = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum,sourcePortNum,
        &ingressFrame,numOfEgressPortsInfo,simpleList_egressPortsArr);

    if(rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("prvTgfPxEgressBasic[%d] : Test FAILED \n",haAction);
    }
    else
    {
        PRV_TGF_LOG1_MAC("prvTgfPxEgressBasic[%d] : Test PASSED \n",haAction);
    }


    restoreConfig(devNum,packetType,sourcePortNum,targetPortNum);

    return rc;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
 *    check for DSA : Extended Port Tagged   to Upstream Port (DSA-ET2U)
 *    check : egress replace ingress VLAN tag with DSA.
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_ET2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
 *    check for DSA : Extended Port UnTagged to Upstream Port (DSA-EU2U)
 *    check : egress adds DSA on ingress packet.
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_EU2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port to Extended Port          (DSA-U2E)
 *    check : if ingress DSA is not 'Extended DSA' (word 0<extended> == 0 or word 1<extended> == 1)
 *          this test do case of :  word 0<extended> = 0
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_U2E_invalidExtendedDsa_word0ExtendedZero)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD0_EXTENDED_ZERO_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port to Extended Port          (DSA-U2E)
 *    check : if ingress DSA is not 'Extended DSA' (word 0<extended> == 0 or word 1<extended> == 1)
 *          this test do case of :  word 1<extended> = 1
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_U2E_invalidExtendedDsa_word1ExtendedOne)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_U2E_INVALID_EXTENDED_DSA_WORD1_EXTENDED_ONE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port to Extended Port          (DSA-U2E)
 *    check that egress packet Popped the extended DSA
 *    mode:The VLAN tag replace the DSA only if the packet's extended DSA <VLAN-ID> equal to the
 *         <pvid> (of this entry)
 *
 *   NOTE: PVID of egress port match vlanId on the extended DSA -->
 *          egress without vlan tag (just popped the DSA)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_U2E_untagged_pvid_match)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_U2E_EGRESS_UNTAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port to Extended Port          (DSA-U2E)
 *    check that egress packet Replaced the extended DSA with vlan tag.
 *    mode:The VLAN tag replace the DSA only if the packet's extended DSA <VLAN-ID> equal to the
 *         <pvid> (of this entry)
 *
 *   NOTE: PVID of egress port not match vlanId on the extended DSA -->
 *      egress with vlan tag (replaced the extended DSA)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_U2E_tagged_pvid_no_match)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_U2E_EGRESS_TAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port to Extended Port          (DSA-U2E)
 *    check that egress packet Replaced the extended DSA with vlan tag.
 *    mode: The VLAN tag will not replace the DSA for all the packets egressing the port.
 *         (egress without DSA and without VLAN tag --> 8 bytes removed)
 *
 *   NOTE: all packets egress without vlan tag (just popped the DSA)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_U2E_untagged_all)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_U2E_EGRESS_ALL_UNTAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port Mirroring to Extended Port(DSA-Mrr2E)
 *    check : if ingress DSA is not 'Standard DSA'
 *          check that no egress if ingress DSA with word 0<extended> = 1
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_Mrr2E_invalidDsa_word0ExtendedOne)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E,
            EXT_PARAM_DSA_MRR2E_INVALID_DSA_WORD0_EXTENDED_ONE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
 *    check for DSA : Upstream Port Mirroring to Extended Port(DSA-Mrr2E)
 *    check that egress without the 4 bytes DSA. (popped 4 bytes of 'FROM_CPU')
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_Mrr2E)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
 *    check for DSA : CPU Port to Upstream Port (DSA-CPU2U)
 *    check that egress packet as ingress (packet egress unchanged - ingress with DSA)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_dsa_CPU2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfEgress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfEgressDsa)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_ET2U)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_EU2U)
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_U2E_invalidExtendedDsa_word0ExtendedZero)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_U2E_invalidExtendedDsa_word1ExtendedOne)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_U2E_untagged_pvid_match)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_U2E_tagged_pvid_no_match)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_U2E_untagged_all)
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_Mrr2E_invalidDsa_word0ExtendedOne)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_Mrr2E)
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_dsa_CPU2U)

UTF_SUIT_END_TESTS_MAC(cpssPxTgfEgressDsa)


