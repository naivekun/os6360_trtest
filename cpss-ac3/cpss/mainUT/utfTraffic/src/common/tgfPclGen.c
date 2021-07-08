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
* @file tgfPclGen.c
*
* @brief Generic API implementation for PCL
*
* @version   108
********************************************************************************
*/
#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfIpGen.h>
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>

static GT_U32   orig_currPortGroupsBmp = 0;
/******************************************************************************\
 *                              Marco definitions                             *
\******************************************************************************/

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>

extern GT_U32 appDemoDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

/* convert UDB offset types device specific format */
#define PRV_TGF_S2D_PCL_UDB_OFFSET_TYPE_CONVERT_MAC(dstType, srcType)   \
    do                                                                  \
    {                                                                   \
        switch (srcType)                                                \
        {                                                               \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_L2_E,                       \
                CPSS_DXCH_PCL_OFFSET_L2_E);                             \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_L3_E,                       \
                CPSS_DXCH_PCL_OFFSET_L3_E);                             \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_L4_E,                       \
                CPSS_DXCH_PCL_OFFSET_L4_E);                             \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_IPV6_EXT_HDR_E,             \
                CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E);                   \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E,       \
                CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E);             \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,               \
                CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E);                     \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E,             \
                CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E);                   \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,                \
                CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,        \
                CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E);              \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_METADATA_E,                 \
                CPSS_DXCH_PCL_OFFSET_METADATA_E);                       \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_OFFSET_INVALID_E,                  \
                CPSS_DXCH_PCL_OFFSET_INVALID_E);                        \
            default:                                                    \
                return GT_BAD_PARAM;                                    \
        }                                                               \
    } while (0)

/* convert UDB override types device specific format */
#define PRV_TGF_S2D_PCL_UDB_OVERRIDE_TYPE_CONVERT_MAC(dstType, srcType) \
    do                                                                  \
    {                                                                   \
        switch (srcType)                                                \
        {                                                               \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,    \
                CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E);          \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,    \
                CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E);          \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,   \
                CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E);         \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,    \
                CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E);          \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,     \
                CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E);           \
            default:                                                    \
                return GT_BAD_PARAM;                                    \
        }                                                               \
    } while (0)


/* convert policy rules modes into device specific format */
#define PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dstKey, srcKey)                \
    do                                                                         \
    {                                                                          \
        switch (srcKey)                                                        \
        {                                                                      \
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,\
                                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                dstKey, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E,\
                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E);\
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/*
*/
/* convert policy rules modes from device specific format */
#define PRV_TGF_D2S_PCL_RULE_FORMAT_CONVERT_MAC(dstKey, srcKey)                \
    do                                                                         \
    {                                                                          \
        switch (dstKey)                                                        \
        {                                                                      \
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,\
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E,\
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E);\
            PRV_TGF_SWITCH_CASE_MAC( \
                srcKey, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E,\
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E);\
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert TCAM segment mode into device specific format */
#define PRV_TGF_S2D_PCL_TCAM_SEGMENT_CONVERT_MAC(dstMode, srcMode)                \
    do                                                                         \
    {                                                                          \
        switch (srcMode)                                                        \
        {                                                                      \
            PRV_TGF_SWITCH_CASE_MAC(dstMode, PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E,\
                                    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstMode, PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E,\
                                    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstMode, PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E,\
                                    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstMode, PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E,\
                                    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E);\
            PRV_TGF_SWITCH_CASE_MAC(dstMode, PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E,\
                                    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E);\
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

#define PRV_TGF_D2S_PCL_TCAM_SEGMENT_CONVERT_MAC(dstMode, srcMode)             \
    do                                                                         \
    {                                                                          \
        switch (dstMode)                                                       \
        {                                                                      \
            PRV_TGF_SWITCH_CASE_MAC(srcMode, CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E ,\
                                    PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcMode, CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E ,\
                                    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcMode, CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E ,\
                                    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcMode, CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E ,\
                                    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E);\
            PRV_TGF_SWITCH_CASE_MAC(srcMode, CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E ,\
                                    PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E);\
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* copy commonStdIp field from source structure into destination structure  */
#define PRV_TGF_STC_COMMON_STD_IP_FIELD_COPY_MAC(dstStcPtr, srcStcPtr) \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipProtocol); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), dscp); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), isL4Valid); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), l4Byte2); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), l4Byte3); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipHeaderOk); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipv4Fragmented); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipv4Fragmented)

/* copy commonExt field from source structure into destination structure  */
#define PRV_TGF_STC_COMMON_EXT_FIELD_COPY_MAC(dstStcPtr, srcStcPtr) \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), isIpv6); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), ipProtocol); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), dscp); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), isL4Valid); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte0); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte1); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte2); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte3); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte13); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), ipHeaderOk)

/* copy commonEgrStdIp field from source structure into destination structure  */
#define PRV_TGF_STC_COMMON_EGR_STD_IP_FIELD_COPY_MAC(dstStcPtr, srcStcPtr) \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), isIpv4); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipProtocol); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), dscp); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), isL4Valid); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), l4Byte2); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), l4Byte3); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), ipv4Fragmented); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonStdIp), &(srcStcPtr->commonStdIp), egrTcpUdpPortComparator)

/* copy commonEgrExt field from source structure into destination structure  */
#define PRV_TGF_STC_COMMON_EGR_EXT_FIELD_COPY_MAC(dstStcPtr, srcStcPtr) \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), isIpv6); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), ipProtocol); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), dscp); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), isL4Valid); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte0); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte1); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte2); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte3); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), l4Byte13); \
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dstStcPtr->commonExt), &(srcStcPtr->commonExt), egrTcpUdpPortComparator)

#endif /* CHX_FAMILY */

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/


/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY



/**
* @internal prvTgfConvertGenericToDxChPclRuleCommon function
* @endinternal
*
* @brief   Convert generic into device specific common segment of all key formats
*
* @param[in] ruleCommonPtr            - (pointer to) common part for all formats
*
* @param[out] dxChRuleCommonPtr        - (pointer to) DxCh common part for all formats
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChPclRuleCommon
(
    IN  PRV_TGF_PCL_RULE_FORMAT_COMMON_STC      *ruleCommonPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC    *dxChRuleCommonPtr
)
{
    /* convert common part into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourcePort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, portListBmp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isUdbValid);
}

/**
* @internal prvTgfConvertGenericToDxChPclRuleEgrCommon function
* @endinternal
*
* @brief   Convert generic into device specific common segment of all egress key formats
*
* @param[in] ruleCommonPtr            - (pointer to) common part for all formats
*
* @param[out] dxChRuleCommonPtr        - (pointer to) DxCh common part for all formats
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPclRuleEgrCommon
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC      *ruleCommonPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC    *dxChRuleCommonPtr
)
{
    /* convert common part into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, egrPacketType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourceId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isVidx);

    dxChRuleCommonPtr->srcHwDev  = ruleCommonPtr->srcDev;

    dxChRuleCommonPtr->sourcePort  = ruleCommonPtr->srcPort;
    dxChRuleCommonPtr->portListBmp = ruleCommonPtr->portListBmp;
    dxChRuleCommonPtr->tag1Exist = ruleCommonPtr->tag1Exist;

    switch (ruleCommonPtr->egrPacketType)
    {
        case 0:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->toCpu),
                                           &(ruleCommonPtr->egrPktType.toCpu), cpuCode);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->toCpu),
                                           &(ruleCommonPtr->egrPktType.toCpu), srcTrg);

            break;

        case 1:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fromCpu),
                                           &(ruleCommonPtr->egrPktType.fromCpu), tc);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fromCpu),
                                           &(ruleCommonPtr->egrPktType.fromCpu), dp);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fromCpu),
                                           &(ruleCommonPtr->egrPktType.fromCpu), egrFilterEnable);

            break;

        case 2:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->toAnalyzer),
                                           &(ruleCommonPtr->egrPktType.toAnalyzer), rxSniff);

            break;

        case 3:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fwdData),
                                           &(ruleCommonPtr->egrPktType.fwdData), qosProfile);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fwdData),
                                           &(ruleCommonPtr->egrPktType.fwdData), srcTrunkId);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fwdData),
                                           &(ruleCommonPtr->egrPktType.fwdData), srcIsTrunk);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fwdData),
                                           &(ruleCommonPtr->egrPktType.fwdData), isUnknown);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleCommonPtr->fwdData),
                                           &(ruleCommonPtr->egrPktType.fwdData), isRouted);

            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChPclRuleIpCommon function
* @endinternal
*
* @brief   Convert generic into device specific common segment of all key formats
*
* @param[in] ruleCommonPtr            - (pointer to) common part for all formats
*
* @param[out] dxChRuleCommonPtr        - (pointer to) DxCh common part for all formats
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChPclRuleIpCommon
(
    IN  PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC      *ruleCommonPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC    *dxChRuleCommonPtr
)
{
    /* convert common part into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourcePort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourceDevice);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, portListBmp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, up);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, tos);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, ipProtocol);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isL4Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte3);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte13);
}
/**
* @internal prvTgfConvertGenericToDxChPclRuleIpEgrCommon function
* @endinternal
*
* @brief   Convert generic into device specific common segment of all key formats
*
* @param[in] ruleCommonPtr            - (pointer to) common part for all formats
*
* @param[out] dxChRuleCommonPtr        - (pointer to) DxCh common part for all formats
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChPclRuleIpEgrCommon
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC      *ruleCommonPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC    *dxChRuleCommonPtr
)
{

    /* convert common part into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourcePort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, qosProfile);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, originalVid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isSrcTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, srcDevOrTrunkId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, srcHwDev);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isArp);
    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleCommonPtr->macDa.arEther,
                 ruleCommonPtr->macDa.arEther,
                 sizeof(dxChRuleCommonPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleCommonPtr->macSa.arEther,
                 ruleCommonPtr->macSa.arEther,
                 sizeof(dxChRuleCommonPtr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, sourceId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, tos);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, ipProtocol);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, ttl);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isL4Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte3);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, l4Byte13);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, tcpUdpPortComparators);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, tc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, dp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, egrPacketType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, srcTrgOrTxMirror);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, assignedUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, trgPhysicalPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, rxSniff);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isRouted);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, isIpv6);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, cpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, srcTrg);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleCommonPtr, ruleCommonPtr, cpuCode);

}

/**
* @internal prvTgfConvertGenericToDxChIngStdNotIp function
* @endinternal
*
* @brief   Convert generic into device specific standard not IP packet key
*
* @param[in] ruleStdNotIpPtr          - (pointer to) standard not IP packet key
*
* @param[out] dxChRuleStdNotIpPtr      - (pointer to) DxCh standard not IP packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChIngStdNotIp
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_NOT_IP_STC      *ruleStdNotIpPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC    *dxChRuleStdNotIpPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(
        &(ruleStdNotIpPtr->common), &(dxChRuleStdNotIpPtr->common));



    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdNotIpPtr, ruleStdNotIpPtr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdNotIpPtr, ruleStdNotIpPtr, l2Encap);

    dxChRuleStdNotIpPtr->isArp  = ruleStdNotIpPtr->common.isArp;
    dxChRuleStdNotIpPtr->isIpv4 = ruleStdNotIpPtr->common.isIpv4;

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->macDa.arEther,
                 ruleStdNotIpPtr->macDa.arEther,
                 sizeof(dxChRuleStdNotIpPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->macSa.arEther,
                 ruleStdNotIpPtr->macSa.arEther,
                 sizeof(dxChRuleStdNotIpPtr->macSa.arEther));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->udb15_17,
                 ruleStdNotIpPtr->udb + 15,
                 sizeof(dxChRuleStdNotIpPtr->udb15_17));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, trunkHash);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, cfi1);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->udb23_26,
                 ruleStdNotIpPtr->udb + 23,
                 sizeof(dxChRuleStdNotIpPtr->udb23_26));
    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngStdIpL2Qos function
* @endinternal
*
* @brief   Convert generic into device specific standard L2 and QOS styled key
*
* @param[in] ruleStdIpL2QosPtr        - (pointer to) standard L2 and QOS styled key
*
* @param[out] dxChRuleStdIpL2QosPtr    - (pointer to) DxCh standard L2 and QOS styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngStdIpL2Qos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC   *ruleStdIpL2QosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC *dxChRuleStdIpL2QosPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(
        &(ruleStdIpL2QosPtr->common), &(dxChRuleStdIpL2QosPtr->common));

    /* convert commonStdIp into device specific format */
    PRV_TGF_STC_COMMON_STD_IP_FIELD_COPY_MAC(
        dxChRuleStdIpL2QosPtr, ruleStdIpL2QosPtr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpL2QosPtr, ruleStdIpL2QosPtr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpL2QosPtr, ruleStdIpL2QosPtr, isIpv6HopByHop);

    dxChRuleStdIpL2QosPtr->commonStdIp.isIpv4 = ruleStdIpL2QosPtr->common.isIpv4;
    dxChRuleStdIpL2QosPtr->isArp = ruleStdIpL2QosPtr->common.isArp;

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->macDa.arEther,
                 ruleStdIpL2QosPtr->macDa.arEther,
                 sizeof(dxChRuleStdIpL2QosPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->macSa.arEther,
                 ruleStdIpL2QosPtr->macSa.arEther,
                 sizeof(dxChRuleStdIpL2QosPtr->macSa.arEther));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->udb18_19,
                 ruleStdIpL2QosPtr->udb + 18,
                 sizeof(dxChRuleStdIpL2QosPtr->udb18_19));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdIpL2QosPtr, ruleStdIpL2QosPtr, vrfId);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->udb27_30,
                 ruleStdIpL2QosPtr->udb + 27,
                 sizeof(dxChRuleStdIpL2QosPtr->udb27_30));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngStdIpv4L4 function
* @endinternal
*
* @brief   Convert generic into device specific standard IPV4 packet L4 styled key
*
* @param[in] ruleStdIpv4L4Ptr         - (pointer to) standard IPV4 packet L4 styled key
*
* @param[out] dxChRuleStdIpv4L4Ptr     - (pointer to) DxCh standard IPV4 packet L4 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngStdIpv4L4
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_L4_STC     *ruleStdIpv4L4Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC   *dxChRuleStdIpv4L4Ptr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(&(ruleStdIpv4L4Ptr->common), &(dxChRuleStdIpv4L4Ptr->common));

    /* convert commonStdIp into device specific format */
    PRV_TGF_STC_COMMON_STD_IP_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdIpv4L4Ptr->sip), &(ruleStdIpv4L4Ptr->sip), u32Ip);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdIpv4L4Ptr->dip), &(ruleStdIpv4L4Ptr->dip), u32Ip);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, isBc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, l4Byte0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, l4Byte1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, l4Byte13);

    dxChRuleStdIpv4L4Ptr->commonStdIp.isIpv4 = ruleStdIpv4L4Ptr->common.isIpv4;
    dxChRuleStdIpv4L4Ptr->isArp = ruleStdIpv4L4Ptr->common.isArp;

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpv4L4Ptr->udb20_22,
                 ruleStdIpv4L4Ptr->udb + 20,
                 sizeof(dxChRuleStdIpv4L4Ptr->udb20_22));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        ruleStdIpv4L4Ptr, dxChRuleStdIpv4L4Ptr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        ruleStdIpv4L4Ptr, dxChRuleStdIpv4L4Ptr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpv4L4Ptr->udb31_34,
                 ruleStdIpv4L4Ptr->udb + 31,
                 sizeof(dxChRuleStdIpv4L4Ptr->udb31_34));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngStdIpv6Dip function
* @endinternal
*
* @brief   Convert generic into device specific standard IPV6 packet DIP styled key
*
* @param[in] ruleStdIpv6DipPtr        - (pointer to) standard IPV6 packet DIP styled key
*
* @param[out] dxChRuleStdIpv6DipPtr    - (pointer to) DxCh standard IPV6 packet DIP styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngStdIpv6Dip
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_IPV6_DIP_STC    *ruleStdIpv6DipPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC  *dxChRuleStdIpv6DipPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(&(ruleStdIpv6DipPtr->common), &(dxChRuleStdIpv6DipPtr->common));

    /* convert commonStdIp into device specific format */
    PRV_TGF_STC_COMMON_STD_IP_FIELD_COPY_MAC(dxChRuleStdIpv6DipPtr, ruleStdIpv6DipPtr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv6DipPtr, ruleStdIpv6DipPtr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv6DipPtr, ruleStdIpv6DipPtr, isIpv6HopByHop);

    dxChRuleStdIpv6DipPtr->isArp = ruleStdIpv6DipPtr->common.isArp;

    /* convert Dip into device specific format */
    cpssOsMemCpy(dxChRuleStdIpv6DipPtr->dip.u32Ip,
                 ruleStdIpv6DipPtr->dip.u32Ip,
                 sizeof(dxChRuleStdIpv6DipPtr->dip.u32Ip));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpv6DipPtr->udb47_49,
                 ruleStdIpv6DipPtr->udb + 47,
                 sizeof(dxChRuleStdIpv6DipPtr->udb47_49));
    dxChRuleStdIpv6DipPtr->udb0 = ruleStdIpv6DipPtr->udb[0];


    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngStdUdb function
* @endinternal
*
* @brief   Convert generic into device specific standard UDB styled packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngStdUdb
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_UDB_STC                 *ruleStdUdbPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC  *dxChRuleStdUdbPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), dscpOrExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), pktTagging);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), l3OffsetInvalid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), l4ProtocolType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), pktType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), ipHeaderOk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), macDaType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), isIpv6Eh);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleStdUdbPtr->commonIngrUdb), &(ruleStdUdbPtr->commonUdb), isIpv6HopByHop);

    dxChRuleStdUdbPtr->commonIngrUdb.pclId       = ruleStdUdbPtr->commonStd.pclId;
    dxChRuleStdUdbPtr->commonIngrUdb.macToMe     = ruleStdUdbPtr->commonStd.macToMe;
    dxChRuleStdUdbPtr->commonIngrUdb.sourcePort  = ruleStdUdbPtr->commonStd.sourcePort;
    dxChRuleStdUdbPtr->commonIngrUdb.portListBmp = ruleStdUdbPtr->commonStd.portListBmp;
    dxChRuleStdUdbPtr->commonIngrUdb.vid         = ruleStdUdbPtr->commonStd.vid;
    dxChRuleStdUdbPtr->commonIngrUdb.up          = ruleStdUdbPtr->commonStd.up;
    dxChRuleStdUdbPtr->commonIngrUdb.isIp        = ruleStdUdbPtr->commonStd.isIp;
    dxChRuleStdUdbPtr->commonIngrUdb.l2Encapsulation = ruleStdUdbPtr->commonUdb.l2Encap;
    dxChRuleStdUdbPtr->isIpv4                   = ruleStdUdbPtr->commonStd.isIpv4;

    dxChRuleStdUdbPtr->commonIngrUdb.l4OffsetInvalid = ruleStdUdbPtr->commonUdb.l4OffsetInvalid;

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdUdbPtr->udb0_15,
                 ruleStdUdbPtr->udb,
                 sizeof(dxChRuleStdUdbPtr->udb0_15));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdUdbPtr, ruleStdUdbPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdUdbPtr, ruleStdUdbPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdUdbPtr, ruleStdUdbPtr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdUdbPtr->udb35_38,
                 ruleStdUdbPtr->udb + 35,
                 sizeof(dxChRuleStdUdbPtr->udb35_38));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngExtNotIpv6 function
* @endinternal
*
* @brief   Convert generic into device specific not IP and IPV4 packet key
*
* @param[in] ruleExtNotIpv6Ptr        - (pointer to) not IP and IPV4 packet key
*
* @param[out] dxChRuleExtNotIpv6Ptr    - (pointer to) DxCh not IP and IPV4 packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngExtNotIpv6
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC    *ruleExtNotIpv6Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC  *dxChRuleExtNotIpv6Ptr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(&(ruleExtNotIpv6Ptr->common), &(dxChRuleExtNotIpv6Ptr->common));

    /* convert commonExt into device specific format */
    PRV_TGF_STC_COMMON_EXT_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtNotIpv6Ptr->sip), &(ruleExtNotIpv6Ptr->sip), u32Ip);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtNotIpv6Ptr->dip), &(ruleExtNotIpv6Ptr->dip), u32Ip);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, l2Encap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, ipv4Fragmented);

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->macDa.arEther,
                 ruleExtNotIpv6Ptr->macDa.arEther,
                 sizeof(dxChRuleExtNotIpv6Ptr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->macSa.arEther,
                 ruleExtNotIpv6Ptr->macSa.arEther,
                 sizeof(dxChRuleExtNotIpv6Ptr->macSa.arEther));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->udb0_5,
                 ruleExtNotIpv6Ptr->udb,
                 sizeof(dxChRuleExtNotIpv6Ptr->udb0_5));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->udb39_46,
                 ruleExtNotIpv6Ptr->udb + 39,
                 sizeof(dxChRuleExtNotIpv6Ptr->udb39_46));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngRuleExtIpv6L2 function
* @endinternal
*
* @brief   Convert generic into device specific extended IPV6 packet L2 styled key
*
* @param[in] ruleExtIpv6L2Ptr         - (pointer to) extended IPV6 packet L2 styled key
*
* @param[out] dxChRuleExtIpv6L2Ptr     - (pointer to) DxCh extended IPV6 packet L2 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleExtIpv6L2
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L2_STC     *ruleExtIpv6L2Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC   *dxChRuleExtIpv6L2Ptr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(&(ruleExtIpv6L2Ptr->common), &(dxChRuleExtIpv6L2Ptr->common));

    /* convert commonExt into device specific format */
    PRV_TGF_STC_COMMON_EXT_FIELD_COPY_MAC(dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, isIpv6HopByHop);

    dxChRuleExtIpv6L2Ptr->dipBits127to120 = ruleExtIpv6L2Ptr->dipBits;

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->sip.u32Ip,
                 ruleExtIpv6L2Ptr->sip.u32Ip,
                 sizeof(dxChRuleExtIpv6L2Ptr->sip.u32Ip));

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->macDa.arEther,
                 ruleExtIpv6L2Ptr->macDa.arEther,
                 sizeof(dxChRuleExtIpv6L2Ptr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->macSa.arEther,
                 ruleExtIpv6L2Ptr->macSa.arEther,
                 sizeof(dxChRuleExtIpv6L2Ptr->macSa.arEther));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->udb6_11,
                 ruleExtIpv6L2Ptr->udb + 6,
                 sizeof(dxChRuleExtIpv6L2Ptr->udb6_11));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->udb47_49,
                 ruleExtIpv6L2Ptr->udb + 47,
                 sizeof(dxChRuleExtIpv6L2Ptr->udb47_49));

    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->udb0_4,
                 ruleExtIpv6L2Ptr->udb,
                 sizeof(dxChRuleExtIpv6L2Ptr->udb0_4));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngRuleExtIpv6L4 function
* @endinternal
*
* @brief   Convert generic into device specific extended IPV6 packet L4 styled key
*
* @param[in] ruleExtIpv6L4Ptr         - (pointer to) extended IPV6 packet L4 styled key
*
* @param[out] dxChRuleExtIpv6L4Ptr     - (pointer to) DxCh extended IPV6 packet L4 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleExtIpv6L4
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L4_STC     *ruleExtIpv6L4Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC   *dxChRuleExtIpv6L4Ptr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleCommon(&(ruleExtIpv6L4Ptr->common), &(dxChRuleExtIpv6L4Ptr->common));

    /* convert commonExt into device specific format */
    PRV_TGF_STC_COMMON_EXT_FIELD_COPY_MAC(dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, isIpv6HopByHop);

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->sip.u32Ip,
                 ruleExtIpv6L4Ptr->sip.u32Ip,
                 sizeof(dxChRuleExtIpv6L4Ptr->sip.u32Ip));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->dip.u32Ip,
                 ruleExtIpv6L4Ptr->dip.u32Ip,
                 sizeof(dxChRuleExtIpv6L4Ptr->dip.u32Ip));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->udb12_14,
                 ruleExtIpv6L4Ptr->udb + 12,
                 sizeof(dxChRuleExtIpv6L4Ptr->udb12_14));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->udb15_22,
                 ruleExtIpv6L4Ptr->udb + 15,
                 sizeof(dxChRuleExtIpv6L4Ptr->udb15_22));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChIngRuleExtUdb function
* @endinternal
*
* @brief   Convert generic into device specific extended UDB styled packet key
*
* @param[in] ruleExtUdbPtr            - (pointer to) extended UDB styled packet key
*
* @param[out] dxChRuleExtUdbPtr        - (pointer to) DxCh extended UDB styled packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleExtUdb
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC                 *ruleExtUdbPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC  *dxChRuleExtUdbPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), dscpOrExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), pktTagging);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), l4ProtocolType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), pktType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), ipHeaderOk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), macDaType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), isIpv6Eh);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleExtUdbPtr->commonIngrUdb), &(ruleExtUdbPtr->commonIngrUdb), isIpv6HopByHop);

    dxChRuleExtUdbPtr->commonIngrUdb.pclId       = ruleExtUdbPtr->commonStd.pclId;
    dxChRuleExtUdbPtr->commonIngrUdb.macToMe     = ruleExtUdbPtr->commonStd.macToMe;
    dxChRuleExtUdbPtr->commonIngrUdb.sourcePort  = ruleExtUdbPtr->commonStd.sourcePort;
    dxChRuleExtUdbPtr->commonIngrUdb.portListBmp = ruleExtUdbPtr->commonStd.portListBmp;
    dxChRuleExtUdbPtr->commonIngrUdb.vid         = ruleExtUdbPtr->commonStd.vid;
    dxChRuleExtUdbPtr->commonIngrUdb.up          = ruleExtUdbPtr->commonStd.up;
    dxChRuleExtUdbPtr->commonIngrUdb.isIp        = ruleExtUdbPtr->commonStd.isIp;
    dxChRuleExtUdbPtr->commonIngrUdb.l2Encapsulation  = ruleExtUdbPtr->commonIngrUdb.l2Encap;
    dxChRuleExtUdbPtr->commonIngrUdb.l4OffsetInvalid  = ruleExtUdbPtr->commonIngrUdb.l4OffsetInvalid;

    dxChRuleExtUdbPtr->isIpv6     = ruleExtUdbPtr->commonExt.isIpv6;
    dxChRuleExtUdbPtr->ipProtocol = ruleExtUdbPtr->commonExt.ipProtocol;

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->sipBits31_0,
                 ruleExtUdbPtr->sipBits31_0,
                 sizeof(dxChRuleExtUdbPtr->sipBits31_0));

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->sipBits79_32orMacSa,
                 ruleExtUdbPtr->macSaOrSipBits79_32,
                 sizeof(dxChRuleExtUdbPtr->sipBits79_32orMacSa));

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->sipBits127_80orMacDa,
                 ruleExtUdbPtr->macDaOrSipBits127_80,
                 sizeof(dxChRuleExtUdbPtr->sipBits127_80orMacDa));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->dipBits127_112,
                 ruleExtUdbPtr->dipBits127_112,
                 sizeof(dxChRuleExtUdbPtr->dipBits127_112));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->dipBits31_0,
                 ruleExtUdbPtr->dipBits31_0,
                 sizeof(dxChRuleExtUdbPtr->dipBits31_0));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->udb0_15,
                 ruleExtUdbPtr->udb,
                 sizeof(dxChRuleExtUdbPtr->udb0_15));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtUdbPtr, ruleExtUdbPtr, trunkHash);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtUdbPtr->udb23_30,
                 ruleExtUdbPtr->udb + 23,
                 sizeof(dxChRuleExtUdbPtr->udb23_30));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleStdNotIp function
* @endinternal
*
* @brief   Convert generic into device specific egress standard not IP packet key
*
* @param[in] ruleStdNotIpPtr          - (pointer to) standard not IP packet key
*
* @param[out] dxChRuleStdNotIpPtr      - (pointer to) DxCh standard not IP packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleStdNotIp
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_STD_NOT_IP_STC      *ruleStdNotIpPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC    *dxChRuleStdNotIpPtr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleStdNotIpPtr->common),
                                                    &(dxChRuleStdNotIpPtr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdNotIpPtr, ruleStdNotIpPtr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdNotIpPtr, ruleStdNotIpPtr, l2Encap);

    dxChRuleStdNotIpPtr->isIpv4 = ruleStdNotIpPtr->common.isIpv4;
    dxChRuleStdNotIpPtr->isArp  = ruleStdNotIpPtr->common.isArp;

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->macDa.arEther,
                 ruleStdNotIpPtr->macDa.arEther,
                 sizeof(dxChRuleStdNotIpPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->macSa.arEther,
                 ruleStdNotIpPtr->macSa.arEther,
                 sizeof(dxChRuleStdNotIpPtr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleStdNotIpPtr, ruleStdNotIpPtr, cfi1);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdNotIpPtr->udb0_3,
                 ruleStdNotIpPtr->udb,
                 sizeof(dxChRuleStdNotIpPtr->udb0_3));
    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos function
* @endinternal
*
* @brief   Convert generic into device specific egress standard IPV4 and IPV6 packets
*         L2 and QOS styled key
* @param[in] ruleStdIpL2QosPtr        - (pointer to) standard IP packets L2 and QOS styled key
*
* @param[out] dxChRuleStdIpL2QosPtr    - (pointer to) DxCh standard IP packets L2 and QOS styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IP_L2_QOS_STC   *ruleStdIpL2QosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC *dxChRuleStdIpL2QosPtr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleStdIpL2QosPtr->common),
                                                    &(dxChRuleStdIpL2QosPtr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert commonEgrStd into device specific format */
    PRV_TGF_STC_COMMON_EGR_STD_IP_FIELD_COPY_MAC(dxChRuleStdIpL2QosPtr, ruleStdIpL2QosPtr);

    /* convert into device specific format */
    dxChRuleStdIpL2QosPtr->isArp    = ruleStdIpL2QosPtr->common.isArp;
    dxChRuleStdIpL2QosPtr->l4Byte13 = ruleStdIpL2QosPtr->commonStdIp.l4Byte13;

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->macDa.arEther,
                 ruleStdIpL2QosPtr->macDa.arEther,
                 sizeof(dxChRuleStdIpL2QosPtr->macDa.arEther));

    /* convert Sip into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->dipBits0to31,
                 ruleStdIpL2QosPtr->commonStdIp.dipBits,
                 sizeof(dxChRuleStdIpL2QosPtr->dipBits0to31));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpL2QosPtr->udb4_7,
                 ruleStdIpL2QosPtr->udb + 4,
                 sizeof(dxChRuleStdIpL2QosPtr->udb4_7));

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleStdIpv4L4 function
* @endinternal
*
* @brief   Convert generic into device specific egress standard IPV4 packet L4 styled key
*
* @param[in] ruleStdIpv4L4Ptr         - (pointer to) standard IPV4 packet L4 styled key
*
* @param[out] dxChRuleStdIpv4L4Ptr     - (pointer to) DxCh standard IPV4 packet L4 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleStdIpv4L4
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IPV4_L4_STC     *ruleStdIpv4L4Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC   *dxChRuleStdIpv4L4Ptr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleStdIpv4L4Ptr->common),
                                                    &(dxChRuleStdIpv4L4Ptr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert commonEgrStd into device specific format */
    PRV_TGF_STC_COMMON_EGR_STD_IP_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, isBc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, l4Byte0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4L4Ptr, ruleStdIpv4L4Ptr, l4Byte1);

    dxChRuleStdIpv4L4Ptr->isArp     = ruleStdIpv4L4Ptr->common.isArp;
    dxChRuleStdIpv4L4Ptr->l4Byte13  = ruleStdIpv4L4Ptr->commonStdIp.l4Byte13;
    dxChRuleStdIpv4L4Ptr->dip.u32Ip = ruleStdIpv4L4Ptr->dip.u32Ip;
    dxChRuleStdIpv4L4Ptr->sip.u32Ip = ruleStdIpv4L4Ptr->sip.u32Ip;

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleStdIpv4L4Ptr->udb8_11,
                 ruleStdIpv4L4Ptr->udb + 8,
                 sizeof(dxChRuleStdIpv4L4Ptr->udb8_11));

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleExtNotIpv6 function
* @endinternal
*
* @brief   Convert generic into device specific egress extended not IP and IPV4 packet key
*
* @param[in] ruleExtNotIpv6Ptr        - (pointer to) extended not IP and IPV4 packet key
*
* @param[out] dxChRuleExtNotIpv6Ptr    - (pointer to) DxCh extended not IP and IPV4 packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleExtNotIpv6
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_NOT_IPV6_STC    *ruleExtNotIpv6Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC  *dxChRuleExtNotIpv6Ptr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleExtNotIpv6Ptr->common),
                                                    &(dxChRuleExtNotIpv6Ptr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert commonEgrExt into device specific format */
    PRV_TGF_STC_COMMON_EGR_EXT_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr);

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, l2Encap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, ipv4Fragmented);

    dxChRuleExtNotIpv6Ptr->dip.u32Ip = ruleExtNotIpv6Ptr->dip.u32Ip;
    dxChRuleExtNotIpv6Ptr->sip.u32Ip = ruleExtNotIpv6Ptr->sip.u32Ip;

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->macDa.arEther,
                 ruleExtNotIpv6Ptr->macDa.arEther,
                 sizeof(dxChRuleExtNotIpv6Ptr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->macSa.arEther,
                 ruleExtNotIpv6Ptr->macSa.arEther,
                 sizeof(dxChRuleExtNotIpv6Ptr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, isMpls);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, numOfMplsLabels);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, protocolTypeAfterMpls);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, mplsLabel0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, mplsExp0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, mplsLabel1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtNotIpv6Ptr, ruleExtNotIpv6Ptr, mplsExp1);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtNotIpv6Ptr->udb12_19,
                 ruleExtNotIpv6Ptr->udb + 12,
                 sizeof(dxChRuleExtNotIpv6Ptr->udb12_19));

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleExtIpv6L2 function
* @endinternal
*
* @brief   Convert generic into device specific egress extended IPV6 packet L2 styled key
*
* @param[in] ruleExtIpv6L2Ptr         - (pointer to) extended IPV6 packet L2 styled key
*
* @param[out] dxChRuleExtIpv6L2Ptr     - (pointer to) DxCh extended IPV6 packet L2 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleExtIpv6L2
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L2_STC     *ruleExtIpv6L2Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC   *dxChRuleExtIpv6L2Ptr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleExtIpv6L2Ptr->common),
                                                    &(dxChRuleExtIpv6L2Ptr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert commonEgrExt into device specific format */
    PRV_TGF_STC_COMMON_EGR_EXT_FIELD_COPY_MAC(dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr);

    /* convert into device specific format */
    dxChRuleExtIpv6L2Ptr->dipBits127to120 = ruleExtIpv6L2Ptr->dipBits;

    /* convert Sip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->sip.u32Ip,
                 ruleExtIpv6L2Ptr->sip.u32Ip,
                 sizeof(dxChRuleExtIpv6L2Ptr->sip.u32Ip));

    /* convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->macDa.arEther,
                 ruleExtIpv6L2Ptr->macDa.arEther,
                 sizeof(dxChRuleExtIpv6L2Ptr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->macSa.arEther,
                 ruleExtIpv6L2Ptr->macSa.arEther,
                 sizeof(dxChRuleExtIpv6L2Ptr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleExtIpv6L2Ptr, ruleExtIpv6L2Ptr, cfi1);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L2Ptr->udb20_27,
                 ruleExtIpv6L2Ptr->udb + 20,
                 sizeof(dxChRuleExtIpv6L2Ptr->udb20_27));

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleExtIpv6L4 function
* @endinternal
*
* @brief   Convert generic into device specific egress extended IPV6 packet L4 styled key
*
* @param[in] ruleExtIpv6L4Ptr         - (pointer to) extended IPV6 packet L4 styled key
*
* @param[out] dxChRuleExtIpv6L4Ptr     - (pointer to) DxCh extended IPV6 packet L4 styled key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleExtIpv6L4
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L4_STC     *ruleExtIpv6L4Ptr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC   *dxChRuleExtIpv6L4Ptr
)
{
    GT_STATUS   rc = GT_OK;


    /* convert common part into device specific format */
    rc = prvTgfConvertGenericToDxChPclRuleEgrCommon(&(ruleExtIpv6L4Ptr->common),
                                                    &(dxChRuleExtIpv6L4Ptr->common));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPclRuleEgrCommon FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert commonEgrExt into device specific format */
    PRV_TGF_STC_COMMON_EGR_EXT_FIELD_COPY_MAC(dxChRuleExtIpv6L4Ptr, ruleExtIpv6L4Ptr);

    /* convert Sip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->sip.u32Ip,
                 ruleExtIpv6L4Ptr->sip.u32Ip,
                 sizeof(dxChRuleExtIpv6L4Ptr->sip.u32Ip));

    /* convert Dip into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->dip.u32Ip,
                 ruleExtIpv6L4Ptr->dip.u32Ip,
                 sizeof(dxChRuleExtIpv6L4Ptr->dip.u32Ip));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv6L4Ptr->udb28_35,
                 ruleExtIpv6L4Ptr->udb + 28,
                 sizeof(dxChRuleExtIpv6L4Ptr->udb28_35));

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos function
* @endinternal
*
* @brief   Convert generic into device specific ingress Standard Routed Qos IPV4 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC   *ruleStdIpv4RoutedAclQosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC *dxChRuleStdIpv4RoutedAclQosPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpCommon(
                               &(ruleStdIpv4RoutedAclQosPtr->ingressIpCommon),
                               &(dxChRuleStdIpv4RoutedAclQosPtr->ingressIpCommon));

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, pktType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, ipFragmented);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, ipHeaderInfo);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, ipPacketLength);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, ttl);


    dxChRuleStdIpv4RoutedAclQosPtr->dip.u32Ip = ruleStdIpv4RoutedAclQosPtr->dip.u32Ip;
    dxChRuleStdIpv4RoutedAclQosPtr->sip.u32Ip = ruleStdIpv4RoutedAclQosPtr->sip.u32Ip;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, vrfId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, tcpUdpPortComparators);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleStdIpv4RoutedAclQosPtr,
                                   ruleStdIpv4RoutedAclQosPtr, isUdbValid);

    /* convert UDB into device specific format */
    dxChRuleStdIpv4RoutedAclQosPtr->udb5 = ruleStdIpv4RoutedAclQosPtr->udb[5];

    cpssOsMemCpy(dxChRuleStdIpv4RoutedAclQosPtr->udb41_44,
                 ruleStdIpv4RoutedAclQosPtr->udb + 41,
                 sizeof(dxChRuleStdIpv4RoutedAclQosPtr->udb41_44));

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos function
* @endinternal
*
* @brief   Convert generic into device specific ingress Extended Port/VLAN Qos IPV4 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC   *ruleExtIpv4PortVlanQosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC *dxChRuleExtIpv4PortVlanQosPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpCommon(
                               &(ruleExtIpv4PortVlanQosPtr->ingressIpCommon),
                               &(dxChRuleExtIpv4PortVlanQosPtr->ingressIpCommon));

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, isBc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, isIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, isArp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, l2Encap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, etherType);

    /*convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->macDa.arEther,
                 ruleExtIpv4PortVlanQosPtr->macDa.arEther,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->macSa.arEther,
                 ruleExtIpv4PortVlanQosPtr->macSa.arEther,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, cfi1);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, ipFragmented);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, ipHeaderInfo);

    dxChRuleExtIpv4PortVlanQosPtr->dip.u32Ip = ruleExtIpv4PortVlanQosPtr->dip.u32Ip;
    dxChRuleExtIpv4PortVlanQosPtr->sip.u32Ip = ruleExtIpv4PortVlanQosPtr->sip.u32Ip;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, vrfId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, trunkHash);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleExtIpv4PortVlanQosPtr,
                                   ruleExtIpv4PortVlanQosPtr, isUdbValid);
    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->UdbStdIpL2Qos,
                 ruleExtIpv4PortVlanQosPtr->UdbStdIpL2Qos,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->UdbStdIpL2Qos));

    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->UdbStdIpV4L4,
                 ruleExtIpv4PortVlanQosPtr->UdbStdIpV4L4,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->UdbStdIpV4L4));

    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->UdbExtIpv6L2,
                 ruleExtIpv4PortVlanQosPtr->UdbExtIpv6L2,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->UdbExtIpv6L2));

    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->UdbExtIpv6L4,
                 ruleExtIpv4PortVlanQosPtr->UdbExtIpv6L4,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->UdbExtIpv6L4));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->udb5_16,
                 ruleExtIpv4PortVlanQosPtr->udb + 5,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->udb5_16));

    cpssOsMemCpy(dxChRuleExtIpv4PortVlanQosPtr->udb31_38,
                 ruleExtIpv4PortVlanQosPtr->udb + 31,
                 sizeof(dxChRuleExtIpv4PortVlanQosPtr->udb31_38));

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos function
* @endinternal
*
* @brief   Convert generic into device specific ingress Ultra Port/VLAN Qos IPV6 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC   *ruleUltraIpv6PortVlanQosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC *dxChRuleUltraIpv6PortVlanQosPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpCommon(
                               &(ruleUltraIpv6PortVlanQosPtr->ingressIpCommon),
                               &(dxChRuleUltraIpv6PortVlanQosPtr->ingressIpCommon));

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isL2Valid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isNd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isBc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, l2Encap);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, etherType);

    /*convert macDa into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->macDa.arEther,
                 ruleUltraIpv6PortVlanQosPtr->macDa.arEther,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->macDa.arEther));

    /* convert macSa into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->macSa.arEther,
                 ruleUltraIpv6PortVlanQosPtr->macSa.arEther,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->macSa.arEther));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, cfi1);

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->sip.u32Ip,
                 ruleUltraIpv6PortVlanQosPtr->sip.u32Ip,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->sip.u32Ip));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->dip.u32Ip,
                 ruleUltraIpv6PortVlanQosPtr->dip.u32Ip,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->dip.u32Ip));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isIpv6HopByHop);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, ipHeaderOk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, vrfId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, isUdbValid);
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtNotIpv6,
                 ruleUltraIpv6PortVlanQosPtr->UdbExtNotIpv6,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtNotIpv6));

    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtIpv6L2,
                 ruleUltraIpv6PortVlanQosPtr->UdbExtIpv6L2,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtIpv6L2));

    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtIpv6L4,
                 ruleUltraIpv6PortVlanQosPtr->UdbExtIpv6L4,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->UdbExtIpv6L4));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->udb0_11,
                 ruleUltraIpv6PortVlanQosPtr->udb,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->udb0_11));

    dxChRuleUltraIpv6PortVlanQosPtr->udb12 =
                                           ruleUltraIpv6PortVlanQosPtr->udb[12];

    cpssOsMemCpy(dxChRuleUltraIpv6PortVlanQosPtr->udb39_40,
                 ruleUltraIpv6PortVlanQosPtr->udb + 39,
                 sizeof(dxChRuleUltraIpv6PortVlanQosPtr->udb39_40));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, srcPortOrTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6PortVlanQosPtr,
                                   ruleUltraIpv6PortVlanQosPtr, trunkHash);

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos function
* @endinternal
*
* @brief   Convert generic into device specific ingress Ultra Routed Qos IPV6 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC   *ruleUltraIpv6RoutedAclQosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC *dxChRuleUltraIpv6RoutedAclQosPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpCommon(
                               &(ruleUltraIpv6RoutedAclQosPtr->ingressIpCommon),
                               &(dxChRuleUltraIpv6RoutedAclQosPtr->ingressIpCommon));

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, pktType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isNd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, tag1Exist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, cfi1);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, mplsOuterLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, mplsOuterLabExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, mplsOuterLabSBit);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, ipPacketLength);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, ipv6HdrFlowLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, ttl);

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->sip.u32Ip,
                 ruleUltraIpv6RoutedAclQosPtr->sip.u32Ip,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->sip.u32Ip));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->dip.u32Ip,
                 ruleUltraIpv6RoutedAclQosPtr->dip.u32Ip,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->dip.u32Ip));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isIpv6HopByHop);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isIpv6LinkLocal);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isIpv6Mld);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, ipHeaderOk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, vrfId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, isUdbValid);
    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdNotIp,
                 ruleUltraIpv6RoutedAclQosPtr->UdbStdNotIp,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdNotIp));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdIpL2Qos,
                 ruleUltraIpv6RoutedAclQosPtr->UdbStdIpL2Qos,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdIpL2Qos));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdIpV4L4,
                 ruleUltraIpv6RoutedAclQosPtr->UdbStdIpV4L4,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbStdIpV4L4));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtNotIpv6,
                 ruleUltraIpv6RoutedAclQosPtr->UdbExtNotIpv6,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtNotIpv6));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L2,
                 ruleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L2,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L2));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L4,
                 ruleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L4,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->UdbExtIpv6L4));

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->udb0_11,
                 ruleUltraIpv6RoutedAclQosPtr->udb,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->udb0_11));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->udb17_22,
                 ruleUltraIpv6RoutedAclQosPtr->udb + 17,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->udb17_22));

    cpssOsMemCpy(dxChRuleUltraIpv6RoutedAclQosPtr->udb45_46,
                 ruleUltraIpv6RoutedAclQosPtr->udb + 45,
                 sizeof(dxChRuleUltraIpv6RoutedAclQosPtr->udb45_46));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, srcPortOrTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleUltraIpv6RoutedAclQosPtr,
                                   ruleUltraIpv6RoutedAclQosPtr, trunkHash);

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChRuleIngressUdbOnly function
* @endinternal
*
* @brief   Convert generic into device specific ingress UDB Only key
*
* @param[in] genRuleIngrUdbOnlyPtr    - (pointer to) generic ingress UDB Only key
*
* @param[out] dxChRuleIngrUdbOnlyPtr   - (pointer to) DxCh ingress UDB Only key
*                                       None.
*/
static GT_VOID prvTgfConvertGenericToDxChRuleIngressUdbOnly
(
    IN  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC    *genRuleIngrUdbOnlyPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC  *dxChRuleIngrUdbOnlyPtr
)
{
    GT_U32 i;

    for (i = 0; (i < PRV_TGF_PCL_UDB_MAX_NUMBER_MAC(prvTgfDevNum)); i++)
    {
        dxChRuleIngrUdbOnlyPtr->udb[i] = genRuleIngrUdbOnlyPtr->udb[i];
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.srcDevIsOwn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.flowId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.srcDevIsOwn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.flowId);
}

/**
* @internal prvTgfConvertGenericToDxChRuleEgressUdbOnly function
* @endinternal
*
* @brief   Convert generic into device specific egress UDB Only key
*
* @param[in] genRuleEgrUdbOnlyPtr     - (pointer to) generic egress UDB Only key
*
* @param[out] dxChRuleEgrUdbOnlyPtr    - (pointer to) DxCh egress UDB Only key
*                                       None.
*/
static GT_VOID prvTgfConvertGenericToDxChRuleEgressUdbOnly
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC    *genRuleEgrUdbOnlyPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC  *dxChRuleEgrUdbOnlyPtr
)
{
    GT_U32 i;

    for (i = 0; (i < PRV_TGF_PCL_UDB_MAX_NUMBER_MAC(prvTgfDevNum)); i++)
    {
        dxChRuleEgrUdbOnlyPtr->udb[i] = genRuleEgrUdbOnlyPtr->udb[i];
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, replacedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, replacedFld.isUdbValid);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.trgPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.srcDev);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.trgDev);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleEgrUdbOnlyPtr, genRuleEgrUdbOnlyPtr, udb60FixedFld.localDevTrgPhyPort);
}

/**
* @internal prvTgfConvertGenericToDxChUdbSelect function
* @endinternal
*
* @brief   Convert generic into device specific UDB Select Structure
*
* @param[in] genUdbSelectPtr          - (pointer to) generic UDB Select Structure
*
* @param[out] dxChUdbSelectPtr         - (pointer to) DxCh UDB Select Structure
*                                       None.
*/
static GT_VOID prvTgfConvertGenericToDxChUdbSelect
(
    IN  PRV_TGF_PCL_UDB_SELECT_STC    *genUdbSelectPtr,
    OUT CPSS_DXCH_PCL_UDB_SELECT_STC  *dxChUdbSelectPtr
)
{
    GT_U32 i;

    for (i = 0; (i < PRV_TGF_PCL_UDB_MAX_NUMBER_MAC(prvTgfDevNum)); i++)
    {
        dxChUdbSelectPtr->udbSelectArr[i] = genUdbSelectPtr->udbSelectArr[i];
    }

    for (i = 0; (i < PRV_TGF_PCL_INGRESS_UDB_REPLACE_MAX_CNS); i++)
    {
        dxChUdbSelectPtr->ingrUdbReplaceArr[i] = genUdbSelectPtr->ingrUdbReplaceArr[i];
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChUdbSelectPtr, genUdbSelectPtr, egrUdb01Replace);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChUdbSelectPtr, genUdbSelectPtr, egrUdbBit15Replace);
}

/**
* @internal prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl function
* @endinternal
*
* @brief   Convert generic into device specific Egress Extended (48B) RACL/VACL IPv4 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC   *ruleEgrExtIpv4RaclVaclPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC *dxChRuleEgrExtIpv4RaclVaclPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpEgrCommon(
                             &(ruleEgrExtIpv4RaclVaclPtr->egressIpCommon),
                             &(dxChRuleEgrExtIpv4RaclVaclPtr->egressIpCommon));

    /* convert into device specific format */

    dxChRuleEgrExtIpv4RaclVaclPtr->dip.u32Ip =
                                           ruleEgrExtIpv4RaclVaclPtr->dip.u32Ip;
    dxChRuleEgrExtIpv4RaclVaclPtr->sip.u32Ip =
                                           ruleEgrExtIpv4RaclVaclPtr->sip.u32Ip;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, ipv4Options);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, isVidx);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrExtIpv4RaclVaclPtr,
                                   ruleEgrExtIpv4RaclVaclPtr, isUdbValid);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleEgrExtIpv4RaclVaclPtr->udb36_49,
                 ruleEgrExtIpv4RaclVaclPtr->udb + 36,
                 sizeof(dxChRuleEgrExtIpv4RaclVaclPtr->udb36_49));

    dxChRuleEgrExtIpv4RaclVaclPtr->udb0 = ruleEgrExtIpv4RaclVaclPtr->udb[14];

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl function
* @endinternal
*
* @brief   Convert generic into device specific Egress Ultra (72B) RACL/VACL IPv6 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl
(
    IN  PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC   *ruleEgrUltraIpv6RaclVaclPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC *dxChRuleEgrUltraIpv6RaclVaclPtr
)
{
    /* convert common part into device specific format */
    prvTgfConvertGenericToDxChPclRuleIpEgrCommon(
                               &(ruleEgrUltraIpv6RaclVaclPtr->egressIpCommon),
                               &(dxChRuleEgrUltraIpv6RaclVaclPtr->egressIpCommon));

    /* convert into device specific format */

    /* convert sip into device specific format */
    cpssOsMemCpy(dxChRuleEgrUltraIpv6RaclVaclPtr->sip.u32Ip,
                 ruleEgrUltraIpv6RaclVaclPtr->sip.u32Ip,
                 sizeof(dxChRuleEgrUltraIpv6RaclVaclPtr->sip.u32Ip));

    /* convert dip into device specific format */
    cpssOsMemCpy(dxChRuleEgrUltraIpv6RaclVaclPtr->dip.u32Ip,
                 ruleEgrUltraIpv6RaclVaclPtr->dip.u32Ip,
                 sizeof(dxChRuleEgrUltraIpv6RaclVaclPtr->dip.u32Ip));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, isNd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, isIpv6ExtHdrExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, isIpv6HopByHop);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, isVidx);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, trgPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleEgrUltraIpv6RaclVaclPtr,
                                   ruleEgrUltraIpv6RaclVaclPtr, isUdbValid);

    /* convert UDB into device specific format */
    cpssOsMemCpy(dxChRuleEgrUltraIpv6RaclVaclPtr->udb1_4,
                 ruleEgrUltraIpv6RaclVaclPtr->udb + 1,
                 sizeof(dxChRuleEgrUltraIpv6RaclVaclPtr->udb1_4));

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChLookupConfig function
* @endinternal
*
* @brief   Convert generic into device specific configuration of IPCL lookups
*
* @param[in] lookupConfigPtr          - (pointer to) configuration of IPCL lookups
*
* @param[out] dxChLookupConfigPtr      - (pointer to) DxCh configuration of IPCL lookups
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChLookupConfig
(
    IN  PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC    *lookupConfigPtr,
    OUT CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC  *dxChLookupConfigPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChLookupConfigPtr, lookupConfigPtr, ipclConfigIndex);

    /* convert pcl0_1OverrideConfigIndex into device specific format */
    switch (lookupConfigPtr->pcl0_1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChLookupConfigPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChLookupConfigPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex into device specific format */
    switch (lookupConfigPtr->pcl1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChLookupConfigPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChLookupConfigPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
/**
* @internal prvTgfConvertDxChToGenericLookupConfig function
* @endinternal
*
* @brief   Convert generic into device specific configuration of IPCL lookups
*
* @param[in] dxChLookupConfigPtr - (pointer to) DxCh
*                                  configuration of IPCL lookups
* @param[out] lookupConfigPtr - (pointer to) configuration of
*                                IPCL lookups
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericLookupConfig
(
    IN  CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC  *dxChLookupConfigPtr,
    OUT PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC    *lookupConfigPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChLookupConfigPtr, lookupConfigPtr, ipclConfigIndex);

    /* convert pcl0_1OverrideConfigIndex into device specific format */
    switch (dxChLookupConfigPtr->pcl0_1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            lookupConfigPtr->pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            lookupConfigPtr->pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex into device specific format */
    switch (dxChLookupConfigPtr->pcl1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            lookupConfigPtr->pcl1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            lookupConfigPtr->pcl1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChQos function
* @endinternal
*
* @brief   Convert generic into device specific QoS attributes mark actions
*/
static GT_VOID prvTgfConvertGenericToDxChQos
(
    IN  PRV_TGF_PCL_ACTION_QOS_STC      *qosAttributesPtr,
    IN  GT_BOOL                          isEgressPolicy,
    OUT CPSS_DXCH_PCL_ACTION_QOS_UNT    *dxChQosAttributesPtr
)
{

    if (GT_TRUE == isEgressPolicy)
    {
        /* convert egress members into device specific format */
        switch (qosAttributesPtr->egressDscpCmd)
        {
            default:
            case PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E:
                dxChQosAttributesPtr->egress.modifyDscp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E:
                dxChQosAttributesPtr->egress.modifyDscp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E:
                dxChQosAttributesPtr->egress.modifyDscp =
                    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E;
                break;
        }

        switch (qosAttributesPtr->egressUp0Cmd)
        {
            default:
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                dxChQosAttributesPtr->egress.modifyUp   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                dxChQosAttributesPtr->egress.modifyUp   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                dxChQosAttributesPtr->egress.modifyUp   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
                break;
        }
        dxChQosAttributesPtr->egress.dscp   =
            qosAttributesPtr->dscp;
        dxChQosAttributesPtr->egress.up     =
            qosAttributesPtr->up;
        dxChQosAttributesPtr->egress.up1ModifyEnable    =
            qosAttributesPtr->egressUp1ModifyEnable;
        dxChQosAttributesPtr->egress.up1    =
            qosAttributesPtr->up1;
    }
    else
    {
        /* convert ingress members into device specific format */
        dxChQosAttributesPtr->ingress.modifyDscp =
            qosAttributesPtr->modifyDscp;
        dxChQosAttributesPtr->ingress.modifyUp =
             qosAttributesPtr->modifyUp;
        dxChQosAttributesPtr->ingress.profileIndex =
            qosAttributesPtr->profileIndex;
        dxChQosAttributesPtr->ingress.profileAssignIndex =
            qosAttributesPtr->profileAssignIndex;
        dxChQosAttributesPtr->ingress.profilePrecedence =
            qosAttributesPtr->profilePrecedence;
        switch (qosAttributesPtr->ingressUp1Cmd)
        {
            default:
            case PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E:
                dxChQosAttributesPtr->ingress.up1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
                break;
            case PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E:
                dxChQosAttributesPtr->ingress.up1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E;
                break;
            case PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E:
                dxChQosAttributesPtr->ingress.up1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E;
                break;
            case PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ALL_E:
                dxChQosAttributesPtr->ingress.up1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E;
                break;
        }
        dxChQosAttributesPtr->ingress.up1 =
            qosAttributesPtr->up1;
    }
}
/**
* @internal prvTgfConvertDxChToGenericQos function
* @endinternal
*
* @brief   Convert device specific into generic QoS attributes
*          mark actions
*/
static GT_VOID prvTgfConvertDxChToGenericQos
(
    IN  CPSS_DXCH_PCL_ACTION_QOS_UNT    *dxChQosAttributesPtr,
    IN  GT_BOOL                          isEgressPolicy,
    OUT PRV_TGF_PCL_ACTION_QOS_STC      *qosAttributesPtr

)
{
    if (GT_TRUE == isEgressPolicy)
    {
        /* convert egress members into device specific format */
        switch (dxChQosAttributesPtr->egress.modifyDscp)
        {
            default:
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E:
                qosAttributesPtr->egressDscpCmd = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E:
                qosAttributesPtr->egressDscpCmd = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E:
                qosAttributesPtr->egressDscpCmd = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E;
                break;
        }

        switch (dxChQosAttributesPtr->egress.modifyUp)
        {
            default:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                qosAttributesPtr->egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                qosAttributesPtr->egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                qosAttributesPtr->egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
                break;
        }
        qosAttributesPtr->dscp = dxChQosAttributesPtr->egress.dscp;
        qosAttributesPtr->up   = dxChQosAttributesPtr->egress.up;
        qosAttributesPtr->egressUp1ModifyEnable = dxChQosAttributesPtr->egress.up1ModifyEnable;
        qosAttributesPtr->up1  = dxChQosAttributesPtr->egress.up1;
    }
    else
    {
        /* convert ingress members into device specific format */
        qosAttributesPtr->modifyDscp = dxChQosAttributesPtr->ingress.modifyDscp;
        qosAttributesPtr->modifyUp = dxChQosAttributesPtr->ingress.modifyUp;
        qosAttributesPtr->profileIndex = dxChQosAttributesPtr->ingress.profileIndex;
        qosAttributesPtr->profileAssignIndex = dxChQosAttributesPtr->ingress.profileAssignIndex;
        qosAttributesPtr->profilePrecedence = dxChQosAttributesPtr->ingress.profilePrecedence;
        switch (dxChQosAttributesPtr->ingress.up1Cmd)
        {
            default:
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E:
                qosAttributesPtr->ingressUp1Cmd = PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
                break;
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E:
                qosAttributesPtr->ingressUp1Cmd = PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E;
                break;
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E:
                qosAttributesPtr->ingressUp1Cmd = PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E;
                break;
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E:
                qosAttributesPtr->ingressUp1Cmd = PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ALL_E;
                break;
        }
        qosAttributesPtr->up1 = dxChQosAttributesPtr->ingress.up1;
    }
}


/**
* @internal prvTgfConvertGenericToDxChRedirect function
* @endinternal
*
* @brief   Convert generic into device specific redirection related actions
*
* @param[in] redirectPtr              - (pointer to) redirection related actions
*
* @param[out] dxChRedirectPtr          - (pointer to) DxCh redirection related actions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChRedirect
(
    IN  PRV_TGF_PCL_ACTION_REDIRECT_STC     *redirectPtr,
    OUT CPSS_DXCH_PCL_ACTION_REDIRECT_STC   *dxChRedirectPtr
)
{
    cpssOsMemSet(dxChRedirectPtr, 0, sizeof(*dxChRedirectPtr));

    /* convert redirectCmd into device specific format */
    switch (redirectPtr->redirectCmd)
    {
        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E:
            dxChRedirectPtr->redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            dxChRedirectPtr->data.noRedirect.arpPtr =
                redirectPtr->data.noRedirect.arpPtr;
            dxChRedirectPtr->data.noRedirect.modifyMacDa =
                redirectPtr->data.noRedirect.modifyMacDa;
            break;

        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            dxChRedirectPtr->redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            /* convert into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), vntL2Echo);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), modifyMacDa);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), modifyMacSa);

            /* convert outInterface into device specific format */
            cpssOsMemCpy(&(dxChRedirectPtr->data.outIf.outInterface),
                         &(redirectPtr->data.outIf.outInterface),
                         sizeof(dxChRedirectPtr->data.outIf.outInterface));

            switch (redirectPtr->data.outIf.outlifType)
            {
                case PRV_TGF_OUTLIF_TYPE_LL_E:
                    dxChRedirectPtr->data.outIf.arpPtr =
                        redirectPtr->data.outIf.outlifPointer.arpPtr;

                    break;
                case PRV_TGF_OUTLIF_TYPE_TUNNEL_E:
                    dxChRedirectPtr->data.outIf.tunnelStart = GT_TRUE;
                    dxChRedirectPtr->data.outIf.tunnelPtr   =
                        redirectPtr->data.outIf.outlifPointer.tunnelStartPtr.ptr;
                    /* convert tunnelType into device specific format */
                    switch (redirectPtr->data.outIf.outlifPointer.tunnelStartPtr.tunnelType)
                    {
                        case PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                            dxChRedirectPtr->data.outIf.tunnelType =
                                CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
                            break;

                        case PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                            dxChRedirectPtr->data.outIf.tunnelType =
                                CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
                            break;

                        default:
                            return GT_BAD_PARAM;
                    }
                    break;
                /* DIT - Dowstream interface - not supported for DX devices */
                case PRV_TGF_OUTLIF_TYPE_DIT_E:
                default:
                    return GT_BAD_PARAM;
            }

            break;

        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            dxChRedirectPtr->redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E;

            /* convert into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data),
                                           &(redirectPtr->data), routerLttIndex);

            break;

        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            dxChRedirectPtr->redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;

            /* convert into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data),
                                           &(redirectPtr->data), vrfId);

            break;

        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
            dxChRedirectPtr->redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E;

            /* convert into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(
                &(dxChRedirectPtr->data.modifyMacSa),
                &(redirectPtr->data.modifyMacSa), macSa);

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(
                &(dxChRedirectPtr->data.modifyMacSa),
                &(redirectPtr->data.modifyMacSa), arpPtr);

            break;

        case PRV_TGF_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E:
            dxChRedirectPtr->redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E;
            /* convert into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), sourceMeshIdSetEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), sourceMeshId);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), userTagAcEnable);

            /* convert logicalInterface into device specific format */
            cpssOsMemCpy(&(dxChRedirectPtr->data.logicalSourceInterface.logicalInterface),
                         &(redirectPtr->data.logicalSourceInterface.logicalInterface),
                         sizeof(dxChRedirectPtr->data.logicalSourceInterface.logicalInterface));

            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericRedirect function
* @endinternal
*
* @brief   Convert device specific into generic redirection
*          related actions
*
* @param[in] dxChRedirectPtr  - (pointer to) DxCh redirection
*                               related actions
*@param[out] redirectPtr      - (pointer to) redirection related
*                               actions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericRedirect
(
    IN  CPSS_DXCH_PCL_ACTION_REDIRECT_STC   *dxChRedirectPtr,
    OUT PRV_TGF_PCL_ACTION_REDIRECT_STC     *redirectPtr
)
{
     cpssOsMemSet(redirectPtr, 0, sizeof(*redirectPtr));

    /* convert redirectCmd into device specific format */
    switch (dxChRedirectPtr->redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            redirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E;
            redirectPtr->data.noRedirect.arpPtr = dxChRedirectPtr->data.noRedirect.arpPtr;
            redirectPtr->data.noRedirect.modifyMacDa = dxChRedirectPtr->data.noRedirect.modifyMacDa;
            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            redirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            /* convert into generic format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), vntL2Echo);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), modifyMacDa);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.outIf),
                                           &(redirectPtr->data.outIf), modifyMacSa);

            /* convert outInterface into device specific format */
            cpssOsMemCpy(&(redirectPtr->data.outIf.outInterface),
                         &(dxChRedirectPtr->data.outIf.outInterface),
                         sizeof(redirectPtr->data.outIf.outInterface));

            if(dxChRedirectPtr->data.outIf.tunnelStart == GT_TRUE)
            {
                redirectPtr->data.outIf.outlifType=PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
                redirectPtr->data.outIf.outlifPointer.tunnelStartPtr.ptr=dxChRedirectPtr->data.outIf.tunnelPtr;
                /* convert tunnelType into generic format */
                switch (dxChRedirectPtr->data.outIf.tunnelType)
                {
                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                        redirectPtr->data.outIf.outlifPointer.tunnelStartPtr.tunnelType=
                            PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
                        break;

                    case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                        redirectPtr->data.outIf.outlifPointer.tunnelStartPtr.tunnelType=
                            PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
                        break;

                    default:
                        return GT_BAD_PARAM;
                }
            }
            else
            {
                redirectPtr->data.outIf.outlifType= PRV_TGF_OUTLIF_TYPE_LL_E;
                redirectPtr->data.outIf.outlifPointer.arpPtr = dxChRedirectPtr->data.outIf.arpPtr;
            }
            break;

         case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            redirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;

            /* convert into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data),
                                           &(redirectPtr->data), routerLttIndex);

            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            redirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;

            /* convert into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data),
                                           &(redirectPtr->data), vrfId);

            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
            redirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E;

            /* convert into generic format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.modifyMacSa),
                                           &(redirectPtr->data.modifyMacSa), macSa);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.modifyMacSa),
                                           &(redirectPtr->data.modifyMacSa), arpPtr);

            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E:
            dxChRedirectPtr->redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E;
            /* convert into generic format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), sourceMeshIdSetEnable);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), sourceMeshId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRedirectPtr->data.logicalSourceInterface),
                                           &(redirectPtr->data.logicalSourceInterface), userTagAcEnable);

            /* convert logicalInterface into generic format */
            cpssOsMemCpy(&(redirectPtr->data.logicalSourceInterface.logicalInterface),
                         &(dxChRedirectPtr->data.logicalSourceInterface.logicalInterface),
                         sizeof(redirectPtr->data.logicalSourceInterface.logicalInterface));

            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPolicer function
* @endinternal
*
* @brief   Convert generic into device specific policer related actions
*
* @param[in] policerPtr               - (pointer to) policer related actions
*
* @param[out] dxChPolicerPtr           - (pointer to) DxCh policer related actions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPolicer
(
    IN  PRV_TGF_PCL_ACTION_POLICER_STC      *policerPtr,
    OUT CPSS_DXCH_PCL_ACTION_POLICER_STC    *dxChPolicerPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChPolicerPtr, policerPtr, policerId);

    /* convert policerEnable into device specific format */
    switch (policerPtr->policerEnable)
    {
        case PRV_TGF_PCL_POLICER_DISABLE_ALL_E:
            dxChPolicerPtr->policerEnable = CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
            break;

        case PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
            dxChPolicerPtr->policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            break;

        case PRV_TGF_PCL_POLICER_ENABLE_METER_ONLY_E:
            dxChPolicerPtr->policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
            break;

        case PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
            dxChPolicerPtr->policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericPolicer function
* @endinternal
*
* @brief   Convert device specific into generic policer related
*          actions
*
* @param[in] dxChPolicerPtr - (pointer to) DxCh policer related
*                             actions
*@param[out] policerPtr     - (pointer to) policer related
*                             actions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericPolicer
(
    IN  CPSS_DXCH_PCL_ACTION_POLICER_STC    *dxChPolicerPtr,
    OUT PRV_TGF_PCL_ACTION_POLICER_STC      *policerPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChPolicerPtr, policerPtr, policerId);

    /* convert policerEnable into device specific format */
    switch (dxChPolicerPtr->policerEnable)
    {
        case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
            policerPtr->policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
            break;

        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
            policerPtr->policerEnable = PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            break;

        case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
            policerPtr->policerEnable = PRV_TGF_PCL_POLICER_ENABLE_METER_ONLY_E;
            break;

        case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
            policerPtr->policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChMirror function
* @endinternal
*
* @brief   Convert generic into device specific configuration of
*          mirror
*
* @param[in] mirrorPtr  - (pointer to) configuration of IPCL
*       mirror
*
* @param[out] dxchMirrorPtr - (pointer to) DxCh configuration of
*       IPCL mirror
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChMirror
(
    IN  PRV_TGF_PCL_ACTION_MIRROR_STC           *mirrorPtr,
    OUT CPSS_DXCH_PCL_ACTION_MIRROR_STC         *dxchMirrorPtr
)
{
    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, cpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorToRxAnalyzerPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, ingressMirrorToAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorTcpRstAndFinPacketsToCpu);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorToTxAnalyzerPortEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, egressMirrorToAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, egressMirrorToAnalyzerMode);

    /* convert egressMirrorToAnalyzerMode into device specific format */
    switch (mirrorPtr->egressMirrorToAnalyzerMode)
    {
        case PRV_TGF_MIRROR_EGRESS_NOT_DROPPED_E:
            dxchMirrorPtr->egressMirrorToAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;
            break;
        case PRV_TGF_MIRROR_EGRESS_TAIL_DROP_E:
            dxchMirrorPtr->egressMirrorToAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
            break;
        case PRV_TGF_MIRROR_EGRESS_CONGESTION_E:
            dxchMirrorPtr->egressMirrorToAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;
            break;
        case PRV_TGF_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E:
            dxchMirrorPtr->egressMirrorToAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericMirror function
* @endinternal
*
* @brief  Convert device specific into generic configuration of
*         mirror
*
* @param[in] dxchMirrorPtr - (pointer to) DxCh configuration of
*       IPCL Mirror
* @param[out] mirrorPtr - (pointer to) configuration of
*       IPCL Mirror
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/

static GT_STATUS prvTgfConvertDxChToGenericMirror
(
    IN  CPSS_DXCH_PCL_ACTION_MIRROR_STC         *dxchMirrorPtr,
    OUT PRV_TGF_PCL_ACTION_MIRROR_STC           *mirrorPtr
)
{
    /* convert into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, cpuCode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorToRxAnalyzerPort);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, ingressMirrorToAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorTcpRstAndFinPacketsToCpu);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, mirrorToTxAnalyzerPortEn);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, egressMirrorToAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxchMirrorPtr,mirrorPtr, egressMirrorToAnalyzerMode);

    /* convert egressMirrorToAnalyzerMode into device specific format */
    switch (dxchMirrorPtr->egressMirrorToAnalyzerMode)
    {
        case CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E:
            mirrorPtr->egressMirrorToAnalyzerMode = PRV_TGF_MIRROR_EGRESS_NOT_DROPPED_E;
            break;
        case CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E:
            mirrorPtr->egressMirrorToAnalyzerMode = PRV_TGF_MIRROR_EGRESS_TAIL_DROP_E;
            break;
        case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E:
            mirrorPtr->egressMirrorToAnalyzerMode = PRV_TGF_MIRROR_EGRESS_CONGESTION_E;
            break;
        case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E:
            mirrorPtr->egressMirrorToAnalyzerMode = PRV_TGF_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
   return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChRuleAction function
* @endinternal
*
* @brief   Convert generic into device specific Policy Engine Action
*
* @param[in] ruleActionPtr            - (pointer to) Policy Engine Action
*
* @param[out] dxChRuleActionPtr        - (pointer to) DxCh Policy Engine Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChRuleAction
(
    IN  PRV_TGF_PCL_ACTION_STC      *ruleActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC    *dxChRuleActionPtr
)
{
    GT_STATUS   rc = GT_OK;

    cpssOsMemSet(dxChRuleActionPtr,0,sizeof(*dxChRuleActionPtr));

    /* convert into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, pktCmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, actionStop);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, bypassBridge);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, bypassIngressPipe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, egressPolicy);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, terminateCutThroughMode);

    /* convert lookupConfig into device specific format */
    rc = prvTgfConvertGenericToDxChLookupConfig(&(ruleActionPtr->lookupConfig),
                                                &(dxChRuleActionPtr->lookupConfig));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChLookupConfig FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mirroring related actions into device specific format */
    rc = prvTgfConvertGenericToDxChMirror(&(ruleActionPtr->mirror),
                                          &(dxChRuleActionPtr->mirror));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMirror FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert rule match counter into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->matchCounter),
                                   &(ruleActionPtr->matchCounter), enableMatchCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->matchCounter),
                                   &(ruleActionPtr->matchCounter), matchCounterIndex);

    /* convert QoS attributes mark actions into device specific format */
    prvTgfConvertGenericToDxChQos(&(ruleActionPtr->qos),
                                  ruleActionPtr->egressPolicy,
                                  &(dxChRuleActionPtr->qos));

    /* convert redirection related actions into device specific format */
    rc = prvTgfConvertGenericToDxChRedirect(&(ruleActionPtr->redirect),
                                            &(dxChRuleActionPtr->redirect));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRedirect FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((ruleActionPtr->redirect.redirectCmd == PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
       && (CPSS_INTERFACE_PORT_E == ruleActionPtr->redirect.data.outIf.outInterface.type))
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)ruleActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                                     &dxChRuleActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* convert policer related actions into device specific format */
    rc = prvTgfConvertGenericToDxChPolicer(&(ruleActionPtr->policer),
                                            &(dxChRuleActionPtr->policer));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicer FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert VLAN modification configuration into device specific format */
    if (dxChRuleActionPtr->egressPolicy == GT_FALSE)
    {
        dxChRuleActionPtr->vlan.ingress.modifyVlan =
            ruleActionPtr->vlan.modifyVlan;
        dxChRuleActionPtr->vlan.ingress.nestedVlan =
            ruleActionPtr->vlan.nestedVlan;
        dxChRuleActionPtr->vlan.ingress.precedence =
            ruleActionPtr->vlan.precedence;
        dxChRuleActionPtr->vlan.ingress.vlanId =
            ruleActionPtr->vlan.vlanId;
        switch (ruleActionPtr->vlan.ingressVlanId1Cmd)
        {
            default:
            case PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E:
                dxChRuleActionPtr->vlan.ingress.vlanId1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
                break;
            case PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E:
                dxChRuleActionPtr->vlan.ingress.vlanId1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E;
                break;
            case PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E:
                dxChRuleActionPtr->vlan.ingress.vlanId1Cmd =
                    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
                break;
        }
        dxChRuleActionPtr->vlan.ingress.vlanId1 =
            ruleActionPtr->vlan.vlanId1;
    }
    else
    {
        switch (ruleActionPtr->vlan.egressVlanId0Cmd)
        {
            default:
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                dxChRuleActionPtr->vlan.egress.vlanCmd   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                dxChRuleActionPtr->vlan.egress.vlanCmd   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            case PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                dxChRuleActionPtr->vlan.egress.vlanCmd   =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
                break;
        }

        dxChRuleActionPtr->vlan.egress.vlanId =
            ruleActionPtr->vlan.vlanId;

        dxChRuleActionPtr->vlan.egress.vlanId1ModifyEnable =
            ruleActionPtr->vlan.egressVlanId1ModifyEnable;

        dxChRuleActionPtr->vlan.egress.vlanId1 =
            ruleActionPtr->vlan.vlanId1;
    }

    /* convert Ip Unicast Route into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), doIpUcRoute);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), arpDaIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), decrementTTL);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), bypassTTLCheck);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), icmpRedirectCheck);

    /* convert packet source Id assignment into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourceId),
                                   &(ruleActionPtr->sourceId), assignSourceId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourceId),
                                   &(ruleActionPtr->sourceId), sourceIdValue);
    /* convert packet OAM assignment into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), timeStampEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), offsetIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), oamProcessEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), oamProfile);

    dxChRuleActionPtr->flowId = ruleActionPtr->flowId;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourcePort),
                                   &(ruleActionPtr->sourcePort), assignSourcePortEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourcePort),
                                   &(ruleActionPtr->sourcePort), sourcePortValue);

    dxChRuleActionPtr->setMacToMe = ruleActionPtr->setMacToMe;
    dxChRuleActionPtr->channelTypeToOpcodeMapEnable =
        ruleActionPtr->channelTypeToOpcodeMapEnable;
    dxChRuleActionPtr->tmQueueId = ruleActionPtr->tmQueueId;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, unknownSaCommandEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, unknownSaCommand);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->latencyMonitor),
                                   &(ruleActionPtr->latencyMonitor), latencyProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChRuleActionPtr->latencyMonitor),
                                   &(ruleActionPtr->latencyMonitor), monitoringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, skipFdbSaLookup);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, triggerInterrupt);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, exactMatchOverPclEn);

    return rc;
}
/**
* @internal prvTgfConvertDxChToGenericRuleAction function
* @endinternal
*
* @brief   Convert device specific Policy Engine into generic
*          Action
*
* @param[in] dxChRuleActionPtr - (pointer to) DxCh Policy Engine
*                                 Action
*
* @param[out] ruleActionPtr  - (pointer to) Policy Engine Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChToGenericRuleAction
(
    IN  CPSS_DXCH_PCL_ACTION_STC    *dxChRuleActionPtr,
    OUT PRV_TGF_PCL_ACTION_STC      *ruleActionPtr
)
{
    GT_STATUS   rc = GT_OK;

    cpssOsMemSet(ruleActionPtr,0,sizeof(*ruleActionPtr));

    /* convert into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, pktCmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, actionStop);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, bypassBridge);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, bypassIngressPipe);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, egressPolicy);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, terminateCutThroughMode);

    /* convert lookupConfig into generic format */
    rc = prvTgfConvertDxChToGenericLookupConfig(&(dxChRuleActionPtr->lookupConfig),
                                                &(ruleActionPtr->lookupConfig));

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericLookupConfig FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mirroring related actions into generic format */
    rc = prvTgfConvertDxChToGenericMirror(&(dxChRuleActionPtr->mirror),
                                           &(ruleActionPtr->mirror));
         if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericMirror FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert rule match counter into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->matchCounter),
                                   &(ruleActionPtr->matchCounter), enableMatchCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->matchCounter),
                                   &(ruleActionPtr->matchCounter), matchCounterIndex);

    /* convert QoS attributes mark actions into generic format */
    prvTgfConvertDxChToGenericQos(&(dxChRuleActionPtr->qos),
                                  dxChRuleActionPtr->egressPolicy,
                                  &(ruleActionPtr->qos));

    /* convert redirection related actions into generic format */
    rc = prvTgfConvertDxChToGenericRedirect(&(dxChRuleActionPtr->redirect),
                                            &(ruleActionPtr->redirect));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRedirect FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((dxChRuleActionPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
       && (CPSS_INTERFACE_PORT_E == dxChRuleActionPtr->redirect.data.outIf.outInterface.type))
    {
        rc = prvUtfHwDeviceNumberGet(dxChRuleActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                                     &ruleActionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* convert policer related actions into generic format */
    rc = prvTgfConvertDxChToGenericPolicer(&(dxChRuleActionPtr->policer),
                                           &(ruleActionPtr->policer));

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericPolicer FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert VLAN modification configuration into generic format */
    if (dxChRuleActionPtr->egressPolicy == GT_FALSE)
    {
        ruleActionPtr->vlan.modifyVlan = dxChRuleActionPtr->vlan.ingress.modifyVlan;
        ruleActionPtr->vlan.nestedVlan = dxChRuleActionPtr->vlan.ingress.nestedVlan;
        ruleActionPtr->vlan.precedence = dxChRuleActionPtr->vlan.ingress.precedence;
        ruleActionPtr->vlan.vlanId     = (GT_U16)dxChRuleActionPtr->vlan.ingress.vlanId;

        switch (dxChRuleActionPtr->vlan.ingress.vlanId1Cmd)
        {
            default:
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E:
                ruleActionPtr->vlan.ingressVlanId1Cmd =
                    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
                break;
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E:
                ruleActionPtr->vlan.ingressVlanId1Cmd =
                    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E;
                break;
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E:
                ruleActionPtr->vlan.ingressVlanId1Cmd =
                    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
                break;
        }
        ruleActionPtr->vlan.vlanId1 =
            dxChRuleActionPtr->vlan.ingress.vlanId1;
    }
    else
    {
        switch (dxChRuleActionPtr->vlan.egress.vlanCmd)
        {
            default:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
                ruleActionPtr->vlan.egressVlanId0Cmd   =
                    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
                ruleActionPtr->vlan.egressVlanId0Cmd   =
                    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                ruleActionPtr->vlan.egressVlanId0Cmd   =
                    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
                break;
        }

        ruleActionPtr->vlan.vlanId = (GT_U16)dxChRuleActionPtr->vlan.egress.vlanId;
        ruleActionPtr->vlan.egressVlanId1ModifyEnable = dxChRuleActionPtr->vlan.egress.vlanId1ModifyEnable;
        ruleActionPtr->vlan.vlanId1 = dxChRuleActionPtr->vlan.egress.vlanId1;
    }

    /* convert Ip Unicast Route into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), doIpUcRoute);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), arpDaIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), decrementTTL);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), bypassTTLCheck);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->ipUcRoute),
                                   &(ruleActionPtr->ipUcRoute), icmpRedirectCheck);

    /* convert packet source Id assignment into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourceId),
                                   &(ruleActionPtr->sourceId), assignSourceId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourceId),
                                   &(ruleActionPtr->sourceId), sourceIdValue);
    /* convert packet OAM assignment into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), timeStampEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), offsetIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), oamProcessEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->oam),
                                   &(ruleActionPtr->oam), oamProfile);

    ruleActionPtr->flowId = dxChRuleActionPtr->flowId;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourcePort),
                                   &(ruleActionPtr->sourcePort), assignSourcePortEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->sourcePort),
                                   &(ruleActionPtr->sourcePort), sourcePortValue);

    ruleActionPtr->setMacToMe = dxChRuleActionPtr->setMacToMe;
    ruleActionPtr->channelTypeToOpcodeMapEnable = dxChRuleActionPtr->channelTypeToOpcodeMapEnable;
    ruleActionPtr->tmQueueId = dxChRuleActionPtr->tmQueueId;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, unknownSaCommandEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, unknownSaCommand);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->latencyMonitor),
                                   &(ruleActionPtr->latencyMonitor), latencyProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChRuleActionPtr->latencyMonitor),
                                   &(ruleActionPtr->latencyMonitor), monitoringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, skipFdbSaLookup);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, triggerInterrupt);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChRuleActionPtr, ruleActionPtr, exactMatchOverPclEn);

    return rc;
}

#endif /* CHX_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPclInit function
* @endinternal
*
* @brief   The function initializes the Policy engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPclInit
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclInit(devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclInit FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclIngressPolicyEnable function
* @endinternal
*
* @brief   Enables Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclIngressPolicyEnable
(
    IN GT_BOOL                        enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclIngressPolicyEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclIngressPolicyEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEgressPolicyEnable function
* @endinternal
*
* @brief   Enables Egress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressPolicyEnable
(
    IN GT_BOOL                        enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh2PclEgressPolicyEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh2PclEgressPolicyEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortIngressPolicyEnable
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPclPortIngressPolicyEnable(prvTgfDevsArray[portIter], portNum, enable);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @param[in] devNum                   - device number
* @param[in] ePortNum                 - ePort number
* @param[in] enable                   - enable/disable Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEPortIngressPolicyEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                         ePortNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPclPortIngressPolicyEnable(
        devNum, ePortNum, enable);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEgressPclPacketTypesSet function
* @endinternal
*
* @brief   Enables/disables egress policy per packet type and port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pktType                  - packet type
* @param[in] enable                   - enable/disable Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressPclPacketTypesSet
(
    IN GT_U8                             devNum,
    IN GT_U32                            portNum,
    IN PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT   pktType,
    IN GT_BOOL                           enable
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT dxChPktType;

    switch (pktType)
    {
        case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_DATA_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TS_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TS_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_NON_TS_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxCh2EgressPclPacketTypesSet(
        devNum, portNum,
        dxChPktType, enable);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEgressPclPacketTypesGet function
* @endinternal
*
* @brief   Get status of Egress PCL (EPCL) for set of packet types on port
*
* @param[in] devNum                   - device number
*                                      port          - port number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
*
* @param[out] enablePtr                - enable EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPclEgressPclPacketTypesGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            portNum,
    IN  PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT   pktType,
    OUT GT_BOOL                           *enablePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT dxChPktType;

    switch (pktType)
    {
        case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_DATA_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_TS_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_TS_E;
            break;
        case PRV_TGF_PCL_EGRESS_PKT_NON_TS_E:
            dxChPktType = CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxCh2EgressPclPacketTypesGet(
        devNum, portNum,
        dxChPktType, enablePtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclPortLookupCfgTabAccessModeSet function
* @endinternal
*
* @brief   Configures VLAN/PORT access mode to Ingress or Egress PCL
*         configuration table perlookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortLookupCfgTabAccessModeSet
(
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_PCL_DIRECTION_ENT                       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                   lookupNum,
    IN GT_U32                                       sublookup,
    IN PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
)
{
    GT_U32  portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT      dxChMode;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert table access mode into device specific format */
    switch (mode)
    {
        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E:
            dxChMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;
            break;

        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E:
            dxChMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E;
            break;
        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E:
            dxChMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChPclPortLookupCfgTabAccessModeSet(
        prvTgfDevsArray[portIter], portNum,
        direction, lookupNum, sublookup, dxChMode);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(sublookup);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclPortLookupCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @param[out] modePtr                  - (pointer to)PCL Configuration Table access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortLookupCfgTabAccessModeGet
(
    IN GT_PORT_NUM                                   portNum,
    IN CPSS_PCL_DIRECTION_ENT                        direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                    lookupNum,
    IN GT_U32                                        sublookup,
    OUT PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT *modePtr
)
{
    GT_U32  portIter = 0;
    GT_STATUS rc=GT_FAIL;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT      dxChMode;

    /* call device specific API */
    rc = cpssDxChPclPortLookupCfgTabAccessModeGet(
                                    prvTgfDevsArray[portIter], portNum,
                                    direction, lookupNum, sublookup, &dxChMode);
    if (GT_OK == rc)
    {
        /* convert table access mode fromo device specific format */
        switch (dxChMode)
        {
            case CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E:
                *modePtr = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;
                break;

            case CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E:
                *modePtr = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E;
                break;

            case CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E:
                *modePtr = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E;
                break;

            default:
                rc = GT_BAD_PARAM;
                break;
        }
    }
#endif /* CHX_FAMILY */


#if !(defined CHX_FAMILY)
    rc = GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
    return rc;
}

/**
* @internal prvTgfPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @param[in] interfaceInfoPtr         - (pointer to) interface data
* @param[in] direction                - Policy direction
* @param[in] lookupNum                - Lookup number
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclCfgTblSet
(
    IN CPSS_INTERFACE_INFO_STC       *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC       interfaceInfo;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    dxChLookupCfg = {0};
#endif /* CHX_FAMILY */

    cpssOsMemCpy(&interfaceInfo, interfaceInfoPtr, sizeof(interfaceInfo));

#ifdef CHX_FAMILY
    /* convert generic lookupCfg into device specific lookupCfg */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, enableLookup);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, dualLookup);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, pclIdL01);

    /* convert generic groupKeyTypes into device specific groupKeyTypes */
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.nonIpKey,
                                            lookupCfgPtr->groupKeyTypes.nonIpKey);
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.ipv4Key,
                                            lookupCfgPtr->groupKeyTypes.ipv4Key);
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.ipv6Key,
                                            lookupCfgPtr->groupKeyTypes.ipv6Key);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, udbKeyBitmapEnable);

    PRV_TGF_S2D_PCL_TCAM_SEGMENT_CONVERT_MAC(dxChLookupCfg.tcamSegmentMode,
                                             lookupCfgPtr->tcamSegmentMode);

    PRV_TGF_S2D_PCL_TCAM_SEGMENT_CONVERT_MAC(dxChLookupCfg.exactMatchLookupSubProfileId,
                                             lookupCfgPtr->exactMatchLookupSubProfileId);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if( CPSS_INTERFACE_PORT_E == interfaceInfo.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH
            rc = prvUtfHwDeviceNumberGet((GT_U8)interfaceInfo.devPort.hwDevNum,
                                         &interfaceInfo.devPort.hwDevNum);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                    "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclCfgTblSet(devNum, &interfaceInfo, direction, lookupNum, &dxChLookupCfg);
        }
        else
        {
            rc = cpssDxChPclPortGroupCfgTblSet(devNum, currPortGroupsBmp, &interfaceInfo, direction, lookupNum, &dxChLookupCfg);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupCfgTblSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclCfgTblGet function
* @endinternal
*
* @brief   Get PCL Configuration table entry's lookup configuration for interface.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - (pointer to) interface data
* @param[in] direction                - Policy direction
* @param[in] lookupNum                - Lookup number
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclCfgTblGet
(
    IN  GT_U8                          devNum,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT         direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    OUT PRV_TGF_PCL_LOOKUP_CFG_STC     *lookupCfgPtr
)
{
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    dxChLookupCfg = {0};
#endif /* CHX_FAMILY */

    cpssOsMemSet(lookupCfgPtr, 0, sizeof(PRV_TGF_PCL_LOOKUP_CFG_STC));

#ifdef CHX_FAMILY
    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChPclCfgTblGet(
            devNum, interfaceInfoPtr, direction, lookupNum, &dxChLookupCfg);
    }
    else
    {
        rc = cpssDxChPclPortGroupCfgTblGet(
            devNum, currPortGroupsBmp, interfaceInfoPtr, direction, lookupNum, &dxChLookupCfg);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupCfgTblGet FAILED, rc = [%d]", rc);

        rc1 = rc;
    }

    /* convert generic lookupCfg into device specific lookupCfg */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, enableLookup);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, dualLookup);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, pclIdL01);

    /* convert generic groupKeyTypes into device specific groupKeyTypes */
    PRV_TGF_D2S_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.nonIpKey,
                                            lookupCfgPtr->groupKeyTypes.nonIpKey);
    PRV_TGF_D2S_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.ipv4Key,
                                            lookupCfgPtr->groupKeyTypes.ipv4Key);
    PRV_TGF_D2S_PCL_RULE_FORMAT_CONVERT_MAC(dxChLookupCfg.groupKeyTypes.ipv6Key,
                                            lookupCfgPtr->groupKeyTypes.ipv6Key);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChLookupCfg, lookupCfgPtr, udbKeyBitmapEnable);

    PRV_TGF_D2S_PCL_TCAM_SEGMENT_CONVERT_MAC(dxChLookupCfg.tcamSegmentMode,
                                             lookupCfgPtr->tcamSegmentMode);

    PRV_TGF_D2S_PCL_TCAM_SEGMENT_CONVERT_MAC(dxChLookupCfg.exactMatchLookupSubProfileId,
                                             lookupCfgPtr->exactMatchLookupSubProfileId);

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclRuleWithOptionsSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleWithOptionsSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN GT_U32                             ruleIndex,
    IN PRV_TGF_PCL_RULE_OPTION_ENT        ruleOptionsBmp,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC            *actionPtr
)
{
    GT_U8       devNum  = 0;
    GT_U32      devRuleIndex;   /* Per device rule index */
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  dxChRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       dxChMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       dxChPattern;
    CPSS_DXCH_PCL_ACTION_STC            dxChAction;
    CPSS_DXCH_PCL_RULE_OPTION_ENT       dxChRuleOptionsBmp;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        tcamRuleSize;

    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChAction, 0, sizeof(dxChAction));

    dxChRuleOptionsBmp = 0;
    if (ruleOptionsBmp & PRV_TGF_PCL_RULE_OPTION_WRITE_INVALID_E)
    {
        dxChRuleOptionsBmp |= CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E;
    }

    if(actionPtr->pktCmd == CPSS_PACKET_CMD_DROP_HARD_E ||
       actionPtr->pktCmd == CPSS_PACKET_CMD_DROP_SOFT_E)
    {
        /* set CPU code -- needed by SIP5 for soft/hard DROP as well as for trap/mirror to cpu */
        actionPtr->mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    }


    /* convert generic ruleFormat into device specific ruleFormat */
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChRuleFormat, ruleFormat);

    /* convert generic Pcl key into device specific Pcl key */
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngStdNotIp(&(patternPtr->ruleStdNotIp),
                                                       &(dxChPattern.ruleStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdNotIp FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdNotIp(&(maskPtr->ruleStdNotIp),
                                                       &(dxChMask.ruleStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdNotIp FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngStdIpL2Qos(&(patternPtr->ruleStdIpL2Qos),
                                                         &(dxChPattern.ruleStdIpL2Qos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpL2Qos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdIpL2Qos(&(maskPtr->ruleStdIpL2Qos),
                                                         &(dxChMask.ruleStdIpL2Qos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpL2Qos FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngStdIpv4L4(&(patternPtr->ruleStdIpv4L4),
                                                        &(dxChPattern.ruleStdIpv4L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpv4L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdIpv4L4(&(maskPtr->ruleStdIpv4L4),
                                                        &(dxChMask.ruleStdIpv4L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpv4L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngStdIpv6Dip(&(patternPtr->ruleStdIpv6Dip),
                                                         &(dxChPattern.ruleStdIpv6Dip));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpv6Dip FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdIpv6Dip(&(maskPtr->ruleStdIpv6Dip),
                                                         &(dxChMask.ruleStdIpv6Dip));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdIpv6Dip FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngStdUdb(&(patternPtr->ruleStdUdb),
                                                     &(dxChPattern.ruleIngrStdUdb));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdUdb FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdUdb(&(maskPtr->ruleStdUdb),
                                                     &(dxChMask.ruleIngrStdUdb));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdUdb FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngExtNotIpv6(&(patternPtr->ruleExtNotIpv6),
                                                         &(dxChPattern.ruleExtNotIpv6));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngExtNotIpv6 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngExtNotIpv6(&(maskPtr->ruleExtNotIpv6),
                                                         &(dxChMask.ruleExtNotIpv6));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngExtNotIpv6 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv6L2(&(patternPtr->ruleExtIpv6L2),
                                                            &(dxChPattern.ruleExtIpv6L2));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv6L2 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv6L2(&(maskPtr->ruleExtIpv6L2),
                                                            &(dxChMask.ruleExtIpv6L2));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv6L2 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv6L4(&(patternPtr->ruleExtIpv6L4),
                                                            &(dxChPattern.ruleExtIpv6L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv6L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv6L4(&(maskPtr->ruleExtIpv6L4),
                                                            &(dxChMask.ruleExtIpv6L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv6L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleExtUdb(&(patternPtr->ruleExtUdb),
                                                         &(dxChPattern.ruleIngrExtUdb));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtUdb FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleExtUdb(&(maskPtr->ruleExtUdb),
                                                         &(dxChMask.ruleIngrExtUdb));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtUdb FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleStdNotIp(&(patternPtr->ruleEgrStdNotIp),
                                                           &(dxChPattern.ruleEgrStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdNotIp FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleStdNotIp(&(maskPtr->ruleEgrStdNotIp),
                                                           &(dxChMask.ruleEgrStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdNotIp FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos(&(patternPtr->ruleEgrStdIpL2Qos),
                                                             &(dxChPattern.ruleEgrStdIpL2Qos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos(&(maskPtr->ruleEgrStdIpL2Qos),
                                                             &(dxChMask.ruleEgrStdIpL2Qos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdIpL2Qos FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleStdIpv4L4(&(patternPtr->ruleEgrStdIpv4L4),
                                                             &(dxChPattern.ruleEgrStdIpv4L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdIpv4L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleStdIpv4L4(&(maskPtr->ruleEgrStdIpv4L4),
                                                             &(dxChMask.ruleEgrStdIpv4L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleStdIpv4L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleExtNotIpv6(&(patternPtr->ruleEgrExtNotIpv6),
                                                             &(dxChPattern.ruleEgrExtNotIpv6));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtNotIpv6 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleExtNotIpv6(&(maskPtr->ruleEgrExtNotIpv6),
                                                             &(dxChMask.ruleEgrExtNotIpv6));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtNotIpv6 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv6L2(&(patternPtr->ruleEgrExtIpv6L2),
                                                             &(dxChPattern.ruleEgrExtIpv6L2));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv6L2 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv6L2(&(maskPtr->ruleEgrExtIpv6L2),
                                                             &(dxChMask.ruleEgrExtIpv6L2));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv6L2 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv6L4(&(patternPtr->ruleEgrExtIpv6L4),
                                                             &(dxChPattern.ruleEgrExtIpv6L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv6L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv6L4(&(maskPtr->ruleEgrExtIpv6L4),
                                                             &(dxChMask.ruleEgrExtIpv6L4));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv6L4 FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos(&(patternPtr->ruleStdIpv4RoutedAclQos),
                                                             &(dxChPattern.ruleStdIpv4RoutedAclQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos(&(maskPtr->ruleStdIpv4RoutedAclQos),
                                                             &(dxChMask.ruleStdIpv4RoutedAclQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleStdIpv4RoutedAclQos FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos(&(patternPtr->ruleExtIpv4PortVlanQos),
                                                             &(dxChPattern.ruleExtIpv4PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos(&(maskPtr->ruleExtIpv4PortVlanQos),
                                                             &(dxChMask.ruleExtIpv4PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleExtIpv4PortVlanQos FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos(&(patternPtr->ruleUltraIpv6PortVlanQos),
                                                             &(dxChPattern.ruleUltraIpv6PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos(&(maskPtr->ruleUltraIpv6PortVlanQos),
                                                             &(dxChMask.ruleUltraIpv6PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos(&(patternPtr->ruleUltraIpv6RoutedAclQos),
                                                             &(dxChPattern.ruleUltraIpv6RoutedAclQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos(&(maskPtr->ruleUltraIpv6RoutedAclQos),
                                                             &(dxChMask.ruleUltraIpv6RoutedAclQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6RoutedAclQos FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl(&(patternPtr->ruleEgrExtIpv4RaclVacl),
                                                             &(dxChPattern.ruleEgrExtIpv4RaclVacl));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl(&(maskPtr->ruleEgrExtIpv4RaclVacl),
                                                             &(dxChMask.ruleEgrExtIpv4RaclVacl));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleExtIpv4RaclVacl FAILED, rc = [%d]", rc);

                return rc;
            }

            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
            /* convert into device specific Pattern */
            rc = prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl(&(patternPtr->ruleEgrUltraIpv6RaclVacl),
                                                             &(dxChPattern.ruleEgrUltraIpv6RaclVacl));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl(&(maskPtr->ruleEgrUltraIpv6RaclVacl),
                                                             &(dxChMask.ruleEgrUltraIpv6RaclVacl));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChEgrRuleUltraIpv6RaclVacl FAILED, rc = [%d]", rc);

                return rc;
            }

            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E:

            /* convert into device specific Pattern */
            prvTgfConvertGenericToDxChRuleIngressUdbOnly(
                &(patternPtr->ruleIngrUdbOnly), &(dxChPattern.ruleIngrUdbOnly));

            /* convert into device specific Mask */
            prvTgfConvertGenericToDxChRuleIngressUdbOnly(
                &(maskPtr->ruleIngrUdbOnly), &(dxChMask.ruleIngrUdbOnly));

            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E:

            /* convert into device specific Pattern */
            prvTgfConvertGenericToDxChRuleEgressUdbOnly(
                &(patternPtr->ruleEgrUdbOnly), &(dxChPattern.ruleEgrUdbOnly));

            /* convert into device specific Mask */
            prvTgfConvertGenericToDxChRuleEgressUdbOnly(
                &(maskPtr->ruleEgrUdbOnly), &(dxChMask.ruleEgrUdbOnly));

            break;
        default: return GT_BAD_PARAM;
    }

    /* convert rule Action into device specific format */
    rc = prvTgfConvertGenericToDxChRuleAction(actionPtr, &dxChAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* convert rule's format to rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize,dxChRuleFormat);

        /* prepare rule index */
        devRuleIndex = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,ruleIndex,tcamRuleSize);

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclRuleSet(
                devNum, dxChRuleFormat, devRuleIndex, dxChRuleOptionsBmp,
                &dxChMask, &dxChPattern, &dxChAction);
        }
        else
        {
            orig_currPortGroupsBmp = currPortGroupsBmp;
            TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

            rc = cpssDxChPclPortGroupRuleSet(
                devNum, currPortGroupsBmp, dxChRuleFormat,
                devRuleIndex, dxChRuleOptionsBmp,
                &dxChMask, &dxChPattern, &dxChAction);

            /*restore value*/
            currPortGroupsBmp = orig_currPortGroupsBmp;
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    rc1 = GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN GT_U32                             ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC            *actionPtr
)
{
    return prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
        maskPtr, patternPtr, actionPtr);
}

/**
* @internal prvTgfPclRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - on the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*
* @note NONE
*
*/
GT_STATUS prvTgfPclRuleActionUpdate
(
    IN CPSS_PCL_RULE_SIZE_ENT         ruleSize,
    IN GT_U32                         ruleIndex,
    IN PRV_TGF_PCL_ACTION_STC        *actionPtr
)
{
    GT_U8       devNum  = 0;
    GT_U32      devRuleIndex;   /* Per device rule index */
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_ACTION_STC            dxChAction;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        tcamRuleSize;

    /* convert rule Action into device specific format */
    rc = prvTgfConvertGenericToDxChRuleAction(actionPtr, &dxChAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* convert pcl rule's size to tcam rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

        /* prepare rule index */
        devRuleIndex = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,ruleIndex,tcamRuleSize);

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclRuleActionUpdate(devNum, ruleSize, devRuleIndex, &dxChAction);
        }
        else
        {
            orig_currPortGroupsBmp = currPortGroupsBmp;
            TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

            rc = cpssDxChPclPortGroupRuleActionUpdate(devNum, currPortGroupsBmp, ruleSize, devRuleIndex, &dxChAction);

            /*restore value*/
            currPortGroupsBmp = orig_currPortGroupsBmp;
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupRuleActionUpdate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on TCAM found rule of different size
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleValidStatusSet
(
    IN CPSS_PCL_RULE_SIZE_ENT         ruleSize,
    IN GT_U32                         ruleIndex,
    IN GT_BOOL                        valid
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum = 0;
    GT_U32      devRuleIndex;   /* Per device rule index */
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        tcamRuleSize;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* convert pcl rule's size to tcam rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

        /* prepare rule index */
        devRuleIndex = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,ruleIndex,tcamRuleSize);

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclRuleValidStatusSet(devNum, ruleSize, devRuleIndex, valid);
        }
        else
        {
            orig_currPortGroupsBmp = currPortGroupsBmp;
            TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

            rc = cpssDxChPclPortGroupRuleValidStatusSet(devNum, currPortGroupsBmp, ruleSize, devRuleIndex, valid);

            /*restore value*/
            currPortGroupsBmp = orig_currPortGroupsBmp;
        }

        if (GT_OK != rc)
        {
            if(valid == 0 && rc == GT_BAD_STATE)
            {
                /* when entry is not initialized yet , this API will return
                    GT_BAD_STATE , but is seems logically to return GT_OK for the
                    'Invalid action' '*/
            }
            else
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupRuleValidStatusSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate should be used to invalidate
*         Rule in old position
* @param[in] ruleSize                 - size of Rule.
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
GT_STATUS prvTgfPclRuleCopy
(
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum = 0;
    GT_U32      devRuleSrcIndex;   /* Per device rule source index */
    GT_U32      devRuleDstIndex;   /* Per device rule destination index */
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        tcamRuleSize;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* convert pcl rule's size to tcam rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

        /* prepare rule index */
        devRuleSrcIndex = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,ruleSrcIndex,tcamRuleSize);
        devRuleDstIndex = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,ruleDstIndex,tcamRuleSize);

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclRuleCopy(devNum, ruleSize, devRuleSrcIndex, devRuleDstIndex);
        }
        else
        {
            orig_currPortGroupsBmp = currPortGroupsBmp;
            TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

            rc = cpssDxChPclPortGroupRuleCopy(devNum, currPortGroupsBmp, ruleSize, devRuleSrcIndex, devRuleDstIndex);

            /*restore value*/
            currPortGroupsBmp = orig_currPortGroupsBmp;
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupRuleRuleCopy FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/* array of UDB (cfg, member) */
/* Should be synchronized with PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT */
static GT_U8    prvTgfUdbIndexArr[][2] =
{
     {3, 15}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E */
     {2, 18}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E */
     {3, 20}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E */
     {6,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E */
     {6,  6}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E */
     {3, 12}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E */
     {16, 0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E */
     {16, 0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_80B_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E */
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E*/
     {0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E*/
     {0,  0}  /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E*/
};
/* extended array of UDB (cfg, member) */
/* Should be synchronized with PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT */
static GT_U8    prvTgfUdbIndexExtArr[][6] =
{
     {3, 15, 4, 23, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E */
     {2, 18, 4, 27, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E */
     {3, 20, 4, 31, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E */
     {3, 47, 1,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E */
     {6,  0, 8, 39, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E */
     {6,  6, 3, 47, 5,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E */
     {3, 12, 8, 15, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E */
     {4,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E */
     {4,  4, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E */
     {4,  8, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E */
     {8, 12, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E */
     {8, 20, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E */
     {8, 28, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E */
     {16, 0, 4, 35, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E */
     {16, 0, 8, 23, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E */
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_80B_E */
     {1,  5, 4, 41, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E */
     {12, 5, 8, 31, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E*/
     {12, 0, 1, 12, 2, 39}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E */
     {12, 0, 6, 17, 2, 45}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E*/
     {14, 36, 1, 0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E*/
     {4,  1, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E*/
     {0,  0, 0,  0, 0,  0}, /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E*/
     {0,  0, 0,  0, 0,  0}  /* PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E*/
};
/**
* @internal prvTgfPclUdbIndexConvert function
* @endinternal
*
* @brief   The function converts the UDB relative index (in rule format)
*         to UDB absolute index in UDB configuration table entry
*         Conversion needed for XCAT and Puma devices, not needed for CH1-3
* @param[in] ruleFormat               - rule format
*                                      packetType - packet Type
*                                      udbIndex   - relative index of UDB in the rule
*
* @param[out] udbAbsIndexPtr           - (pointer to) UDB absolute index in
*                                      UDB configuration table entry
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
*/
GT_STATUS prvTgfPclUdbIndexConvert
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  GT_U32                               udbRelIndex,
    OUT GT_U32                               *udbAbsIndexPtr
)
{
    /* device found */
    if (udbRelIndex >= prvTgfUdbIndexArr[ruleFormat][0])
    {
        return GT_OUT_OF_RANGE;
    }

    *udbAbsIndexPtr = prvTgfUdbIndexArr[ruleFormat][1] + udbRelIndex;
    return GT_OK;
}

/**
* @internal prvTgfPclUdbIndexExtConvert function
* @endinternal
*
* @brief   The function converts the UDB relative index (in rule format)
*         to UDB absolute index in UDB configuration table entry
*         Conversion needed for XCAT and Puma devices, not needed for CH1-3
* @param[in] ruleFormat               - rule format
*                                      packetType - packet Type
*                                      udbIndex   - relative index of UDB in the rule
*
* @param[out] udbAbsIndexPtr           - (pointer to) UDB absolute index in
*                                      UDB configuration table entry
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
*/
GT_STATUS prvTgfPclUdbIndexExtConvert
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  GT_U32                               udbRelIndex,
    OUT GT_U32                               *udbAbsIndexPtr
)
{
    if (udbRelIndex < prvTgfUdbIndexExtArr[ruleFormat][0])
    {
        *udbAbsIndexPtr = prvTgfUdbIndexExtArr[ruleFormat][1] + udbRelIndex;
        return GT_OK;
    }
    else
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(prvTgfDevNum) != GT_FALSE)
    {
        if ((udbRelIndex - prvTgfUdbIndexExtArr[ruleFormat][0]) <
                                            prvTgfUdbIndexExtArr[ruleFormat][2])
        {
            *udbAbsIndexPtr = prvTgfUdbIndexExtArr[ruleFormat][3] +
                            (udbRelIndex - prvTgfUdbIndexExtArr[ruleFormat][0]);
            return GT_OK;
        }
        else if ((udbRelIndex - prvTgfUdbIndexExtArr[ruleFormat][0] - prvTgfUdbIndexExtArr[ruleFormat][2]) <
                                            prvTgfUdbIndexExtArr[ruleFormat][4])
        {
            *udbAbsIndexPtr = prvTgfUdbIndexExtArr[ruleFormat][5] +
                            (udbRelIndex - prvTgfUdbIndexExtArr[ruleFormat][0] -
                             prvTgfUdbIndexExtArr[ruleFormat][2]);
            return GT_OK;
        }
    }
    return GT_OUT_OF_RANGE;
}
/**
* @internal prvTgfPclUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Relevant for DxChXCat and above devices
* @param[in] direction                -  (APPLICABLE DEVICES Lion2)
* @param[in] udbIndex                 - index of User Defined Byte to configure
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The offset of the user-defined byte
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclUserDefinedByteSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType,
    IN GT_U8                                offset
)
{
    GT_U8       devNum = 0;
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  dxChRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       dxChPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT       dxChOffsetType = CPSS_DXCH_PCL_OFFSET_L2_E;

    /* for backward compatibility if this function called with packet type _IPV6_E  */
    /* and EACRCH sevice (not relevant for _IPV6_E) it must recursve call itself    */
    /* with _IPV6_TCP_E, _IPV6_UDP_E, _IPV6_OTHER_E to reach the same functionality */

    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        && (packetType == PRV_TGF_PCL_PACKET_TYPE_IPV6_E))
    {
        GT_U32                             _ii;
        GT_STATUS                          _rc;
        static PRV_TGF_PCL_PACKET_TYPE_ENT _pktType[3] =
        {
            PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,
            PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
            PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E
        };

        for (_ii = 0; (_ii < 3); _ii++)
        {
            _rc = prvTgfPclUserDefinedByteSet(
                ruleFormat,  _pktType[_ii], direction, udbIndex, offsetType, offset);
            if (_rc != GT_OK)
            {
                return _rc;
            }
        }

        return GT_OK;
    }

    /* convert generic ruleFormat into device specific ruleFormat */
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChRuleFormat, ruleFormat);

    /* convert generic into device specific packetType */
    PRV_TGF_S2D_PCL_UDB_PACKET_TYPE_CONVERT_MAC(dxChPacketType, packetType);

    /* convert generic into device specific offsetType */
    PRV_TGF_S2D_PCL_UDB_OFFSET_TYPE_CONVERT_MAC(dxChOffsetType, offsetType);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclUserDefinedByteSet(
                devNum, dxChRuleFormat, dxChPacketType, direction,
                udbIndex, dxChOffsetType, offset);
        }
        else
        {
            rc = cpssDxChPclPortGroupUserDefinedByteSet(
                devNum, currPortGroupsBmp,
                dxChRuleFormat, dxChPacketType, direction,
                udbIndex, dxChOffsetType, offset);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortGroupUserDefinedByteSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclUserDefinedBytesSelectSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
* @param[in] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclUserDefinedBytesSelectSet
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  PRV_TGF_PCL_UDB_SELECT_STC           *udbSelectPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                                  devNum = 0;
    GT_STATUS                              rc     = GT_OK;
    GT_STATUS                              rc1    = GT_OK;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT     dxChRuleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT          dxChPacketType;
    CPSS_DXCH_PCL_UDB_SELECT_STC           dxChUdbSelect;


    /* convert generic ruleFormat into device specific ruleFormat */
    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(dxChRuleFormat, ruleFormat);

    /* convert generic into device specific packetType */
    PRV_TGF_S2D_PCL_UDB_PACKET_TYPE_CONVERT_MAC(dxChPacketType, packetType);

    /* convert generic into device specific udbSelect */
    prvTgfConvertGenericToDxChUdbSelect(
        udbSelectPtr, &dxChUdbSelect);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc,
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
         UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E));

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclUserDefinedBytesSelectSet(
            devNum, dxChRuleFormat, dxChPacketType, lookupNum, &dxChUdbSelect);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPclUserDefinedBytesSelectSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclOverrideUserDefinedBytesSet function
* @endinternal
*
* @brief   The function sets overriding of User Defined Bytes by predefined key fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclOverrideUserDefinedBytesSet
(
    IN  PRV_TGF_PCL_OVERRIDE_UDB_STC *udbOverridePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum = 0;
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
    GT_U32      i;

    static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat[] =
    {
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
    };

    static CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  overrideType[] =
    {
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E
    };

    static GT_U32 cfgAmount = sizeof(ruleFormat) / sizeof(ruleFormat[0]);

    GT_BOOL enable[sizeof(ruleFormat) / sizeof(ruleFormat[0])];

    enable[0]  =  udbOverridePtr->vrfIdLsbEnableStdNotIp;
    enable[1]  =  udbOverridePtr->vrfIdMsbEnableStdNotIp;
    enable[2]  =  udbOverridePtr->vrfIdLsbEnableStdIpL2Qos;
    enable[3]  =  udbOverridePtr->vrfIdMsbEnableStdIpL2Qos;
    enable[4]  =  udbOverridePtr->vrfIdLsbEnableStdIpv4L4;
    enable[5]  =  udbOverridePtr->vrfIdMsbEnableStdIpv4L4;
    enable[6]  =  udbOverridePtr->vrfIdLsbEnableStdUdb;
    enable[7]  =  udbOverridePtr->vrfIdMsbEnableStdUdb;
    enable[8]  =  udbOverridePtr->vrfIdLsbEnableExtNotIpv6;
    enable[9]  =  udbOverridePtr->vrfIdMsbEnableExtNotIpv6;
    enable[10] =  udbOverridePtr->vrfIdLsbEnableExtIpv6L2;
    enable[11] =  udbOverridePtr->vrfIdMsbEnableExtIpv6L2;
    enable[12] =  udbOverridePtr->vrfIdLsbEnableExtIpv6L4;
    enable[13] =  udbOverridePtr->vrfIdMsbEnableExtIpv6L4;
    enable[14] =  udbOverridePtr->vrfIdLsbEnableExtUdb;
    enable[15] =  udbOverridePtr->vrfIdMsbEnableExtUdb;
    enable[16] =  udbOverridePtr->qosProfileEnableStdUdb;
    enable[17] =  udbOverridePtr->qosProfileEnableExtUdb;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (i = 0; (i < cfgAmount); i++)
        {
            /* call device specific API */
            if(usePortGroupsBmp == GT_FALSE)
            {
                rc = cpssDxChPclOverrideUserDefinedBytesEnableSet(
                    devNum, ruleFormat[i], overrideType[i], enable[i]);
            }
            else
            {
                rc = cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet(
                    devNum, currPortGroupsBmp,
                    ruleFormat[i], overrideType[i], enable[i]);
            }

            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                    "[TGF]: cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclInvalidUdbCmdSet function
* @endinternal
*
* @brief   Set the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header depth
* @param[in] udbErrorCmd              - command applied to a packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclInvalidUdbCmdSet
(
    IN PRV_TGF_UDB_ERROR_CMD_ENT    udbErrorCmd
)
{
    GT_U8       devNum = 0;
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_UDB_ERROR_CMD_ENT  dxChUdbErrorCmd = CPSS_DXCH_UDB_ERROR_CMD_LOOKUP_E;

    /* convert generic into device specific udbErrorCmd */
    switch (udbErrorCmd)
    {
        case PRV_TGF_UDB_ERROR_CMD_LOOKUP_E:
            dxChUdbErrorCmd = CPSS_DXCH_UDB_ERROR_CMD_LOOKUP_E;
            break;

        case PRV_TGF_UDB_ERROR_CMD_TRAP_TO_CPU_E:
            dxChUdbErrorCmd = CPSS_DXCH_UDB_ERROR_CMD_TRAP_TO_CPU_E;
            break;

        case PRV_TGF_UDB_ERROR_CMD_DROP_HARD_E:
            dxChUdbErrorCmd = CPSS_DXCH_UDB_ERROR_CMD_DROP_HARD_E;
            break;

        case PRV_TGF_UDB_ERROR_CMD_DROP_SOFT_E:
            dxChUdbErrorCmd = CPSS_DXCH_UDB_ERROR_CMD_DROP_SOFT_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclInvalidUdbCmdSet(devNum, dxChUdbErrorCmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclInvalidUdbCmdSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/

/* variable for save and restore the enable/disable state */
/* of EPCL for packets TO_CPU on CPU port                 */
static struct
{
    GT_BOOL                    initialized;
    GT_BOOL                    enable;
    PRV_TGF_PCL_LOOKUP_CFG_STC cfgTabEntry;
} prvTgfPclEgressPclCpuPortCfg = {GT_FALSE, GT_FALSE, {0}};


/**
* @internal prvTgfPclDefPortInitExt2 function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInitExt2
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_BOOL                   isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    if (GT_FALSE == isDeviceInited)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclInit FAILED, rc = [%d]", rc);

            return rc;
        }

        /* do not init next time */
        isDeviceInited = GT_TRUE;
    }

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        /* enables ingress policy per devices */
        rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclIngressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }

        /* enables ingress policy */
        rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclPortIngressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    else
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEgressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }

        if (prvTgfPclEgressPclCpuPortCfg.initialized == GT_FALSE)
        {
            prvTgfPclEgressPclCpuPortCfg.initialized = GT_TRUE;
            rc = prvTgfPclEgressPclPacketTypesGet(
                prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                &(prvTgfPclEgressPclCpuPortCfg.enable));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEgressPclPacketTypesGet FAILED, rc = [%d]", rc);

                return rc;
            }

            interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
            interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
            interfaceInfo.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

            rc = prvTgfPclCfgTblGet(
                prvTgfDevNum, &interfaceInfo,
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &(prvTgfPclEgressPclCpuPortCfg.cfgTabEntry));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclCfgTblGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }

        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, portNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEgressPclPacketTypesSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* configure access accessMode */
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclPortLookupCfgTabAccessModeSet FAILED, rc = [%d]", rc);

        return rc;
    }


    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = portNum;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, direction, lookupNum, lookupCfgPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclCfgTblSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}

/**
* @internal prvTgfPclDefPortInitExt1 function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] pclId                    - pcl Id
* @param[in] nonIpKey                 - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv4Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv6Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInitExt1
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpKey,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv4Key,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv6Key
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = pclId;
    lookupCfg.groupKeyTypes.nonIpKey = nonIpKey;
    lookupCfg.groupKeyTypes.ipv4Key  = ipv4Key;
    lookupCfg.groupKeyTypes.ipv6Key  = ipv6Key;

    return prvTgfPclDefPortInitExt2(
        portNum, direction, lookupNum, &lookupCfg);
}

/**
* @internal prvTgfPclDefPortInit function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] nonIpKey                 - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv4Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv6Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInit
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpKey,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv4Key,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv6Key
)
{
    return prvTgfPclDefPortInitExt1(
        portNum, direction, lookupNum,
        PRV_TGF_PCL_DEFAULT_ID_MAC(direction, lookupNum, portNum),
        nonIpKey, ipv4Key, ipv6Key);
}

/**
* @internal prvTgfPclUdeEtherTypeSet function
* @endinternal
*
* @brief   This function sets UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclUdeEtherTypeSet
(
    IN  GT_U32          index,
    IN  GT_U32          ethType
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclUdeEtherTypeSet(devNum, index, ethType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclUdeEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclOverrideUserDefinedBytesByTrunkHashSet function
* @endinternal
*
* @brief   The function sets overriding of User Defined Bytes
*         by packets Trunk Hash value.
* @param[in] udbOverTrunkHashPtr      - (pointer to) UDB override trunk hash structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclOverrideUserDefinedBytesByTrunkHashSet
(
    IN  PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC *udbOverTrunkHashPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum = 0;
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;
    GT_U32      i;

    static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat[] =
    {
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
    };

    static CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  overrideType[] =
    {
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E
    };

    static GT_U32 cfgAmount = sizeof(ruleFormat) / sizeof(ruleFormat[0]);

    GT_BOOL enable[sizeof(ruleFormat) / sizeof(ruleFormat[0])];

    enable[0] = udbOverTrunkHashPtr->trunkHashEnableStdNotIp;
    enable[1] = udbOverTrunkHashPtr->trunkHashEnableStdIpv4L4;
    enable[2] = udbOverTrunkHashPtr->trunkHashEnableStdUdb;
    enable[3] = udbOverTrunkHashPtr->trunkHashEnableExtNotIpv6;
    enable[4] = udbOverTrunkHashPtr->trunkHashEnableExtIpv6L2;
    enable[5] = udbOverTrunkHashPtr->trunkHashEnableExtIpv6L4;
    enable[6] = udbOverTrunkHashPtr->trunkHashEnableExtUdb;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (i = 0; (i < cfgAmount); i++)
        {
            /* call device specific API */
            if(usePortGroupsBmp == GT_FALSE)
            {
                rc = cpssDxChPclOverrideUserDefinedBytesEnableSet(
                    devNum, ruleFormat[i], overrideType[i], enable[i]);
            }
            else
            {
                rc = cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet(
                    devNum, currPortGroupsBmp,
                    ruleFormat[i], overrideType[i], enable[i]);
            }

            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                    "[TGF]: cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressKeyFieldsVidUpModeSet
(
    IN  PRV_TGF_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT cpssVidUpMode;

    switch (vidUpMode)
    {
        case PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E:
            cpssVidUpMode = CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E;
            break;
        case PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E:
        cpssVidUpMode = CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclEgressKeyFieldsVidUpModeSet(
            devNum, cpssVidUpMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPclEgressKeyFieldsVidUpModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclLookupCfgPortListEnableSet
(
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclLookupCfgPortListEnableSet(
            devNum, direction, lookupNum, subLookupNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPclLookupCfgPortListEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChPclLookupCfgPortListEnableGet(
            devNum, direction, lookupNum, subLookupNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPclLookupCfgPortListEnableGet FAILED, rc = [%d]", rc);
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal tgfPclOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS tgfPclOverrideUserDefinedBytesEnableSet
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  PRV_TGF_PCL_UDB_OVERRIDE_TYPE_ENT    udbOverrideType,
    IN  GT_BOOL                              enable
)
{
#ifdef CHX_FAMILY
    GT_U8                                devNum = 0;
    GT_STATUS                            rc     = GT_OK;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   cpssRuleFormat;
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  cpssUdbOverrideType;

    PRV_TGF_S2D_PCL_RULE_FORMAT_CONVERT_MAC(
        cpssRuleFormat, ruleFormat);
    PRV_TGF_S2D_PCL_UDB_OVERRIDE_TYPE_CONVERT_MAC(
        cpssUdbOverrideType, udbOverrideType);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxChPclOverrideUserDefinedBytesEnableSet(
                devNum, cpssRuleFormat, cpssUdbOverrideType, enable);
        }
        else
        {
            rc = cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet(
                devNum, currPortGroupsBmp,
                cpssRuleFormat, cpssUdbOverrideType, enable);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet FAILED, rc = [%d]", rc);
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclRestore function
* @endinternal
*
* @brief   Function clears ingress pcl settings.
*/
GT_VOID prvTgfPclRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    GT_U32                          portIndex;

    /* AUTODOC: remove PCL configurations */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     = prvTgfDevNum;
    lookupCfg.enableLookup           = GT_FALSE;  /* --> disable the PCL lookup */
    lookupCfg.pclId                  = 0;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;

    for(portIndex = 0 ; portIndex < prvTgfPortsNum ;portIndex++)
    {
        interfaceInfo.devPort.portNum    = prvTgfPortsArray[portIndex];

        for(lookupNum = CPSS_PCL_LOOKUP_0_E; lookupNum <= CPSS_PCL_LOOKUP_1_E; lookupNum ++)
        {
            if ((lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E) &&
                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass))
            {
                /* PCL stage is bypassed */
                continue;
            }
            rc = prvTgfPclCfgTblSet(
                &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, lookupNum, &lookupCfg);
            if (GT_OK != rc)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "[TGF]: prvTgfPclCfgTblSet FAILED: direction:%d, lookup:%d, port:%d",
                                             CPSS_PCL_DIRECTION_INGRESS_E,
                                             lookupNum,
                                             interfaceInfo.devPort.portNum);
            }
        }
    }
}

/**
* @internal prvTgfPclPortsRestore function
* @endinternal
*
* @brief   Function clears port related pcl settings.
*/
GT_VOID prvTgfPclPortsRestore
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum
)
{
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U32                          portIndex;

    /* AUTODOC: remove PCL configurations */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     = prvTgfDevNum;
    lookupCfg.enableLookup           = GT_FALSE;  /* --> disable the PCL lookup */
    lookupCfg.pclId                  = 0;
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/;
        lookupCfg.groupKeyTypes.ipv4Key  =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/;
        lookupCfg.groupKeyTypes.ipv6Key  =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;
    }
    else
    {
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/;
        lookupCfg.groupKeyTypes.ipv4Key  =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E /*ipv4Key*/;
        lookupCfg.groupKeyTypes.ipv6Key  =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;

        if (prvTgfPclEgressPclCpuPortCfg.initialized != GT_FALSE)
        {
            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                prvTgfPclEgressPclCpuPortCfg.enable);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

            interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
            interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
            interfaceInfo.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

            rc = prvTgfPclCfgTblSet(
                &interfaceInfo,
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &(prvTgfPclEgressPclCpuPortCfg.cfgTabEntry));
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclCfgTblSet");
        }
    }

    for(portIndex = 0; (portIndex < prvTgfPortsNum); portIndex++)
    {
        interfaceInfo.devPort.hwDevNum     = prvTgfDevsArray[portIndex];
        interfaceInfo.devPort.portNum    = prvTgfPortsArray[portIndex];

        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, lookupNum, &lookupCfg);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclCfgTblSet");

        if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            rc = prvTgfPclPortIngressPolicyEnable(
                prvTgfPortsArray[portIndex], GT_FALSE);
        }
        else
        {
            /*
            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex],
                PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E, GT_FALSE);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex],
                PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E, GT_FALSE);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex],
                PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, GT_FALSE);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
            */

            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex],
                PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

            rc = prvTgfPclEgressPclPacketTypesSet(
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex],
                PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
            PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
        }
    }

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */

}

/**
* @internal prvTgfPclPortsRestoreAll function
* @endinternal
*
* @brief   Function clears port related pcl settings - FOR ALL PORTS , EPCL,IPCL0,1,2
*/
GT_VOID prvTgfPclPortsRestoreAll
(
    GT_VOID
)
{
    prvTgfPclRestore();
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_0_E);
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_1_E);
    }
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E ,CPSS_PCL_LOOKUP_0_E);
}


/**
* @internal prvTgfPclPolicerCheck function
* @endinternal
*
* @brief   check if the device supports the policer pointed from the PCL/TTI
*
* @retval GT_TRUE                  - the device supports the policer pointed from the PCL/TTI
* @retval GT_FALSE                 - the device not supports the policer pointed from the PCL/TTI
*/
GT_BOOL prvTgfPclPolicerCheck(
    void
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
    }

    if(rc != GT_OK || routingMode == PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E)
    {
        return GT_FALSE;
    }

    return GT_TRUE;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_TRUE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (0-7)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclTcpUdpPortComparatorSet
(
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U32                            entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        if (usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxCh2PclTcpUdpPortComparatorSet(
                devNum,
                direction, l4Protocol, (GT_U8)entryIndex,
                l4PortType, compareOperator, value);
        }
        else
        {
            rc = cpssDxChPclPortGroupTcpUdpPortComparatorSet(
                devNum, currPortGroupsBmp,
                direction, l4Protocol, (GT_U8)entryIndex,
                l4PortType, compareOperator, value);
        }
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh2PclTcpUdpPortComparatorSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclL3L4ParsingOverMplsEnableSet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclL3L4ParsingOverMplsEnableSet
(
    IN  GT_BOOL                            enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E );

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclL3L4ParsingOverMplsEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPclL3L4ParsingOverMplsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclOamRdiSet function
* @endinternal
*
* @brief   This function sets OAM RDI configuration.
*
* @note   APPLICABLE DEVICES:      Lion 3 and above.
* @param[in] direction                - policy direction
* @param[in] profile                  - OAM RDI  index, range 0..1
* @param[in] rdiUdbValue              - RDI UDB value (range 0..0xFF)
* @param[in] rdiUdbMask               - RDI UDB mask  (range 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclOamRdiSet
(
    IN CPSS_PCL_DIRECTION_ENT       direction,
    IN  GT_U32                      profile,
    IN  GT_U32                      rdiUdbValue,
    IN  GT_U32                      rdiUdbMask
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclOamRdiMatchingSet(devNum, direction, profile, rdiUdbMask, rdiUdbValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclOamRdiMatchingSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !defined(CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(profile);
    TGF_PARAM_NOT_USED(rdiUdbValue);
    TGF_PARAM_NOT_USED(rdiUdbMask);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclOamChannelTypeProfileToOpcodeMappingSet function
* @endinternal
*
* @brief   Set Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2 and above.
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
* @param[in] opcode                   - OAM  (APPLICABLE RANGES: 0..255)
* @param[in] RBitAssignmentEnable     - replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclOamChannelTypeProfileToOpcodeMappingSet
(
    IN  GT_U32                    channelTypeProfile,
    IN  GT_U32                    opcode,
    IN  GT_BOOL                   RBitAssignmentEnable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet(
            devNum, channelTypeProfile, opcode, RBitAssignmentEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]:cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet  FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !defined(CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(channelTypeProfile);
    TGF_PARAM_NOT_USED(opcode);
    TGF_PARAM_NOT_USED(RBitAssignmentEnable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclLookup0ForRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup0 for routed packets
*
* @note   APPLICABLE DEVICES:      Lion and above.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclLookup0ForRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  GT_BOOL                            enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChPclLookup0ForRoutedPacketsEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPclLookup0ForRoutedPacketsEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* (!defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclTunnelTermForceVlanModeEnableSet function
* @endinternal
*
* @brief   The function enables/disables forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3 and Above.
* @param[in] enable                   - force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclTunnelTermForceVlanModeEnableSet
(
    IN  GT_BOOL                       enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]:cpssDxCh3PclTunnelTermForceVlanModeEnableSet  FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* (!defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet function
* @endinternal
*
* @brief   Sets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3 and Above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet
(
    IN  PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT     cfgTabAccMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT     dxChCfgTabAccMode;

    switch (cfgTabAccMode)
    {
        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E:
            dxChCfgTabAccMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;
            break;
        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E:
            dxChCfgTabAccMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E;
            break;
        case PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E:
            dxChCfgTabAccMode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(
            devNum, dxChCfgTabAccMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]:cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet  FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cfgTabAccMode);
    return GT_BAD_STATE;
#endif /* (!defined CHX_FAMILY) */
}

/**
* @internal prvTgfPclSourceIdMaskSet function
* @endinternal
*
* @brief   Set mask so only certain bits in the source ID will be modified due to PCL
*         action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lookupNum                - lookup number
* @param[in] mask                     - Source ID  (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclSourceIdMaskSet
(
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    IN  GT_U32                      mask
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclSourceIdMaskSet(devNum, lookupNum, mask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclSourceIdMaskSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(lookupNum);
    TGF_PARAM_NOT_USED(mask);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclSourceIdMaskGet function
* @endinternal
*
* @brief   Get mask used for source ID modify due to PCL action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
*
* @param[out] maskPtr                  - (pointer to) Source ID mask (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclSourceIdMaskGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    OUT GT_U32                      *maskPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;

    rc = cpssDxChPclSourceIdMaskGet(devNum, lookupNum, maskPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclSourceIdMaskGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(lookupNum);
    TGF_PARAM_NOT_USED(maskPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressSourcePortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portSelectionMode        - Source port selection mode for DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressSourcePortSelectionModeSet
(
    IN  PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT   portSelectionMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT dxChPortSelectionMode;

    switch (portSelectionMode)
    {
        case PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E:
            dxChPortSelectionMode = CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E;
            break;
        case PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E:
            dxChPortSelectionMode = CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclEgressSourcePortSelectionModeSet(devNum, dxChPortSelectionMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressSourcePortSelectionModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(portSelectionMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressSourcePortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Source port selection mode for
*                                      DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressSourcePortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT   *portSelectionModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT dxChPortSelectionMode;

    rc = cpssDxChPclEgressSourcePortSelectionModeGet(devNum, &dxChPortSelectionMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressSourcePortSelectionModeGet FAILED, rc = [%d]", rc);

    }

    switch (dxChPortSelectionMode)
    {
        case CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E:
            *portSelectionModePtr = PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E;
            break;
        case CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E:
            *portSelectionModePtr = PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
#endif

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portSelectionModePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressTargetPortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portSelectionMode        - Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressTargetPortSelectionModeSet
(
    IN  PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT dxChPortSelectionMode;

    switch (portSelectionMode)
    {
        case PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E:
            dxChPortSelectionMode = CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E;
            break;
        case PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E:
            dxChPortSelectionMode = CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclEgressTargetPortSelectionModeSet(devNum, dxChPortSelectionMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclTargetPortSelectionModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portSelectionMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressTargetPortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressTargetPortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT   *portSelectionModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT dxChPortSelectionMode;

    rc = cpssDxChPclEgressTargetPortSelectionModeGet(devNum, &dxChPortSelectionMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressTargetPortSelectionModeGet FAILED, rc = [%d]", rc);

    }

    switch (dxChPortSelectionMode)
    {
        case CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E:
            *portSelectionModePtr = PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E;
            break;
        case CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E:
            *portSelectionModePtr = PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
#endif

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portSelectionModePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEportAccessModeInit function
* @endinternal
*
* @brief   Initialize ePort access on ePort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
* @param[in] ePortNum                 - ePort number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*
* @note Enable PCL on source ePort
*       Enable access to IPCL/EPCL configuration table on ePort-based binding mode
*
*/
GT_STATUS prvTgfPclEportAccessModeInit
(
    IN GT_PORT_NUM                      portNum,
    IN GT_PORT_NUM                      ePortNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;

    /* AUTODOC: Configure access mode on ePort */
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E;

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclPortLookupCfgTabAccessModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* AUTODOC: Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;
    if(ePortNum > 0xFFF)
    {
        return GT_BAD_PARAM;
    }
    interfaceInfo.index = ePortNum;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, direction, lookupNum, lookupCfgPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclCfgTblSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}

/**
* @internal prvTgfPclDefEportInitExt function
* @endinternal
*
* @brief   Initialize PCL Engine on ePort-based binding mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ePortNum                 - source ePort number
* @param[in] portNum                  - physical port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*
* @note Enable access to IPCL/EPCL configuration table on ePort-based binding mode
*
*/
GT_STATUS prvTgfPclDefEportInitExt
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      ePortNum,
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
)
{
    GT_STATUS                       rc = GT_OK;
    static GT_BOOL                  isDeviceInitialized = GT_FALSE;

    if (GT_FALSE == isDeviceInitialized)
    {
        /* AUTODOC: Init PCL */
        rc = prvTgfPclInit();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclInit FAILED, rc = [%d]", rc);

            return rc;
        }

        /* do not init next time */
        isDeviceInitialized = GT_TRUE;
    }

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        /* AUTODOC: Enables ingress policy per devices */
        rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclIngressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }

        /* AUTODOC: Enables ingress policy on ePort */
        rc = prvTgfPclEPortIngressPolicyEnable(devNum, ePortNum, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclPortIngressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    else
    {
        /* AUTODOC: Enables egress policy per devices */
        rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEgressPolicyEnable FAILED, rc = [%d]", rc);

            return rc;
        }
        /* AUTODOC: Enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            devNum, portNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEgressPclPacketTypesSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* AUTODOC: Configure ePort access mode and PCL configuaration table */
    rc = prvTgfPclEportAccessModeInit(ePortNum, ePortNum,
                                      direction,lookupNum, lookupCfgPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclEportAccessModeInit FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}


/**
* @internal prvTgfPclPortListGroupingEnableSet function
* @endinternal
*
* @brief   Enable/disable port grouping mode per direction. When enabled, the 4 MSB
*         bits of the <Port List> field in the PCL keys are replaced by a 4-bit
*         <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - Policy direction
* @param[in] enable                   - enable/disable port grouping mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListGroupingEnableSet
(
    IN CPSS_PCL_DIRECTION_ENT   direction,
    IN GT_BOOL                  enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclPortListGroupingEnableSet(devNum, direction, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortListGroupingEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclPortListGroupingEnableGet function
* @endinternal
*
* @brief   Get the port grouping mode enabling status per direction. When enabled,
*         the 4 MSB bits of the <Port List> field in the PCL keys are replaced by a
*         4-bit <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
*
* @param[out] enablePtr                - (pointer to) the port grouping mode enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListGroupingEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;

    rc = cpssDxChPclPortListGroupingEnableGet(devNum, direction, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortListGroupingEnableGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclPortListPortMappingSet function
* @endinternal
*
* @brief   Set port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
* @param[in] enable                   - port participate in the port list vector.
* @param[in] group                    - PCL port group
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] offset                   -  in the port list bit vector.
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..27)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When working in PCL Port List mode with port grouping enabled, <offset>
*       value in the range 23..27 has no influence.
*
*/
GT_STATUS prvTgfPclPortListPortMappingSet
(
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclPortListPortMappingSet(devNum, direction, portNum,
                                               enable, group, offset);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortListPortMappingSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(group);
    TGF_PARAM_NOT_USED(offset);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclPortListPortMappingGet function
* @endinternal
*
* @brief   Get port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) port participate in the port list vector.
* @param[out] groupPtr                 - (pointer to) PCL port group
* @param[out] offsetPtr                - (pointer to) offset in the port list bit vector.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListPortMappingGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *groupPtr,
    OUT GT_U32                  *offsetPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;

    rc = cpssDxChPclPortListPortMappingGet(devNum, direction, portNum,
                                           enablePtr, groupPtr, offsetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclPortListPortMappingGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(groupPtr);
    TGF_PARAM_NOT_USED(offsetPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet function
* @endinternal
*
* @brief   Function enables/disables for ingress analyzed packet the using of VID from
*         the incoming packet to access the EPCL Configuration table and for lookup
*         keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] enable                   - GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet
(
    IN  GT_BOOL      enable
)
{
#ifdef CHX_FAMILY
    GT_U8      devNum  = 0;
    GT_STATUS  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet function
* @endinternal
*
* @brief   Get state of the flag in charge of the using of VID of the incoming packet
*         for ingress analyzed packet to access the EPCL Configuration table and for
*         lookup keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable/disable the using of original VID.
*                                      GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet
(
    IN  GT_U8         devNum,
    OUT GT_BOOL       *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;

    rc = cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet FAILED, rc = [%d]",
                         rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet function
* @endinternal
*
* @brief   Get absolute IPCL rules index bases in TCAM for IPCL0/1/2
*         the 'relative index' is modulo (%) by the function according to
*         appDemoDxChTcamIpclNumOfIndexsGet(lookupId)
*         relevant for earch devices, otherwise returns relativeTcamEntryIndex
* @param[in] lookupId                 -  IPCL - 0/1/2
* @param[in] relativeTcamEntryIndex
*                                       None
*/
GT_U32 prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet
(
    IN GT_U32      lookupId,
    IN  GT_U32     relativeTcamEntryIndex
)
{
    GT_U32 numIndexGet = appDemoDxChTcamIpclNumOfIndexsGet(prvTgfDevNum,lookupId);
    GT_U32 baseIndex   = appDemoDxChTcamIpclBaseIndexGet(prvTgfDevNum,lookupId);

    if(numIndexGet == 0)
    {
        /* support legacy device */
        return relativeTcamEntryIndex;
    }

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}
/**
* @internal prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet function
* @endinternal
*
* @brief   Get absolute EPCL rules index bases in TCAM for EPCL
*         the 'relative index' is modulo (%) by the function according to
*         appDemoDxChTcamEpclNumOfIndexsGet()
*         relevant for earch devices, otherwise returns relativeTcamEntryIndex
* @param[in] relativeTcamEntryIndex
*/
GT_U32 prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet
(
    IN  GT_U32     relativeTcamEntryIndex
)
{
    GT_U32 numIndexGet = appDemoDxChTcamEpclNumOfIndexsGet(prvTgfDevNum);
    GT_U32 baseIndex   = appDemoDxChTcamEpclBaseIndexGet(prvTgfDevNum);

    if(numIndexGet == 0)
    {
        /* support legacy device */
        return relativeTcamEntryIndex;
    }

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}

