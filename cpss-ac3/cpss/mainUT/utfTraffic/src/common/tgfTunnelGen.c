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
* @file tgfTunnelGen.c
*
* @brief Generic API implementation for Tunnel
*
* @version   114
********************************************************************************
*/

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
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
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfInlifGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPclGen.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

extern GT_U32 appDemoDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
);

extern GT_VOID appDemoDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useAppdemoOffset
);

/* whether the TTI indexes are relative to the start of the TTI section in TCAM */
static GT_BOOL ttiIndexIsRelative = GT_FALSE;

#endif /*CHX_FAMILY*/
/******************************************************************************\
 *                              Macro definitions                             *
\******************************************************************************/

#ifdef CHX_FAMILY

/* convert QoS mark modes into device specific format */
#define PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dstMarkMode, srcMarkMode)            \
    do                                                                         \
    {                                                                          \
        switch (srcMarkMode)                                                   \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:                       \
                dstMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E:          \
                dstMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;\
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert QoS mark modes from device specific format */
#define PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dstMarkMode, srcMarkMode)            \
    do                                                                         \
    {                                                                          \
        switch (dstMarkMode)                                                   \
        {                                                                      \
            case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E:                 \
                srcMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;          \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E:    \
                srcMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E; \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI key type into device specific format */
#define PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dstKeyType, srcKeyType)               \
    do                                                                         \
    {                                                                          \
        switch (srcKeyType)                                                    \
        {                                                                      \
            case PRV_TGF_TTI_KEY_IPV4_E:                                       \
                dstKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;                         \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_MPLS_E:                                       \
                dstKeyType = CPSS_DXCH_TTI_KEY_MPLS_E;                         \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_ETH_E:                                        \
                dstKeyType = CPSS_DXCH_TTI_KEY_ETH_E;                          \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_MIM_E:                                        \
                dstKeyType = CPSS_DXCH_TTI_KEY_MIM_E;                          \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E:                               \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;                 \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E:                               \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E;                 \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_MPLS_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_MPLS_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E:                          \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E;            \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E:                             \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E;               \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E:                         \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;           \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV6_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E:                               \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E;                 \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E:                               \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E;                 \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE_E:                                    \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE_E;                      \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE1_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE1_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE2_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE2_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE3_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE3_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE4_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE4_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE5_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE5_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_UDB_UDE6_E:                                   \
                dstKeyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;                     \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert TTI key type from device specific format */
#define PRV_TGF_D2S_KEY_TYPE_CONVERT_MAC(dstKeyType, srcKeyType)               \
    do                                                                         \
    {                                                                          \
        switch (dstKeyType)                                                    \
        {                                                                      \
            case CPSS_DXCH_TTI_KEY_IPV4_E:                                     \
                srcKeyType = PRV_TGF_TTI_KEY_IPV4_E;                           \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_MPLS_E:                                     \
                srcKeyType = PRV_TGF_TTI_KEY_MPLS_E;                           \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_ETH_E:                                      \
                srcKeyType = PRV_TGF_TTI_KEY_ETH_E;                            \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_MIM_E:                                      \
                srcKeyType = PRV_TGF_TTI_KEY_MIM_E;                            \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:                             \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E;                   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:                             \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E;                   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_MPLS_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:                        \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E;              \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:                           \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E;                 \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:                       \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;             \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV6_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:                             \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E;                   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:                             \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E;                   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE_E:                                  \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE_E;                        \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE1_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE2_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE3_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE4_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE5_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:                                 \
                srcKeyType = PRV_TGF_TTI_KEY_UDB_UDE6_E;                       \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert MAC mode into device specific format */
#define PRV_TGF_S2D_MAC_MODE_CONVERT_MAC(dstMacMode, srcMacMode)               \
    do                                                                         \
    {                                                                          \
        switch (srcMacMode)                                                    \
        {                                                                      \
            case PRV_TGF_TTI_MAC_MODE_DA_E:                                    \
                dstMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;                      \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_MAC_MODE_SA_E:                                    \
                dstMacMode = CPSS_DXCH_TTI_MAC_MODE_SA_E;                      \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert MAC mode from device specific format */
#define PRV_TGF_D2S_MAC_MODE_CONVERT_MAC(dstMacMode, srcMacMode)               \
    do                                                                         \
    {                                                                          \
        switch (dstMacMode)                                                    \
        {                                                                      \
            case CPSS_DXCH_TTI_MAC_MODE_DA_E:                                  \
                srcMacMode = PRV_TGF_TTI_MAC_MODE_DA_E;                        \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_MAC_MODE_SA_E:                                  \
                srcMacMode = PRV_TGF_TTI_MAC_MODE_SA_E;                        \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI exception type into device specific format */
#define PRV_TGF_S2D_EXCEPTION_TYPE_CONVERT_MAC(dstExcpType, srcExcpType)       \
    do                                                                         \
    {                                                                          \
        switch (srcExcpType)                                                   \
        {                                                                      \
            case PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:                    \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:                 \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E;  \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E:                \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E:                 \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E;  \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_BAD_VERSION_E:                    \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E;     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_E:                        \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E;         \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_CHBH_E:                           \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E;            \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E:                 \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E;  \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E:                   \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E;    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E:                \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E:                \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E:              \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E:           \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E:        \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E;  \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E:                 \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E;  \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E:                    \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E;     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E:                       \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E;        \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E:                   \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E;    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E:               \
                dstExcpType = CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E;\
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI exception type from device specific format */
#define PRV_TGF_D2S_EXCEPTION_TYPE_CONVERT_MAC(dstExcpType, srcExcpType)       \
    do                                                                         \
    {                                                                          \
        switch (dstExcpType)                                                   \
        {                                                                      \
            case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:                  \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E;       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E;  \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E;  \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E;  \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E:              \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E;   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E:               \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E;    \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E:                  \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_BAD_VERSION_E;       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E;  \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E:                      \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_E;           \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E:                         \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_CHBH_E;              \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E:               \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E;    \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E:                 \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E;      \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E:              \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E;   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E:              \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E;   \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E:            \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E:         \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E;\
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E:      \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E;  \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E:               \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E;    \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E:                  \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E;       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E:                     \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E;          \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E:                 \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E;      \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E:             \
                srcExcpType = PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E;  \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI offset type into device specific format */
#define PRV_TGF_S2D_OFFSET_TYPE_CONVERT_MAC(dstOffsetType, srcOffsetType)      \
    do                                                                         \
    {                                                                          \
        switch (srcOffsetType)                                                 \
        {                                                                      \
            case PRV_TGF_TTI_OFFSET_L2_E:                                      \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_L2_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_OFFSET_L3_MINUS_2_E:                              \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_OFFSET_L4_E:                                      \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_L4_E;                     \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E:                            \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E;           \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_OFFSET_METADATA_E:                                \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_METADATA_E;               \
                break;                                                         \
                                                                               \
            case PRV_TGFH_TTI_OFFSET_INVALID_E:                                \
                dstOffsetType = CPSS_DXCH_TTI_OFFSET_INVALID_E;                \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* convert TTI offset type from device specific format */
#define PRV_TGF_D2S_OFFSET_TYPE_CONVERT_MAC(dstOffsetType, srcOffsetType)      \
    do                                                                         \
    {                                                                          \
        switch (dstOffsetType)                                                 \
        {                                                                      \
            case CPSS_DXCH_TTI_OFFSET_L2_E:                                    \
                srcOffsetType = PRV_TGF_TTI_OFFSET_L2_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E:                            \
                srcOffsetType = PRV_TGF_TTI_OFFSET_L3_MINUS_2_E;               \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_OFFSET_L4_E:                                    \
                srcOffsetType = PRV_TGF_TTI_OFFSET_L4_E;                       \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E:                          \
                srcOffsetType = PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E;             \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_OFFSET_METADATA_E:                              \
                srcOffsetType = PRV_TGF_TTI_OFFSET_METADATA_E;                 \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_OFFSET_INVALID_E:                               \
                srcOffsetType = PRV_TGFH_TTI_OFFSET_INVALID_E;                 \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert key size into device specific format */
#define PRV_TGF_S2D_KEY_SIZE_CONVERT_MAC(dstKeySize, srcKeySize)               \
    do                                                                         \
    {                                                                          \
        switch (srcKeySize)                                                    \
        {                                                                      \
            case PRV_TGF_TTI_KEY_SIZE_10_B_E:                                  \
                dstKeySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;                    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_SIZE_20_B_E:                                  \
                dstKeySize = CPSS_DXCH_TTI_KEY_SIZE_20_B_E;                    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_SIZE_30_B_E:                                  \
                dstKeySize = CPSS_DXCH_TTI_KEY_SIZE_30_B_E;                    \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert key size from device specific format */
#define PRV_TGF_D2S_KEY_SIZE_CONVERT_MAC(dstKeySize, srcKeySize)               \
    do                                                                         \
    {                                                                          \
        switch (dstKeySize)                                                    \
        {                                                                      \
            case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:                                \
                srcKeySize = PRV_TGF_TTI_KEY_SIZE_10_B_E;                      \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:                                \
                srcKeySize = PRV_TGF_TTI_KEY_SIZE_20_B_E;                      \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:                                \
                srcKeySize = PRV_TGF_TTI_KEY_SIZE_30_B_E;                      \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert i-sid mode into device specific format */
#define PRV_TGF_S2D_MIM_ISID_MODE_CONVERT_MAC(dstMode, srcMode)                \
    do                                                                         \
    {                                                                          \
        switch (srcMode)                                                       \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E:        \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E:      \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E; \
                break;                                                         \
                                                                               \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert i-sid mode from device specific format */
#define PRV_TGF_D2S_MIM_ISID_MODE_CONVERT_MAC(dstMode, srcMode)                \
    do                                                                         \
    {                                                                          \
        switch (dstMode)                                                       \
        {                                                                      \
            case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E:      \
                srcMode = PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E:    \
                srcMode = PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E; \
                break;                                                         \
                                                                               \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert i-sid mode into device specific format */
#define PRV_TGF_S2D_MIM_BDA_MODE_CONVERT_MAC(dstMode, srcMode)                 \
    do                                                                         \
    {                                                                          \
        switch (srcMode)                                                       \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E:         \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E:  \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E; \
                break;                                                         \
                                                                               \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert b-sa assignmode into device specific format */
#define PRV_TGF_S2D_MIM_BSA_MODE_CONVERT_MAC(dstMode, srcMode)                 \
    do                                                                         \
    {                                                                          \
        switch (srcMode)                                                       \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E:         \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E:  \
                dstMode = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E; \
                break;                                                         \
                                                                               \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert i-sid mode from device specific format */
#define PRV_TGF_D2S_MIM_BDA_MODE_CONVERT_MAC(dstMode, srcMode)                 \
    do                                                                         \
    {                                                                          \
        switch (dstMode)                                                       \
        {                                                                      \
            case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E:       \
                srcMode = PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E:  \
                srcMode = PRV_TGF_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E; \
                break;                                                         \
                                                                               \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert PW CW error from device specific format */
#define PRV_TGF_PW_CW_ERROR_CONVERT_MAC(dstError, srcError)                    \
    do                                                                         \
    {                                                                          \
        switch (srcError)                                                      \
        {                                                                      \
            case PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E:           \
                dstError = CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E:                    \
                dstError = CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E;        \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E:          \
                dstError = CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_PW_CW_EXCEPTION_FRAGMENTED_E:                     \
                dstError = CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E;         \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E:                 \
                dstError = CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E;     \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI Key TCAM segment modes into device specific format */
#define PRV_TGF_S2D_SEGMENT_MODE_CONVERT_MAC(dstSegmentMode, srcSegmentMode)   \
    do                                                                         \
    {                                                                          \
        switch (srcSegmentMode)                                                \
        {                                                                      \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E:                 \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E:                 \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E:         \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E:         \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E:         \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E; \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E:                \
                dstSegmentMode = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E; \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI Key TCAM segment modes from device specific format */
#define PRV_TGF_D2S_SEGMENT_MODE_CONVERT_MAC(dstSegmentMode, srcSegmentMode)   \
    do                                                                         \
    {                                                                          \
        switch (dstSegmentMode)                                                \
        {                                                                      \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E:               \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E:               \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E:       \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E:       \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E:       \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E; \
                break;                                                         \
                                                                               \
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E:              \
                srcSegmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E;\
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert Tunnel-start ECN mode into device specific format */
#define PRV_TGF_TUNNEL_START_ECN_MODE_CONVERT_MAC(dstEcnMode, srcEcnMode)   \
    do                                                                         \
    {                                                                          \
        switch (srcEcnMode)                                                \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_ECN_COMPATIBILITY_MODE_E:                 \
                dstEcnMode = CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;\
                break;                                                         \
                                                                               \
            case PRV_TGF_TUNNEL_START_ECN_NORMAL_MODE_E:                 \
                dstEcnMode = CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E;\
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
/* convert QoS mark modes into device specific format */
#define PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dstMarkMode, srcMarkMode)            \
    do                                                                         \
    {                                                                          \
        switch (srcMarkMode)                                                   \
        {                                                                      \
            case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:                       \
                dstMarkMode = CPSS_EXMXPM_TUNNEL_START_MARK_FROM_ENTRY_E;      \
                break;                                                         \
                                                                               \
            case PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:                \
                dstMarkMode = CPSS_EXMXPM_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;\
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert QoS mark modes from device specific format */
#define PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dstMarkMode, srcMarkMode)            \
    do                                                                         \
    {                                                                          \
        switch (dstMarkMode)                                                   \
        {                                                                      \
            case CPSS_EXMXPM_TUNNEL_START_MARK_FROM_ENTRY_E:                   \
                srcMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;          \
                break;                                                         \
                                                                               \
            case CPSS_EXMXPM_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:            \
                srcMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;   \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI key type into device specific format */
#define PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dstKeyType, srcKeyType)               \
    do                                                                         \
    {                                                                          \
        switch (srcKeyType)                                                    \
        {                                                                      \
            case PRV_TGF_TTI_KEY_IPV4_E:                                       \
                dstKeyType = CPSS_EXMXPM_TTI_KEY_IPV4_E;                       \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_MPLS_E:                                       \
                dstKeyType = CPSS_EXMXPM_TTI_KEY_MPLS_E;                       \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_ETH_E:                                        \
                dstKeyType = CPSS_EXMXPM_TTI_KEY_ETH_E;                        \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_KEY_MIM_E:                                        \
                dstKeyType = CPSS_EXMXPM_TTI_KEY_MIM_E;                        \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert TTI key type from device specific format */
#define PRV_TGF_D2S_KEY_TYPE_CONVERT_MAC(dstKeyType, srcKeyType)               \
    do                                                                         \
    {                                                                          \
        switch (dstKeyType)                                                    \
        {                                                                      \
            case CPSS_EXMXPM_TTI_KEY_IPV4_E:                                   \
                srcKeyType = PRV_TGF_TTI_KEY_IPV4_E;                           \
                break;                                                         \
                                                                               \
            case CPSS_EXMXPM_TTI_KEY_MPLS_E:                                   \
                srcKeyType = PRV_TGF_TTI_KEY_MPLS_E;                           \
                break;                                                         \
                                                                               \
            case CPSS_EXMXPM_TTI_KEY_ETH_E:                                    \
                srcKeyType = PRV_TGF_TTI_KEY_ETH_E;                            \
                break;                                                         \
                                                                               \
            case CPSS_EXMXPM_TTI_KEY_MIM_E:                                    \
                srcKeyType = PRV_TGF_TTI_KEY_MIM_E;                            \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert MAC mode into device specific format */
#define PRV_TGF_S2D_MAC_MODE_CONVERT_MAC(dstMacMode, srcMacMode)               \
    do                                                                         \
    {                                                                          \
        switch (srcMacMode)                                                    \
        {                                                                      \
            case PRV_TGF_TTI_MAC_MODE_DA_E:                                    \
                dstMacMode = CPSS_EXMXPM_TTI_MAC_MODE_DA_E;                    \
                break;                                                         \
                                                                               \
            case PRV_TGF_TTI_MAC_MODE_SA_E:                                    \
                dstMacMode = CPSS_EXMXPM_TTI_MAC_MODE_SA_E;                    \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)

/* convert MAC mode from device specific format */
#define PRV_TGF_D2S_MAC_MODE_CONVERT_MAC(dstMacMode, srcMacMode)               \
    do                                                                         \
    {                                                                          \
        switch (dstMacMode)                                                    \
        {                                                                      \
            case CPSS_EXMXPM_TTI_MAC_MODE_DA_E:                                \
                srcMacMode = PRV_TGF_TTI_MAC_MODE_DA_E;                        \
                break;                                                         \
                                                                               \
            case CPSS_EXMXPM_TTI_MAC_MODE_SA_E:                                \
                srcMacMode = PRV_TGF_TTI_MAC_MODE_SA_E;                        \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericAction2ToDxChTtiAction2 function
* @endinternal
*
* @brief   Convert generic TTI action type2 into device specific TTI action
*
* @param[in] actionPtr                - (pointer to) TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericAction2ToDxChTtiAction2
(
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr,
    OUT CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr
)
{
    GT_STATUS rc;

    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChTtiActionPtr, 0, sizeof(*dxChTtiActionPtr));

    /* convert tti action into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelTerminate);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, copyTtlExpFromTunnelHeader);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mplsTtl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, enableDecrementTtl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, command);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, arpPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStart);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStartPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, routerLttPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceIdSetEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1VlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, nestedVlanEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicerMeter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, keepPreviousQoS);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0Up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1Up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, remapDSCP);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclConfigIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToCentralCounter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, centralCounterIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vntl2Echo);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bridgeBypass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ingressPipeBypass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, actionStop);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, hashMaskIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, modifyMacSa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, modifyMacDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ResetSrcPortGroupId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, multiPortGroupTtiEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceEPortAssignmentEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceEPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ttHeaderLength);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, passengerParsingOfTransitNonMplsTransitTunnelEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, cwBasedPw);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, applyNonDataCwCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, continueToNextTtiLookup);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, rxProtectionSwitchEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, rxIsProtectionPath);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, setMacToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosUseUpAsIndexEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosMappingTableIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclUdbConfigTableEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ttlExpiryVccvEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, pwe3FlowLabelExist);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, pwCwBasedETreeEnable);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamProcessEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamChannelTypeToOpcodeMappingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, unknownSaCommandEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, unknownSaCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceMeshIdSetEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceMeshId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelHeaderStartL4Enable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, skipFdbSaLookupEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ipv6SegmentRoutingEndNodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, exactMatchOverTtiEn);

    /* set interface info */
    cpssOsMemCpy(&(dxChTtiActionPtr->egressInterface),
                 &(actionPtr->egressInterface),
                 sizeof(dxChTtiActionPtr->egressInterface));

    if( PRV_TGF_TTI_REDIRECT_TO_EGRESS_E == actionPtr->redirectCommand &&
        CPSS_INTERFACE_PORT_E == actionPtr->egressInterface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)actionPtr->egressInterface.devPort.hwDevNum,
                                     &dxChTtiActionPtr->egressInterface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }

    /* convert ttPassengerPacketType into device specific format */
    switch (actionPtr->ttPassengerPacketType)
    {
        case PRV_TGF_TTI_PASSENGER_IPV4_E:
        case PRV_TGF_TTI_PASSENGER_IPV6_E:
        case PRV_TGF_TTI_PASSENGER_IPV4V6_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_MPLS_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_MPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tsPassengerPacketType into device specific format */
    switch (actionPtr->tsPassengerPacketType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChTtiActionPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChTtiActionPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand into device specific format */
    switch (actionPtr->mplsCommand)
    {
        case PRV_TGF_TTI_MPLS_NOP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_SWAP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP3_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redirectCommand into device specific format */
    switch (actionPtr->redirectCommand)
    {
        case PRV_TGF_TTI_NO_REDIRECT_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
            break;

        case PRV_TGF_TTI_REDIRECT_TO_EGRESS_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
            break;

        case PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
            break;

        case PRV_TGF_TTI_VRF_ID_ASSIGN_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
            break;

        case PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag0VlanCmd into device specific format */
    switch (actionPtr->tag0VlanCmd)
    {
        case PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_ALL_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    switch (actionPtr->tag1VlanCmd)
    {
        case PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E:
            dxChTtiActionPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN1_MODIFY_ALL_E:
            dxChTtiActionPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyTag0Up into device specific format */
    switch (actionPtr->modifyTag0Up)
    {
        case PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_ENABLE_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_DISABLE_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_RESERVED_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1UpCommand into device specific format */
    switch (actionPtr->tag1UpCommand)
    {
        case PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_TAG1_UP_ASSIGN_ALL_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyDscp into device specific format */
    switch (actionPtr->modifyDscp)
    {
        case PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_RESERVED_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl0OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0_1OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl0_1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerParsingOfTransitMplsTunnelMode into device specific format */
    switch (actionPtr->passengerParsingOfTransitMplsTunnelMode)
    {
        case PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E:
            dxChTtiActionPtr->passengerParsingOfTransitMplsTunnelMode =
                CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E;
            break;

        case PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E:
            dxChTtiActionPtr->passengerParsingOfTransitMplsTunnelMode =
                CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E;
            break;

        case PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E:
            dxChTtiActionPtr->passengerParsingOfTransitMplsTunnelMode =
                CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E;
            break;

        case PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E:
            dxChTtiActionPtr->passengerParsingOfTransitMplsTunnelMode =
                CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E;
            break;

        default:
            return GT_BAD_PARAM;
    }


    if(GT_TRUE == actionPtr->iPclUdbConfigTableEnable)
    {
        /* convert iPclUdbConfigTableIndex into device specific format */
        switch (actionPtr->iPclUdbConfigTableIndex)
        {
            case PRV_TGF_PCL_PACKET_TYPE_UDE_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE_1_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE_2_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE_3_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE_4_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE5_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
                break;
            case PRV_TGF_PCL_PACKET_TYPE_UDE6_E:
                dxChTtiActionPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTtiAction2 function
* @endinternal
*
* @brief   Convert generic TTI action type2 into device specific TTI action
*
* @param[in] actionPtr                - (pointer to) TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTtiAction2
(
    IN  PRV_TGF_TTI_ACTION_STC     *actionPtr,
    OUT CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr
)
{
    GT_STATUS rc;

    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChTtiActionPtr, 0, sizeof(*dxChTtiActionPtr));

    /* convert tti action into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelTerminate);
    dxChTtiActionPtr->copyTtlExpFromTunnelHeader = actionPtr->copyTtlFromTunnelHeader;
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, command);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, arpPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStart);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStartPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceIdSetEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1VlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, nestedVlanEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, keepPreviousQoS);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0Up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1Up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, remapDSCP);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclConfigIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToCentralCounter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, centralCounterIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vntl2Echo);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bridgeBypass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ingressPipeBypass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, actionStop);


    /* convert tti action into device specific format */
    dxChTtiActionPtr->mplsTtl            = actionPtr->ttl;
    dxChTtiActionPtr->enableDecrementTtl = actionPtr->enableDecTtl;
    dxChTtiActionPtr->routerLttPtr       = actionPtr->routerLookupPtr;
    dxChTtiActionPtr->tag0VlanPrecedence = actionPtr->vlanPrecedence;
    dxChTtiActionPtr->bindToPolicerMeter = actionPtr->meterEnable;

    /* set interface info */
    cpssOsMemCpy(&(dxChTtiActionPtr->egressInterface),
                 &(actionPtr->interfaceInfo),
                 sizeof(dxChTtiActionPtr->egressInterface));

    if(( PRV_TGF_TTI_REDIRECT_TO_EGRESS_E == actionPtr->redirectCommand &&
         CPSS_INTERFACE_PORT_E == actionPtr->interfaceInfo.type ) ||
       (PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E == actionPtr->redirectCommand &&
        CPSS_INTERFACE_PORT_E == actionPtr->interfaceInfo.type))
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)actionPtr->interfaceInfo.devPort.hwDevNum,
                                     &dxChTtiActionPtr->egressInterface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }

    /* convert ttPassengerPacketType into device specific format */
    switch (actionPtr->passengerPacketType)
    {
        case PRV_TGF_TTI_PASSENGER_IPV4_E:
        case PRV_TGF_TTI_PASSENGER_IPV6_E:
        case PRV_TGF_TTI_PASSENGER_IPV4V6_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_MPLS_E:
            dxChTtiActionPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_MPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tsPassengerPacketType into device specific format */
    switch (actionPtr->tsPassengerPacketType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChTtiActionPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChTtiActionPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand into device specific format */
    switch (actionPtr->mplsCmd)
    {
        case PRV_TGF_TTI_MPLS_NOP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_SWAP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP3_CMD_E;
            break;

        case PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            dxChTtiActionPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redirectCommand into device specific format */
    switch (actionPtr->redirectCommand)
    {
        case PRV_TGF_TTI_NO_REDIRECT_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
            break;

        case PRV_TGF_TTI_REDIRECT_TO_EGRESS_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
            break;

        case PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
            break;

        case PRV_TGF_TTI_VRF_ID_ASSIGN_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
            break;

        case PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E:
            dxChTtiActionPtr->redirectCommand = CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag0VlanCmd into device specific format */
    switch (actionPtr->tag0VlanCmd)
    {
        case PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_ALL_E:
            dxChTtiActionPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    switch (actionPtr->tag1VlanCmd)
    {
        case PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E:
            dxChTtiActionPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN1_MODIFY_ALL_E:
            dxChTtiActionPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyTag0Up into device specific format */
    switch (actionPtr->modifyUpEnable)
    {
        case PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_ENABLE_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_DISABLE_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_UP_RESERVED_E:
            dxChTtiActionPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1UpCommand into device specific format */
    switch (actionPtr->tag1UpCommand)
    {
        case PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_TAG1_UP_ASSIGN_ALL_E:
            dxChTtiActionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyDscp into device specific format */
    switch (actionPtr->modifyDscpEnable)
    {
        case PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E;
            break;

        case PRV_TGF_TTI_MODIFY_DSCP_RESERVED_E:
            dxChTtiActionPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl0OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0_1OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl0_1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex into device specific format */
    switch (actionPtr->pcl1OverrideConfigIndex)
    {
        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            dxChTtiActionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            dxChTtiActionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTtiAction2 function
* @endinternal
*
* @brief   Convert device specific TTI action type2 into generic TTI action
*
* @param[in] dxChTtiActionPtr         - (pointer to) DxCh TTI rule action
*
* @param[out] actionPtr                - (pointer to) TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTtiAction2
(
    IN  CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr,
    OUT PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
    GT_STATUS rc;

    /* clear tti action */
    cpssOsMemSet((GT_VOID*) actionPtr, 0, sizeof(PRV_TGF_TTI_ACTION_STC));

    /* convert tti action from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelTerminate);
    actionPtr->copyTtlFromTunnelHeader = dxChTtiActionPtr->copyTtlExpFromTunnelHeader;
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, command);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, arpPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStart);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStartPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vrfId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceIdSetEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1VlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, nestedVlanEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, keepPreviousQoS);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustDscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustExp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0Up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1Up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, remapDSCP);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclConfigIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToCentralCounter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, centralCounterIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vntl2Echo);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bridgeBypass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ingressPipeBypass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, actionStop);

    /* convert tti action from device specific format */
    actionPtr->ttl             = dxChTtiActionPtr->mplsTtl;
    actionPtr->enableDecTtl    = dxChTtiActionPtr->enableDecrementTtl;
    actionPtr->routerLookupPtr = dxChTtiActionPtr->routerLttPtr;
    actionPtr->vlanPrecedence  = dxChTtiActionPtr->tag0VlanPrecedence;
    actionPtr->meterEnable     = dxChTtiActionPtr->bindToPolicerMeter;

    /* set interface info */
    cpssOsMemCpy(&(actionPtr->interfaceInfo),
                 &(dxChTtiActionPtr->egressInterface),
                 sizeof(actionPtr->interfaceInfo));

    if( CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == dxChTtiActionPtr->redirectCommand &&
        CPSS_INTERFACE_PORT_E == dxChTtiActionPtr->egressInterface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfSwFromHwDeviceNumberGet(dxChTtiActionPtr->egressInterface.devPort.hwDevNum,
                                       &actionPtr->interfaceInfo.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }

    /* convert ttPassengerPacketType from device specific format */
    switch (dxChTtiActionPtr->ttPassengerPacketType)
    {
        case CPSS_DXCH_TTI_PASSENGER_IPV4_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_IPV6_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV6_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_IPV4V6_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4V6_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_MPLS_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_MPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tsPassengerPacketType from device specific format */
    switch (dxChTtiActionPtr->tsPassengerPacketType)
    {
        case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
            actionPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
            actionPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand from device specific format */
    switch (dxChTtiActionPtr->mplsCommand)
    {
        case CPSS_DXCH_TTI_MPLS_NOP_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_SWAP_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP1_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP2_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP3_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redirectCommand from device specific format */
    switch (dxChTtiActionPtr->redirectCommand)
    {
        case CPSS_DXCH_TTI_NO_REDIRECT_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
            break;

        case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
            break;

        case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
            break;

        case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
            break;

        case CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag0VlanCmd from device specific format */
    switch (dxChTtiActionPtr->tag0VlanCmd)
    {
        case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1VlanCmd from device specific format */
    switch (dxChTtiActionPtr->tag1VlanCmd)
    {
        case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_DO_NOT_MODIFY_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_TAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyTag0Up from device specific format */
    switch (dxChTtiActionPtr->modifyTag0Up)
    {
        case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E:
            actionPtr->modifyUpEnable = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E:
            actionPtr->modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E:
            actionPtr->modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_DISABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E:
            actionPtr->modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1UpCommand from device specific format */
    switch (dxChTtiActionPtr->tag1UpCommand)
    {
        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyDscp from device specific format */
    switch (dxChTtiActionPtr->modifyDscp)
    {
        case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
            actionPtr->modifyDscpEnable = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E:
            actionPtr->modifyDscpEnable = PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E:
            actionPtr->modifyDscpEnable = PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E:
            actionPtr->modifyDscpEnable = PRV_TGF_TTI_MODIFY_DSCP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl0OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0_1OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl0_1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChAction2ToGenericTtiAction2 function
* @endinternal
*
* @brief   Convert device specific TTI action type2 into generic TTI action 2
*
* @param[in] dxChTtiActionPtr         - (pointer to) DxCh TTI rule action 2
*
* @param[out] actionPtr                - (pointer to) TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChAction2ToGenericTtiAction2
(
    IN  CPSS_DXCH_TTI_ACTION_2_STC   *dxChTtiActionPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
)
{
    GT_STATUS rc;

    /* clear tti action */
    cpssOsMemSet((GT_VOID*) actionPtr, 0, sizeof(PRV_TGF_TTI_ACTION_2_STC));

    /* convert tti action from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelTerminate);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, copyTtlExpFromTunnelHeader);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mplsTtl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, enableDecrementTtl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, command);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, arpPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStart);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelStartPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, routerLttPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vrfId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceIdSetEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1VlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0VlanPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, nestedVlanEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicerMeter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToPolicer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, keepPreviousQoS);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustDscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, trustExp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag0Up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tag1Up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, remapDSCP);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclConfigIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, mirrorToIngressAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bindToCentralCounter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, centralCounterIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, vntl2Echo);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, bridgeBypass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ingressPipeBypass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, actionStop);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, hashMaskIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, modifyMacSa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, modifyMacDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ResetSrcPortGroupId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, multiPortGroupTtiEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceEPortAssignmentEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceEPort);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ttHeaderLength);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, passengerParsingOfTransitNonMplsTransitTunnelEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, cwBasedPw);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, applyNonDataCwCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, continueToNextTtiLookup);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, rxProtectionSwitchEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, rxIsProtectionPath);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, setMacToMe);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosUseUpAsIndexEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, qosMappingTableIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, iPclUdbConfigTableEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ttlExpiryVccvEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, pwe3FlowLabelExist);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, pwCwBasedETreeEnable);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamProcessEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, oamChannelTypeToOpcodeMappingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, unknownSaCommandEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, unknownSaCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceMeshIdSetEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, sourceMeshId);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, tunnelHeaderStartL4Enable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, skipFdbSaLookupEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, ipv6SegmentRoutingEndNodeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTtiActionPtr, actionPtr, exactMatchOverTtiEn);

    /* set interface info */
    cpssOsMemCpy(&(actionPtr->egressInterface),
                 &(dxChTtiActionPtr->egressInterface),
                 sizeof(actionPtr->egressInterface));

    if( CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == dxChTtiActionPtr->redirectCommand &&
        CPSS_INTERFACE_PORT_E == dxChTtiActionPtr->egressInterface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfSwFromHwDeviceNumberGet(dxChTtiActionPtr->egressInterface.devPort.hwDevNum,
                                       &actionPtr->egressInterface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }

    /* convert ttPassengerPacketType from device specific format */
    switch (dxChTtiActionPtr->ttPassengerPacketType)
    {
        case CPSS_DXCH_TTI_PASSENGER_IPV4_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_IPV6_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV6_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_IPV4V6_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4V6_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_MPLS_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_MPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tsPassengerPacketType from device specific format */
    switch (dxChTtiActionPtr->tsPassengerPacketType)
    {
        case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
            actionPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
            actionPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand from device specific format */
    switch (dxChTtiActionPtr->mplsCommand)
    {
        case CPSS_DXCH_TTI_MPLS_NOP_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_SWAP_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP1_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP2_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP3_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redirectCommand from device specific format */
    switch (dxChTtiActionPtr->redirectCommand)
    {
        case CPSS_DXCH_TTI_NO_REDIRECT_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
            break;

        case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
            break;

        case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
            break;

        case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
            break;

        case CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag0VlanCmd from device specific format */
    switch (dxChTtiActionPtr->tag0VlanCmd)
    {
        case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1VlanCmd from device specific format */
    switch (dxChTtiActionPtr->tag1VlanCmd)
    {
        case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_DO_NOT_MODIFY_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_TAGGED_E;
            break;

        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            actionPtr->tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyTag0Up from device specific format */
    switch (dxChTtiActionPtr->modifyTag0Up)
    {
        case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E:
            actionPtr->modifyTag0Up = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E:
            actionPtr->modifyTag0Up = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E:
            actionPtr->modifyTag0Up = PRV_TGF_TTI_MODIFY_UP_DISABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E:
            actionPtr->modifyTag0Up = PRV_TGF_TTI_MODIFY_UP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tag1UpCommand from device specific format */
    switch (dxChTtiActionPtr->tag1UpCommand)
    {
        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
            break;

        case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E:
            actionPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert modifyDscp from device specific format */
    switch (dxChTtiActionPtr->modifyDscp)
    {
        case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
            actionPtr->modifyDscp = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E:
            actionPtr->modifyDscp = PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E:
            actionPtr->modifyDscp = PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E;
            break;

        case CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E:
            actionPtr->modifyDscp = PRV_TGF_TTI_MODIFY_DSCP_RESERVED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl0OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl0_1OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl0_1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pcl1OverrideConfigIndex from device specific format */
    switch (dxChTtiActionPtr->pcl1OverrideConfigIndex)
    {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            actionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;

        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            actionPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerParsingOfTransitMplsTunnelMode from device specific format */
    switch (dxChTtiActionPtr->passengerParsingOfTransitMplsTunnelMode)
    {
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E:
            actionPtr->passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E:
            actionPtr->passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E:
            actionPtr->passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E;
            break;

        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E:
            actionPtr->passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    if(GT_TRUE == dxChTtiActionPtr->iPclUdbConfigTableEnable)
    {
        /* convert iPclUdbConfigTableIndex from device specific format */
        switch (dxChTtiActionPtr->iPclUdbConfigTableIndex)
        {
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE_1_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE_2_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE_3_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE_4_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE5_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:
                actionPtr->iPclUdbConfigTableIndex = PRV_TGF_PCL_PACKET_TYPE_UDE6_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    return GT_OK;
}
/* check if need to mask fields for the device .
    macro do 'return' when not needed.
*/
#define CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(_devNum)                   \
    if(GT_FALSE == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum))  \
    {                                                                     \
        /* it seems that CPSS for legacy devices did not care about overflow of fields values */\
        /* so do not mask any field */                                    \
        return;                                                           \
    }

/**
* @internal prvTgfMaskDxChCommonFeildsInTtiRule function
* @endinternal
*
* @brief   mask common fields with proper mask
*
* @param[in,out] commonPtr                - (pointer to) common fields that need to be masked with proper mask.
* @param[in,out] commonPtr                - (pointer to) updated common fields after masking of proper mask.
*                                       None
*/
static void prvTgfMaskDxChCommonFeildsInTtiRule
(
    INOUT CPSS_DXCH_TTI_RULE_COMMON_STC *commonPtr
)
{
    GT_U8 devNum = prvTgfDevNum;
    GT_U32  portTrunkMask = (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum)) ?
                                PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) : PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum);

    CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(prvTgfDevNum);

    /*JIRA : CPSS-4581 - cpssDxChTtiRuleSet not support EPORTs up to 16K --> for BC3 has been successfully created.*/
    portTrunkMask &= BIT_13-1;  CPSS_TBD_BOOKMARK_BOBCAT3


    commonPtr->pclId            &= BIT_10 - 1;
    commonPtr->srcIsTrunk       &= 1;
    commonPtr->srcPortTrunk     &= portTrunkMask;
    commonPtr->vid              &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum);
    commonPtr->isTagged         &= 1;
    commonPtr->dsaSrcIsTrunk    &= 1;
    commonPtr->dsaSrcPortTrunk  &= portTrunkMask;
    commonPtr->dsaSrcDevice     &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum);
}

/**
* @internal prvTgfMaskDxChIpv4FeildsInTtiRule function
* @endinternal
*
* @brief   mask Ipv4 fields with proper mask
*/
static void prvTgfMaskDxChIpv4FeildsInTtiRule
(
    INOUT CPSS_DXCH_TTI_IPV4_RULE_STC *ipv4Ptr
)
{
    CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(prvTgfDevNum);

    /* mask the common fields */
    prvTgfMaskDxChCommonFeildsInTtiRule(&(ipv4Ptr->common));

    /* mask the specific ipv4 fields */
    ipv4Ptr->tunneltype     &= BIT_3 - 1;
    ipv4Ptr->isArp          &= 1;

}


/**
* @internal prvTgfMaskDxChMplsFeildsInTtiRule function
* @endinternal
*
* @brief   mask MPLS fields with proper mask
*
* @param[in,out] mplsPtr                  - (pointer to) MPLS fields that need to be masked with proper mask.
* @param[in,out] mplsPtr                  - (pointer to) updated MPLS fields after masking of proper mask.
*                                       None
*/
static void prvTgfMaskDxChMplsFeildsInTtiRule
(
    INOUT CPSS_DXCH_TTI_MPLS_RULE_STC *mplsPtr
)
{
    CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(prvTgfDevNum);

    /* mask the common fields */
    prvTgfMaskDxChCommonFeildsInTtiRule(&(mplsPtr->common));

    /* mask the specific MPLS fields */
    mplsPtr->label0               &= BIT_20 - 1;
    mplsPtr->exp0                 &= BIT_3 - 1;
    mplsPtr->label1               &= BIT_20 - 1;
    mplsPtr->exp1                 &= BIT_3 - 1;
    mplsPtr->label2               &= BIT_20 - 1;
    mplsPtr->exp2                 &= BIT_3 - 1;
/*    mplsPtr->numOfLabels*/
    mplsPtr->protocolAboveMPLS    &= BIT_2 - 1;
    mplsPtr->reservedLabelExist   &= 1;
    mplsPtr->reservedLabelValue   &= BIT_4 - 1;
    mplsPtr->channelTypeProfile   &= BIT_4 - 1;
    mplsPtr->dataAfterInnerLabel  &= BIT_5 - 1;
}

/**
* @internal prvTgfMaskDxChEthernetFeildsInTtiRule function
* @endinternal
*
* @brief   mask ETHERNET fields with proper mask
*
* @param[in,out] ethPtr                   - (pointer to) ETHERNET fields that need to be masked with proper mask.
* @param[in,out] ethPtr                   - (pointer to) updated ETHERNET fields after masking of proper mask.
*                                       None
*/
static void prvTgfMaskDxChEthernetFeildsInTtiRule
(
    INOUT CPSS_DXCH_TTI_ETH_RULE_STC *ethPtr
)
{
    CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(prvTgfDevNum);

    /* mask the common fields */
    prvTgfMaskDxChCommonFeildsInTtiRule(&(ethPtr->common));

    /* mask the specific ETHERNETMPLS fields */
    ethPtr->up0                 &= BIT_3 - 1;
    ethPtr->cfi0                &= 1;
    ethPtr->isVlan1Exists       &= 1;
    ethPtr->vid1                &= BIT_12 - 1;
    ethPtr->up1                 &= BIT_3 - 1;
    ethPtr->cfi1                &= 1;
    ethPtr->etherType           &= BIT_16 - 1;
    ethPtr->macToMe             &= 1;
    ethPtr->srcId               &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(prvTgfDevNum);
    ethPtr->dsaQosProfile       &= BIT_7 - 1;
    ethPtr->tag0TpidIndex       &= BIT_3 - 1;
    ethPtr->tag1TpidIndex       &= BIT_3 - 1;
}

/**
* @internal prvTgfMaskDxChMimFeildsInTtiRule function
* @endinternal
*
* @brief   mask MIM fields with proper mask
*
* @param[in,out] mimPtr                   - (pointer to) MIM fields that need to be masked with proper mask.
* @param[in,out] mimPtr                   - (pointer to) updated MIM fields after masking of proper mask.
*                                       None
*/
static void prvTgfMaskDxChMimFeildsInTtiRule
(
    INOUT CPSS_DXCH_TTI_MIM_RULE_STC *mimPtr
)
{
    CHECK_NEED_TO_MASK_TTI_RULE_FIELDS_MAC(prvTgfDevNum);

    /* mask the common fields */
    prvTgfMaskDxChCommonFeildsInTtiRule(&(mimPtr->common));

    /* mask the specific MIM fields */
    mimPtr->bUp                         &= BIT_3 - 1;
    mimPtr->bDp                         &= 1;
    mimPtr->iSid                        &= BIT_24 - 1;
    mimPtr->iUp                         &= BIT_3 - 1;
    mimPtr->iDp                         &= 1;
    mimPtr->iRes1                       &= BIT_2 - 1;
    mimPtr->iRes2                       &= BIT_2 - 1;
    mimPtr->macToMe                     &= 1;
    mimPtr->passengerPacketOuterTagExists       &= 1;
    mimPtr->passengerPacketOuterTagVid          &= BIT_12 - 1;
    mimPtr->passengerPacketOuterTagUp           &= BIT_3 - 1;
    mimPtr->passengerPacketOuterTagDei          &= 1;

}


/**
* @internal prvTgfConvertGenericToDxChIpv4TtiRule function
* @endinternal
*
* @brief   Convert generic IPv4 TTI rule into device specific IPv4 TTI rule
*
* @param[in] ipv4TtiRulePtr           - (pointer to) IPv4 TTI rule
*
* @param[out] dxChIpv4TtiRulePtr       - (pointer to) DxCh IPv4 TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChIpv4TtiRule
(
    IN  PRV_TGF_TTI_IPV4_RULE_STC    *ipv4TtiRulePtr,
    OUT CPSS_DXCH_TTI_IPV4_RULE_STC  *dxChIpv4TtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChIpv4TtiRulePtr, 0, sizeof(*dxChIpv4TtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcPortTrunk);
    dxChIpv4TtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(ipv4TtiRulePtr->common.dsaSrcDevice, GT_TRUE);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4TtiRulePtr, ipv4TtiRulePtr, tunneltype);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4TtiRulePtr, ipv4TtiRulePtr, isArp);


    /* set MAC */
    cpssOsMemCpy(dxChIpv4TtiRulePtr->common.mac.arEther,
                 ipv4TtiRulePtr->common.mac.arEther,
                 sizeof(ipv4TtiRulePtr->common.mac.arEther));

    /* set source IP */
    cpssOsMemCpy(dxChIpv4TtiRulePtr->srcIp.arIP,
                 ipv4TtiRulePtr->srcIp.arIP,
                 sizeof(ipv4TtiRulePtr->srcIp.arIP));

    /* set destination IP */
    cpssOsMemCpy(dxChIpv4TtiRulePtr->destIp.arIP,
                 ipv4TtiRulePtr->destIp.arIP,
                 sizeof(ipv4TtiRulePtr->destIp.arIP));

    /* mask the ipv4 fields */
    prvTgfMaskDxChIpv4FeildsInTtiRule(dxChIpv4TtiRulePtr);
}

/**
* @internal prvTgfConvertDxChToGenericIpv4TtiRule function
* @endinternal
*
* @brief   Convert device specific IPv4 TTI rule into generic IPv4 TTI rule
*
* @param[in] dxChIpv4TtiRulePtr       - (pointer to) DxCh IPv4 TTI rule
*
* @param[out] ipv4TtiRulePtr           - (pointer to) IPv4 TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericIpv4TtiRule
(
    IN  CPSS_DXCH_TTI_IPV4_RULE_STC  *dxChIpv4TtiRulePtr,
    OUT PRV_TGF_TTI_IPV4_RULE_STC    *ipv4TtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) ipv4TtiRulePtr, 0, sizeof(*ipv4TtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4TtiRulePtr, ipv4TtiRulePtr, tunneltype);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4TtiRulePtr, ipv4TtiRulePtr, isArp);

    /* set MAC */
    cpssOsMemCpy(ipv4TtiRulePtr->common.mac.arEther,
                 dxChIpv4TtiRulePtr->common.mac.arEther,
                 sizeof(dxChIpv4TtiRulePtr->common.mac.arEther));

    /* set source IP */
    cpssOsMemCpy(ipv4TtiRulePtr->srcIp.arIP,
                 dxChIpv4TtiRulePtr->srcIp.arIP,
                 sizeof(dxChIpv4TtiRulePtr->srcIp.arIP));

    /* set destination IP */
    cpssOsMemCpy(ipv4TtiRulePtr->destIp.arIP,
                 dxChIpv4TtiRulePtr->destIp.arIP,
                 sizeof(dxChIpv4TtiRulePtr->destIp.arIP));
}

/**
* @internal prvTgfConvertGenericToDxChMplsTtiRule function
* @endinternal
*
* @brief   Convert generic Mpls TTI rule into device specific Mpls TTI rule
*
* @param[out] dxChMplsTtiRulePtr       - (pointer to) DxCh Mpls TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChMplsTtiRule
(
    IN  PRV_TGF_TTI_MPLS_RULE_STC    *mplsTtiRulePtr,
    OUT CPSS_DXCH_TTI_MPLS_RULE_STC  *dxChMplsTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChMplsTtiRulePtr, 0, sizeof(*dxChMplsTtiRulePtr));

    /* convert Mpls tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcPortTrunk);
    dxChMplsTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(mplsTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, numOfLabels);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, protocolAboveMPLS);

    /* set MAC */
    cpssOsMemCpy(dxChMplsTtiRulePtr->common.mac.arEther,
                 mplsTtiRulePtr->common.mac.arEther,
                 sizeof(mplsTtiRulePtr->common.mac.arEther));


    /* mask the MPLS fields */
    prvTgfMaskDxChMplsFeildsInTtiRule(dxChMplsTtiRulePtr);
}

/**
* @internal prvTgfConvertDxChToGenericMplsTtiRule function
* @endinternal
*
* @brief   Convert device specific Mpls TTI rule into generic Mpls TTI rule
*
* @param[in] dxChMplsTtiRulePtr       - (pointer to) DxCh Mpls TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericMplsTtiRule
(
    IN  CPSS_DXCH_TTI_MPLS_RULE_STC  *dxChMplsTtiRulePtr,
    OUT PRV_TGF_TTI_MPLS_RULE_STC    *mplsTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) mplsTtiRulePtr, 0, sizeof(*mplsTtiRulePtr));

    /* convert Mpls tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, label2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, exp2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, numOfLabels);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsTtiRulePtr, mplsTtiRulePtr, protocolAboveMPLS);

    /* set MAC */
    cpssOsMemCpy(mplsTtiRulePtr->common.mac.arEther,
                 dxChMplsTtiRulePtr->common.mac.arEther,
                 sizeof(dxChMplsTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToDxChEthTtiRule function
* @endinternal
*
* @brief   Convert generic Ethernet TTI rule into device specific Ethernet TTI rule
*
* @param[in] ethTtiRulePtr            - (pointer to) Ethernet TTI rule
*
* @param[out] dxChEthTtiRulePtr        - (pointer to) DxCh Ethernet TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChEthTtiRule
(
    IN  PRV_TGF_TTI_ETH_RULE_STC     *ethTtiRulePtr,
    OUT CPSS_DXCH_TTI_ETH_RULE_STC   *dxChEthTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChEthTtiRulePtr, 0, sizeof(*dxChEthTtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcPortTrunk);
    dxChEthTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(ethTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, up0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, cfi0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, isVlan1Exists);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, macToMe);

    /* set MAC */
    cpssOsMemCpy(dxChEthTtiRulePtr->common.mac.arEther,
                 ethTtiRulePtr->common.mac.arEther,
                 sizeof(ethTtiRulePtr->common.mac.arEther));

    /* mask the ETHERNET fields */
    prvTgfMaskDxChEthernetFeildsInTtiRule(dxChEthTtiRulePtr);
}

/**
* @internal prvTgfConvertDxChToGenericEthTtiRule function
* @endinternal
*
* @brief   Convert device specific Ethernet TTI rule into generic Ethernet TTI rule
*
* @param[in] dxChEthTtiRulePtr        - (pointer to) DxCh Ethernet TTI rule
*
* @param[out] ethTtiRulePtr            - (pointer to) Ethernet TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericEthTtiRule
(
    IN  CPSS_DXCH_TTI_ETH_RULE_STC   *dxChEthTtiRulePtr,
    OUT PRV_TGF_TTI_ETH_RULE_STC     *ethTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) ethTtiRulePtr, 0, sizeof(*ethTtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, up0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, cfi0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, isVlan1Exists);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, vid1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, up1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, cfi1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, etherType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEthTtiRulePtr, ethTtiRulePtr, macToMe);

    /* set MAC */
    cpssOsMemCpy(ethTtiRulePtr->common.mac.arEther,
                 dxChEthTtiRulePtr->common.mac.arEther,
                 sizeof(dxChEthTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToDxChMimTtiRule function
* @endinternal
*
* @brief   Convert generic MIM TTI rule into device specific MIM TTI rule
*
* @param[in] mimTtiRulePtr            - (pointer to) MIM TTI rule
*
* @param[out] dxChMimTtiRulePtr        - (pointer to) DxCh MIM TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChMimTtiRule
(
    IN  PRV_TGF_TTI_MIM_RULE_STC       *mimTtiRulePtr,
    OUT CPSS_DXCH_TTI_MIM_RULE_STC     *dxChMimTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChMimTtiRulePtr, 0, sizeof(*dxChMimTtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcPortTrunk);
    dxChMimTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(mimTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, bUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, bDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iSid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iRes1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iRes2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagExists);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagVid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagDei);

    /* set MAC */
    cpssOsMemCpy(dxChMimTtiRulePtr->common.mac.arEther,
                 mimTtiRulePtr->common.mac.arEther,
                 sizeof(dxChMimTtiRulePtr->common.mac.arEther));

    /* mask the MIM fields */
    prvTgfMaskDxChMimFeildsInTtiRule(dxChMimTtiRulePtr);

}

/**
* @internal prvTgfConvertDxChToGenericMimTtiRule function
* @endinternal
*
* @brief   Convert device specific MIM TTI rule into generic MIM TTI rule
*
* @param[in] dxChMimTtiRulePtr        - (pointer to) DxCh MIM TTI rule
*
* @param[out] mimTtiRulePtr            - (pointer to) MIM TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericMimTtiRule
(
    IN  CPSS_DXCH_TTI_MIM_RULE_STC     *dxChMimTtiRulePtr,
    OUT PRV_TGF_TTI_MIM_RULE_STC       *mimTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) mimTtiRulePtr, 0, sizeof(*mimTtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, bUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, bDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iSid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iRes1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, iRes2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, macToMe);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagExists);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagVid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimTtiRulePtr, mimTtiRulePtr, passengerPacketOuterTagDei);

    /* set MAC */
    cpssOsMemCpy(mimTtiRulePtr->common.mac.arEther,
                 dxChMimTtiRulePtr->common.mac.arEther,
                 sizeof(mimTtiRulePtr->common.mac.arEther));
}
#if 0
/**
* @internal prvTgfConvertGenericToDxChTrillTtiRule function
* @endinternal
*
* @brief   Convert generic TRILL TTI rule into device specific TRILL TTI rule
*
* @param[in] trillTtiRulePtr          - (pointer to) TRILL TTI rule
*
* @param[out] dxChTrillTtiRulePtr      - (pointer to) DxCh TRILL TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChTrillTtiRule
(
    IN  PRV_TGF_TTI_TRILL_RULE_STC       *trillTtiRulePtr,
    OUT CPSS_DXCH_TTI_TRILL_RULE_STC     *dxChTrillTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) dxChTrillTtiRulePtr, 0, sizeof(*dxChTrillTtiRulePtr));

    /* convert TRILL tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), dsaSrcPortTrunk);
    dxChTrillTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(trillTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillMBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillEgressRbid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillIngressRbid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketMacDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketTag0Exists);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketTag0Vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketFieldsAreValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillMcDescriptorInstance);

    /* set MAC */
    cpssOsMemCpy(dxChTrillTtiRulePtr->common.mac.arEther,
                 trillTtiRulePtr->common.mac.arEther,
                 sizeof(dxChTrillTtiRulePtr->common.mac.arEther));
}
#endif /*0*/
/**
* @internal prvTgfConvertDxChToGenericTrillTtiRule function
* @endinternal
*
* @brief   Convert device specific TRILL TTI rule into generic TRILL TTI rule
*
* @param[in] dxChTrillTtiRulePtr      - (pointer to) DxCh TRILL TTI rule
*
* @param[out] trillTtiRulePtr          - (pointer to) TRILL TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericTrillTtiRule
(
    IN  CPSS_DXCH_TTI_TRILL_RULE_STC     *dxChTrillTtiRulePtr,
    OUT PRV_TGF_TTI_TRILL_RULE_STC       *trillTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) trillTtiRulePtr, 0, sizeof(*trillTtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChTrillTtiRulePtr->common), &(trillTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillMBit);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillEgressRbid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillIngressRbid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketMacDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketTag0Exists);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketTag0Vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, innerPacketFieldsAreValid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillTtiRulePtr, trillTtiRulePtr, trillMcDescriptorInstance);

    /* set MAC */
    cpssOsMemCpy(trillTtiRulePtr->common.mac.arEther,
                 dxChTrillTtiRulePtr->common.mac.arEther,
                 sizeof(trillTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartIpv4Entry function
* @endinternal
*
* @brief   Convert generic tunnel start IPv4 entry into device specific
*
* @param[in] ipv4EntryPtr             - (pointer to) tunnel start IPv4 entry
*
* @param[out] dxChIpv4EntryPtr         - (pointer to) DxCh tunnel start IPv4 entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartIpv4Entry
(
    IN  PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC       *ipv4EntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC    *dxChIpv4EntryPtr
)
{
    /* clear tunnel start IPv4 entry */
    cpssOsMemSet((GT_VOID*) dxChIpv4EntryPtr, 0, sizeof(*dxChIpv4EntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChIpv4EntryPtr->upMarkMode, ipv4EntryPtr->upMarkMode);

    /* convert dscpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChIpv4EntryPtr->dscpMarkMode, ipv4EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv4 entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, dscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, dontFragmentFlag);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, autoTunnel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, autoTunnelOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, destIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, srcIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, cfi);
    dxChIpv4EntryPtr->retainCRC =  ipv4EntryPtr->retainCrc;
    switch(ipv4EntryPtr->ipHeaderProtocol)
    {
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:
        dxChIpv4EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        break;
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:
        dxChIpv4EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        break;
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:
        dxChIpv4EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        break;
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:
        dxChIpv4EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, ipProtocol);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, profileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, greProtocolForEthernet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, greFlagsAndVersion);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, udpDstPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, udpSrcPort);
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartIpv4Entry function
* @endinternal
*
* @brief   Convert device specific tunnel start IPv4 entry into generic
*
* @param[in] dxChIpv4EntryPtr         - (pointer to) DxCh tunnel start IPv4 entry
*
* @param[out] ipv4EntryPtr             - (pointer to) tunnel start IPv4 entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartIpv4Entry
(
    IN  CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC    *dxChIpv4EntryPtr,
    OUT PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC       *ipv4EntryPtr
)
{
    /* clear tunnel start IPv4 entry */
    cpssOsMemSet((GT_VOID*) ipv4EntryPtr, 0, sizeof(*ipv4EntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChIpv4EntryPtr->upMarkMode, ipv4EntryPtr->upMarkMode);

    /* convert dscpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChIpv4EntryPtr->dscpMarkMode, ipv4EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv4 entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, dscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, dontFragmentFlag);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, autoTunnel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, autoTunnelOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, destIp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, srcIp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, cfi);
    ipv4EntryPtr->retainCrc = dxChIpv4EntryPtr->retainCRC;
    switch(dxChIpv4EntryPtr->ipHeaderProtocol)
    {
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:
        ipv4EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:
        ipv4EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:
        ipv4EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:
        ipv4EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
        break;
    default:
        return GT_BAD_STATE;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, profileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, greProtocolForEthernet);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, greFlagsAndVersion);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, udpDstPort);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, udpSrcPort);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv4EntryPtr, ipv4EntryPtr, ipProtocol);
    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartIpv6Entry function
* @endinternal
*
* @brief   Convert generic tunnel start IPv4 entry into device specific
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartIpv6Entry
(
    IN  PRV_TGF_TUNNEL_START_IPV6_ENTRY_STC       *ipv6EntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC    *dxChIpv6EntryPtr
)
{
    /* clear tunnel start IPv4 entry */
    cpssOsMemSet((GT_VOID*) dxChIpv6EntryPtr, 0, sizeof(*dxChIpv6EntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChIpv6EntryPtr->upMarkMode, ipv6EntryPtr->upMarkMode);

    /* convert dscpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChIpv6EntryPtr->dscpMarkMode, ipv6EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv4 entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, dscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, destIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, srcIp);
    dxChIpv6EntryPtr->retainCRC =  ipv6EntryPtr->retainCrc;
    switch(ipv6EntryPtr->ipHeaderProtocol)
    {
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:
        dxChIpv6EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        break;
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:
        dxChIpv6EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        break;
    case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:
        dxChIpv6EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        break;
   case PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:
        dxChIpv6EntryPtr->ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, ipProtocol);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, profileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, greProtocolForEthernet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, greFlagsAndVersion);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, udpDstPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, udpSrcPort);
    switch(ipv6EntryPtr->flowLabelMode)
    {
    case PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E:
        dxChIpv6EntryPtr->flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E;
        break;
    case PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E:
        dxChIpv6EntryPtr->flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartIpv6Entry function
* @endinternal
*
* @brief   Convert device specific tunnel start IPv6 entry into generic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartIpv6Entry
(
    IN  CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC    *dxChIpv6EntryPtr,
    OUT PRV_TGF_TUNNEL_START_IPV6_ENTRY_STC       *ipv6EntryPtr
)
{
    /* clear tunnel start IPv6 entry */
    cpssOsMemSet((GT_VOID*) ipv6EntryPtr, 0, sizeof(*ipv6EntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChIpv6EntryPtr->upMarkMode, ipv6EntryPtr->upMarkMode);

    /* convert dscpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChIpv6EntryPtr->dscpMarkMode, ipv6EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv6 entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, dscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, destIp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, srcIp);
    ipv6EntryPtr->retainCrc = dxChIpv6EntryPtr->retainCRC;
    switch(dxChIpv6EntryPtr->ipHeaderProtocol)
    {
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:
        ipv6EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:
        ipv6EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:
        ipv6EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        break;
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:
        ipv6EntryPtr->ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
        break;
    default:
        return GT_BAD_STATE;
    }
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, ipProtocol);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, profileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, greProtocolForEthernet);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, greFlagsAndVersion);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, udpDstPort);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpv6EntryPtr, ipv6EntryPtr, udpSrcPort);

    switch(dxChIpv6EntryPtr->flowLabelMode)
    {
    case CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E:
        ipv6EntryPtr->flowLabelMode = PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E;
        break;
    case CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E:
        ipv6EntryPtr->flowLabelMode = PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartProfileEntry function
* @endinternal
*
* @brief   Convert generic tunnel start profile entry into device specific
*
* @param[in] profileEntryPtr          - (pointer to) tunnel start profile entry
*
* @param[out] dxChProfileEntryPtr      - (pointer to) DxCh tunnel start profile entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartProfileEntry
(
    IN  PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC       *profileEntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC    *dxChProfileEntryPtr
)
{
    /* clear tunnel start profile entry */
    cpssOsMemSet((GT_VOID*) dxChProfileEntryPtr, 0, sizeof(*dxChProfileEntryPtr));

    /* convert tunnel start generic profile entry into device specific format */
    switch(profileEntryPtr->templateDataSize)
    {
    case PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E:
        dxChProfileEntryPtr->templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;
        break;
    case PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E:
        dxChProfileEntryPtr->templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E;
        break;
    case PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E:
        dxChProfileEntryPtr->templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E;
        break;
    case PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E:
        dxChProfileEntryPtr->templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E;
        break;
    case PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E:
        dxChProfileEntryPtr->templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    cpssOsMemCpy(dxChProfileEntryPtr->templateDataBitsCfg,profileEntryPtr->templateDataBitsCfg,sizeof(profileEntryPtr->templateDataBitsCfg));
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChProfileEntryPtr, profileEntryPtr, hashShiftLeftBitsNumber);
    switch(profileEntryPtr->udpSrcPortMode)
    {
    case PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E:
        dxChProfileEntryPtr->udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
        break;
    case PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E:
        dxChProfileEntryPtr->udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChProfileEntryPtr, profileEntryPtr, serviceIdCircularShiftSize);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChProfileEntryPtr, profileEntryPtr, macDaMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChProfileEntryPtr, profileEntryPtr, dipMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChProfileEntryPtr, profileEntryPtr, controlWordIndex);
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartProfileEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start profile entry into generic
*
* @param[in] dxChProfileEntryPtr      - (pointer to) DxCh tunnel start profile entry
*
* @param[out] profileEntryPtr          - (pointer to) tunnel start profile entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartProfileEntry
(
    IN  CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC    *dxChProfileEntryPtr,
    OUT PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC       *profileEntryPtr
)
{
    /* clear tunnel start profile entry */
    cpssOsMemSet((GT_VOID*) profileEntryPtr, 0, sizeof(*profileEntryPtr));
    /* convert tunnel start generic profile entry into device specific format */
    switch(dxChProfileEntryPtr->templateDataSize)
    {
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E:
        profileEntryPtr->templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;
        break;
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E:
        profileEntryPtr->templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E;
        break;
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E:
        profileEntryPtr->templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E;
        break;
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E:
        profileEntryPtr->templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E;
        break;
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E:
        profileEntryPtr->templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
        break;
    default:
        return GT_BAD_STATE;
    }
    cpssOsMemCpy(profileEntryPtr->templateDataBitsCfg,dxChProfileEntryPtr->templateDataBitsCfg,sizeof(dxChProfileEntryPtr->templateDataBitsCfg));
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(profileEntryPtr,dxChProfileEntryPtr,hashShiftLeftBitsNumber);
    switch(dxChProfileEntryPtr->udpSrcPortMode)
    {
    case CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E:
        profileEntryPtr->udpSrcPortMode = PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
        break;
    case CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E:
        profileEntryPtr->udpSrcPortMode = PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(profileEntryPtr,dxChProfileEntryPtr,serviceIdCircularShiftSize);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(profileEntryPtr,dxChProfileEntryPtr,macDaMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(profileEntryPtr,dxChProfileEntryPtr,dipMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(profileEntryPtr,dxChProfileEntryPtr,controlWordIndex);
    return GT_OK;
}



/**
* @internal prvTgfConvertGenericToDxChTunnelStartMplsEntry function
* @endinternal
*
* @brief   Convert generic tunnel start Mpls entry into device specific
*
* @param[in] mplsEntryPtr             - (pointer to) tunnel start Mpls entry
*
* @param[out] dxChMplsEntryPtr         - (pointer to) DxCh tunnel start Mpls entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartMplsEntry
(
    IN  GT_U8                                     devNum,
    IN  PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC       *mplsEntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC    *dxChMplsEntryPtr
)
{
    /* clear tunnel start Mpls entry */
    cpssOsMemSet((GT_VOID*) dxChMplsEntryPtr, 0, sizeof(*dxChMplsEntryPtr));

    /* convert MarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->upMarkMode,   mplsEntryPtr->upMarkMode);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp1MarkMode, mplsEntryPtr->exp1MarkMode);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp2MarkMode, mplsEntryPtr->exp2MarkMode);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp3MarkMode, mplsEntryPtr->exp3MarkMode);

    /* convert ttlMode into device specific format */
    switch (mplsEntryPtr->ttlMode)
    {
        case PRV_TGF_TUNNEL_START_TTL_TO_INCOMING_TTL_E:
            dxChMplsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
            break;

        case PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_E:
            dxChMplsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E;
            break;

        case PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E:
            dxChMplsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E;
            break;
        case PRV_TGF_TUNNEL_START_TTL_TO_POP_OUTER_E:
            if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                return GT_BAD_PARAM;
            }
            dxChMplsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start Mpls entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, numLabels);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label3);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp3);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, retainCRC);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, setSBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, cfi);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, controlWordEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, controlWordIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, mplsEthertypeSelect);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartMplsEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start Mpls entry into generic
*
* @param[in] dxChMplsEntryPtr         - (pointer to) DxCh tunnel start Mpls entry
*
* @param[out] mplsEntryPtr             - (pointer to) tunnel start Mpls entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartMplsEntry
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC    *dxChMplsEntryPtr,
    OUT PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC       *mplsEntryPtr
)
{
    /* clear tunnel start Mpls entry */
    cpssOsMemSet((GT_VOID*) mplsEntryPtr, 0, sizeof(*mplsEntryPtr));

    /* convert MarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->upMarkMode,   mplsEntryPtr->upMarkMode);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp1MarkMode, mplsEntryPtr->exp1MarkMode);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp2MarkMode, mplsEntryPtr->exp2MarkMode);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMplsEntryPtr->exp3MarkMode, mplsEntryPtr->exp3MarkMode);

    /* convert ttlMode from device specific format */
    switch (dxChMplsEntryPtr->ttlMode)
    {
        case CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E:
            mplsEntryPtr->ttlMode = PRV_TGF_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
            break;

        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E:
            mplsEntryPtr->ttlMode = PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_E;
            break;

        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E:
            mplsEntryPtr->ttlMode = PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E;
            break;

        case CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E:
            if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                return GT_BAD_PARAM;
            }
            mplsEntryPtr->ttlMode = PRV_TGF_TUNNEL_START_TTL_TO_POP_OUTER_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start Mpls entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, numLabels);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, label3);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, exp3);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, retainCRC);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, setSBit);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, cfi);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, controlWordEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, controlWordIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMplsEntryPtr, mplsEntryPtr, mplsEthertypeSelect);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartMimEntry function
* @endinternal
*
* @brief   Convert generic tunnel start Mim entry into device specific
*
* @param[in] mimEntryPtr              - (pointer to) tunnel start Mim entry
*
* @param[out] dxChMimEntryPtr          - (pointer to) DxCh tunnel start Mim entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartMimEntry
(
    IN  PRV_TGF_TUNNEL_START_MIM_ENTRY_STC      *mimEntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC   *dxChMimEntryPtr
)
{
    /* clear tunnel start Mim entry */
    cpssOsMemSet((GT_VOID*) dxChMimEntryPtr, 0, sizeof(*dxChMimEntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->upMarkMode, mimEntryPtr->upMarkMode);

    /* convert iUpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->iUpMarkMode, mimEntryPtr->iUpMarkMode);

    /* convert iDpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->iDpMarkMode, mimEntryPtr->iDpMarkMode);

    /* convert i-sid asignment mode into device specific format */
    PRV_TGF_S2D_MIM_ISID_MODE_CONVERT_MAC(dxChMimEntryPtr->iSidAssignMode, mimEntryPtr->mimISidAssignMode);

    /* convert BDA asignment mode into device specific format */
    PRV_TGF_S2D_MIM_BDA_MODE_CONVERT_MAC(dxChMimEntryPtr->bDaAssignMode, mimEntryPtr->mimBDaAssignMode);

    /* convert BSA asignment mode into device specific format */
    PRV_TGF_S2D_MIM_BSA_MODE_CONVERT_MAC(dxChMimEntryPtr->bSaAssignMode, mimEntryPtr->mimBSaAssignMode);

    /* convert tunnel start Mim entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, retainCrc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iSid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iTagReserved);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartMimEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start Mim entry into generic
*
* @param[in] dxChMimEntryPtr          - (pointer to) DxCh tunnel start Mim entry
*
* @param[out] mimEntryPtr              - (pointer to) tunnel start Mim entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartMimEntry
(
    IN  CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC   *dxChMimEntryPtr,
    OUT PRV_TGF_TUNNEL_START_MIM_ENTRY_STC      *mimEntryPtr
)
{
    /* clear tunnel start Mim entry */
    cpssOsMemSet((GT_VOID*) mimEntryPtr, 0, sizeof(*mimEntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->upMarkMode, mimEntryPtr->upMarkMode);

    /* convert iUpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->iUpMarkMode, mimEntryPtr->iUpMarkMode);

    /* convert iDpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChMimEntryPtr->iDpMarkMode, mimEntryPtr->iDpMarkMode);

    /* convert i-sid asignment mode from device specific format */
    PRV_TGF_D2S_MIM_ISID_MODE_CONVERT_MAC(dxChMimEntryPtr->iSidAssignMode, mimEntryPtr->mimISidAssignMode);

    /* convert BDA asignment mode from device specific format */
    PRV_TGF_D2S_MIM_BDA_MODE_CONVERT_MAC(dxChMimEntryPtr->bDaAssignMode, mimEntryPtr->mimBDaAssignMode);

    /* convert tunnel start Mim entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, retainCrc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iSid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChMimEntryPtr, mimEntryPtr, iTagReserved);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartTrillEntry function
* @endinternal
*
* @brief   Convert generic tunnel start TRILL entry into device specific
*
* @param[in] trillEntryPtr            - (pointer to) tunnel start TRILL entry
*
* @param[out] dxChTrillEntryPtr        - (pointer to) DxCh tunnel start TRILL entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartTrillEntry
(
    IN  PRV_TGF_TUNNEL_START_TRILL_ENTRY_STC      *trillEntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC   *dxChTrillEntryPtr
)
{
    /* clear tunnel start TRILL entry */
    cpssOsMemSet((GT_VOID*) dxChTrillEntryPtr, 0, sizeof(*dxChTrillEntryPtr));

    /* convert tunnel start TRILL entry into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChTrillEntryPtr->upMarkMode, trillEntryPtr->upMarkMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, nextHopMacDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.version);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.mBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.opLength);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.hopCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.eRbid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.iRbid);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartTrillEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start TRILL entry into generic
*
* @param[in] dxChTrillEntryPtr        - (pointer to) DxCh tunnel start TRILL entry
*
* @param[out] trillEntryPtr            - (pointer to) tunnel start TRILL entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartTrillEntry
(
    IN  CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC   *dxChTrillEntryPtr,
    OUT PRV_TGF_TUNNEL_START_TRILL_ENTRY_STC      *trillEntryPtr
)
{
    /* clear tunnel start TRILL entry */
    cpssOsMemSet((GT_VOID*) trillEntryPtr, 0, sizeof(*trillEntryPtr));

    /* convert tunnel start TRILL entry from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChTrillEntryPtr->upMarkMode, trillEntryPtr->upMarkMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, nextHopMacDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.version);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.mBit);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.opLength);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.hopCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.eRbid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTrillEntryPtr, trillEntryPtr, trillHeader.iRbid);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTunnelStartGenericEntry function
* @endinternal
*
* @brief   Convert generic tunnel start generic entry into device specific
*
* @param[in] genEntryPtr            - (pointer to) tunnel start generic entry
*
* @param[out] dxChGenEntryPtr       - (pointer to) DxCh tunnel start generic entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTunnelStartGenericEntry
(
    IN  PRV_TGF_TUNNEL_START_GENERIC_ENTRY_STC      *genEntryPtr,
    OUT CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC   *dxChGenEntryPtr
)
{
    /* clear tunnel start TRILL entry */
    cpssOsMemSet((GT_VOID*) dxChGenEntryPtr, 0, sizeof(*dxChGenEntryPtr));

    /* convert tunnel start TRILL entry into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(dxChGenEntryPtr->upMarkMode, genEntryPtr->upMarkMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, genericType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, retainCrc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, etherType);
    cpssOsMemCpy(dxChGenEntryPtr->data, genEntryPtr->data, 36);
    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTunnelStartGenericEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start TRILL entry into generic
*
* @param[in] dxChGenEntryPtr        - (pointer to) DxCh tunnel start generic entry
*
* @param[out] genEntryPtr           - (pointer to) tunnel start generic entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTunnelStartGenericEntry
(
    IN  CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC   *dxChGenEntryPtr,
    OUT PRV_TGF_TUNNEL_START_GENERIC_ENTRY_STC      *genEntryPtr
)
{
    /* clear tunnel start TRILL entry */
    cpssOsMemSet((GT_VOID*) genEntryPtr, 0, sizeof(*genEntryPtr));

    /* convert tunnel start TRILL entry from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(dxChGenEntryPtr->upMarkMode, genEntryPtr->upMarkMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, genericType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, retainCrc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChGenEntryPtr, genEntryPtr, etherType);
    cpssOsMemCpy(genEntryPtr->data, dxChGenEntryPtr->data, 36);

    return GT_OK;
}

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
/*the TTI unit uses PCLD-ID as 'bit 0 is indication of "regular port / LBP (loopback port)"
    so we need to use only 9 MSBits of the PCL-ID for TTI rules ! */
#define EXMXPM_TTI_PCL_ID_SHIFT_MAC(_pcliId) (_pcliId << 1)
GT_STATUS   exMxPmTtiInit(IN GT_U8 dev)
{
    GT_STATUS rc;
    GT_U8  dummyPort = 0;/* the PCL_ID is global for the device (not per port) */
    CPSS_EXMXPM_TTI_LOOKUP_CONFIG_STC ttiLookupConfigInfo;/* info for TTI lookups */
    static GT_U32   firstTime = 1;

    if(firstTime == 0)
    {
        return GT_OK;
    }

    firstTime = 0;

    /*********************************************************/
    /* Default TTI configuration :                           */
    /*   - Set TTI PCL ID for IPV4 lookup to 1               */
    /*   - Set TTI PCL ID for MPLS lookup to 2               */
    /*   - Set TTI PCL ID for ETH  lookup to 3               */
    /*   - Set TTI PCL ID for MIM  lookup to 4               */
    /*********************************************************/
    /* but the PCL_ID bit 0 is indication that the packet came from 'LOOPBACK PORT' */
    /* so we actually need this bit to be always 0           */
    /* meaning we need to use 9 MSBits of PCL-ID             */
    /*   - Set TTI PCL ID for IPV4 lookup to 1<<1            */
    /*   - Set TTI PCL ID for MPLS lookup to 2<<1            */
    /*   - Set TTI PCL ID for ETH  lookup to 3<<1            */
    /*   - Set TTI PCL ID for MIM  lookup to 4<<1            */
    /*********************************************************/

    ttiLookupConfigInfo.actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    ttiLookupConfigInfo.pclId = EXMXPM_TTI_PCL_ID_SHIFT_MAC(1);
    rc = cpssExMxPmTtiPortLookupConfigSet(dev,dummyPort,CPSS_EXMXPM_TTI_KEY_IPV4_E,&ttiLookupConfigInfo);
    if (rc != GT_OK)
       return rc;
    ttiLookupConfigInfo.pclId = EXMXPM_TTI_PCL_ID_SHIFT_MAC(2);
    rc = cpssExMxPmTtiPortLookupConfigSet(dev,dummyPort,CPSS_EXMXPM_TTI_KEY_MPLS_E,&ttiLookupConfigInfo);
    if (rc != GT_OK)
       return rc;
    ttiLookupConfigInfo.pclId = EXMXPM_TTI_PCL_ID_SHIFT_MAC(3);
    rc = cpssExMxPmTtiPortLookupConfigSet(dev,dummyPort,CPSS_EXMXPM_TTI_KEY_ETH_E,&ttiLookupConfigInfo);
    if (rc != GT_OK)
       return rc;
    ttiLookupConfigInfo.pclId = EXMXPM_TTI_PCL_ID_SHIFT_MAC(4);
    rc = cpssExMxPmTtiPortLookupConfigSet(dev,dummyPort,CPSS_EXMXPM_TTI_KEY_MIM_E,&ttiLookupConfigInfo);
    if (rc != GT_OK)
       return rc;

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmTunnelStartQInQEntry function
* @endinternal
*
* @brief   Convert generic tunnel start QInQ entry into device specific
*
* @param[in] qinqEntryPtr             - (pointer to) tunnel start QInQ entry
*
* @param[out] exMxPmQInQEntryPtr       - (pointer to) ExMxPm tunnel start QInQ entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTunnelStartQInQEntry
(
    IN  PRV_TGF_TUNNEL_START_QINQ_ENTRY_STC       *qinqEntryPtr,
    OUT CPSS_EXMXPM_TUNNEL_START_QINQ_ENTRY_STC   *exMxPmQInQEntryPtr
)
{
    /* clear tunnel start QInQ entry */
    cpssOsMemSet((GT_VOID*) exMxPmQInQEntryPtr, 0, sizeof(*exMxPmQInQEntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->upMarkMode, qinqEntryPtr->upMarkMode);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->cTagUpMarkMode, qinqEntryPtr->cTagUpMarkMode);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->cTagCommand, qinqEntryPtr->cTagCommand);
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->sTagCommand, qinqEntryPtr->sTagCommand);

    /* convert ethType into device specific format */
    switch (qinqEntryPtr->mplsEthType)
    {
        case PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:
            exMxPmQInQEntryPtr->mplsEthType = CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
            break;

        case PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:
            exMxPmQInQEntryPtr->mplsEthType = CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start QInQ entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, sTagEthType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagEthType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagVid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagUp);

    exMxPmQInQEntryPtr->retainCrc = qinqEntryPtr->retainCrc;
    exMxPmQInQEntryPtr->sTagCfi = qinqEntryPtr->sTagCfi;
    exMxPmQInQEntryPtr->cTagCfi = qinqEntryPtr->cTagCfi;

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTunnelStartQInQEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start QInQ entry into generic
*
* @param[in] exMxPmQInQEntryPtr       - (pointer to) ExMxPm tunnel start QInQ entry
*
* @param[out] qinqEntryPtr             - (pointer to) tunnel start QInQ entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTunnelStartQInQEntry
(
    IN  CPSS_EXMXPM_TUNNEL_START_QINQ_ENTRY_STC   *exMxPmQInQEntryPtr,
    OUT PRV_TGF_TUNNEL_START_QINQ_ENTRY_STC       *qinqEntryPtr
)
{
    /* clear tunnel start Mpls entry */
    cpssOsMemSet((GT_VOID*) qinqEntryPtr, 0, sizeof(*qinqEntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->upMarkMode, qinqEntryPtr->upMarkMode);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->cTagUpMarkMode, qinqEntryPtr->cTagUpMarkMode);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->cTagCommand, qinqEntryPtr->cTagCommand);
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmQInQEntryPtr->sTagCommand, qinqEntryPtr->sTagCommand);

    /* convert ethType into device specific format */
    switch (exMxPmQInQEntryPtr->mplsEthType)
    {
        case CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:
             qinqEntryPtr->mplsEthType = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:
            qinqEntryPtr->mplsEthType = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start QInQ entry into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, sTagEthType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagEthType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagVid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmQInQEntryPtr, qinqEntryPtr, cTagUp);

    qinqEntryPtr->retainCrc=exMxPmQInQEntryPtr->retainCrc;
    qinqEntryPtr->sTagCfi=exMxPmQInQEntryPtr->sTagCfi;
    qinqEntryPtr->cTagCfi=exMxPmQInQEntryPtr->cTagCfi;


    return GT_OK;
}


/**
* @internal prvTgfConvertGenericToExMxPmTtiAction function
* @endinternal
*
* @brief   Convert generic TTI action into device specific TTI action
*
* @param[in] actionPtr                - (pointer to) TTI rule action
*
* @param[out] exMxPmTtiActionPtr       - (pointer to) ExMxPm standert TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTtiAction
(
    IN  PRV_TGF_TTI_ACTION_STC                  *actionPtr,
    OUT CPSS_EXMXPM_TTI_STANDARD_ACTION_STC     *exMxPmTtiActionPtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmTtiActionPtr, 0, sizeof(*exMxPmTtiActionPtr));

    /* convert tti action into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, command);

    /* set redirect command */
    switch (actionPtr->redirectCommand)
    {
        case PRV_TGF_TTI_NO_REDIRECT_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_NO_REDIRECT_E;
            break;
        case PRV_TGF_TTI_REDIRECT_TO_EGRESS_E:
        case PRV_TGF_TTI_REDIRECT_TO_OUTLIF_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_REDIRECT_TO_OUTLIF_E;
            break;
        case PRV_TGF_TTI_REDIRECT_TO_NEXT_HOP_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_REDIRECT_TO_NEXT_HOP_E;
            break;
        case PRV_TGF_TTI_TRIGGER_VPLS_LOOKUP_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_TRIGGER_VPLS_LOOKUP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* set outlif info */
    cpssOsMemCpy((GT_VOID*) &(exMxPmTtiActionPtr->outlif.interfaceInfo),
                 (GT_VOID*) &(actionPtr-> interfaceInfo),
                 sizeof(actionPtr->interfaceInfo));

    if(actionPtr->tunnelStart==GT_FALSE)
    {
        exMxPmTtiActionPtr->outlif.outlifType           = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
        exMxPmTtiActionPtr->outlif.outlifPointer.arpPtr = actionPtr->arpPtr;

        /* need to add support for CPSS_EXMXPM_OUTLIF_TYPE_DIT_E */
    }
    else
    {
        exMxPmTtiActionPtr->outlif.outlifType           = CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E;
        exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.ptr = actionPtr->tunnelStartPtr;
        switch(actionPtr->tsPassengerPacketType)
        {
            case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
                exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.passengerPacketType = CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                break;
            case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
                exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.passengerPacketType = CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    /* set vlanCmd */
    switch (actionPtr->vlanCmd)
    {
        case PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_DO_NOT_MODIFY_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_TAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_ALL_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    exMxPmTtiActionPtr->inLifVidId = actionPtr->vlanId;

    /* convert mplsCmd into device specific format */
    switch (actionPtr->mplsCmd)
    {
        case PRV_TGF_TTI_MPLS_NOP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_NOP_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_SWAP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerPacketType into device specific format */
    switch (actionPtr->passengerPacketType)
    {
        case PRV_TGF_TTI_PASSENGER_IPV4_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_IPV4_E;
            break;

        case PRV_TGF_TTI_PASSENGER_IPV6_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_IPV6_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pwIdMode into device specific format */
    switch (actionPtr->pwIdMode)
    {
        case PRV_TGF_TTI_PW_ID_MODE_ENTRY_E:
            exMxPmTtiActionPtr->pwIdMode = CPSS_EXMXPM_TTI_PW_ID_MODE_ENTRY_E;
            break;

        case PRV_TGF_TTI_PW_ID_MODE_BASE_E:
            exMxPmTtiActionPtr->pwIdMode = CPSS_EXMXPM_TTI_PW_ID_MODE_BASE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, pwIdMode);

    /* convert tti action into device specific format */
    exMxPmTtiActionPtr->tunnelTerminateEnable = actionPtr->tunnelTerminate;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, copyTtlFromTunnelHeader);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, vlanPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, nestedVlanEnable);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, exp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, setExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableDecTtl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mplsLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, pwId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sourceIsPE);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableSourceLocalFiltering);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, floodDitPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, baseMplsLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, counterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, meterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstIdEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstId);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmTtiAction2 function
* @endinternal
*
* @brief   Convert generic TTI action type2 into device specific TTI action
*
* @param[in] actionPtr                - (pointer to) TTI rule action type2
*
* @param[out] exMxPmTtiActionPtr       - (pointer to) ExMxPm standert TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTtiAction2
(
    IN  PRV_TGF_TTI_ACTION_2_STC                *actionPtr,
    OUT CPSS_EXMXPM_TTI_STANDARD_ACTION_STC     *exMxPmTtiActionPtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmTtiActionPtr, 0, sizeof(*exMxPmTtiActionPtr));

    /* convert tti action into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, command);

    /* set redirect command */
    switch (actionPtr->redirectCommand)
    {
        case PRV_TGF_TTI_NO_REDIRECT_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_NO_REDIRECT_E;
            break;
        case PRV_TGF_TTI_REDIRECT_TO_EGRESS_E:
        case PRV_TGF_TTI_REDIRECT_TO_OUTLIF_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_REDIRECT_TO_OUTLIF_E;
            break;
        case PRV_TGF_TTI_REDIRECT_TO_NEXT_HOP_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_REDIRECT_TO_NEXT_HOP_E;
            break;
        case PRV_TGF_TTI_TRIGGER_VPLS_LOOKUP_E:
            exMxPmTtiActionPtr->redirectCommand = CPSS_EXMXPM_TTI_TRIGGER_VPLS_LOOKUP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* set outlif info */
    cpssOsMemCpy((GT_VOID*) &(exMxPmTtiActionPtr->outlif.interfaceInfo),
                 (GT_VOID*) &(actionPtr->egressInterface),
                 sizeof(actionPtr->egressInterface));

    if(actionPtr->tunnelStart==GT_FALSE)
    {
       exMxPmTtiActionPtr->outlif.outlifType           = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
       exMxPmTtiActionPtr->outlif.outlifPointer.arpPtr = actionPtr->arpPtr;

        /* need to add support for CPSS_EXMXPM_OUTLIF_TYPE_DIT_E */
    }
    else
    {
        exMxPmTtiActionPtr->outlif.outlifType           = CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E;
        exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.ptr = actionPtr->tunnelStartPtr;
        switch(actionPtr->tsPassengerPacketType)
        {
            case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
                exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.passengerPacketType = CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                break;
            case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
                exMxPmTtiActionPtr->outlif.outlifPointer.tunnelStartPtr.passengerPacketType = CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    /* set vlanCmd */
    switch (actionPtr->tag0VlanCmd)
    {
        case PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_DO_NOT_MODIFY_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_UNTAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_TAGGED_E;
            break;

        case PRV_TGF_TTI_VLAN_MODIFY_ALL_E:
            exMxPmTtiActionPtr->inLifVidCmd = CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    exMxPmTtiActionPtr->inLifVidId = actionPtr->tag0VlanId;

    /* convert mplsCmd into device specific format */
    switch (actionPtr->mplsCommand)
    {
        case PRV_TGF_TTI_MPLS_NOP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_NOP_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_SWAP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            exMxPmTtiActionPtr->mplsCmd = CPSS_EXMXPM_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerPacketType into device specific format */
    switch (actionPtr->ttPassengerPacketType)
    {
        case PRV_TGF_TTI_PASSENGER_IPV4_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_IPV4_E;
            break;

        case PRV_TGF_TTI_PASSENGER_IPV6_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_IPV6_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            exMxPmTtiActionPtr->passengerPacketType = CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pwIdMode into device specific format */
    switch (actionPtr->pwIdMode)
    {
        case PRV_TGF_TTI_PW_ID_MODE_ENTRY_E:
            exMxPmTtiActionPtr->pwIdMode = CPSS_EXMXPM_TTI_PW_ID_MODE_ENTRY_E;
            break;

        case PRV_TGF_TTI_PW_ID_MODE_BASE_E:
            exMxPmTtiActionPtr->pwIdMode = CPSS_EXMXPM_TTI_PW_ID_MODE_BASE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti action into device specific format */
    exMxPmTtiActionPtr->tunnelTerminateEnable = actionPtr->tunnelTerminate;
    exMxPmTtiActionPtr->vlanPrecedence = actionPtr->tag0VlanPrecedence;
    exMxPmTtiActionPtr->enableDecTtl = actionPtr->enableDecrementTtl;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, userDefinedCpuCode);
    exMxPmTtiActionPtr->copyTtlFromTunnelHeader = actionPtr->copyTtlExpFromTunnelHeader;
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, nestedVlanEnable);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, exp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, setExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mplsLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, pwId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sourceIsPE);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableSourceLocalFiltering);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, floodDitPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, baseMplsLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, counterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, meterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstIdEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstId);

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTtiAction function
* @endinternal
*
* @brief   Convert device specific TTI action into generic TTI action
*
* @param[in] exMxPmTtiActionPtr       - (pointer to) ExMxPm standert TTI rule action
*
* @param[out] actionPtr                - (pointer to) TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTtiAction
(
    IN  CPSS_EXMXPM_TTI_STANDARD_ACTION_STC     *exMxPmTtiActionPtr,
    OUT PRV_TGF_TTI_ACTION_STC                  *actionPtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) actionPtr, 0, sizeof(*actionPtr));

    /* convert tti action from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, command);

    /* set redirect command */
    switch (exMxPmTtiActionPtr->redirectCommand)
    {
        case CPSS_EXMXPM_TTI_NO_REDIRECT_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
            break;
        case CPSS_EXMXPM_TTI_REDIRECT_TO_OUTLIF_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_OUTLIF_E;
            break;
        case CPSS_EXMXPM_TTI_REDIRECT_TO_NEXT_HOP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_NEXT_HOP_E;
            break;
        case CPSS_EXMXPM_TTI_TRIGGER_VPLS_LOOKUP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_TRIGGER_VPLS_LOOKUP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* set outlif info */
    cpssOsMemCpy((GT_VOID*) &(actionPtr->interfaceInfo),
                 (GT_VOID*) &(exMxPmTtiActionPtr->outlif.interfaceInfo),
                 sizeof(exMxPmTtiActionPtr->outlif.interfaceInfo));

    actionPtr->arpPtr = exMxPmTtiActionPtr->outlif.outlifPointer.arpPtr;

    /* set vlanCmd */
    switch (exMxPmTtiActionPtr->inLifVidCmd)
    {
        case CPSS_EXMXPM_TTI_VLAN_INLIF_DO_NOT_MODIFY_E:
            actionPtr->vlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_UNTAGGED_E:
            actionPtr->vlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_TAGGED_E:
            actionPtr->vlanCmd = PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_ALL_E:
            actionPtr->vlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    actionPtr->vlanId = exMxPmTtiActionPtr->inLifVidId;

    /* convert mplsCmd from device specific format */
    switch (exMxPmTtiActionPtr->mplsCmd)
    {
        case CPSS_EXMXPM_TTI_MPLS_NOP_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_SWAP_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP1_LABEL_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP2_LABELS_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP3_LABELS_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            actionPtr->mplsCmd = PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerPacketType from device specific format */
    switch (exMxPmTtiActionPtr->passengerPacketType)
    {
        case CPSS_EXMXPM_TTI_PASSENGER_IPV4_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_IPV6_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV6_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_CRC_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            actionPtr->passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pwIdMode from device specific format */
    switch (exMxPmTtiActionPtr->pwIdMode)
    {
        case CPSS_EXMXPM_TTI_PW_ID_MODE_ENTRY_E:
            actionPtr->pwIdMode = PRV_TGF_TTI_PW_ID_MODE_ENTRY_E;
            break;

        case CPSS_EXMXPM_TTI_PW_ID_MODE_BASE_E:
            actionPtr->pwIdMode = PRV_TGF_TTI_PW_ID_MODE_BASE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti action from device specific format */
    actionPtr->tunnelTerminate = exMxPmTtiActionPtr->tunnelTerminateEnable;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, copyTtlFromTunnelHeader);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, vlanPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, nestedVlanEnable);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, exp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, setExp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableDecTtl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mplsLabel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, pwId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sourceIsPE);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableSourceLocalFiltering);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, floodDitPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, baseMplsLabel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, counterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, meterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstIdEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstId);

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTtiAction2 function
* @endinternal
*
* @brief   Convert device specific TTI action type2 into generic TTI action
*
* @param[in] exMxPmTtiActionPtr       - (pointer to) ExMxPm standert TTI rule action
*
* @param[out] actionPtr                - (pointer to) TTI rule action type2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTtiAction2
(
    IN  CPSS_EXMXPM_TTI_STANDARD_ACTION_STC     *exMxPmTtiActionPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC                  *actionPtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) actionPtr, 0, sizeof(*actionPtr));

    /* convert tti action from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, command);

    /* set redirect command */
    switch (exMxPmTtiActionPtr->redirectCommand)
    {
        case CPSS_EXMXPM_TTI_NO_REDIRECT_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
            break;
        case CPSS_EXMXPM_TTI_REDIRECT_TO_OUTLIF_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_OUTLIF_E;
            break;
        case CPSS_EXMXPM_TTI_REDIRECT_TO_NEXT_HOP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_NEXT_HOP_E;
            break;
        case CPSS_EXMXPM_TTI_TRIGGER_VPLS_LOOKUP_E:
            actionPtr->redirectCommand = PRV_TGF_TTI_TRIGGER_VPLS_LOOKUP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* set outlif info */
    cpssOsMemCpy((GT_VOID*) &(actionPtr->egressInterface),
                 (GT_VOID*) &(exMxPmTtiActionPtr->outlif.interfaceInfo),
                 sizeof(exMxPmTtiActionPtr->outlif.interfaceInfo));

    actionPtr->arpPtr = exMxPmTtiActionPtr->outlif.outlifPointer.arpPtr;

    /* set vlanCmd */
    switch (exMxPmTtiActionPtr->inLifVidCmd)
    {
        case CPSS_EXMXPM_TTI_VLAN_INLIF_DO_NOT_MODIFY_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_UNTAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_TAGGED_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E;
            break;

        case CPSS_EXMXPM_TTI_VLAN_INLIF_MODIFY_ALL_E:
            actionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    actionPtr->tag0VlanId = exMxPmTtiActionPtr->inLifVidId;

    /* convert mplsCmd from device specific format */
    switch (exMxPmTtiActionPtr->mplsCmd)
    {
        case CPSS_EXMXPM_TTI_MPLS_NOP_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_SWAP_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_SWAP_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_PUSH1_LABEL_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP1_LABEL_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP2_LABELS_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP3_LABELS_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E;
            break;

        case CPSS_EXMXPM_TTI_MPLS_POP_AND_SWAP_COMMAND_E:
            actionPtr->mplsCommand = PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert passengerPacketType from device specific format */
    switch (exMxPmTtiActionPtr->passengerPacketType)
    {
        case CPSS_EXMXPM_TTI_PASSENGER_IPV4_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_IPV6_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV6_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_CRC_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
            break;

        case CPSS_EXMXPM_TTI_PASSENGER_ETHERNET_NO_CRC_E:
            actionPtr->ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert pwIdMode from device specific format */
    switch (exMxPmTtiActionPtr->pwIdMode)
    {
        case CPSS_EXMXPM_TTI_PW_ID_MODE_ENTRY_E:
            actionPtr->pwIdMode = PRV_TGF_TTI_PW_ID_MODE_ENTRY_E;
            break;

        case CPSS_EXMXPM_TTI_PW_ID_MODE_BASE_E:
            actionPtr->pwIdMode = PRV_TGF_TTI_PW_ID_MODE_BASE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti action from device specific format */
    actionPtr->tunnelTerminate = exMxPmTtiActionPtr->tunnelTerminateEnable;
    actionPtr->enableDecrementTtl = exMxPmTtiActionPtr->enableDecTtl;
    actionPtr->tag0VlanPrecedence = exMxPmTtiActionPtr->vlanPrecedence;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, userDefinedCpuCode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    actionPtr->copyTtlExpFromTunnelHeader = exMxPmTtiActionPtr->copyTtlFromTunnelHeader;
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mirrorToIngressAnalyzerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, policerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, nestedVlanEnable);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ipNextHopIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, exp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, setExp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, mplsLabel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, pwId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sourceIsPE);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, enableSourceLocalFiltering);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, floodDitPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, baseMplsLabel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, counterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, meterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, flowId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstIdEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmTtiActionPtr, actionPtr, sstId);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmIpv4TtiRule function
* @endinternal
*
* @brief   Convert generic IPv4 TTI rule into device specific IPv4 TTI rule
*
* @param[in] ipv4TtiRulePtr           - (pointer to) IPv4 TTI rule
*
* @param[out] exMxPmIpv4TtiRulePtr     - (pointer to) ExMxPm IPv4 TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToExMxPmIpv4TtiRule
(
    IN  PRV_TGF_TTI_IPV4_RULE_STC      *ipv4TtiRulePtr,
    OUT CPSS_EXMXPM_TTI_IPV4_RULE_STC  *exMxPmIpv4TtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmIpv4TtiRulePtr, 0, sizeof(*exMxPmIpv4TtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcIsTrunk);
    exMxPmIpv4TtiRulePtr->common.dsaSrcPortTrunk = (GT_U8)ipv4TtiRulePtr->common.dsaSrcPortTrunk;
    exMxPmIpv4TtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(ipv4TtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4TtiRulePtr, ipv4TtiRulePtr, tunneltype);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4TtiRulePtr, ipv4TtiRulePtr, isArp);

    /* set MAC */
    cpssOsMemCpy(exMxPmIpv4TtiRulePtr->common.mac.arEther,
                 ipv4TtiRulePtr->common.mac.arEther,
                 sizeof(ipv4TtiRulePtr->common.mac.arEther));

    /* set source IP */
    cpssOsMemCpy(exMxPmIpv4TtiRulePtr->srcIp.arIP,
                 ipv4TtiRulePtr->srcIp.arIP,
                 sizeof(ipv4TtiRulePtr->srcIp.arIP));

    /* set destination IP */
    cpssOsMemCpy(exMxPmIpv4TtiRulePtr->destIp.arIP,
                 ipv4TtiRulePtr->destIp.arIP,
                 sizeof(ipv4TtiRulePtr->destIp.arIP));
}

/**
* @internal prvTgfConvertExMxPmToGenericIpv4TtiRule function
* @endinternal
*
* @brief   Convert device specific IPv4 TTI rule into generic IPv4 TTI rule
*
* @param[in] exMxPmIpv4TtiRulePtr     - (pointer to) ExMxPm IPv4 TTI rule
*
* @param[out] ipv4TtiRulePtr           - (pointer to) IPv4 TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertExMxPmToGenericIpv4TtiRule
(
    IN  CPSS_EXMXPM_TTI_IPV4_RULE_STC  *exMxPmIpv4TtiRulePtr,
    OUT PRV_TGF_TTI_IPV4_RULE_STC      *ipv4TtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) ipv4TtiRulePtr, 0, sizeof(*ipv4TtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmIpv4TtiRulePtr->common), &(ipv4TtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4TtiRulePtr, ipv4TtiRulePtr, tunneltype);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4TtiRulePtr, ipv4TtiRulePtr, isArp);

    /* set MAC */
    cpssOsMemCpy(ipv4TtiRulePtr->common.mac.arEther,
                 exMxPmIpv4TtiRulePtr->common.mac.arEther,
                 sizeof(exMxPmIpv4TtiRulePtr->common.mac.arEther));

    /* set source IP */
    cpssOsMemCpy(ipv4TtiRulePtr->srcIp.arIP,
                 exMxPmIpv4TtiRulePtr->srcIp.arIP,
                 sizeof(exMxPmIpv4TtiRulePtr->srcIp.arIP));

    /* set destination IP */
    cpssOsMemCpy(ipv4TtiRulePtr->destIp.arIP,
                 exMxPmIpv4TtiRulePtr->destIp.arIP,
                 sizeof(exMxPmIpv4TtiRulePtr->destIp.arIP));
}

/**
* @internal prvTgfConvertGenericToExMxPmMplsTtiRule function
* @endinternal
*
* @brief   Convert generic Mpls TTI rule into device specific Mpls TTI rule
*
* @param[in] mplsTtiRulePtr           - (pointer to) Mpls TTI rule
*
* @param[out] exMxPmMplsTtiRulePtr     - (pointer to) ExMxPm Mpls TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToExMxPmMplsTtiRule
(
    IN  PRV_TGF_TTI_MPLS_RULE_STC      *mplsTtiRulePtr,
    OUT CPSS_EXMXPM_TTI_MPLS_RULE_STC  *exMxPmMplsTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmMplsTtiRulePtr, 0, sizeof(*exMxPmMplsTtiRulePtr));

    /* convert Mpls tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcIsTrunk);
    exMxPmMplsTtiRulePtr->common.dsaSrcPortTrunk = (GT_U8)mplsTtiRulePtr->common.dsaSrcPortTrunk;
    exMxPmMplsTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(mplsTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, numOfLabels);

    exMxPmMplsTtiRulePtr->nlpAboveMPLS = mplsTtiRulePtr->protocolAboveMPLS;

    /* set MAC */
    cpssOsMemCpy(exMxPmMplsTtiRulePtr->common.mac.arEther,
                 mplsTtiRulePtr->common.mac.arEther,
                 sizeof(mplsTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertExMxPmToGenericMplsTtiRule function
* @endinternal
*
* @brief   Convert device specific Mpls TTI rule into generic Mpls TTI rule
*
* @param[in] exMxPmMplsTtiRulePtr     - (pointer to) ExMxPm Mpls TTI rule
*
* @param[out] mplsTtiRulePtr           - (pointer to) Mpls TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertExMxPmToGenericMplsTtiRule
(
    IN  CPSS_EXMXPM_TTI_MPLS_RULE_STC  *exMxPmMplsTtiRulePtr,
    OUT PRV_TGF_TTI_MPLS_RULE_STC      *mplsTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) mplsTtiRulePtr, 0, sizeof(*mplsTtiRulePtr));

    /* convert Mpls tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMplsTtiRulePtr->common), &(mplsTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, label2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, exp2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsTtiRulePtr, mplsTtiRulePtr, numOfLabels);

    mplsTtiRulePtr->protocolAboveMPLS = exMxPmMplsTtiRulePtr->nlpAboveMPLS;

    /* set MAC */
    cpssOsMemCpy(mplsTtiRulePtr->common.mac.arEther,
                 exMxPmMplsTtiRulePtr->common.mac.arEther,
                 sizeof(exMxPmMplsTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToExMxPmEthTtiRule function
* @endinternal
*
* @brief   Convert generic Ethernet TTI rule into device specific Ethernet TTI rule
*
* @param[in] ethTtiRulePtr            - (pointer to) Ethernet TTI rule
*
* @param[out] exMxPmEthTtiRulePtr      - (pointer to) ExMxPm Ethernet TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToExMxPmEthTtiRule
(
    IN  PRV_TGF_TTI_ETH_RULE_STC       *ethTtiRulePtr,
    OUT CPSS_EXMXPM_TTI_ETH_RULE_STC   *exMxPmEthTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmEthTtiRulePtr, 0, sizeof(*exMxPmEthTtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcIsTrunk);
    exMxPmEthTtiRulePtr->common.dsaSrcPortTrunk = (GT_U8)ethTtiRulePtr->common.dsaSrcPortTrunk;
    exMxPmEthTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(ethTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, up0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, cfi0);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, isVlan1Exists);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, cfi1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, etherType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, macToMe);

    /* set MAC */
    cpssOsMemCpy(exMxPmEthTtiRulePtr->common.mac.arEther,
                 ethTtiRulePtr->common.mac.arEther,
                 sizeof(ethTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertExMxPmToGenericEthTtiRule function
* @endinternal
*
* @brief   Convert device specific Ethernet TTI rule into generic Ethernet TTI rule
*
* @param[in] exMxPmEthTtiRulePtr      - (pointer to) ExMxPm Ethernet TTI rule
*
* @param[out] ethTtiRulePtr            - (pointer to) Ethernet TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertExMxPmToGenericEthTtiRule
(
    IN  CPSS_EXMXPM_TTI_ETH_RULE_STC   *exMxPmEthTtiRulePtr,
    OUT PRV_TGF_TTI_ETH_RULE_STC       *ethTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) ethTtiRulePtr, 0, sizeof(*ethTtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEthTtiRulePtr->common), &(ethTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, up0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, cfi0);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, isVlan1Exists);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, vid1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, up1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, cfi1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, etherType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEthTtiRulePtr, ethTtiRulePtr, macToMe);

    /* set MAC */
    cpssOsMemCpy(ethTtiRulePtr->common.mac.arEther,
                 exMxPmEthTtiRulePtr->common.mac.arEther,
                 sizeof(exMxPmEthTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToExMxPmMimTtiRule function
* @endinternal
*
* @brief   Convert generic MIM TTI rule into device specific MIM TTI rule
*
* @param[out] exMxPmMimTtiRulePtr      - (pointer to) ExMxPm MIM TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertGenericToExMxPmMimTtiRule
(
    IN  PRV_TGF_TTI_MIM_RULE_STC       *mimTtiRulePtr,
    OUT CPSS_EXMXPM_TTI_MIM_RULE_STC   *exMxPmMimTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) exMxPmMimTtiRulePtr, 0, sizeof(*exMxPmMimTtiRulePtr));

    /* convert IPv4 tti rule into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), isTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcIsTrunk);
    exMxPmMimTtiRulePtr->common.dsaSrcPortTrunk = (GT_U8)mimTtiRulePtr->common.dsaSrcPortTrunk;
    exMxPmMimTtiRulePtr->common.dsaSrcDevice =
        prvTgfCommonFixHwDevId(mimTtiRulePtr->common.dsaSrcDevice, GT_TRUE);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, bUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, bDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iSid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iRes1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iRes2);

    /* set MAC */
    cpssOsMemCpy(exMxPmMimTtiRulePtr->common.mac.arEther,
                 mimTtiRulePtr->common.mac.arEther,
                 sizeof(mimTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertExMxPmToGenericMimTtiRule function
* @endinternal
*
* @brief   Convert device specific MIM TTI rule into generic MIM TTI rule
*
* @param[in] exMxPmMimTtiRulePtr      - (pointer to) ExMxPm MIM TTI rule
*                                       None
*/
static GT_VOID prvTgfConvertExMxPmToGenericMimTtiRule
(
    IN  CPSS_EXMXPM_TTI_MIM_RULE_STC   *exMxPmMimTtiRulePtr,
    OUT PRV_TGF_TTI_MIM_RULE_STC       *mimTtiRulePtr
)
{
    /* clear tti action */
    cpssOsMemSet((GT_VOID*) mimTtiRulePtr, 0, sizeof(*mimTtiRulePtr));

    /* convert IPv4 tti rule from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), pclId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), srcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), vid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), isTagged);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcPortTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmMimTtiRulePtr->common), &(mimTtiRulePtr->common), dsaSrcDevice);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, bUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, bDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iSid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iRes1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimTtiRulePtr, mimTtiRulePtr, iRes2);

    /* set MAC */
    cpssOsMemCpy(mimTtiRulePtr->common.mac.arEther,
                 exMxPmMimTtiRulePtr->common.mac.arEther,
                 sizeof(exMxPmMimTtiRulePtr->common.mac.arEther));
}

/**
* @internal prvTgfConvertGenericToExMxPmTunnelStartIpv4Entry function
* @endinternal
*
* @brief   Convert generic tunnel start IPv4 entry into device specific
*
* @param[in] ipv4EntryPtr             - (pointer to) tunnel start IPv4 entry
*
* @param[out] exMxPmIpv4EntryPtr       - (pointer to) ExMxPm tunnel start IPv4 entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTunnelStartIpv4Entry
(
    IN  PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC       *ipv4EntryPtr,
    OUT CPSS_EXMXPM_TUNNEL_START_IPV4_ENTRY_STC   *exMxPmIpv4EntryPtr
)
{
    /* clear tunnel start IPv4 entry */
    cpssOsMemSet((GT_VOID*) exMxPmIpv4EntryPtr, 0, sizeof(*exMxPmIpv4EntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmIpv4EntryPtr->upMarkMode, ipv4EntryPtr->upMarkMode);

    /* convert dscpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmIpv4EntryPtr->dscpMarkMode, ipv4EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv4 entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, dscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, dontFragmentFlag);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, autoTunnel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, autoTunnelOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, ethType);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, destIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, srcIp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, retainCrc);

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTunnelStartIpv4Entry function
* @endinternal
*
* @brief   Convert device specific tunnel start IPv4 entry into generic
*
* @param[in] exMxPmIpv4EntryPtr       - (pointer to) ExMxPm tunnel start IPv4 entry
*
* @param[out] ipv4EntryPtr             - (pointer to) tunnel start IPv4 entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTunnelStartIpv4Entry
(
    IN  CPSS_EXMXPM_TUNNEL_START_IPV4_ENTRY_STC   *exMxPmIpv4EntryPtr,
    OUT PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC       *ipv4EntryPtr
)
{
    /* clear tunnel start IPv4 entry */
    cpssOsMemSet((GT_VOID*) ipv4EntryPtr, 0, sizeof(*ipv4EntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmIpv4EntryPtr->upMarkMode, ipv4EntryPtr->upMarkMode);

    /* convert dscpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmIpv4EntryPtr->dscpMarkMode, ipv4EntryPtr->dscpMarkMode);

    /* convert tunnel start IPv4 entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, dscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, dontFragmentFlag);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, autoTunnel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, autoTunnelOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, ethType);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, destIp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, srcIp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpv4EntryPtr, ipv4EntryPtr, retainCrc);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmTunnelStartMplsEntry function
* @endinternal
*
* @brief   Convert generic tunnel start Mpls entry into device specific
*
* @param[in] mplsEntryPtr             - (pointer to) tunnel start Mpls entry
*
* @param[out] exMxPmMplsEntryPtr       - (pointer to) ExMxPm tunnel start Mpls entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTunnelStartMplsEntry
(
    IN  PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC       *mplsEntryPtr,
    OUT CPSS_EXMXPM_TUNNEL_START_MPLS_ENTRY_STC   *exMxPmMplsEntryPtr
)
{
    /* clear tunnel start Mpls entry */
    cpssOsMemSet((GT_VOID*) exMxPmMplsEntryPtr, 0, sizeof(*exMxPmMplsEntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmMplsEntryPtr->upMarkMode, mplsEntryPtr->upMarkMode);

    /* convert ethType into device specific format */
    switch (mplsEntryPtr->ethType)
    {
        case PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:
            exMxPmMplsEntryPtr->ethType = CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
            break;

        case PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:
            exMxPmMplsEntryPtr->ethType = CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert exp1MarkMode into device specific format */
    switch (mplsEntryPtr->exp1MarkMode)
    {
        /*case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E;
            break;
        */
        case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert exp2MarkMode into device specific format */
    switch (mplsEntryPtr->exp2MarkMode)
    {
        /*case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:
            exMxPmMplsEntryPtr->exp2MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E:
            exMxPmMplsEntryPtr->exp2MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:
            exMxPmMplsEntryPtr->exp2MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E;
            break;
        */
        case PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E;
            break;

        case PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E:
            exMxPmMplsEntryPtr->exp1MarkMode = CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start Mpls entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, numLabels);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, label1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, exp1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, label2);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, exp2);

    exMxPmMplsEntryPtr->retainCrc = mplsEntryPtr->retainCRC;

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTunnelStartMplsEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start Mpls entry into generic
*
* @param[in] exMxPmMplsEntryPtr       - (pointer to) ExMxPm tunnel start Mpls entry
*
* @param[out] mplsEntryPtr             - (pointer to) tunnel start Mpls entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTunnelStartMplsEntry
(
    IN  CPSS_EXMXPM_TUNNEL_START_MPLS_ENTRY_STC   *exMxPmMplsEntryPtr,
    OUT PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC       *mplsEntryPtr
)
{
    /* clear tunnel start Mpls entry */
    cpssOsMemSet((GT_VOID*) mplsEntryPtr, 0, sizeof(*mplsEntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmMplsEntryPtr->upMarkMode, mplsEntryPtr->upMarkMode);

    /* convert ethType from device specific format */
    switch (exMxPmMplsEntryPtr->ethType)
    {
        case CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:
            mplsEntryPtr->ethType = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:
            mplsEntryPtr->ethType = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert exp1MarkMode from device specific format */
    switch (exMxPmMplsEntryPtr->exp1MarkMode)
    {
        /*case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;
            break;
        */
        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E:
            mplsEntryPtr->exp1MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert exp2MarkMode from device specific format */
    switch (exMxPmMplsEntryPtr->exp2MarkMode)
    {
        /*case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;
            break;
        */
        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_TS_ENTRY_DP_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E;
            break;

        case CPSS_EXMXPM_TUNNEL_START_EXP_MARK_FROM_INGRESS_PIPE_E:
            mplsEntryPtr->exp2MarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert tunnel start Mpls entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, numLabels);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, label1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, exp1);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, label2);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMplsEntryPtr, mplsEntryPtr, exp2);

    mplsEntryPtr->retainCRC = exMxPmMplsEntryPtr->retainCrc;

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmTunnelStartMimEntry function
* @endinternal
*
* @brief   Convert generic tunnel start Mim entry into device specific
*
* @param[in] mimEntryPtr              - (pointer to) tunnel start Mim entry
*
* @param[out] exMxPmMimEntryPtr        - (pointer to) ExMxPm tunnel start Mim entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmTunnelStartMimEntry
(
    IN  PRV_TGF_TUNNEL_START_MIM_ENTRY_STC        *mimEntryPtr,
    OUT CPSS_EXMXPM_TUNNEL_START_MIM_ENTRY_STC    *exMxPmMimEntryPtr
)
{
    /* clear tunnel start Mim entry */
    cpssOsMemSet((GT_VOID*) exMxPmMimEntryPtr, 0, sizeof(*exMxPmMimEntryPtr));

    /* convert upMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->upMarkMode, mimEntryPtr->upMarkMode);

    /* convert iSidAssignMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iSidAssignMode, mimEntryPtr->iSidAssignMode);

    /* convert iUpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iUpMarkMode, mimEntryPtr->iUpMarkMode);

    /* convert iDpMarkMode into device specific format */
    PRV_TGF_S2D_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iDpMarkMode, mimEntryPtr->iDpMarkMode);

    /* convert tunnel start Mim entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, tagEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, macDa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, retainCrc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iSid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iTagReserved);

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericTunnelStartMimEntry function
* @endinternal
*
* @brief   Convert device specific tunnel start Mim entry into generic
*
* @param[in] exMxPmMimEntryPtr        - (pointer to) ExMxPm tunnel start Mim entry
*
* @param[out] mimEntryPtr              - (pointer to) tunnel start Mim entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericTunnelStartMimEntry
(
    IN  CPSS_EXMXPM_TUNNEL_START_MIM_ENTRY_STC   *exMxPmMimEntryPtr,
    OUT PRV_TGF_TUNNEL_START_MIM_ENTRY_STC       *mimEntryPtr
)
{
    /* clear tunnel start Mim entry */
    cpssOsMemSet((GT_VOID*) mimEntryPtr, 0, sizeof(*mimEntryPtr));

    /* convert upMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->upMarkMode, mimEntryPtr->upMarkMode);

    /* convert iSidAssignMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iSidAssignMode, mimEntryPtr->iSidAssignMode);

    /* convert iUpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iUpMarkMode, mimEntryPtr->iUpMarkMode);

    /* convert iDpMarkMode from device specific format */
    PRV_TGF_D2S_MARK_MODE_CONVERT_MAC(exMxPmMimEntryPtr->iDpMarkMode, mimEntryPtr->iDpMarkMode);

    /* convert tunnel start Mim entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, tagEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, macDa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, retainCrc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iSid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmMimEntryPtr, mimEntryPtr, iTagReserved);

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfTtiTcamIndexIsRelativeSet function
* @endinternal
*
* @brief   Set whether the TTI indexes are relative to the start of the TTI section
*         in TCAM
* @param[in] isRelative               - GT_TRUE: TTI indexes are relative to the start of the TTI section
*                                      GT_FALSE: TTI indexes are absolute
*                                       None
*/
GT_VOID prvTgfTtiTcamIndexIsRelativeSet
(
    IN  GT_BOOL     isRelative
)
{
#ifdef CHX_FAMILY
    ttiIndexIsRelative = isRelative;
#else
    /* avoid warning */
    TGF_PARAM_NOT_USED(isRelative);
#endif
}

/**
* @internal prvTgfTtiTcamAbsoluteBaseGet function
* @endinternal
*
* @brief   Get absolute TTI rules index bases in TCAM for Hit0 and for Hit1
*         relevant for earch devices, otherwise returns 0
* @param[in] hitIndex                 -  0 or 1
*                                       None
*/
GT_U32 prvTgfTtiTcamAbsoluteBaseGet
(
    IN  GT_U32     hitIndex
)
{
#ifdef CHX_FAMILY
    return appDemoDxChTcamTtiBaseIndexGet(prvTgfDevNum, hitIndex);
#else
    /* avoid warning */
    TGF_PARAM_NOT_USED(hitIndex);
    return 0;
#endif
}

/**
* @internal prvTgfTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 prvTgfTcamTtiNumOfIndexsGet
(
    IN     GT_U32                           hitNum
)
{
#ifdef CHX_FAMILY
    return appDemoDxChTcamTtiNumOfIndexsGet(prvTgfDevNum, hitNum);
#else
    /* avoid warning */
    TGF_PARAM_NOT_USED(hitNum);
    return 0;
#endif
}

/**
* @internal prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet function
* @endinternal
*
* @brief   Get absolute TTI rules index bases in TCAM for Hit0/1/2/3
*         the 'relative index' is modulo (%) by the function according to
*         prvTgfTcamTtiNumOfIndexsGet(hitNum)
*         relevant for earch devices, otherwise returns 0
* @param[in] hitIndex                 -  0 .. 3
* @param[in] relativeTcamEntryIndex
*                                       None
*/
GT_U32 prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet
(
    IN  GT_U32     hitIndex,
    IN  GT_U32     relativeTcamEntryIndex
)
{
    GT_U32 numIndexGet = prvTgfTcamTtiNumOfIndexsGet(hitIndex);
    GT_U32 baseIndex = prvTgfTtiTcamAbsoluteBaseGet(hitIndex);
    GT_U32 alignedIndex;

    if(numIndexGet == 0)
    {
        /*error*/
        return 0;
    }

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        PRV_CPSS_PP_MAC(prvTgfDevNum)->devType   == CPSS_98DX8332_Z0_CNS)
    {
        /* Aldrin has 3 floors those have default partition:
           Floor 0 - PCL
           Floor 1, blocks 0..2 TTI_0, blocks 3..5 TTI_1
           Floor 2, blocks 0..2 TTI_2, blocks 3..5 TTI_3
           TCAM block is two TCAM arrays.
           Index must be aligned with proper block.
           e.g. Indexes for TTI_x are 0, 3, 12, 15, 24, 27...
           baseIndex points to first entry of TTI lookup */

        /* caller use indexes aligned with 3 entries.
           Get relative number. */
        alignedIndex = relativeTcamEntryIndex / 3;

        relativeTcamEntryIndex = (alignedIndex >> 1) * 12 + (alignedIndex & 1) * 3;
    }

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}

/**
* @internal prvTgfTtiRuleActionSet function
* @endinternal
*
* @brief   This function sets rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleActionSet
(
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT             devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_ACTION_UNT            dxChTtiAction;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_TYPE_ENT           tcamType   = CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E;
    CPSS_EXMXPM_TTI_ACTION_TYPE_ENT     actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    CPSS_EXMXPM_TCAM_ACTION_INFO_UNT    actionInfo;
    CPSS_EXMXPM_TTI_ACTION_UNT          exMxPmTtiAction;
    GT_BOOL                             extTcam;
    GT_U32                              ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericToDxChTtiAction2(actionPtr, &dxChTtiAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleActionUpdate(devNum, ruleIndex, actionType, &dxChTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleActionUpdate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    /* set tcam type and tcam action type */
    tcamType   = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;
    actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;

    /* clear tcam action info and tti action */
    cpssOsMemSet((GT_VOID*) &actionInfo, 0, sizeof(actionInfo));
    cpssOsMemSet((GT_VOID*) &exMxPmTtiAction, 0, sizeof(exMxPmTtiAction));

    /* set rule index */
    if (extTcam == GT_FALSE)
    {
    actionInfo.internalTcamRuleStartIndex = ruleIndex;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, ruleIndex,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }
        actionInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        actionInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        actionInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* convert tti action into device specific format */
    rc = prvTgfConvertGenericToExMxPmTtiAction(actionPtr, &(exMxPmTtiAction.standard));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmTtiAction FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiRuleActionUpdate(devNum, tcamType, &actionInfo, actionType, &exMxPmTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleActionUpdate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfTtiRuleAction2Set function
* @endinternal
*
* @brief   This function sets rule action
*
* @param[in] ruleIndex                - index of the rule in the TCAM
*                                      actionPtr - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleAction2Set
(
    IN  GT_U32                          ruleIndex,
    IN  PRV_TGF_TTI_ACTION_2_STC       *action2Ptr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_TTI_ACTION_UNT            dxChAction;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericAction2ToDxChTtiAction2(action2Ptr, &dxChAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleActionUpdate(devNum, ruleIndex,
                                         CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                                         &dxChAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleActionUpdate FAILED, rc = [%d]", rc);
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(ruleIndex);
    TGF_PARAM_NOT_USED(action2Ptr);
    return GT_BAD_STATE;
#endif
}

#ifdef CHX_FAMILY

static GT_VOID prvTgfTtiConvertKeyTypeToRuleType
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT    keyType,
    OUT PRV_TGF_TTI_RULE_TYPE_ENT   *ruleTypePtr
)
{
    switch(keyType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            *ruleTypePtr = PRV_TGF_TTI_RULE_IPV4_E;
            break;
        case PRV_TGF_TTI_KEY_MPLS_E:
            *ruleTypePtr = PRV_TGF_TTI_RULE_MPLS_E;
            break;
        case PRV_TGF_TTI_KEY_ETH_E:
            *ruleTypePtr = PRV_TGF_TTI_RULE_ETH_E;
            break;
        case PRV_TGF_TTI_KEY_MIM_E:
            *ruleTypePtr = PRV_TGF_TTI_RULE_MIM_E;
            break;
        default:
            *ruleTypePtr = PRV_TGF_TTI_RULE_UDB_30_E;
            break;
    }
}

/**
* @internal prvTgfTtiRuleConvertGenericToDx function
* @endinternal
*
* @brief   convert 'TGF' generic Pattern/Mask to DXCH Pattern/Mask
*
* @param[in] ruleType                 - TTI rule type
*                                      ruleIndex  - index of the tunnel termination entry
* @param[in] patternPtr               - (pointer to) the rule's pattern
* @param[in] maskPtr                  - (pointer to) the rule's mask
*                                      actionPtr  - (pointer to) the TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleConvertGenericToDx
(
    IN  PRV_TGF_TTI_RULE_TYPE_ENT      ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT       *dxChPatternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT       *dxChMaskPtr
)
{
    cpssOsMemSet(dxChPatternPtr,   0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(dxChMaskPtr,      0, sizeof(CPSS_DXCH_TTI_RULE_UNT));

    /* convert tti rule into device specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_RULE_IPV4_E:
            /* convert IPv4 tti rule pattern into device specific format */
            prvTgfConvertGenericToDxChIpv4TtiRule(&(patternPtr->ipv4),
                                                  &(dxChPatternPtr->ipv4));
            /* convert IPv4 tti rule mask into device specific format */
            prvTgfConvertGenericToDxChIpv4TtiRule(&(maskPtr->ipv4),
                                                  &(dxChMaskPtr->ipv4));
            break;

        case PRV_TGF_TTI_RULE_MPLS_E:
            /* convert Mpls tti rule pattern into device specific format */
            prvTgfConvertGenericToDxChMplsTtiRule(&(patternPtr->mpls),
                                                  &(dxChPatternPtr->mpls));
            /* convert Mpls tti rule mask into device specific format */
            prvTgfConvertGenericToDxChMplsTtiRule(&(maskPtr->mpls),
                                                  &(dxChMaskPtr->mpls));
            break;

        case PRV_TGF_TTI_RULE_ETH_E:
            /* convert Ethernet tti rule pattern into device specific format */
            prvTgfConvertGenericToDxChEthTtiRule(&(patternPtr->eth),
                                                  &(dxChPatternPtr->eth));
            /* convert Ethernet tti rule mask into device specific format */
            prvTgfConvertGenericToDxChEthTtiRule(&(maskPtr->eth),
                                                  &(dxChMaskPtr->eth));
            break;

        case PRV_TGF_TTI_RULE_MIM_E:
            /* convert Mac-in-Mac tti rule pattern into device specific format */
            prvTgfConvertGenericToDxChMimTtiRule(&(patternPtr->mim),
                                                 &(dxChPatternPtr->mim));
            /* convert Mac-in-Mac tti rule mask into device specific format */
            prvTgfConvertGenericToDxChMimTtiRule(&(maskPtr->mim),
                                                 &(dxChMaskPtr->mim));
            break;

        case PRV_TGF_TTI_RULE_UDB_10_E:
        case PRV_TGF_TTI_RULE_UDB_20_E:
        case PRV_TGF_TTI_RULE_UDB_30_E:
            /* convert UDB tti rule pattern into device specific format */
            cpssOsMemCpy(dxChPatternPtr->udbArray, patternPtr->udbArray, sizeof(dxChPatternPtr->udbArray));

            /* convert UDB tti rule mask into device specific format */
            cpssOsMemCpy(dxChMaskPtr->udbArray, maskPtr->udbArray, sizeof(dxChMaskPtr->udbArray));
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /*CHX_FAMILY*/

/**
* @internal prvTgfTtiRuleDefaultActionSet function
* @endinternal
*
* @brief   This function sets the global default action (used by standard
*         actions).
* @param[in] defaultQosParamsPtr      - points to the default TTI rule action qos
*                                      parameters that applied on packet if packet's
*                                      search key matched with masked pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiRuleDefaultActionSet
(
    IN  PRV_TGF_TTI_ACTION_QOS_STC  *defaultQosParamsPtr
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(defaultQosParamsPtr);
    return GT_BAD_STATE;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_U8       devNum = prvTgfDevNum;
    GT_STATUS   rc     = GT_OK;
    GT_STATUS   rc1    = GT_OK;

    CPSS_EXMXPM_TTI_ACTION_QOS_STC  exMxPmDefaultQosParams;

    exMxPmDefaultQosParams.exp=defaultQosParamsPtr->exp;
    exMxPmDefaultQosParams.qosParams.keepPreviousQoS=defaultQosParamsPtr->qosParams.keepPreviousQoS;
    exMxPmDefaultQosParams.qosParams.trustUp=defaultQosParamsPtr->qosParams.trustUp;
    exMxPmDefaultQosParams.qosParams.trustDscp=defaultQosParamsPtr->qosParams.trustDscp;
    exMxPmDefaultQosParams.qosParams.trustExp=defaultQosParamsPtr->qosParams.trustExp;
    exMxPmDefaultQosParams.qosParams.enableUpToUpRemapping=defaultQosParamsPtr->qosParams.enableUpToUpRemapping;
    exMxPmDefaultQosParams.qosParams.enableDscpToDscpRemapping=defaultQosParamsPtr->qosParams.enableDscpToDscpRemapping;
    exMxPmDefaultQosParams.qosParams.enableCfiToDpMapping=defaultQosParamsPtr->qosParams.enableCfiToDpMapping;
    exMxPmDefaultQosParams.qosParams.up=defaultQosParamsPtr->qosParams.up;
    exMxPmDefaultQosParams.qosParams.dscp=defaultQosParamsPtr->qosParams.dscp;
    exMxPmDefaultQosParams.qosParams.tc=defaultQosParamsPtr->qosParams.tc;
    exMxPmDefaultQosParams.qosParams.dp=defaultQosParamsPtr->qosParams.dp;
    exMxPmDefaultQosParams.qosParams.modifyDscp=defaultQosParamsPtr->qosParams.modifyDscp;
    exMxPmDefaultQosParams.qosParams.modifyUp=defaultQosParamsPtr->qosParams.modifyUp;
    exMxPmDefaultQosParams.qosParams.modifyExp=defaultQosParamsPtr->qosParams.modifyExp;
    exMxPmDefaultQosParams.qosParams.qosPrecedence=defaultQosParamsPtr->qosParams.qosPrecedence;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiRuleDefaultActionSet(devNum,&exMxPmDefaultQosParams);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssExMxPmTtiRuleDefaultActionSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiRuleDefaultActionGet function
* @endinternal
*
* @brief   This function gets the global default action qos parameters.
*
* @param[in] devNum                   - device number
*
* @param[out] defaultQosParamsPtr      - points to the default TTI rule action qos
*                                      parameters that applied on packet if packet's
*                                      search key matched with masked pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiRuleDefaultActionGet
(
    IN  GT_U8                       devNum,
    OUT PRV_TGF_TTI_ACTION_QOS_STC  *defaultQosParamsPtr
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(defaultQosParamsPtr);
    return GT_BAD_STATE;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                       rc=GT_OK;
    CPSS_EXMXPM_TTI_ACTION_QOS_STC  exMxPmDefaultQosParams;

    rc = cpssExMxPmTtiRuleDefaultActionGet(devNum,&exMxPmDefaultQosParams);
    if(rc != GT_OK)
        return rc;

    defaultQosParamsPtr->exp=exMxPmDefaultQosParams.exp;
    defaultQosParamsPtr->qosParams.keepPreviousQoS=exMxPmDefaultQosParams.qosParams.keepPreviousQoS;
    defaultQosParamsPtr->qosParams.trustUp=exMxPmDefaultQosParams.qosParams.trustUp;
    defaultQosParamsPtr->qosParams.trustDscp=exMxPmDefaultQosParams.qosParams.trustDscp;
    defaultQosParamsPtr->qosParams.trustExp=exMxPmDefaultQosParams.qosParams.trustExp;
    defaultQosParamsPtr->qosParams.enableUpToUpRemapping=exMxPmDefaultQosParams.qosParams.enableUpToUpRemapping;
    defaultQosParamsPtr->qosParams.enableDscpToDscpRemapping=exMxPmDefaultQosParams.qosParams.enableDscpToDscpRemapping;
    defaultQosParamsPtr->qosParams.enableCfiToDpMapping=exMxPmDefaultQosParams.qosParams.enableCfiToDpMapping;
    defaultQosParamsPtr->qosParams.up=exMxPmDefaultQosParams.qosParams.up;
    defaultQosParamsPtr->qosParams.dscp=exMxPmDefaultQosParams.qosParams.dscp;
    defaultQosParamsPtr->qosParams.tc=exMxPmDefaultQosParams.qosParams.tc;
    defaultQosParamsPtr->qosParams.dp=exMxPmDefaultQosParams.qosParams.dp;
    defaultQosParamsPtr->qosParams.modifyDscp=exMxPmDefaultQosParams.qosParams.modifyDscp;
    defaultQosParamsPtr->qosParams.modifyUp=exMxPmDefaultQosParams.qosParams.modifyUp;
    defaultQosParamsPtr->qosParams.modifyExp=exMxPmDefaultQosParams.qosParams.modifyExp;
    defaultQosParamsPtr->qosParams.qosPrecedence=exMxPmDefaultQosParams.qosParams.qosPrecedence;

    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfTtiRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleSet
(
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT     ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_TYPE_ENT               tcamType   = CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E;
    CPSS_EXMXPM_TTI_ACTION_TYPE_ENT         actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    GT_BOOL                                 valid      = GT_FALSE;
    CPSS_EXMXPM_TCAM_RULE_ACTION_INFO_UNT   ruleInfo;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmPattern;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmMask;
    CPSS_EXMXPM_TTI_ACTION_UNT              exMxPmTtiAction;
    GT_U32  pclIdPattern , PclIdMask;
    GT_BOOL                                 extTcam;
    GT_U32                                  ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChTtiAction, 0, sizeof(dxChTtiAction));

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule into DXCH specific format */
    rc = prvTgfTtiRuleConvertGenericToDx(ruleType,patternPtr,maskPtr,&dxChPattern,&dxChMask);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTtiRuleConvertGenericToDx FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericToDxChTtiAction2(actionPtr, &dxChTtiAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleSet(devNum, ruleIndex, (CPSS_DXCH_TTI_KEY_TYPE_ENT) ruleType,
                                &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* reset variables */
    cpssOsMemSet((GT_VOID*) &ruleInfo, 0, sizeof(ruleInfo));
    cpssOsMemSet((GT_VOID*) &exMxPmPattern, 0, sizeof(exMxPmPattern));
    cpssOsMemSet((GT_VOID*) &exMxPmMask, 0, sizeof(exMxPmMask));
    cpssOsMemSet((GT_VOID*) &exMxPmTtiAction, 0, sizeof(exMxPmTtiAction));

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    /* set tcam type and tcam action type */
    tcamType   = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;
    actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    valid      = GT_TRUE;

    /* set rule index */
    if (extTcam == GT_FALSE)
    {
        ruleInfo.internalTcamRuleStartIndex = ruleIndex;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, ruleIndex,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }

        ruleInfo.externalTcamInfo.ruleInfo.ageEnable = GT_FALSE;
        ruleInfo.externalTcamInfo.ruleInfo.applicationInfo = 0;
        ruleInfo.externalTcamInfo.ruleInfo.ruleSize = CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E;
        ruleInfo.externalTcamInfo.ruleInfo.ruleStartIndex = ruleAlignedIndex;
        ruleInfo.externalTcamInfo.ruleInfo.tcamId = 0;

        ruleInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        ruleInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        ruleInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* the PCL-ID of the TTI lookup is build from 9 bits + 1 bit of LoopBack Port indication */
    /* so we do 'hide' of this info ... to let it look and fill like in DXCH devices */
    pclIdPattern = patternPtr->ipv4.common.pclId;
    PclIdMask    = maskPtr   ->ipv4.common.pclId;
    if(((pclIdPattern & PclIdMask) & (BIT_9)) == BIT_9)
    {
        /* can't support this option , because we use only 9 bits for PCL-id for TTI */
        return GT_BAD_PARAM;
    }
    pclIdPattern = (EXMXPM_TTI_PCL_ID_SHIFT_MAC(pclIdPattern)) & 0x3FF;
    PclIdMask    = (EXMXPM_TTI_PCL_ID_SHIFT_MAC(PclIdMask)) & 0x3FF;

    /* convert tti rule into device specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmIpv4TtiRule(&(patternPtr->ipv4),
                                                    &(exMxPmPattern.ipv4));
            /* convert IPv4 tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmIpv4TtiRule(&(maskPtr->ipv4),
                                                    &(exMxPmMask.ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmMplsTtiRule(&(patternPtr->mpls),
                                                    &(exMxPmPattern.mpls));
            /* convert Mpls tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmMplsTtiRule(&(maskPtr->mpls),
                                                    &(exMxPmMask.mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmEthTtiRule(&(patternPtr->eth),
                                                   &(exMxPmPattern.eth));
            /* convert Ethernet tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmEthTtiRule(&(maskPtr->eth),
                                                   &(exMxPmMask.eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert MIM tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmMimTtiRule(&(patternPtr->mim),
                                                   &(exMxPmPattern.mim));
            /* convert MIM tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmMimTtiRule(&(maskPtr->mim),
                                                   &(exMxPmMask.mim));
            break;

        default:
            return GT_BAD_PARAM;
    }

    exMxPmPattern.ipv4.common.pclId = pclIdPattern;
    exMxPmMask.ipv4.common.pclId    = PclIdMask;

    /* convert tti action into device specific format */
    rc = prvTgfConvertGenericToExMxPmTtiAction(actionPtr, &(exMxPmTtiAction.standard));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmTtiAction FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        exMxPmTtiInit(devNum);

        /* call device specific API */
        rc = cpssExMxPmTtiRuleSet(devNum, tcamType, &ruleInfo, (CPSS_EXMXPM_TTI_KEY_TYPE_ENT) ruleType,
                                  actionType, valid, &exMxPmMask, &exMxPmPattern, &exMxPmTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
*
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI rule type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT     ruleType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;
#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_TYPE_ENT               tcamType   = CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E;
    CPSS_EXMXPM_TTI_ACTION_TYPE_ENT         actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    GT_BOOL                                 valid      = GT_FALSE;
    CPSS_EXMXPM_TCAM_RULE_ACTION_INFO_UNT   ruleInfo;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmPattern;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmMask;
    CPSS_EXMXPM_TTI_ACTION_UNT              exMxPmTtiAction;
    GT_BOOL                                 extTcam;
    GT_U32                                  ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* initialize action, pattern and mask */
    cpssOsMemSet(&dxChTtiAction, 0, sizeof(dxChTtiAction));
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));


    /* prepare rule index */
    if (ttiIndexIsRelative == GT_TRUE)
    {
        ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
    }

    /* call device specific API */
    rc = cpssDxChTtiRuleGet(devNum, ruleIndex, (CPSS_DXCH_TTI_KEY_TYPE_ENT) ruleType,
                            &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule from device specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_RULE_IPV4_E:
            /* convert IPv4 tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChPattern.ipv4),
                                                  &(patternPtr->ipv4));
            /* convert IPv4 tti rule mask from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChMask.ipv4),
                                                  &(maskPtr->ipv4));
            break;

        case PRV_TGF_TTI_RULE_MPLS_E:
            /* convert Mpls tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChPattern.mpls),
                                                  &(patternPtr->mpls));
            /* convert Mpls tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChMask.mpls),
                                                  &(maskPtr->mpls));
            break;

        case PRV_TGF_TTI_RULE_ETH_E:
            /* convert Ethernet tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChPattern.eth),
                                                 &(patternPtr->eth));
            /* convert Ethernet tti rule mask from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChMask.eth),
                                                 &(maskPtr->eth));
            break;

        case PRV_TGF_TTI_RULE_MIM_E:
            /* convert Mac-in-Mac tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChPattern.mim),
                                                 &(patternPtr->mim));
            /* convert Mac-in-Mac tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChMask.mim),
                                                 &(maskPtr->mim));
            break;

#if 0 /* prevent warning: PRV_TGF_TTI_KEY_TRILL_E is not in ruleType enum */

        case PRV_TGF_TTI_KEY_TRILL_E:
            /* convert Trill tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericTrillTtiRule(&(dxChPattern.trill),
                                                 &(patternPtr->trill));
            /* convert Trill tti rule mask from device specific format */
            prvTgfConvertDxChToGenericTrillTtiRule(&(dxChMask.trill),
                                                 &(maskPtr->trill));
            break;
#endif

        case PRV_TGF_TTI_RULE_UDB_10_E:
        case PRV_TGF_TTI_RULE_UDB_20_E:
        case PRV_TGF_TTI_RULE_UDB_30_E:
            /* do nothing */
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti type2 action from device specific format */
    return prvTgfConvertDxChToGenericTtiAction2(&dxChTtiAction.type2, actionPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* reset variables */
    cpssOsMemSet((GT_VOID*) &ruleInfo, 0, sizeof(ruleInfo));

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    /* set tcam type and tcam action type */
    tcamType   = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;
    actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    valid      = GT_TRUE;

    /* set rule index */
    if (extTcam == GT_FALSE)
    {
        ruleInfo.internalTcamRuleStartIndex = ruleIndex;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, ruleIndex,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }

        ruleInfo.externalTcamInfo.ruleInfo.ageEnable = GT_FALSE;
        ruleInfo.externalTcamInfo.ruleInfo.applicationInfo = 0;
        ruleInfo.externalTcamInfo.ruleInfo.ruleSize = CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E;
        ruleInfo.externalTcamInfo.ruleInfo.ruleStartIndex = ruleAlignedIndex;
        ruleInfo.externalTcamInfo.ruleInfo.tcamId = 0;

        ruleInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        ruleInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        ruleInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* call device specific API */
    rc = cpssExMxPmTtiRuleGet(devNum, tcamType, &ruleInfo, (CPSS_EXMXPM_TTI_KEY_TYPE_ENT) ruleType,
                              actionType, &valid, &exMxPmMask, &exMxPmPattern, &exMxPmTtiAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule from device specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericIpv4TtiRule(&(exMxPmPattern.ipv4),
                                                    &(patternPtr->ipv4));
            /* convert IPv4 tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericIpv4TtiRule(&(exMxPmMask.ipv4),
                                                    &(maskPtr->ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericMplsTtiRule(&(exMxPmPattern.mpls),
                                                    &(patternPtr->mpls));
            /* convert Mpls tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericMplsTtiRule(&(exMxPmMask.mpls),
                                                    &(maskPtr->mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericEthTtiRule(&(exMxPmPattern.eth),
                                                   &(patternPtr->eth));
            /* convert Ethernet tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericEthTtiRule(&(exMxPmMask.eth),
                                                   &(maskPtr->eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert MIM tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericMimTtiRule(&(exMxPmPattern.mim),
                                                   &(patternPtr->mim));
            /* convert MIM tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericMimTtiRule(&(exMxPmMask.mim),
                                                   &(maskPtr->mim));
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti action from device specific format */
    return prvTgfConvertExMxPmToGenericTtiAction(&(exMxPmTtiAction.standard), actionPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfTtiRule2Set function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRule2Set
(
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    keyType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;
    PRV_TGF_TTI_RULE_TYPE_ENT       ruleType;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_TYPE_ENT               tcamType   = CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E;
    CPSS_EXMXPM_TTI_ACTION_TYPE_ENT         actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    GT_BOOL                                 valid      = GT_FALSE;
    CPSS_EXMXPM_TCAM_RULE_ACTION_INFO_UNT   ruleInfo;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmPattern;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmMask;
    CPSS_EXMXPM_TTI_ACTION_UNT              exMxPmTtiAction;
    GT_U32  pclIdPattern , PclIdMask;
    GT_BOOL                                 extTcam;
    GT_U32                                  ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    PRV_UTF_LOG2_MAC("prvTgfTtiRule2Set : (ruleIndex[%d],keyType[%d]) =======\n" ,
                        ruleIndex,
                        keyType);

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChTtiAction, 0, sizeof(dxChTtiAction));

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    prvTgfTtiConvertKeyTypeToRuleType(keyType,&ruleType);

    /* convert tti rule into DXCH specific format */
    rc = prvTgfTtiRuleConvertGenericToDx(ruleType,patternPtr,maskPtr,&dxChPattern,&dxChMask);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTtiRuleConvertGenericToDx FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericAction2ToDxChTtiAction2(actionPtr, &dxChTtiAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleSet(devNum, ruleIndex, ruleType,
                                &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* reset variables */
    cpssOsMemSet((GT_VOID*) &ruleInfo, 0, sizeof(ruleInfo));
    cpssOsMemSet((GT_VOID*) &exMxPmPattern, 0, sizeof(exMxPmPattern));
    cpssOsMemSet((GT_VOID*) &exMxPmMask, 0, sizeof(exMxPmMask));
    cpssOsMemSet((GT_VOID*) &exMxPmTtiAction, 0, sizeof(exMxPmTtiAction));

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    /* set tcam type and tcam action type */
    tcamType   = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;
    actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    valid      = GT_TRUE;

    /* set rule index */
    if (extTcam == GT_FALSE)
    {
        ruleInfo.internalTcamRuleStartIndex = ruleIndex;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, ruleIndex,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }

        ruleInfo.externalTcamInfo.ruleInfo.ageEnable = GT_FALSE;
        ruleInfo.externalTcamInfo.ruleInfo.applicationInfo = 0;
        ruleInfo.externalTcamInfo.ruleInfo.ruleSize = CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E;
        ruleInfo.externalTcamInfo.ruleInfo.ruleStartIndex = ruleAlignedIndex;
        ruleInfo.externalTcamInfo.ruleInfo.tcamId = 0;

        ruleInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        ruleInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        ruleInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* the PCL-ID of the TTI lookup is build from 9 bits + 1 bit of LoopBack Port indication */
    /* so we do 'hide' of this info ... to let it look and fill like in DXCH devices */
    pclIdPattern = patternPtr->ipv4.common.pclId;
    PclIdMask    = maskPtr   ->ipv4.common.pclId;
    if(((pclIdPattern & PclIdMask) & (BIT_9)) == BIT_9)
    {
        /* can't support this option , because we use only 9 bits for PCL-id for TTI */
        return GT_BAD_PARAM;
    }

    pclIdPattern = (EXMXPM_TTI_PCL_ID_SHIFT_MAC(pclIdPattern)) & 0x3FF;
    PclIdMask    = (EXMXPM_TTI_PCL_ID_SHIFT_MAC(PclIdMask)) & 0x3FF;

    /* convert tti rule into device specific format */
    switch (keyType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmIpv4TtiRule(&(patternPtr->ipv4),
                                                    &(exMxPmPattern.ipv4));
            /* convert IPv4 tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmIpv4TtiRule(&(maskPtr->ipv4),
                                                    &(exMxPmMask.ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmMplsTtiRule(&(patternPtr->mpls),
                                                    &(exMxPmPattern.mpls));
            /* convert Mpls tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmMplsTtiRule(&(maskPtr->mpls),
                                                    &(exMxPmMask.mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmEthTtiRule(&(patternPtr->eth),
                                                   &(exMxPmPattern.eth));
            /* convert Ethernet tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmEthTtiRule(&(maskPtr->eth),
                                                   &(exMxPmMask.eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert MIM tti rule pattern into device specific format */
            prvTgfConvertGenericToExMxPmMimTtiRule(&(patternPtr->mim),
                                                   &(exMxPmPattern.mim));
            /* convert MIM tti rule mask into device specific format */
            prvTgfConvertGenericToExMxPmMimTtiRule(&(maskPtr->mim),
                                                   &(exMxPmMask.mim));
            break;

        default:
            return GT_BAD_PARAM;
    }

    exMxPmPattern.ipv4.common.pclId = pclIdPattern;
    exMxPmMask.ipv4.common.pclId    = PclIdMask;

    /* convert tti action into device specific format */
    rc = prvTgfConvertGenericToExMxPmTtiAction2(actionPtr, &(exMxPmTtiAction.standard));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        exMxPmTtiInit(devNum);
        /* call device specific API */
        rc = cpssExMxPmTtiRuleSet(devNum, tcamType, &ruleInfo, (CPSS_EXMXPM_TTI_KEY_TYPE_ENT) keyType,
                                  actionType, valid, &exMxPmMask, &exMxPmPattern, &exMxPmTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiRule2Get function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRule2Get
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT     keyType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;
#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_TYPE_ENT               tcamType   = CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E;
    CPSS_EXMXPM_TTI_ACTION_TYPE_ENT         actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    GT_BOOL                                 valid      = GT_FALSE;
    CPSS_EXMXPM_TCAM_RULE_ACTION_INFO_UNT   ruleInfo;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmPattern;
    CPSS_EXMXPM_TTI_RULE_UNT                exMxPmMask;
    CPSS_EXMXPM_TTI_ACTION_UNT              exMxPmTtiAction;
    GT_BOOL                                 extTcam;
    GT_U32                                  ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* initialize action, pattern and mask */
    cpssOsMemSet(&dxChTtiAction, 0, sizeof(dxChTtiAction));
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));


    /* prepare rule index */
    if (ttiIndexIsRelative == GT_TRUE)
    {
        ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
    }

    /* call device specific API */
    rc = cpssDxChTtiRuleGet(devNum, ruleIndex, (CPSS_DXCH_TTI_KEY_TYPE_ENT) keyType,
                            &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule from device specific format */
    switch (keyType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChPattern.ipv4),
                                                  &(patternPtr->ipv4));
            /* convert IPv4 tti rule mask from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChMask.ipv4),
                                                  &(maskPtr->ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChPattern.mpls),
                                                  &(patternPtr->mpls));
            /* convert Mpls tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChMask.mpls),
                                                  &(maskPtr->mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChPattern.eth),
                                                 &(patternPtr->eth));
            /* convert Ethernet tti rule mask from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChMask.eth),
                                                 &(maskPtr->eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert Mac-in-Mac tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChPattern.mim),
                                                 &(patternPtr->mim));
            /* convert Mac-in-Mac tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChMask.mim),
                                                 &(maskPtr->mim));
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti type2 action from device specific format */
    return prvTgfConvertDxChAction2ToGenericTtiAction2(&dxChTtiAction.type2, actionPtr);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* reset variables */
    cpssOsMemSet((GT_VOID*) &ruleInfo, 0, sizeof(ruleInfo));

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    /* set tcam type and tcam action type */
    tcamType   = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;
    actionType = CPSS_EXMXPM_TTI_ACTION_STANDARD_E;
    valid      = GT_TRUE;

    /* set rule index */
    if (extTcam == GT_FALSE)
    {
        ruleInfo.internalTcamRuleStartIndex = ruleIndex;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, ruleIndex,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }

        ruleInfo.externalTcamInfo.ruleInfo.ageEnable = GT_FALSE;
        ruleInfo.externalTcamInfo.ruleInfo.applicationInfo = 0;
        ruleInfo.externalTcamInfo.ruleInfo.ruleSize = CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E;
        ruleInfo.externalTcamInfo.ruleInfo.ruleStartIndex = ruleAlignedIndex;
        ruleInfo.externalTcamInfo.ruleInfo.tcamId = 0;

        ruleInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        ruleInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        ruleInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* call device specific API */
    rc = cpssExMxPmTtiRuleGet(devNum, tcamType, &ruleInfo, (CPSS_EXMXPM_TTI_KEY_TYPE_ENT) keyType,
                              actionType, &valid, &exMxPmMask, &exMxPmPattern, &exMxPmTtiAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule from device specific format */
    switch (keyType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericIpv4TtiRule(&(exMxPmPattern.ipv4),
                                                    &(patternPtr->ipv4));
            /* convert IPv4 tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericIpv4TtiRule(&(exMxPmMask.ipv4),
                                                    &(maskPtr->ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericMplsTtiRule(&(exMxPmPattern.mpls),
                                                    &(patternPtr->mpls));
            /* convert Mpls tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericMplsTtiRule(&(exMxPmMask.mpls),
                                                    &(maskPtr->mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericEthTtiRule(&(exMxPmPattern.eth),
                                                   &(patternPtr->eth));
            /* convert Ethernet tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericEthTtiRule(&(exMxPmMask.eth),
                                                   &(maskPtr->eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert MIM tti rule pattern from device specific format */
            prvTgfConvertExMxPmToGenericMimTtiRule(&(exMxPmPattern.mim),
                                                   &(patternPtr->mim));
            /* convert MIM tti rule mask from device specific format */
            prvTgfConvertExMxPmToGenericMimTtiRule(&(exMxPmMask.mim),
                                                   &(maskPtr->mim));
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti action from device specific format */
    return prvTgfConvertExMxPmToGenericTtiAction2(&(exMxPmTtiAction.standard), actionPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiRuleUdbSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and
*          Action Acording to UDB rule type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleUdbSet
(
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                           devNum  = 0;
    GT_STATUS                       rc, rc1 = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    PRV_UTF_LOG2_MAC("prvTgfTtiRuleUdbSet : (ruleIndex[%d],ruleType[%d]) =======\n" ,
                        ruleIndex,
                        ruleType);

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChTtiAction, 0, sizeof(dxChTtiAction));

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule into DXCH specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_RULE_UDB_10_E:
        case PRV_TGF_TTI_RULE_UDB_20_E:
        case PRV_TGF_TTI_RULE_UDB_30_E:
            /* convert UDB tti rule pattern into device specific format */
            cpssOsMemCpy(dxChPattern.udbArray, patternPtr->udbArray, sizeof(dxChPattern.udbArray));

            /* convert UDB tti rule mask into device specific format */
            cpssOsMemCpy(dxChMask.udbArray, maskPtr->udbArray, sizeof(dxChMask.udbArray));
            break;
        default:
            PRV_UTF_LOG1_MAC("[TGF]: ruleType illegal FAILED, rc = [%d]", rc);
            return GT_BAD_PARAM;
    }

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericAction2ToDxChTtiAction2(actionPtr, &dxChTtiAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleSet(devNum, ruleIndex, ruleType,
                                &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(ruleIndex);
    TGF_PARAM_NOT_USED(ruleType);
    TGF_PARAM_NOT_USED(patternPtr);
    TGF_PARAM_NOT_USED(maskPtr);
    TGF_PARAM_NOT_USED(actionPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiRuleUdbGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and
*          Action Acording to UDB rule type
*
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI UDB rule type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleUdbGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    ruleType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* initialize action, pattern and mask */
    cpssOsMemSet(&dxChTtiAction, 0, sizeof(dxChTtiAction));
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));


    /* prepare rule index */
    if (ttiIndexIsRelative == GT_TRUE)
    {
        ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
    }

     /* convert tti rule from device specific format */
    switch (ruleType)
    {
        case PRV_TGF_TTI_RULE_IPV4_E:
        case PRV_TGF_TTI_RULE_MPLS_E:
        case PRV_TGF_TTI_RULE_ETH_E:
        case PRV_TGF_TTI_RULE_MIM_E:
            PRV_UTF_LOG1_MAC("[TGF]: ruleType illegal FAILED, rc = [%d]", rc);
            return GT_BAD_PARAM;

        case PRV_TGF_TTI_RULE_UDB_10_E:
        case PRV_TGF_TTI_RULE_UDB_20_E:
        case PRV_TGF_TTI_RULE_UDB_30_E:
            /* call device specific API */
            rc = cpssDxChTtiRuleGet(devNum, ruleIndex, ruleType,
                                    &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleGet FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert DXCH specific format into tti rule into  */
    switch (ruleType)
    {
        case PRV_TGF_TTI_RULE_UDB_10_E:
        case PRV_TGF_TTI_RULE_UDB_20_E:
        case PRV_TGF_TTI_RULE_UDB_30_E:
            /* convert UDB tti rule pattern into device specific format */
            cpssOsMemCpy(patternPtr->udbArray, dxChPattern.udbArray,sizeof(dxChPattern.udbArray));

            /* convert UDB tti rule mask into device specific format */
            cpssOsMemCpy(maskPtr->udbArray, dxChMask.udbArray, sizeof(dxChMask.udbArray));
            break;
        default:
            PRV_UTF_LOG1_MAC("[TGF]: ruleType illegal FAILED, rc = [%d]", rc);
            return GT_BAD_PARAM;
    }

    /* convert tti type2 action from device specific format */
    return prvTgfConvertDxChAction2ToGenericTtiAction2(&dxChTtiAction.type2, actionPtr);

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ruleIndex);
    TGF_PARAM_NOT_USED(ruleType);
    TGF_PARAM_NOT_USED(patternPtr);
    TGF_PARAM_NOT_USED(maskPtr);
    TGF_PARAM_NOT_USED(actionPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS prvTgfTtiRuleValidStatusSet
(
    IN  GT_U32                        routerTtiTcamRow,
    IN  GT_BOOL                       valid
)
{
    GT_U8       devNum = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TCAM_RULE_ACTION_INFO_UNT    ruleActionInfo = {0};
    GT_BOOL                                  extTcam;
    CPSS_EXMXPM_TCAM_TYPE_ENT                tcamType;
    GT_U32                                   ruleAlignedIndex, actionAlignedIndex;
#endif /* EXMXPM_FAMILY */

#if 0  /* must be commented, needs only for manual debug - prevent huge tests log files */
    /* log function call with parameters, used for debug */
    PRV_UTF_LOG2_MAC("prvTgfTtiRuleValidStatusSet : (ruleIndex[%d], valid[%d]) =======\n" ,
                        routerTtiTcamRow, valid);
#endif

#ifdef CHX_FAMILY
    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
        }

        /* call device specific API */
        rc = cpssDxChTtiRuleValidStatusSet(devNum, routerTtiTcamRow, valid);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMimEthTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    extTcam = prvTgfCommonIsExternalTcamUseForced();

    tcamType = (extTcam == GT_FALSE)
        ? CPSS_EXMXPM_TCAM_TYPE_INTERNAL_E
        : CPSS_EXMXPM_TCAM_TYPE_EXTERNAL_E;

    /* configure rule action info */
    if (extTcam == GT_FALSE)
    {
    ruleActionInfo.internalTcamRuleStartIndex = routerTtiTcamRow;
    }
    else
    {
        rc = cmdAppExMxPmExternalTcamRuleAndActionIndexGet(
            devNum, CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E,
            0 /*lookupIndex*/, routerTtiTcamRow,
            &ruleAlignedIndex, &actionAlignedIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cmdAppExMxPmExternalTcamRuleAndActionIndexGet FAILED, rc = [%d]", rc);
            return rc;
        }

        ruleActionInfo.externalTcamInfo.ruleInfo.ageEnable = GT_FALSE;
        ruleActionInfo.externalTcamInfo.ruleInfo.applicationInfo = 0;
        ruleActionInfo.externalTcamInfo.ruleInfo.ruleSize = CPSS_EXMXPM_EXTERNAL_TCAM_RULE_SIZE_160_E;
        ruleActionInfo.externalTcamInfo.ruleInfo.ruleStartIndex = ruleAlignedIndex;
        ruleActionInfo.externalTcamInfo.ruleInfo.tcamId = 0;

        ruleActionInfo.externalTcamInfo.actionInfo.actionStartIndex = actionAlignedIndex;
        ruleActionInfo.externalTcamInfo.actionInfo.memoryType = CPSS_EXMXPM_CONTROL_MEMORY_0_E;
        ruleActionInfo.externalTcamInfo.actionInfo.applicationInfo = 0;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiRuleValidStatusSet(
            devNum, tcamType, &ruleActionInfo, valid);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiRuleValidStatusSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTunnelStartEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
*/
GT_STATUS prvTgfTunnelStartEntrySet
(
    IN  GT_U32                          index,
    IN  CPSS_TUNNEL_TYPE_ENT            tunnelType,
    IN  PRV_TGF_TUNNEL_START_ENTRY_UNT *configPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    #ifdef CHX_FAMILY
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT dxChTunnelStartEntry = {{0}};

    switch (tunnelType)
    {
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            /* convert tunnel start Ipv4 entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartIpv4Entry(&(configPtr->ipv4Cfg), &(dxChTunnelStartEntry.ipv4Cfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartIpv4Entry FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case CPSS_TUNNEL_GENERIC_IPV6_E:
            /* convert tunnel start Ipv4 entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartIpv6Entry(&(configPtr->ipv6Cfg), &(dxChTunnelStartEntry.ipv6Cfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartIpv6Entry FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case CPSS_TUNNEL_X_OVER_MPLS_E:
            /* convert tunnel start Mpls entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartMplsEntry(devNum, &(configPtr->mplsCfg), &(dxChTunnelStartEntry.mplsCfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartMplsEntry FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case CPSS_TUNNEL_MAC_IN_MAC_E:
            /* convert tunnel start Mim entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartMimEntry(&(configPtr->mimCfg), &(dxChTunnelStartEntry.mimCfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartMimEntry FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case CPSS_TUNNEL_TRILL_E:
            /* convert tunnel start TRILL entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartTrillEntry(&(configPtr->trillCfg), &(dxChTunnelStartEntry.trillCfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartTrillEntry FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case CPSS_TUNNEL_GENERIC_E:
            /* convert tunnel start TRILL entry into device specific format */
            rc = prvTgfConvertGenericToDxChTunnelStartGenericEntry(&(configPtr->genCfg), &(dxChTunnelStartEntry.genCfg));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartGenericEntry FAILED, rc = [%d]", rc);

                return rc;
            }
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
        rc = cpssDxChTunnelStartEntrySet(devNum, index, tunnelType, &dxChTunnelStartEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartEntrySet FAILED, rc = [%d]", rc);

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
* @internal prvTgfTunnelStartEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - line  for the tunnel start entry
* @param[in,out] tunnelTypePtr            - (pointer to) the type of the tunnel
*
* @param[out] configPtr                - (pointer to) tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfTunnelStartEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    INOUT CPSS_TUNNEL_TYPE_ENT           *tunnelTypePtr,
    OUT PRV_TGF_TUNNEL_START_ENTRY_UNT *configPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT dxChTunnelStartEntry = {{0}};

    /* call device specific API */
    rc = cpssDxChTunnelStartEntryGet(devNum, index, tunnelTypePtr, &dxChTunnelStartEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch (*tunnelTypePtr)
    {
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            /* convert tunnel start Ipv4 entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartIpv4Entry(&(dxChTunnelStartEntry.ipv4Cfg), &(configPtr->ipv4Cfg));
        case CPSS_TUNNEL_GENERIC_IPV6_E:
            /* convert tunnel start Ipv6 entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartIpv6Entry(&(dxChTunnelStartEntry.ipv6Cfg), &(configPtr->ipv6Cfg));
        case CPSS_TUNNEL_X_OVER_MPLS_E:
            /* convert tunnel start Mpls entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartMplsEntry(devNum, &(dxChTunnelStartEntry.mplsCfg), &(configPtr->mplsCfg));
        case CPSS_TUNNEL_MAC_IN_MAC_E:
            /* convert tunnel start Mim entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartMimEntry(&(dxChTunnelStartEntry.mimCfg), &(configPtr->mimCfg));
        case CPSS_TUNNEL_TRILL_E:
            /* convert tunnel start TRILL entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartTrillEntry(&(dxChTunnelStartEntry.trillCfg), &(configPtr->trillCfg));
        case CPSS_TUNNEL_GENERIC_E:
            /* convert tunnel start generic entry from device specific format */
            return prvTgfConvertDxChToGenericTunnelStartGenericEntry(&(dxChTunnelStartEntry.genCfg), &(configPtr->genCfg));
        default:
            return GT_BAD_PARAM;
    }
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfTunnelStartGenProfileTableEntrySet function
* @endinternal
*
* @brief   Set Generic Tunnel-start Profile table entry .
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/

GT_STATUS prvTgfTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN  PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC         *profileDataPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC    dxChProfileEntry = {0};
    /* convert tunnel start profile entry into device specific format */
    rc = prvTgfConvertGenericToDxChTunnelStartProfileEntry(profileDataPtr, &dxChProfileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTunnelStartProfileEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTunnelStartGenProfileTableEntrySet(devNum, profileIndex, &dxChProfileEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartGenProfileTableEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;

#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileIndex);
    TGF_PARAM_NOT_USED(profileDataPtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTunnelStartGenProfileTableEntryGet function
* @endinternal
*
* @brief   Get Generic Tunnel-start Profile table entry .
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*
* @param[out] profileDataPtr           - (pointer to) tunnel start profile configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfTunnelStartGenProfileTableEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileIndex,
    OUT PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC *profileDataPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC    dxChProfileEntry = {0};

    /* call device specific API */
    rc = cpssDxChTunnelStartGenProfileTableEntryGet(devNum, profileIndex, &dxChProfileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartGenProfileTableEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }
    /* convert tunnel start profile entry from device specific format */
    return prvTgfConvertDxChToGenericTunnelStartProfileEntry(&dxChProfileEntry, profileDataPtr);

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileIndex);
    TGF_PARAM_NOT_USED(profileDataPtr);
    return GT_BAD_STATE;
#endif
}



/**
* @internal prvTgfTunnelStartMimITagEtherTypeSet function
* @endinternal
*
* @brief   Sets globally the MAC in MAC I-Tag Ethertype (802.1ah).
*         This is the Backbone Service Instance TAG identified by
*         a 802.1ah Ethertype
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMimITagEtherTypeSet
(
    IN  GT_U32  iTagEthType
)
{

#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(iTagEthType);

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc = GT_OK;
    GT_STATUS   rc1= GT_OK;
    GT_U8       devNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTunnelStartMimITagEtherTypeSet(devNum, iTagEthType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTunnelStartMimITagEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfTunnelStartMimITagEtherTypeGet function
* @endinternal
*
* @brief   Gets the MAC in MAC I-Tag Ethertype (802.1ah).
*         This is the Backbone Service Instance TAG identified by
*         a 802.1ah Ethertype.
* @param[in] devNum                   - device number
*
* @param[out] iTagEthTypePtr           - i-tag (instance tag) ethertype
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMimITagEtherTypeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *iTagEthTypePtr
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(iTagEthTypePtr);

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
   return cpssExMxPmTunnelStartMimITagEtherTypeGet(devNum, iTagEthTypePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortLookupEnableSet
(
    IN  GT_U32                         portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_BOOL                       enable
)
{
    GT_U32  portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TTI_KEY_TYPE_ENT    exMxPmKeyType = CPSS_EXMXPM_TTI_KEY_IPV4_E;
    GT_BOOL                         extTcam;
    GT_STATUS                       rc;
#endif /* EXMXPM_FAMILY */

    PRV_UTF_LOG3_MAC("prvTgfTtiPortLookupEnableSet(%d,%d,%d) \n",
        portNum,keyType,enable);

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
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    return cpssDxChTtiPortLookupEnableSet(prvTgfDevsArray[portIter], portNum, dxChKeyType, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(exMxPmKeyType, keyType);

    extTcam = prvTgfCommonIsExternalTcamUseForced();
    if (extTcam != GT_FALSE)
    {
        rc = cpssExMxPmTtiPortLookupModeSet(
            prvTgfDevsArray[portIter],  portNum, exMxPmKeyType,
            ((enable == GT_FALSE)
                ? CPSS_EXMXPM_TTI_LOOKUP_MODE_INT_TCAM_E
                : CPSS_EXMXPM_TTI_LOOKUP_MODE_EXT_TCAM_E));
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiPortLookupModeSet failed", rc);
            return rc;
        }
    }

    /* call device specific API */
    return cpssExMxPmTtiPortLookupEnableSet(prvTgfDevsArray[portIter], portNum, exMxPmKeyType, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @param[out] enablePtr                - points to enable/disable TTI lookup
*                                      GT_TRUE: TTI lookup is enabled
*                                      GT_FALSE: TTI lookup is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortLookupEnableGet
(
    IN  GT_U32                         portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_U32  portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TTI_KEY_TYPE_ENT    exMxPmKeyType = CPSS_EXMXPM_TTI_KEY_IPV4_E;
#endif /* EXMXPM_FAMILY */

    PRV_UTF_LOG2_MAC("prvTgfTtiPortLookupEnableGet(%d,%d) \n",
        portNum,keyType);

    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (portIter >= prvTgfPortsNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    return cpssDxChTtiPortLookupEnableGet(prvTgfDevsArray[portIter], portNum, dxChKeyType, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(exMxPmKeyType, keyType);

    /* call device specific API */
    return cpssExMxPmTtiPortLookupEnableGet(prvTgfDevsArray[portIter], portNum, exMxPmKeyType, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMacModeSet
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT      macMode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT      dxChMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TTI_KEY_TYPE_ENT    exMxPmKeyType = CPSS_EXMXPM_TTI_KEY_IPV4_E;
    CPSS_EXMXPM_TTI_MAC_MODE_ENT    exMxPmMacMode = CPSS_EXMXPM_TTI_MAC_MODE_DA_E;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* convert macMode into device specific format */
    PRV_TGF_S2D_MAC_MODE_CONVERT_MAC(dxChMacMode, macMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTtiMacModeSet(devNum, dxChKeyType, dxChMacMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMacModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(exMxPmKeyType, keyType);

    /* convert macMode into device specific format */
    PRV_TGF_S2D_MAC_MODE_CONVERT_MAC(exMxPmMacMode, macMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiMacModeSet(devNum, exMxPmKeyType, exMxPmMacMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiMacModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
*
* @param[out] macModePtr               - (pointer to) MAC mode to use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMacModeGet
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_MAC_MODE_ENT     *macModePtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT      dxChMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TTI_KEY_TYPE_ENT    exMxPmKeyType = CPSS_EXMXPM_TTI_KEY_IPV4_E;
    CPSS_EXMXPM_TTI_MAC_MODE_ENT    exMxPmMacMode = CPSS_EXMXPM_TTI_MAC_MODE_DA_E;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    rc = cpssDxChTtiMacModeGet(devNum, dxChKeyType, &dxChMacMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMacModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert macMode from device specific format */
    PRV_TGF_D2S_MAC_MODE_CONVERT_MAC(dxChMacMode, *macModePtr);

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(exMxPmKeyType, keyType);

    /* call device specific API */
    rc = cpssExMxPmTtiMacModeGet(devNum, exMxPmKeyType, &exMxPmMacMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiMacModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert macMode from device specific format */
    PRV_TGF_D2S_MAC_MODE_CONVERT_MAC(exMxPmMacMode, *macModePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U32                         portNum,
    IN  GT_BOOL                       enable
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
    return cpssDxChTtiPortIpv4OnlyTunneledEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmTtiPortIpv4OnlyTunneledEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiMimEthTypeSet function
* @endinternal
*
* @brief   This function sets the MIM Ethernet type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMimEthTypeSet
(
    IN  GT_U32                        ethType
)
{
    GT_U8       devNum = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    PRV_UTF_LOG1_MAC("prvTgfTtiMimEthTypeSet(0x%x) \n",
        ethType);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTtiEthernetTypeSet(devNum, CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E, ethType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMimEthTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiMimEtherTypeSet(devNum, ethType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiMimEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiMacToMeSet function
* @endinternal
*
* @brief   This function sets the TTI MacToMe relevant Mac address and Vlan.
*         if a match is found, an internal flag is set. The MACTOME flag
*         is optionally used as part of the TT lookup trigger for IPV4 or MPLS traffic.
*         Note: if the packet is TT and the pasenger is Ethernet, another MACTOME
*         lookup is performed and the internal flag is set accordingly.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiMacToMeSet
(
    IN  GT_U32                            entryIndex,
    IN  PRV_TGF_TTI_MAC_VLAN_STC         *valuePtr,
    IN  PRV_TGF_TTI_MAC_VLAN_STC         *maskPtr
)
{
    GT_U8       devNum = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
{
    CPSS_DXCH_TTI_MAC_VLAN_STC dxchValue;
    CPSS_DXCH_TTI_MAC_VLAN_STC dxchMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  dxchInterfaceInfo;

    dxchValue.mac = valuePtr->mac;
    dxchValue.vlanId = valuePtr->vlanId;
    dxchMask.mac = maskPtr->mac;
    dxchMask.vlanId = maskPtr->vlanId;
    dxchInterfaceInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTtiMacToMeSet(devNum, entryIndex, &dxchValue, &dxchMask, &dxchInterfaceInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMacToMeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
}
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
{

    CPSS_EXMXPM_TTI_MAC_VLAN_STC exmxpmValue;
    CPSS_EXMXPM_TTI_MAC_VLAN_STC exmxpmMask;

    exmxpmValue.mac = valuePtr->mac;
    exmxpmValue.vlanId = valuePtr->vlanId;
    exmxpmMask.mac = maskPtr->mac;
    exmxpmMask.vlanId = maskPtr->vlanId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTtiMacToMeSet(devNum, entryIndex, GT_TRUE, &exmxpmValue, &exmxpmMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTtiMacToMeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
}
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTtiPortGroupRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupRuleSet
(
    IN  GT_U32                        ruleIndex,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;
    PRV_TGF_TTI_RULE_TYPE_ENT       ruleType;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChPattern,   0, sizeof(dxChPattern));
    cpssOsMemSet((GT_VOID*) &dxChMask,      0, sizeof(dxChMask));
    cpssOsMemSet((GT_VOID*) &dxChTtiAction, 0, sizeof(dxChTtiAction));

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    prvTgfTtiConvertKeyTypeToRuleType(keyType,&ruleType);

    /* convert tti rule into DXCH specific format */
    rc = prvTgfTtiRuleConvertGenericToDx(ruleType,patternPtr,maskPtr,&dxChPattern,&dxChMask);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTtiRuleConvertGenericToDx FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti action type2 into device specific format */
    rc = prvTgfConvertGenericToDxChTtiAction2(actionPtr, &dxChTtiAction.type2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTtiAction2 FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
        }

        if(GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        {
            if ((portGroupsBmp & 0xF) && (portGroupsBmp & 0xF0))
            {
                portGroupsBmp = 0xFF;
            }
            else if (portGroupsBmp & 0xF)
            {
                portGroupsBmp = 0xF;
            }
            else if (portGroupsBmp & 0xF0)
            {
                portGroupsBmp = 0xF0;
            }
            else
            {
                PRV_UTF_LOG0_MAC("[TGF]: prvTgfTtiPortGroupRuleSet port group convertion for earch devices FAILED");
                return GT_BAD_PARAM;
            }
        }

        /* call device specific API */
        rc = cpssDxChTtiPortGroupRuleSet(devNum, portGroupsBmp, ruleIndex, (CPSS_DXCH_TTI_KEY_TYPE_ENT) keyType,
                                &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ruleIndex);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(patternPtr);
    TGF_PARAM_NOT_USED(maskPtr);
    TGF_PARAM_NOT_USED(actionPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfTtiPortGroupRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
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
* @param[in] keyType                  - TTI key type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupRuleGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_STC       *actionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_PP_FAMILY_TYPE_ENT         devFamily  = CPSS_PP_FAMILY_START_DXCH_E;
    CPSS_DXCH_TTI_ACTION_TYPE_ENT   actionType = CPSS_DXCH_TTI_ACTION_TYPE2_ENT;
    CPSS_DXCH_TTI_RULE_UNT          dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT          dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT        dxChTtiAction;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* initialize action, pattern and mask */
    cpssOsMemSet(&dxChTtiAction, 0, sizeof(dxChTtiAction));
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));

    /* prepare rule index */
    if (ttiIndexIsRelative == GT_TRUE)
    {
        ruleIndex = appDemoDxChTcamTtiConvertedIndexGet(devNum,ruleIndex);
    }

    if(GT_TRUE == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        if ((portGroupsBmp & 0xF) && (portGroupsBmp & 0xF0))
        {
            portGroupsBmp = 0xFF;
        }
        else if (portGroupsBmp & 0xF)
        {
            portGroupsBmp = 0xF;
        }
        else if (portGroupsBmp & 0xF0)
        {
            portGroupsBmp = 0xF0;
        }
        else
        {
            PRV_UTF_LOG0_MAC("[TGF]: prvTgfTtiPortGroupRuleGet port group convertion for earch devices FAILED");
            return GT_BAD_PARAM;
        }
    }

    /* call device specific API */
    rc = cpssDxChTtiPortGroupRuleGet(devNum, portGroupsBmp, ruleIndex, (CPSS_DXCH_TTI_KEY_TYPE_ENT) keyType,
                            &dxChPattern, &dxChMask, actionType, &dxChTtiAction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiRuleGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert tti rule from device specific format */
    switch (keyType)
    {
        case PRV_TGF_TTI_KEY_IPV4_E:
            /* convert IPv4 tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChPattern.ipv4),
                                                  &(patternPtr->ipv4));
            /* convert IPv4 tti rule mask from device specific format */
            prvTgfConvertDxChToGenericIpv4TtiRule(&(dxChMask.ipv4),
                                                  &(maskPtr->ipv4));
            break;

        case PRV_TGF_TTI_KEY_MPLS_E:
            /* convert Mpls tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChPattern.mpls),
                                                  &(patternPtr->mpls));
            /* convert Mpls tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMplsTtiRule(&(dxChMask.mpls),
                                                  &(maskPtr->mpls));
            break;

        case PRV_TGF_TTI_KEY_ETH_E:
            /* convert Ethernet tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChPattern.eth),
                                                 &(patternPtr->eth));
            /* convert Ethernet tti rule mask from device specific format */
            prvTgfConvertDxChToGenericEthTtiRule(&(dxChMask.eth),
                                                 &(maskPtr->eth));
            break;

        case PRV_TGF_TTI_KEY_MIM_E:
            /* convert Mac-in-Mac tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChPattern.mim),
                                                 &(patternPtr->mim));
            /* convert Mac-in-Mac tti rule mask from device specific format */
            prvTgfConvertDxChToGenericMimTtiRule(&(dxChMask.mim),
                                                 &(maskPtr->mim));
            break;

        case PRV_TGF_TTI_KEY_TRILL_E:
            /* convert Trill tti rule pattern from device specific format */
            prvTgfConvertDxChToGenericTrillTtiRule(&(dxChPattern.trill),
                                                 &(patternPtr->trill));
            /* convert Trill tti rule mask from device specific format */
            prvTgfConvertDxChToGenericTrillTtiRule(&(dxChMask.trill),
                                                 &(maskPtr->trill));
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert tti type2 action from device specific format */
    return prvTgfConvertDxChToGenericTtiAction2(&dxChTtiAction.type2, actionPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ruleIndex);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(patternPtr);
    TGF_PARAM_NOT_USED(maskPtr);
    TGF_PARAM_NOT_USED(actionPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfTtiPortGroupMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupMacModeSet
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_MAC_MODE_ENT      macMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT      dxChMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* convert macMode into device specific format */
    PRV_TGF_S2D_MAC_MODE_CONVERT_MAC(dxChMacMode, macMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, dxChKeyType, dxChMacMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMacModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(macMode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfTtiPortGroupMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type
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
* @param[in] keyType                  - TTI key type
*
* @param[out] macModePtr               - (pointer to) MAC mode to use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupMacModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_MAC_MODE_ENT     *macModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_MAC_MODE_ENT      dxChMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    rc = cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, dxChKeyType, &dxChMacMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMacModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert macMode from device specific format */
    PRV_TGF_D2S_MAC_MODE_CONVERT_MAC(dxChMacMode, *macModePtr);

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(macModePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfTtiPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS prvTgfTtiPortGroupRuleValidStatusSet
(
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        routerTtiTcamRow,
    IN  GT_BOOL                       valid
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    appDemoDxChNewTtiTcamSupportSet(ttiIndexIsRelative);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare rule index */
        if (ttiIndexIsRelative == GT_TRUE)
        {
            routerTtiTcamRow = appDemoDxChTcamTtiConvertedIndexGet(devNum,routerTtiTcamRow);
        }

        /* call device specific API */
        rc = cpssDxChTtiPortGroupRuleValidStatusSet(devNum, portGroupsBmp, routerTtiTcamRow, valid);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiMimEthTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(routerTtiTcamRow);
    TGF_PARAM_NOT_USED(valid);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgTtiCheck function
* @endinternal
*
* @brief   check if the device supports the tti
*
* @retval GT_TRUE                  - the device supports the tti
* @retval GT_FALSE                 - the device not supports the tti
*/
GT_BOOL prvTgfBrgTtiCheck(
    void
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum = prvTgfDevNum;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm == 0)
    {
        /* the device not support the tti */
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_TRUE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfTtiIpv4GreEthTypeSet function
* @endinternal
*
* @brief   This function sets the IPv4 GRE protocol Ethernet type.
*
* @param[in] devNum                   - device number
* @param[in] greTunnelType            - GRE tunnel type
* @param[in] ethType                  - Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4GreEthTypeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT   greTunnelType,
    IN  GT_U32                              ethType
)
{

#ifdef EXMXPM_FAMILY
    CPSS_TUNNEL_GRE_ETHER_TYPE_ENT exMxPmGreTunnelType;

    /* convert greTunnelType into device specific format */
    switch (greTunnelType)
    {
        case PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E:
            exMxPmGreTunnelType = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
            break;

        case PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E:
            exMxPmGreTunnelType = CPSS_TUNNEL_GRE1_ETHER_TYPE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmTtiIpv4GreEthTypeSet(devNum, exMxPmGreTunnelType, ethType);
#endif /* EXMXPM_FAMILY */

#if (!defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(greTunnelType);
    TGF_PARAM_NOT_USED(ethType);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfTtiIpv4GreEthTypeGet function
* @endinternal
*
* @brief   This function gets the IPv4 GRE protocol Ethernet type.
*
* @param[in] devNum                   - device number
* @param[in] greTunnelType            - GRE tunnel type
*
* @param[out] ethTypePtr               - (pointer to) Ethernet type value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4GreEthTypeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT   greTunnelType,
    OUT GT_U32                              *ethTypePtr
)
{

#ifdef EXMXPM_FAMILY
    CPSS_TUNNEL_GRE_ETHER_TYPE_ENT exMxPmGreTunnelType;

    /* convert greTunnelType into device specific format */
    switch (greTunnelType)
    {
        case PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E:
            exMxPmGreTunnelType = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
            break;

        case PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E:
            exMxPmGreTunnelType = CPSS_TUNNEL_GRE1_ETHER_TYPE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmTtiIpv4GreEthTypeGet(devNum, exMxPmGreTunnelType, ethTypePtr);
#endif /* EXMXPM_FAMILY */

#if (!defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(greTunnelType);
    TGF_PARAM_NOT_USED(ethTypePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
* @param[in] devNum                   - device number
* @param[in] ethertype                - Ethernet type
* @param[in] ethertype                - Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS prvTgfTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT   ethertypeType,
    IN  GT_U32                              ethertype
)
{
#ifdef CHX_FAMILY
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT dxChEthertypeType;

    /* convert greTunnelType into device specific format */
    switch (ethertypeType)
    {
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MIM_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChTtiEthernetTypeSet(devNum, dxChEthertypeType, ethertype);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ethertypeType);
    TGF_PARAM_NOT_USED(ethertype);
    return GT_BAD_PARAM;
#endif
}

/**
* @internal prvTgfTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
* @param[in] devNum                   - device number
* @param[in] ethertypeType            - Ethernet type
*
* @param[out] ethertypePtr             - Points to Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS prvTgfTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT   ethertypeType,
    OUT GT_U32                              *ethertypePtr
)
{
#ifdef CHX_FAMILY
    CPSS_TUNNEL_ETHERTYPE_TYPE_ENT dxChEthertypeType;

    /* convert greTunnelType into device specific format */
    switch (ethertypeType)
    {
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MIM_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E;
        break;
    case PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E:
        dxChEthertypeType = CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E;
        break;

    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChTtiEthernetTypeGet(devNum, dxChEthertypeType, ethertypePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ethertypeType);
    TGF_PARAM_NOT_USED(ethertypePtr);
    return GT_BAD_PARAM;
#endif
}
/**
* @internal prvTgfTunnelStartPortIpTunnelTotalLengthOffsetEnableSet function
* @endinternal
*
* @brief   This feature allows overriding the <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32   port,
    IN  GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(devNum, port, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(port);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelStartIpTunnelTotalLengthOffsetSet function
* @endinternal
*
* @brief   This API sets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
* @param[in] devNum                   - device number
* @param[in] additionToLength         - Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range values
*/
GT_STATUS prvTgfTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 additionToLength
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(devNum, additionToLength);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(additionToLength);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortTrillEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TRILL engine on the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  TRILL engine on port
*                                      GT_FALSE: disable TRILL engine on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortTrillEnableGet function
* @endinternal
*
* @brief   This function gets the current state enables/disables of TRILL engine
*         on the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to enable/disable TRILL
*                                      GT_TRUE:  TRILL engine enabled on port
*                                      GT_FALSE: TRILL engine disabled on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortTrillEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillAdjacencyCheckEntrySet function
* @endinternal
*
* @brief   This function sets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM (0..255)
* @param[in] ttiTrillAdjacencyPtr     - points to TRILL Adjacency STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  PRV_TGF_TTI_TRILL_ADJACENCY_STC    *ttiTrillAdjacencyPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   dxChValue;
    GT_STATUS rc;

    /* convert tti trill adjacency check into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, outerMacSa);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, trillMBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, trillEgressRbid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcHwDevice);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcIsTrunk);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcPortTrunk);

    /* covert the SW device to HW devNum */
    rc = prvUtfHwDeviceNumberGet((GT_U8)dxChValue.srcHwDevice,
                                 &dxChValue.srcHwDevice);
    if (rc != GT_OK)
        return rc;

    /* call device specific API */
    return cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, &dxChValue);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(ttiTrillAdjacencyPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillAdjacencyCheckEntryGet function
* @endinternal
*
* @brief   This function gets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM (0..255)
*
* @param[out] ttiTrillAdjacencyPtr     - points to TRILL Adjacency STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT PRV_TGF_TTI_TRILL_ADJACENCY_STC    *ttiTrillAdjacencyPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   dxChValue;
    GT_STATUS rc;

    /* call device specific API */
    rc = cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, &dxChValue);
    if (rc != GT_OK)
        return rc;

    /* convert tti trill adjacency check from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, outerMacSa);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, trillMBit);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, trillEgressRbid);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcHwDevice);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcIsTrunk);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChValue, ttiTrillAdjacencyPtr, srcPortTrunk);


    /* covert the HW devNum to SW device */
    rc = prvUtfSwFromHwDeviceNumberGet(ttiTrillAdjacencyPtr->srcHwDevice,
                                       &ttiTrillAdjacencyPtr->srcHwDevice);

    if (rc != GT_OK)
        return rc;

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(ttiTrillAdjacencyPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortTrillOuterVid0Set function
* @endinternal
*
* @brief   This function sets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - TRILL Outer Tag0 VID (0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(vlanId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortTrillOuterVid0Get function
* @endinternal
*
* @brief   This function gets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanIdPtr                - (pointer to) TRILL Outer Tag0 VID (0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, vlanIdPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(vlanIdPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillCpuCodeBaseSet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - The base CPU code value for the TRILL engine (192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCodeBase);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillCpuCodeBaseGet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - Points to the base CPU code value for the TRILL engine (192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiTrillCpuCodeBaseGet(devNum, cpuCodeBasePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCodeBasePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS prvTgfTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
)
{
#ifdef CHX_FAMILY

CPSS_DXCH_TTI_EXCEPTION_ENT            dxChExceptionType;

/* convert TTI exception type into device specific format */
PRV_TGF_S2D_EXCEPTION_TYPE_CONVERT_MAC(dxChExceptionType, exceptionType);

/* call device specific API */
return cpssDxChTtiExceptionCmdSet(devNum, dxChExceptionType, command);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(command);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
*
* @param[out] commandPtr               - points to the command for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS prvTgfTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
#ifdef CHX_FAMILY

CPSS_DXCH_TTI_EXCEPTION_ENT            dxChExceptionType;

/* convert TTI exception type into device specific format */
PRV_TGF_S2D_EXCEPTION_TYPE_CONVERT_MAC(dxChExceptionType, exceptionType);

/* call device specific API */
return cpssDxChTtiExceptionCmdGet(devNum, dxChExceptionType, commandPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(commandPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillMaxVersionSet function
* @endinternal
*
* @brief   This function sets the max Trill version.
*
* @param[in] devNum                   - device number
* @param[in] maxTrillVersion          - max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
)
{
#ifdef CHX_FAMILY

/* call device specific API */
return cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxTrillVersion);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTrillMaxVersionGet function
* @endinternal
*
* @brief   This function gets the max Trill version.
*
* @param[in] devNum                   - device number
*
* @param[out] maxTrillVersionPtr       - Points to max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
)
{
#ifdef CHX_FAMILY

/* call device specific API */
return cpssDxChTtiTrillMaxVersionGet(devNum, maxTrillVersionPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxTrillVersionPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelTermInilifConfigurationSet function
* @endinternal
*
* @brief   Set Inlif Configuration
*
* @param[in] portIndex                - port index
*
* @param[out] defaultInlifModePtr      - pointer to the default inlif mode
*                                       None
*/
GT_VOID prvTgfTunnelTermInilifConfigurationSet
(
    IN  GT_U8                               portIndex,
    OUT PRV_TGF_INLIF_PORT_MODE_ENT         *defaultInlifModePtr
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(defaultInlifModePtr);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;

    PRV_UTF_LOG0_MAC("======= Setting Inlif Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

     /* save inlif port mode */
    rc = prvTgfInlifPortModeGet(portIndex, defaultInlifModePtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(portIndex,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[portIndex],
                             &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);


     /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(portIndex, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIndex]);

    /* Force-Vlan-Mode-on-TT */
    rc = prvTgfInlifIntForceVlanModeOnTtSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");

    /* Disable VLAN Egress Filtering on specified device for Bridged Known Unicast packets*/
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");

#endif /*EXMXPM_FAMILY*/
}



/**
* @internal prvTgfTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
* @param[in] pclId                    - PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS prvTgfTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT        keyType,
    IN  GT_U32                          pclId
)
{
#ifdef CHX_FAMILY
    PRV_UTF_LOG3_MAC("prvTgfTtiPclIdSet(%d,%d,%d) \n",
        devNum,keyType,pclId);

    /* call device specific API */
    return cpssDxChTtiPclIdSet(devNum, (CPSS_DXCH_TTI_KEY_TYPE_ENT) keyType,pclId);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(pclId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
*
* @param[out] pclIdPtr                 - (points to) PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS prvTgfTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT        keyType,
    OUT GT_U32                          *pclIdPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPclIdGet(devNum, (CPSS_DXCH_TTI_KEY_TYPE_ENT) keyType,pclIdPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(pclIdPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] size                     - key  in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    IN PRV_TGF_TTI_KEY_SIZE_ENT             size
)
{
#ifdef CHX_FAMILY

    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_KEY_SIZE_ENT      dxChKeySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* convert keySize into device specific format */
    PRV_TGF_S2D_KEY_SIZE_CONVERT_MAC(dxChKeySize, size);

    /* call device specific API */
    return cpssDxChTtiPacketTypeKeySizeSet(devNum, dxChKeyType, dxChKeySize);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(size);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
*
* @param[out] sizePtr                  - points to key size in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    OUT PRV_TGF_TTI_KEY_SIZE_ENT            *sizePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_KEY_SIZE_ENT      dxChKeySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;
    GT_STATUS   rc = GT_OK;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    rc =  cpssDxChTtiPacketTypeKeySizeGet(devNum, dxChKeyType, &dxChKeySize);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiPacketTypeKeySizeGet FAILED, rc = [%d]", rc);

        return rc;
    }

     /* convert keySize from device specific format */
    PRV_TGF_D2S_KEY_SIZE_CONVERT_MAC(dxChKeySize, *sizePtr);

    return GT_OK;

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(sizePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortPassengerOuterIsTag0Or1Set function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port , mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
)
{
#ifdef CHX_FAMILY
    PRV_UTF_LOG3_MAC("prvTgfTtiPortPassengerOuterIsTag0Or1Set(%d,%d,%d) \n",
        devNum,portNum,mode);

    /* call device specific API */
    return cpssDxChTtiPortPassengerOuterIsTag0Or1Set(devNum, portNum,mode);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mode);
    return GT_BAD_STATE;
#endif
}
/**
* @internal prvTgfTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/disable Ipv4/6 Total Length Deduction.
*                                      GT_TRUE: When enabled, and ePort default attribute
*                                      <IPv4/6 Total Length Deduction Enable> == Enabled:
*                                      - for IPv4:
*                                      <IPv4 Total Length> = Packet IPv4 header <Total Length> -
*                                      Global configuration < IPv4 Total Length Deduction Value>
*                                      - for IPv6:
*                                      <IPv6 Total Length> = Packet IPv6 header <Total Length> -
*                                      Global configuration < IPv6 Total Length Deduction Value>
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                -  Enable/disable Ipv4/6 Total Length Deduction.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
* @param[in] value                    - IPv4 or IPv6 Total Length Deduction Value (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipType);
    TGF_PARAM_NOT_USED(value);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
*
* @param[out] valuePtr                 -  (pointer to) IPv4 or IPv6 Total Length Deduction Value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, valuePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipType);
    TGF_PARAM_NOT_USED(valuePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.
* @param[in] offset                   - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    IN GT_U32                               udbIndex,
    IN PRV_TGF_TTI_OFFSET_TYPE_ENT          offsetType,
    IN GT_U8                                offset
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   dxChOffsetType = CPSS_DXCH_TTI_OFFSET_L2_E;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* convert offsetType into device specific format */
    PRV_TGF_S2D_OFFSET_TYPE_CONVERT_MAC(dxChOffsetType, offsetType);

    /* call device specific API */
    return cpssDxChTtiUserDefinedByteSet(devNum, dxChKeyType, udbIndex, dxChOffsetType, offset);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(udbIndex);
    TGF_PARAM_NOT_USED(offsetType);
    TGF_PARAM_NOT_USED(offset);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*
* @param[out] offsetTypePtr            - (pointer to) TTI offset type.
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  GT_U32                              udbIndex,
    OUT PRV_TGF_TTI_OFFSET_TYPE_ENT         *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   dxChOffsetType = CPSS_DXCH_TTI_OFFSET_L2_E;

    /* convert keyType into device specific format */
    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    rc = cpssDxChTtiUserDefinedByteGet(devNum, dxChKeyType, udbIndex, &dxChOffsetType, offsetPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTtiUserDefinedByteGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert offsetType from device specific format */
    PRV_TGF_D2S_OFFSET_TYPE_CONVERT_MAC(dxChOffsetType, *offsetTypePtr);


    return rc;

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(udbIndex);
    TGF_PARAM_NOT_USED(offsetTypePtr);
    TGF_PARAM_NOT_USED(offsetPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelStartEgessVlanTableServiceIdSet function
* @endinternal
*
* @brief   Set egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
* @param[in] vlanServiceId            - tunnel start entry extension value (APPLICABLE RANGES: 0...FFFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;


    /* call device specific API */
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(devNum, vlanId, vlanServiceId);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTunnelStartEgessVlanTableServiceIdSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(vlanServiceId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelStartEgessVlanTableServiceIdGet function
* @endinternal
*
* @brief   Get egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
*
* @param[out] vlanServiceIdPtr         - (pointer to) vlan service Id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdGet(devNum, vlanId, vlanServiceIdPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartEgessVlanTableServiceIdGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(vlanServiceIdPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiDefaultAction2Get function
* @endinternal
*
* @brief   This function gets default action values.
*
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiDefaultAction2Get
(
    OUT  PRV_TGF_TTI_ACTION_2_STC         *actionPtr
)
{

    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    cpssOsMemSet(actionPtr, 0, sizeof(PRV_TGF_TTI_ACTION_2_STC));

    actionPtr->tunnelTerminate              = GT_TRUE; /* this is not the default value !!! */

    /* default values */
    actionPtr->command                      = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->redirectCommand              = PRV_TGF_TTI_NO_REDIRECT_E;
    actionPtr->pcl0OverrideConfigIndex      = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    actionPtr->pcl0_1OverrideConfigIndex    = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    actionPtr->pcl1OverrideConfigIndex      = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    actionPtr->tag0VlanPrecedence           = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    actionPtr->tag0VlanCmd                  = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    actionPtr->tag1VlanCmd                  = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    actionPtr->qosPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    actionPtr->modifyTag0Up                 = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    actionPtr->tag1UpCommand                = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
    actionPtr->modifyDscp                   = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    actionPtr->ttPassengerPacketType        = PRV_TGF_TTI_PASSENGER_IPV4_E;

    return GT_OK;
}

/**
* @internal prvTgfTtiDefaultIpv4PatternMaskGet function
* @endinternal
*
* @brief   This function gets default rule's pattern and mask values for IPv4 key.
*
* @param[in] defaultInfo              - default information to be set
*
* @param[out] patternPtr               - (pointer to) the TTI rule's pattern for IPv4 key
* @param[out] maskPtr                  - (pointer to) the TTI rule's mask for IPv4 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note default values:
*       pclId = 1;
*       srcIsTrunk = GT_FALSE;
*       isTagged = GT_TRUE;
*       tunneltype = IPv4-Over-IPv4;
*       All the other fields that are not input parameter are masked.
*       Input parameters are exect match.
*
*/
GT_STATUS prvTgfTtiDefaultIpv4PatternMaskGet
(
    IN   PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC defaultInfo,
    OUT  PRV_TGF_TTI_IPV4_RULE_STC         *patternPtr,
    OUT  PRV_TGF_TTI_IPV4_RULE_STC         *maskPtr
)
{

    /* clear pattern and mask */
    cpssOsMemSet(patternPtr, 0, sizeof(PRV_TGF_TTI_IPV4_RULE_STC));
    cpssOsMemSet(maskPtr,    0, sizeof(PRV_TGF_TTI_IPV4_RULE_STC));

    patternPtr->common.pclId = PRV_TGF_TTI_IPV4_DEFAULT_PCL_ID_CNS;
    patternPtr->common.srcIsTrunk = GT_FALSE;
    patternPtr->common.srcPortTrunk = defaultInfo.common.srcPort;
    patternPtr->common.vid = defaultInfo.common.vid;
    patternPtr->common.isTagged = GT_TRUE;
    cpssOsMemCpy((GT_VOID*)patternPtr->common.mac.arEther,
                 (GT_VOID*)defaultInfo.common.mac.arEther,
                 sizeof(GT_ETHERADDR));
    patternPtr->tunneltype = 0; /* IPv4_OVER_IPv4 */
    cpssOsMemCpy(patternPtr->destIp.arIP, defaultInfo.destIp.arIP,
                 sizeof(GT_U32));
    cpssOsMemCpy(patternPtr->srcIp.arIP, defaultInfo.srcIp.arIP,
                 sizeof(GT_U32));
    patternPtr->isArp = GT_FALSE;

    maskPtr->common.pclId = BIT_10 - 1;
    maskPtr->common.srcIsTrunk = GT_TRUE;
    maskPtr->common.srcPortTrunk = BIT_7 - 1;
    maskPtr->common.vid = BIT_12 - 1;
    maskPtr->common.isTagged = GT_TRUE;
    cpssOsMemSet((GT_VOID*)maskPtr->common.mac.arEther, 0xFF,
                 sizeof(GT_ETHERADDR));
    maskPtr->tunneltype = BIT_3- 1;
    cpssOsMemSet(maskPtr->srcIp.arIP,  0xFF, sizeof(GT_U32));
    cpssOsMemSet(maskPtr->destIp.arIP, 0xFF, sizeof(GT_U32));
    maskPtr->isArp = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvTgfTtiGreExtensionsSet function
* @endinternal
*
* @brief   Enable/Disable GRE extensions
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the GRE extensions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiGreExtensionsSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
/* call device specific API */
return cpssDxChTtiGreExtensionsEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiIpv4McEnableSet function
* @endinternal
*
* @brief   Enable/disable the TTI lookup for IPv4 multicast
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:   the TTI lookup for IPv4 multicast
*                                      - GT_FALSE: disable the TTI lookup for IPv4 multicast
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4McEnableSet
(
    IN GT_U8       devNum,
    IN GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
/* call device specific API */
return cpssDxChTtiIpv4McEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelStartMplsPwControlWordSet function
* @endinternal
*
* @brief   Sets a Pseudo Wire control word
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
* @param[in] value                    - the PW word to write
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          value
)
{
#ifdef CHX_FAMILY
/* call device specific API */
return cpssDxChTunnelStartMplsPwControlWordSet(devNum, index, value);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(value);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTunnelStartMplsPwControlWordGet function
* @endinternal
*
* @brief   Gets a Pseudo Wire control word
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
*
* @param[out] valuePtr                 - (pointer to) the read PW word
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *valuePtr
)
{
#ifdef CHX_FAMILY
/* call device specific API */
return cpssDxChTunnelStartMplsPwControlWordGet(devNum, index, valuePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(valuePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the IPv4 TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortMplsOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the MPLS TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the MIM TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:   MIM TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwExceptionCmdSet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_TTI_PW_CW_EXCEPTION_ENT  exceptionType,
    IN CPSS_PACKET_CMD_ENT              command
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   dxExceptionType;

    /* call device specific API */
    PRV_TGF_PW_CW_ERROR_CONVERT_MAC(dxExceptionType, exceptionType);
    return cpssDxChTtiPwCwExceptionCmdSet(devNum, dxExceptionType, command);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(command);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwExceptionCmdGet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_PW_CW_EXCEPTION_ENT exceptionType,
    OUT CPSS_PACKET_CMD_ENT             *commandPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   dxExceptionType;

    /* call device specific API */
    PRV_TGF_PW_CW_ERROR_CONVERT_MAC(dxExceptionType, exceptionType);
    return cpssDxChTtiPwCwExceptionCmdGet(devNum, dxExceptionType, commandPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(commandPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwCpuCodeBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   cpuCodeBase
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPwCwCpuCodeBaseSet(devNum, cpuCodeBase);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCodeBase);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - (pointer to) the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwCpuCodeBaseGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *cpuCodeBasePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiPwCwCpuCodeBaseGet(devNum, cpuCodeBasePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCodeBasePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] extensionHeaderValue     - the configurable extension header value
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiIPv6ExtensionHeaderSet
(
    IN GT_U8    devNum,
    IN GT_U32   extensionHeaderId,
    IN GT_U32   extensionHeaderValue
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiIPv6ExtensionHeaderSet(devNum, extensionHeaderId,
                                                     extensionHeaderValue);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(extensionHeaderId);
    TGF_PARAM_NOT_USED(extensionHeaderValue);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
*                                      extensionHeaderValue - the configurable extension header value
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiIPv6ExtensionHeaderGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   extensionHeaderId,
    OUT GT_U32  *extensionHeaderValuePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiIPv6ExtensionHeaderGet(devNum, extensionHeaderId,
                                                     extensionHeaderValuePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(extensionHeaderId);
    TGF_PARAM_NOT_USED(extensionHeaderValuePtr);
    return GT_BAD_STATE;
#endif
}

#define SUPPORT_TCAM_SEGMENT_MODE_MAC(dev) \
    ((PRV_CPSS_SIP_5_CHECK_MAC(dev) && !PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) ? 1 : 0)

/**
* @internal prvTgfTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
* @param[in] segmentMode              - TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiTcamSegmentModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT                 keyType,
    IN PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT    segmentMode
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT dxChKeyType;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT dxChSegmentMode;

    if(!SUPPORT_TCAM_SEGMENT_MODE_MAC(devNum))
    {
        /* the device not support it ... let the test continue without fail */
        return GT_OK;
    }

    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);
    PRV_TGF_S2D_SEGMENT_MODE_CONVERT_MAC(dxChSegmentMode, segmentMode);
    /* call device specific API */
    return cpssDxChTtiTcamSegmentModeSet(devNum, dxChKeyType, dxChSegmentMode);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(segmentMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
*                                      OUTPUTS:
* @param[in] segmentModePtr           - (pointer to) TTI TCAM segment mode
*
* @param[out] segmentModePtr           - (pointer to) TTI TCAM segment mode
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiTcamSegmentModeGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT                keyType,
    OUT PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT   *segmentModePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_KEY_TYPE_ENT  dxChKeyType;
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT dxChSegmentMode;
    GT_STATUS rc;

    if(!SUPPORT_TCAM_SEGMENT_MODE_MAC(devNum))
    {
        /* the device not support it ... let the test continue without fail */
        *segmentModePtr = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E;
        return GT_OK;
    }

    PRV_TGF_S2D_KEY_TYPE_CONVERT_MAC(dxChKeyType, keyType);

    /* call device specific API */
    rc = cpssDxChTtiTcamSegmentModeGet(devNum, dxChKeyType, &dxChSegmentMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiTcamSegmentModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    PRV_TGF_D2S_SEGMENT_MODE_CONVERT_MAC(dxChSegmentMode, *segmentModePtr);
    return rc;

#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(keyType);
    TGF_PARAM_NOT_USED(segmentModePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
* @param[in] code                     - the  for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            code
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_EXCEPTION_ENT            dxChExceptionType;

    /* convert TTI exception type into device specific format */
    PRV_TGF_S2D_EXCEPTION_TYPE_CONVERT_MAC(dxChExceptionType, exceptionType);
    /* call device specific API */
    return cpssDxChTtiExceptionCpuCodeSet(devNum, dxChExceptionType, code);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(code);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*
* @param[out] codePtr                  - (points to) the code for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            *codePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TTI_EXCEPTION_ENT            dxChExceptionType;

    /* convert TTI exception type into device specific format */
    PRV_TGF_S2D_EXCEPTION_TYPE_CONVERT_MAC(dxChExceptionType, exceptionType);
    /* call device specific API */
    return cpssDxChTtiExceptionCpuCodeGet(devNum, dxChExceptionType, codePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(codePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
* @param[in] overrideBitmap           - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*                                      (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiSourceIdBitsOverrideSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookup,
    IN GT_U32   overrideBitmap
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiSourceIdBitsOverrideSet(devNum, lookup, overrideBitmap);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(lookup);
    TGF_PARAM_NOT_USED(overrideBitmap);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
*
* @param[out] overrideBitmapPtr        - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiSourceIdBitsOverrideGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   lookup,
    OUT GT_U32  *overrideBitmapPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTtiSourceIdBitsOverrideGet(devNum, lookup, overrideBitmapPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(lookup);
    TGF_PARAM_NOT_USED(overrideBitmapPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeForwardingEnableSet
(
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc;

    rc = cpssDxChTtiFcoeForwardingEnableSet(prvTgfDevNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeForwardingEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeForwardingEnableGet
(
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;
    rc = cpssDxChTtiFcoeForwardingEnableGet(prvTgfDevNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeForwardingEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] etherType                - EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeEtherTypeSet
(
    IN GT_U16       etherType
)
{
    GT_STATUS   rc;
    rc = cpssDxChTtiFcoeEtherTypeSet(prvTgfDevNum, etherType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeEtherTypeSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] etherTypePtr             - (pointer to) EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeEtherTypeGet
(
    OUT GT_U16      *etherTypePtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChTtiFcoeEtherTypeGet(prvTgfDevNum, etherTypePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeEtherTypeGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeAssignVfIdEnableSet
(
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc;

    rc = cpssDxChTtiFcoeAssignVfIdEnableSet(prvTgfDevNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeAssignVfIdEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if FCoE packet that contains a VF Tag is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeAssignVfIdEnableGet
(
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChTtiFcoeAssignVfIdEnableGet(prvTgfDevNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiFcoeAssignVfIdEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTunnelStartEcnModeSet function
* @endinternal
*
* @brief   Set Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] mode                  - ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or ECN mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TUNNEL_START_ECN_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT dxChEcnMode;

    PRV_TGF_TUNNEL_START_ECN_MODE_CONVERT_MAC(dxChEcnMode, mode);

    rc = cpssDxChTunnelStartEcnModeSet(devNum, dxChEcnMode);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTunnelStartEcnModeSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTunnelEcnPacketCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or packet command.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelEcnPacketCommandSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT   command
)
{
    GT_STATUS rc;

    rc = cpssDxChTtiEcnPacketCommandSet(devNum, command);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiEcnPacketCommandSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfTunnelEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is non-zero.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] cpuCode               - CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelEcnCpuCodeSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode
)
{
    GT_STATUS rc;

    rc = cpssDxChTtiEcnCpuCodeSet(devNum, cpuCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTtiEcnPacketCommandSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

