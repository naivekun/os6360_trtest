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
* @file cpssDxChTunnel.c
*
* @brief CPSS tunnel implementation.
*
* @version   85
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/tunnel/private/prvCpssDxChTunnelLog.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

/* the size of tunnel start entry in words */
#define TUNNEL_START_HW_SIZE_CNS       12

/* maximum value for time to live (TTL) field */
#define TTL_MAX_CNS                     255
#define MAX_HASH_SHIFT_LEFT_BITS_CNS    8

/* maximum value for MPLS label field */
#define PRV_TUNNEL_START_MPLS_LABEL_CHECK_MAC(label)    \
    if((label) >= BIT_20)                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* maximum value for MPLS label stop bit field */
#define PRV_TUNNEL_START_MPLS_SBIT_CHECK_MAC(sBit)  \
    if((sBit) > 1)                      \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* maximum value for MPLS label exp field */
#define PRV_TUNNEL_START_MPLS_EXP_CHECK_MAC(exp)    \
    if((exp) >= BIT_3)                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


/* maximum value for Egress Vlan Service Id field */
#define PRV_TUNNEL_START_VLAN_SERVICE_ID_CHECK_MAC(vlanServiceId)    \
    if((vlanServiceId) >= BIT_24)                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* the size of tunnel start profile entry in words */
#define TUNNEL_START_PROFILE_HW_SIZE_CNS       17

/* maximum value for trunk interface field */
#define TRUNK_INTERFACE_MAX_CNS     127

/* maximum value for policer index field */
#define POLICER_INDEX_MAX_CNS       255

/* maximum value for tunnel termination match counter value */
#define MATCH_COUNTER_INDEX_MAX_CNS     31

/* maximum value for control word index xCat3  */
#define CONTROL_WORD_INDEX_XCAT_MAX_CNS 7

/* maximum value for control word index Bobcat2; Caelum; Bobcat3; Aldrin2  */
#define CONTROL_WORD_INDEX_MAX_CNS      15

/* maximum value for control word index in Generic IP Profile Table Entry  */
#define GENERIC_IP_PROFILE_CONTROL_WORD_INDEX_MAX_CNS      7

/* maximum value for service-ID circular shift size in Generic IP Profile Table Entry */
#define GENERIC_IP_PROFILE_SERVICE_ID_CIRCULAR_SHIFT_SIZE_MAX_CNS 23

/* maximum value for the Tunnel-start MAC DA mode in Generic IP Profile Table Entry */
#define GENERIC_IP_PROFILE_MAC_DA_MODE_MAX_CNS 24

/* maximum value for the Tunnel-start desination IP mode in Generic IP Profile Table Entry */
#define GENERIC_IP_PROFILE_DIP_MODE_MAX_CNS 24

/* macro to check line index */
#define LINE_INDEX_CHECK_MAC(devNum,index)                             \
    if(index >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(mode)                  \
    switch ((mode))                                                     \
    {                                                                   \
    case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E:                  \
    case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E:     \
        break;                                                          \
    default:                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
    }

#define PRV_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_CHECK_MAC(mode)                  \
    switch ((mode))                                                     \
    {                                                                   \
    case CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_EPORT_E:                  \
    case CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_PACKET_QOS_PROFILE_E:     \
        break;                                                          \
    default:                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
    }

#define PRV_TUNNEL_START_TTL_CHECK_MAC(ttl)     \
    if((ttl) > TTL_MAX_CNS)                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_AUTO_TUNNEL_OFFSET_CHECK_MAC(offset)   \
    if((offset) > 12)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_CFI_CHECK_MAC(cfi)   \
    if((cfi) > 1)                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


#define PRV_TUNNEL_START_PROFILE_INDEX_CHECK_MAC(profileIndex)   \
    if((profileIndex) >= BIT_3)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

#define PRV_TUNNEL_START_GRE_PROTOCOL_FOR_ETHERNET_CHECK_MAC(greProtocolForEthernet)   \
    if((greProtocolForEthernet) >= BIT_16)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

#define PRV_TUNNEL_START_GRE_FLAGS_AND_VERSION_CHECK_MAC(greFlagsAndVersion)   \
    if((greFlagsAndVersion) >= BIT_16)                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

#define PRV_TUNNEL_START_CHECK_TTL_MODE_MAC(devNum, mode)                    \
        switch ((mode))                                                      \
        {                                                                    \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E:                   \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E:                     \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E:           \
            break;                                                           \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E:               \
            if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))                           \
            {                                                                \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
            }                                                                \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);   \
        }

#define PRV_TUNNEL_START_ISID_CHECK_MAC(iSid)       \
    if((iSid) >= BIT_24)                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_UDP_DST_PORT_CHECK_MAC(udpDstPort) \
    if ((udpDstPort) >= BIT_16)                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


#define PRV_TUNNEL_START_UDP_SRC_PORT_CHECK_MAC(udpSrcPort) \
    if ((udpSrcPort) >= BIT_16)                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

#define PRV_TUNNEL_START_IP_PROTOCOL_CHECK_MAC(protocolNum) \
    if ((protocolNum) >= BIT_8)                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


#define PRV_TUNNEL_START_CONVERT_MIM_I_SID_MODE_TO_HW_FORMAT_MAC(mode,value) \
    switch ((mode))                                                         \
    {                                                                       \
    case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E:           \
        (value) = 0;                                                        \
        break;                                                              \
    case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E:         \
        (value) = 1;                                                        \
        break;                                                              \
    default:  /* default set some value as the field must not be valid */   \
        (value) = 0;                                                        \
        break;                                                              \
    }

#define PRV_TUNNEL_START_MIM_I_SID_MODE_CHECK_MAC(mode)                 \
    switch ((mode))                                                     \
    {                                                                   \
    case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E:       \
    case CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E:     \
        break;                                                          \
    default:                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
    }

#define PRV_TUNNEL_START_CONVERT_MIM_B_DA_MODE_TO_HW_FORMAT_MAC(mode,value)             \
    switch ((mode))                                                                     \
    {                                                                                   \
    case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E:                        \
        (value) = 0;                                                                    \
        break;                                                                          \
    case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E:         \
        (value) = 1;                                                                    \
        break;                                                                          \
    default:  /* default set some value as the field must not be valid */               \
        (value) = 0;                                                                    \
        break;                                                                          \
    }

#define PRV_TUNNEL_START_CONVERT_MIM_B_SA_MODE_TO_HW_FORMAT_MAC(mode,value)             \
    switch ((mode))                                                                     \
    {                                                                                   \
    case CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E:                    \
        (value) = 0;                                                                    \
        break;                                                                          \
    case CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E:                    \
        (value) = 1;                                                                    \
        break;                                                                          \
    default:  /* default set some value as the field must not be valid */                       \
        (value) = 0;                                                                    \
        break;                                                                          \
    }



#define PRV_TUNNEL_START_MIM_B_DA_MODE_CHECK_MAC(mode)                              \
    switch ((mode))                                                                 \
    {                                                                               \
    case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E:                    \
    case CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E:     \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }

#define PRV_TUNNEL_START_MIM_B_SA_MODE_CHECK_MAC(mode)                              \
    switch ((mode))                                                                 \
    {                                                                               \
    case CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E:                 \
    case CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E:                 \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }


#define PRV_TUNNEL_START_ITAG_RES_CHECK_MAC(iTagRes)    \
    if((iTagRes) > 15)                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(mode,value) \
    switch ((mode))                                                         \
    {                                                                       \
    case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E:                      \
        (value) = 1;                                                        \
        break;                                                              \
    case CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E:         \
        (value) = 0;                                                        \
        break;                                                              \
    default:  /* default set some value as the field must not be valid */   \
        (value) = 0;                                                        \
        break;                                                              \
    }

#define PRV_TUNNEL_START_CONVERT_MPLS_PW_EXP_MARK_MODE_TO_HW_FORMAT_MAC(mode,value) \
    switch ((mode))                                                         \
    {                                                                       \
    case CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_EPORT_E:                      \
        (value) = 0;                                                        \
        break;                                                              \
    case CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_PACKET_QOS_PROFILE_E:         \
        (value) = 1;                                                        \
        break;                                                              \
    default:  /* default set some value as the field must not be valid */   \
        (value) = 0;                                                        \
        break;                                                              \
    }

#define PRV_TUNNEL_START_IP_HEADER_PROTOCOL_CHECK_MAC(ipHeaderProtocol) \
    switch ((ipHeaderProtocol))                                         \
    {                                                                   \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:                \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:               \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:               \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:           \
        break;                                                          \
    default:                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
    }

#define PRV_TUNNEL_START_CONVERT_IP_HEADER_PROTOCOL_TO_HW_FORMAT_MAC(ipHeaderProtocol,value) \
    switch ((ipHeaderProtocol))                                                              \
    {                                                                                        \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E:                                     \
        (value) = 0;                                                                         \
        break;                                                                               \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E:                                    \
        (value) = 1;                                                                         \
        break;                                                                               \
    case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E:                                    \
        (value) = 2;                                                                         \
        break;                                                                               \
     case CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E:                                    \
        (value) = 3;                                                                         \
         break;                                                                               \
    default:  /* default set some value as the field must not be valid */                    \
        (value) = 0;                                                                         \
        break;                                                                               \
    }

#define PRV_TUNNEL_START_CONVERT_TTL_MODE_TO_HW_FORMAT_MAC(mode,value)       \
        switch ((mode))                                                      \
        {                                                                    \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E:                   \
            (value) = 0;                                                     \
            break;                                                           \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E:                     \
            (value) = 1;                                                     \
            break;                                                           \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E:           \
            (value) = 2;                                                     \
            break;                                                           \
        case CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E:               \
            (value) = 3;                                                     \
            break;                                                           \
        default:  /* default set some value as the field must not be valid */\
            (value) = 0;                                                     \
            break;                                                           \
        }

#define PRV_TUNNEL_START_MPLS_ETHER_TYPE_CHECK_MAC(mplsEtherType)            \
        switch ((mplsEtherType))                                             \
        {                                                                    \
        case CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:                    \
        case CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:                    \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
        }

#define PRV_TUNNEL_START_CONVERT_MPLS_ETHER_TYPE_TO_HW_FORMAT_MAC(etherType,value)\
        switch ((etherType))                                                 \
        {                                                                    \
        case CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E:                    \
            (value) = 0;                                                     \
            break;                                                           \
        case CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E:                    \
            (value) = 1;                                                     \
            break;                                                           \
        default:  /* default set some value as the field must not be valid */\
            (value) = 0;                                                     \
            break;                                                           \
        }

#define PRV_TUNNEL_START_FLOW_LABEL_MODE_CHECK_AND_CONVERT_TO_HW_FORMAT_MAC(flowLabelMode,value)\
        switch ((flowLabelMode))                                                              \
        {                                                                                     \
        case CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E:                              \
            (value) = 0;                                                                      \
            break;                                                                            \
        case CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E:                 \
            (value) = 1;                                                                      \
            break;                                                                            \
        default:                                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                              \
            break;                                                                            \
        }


#define PRV_TUNNEL_START_GEN_PROFILE_CHECK_TEMPLATE_DATA_VALUE_AND_CONVERT_TO_HW_FORMAT_MAC(templateValue,hwValue) \
    switch ((templateValue))                                                              \
    {                                                                                     \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E:                              \
        (hwValue) = 0;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E:                              \
        (hwValue) = 1;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E:                       \
        (hwValue) = 2;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E:                       \
        (hwValue) = 3;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E:                       \
        (hwValue) = 4;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E:                       \
        (hwValue) = 5;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E:                        \
        (hwValue) = 6;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E:                        \
        (hwValue) = 7;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E:                        \
        (hwValue) = 8;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_0_E:                         \
        (hwValue) = 9;                                                                    \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_1_E:                         \
        (hwValue) = 10;                                                                   \
        break;                                                                            \
    default:                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                              \
        break;                                                                            \
    }

#define PRV_TUNNEL_START_GEN_PROFILE_TEMPLATE_DATA_VALUE_HW_TO_SW_MAC(templateData,hwValue) \
    switch ((hwValue))                                                                    \
    {                                                                                     \
    case 0:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E;              \
        break;                                                                            \
    case 1:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;              \
        break;                                                                            \
    case 2:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E;       \
        break;                                                                            \
    case 3:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E;       \
        break;                                                                            \
    case 4:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E;       \
        break;                                                                            \
    case 5:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E;       \
        break;                                                                            \
    case 6:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E;        \
        break;                                                                            \
    case 7:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E;        \
        break;                                                                            \
    case 8:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E;        \
        break;                                                                            \
    case 9:                                                                               \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_0_E;         \
        break;                                                                            \
    case 10:                                                                              \
        (templateData) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_1_E;         \
        break;                                                                            \
    default:                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                                              \
        break;                                                                            \
    }


#define PRV_TUNNEL_START_GEN_PROFILE_CHECK_TEMPLATE_DATA_SIZE_AND_CONVERT_TO_HW_FORMAT_MAC(templateDataSize,value) \
    switch ((templateDataSize))                                                           \
    {                                                                                     \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E:                                \
        (value) = 0;                                                                      \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E:                                  \
        (value) = 1;                                                                      \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E:                                  \
        (value) = 2;                                                                      \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E:                                 \
        (value) = 3;                                                                      \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E:                                 \
        (value) = 4;                                                                      \
        break;                                                                            \
    default:                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                              \
        break;                                                                            \
    }

#define PRV_TUNNEL_START_GEN_PROFILE_TEMPLATE_DATA_SIZE_HW_TO_SW_MAC(templateDataSize,value) \
    switch ((value))                                                                      \
    {                                                                                     \
    case 0:                                                                               \
        (templateDataSize) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;            \
        break;                                                                            \
    case 1:                                                                               \
        (templateDataSize) = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E;              \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E:                                  \
        (templateDataSize) = 2;                                                           \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E:                                 \
        (templateDataSize) = 3;                                                           \
        break;                                                                            \
    case CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E:                                 \
        (templateDataSize) = 4;                                                           \
        break;                                                                            \
    default:                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                                              \
        break;                                                                            \
    }


#define PRV_TUNNEL_START_GEN_PROFILE_HASH_SHIFT_BITS_CHECK_MAC(hashShiftLeftBitsNum)     \
    if((hashShiftLeftBitsNum) >= MAX_HASH_SHIFT_LEFT_BITS_CNS)                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#define PRV_TUNNEL_START_GEN_PROFILE_CHECK_UDP_SRC_MODE_AND_CONVERT_TO_HW_FORMAT_MAC(udpSrcPortMode, value)\
        switch ((udpSrcPortMode))                                                             \
        {                                                                                     \
        case CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E:               \
            (value) = 0;                                                                      \
            break;                                                                            \
        case CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E:             \
            (value) = 1;                                                                      \
            break;                                                                            \
        default:                                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                              \
            break;                                                                            \
        }

#define PRV_TUNNEL_START_GEN_PROFILE_UDP_SRC_MODE_HW_TO_SW_MAC(udpSrcPortMode, value)               \
        switch ((value))                                                                            \
        {                                                                                           \
        case 0:                                                                                     \
            (udpSrcPortMode) = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;   \
            break;                                                                                  \
        case 1:                                                                                     \
            (udpSrcPortMode) = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E; \
            break;                                                                                  \
        default:                                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                                                    \
            break;                                                                                  \
        }


/* macro assume : value hold the 'value' and set it into hwFormatArray */
#define HA_TUNNEL_START_FIELD_VALUE_SET_MAC(_dev,_fieldName) \
    SIP5_HA_TUNNEL_START_FIELD_SET_MAC(_dev,hwFormatArray,_fieldName,value)

/* macro assume : hwFormatArray hold the entry and macro fill the value with 'value'*/
#define HA_TUNNEL_START_FIELD_VALUE_GET_MAC(_dev,_fieldName) \
    SIP5_HA_TUNNEL_START_FIELD_GET_MAC(_dev,hwFormatArray,_fieldName,value)

/* macro assume : value hold the 'value' and set it into hwFormatArray */
#define HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(_dev,_fieldName) \
    SIP6_HA_TUNNEL_START_GENERIC_FIELD_SET_MAC(_dev,hwFormatArray,_fieldName,value)

/* macro assume : hwFormatArray hold the entry and macro fill the value with 'value'*/
#define HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(_dev,_fieldName) \
    SIP6_HA_TUNNEL_START_GENERIC_FIELD_GET_MAC(_dev,hwFormatArray,_fieldName,value)

/* macro assume : value hold the 'value' and set it into hwFormatArray */
#define HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(_dev,_fieldName) \
    SIP5_HA_GEN_TUNNEL_START_PROFILE_FIELD_SET_MAC(_dev,hwFormatArray,_fieldName,value)

/* macro assume : hwFormatArray hold the entry and macro fill the value with 'value'*/
#define HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(_dev,_fieldName) \
    SIP5_HA_GEN_TUNNEL_START_PROFILE_FIELD_GET_MAC(_dev,hwFormatArray,_fieldName,value)

/* assign variable to field by the same name that is common to several 'Tunnel Start' formats */
#define GET_FIELD_FROM_USER_MAC(_keyType,_fieldName)    \
    _fieldName = logicFormatPtr->_keyType._fieldName

/* assign 'SW format field' to field by the same name that is common to several 'Tunnel Start' formats */
#define SET_FIELD_TO_USER_MAC(_keyType,_fieldName)    \
    logicFormatPtr->_keyType._fieldName = _fieldName

/* hw to sw for CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT */
#define HW_TO_SW_QOS_MARK_MODE_MAC(swValue,hwValue) \
    if (hwValue)                       \
        swValue = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E; \
    else                                    \
        swValue = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E

/* hw to sw for CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT */
#define HW_TO_SW_MPLS_PW_EXP_MARK_MODE_MAC(swValue,hwValue) \
    if (hwValue)                       \
        swValue = CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_PACKET_QOS_PROFILE_E; \
    else                                    \
        swValue = CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_EPORT_E


/* hw to sw for CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT */
#define HW_TO_SW_TTL_MODE_MAC(swValue,hwValue)                                  \
    switch (hwValue)                                                   \
    {                                                                  \
        case 0:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;    \
            break;                                                     \
        case 1:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E;      \
            break;                                                     \
        case 2:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E; \
            break;                                                     \
        case 3:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E; \
            break;                                                     \
        default:                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                       \
    }

/* hw to sw for CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT */
#define HW_TO_SW_IP_HEADER_PROTOCOL_MAC(swValue,hwValue)               \
    switch (hwValue)                                                   \
    {                                                                  \
        case 0:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;  \
            break;                                                     \
        case 1:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E; \
            break;                                                     \
        case 2:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E; \
            break;                                                     \
        case 3:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E; \
            break;                                                     \
        default:                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                       \
    }

#define HW_TO_SW_FLOW_LABEL_MODE_MAC(swValue,hwValue)                                   \
    switch (hwValue)                                                                    \
    {                                                                                   \
        case 0:                                                                         \
            swValue = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E;               \
            break;                                                                      \
        case 1:                                                                         \
            swValue = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E;  \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                                        \
    }

/* hw to sw for CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT */
#define HW_TO_SW_MPLS_ETHER_TYPE_MAC(swValue,hwValue)                  \
    switch (hwValue)                                                   \
    {                                                                  \
        case 0:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;     \
            break;                                                     \
        case 1:                                                        \
            swValue = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;     \
            break;                                                     \
        default:                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                       \
    }

/* hw to sw for CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT */
#define HW_TO_SW_I_SID_ASSIGN_MODE_MAC(swValue,hwValue)                      \
    if (hwValue)                                                             \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E; \
    else                                                                     \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E

/* hw to sw for CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT */
#define HW_TO_SW_B_DA_ASSIGN_MODE_MAC(swValue,hwValue)                       \
    if (hwValue)                                                             \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E; \
    else                                                                     \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E


 /* hw to sw for CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT */
#define HW_TO_SW_B_SA_ASSIGN_MODE_MAC(swValue,hwValue)                       \
    if (hwValue)                                                             \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E; \
    else                                                                     \
        swValue = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E



/**
* @enum TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT
 *
 * @brief Enumeration of HW value for tunnel start types.
*/
typedef enum{

    /** 0x0 = MPLS; MPLS; This Entry is an X */
    TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MPLS_E = 0x00

    /** 0x1 = IPv4; IPv4; This Entry is an X */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_IPV4_E = 0x00

    /** 0x2 = MacInMac; MacInMac; MacInMac.; */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MIM_E = 0x02

    /** 0x3 = TRILL; TRILL; TRILL; */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_TRILL_E = 0x03

    /** 0x4 = GENERIC; GENERIC; GENERIC. Applicable for sip6 only */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_E = 0x04

    /** 0x5 = IPv6; IPv6; This Entry is an X-over-IPv6 or X-Over-IPv6/GRE Tunnel Start.; */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_IPV6_E = 0x05

    /** 0x6 = Generic IPv4; GenericIPv4; Flexible X-over-IPv4/GRE/UDP; */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV4_E = 0x06

    /** 0x7 = GenericIPv6; GenericIPv6; Flexible X-over-IPv6/GRE/UDP; */
    ,TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV6_E = 0x07

} TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT;

/* generic tunnel start entry fields array initilized in accordance with fields sequence */
SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_ENT genTunnelStartProfilesFields[] =
{
    SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_UDP_SRC_PORT_MODE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_SERVICE_ID_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_MAC_DA_MODE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE_E
    ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS____LAST_VALUE___E/* used for array size */
};

/**
* @internal tunnelStartConfigLogic2HwFormat_sip5 function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from logic format
*         to hardware format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] tunnelType               - type of the tunnel
* @param[in] logicFormatPtr           - points to tunnel start configuration
*
* @param[out] hwFormatArray            - The configuration in HW format (6 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS tunnelStartConfigLogic2HwFormat_sip5
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr,
    OUT GT_U32                              *hwFormatArray
)
{
    GT_U32      value;
    GT_U32      hwTunnelType;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    sw_upMarkMode;/*SW up mark mode*/
    GT_U32      upMarkMode;         /* UP marking mode in hw format   */
    GT_U32      up;                 /* UP */
    GT_BOOL     tagEnable;
    GT_U16      vlanId;
    GT_ETHERADDR macDa;
    GT_U32      trillHeaderWords[2];

    switch (tunnelType)
    {
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        case CPSS_TUNNEL_X_OVER_MPLS_E:
        hwTunnelType = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MPLS_E;

        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.upMarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.upMarkMode,upMarkMode);
        GET_FIELD_FROM_USER_MAC(mplsCfg,up);
        GET_FIELD_FROM_USER_MAC(mplsCfg,tagEnable);
        GET_FIELD_FROM_USER_MAC(mplsCfg,vlanId);
        GET_FIELD_FROM_USER_MAC(mplsCfg,macDa);

        value = logicFormatPtr->mplsCfg.ttl;
        PRV_TUNNEL_START_TTL_CHECK_MAC(value);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);

        if(logicFormatPtr->mplsCfg.ttl == 0)
        {
            PRV_TUNNEL_START_CONVERT_TTL_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.ttlMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE_E);
        }

        value = BOOL2BIT_MAC(logicFormatPtr->mplsCfg.retainCRC);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);

        value = logicFormatPtr->mplsCfg.numLabels - 1;
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS_E);

        value = logicFormatPtr->mplsCfg.label1;
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL1_E);

        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            value = BOOL2BIT_MAC(logicFormatPtr->mplsCfg.pushEliAndElAfterLabel1);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1_E);
        }

        PRV_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.pwExpMarkMode);
        PRV_TUNNEL_START_CONVERT_MPLS_PW_EXP_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.pwExpMarkMode,value);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE_E);

        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp1MarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp1MarkMode,value);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE_E);

        if (logicFormatPtr->mplsCfg.exp1MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
        {
            value = logicFormatPtr->mplsCfg.exp1;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_E);
        }

        if(logicFormatPtr->mplsCfg.numLabels >= 2)
        {
            value = logicFormatPtr->mplsCfg.label2;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL2_E);

            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                value = BOOL2BIT_MAC(logicFormatPtr->mplsCfg.pushEliAndElAfterLabel2);
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2_E);
            }

            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp2MarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp2MarkMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE_E);

            if (logicFormatPtr->mplsCfg.exp2MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                value = logicFormatPtr->mplsCfg.exp2;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_E);
            }

            if(logicFormatPtr->mplsCfg.numLabels >= 3)
            {
                value = logicFormatPtr->mplsCfg.label3;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL3_E);

                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    value = BOOL2BIT_MAC(logicFormatPtr->mplsCfg.pushEliAndElAfterLabel3);
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3_E);
                }

                PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp3MarkMode);
                PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp3MarkMode,value);
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE_E);

                if (logicFormatPtr->mplsCfg.exp3MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                {
                    value = logicFormatPtr->mplsCfg.exp3;
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_E);
                }
            }
        }

        value = BOOL2BIT_MAC(logicFormatPtr->mplsCfg.setSBit);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT_E);

        if (BOOL2BIT_MAC(logicFormatPtr->mplsCfg.controlWordEnable))
        {
            value = logicFormatPtr->mplsCfg.controlWordIndex + 1;
        }
        else
        {
            value = 0;
        }
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX_E);

        PRV_TUNNEL_START_MPLS_ETHER_TYPE_CHECK_MAC(logicFormatPtr->mplsCfg.mplsEthertypeSelect);
        PRV_TUNNEL_START_CONVERT_MPLS_ETHER_TYPE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.mplsEthertypeSelect,value);
        HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL_E);

        break;
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            hwTunnelType = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV4_E;
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv4Cfg.upMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv4Cfg.upMarkMode,upMarkMode);
            GET_FIELD_FROM_USER_MAC(ipv4Cfg,up);
            GET_FIELD_FROM_USER_MAC(ipv4Cfg,tagEnable);
            GET_FIELD_FROM_USER_MAC(ipv4Cfg,vlanId);
            GET_FIELD_FROM_USER_MAC(ipv4Cfg,macDa);

            /* ttl value */
            value = logicFormatPtr->ipv4Cfg.ttl;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);

            /* retain CRC */
            value = BOOL2BIT_MAC(logicFormatPtr->ipv4Cfg.retainCRC);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);


            PRV_TUNNEL_START_IP_HEADER_PROTOCOL_CHECK_MAC(logicFormatPtr->ipv4Cfg.ipHeaderProtocol);
            if((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_FALSE) &&
                (logicFormatPtr->ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            PRV_TUNNEL_START_CONVERT_IP_HEADER_PROTOCOL_TO_HW_FORMAT_MAC(logicFormatPtr->ipv4Cfg.ipHeaderProtocol,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E);

            /* gre fields */
            if (value == 1)/* GRE protocol*/
            {
                /*  gre flags and version */
                PRV_TUNNEL_START_GRE_FLAGS_AND_VERSION_CHECK_MAC(logicFormatPtr->ipv4Cfg.greFlagsAndVersion);

                value = logicFormatPtr->ipv4Cfg.greFlagsAndVersion;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E);

                /*  GRE Protocol for Ethernet Passenger*/
                PRV_TUNNEL_START_GRE_PROTOCOL_FOR_ETHERNET_CHECK_MAC(logicFormatPtr->ipv4Cfg.greProtocolForEthernet);
                value = logicFormatPtr->ipv4Cfg.greProtocolForEthernet;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E);
            }
            else
            if (value == 2) /* UDP protocol*/
            {
                /* udp dst port */
                PRV_TUNNEL_START_UDP_DST_PORT_CHECK_MAC(logicFormatPtr->ipv4Cfg.udpDstPort);
                value = logicFormatPtr->ipv4Cfg.udpDstPort;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E);
                /* udp src port */
                PRV_TUNNEL_START_UDP_SRC_PORT_CHECK_MAC(logicFormatPtr->ipv4Cfg.udpSrcPort);
                value = logicFormatPtr->ipv4Cfg.udpSrcPort;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E);
            }
            else if (value == 3) /* generic  protocol*/
            {
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_TRUE)
                {
                    PRV_TUNNEL_START_IP_PROTOCOL_CHECK_MAC(logicFormatPtr->ipv4Cfg.ipProtocol);
                    value = logicFormatPtr->ipv4Cfg.ipProtocol;
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E);
                }
            }



            /* dscp mark mode */
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv4Cfg.dscpMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv4Cfg.dscpMarkMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E);
            /* dscp value */
            if (logicFormatPtr->ipv4Cfg.dscpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                value = logicFormatPtr->ipv4Cfg.dscp;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E);
            }

            /* DF flag*/
            value = BOOL2BIT_MAC(logicFormatPtr->ipv4Cfg.dontFragmentFlag);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG_E);

            /* Auto tunnel flag */
            value = BOOL2BIT_MAC(logicFormatPtr->ipv4Cfg.autoTunnel);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG_E);

            /* profile index */
            PRV_TUNNEL_START_PROFILE_INDEX_CHECK_MAC(logicFormatPtr->ipv4Cfg.profileIndex);
            value = logicFormatPtr->ipv4Cfg.profileIndex;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E);

            /* Auto tunnel offset */
            if(logicFormatPtr->ipv4Cfg.autoTunnel == GT_TRUE)
            {
                value = logicFormatPtr->ipv4Cfg.autoTunnelOffset;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET_E);
            }

            /* dip */
            value =
                (logicFormatPtr->ipv4Cfg.destIp.arIP[0] << 24) |
                (logicFormatPtr->ipv4Cfg.destIp.arIP[1] << 16) |
                (logicFormatPtr->ipv4Cfg.destIp.arIP[2] <<  8) |
                (logicFormatPtr->ipv4Cfg.destIp.arIP[3] <<  0);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DIP_E);

            /* sip */
            value =
                (logicFormatPtr->ipv4Cfg.srcIp.arIP[0] << 24) |
                (logicFormatPtr->ipv4Cfg.srcIp.arIP[1] << 16) |
                (logicFormatPtr->ipv4Cfg.srcIp.arIP[2] <<  8) |
                (logicFormatPtr->ipv4Cfg.srcIp.arIP[3] <<  0);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SIP_E);

            break;
        case CPSS_TUNNEL_GENERIC_IPV6_E:
            hwTunnelType = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV6_E;
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv6Cfg.upMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv6Cfg.upMarkMode,upMarkMode);
            GET_FIELD_FROM_USER_MAC(ipv6Cfg,up);
            GET_FIELD_FROM_USER_MAC(ipv6Cfg,tagEnable);
            GET_FIELD_FROM_USER_MAC(ipv6Cfg,vlanId);
            GET_FIELD_FROM_USER_MAC(ipv6Cfg,macDa);

            /* ttl value */
            value = logicFormatPtr->ipv6Cfg.ttl;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);
            /* retain CRC */
            value = BOOL2BIT_MAC(logicFormatPtr->ipv6Cfg.retainCRC);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);
            /*  ip header protocol */
            PRV_TUNNEL_START_IP_HEADER_PROTOCOL_CHECK_MAC(logicFormatPtr->ipv6Cfg.ipHeaderProtocol);

            if((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_FALSE) &&
                (logicFormatPtr->ipv6Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            PRV_TUNNEL_START_CONVERT_IP_HEADER_PROTOCOL_TO_HW_FORMAT_MAC(logicFormatPtr->ipv6Cfg.ipHeaderProtocol,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E);

            /* gre fields */
            if (value == 1)/* GRE protocol*/
            {
                PRV_TUNNEL_START_GRE_FLAGS_AND_VERSION_CHECK_MAC(logicFormatPtr->ipv6Cfg.greFlagsAndVersion);

                value = logicFormatPtr->ipv6Cfg.greFlagsAndVersion;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E);

                /*  GRE Protocol for Ethernet Passenger*/
                PRV_TUNNEL_START_GRE_PROTOCOL_FOR_ETHERNET_CHECK_MAC(logicFormatPtr->ipv6Cfg.greProtocolForEthernet);
                value = logicFormatPtr->ipv6Cfg.greProtocolForEthernet;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E);
            }
            else
                if (value == 2) /* UDP protocol*/
                {
                    /* udp dst port */
                    PRV_TUNNEL_START_UDP_DST_PORT_CHECK_MAC(logicFormatPtr->ipv6Cfg.udpDstPort);
                    value = logicFormatPtr->ipv6Cfg.udpDstPort;
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E);
                    /* udp src port */
                    PRV_TUNNEL_START_UDP_SRC_PORT_CHECK_MAC(logicFormatPtr->ipv6Cfg.udpSrcPort);
                    value = logicFormatPtr->ipv6Cfg.udpSrcPort;
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E);
                }
            else if (value == 3) /* generic protocol*/
            {
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_TRUE)
                {
                    PRV_TUNNEL_START_IP_PROTOCOL_CHECK_MAC(logicFormatPtr->ipv6Cfg.ipProtocol);
                    value = logicFormatPtr->ipv6Cfg.ipProtocol;
                    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E);
                }
            }

            /* dscp mark mode */
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv6Cfg.dscpMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv6Cfg.dscpMarkMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E);
            /* dscp value */
            if (logicFormatPtr->ipv6Cfg.dscpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                value = logicFormatPtr->ipv6Cfg.dscp;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E);
            }

            /* flow label mode  */
            PRV_TUNNEL_START_FLOW_LABEL_MODE_CHECK_AND_CONVERT_TO_HW_FORMAT_MAC(logicFormatPtr->ipv6Cfg.flowLabelMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE_E);

            /* profile index */
            PRV_TUNNEL_START_PROFILE_INDEX_CHECK_MAC(logicFormatPtr->ipv6Cfg.profileIndex);
            value = logicFormatPtr->ipv6Cfg.profileIndex;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E);

            {
                GT_U32 wordNum;
                /* ipv6 destination address */
                for(wordNum = 0; wordNum < 4; wordNum++)
                {
                    /* first register comprize LSB of IP address- bits 31:0
                       second register comprize bits 63:32 and so on  */
                    hwFormatArray[4+wordNum] =    (logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4)]<< 24)      |
                                                  (logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 1] << 16) |
                                                  (logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 2] << 8)  |
                                                  (logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 3]);
               }

                 /* ipv6 src address */
                for(wordNum = 0; wordNum < 4; wordNum++)
                {
                    /* first register comprize LSB of IP address- bits 31:0
                       second register comprize bits 63:32 and so on  */
                    hwFormatArray[8+wordNum] =    (logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4)]<< 24)      |
                                                  (logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 1] << 16) |
                                                  (logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 2] << 8)  |
                                                  (logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 3]);
               }
            }

        break;
        case CPSS_TUNNEL_MAC_IN_MAC_E:
            hwTunnelType = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MIM_E;
            PRV_TUNNEL_START_MIM_I_SID_MODE_CHECK_MAC(logicFormatPtr->mimCfg.iSidAssignMode);
            PRV_TUNNEL_START_MIM_B_DA_MODE_CHECK_MAC(logicFormatPtr->mimCfg.bDaAssignMode);
            PRV_TUNNEL_START_MIM_B_SA_MODE_CHECK_MAC(logicFormatPtr->mimCfg.bSaAssignMode);

            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.upMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.upMarkMode,upMarkMode);
            GET_FIELD_FROM_USER_MAC(mimCfg,up);
            GET_FIELD_FROM_USER_MAC(mimCfg,tagEnable);
            GET_FIELD_FROM_USER_MAC(mimCfg,vlanId);
            GET_FIELD_FROM_USER_MAC(mimCfg,macDa);

            value = BOOL2BIT_MAC(logicFormatPtr->mimCfg.retainCrc);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);

            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.iUpMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.iUpMarkMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE_E);

            if (logicFormatPtr->mimCfg.iUpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                value = logicFormatPtr->mimCfg.iUp;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_E);
            }

            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.iDpMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.iDpMarkMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE_E);

            if (logicFormatPtr->mimCfg.iDpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                value = logicFormatPtr->mimCfg.iDp;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_E);
            }

            value = U32_GET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,3,1);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA_E);
            value = U32_GET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,2,1);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1_E);
            value = U32_GET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,0,2);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2_E);

            PRV_TUNNEL_START_CONVERT_MIM_I_SID_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.iSidAssignMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE_E);

            if (logicFormatPtr->mimCfg.iSidAssignMode == CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E)
            {
                value = logicFormatPtr->mimCfg.iSid;
                HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_E);
            }

            PRV_TUNNEL_START_CONVERT_MIM_B_DA_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.bDaAssignMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE_E);

            PRV_TUNNEL_START_CONVERT_MIM_B_SA_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.bSaAssignMode,value);
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE_E);
            break;

        case CPSS_TUNNEL_TRILL_E:
            hwTunnelType = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_TRILL_E;
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->trillCfg.upMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->trillCfg.upMarkMode,upMarkMode);
            GET_FIELD_FROM_USER_MAC(trillCfg,up);
            GET_FIELD_FROM_USER_MAC(trillCfg,tagEnable);
            GET_FIELD_FROM_USER_MAC(trillCfg,vlanId);
            macDa = logicFormatPtr->trillCfg.nextHopMacDa;

            /* build the 48 bits 'Trill header' */
            trillHeaderWords[0] = 0;
            trillHeaderWords[1] = 0;

            value = (logicFormatPtr->trillCfg.trillHeader.version << 14) |
                    (BOOL2BIT_MAC(logicFormatPtr->trillCfg.trillHeader.mBit) << 11) |
                    (logicFormatPtr->trillCfg.trillHeader.opLength << 6) |
                    logicFormatPtr->trillCfg.trillHeader.hopCount;
            U32_SET_FIELD_IN_ENTRY_MAC(trillHeaderWords,32,16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(trillHeaderWords,16,16,logicFormatPtr->trillCfg.trillHeader.eRbid);
            U32_SET_FIELD_IN_ENTRY_MAC(trillHeaderWords, 0,16,logicFormatPtr->trillCfg.trillHeader.iRbid);

            /* set the field 'TRILL_HEADER' */
            U_ANY_SET_FIELD_BY_ID_MAC(hwFormatArray,
                PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr,
                    SIP5_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER_E,
                    trillHeaderWords);

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /*********************/
    /* set common fields */
    /*********************/

    value = hwTunnelType;
    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE_E);

    value = upMarkMode;
    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE_E);

     /* the vlanId is used for DSA tag when the TS send via cascade port (even if tagEnable == GT_FALSE)
     * NOTE: this issue currently not documented in the FS.
     * */
    value = vlanId;
    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_VID_E);


    value = BOOL2BIT_MAC(tagEnable);
    HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE_E);

    if (tagEnable == GT_TRUE)
    {
        HW_TO_SW_QOS_MARK_MODE_MAC(sw_upMarkMode,upMarkMode);
        if(sw_upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
        {
            value = up;
            HA_TUNNEL_START_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_E);
        }
    }

    /*SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E*/
    SIP5_HA_TUNNEL_START_FIELD_NEXT_HOP_MAC_DA_SET_MAC(devNum,hwFormatArray,&macDa.arEther[0]);


    return GT_OK;
}

static GT_STATUS tunnelStartConfigLogic2HwFormat_sip6
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr,
    OUT GT_U32                              *hwFormatArray
)
{
    GT_U32      value;
    GT_U32      up;
    GT_BOOL     tagEnable;
    GT_U16      vlanId;
    GT_ETHERADDR macDa;
    GT_U32      i;
    switch (tunnelType)
    {
        case CPSS_TUNNEL_GENERIC_E:
            value = TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_E;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TUNNEL_TYPE_E);

            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->genCfg.upMarkMode);
            PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->genCfg.upMarkMode,value);
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_MARKING_MODE_E);

            GET_FIELD_FROM_USER_MAC(genCfg,up);
            GET_FIELD_FROM_USER_MAC(genCfg,tagEnable);
            GET_FIELD_FROM_USER_MAC(genCfg,vlanId);
            GET_FIELD_FROM_USER_MAC(genCfg,macDa);

            if (tagEnable == GT_TRUE)
            {
                value = up;
                HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_E);
            }

            value = tagEnable;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TAG_ENABLE_E);

            value = vlanId;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_VID_E);

            /* retain CRC */
            value = BOOL2BIT_MAC(logicFormatPtr->genCfg.retainCrc);
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RETAIN_INNER_CRC_E);
            /* generic TS type */
            switch(logicFormatPtr->genCfg.genericType)
            {
                case CPSS_DXCH_TUNNEL_START_GENERIC_SHORT_TYPE_E:
                    value = 0;
                    break;
                case CPSS_DXCH_TUNNEL_START_GENERIC_MEDIUM_TYPE_E:
                    value = 1;
                    /* set <Data1 bytes> */
                    cpssOsMemSet(&hwFormatArray[3], 0, 12);
                    for (i = 0; i < 12; i++)
                        U32_SET_FIELD_MAC(hwFormatArray[3 + (i >>2 )], 8 * (i & 0x03), 8, logicFormatPtr->genCfg.data[11-i]);
                    break;
                case CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E:
                    value = 3;
                    cpssOsMemSet(&hwFormatArray[3], 0, 36);
                    /* set <Data1 bytes> */
                    for (i = 0; i < 12; i++)
                        U32_SET_FIELD_MAC(hwFormatArray[3 + (i >>2 )], 8 * (i & 0x03), 8, logicFormatPtr->genCfg.data[11-i]);
                    /* set <Data2 bytes> */
                    for (i = 0; i < 24; i++)
                        U32_SET_FIELD_MAC(hwFormatArray[6 + (i >> 2)], 8 * (i & 0x03), 8, logicFormatPtr->genCfg.data[35-i]);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_GENERIC_TUNNEL_START_TYPE_E);
            SIP6_HA_TUNNEL_START_GENERIC_FIELD_MAC_DA_SET_MAC(devNum,hwFormatArray,&macDa.arEther[0]);
            /* ethertype */
            value = logicFormatPtr->genCfg.etherType;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_SET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_ETHER_TYPE_E);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal tunnelStartProfileConfigHw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwFormatArray            - The configuration in HW format (6 words).
*
* @param[out] logicFormatPtr           - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS tunnelStartProfileConfigHw2LogicFormat
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          *hwFormatArray,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *logicFormatPtr
)
{
    GT_U32 i;
    GT_U32 value;
    GT_U32 templateDataHw;
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    /* handle template size */
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE_E);
    templateDataHw = value;
    PRV_TUNNEL_START_GEN_PROFILE_TEMPLATE_DATA_SIZE_HW_TO_SW_MAC(logicFormatPtr->templateDataSize,value);

    /* handle template data */
    for (i = 0; i < templateDataHw*4*8; i++)
    {
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,genTunnelStartProfilesFields[i]);
        PRV_TUNNEL_START_GEN_PROFILE_TEMPLATE_DATA_VALUE_HW_TO_SW_MAC(logicFormatPtr->templateDataBitsCfg[i], value);
    }

    /* handle hash circular shift left */
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT_E);
    logicFormatPtr->hashShiftLeftBitsNumber = value;

    /* handle UDP source port mode */
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_UDP_SRC_PORT_MODE_E);
    PRV_TUNNEL_START_GEN_PROFILE_UDP_SRC_MODE_HW_TO_SW_MAC(logicFormatPtr->udpSrcPortMode, value);

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* handle Generic TS Service-Id Circular Shift Size */
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_SERVICE_ID_E);
        logicFormatPtr->serviceIdCircularShiftSize = value;

        /* handle Generic TS MAC DA Mode */
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_MAC_DA_MODE_E);
        logicFormatPtr->macDaMode = value;

        /* handle Generic TS Profile DIP Mode */
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE_E);
        logicFormatPtr->dipMode = value;

        /* handle MPLS control word index */
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX_E);
        logicFormatPtr->controlWordIndex = value;
    }

    return GT_OK;
}

/**
* @internal tunnelStartConfigHw2LogicFormat_sip5 function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwFormatArray            - The configuration in HW format (6 words).
*
* @param[out] tunnelTypePtr            - points to tunnel type
* @param[out] logicFormatPtr           - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS tunnelStartConfigHw2LogicFormat_sip5
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_TUNNEL_TYPE_ENT                *tunnelTypePtr,
    OUT CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr
)
{
    CPSS_TUNNEL_TYPE_ENT  swTunnelType; /* SW tunnel type */
    GT_U32      value;
    TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT      hwTunnelType;
    GT_U32      upMarkMode;         /* UP marking mode in hw format   */
    GT_U32      up;                 /* UP */
    GT_BOOL     tagEnable;
    GT_U16      vlanId;
    GT_ETHERADDR    macDa;
    GT_U32      trillHeaderWords[2];

    devNum = devNum;

    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE_E);
    hwTunnelType = (TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT)value;

    /*********************/
    /* get common fields */
    /*********************/

    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE_E);
    upMarkMode = value;

    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE_E);
    tagEnable = BIT2BOOL_MAC(value);

    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_VID_E);
    vlanId = (GT_U16)value;

    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_E);
    up = value;

    /*SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E*/
    SIP5_HA_TUNNEL_START_FIELD_NEXT_HOP_MAC_DA_GET_MAC(devNum,hwFormatArray,&macDa.arEther[0]);


    switch(hwTunnelType)
    {
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MPLS_E:
            swTunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mplsCfg.upMarkMode,upMarkMode);
            SET_FIELD_TO_USER_MAC(mplsCfg,up);
            SET_FIELD_TO_USER_MAC(mplsCfg,tagEnable);
            SET_FIELD_TO_USER_MAC(mplsCfg,vlanId);
            SET_FIELD_TO_USER_MAC(mplsCfg,macDa);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);
            logicFormatPtr->mplsCfg.ttl = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE_E);
            HW_TO_SW_TTL_MODE_MAC(logicFormatPtr->mplsCfg.ttlMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);
            logicFormatPtr->mplsCfg.retainCRC = BIT2BOOL_MAC(value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS_E);
            logicFormatPtr->mplsCfg.numLabels = value + 1;

            /* label #1 */

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL1_E);
            logicFormatPtr->mplsCfg.label1 = value;

            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1_E);
                logicFormatPtr->mplsCfg.pushEliAndElAfterLabel1 = BIT2BOOL_MAC(value);
            }

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE_E);
            HW_TO_SW_MPLS_PW_EXP_MARK_MODE_MAC(logicFormatPtr->mplsCfg.pwExpMarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE_E);
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mplsCfg.exp1MarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_E);
            logicFormatPtr->mplsCfg.exp1 = value;

            /* label #2 */
            if(logicFormatPtr->mplsCfg.numLabels >= 2)
            {
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL2_E);
                logicFormatPtr->mplsCfg.label2 = value;

                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2_E);
                    logicFormatPtr->mplsCfg.pushEliAndElAfterLabel2 = BIT2BOOL_MAC(value);
                }

                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE_E);
                HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mplsCfg.exp2MarkMode,value);

                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_E);
                logicFormatPtr->mplsCfg.exp2 = value;

                /* label #3 */
                if(logicFormatPtr->mplsCfg.numLabels >= 3)
                {
                    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL3_E);
                    logicFormatPtr->mplsCfg.label3 = value;

                    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3_E);
                        logicFormatPtr->mplsCfg.pushEliAndElAfterLabel3 = BIT2BOOL_MAC(value);
                    }

                    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE_E);
                    HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mplsCfg.exp3MarkMode,value);

                    HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_E);
                    logicFormatPtr->mplsCfg.exp3 = value;
                }
            }


            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT_E);
            logicFormatPtr->mplsCfg.setSBit = BIT2BOOL_MAC(value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX_E);
            logicFormatPtr->mplsCfg.controlWordEnable = (value > 0) ? GT_TRUE : GT_FALSE;
            logicFormatPtr->mplsCfg.controlWordIndex = value - 1;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL_E);
            HW_TO_SW_MPLS_ETHER_TYPE_MAC(logicFormatPtr->mplsCfg.mplsEthertypeSelect,value);

            break;
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV4_E:
            swTunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->ipv4Cfg.upMarkMode,upMarkMode);
            SET_FIELD_TO_USER_MAC(ipv4Cfg,up);
            SET_FIELD_TO_USER_MAC(ipv4Cfg,tagEnable);
            SET_FIELD_TO_USER_MAC(ipv4Cfg,vlanId);
            SET_FIELD_TO_USER_MAC(ipv4Cfg,macDa);


            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);
            logicFormatPtr->ipv4Cfg.ttl = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E);
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->ipv4Cfg.dscpMarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E);
            logicFormatPtr->ipv4Cfg.dscp = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG_E);
            logicFormatPtr->ipv4Cfg.dontFragmentFlag = BIT2BOOL_MAC(value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG_E);
            logicFormatPtr->ipv4Cfg.autoTunnel = BIT2BOOL_MAC(value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET_E);
            logicFormatPtr->ipv4Cfg.autoTunnelOffset = value;
            /* profile number */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E);
            logicFormatPtr->ipv4Cfg.profileIndex = value;
            /* retain crc */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);
            logicFormatPtr->ipv4Cfg.retainCRC = BIT2BOOL_MAC(value);

            /* ip header protocol */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E);
            HW_TO_SW_IP_HEADER_PROTOCOL_MAC(logicFormatPtr->ipv4Cfg.ipHeaderProtocol,value);
            if (logicFormatPtr->ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E)
            {
                /*  GRE flags and version */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E);
                logicFormatPtr->ipv4Cfg.greFlagsAndVersion = value;
                /*  GRE protocol for ethernet passenger*/
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E);
                logicFormatPtr->ipv4Cfg.greProtocolForEthernet = value;
            }
            if (logicFormatPtr->ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E)
            {
                /* udp dst port */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E);
                logicFormatPtr->ipv4Cfg.udpDstPort = value;
                /* udp src port */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E);
                logicFormatPtr->ipv4Cfg.udpSrcPort = value;
            }

            if (logicFormatPtr->ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
            {
                /* generic ip protocol number */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E);
                logicFormatPtr->ipv4Cfg.ipProtocol = value;

            }

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DIP_E);
            logicFormatPtr->ipv4Cfg.destIp.arIP[0] = (GT_U8)(value >> 24);
            logicFormatPtr->ipv4Cfg.destIp.arIP[1] = (GT_U8)(value >> 16);
            logicFormatPtr->ipv4Cfg.destIp.arIP[2] = (GT_U8)(value >>  8);
            logicFormatPtr->ipv4Cfg.destIp.arIP[3] = (GT_U8)(value >>  0);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SIP_E);
            logicFormatPtr->ipv4Cfg.srcIp.arIP[0] = (GT_U8)(value >> 24);
            logicFormatPtr->ipv4Cfg.srcIp.arIP[1] = (GT_U8)(value >> 16);
            logicFormatPtr->ipv4Cfg.srcIp.arIP[2] = (GT_U8)(value >>  8);
            logicFormatPtr->ipv4Cfg.srcIp.arIP[3] = (GT_U8)(value >>  0);

            break;
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_IPV6_E:
            swTunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->ipv6Cfg.upMarkMode,upMarkMode);
            SET_FIELD_TO_USER_MAC(ipv6Cfg,up);
            SET_FIELD_TO_USER_MAC(ipv6Cfg,tagEnable);
            SET_FIELD_TO_USER_MAC(ipv6Cfg,vlanId);
            SET_FIELD_TO_USER_MAC(ipv6Cfg,macDa);


            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E);
            logicFormatPtr->ipv6Cfg.ttl = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E);
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->ipv6Cfg.dscpMarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E);
            logicFormatPtr->ipv6Cfg.dscp = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE_E);
            HW_TO_SW_FLOW_LABEL_MODE_MAC(logicFormatPtr->ipv6Cfg.flowLabelMode,value);

            /* profile number */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E);
            logicFormatPtr->ipv6Cfg.profileIndex = value;
            /* retain crc */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);
            logicFormatPtr->ipv6Cfg.retainCRC = BIT2BOOL_MAC(value);

            /* ip header protocol */
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E);
            HW_TO_SW_IP_HEADER_PROTOCOL_MAC(logicFormatPtr->ipv6Cfg.ipHeaderProtocol,value);
            if (logicFormatPtr->ipv6Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E)
            {
                /*  GRE flags and version */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E);
                logicFormatPtr->ipv6Cfg.greFlagsAndVersion = value;
                /*  GRE protocol for ethernet passenger*/
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E);
                logicFormatPtr->ipv6Cfg.greProtocolForEthernet = value;
            }
            if (logicFormatPtr->ipv6Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E)
            {
                /* udp dst port */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E);
                logicFormatPtr->ipv6Cfg.udpDstPort = value;
                /* udp src port */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E);
                logicFormatPtr->ipv6Cfg.udpSrcPort = value;
            }

            if (logicFormatPtr->ipv6Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
            {
                /* generic ip protocol number */
                HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E);
                logicFormatPtr->ipv6Cfg.ipProtocol = value;

            }

            {
                GT_U32 wordNum;
                /* ipv6 destination address */
                for(wordNum = 0; wordNum < 4; wordNum++)
                {
                    /* first register comprize LSB of IP address- bits 31:0
                       second register comprize bits 63:32 and so on  */
                    logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4)]     = (GT_U8)(hwFormatArray[4+wordNum] >> 24);
                    logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 1] = (GT_U8)(hwFormatArray[4+wordNum] >> 16);
                    logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 2] = (GT_U8)(hwFormatArray[4+wordNum] >> 8);
                    logicFormatPtr->ipv6Cfg.destIp.arIP[((3 - wordNum) * 4) + 3] = (GT_U8)(hwFormatArray[4+wordNum]);
               }

                 /* ipv6 src address */
                for(wordNum = 0; wordNum < 4; wordNum++)
                {
                    /* first register comprize LSB of IP address- bits 31:0
                       second register comprize bits 63:32 and so on  */
                    logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4)]     = (GT_U8)(hwFormatArray[8+wordNum] >> 24);
                    logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 1] = (GT_U8)(hwFormatArray[8+wordNum] >> 16);
                    logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 2] = (GT_U8)(hwFormatArray[8+wordNum] >> 8);
                    logicFormatPtr->ipv6Cfg.srcIp.arIP[((3 - wordNum) * 4) + 3] = (GT_U8)(hwFormatArray[8+wordNum]);
               }
            }

            break;
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_MIM_E:
            swTunnelType = CPSS_TUNNEL_MAC_IN_MAC_E;

            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mimCfg.upMarkMode,upMarkMode);
            SET_FIELD_TO_USER_MAC(mimCfg,up);
            SET_FIELD_TO_USER_MAC(mimCfg,tagEnable);
            SET_FIELD_TO_USER_MAC(mimCfg,vlanId);
            SET_FIELD_TO_USER_MAC(mimCfg,macDa);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E);
            logicFormatPtr->mimCfg.retainCrc = BIT2BOOL_MAC(value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE_E);
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mimCfg.iUpMarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_E);
            logicFormatPtr->mimCfg.iUp = value;

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE_E);
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->mimCfg.iDpMarkMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_E);
            logicFormatPtr->mimCfg.iDp = value;

            logicFormatPtr->mimCfg.iTagReserved = 0;
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA_E);
            U32_SET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,3,1,value);
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1_E);
            U32_SET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,2,1,value);
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2_E);
            U32_SET_FIELD_MAC(logicFormatPtr->mimCfg.iTagReserved,0,2,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE_E);
            HW_TO_SW_I_SID_ASSIGN_MODE_MAC(logicFormatPtr->mimCfg.iSidAssignMode,value);
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_E);
            logicFormatPtr->mimCfg.iSid = value;
            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE_E);
            HW_TO_SW_B_DA_ASSIGN_MODE_MAC(logicFormatPtr->mimCfg.bDaAssignMode,value);

            HA_TUNNEL_START_FIELD_VALUE_GET_MAC(devNum,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE_E);
            HW_TO_SW_B_SA_ASSIGN_MODE_MAC(logicFormatPtr->mimCfg.bSaAssignMode,value);

            break;
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_TRILL_E:
            swTunnelType = CPSS_TUNNEL_TRILL_E;

            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->trillCfg.upMarkMode,upMarkMode);
            SET_FIELD_TO_USER_MAC(trillCfg,up);
            SET_FIELD_TO_USER_MAC(trillCfg,tagEnable);
            SET_FIELD_TO_USER_MAC(trillCfg,vlanId);
            logicFormatPtr->trillCfg.nextHopMacDa = macDa;

            /* get the field 'TRILL_HEADER' */
            U_ANY_GET_FIELD_BY_ID_MAC(hwFormatArray,
                PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr,
                    SIP5_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER_E,
                    trillHeaderWords);

            /* parse the 48 bits 'Trill header' */
            U32_GET_FIELD_IN_ENTRY_MAC(trillHeaderWords,32,16,value);
            U32_GET_FIELD_IN_ENTRY_MAC(trillHeaderWords,16,16,logicFormatPtr->trillCfg.trillHeader.eRbid);
            U32_GET_FIELD_IN_ENTRY_MAC(trillHeaderWords, 0,16,logicFormatPtr->trillCfg.trillHeader.iRbid);

            logicFormatPtr->trillCfg.trillHeader.version = U32_GET_FIELD_MAC(value,14,2);
            logicFormatPtr->trillCfg.trillHeader.mBit = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value,11,1));
            logicFormatPtr->trillCfg.trillHeader.opLength = U32_GET_FIELD_MAC(value,6,5);
            logicFormatPtr->trillCfg.trillHeader.hopCount = U32_GET_FIELD_MAC(value,0,6);

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    *tunnelTypePtr = swTunnelType;

    return GT_OK;
}

/**
* @internal tunnelStartConfigHw2LogicFormat_sip6 function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwFormatArray            - The configuration in HW format (6 words).
*
* @param[out] tunnelTypePtr            - points to tunnel type
* @param[out] logicFormatPtr           - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS tunnelStartConfigHw2LogicFormat_sip6
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_TUNNEL_TYPE_ENT                *tunnelTypePtr,
    OUT CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr
)
{
    CPSS_TUNNEL_TYPE_ENT  swTunnelType; /* SW tunnel type */
    GT_U32      value;
    TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT      hwTunnelType;
    GT_U32      upMarkMode;         /* UP marking mode in hw format   */
    GT_U32      i;
    HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TUNNEL_TYPE_E);
    hwTunnelType = (TUNNEL_START_HW_FIELD_TUNNEL_TYPE_ENT)value;

    switch(hwTunnelType)
    {
        case TUNNEL_START_HW_FIELD_TUNNEL_TYPE_GENERIC_E:
            swTunnelType = CPSS_TUNNEL_GENERIC_E;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_MARKING_MODE_E);
            upMarkMode = value;
            HW_TO_SW_QOS_MARK_MODE_MAC(logicFormatPtr->genCfg.upMarkMode,upMarkMode);
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TAG_ENABLE_E);
            logicFormatPtr->genCfg.tagEnable = BIT2BOOL_MAC(value);
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_VID_E);
            logicFormatPtr->genCfg.vlanId = (GT_U16)value;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_E);
            logicFormatPtr->genCfg.up = value;
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_GENERIC_TUNNEL_START_TYPE_E);

            switch (value)
            {
            case 0:
                logicFormatPtr->genCfg.genericType = CPSS_DXCH_TUNNEL_START_GENERIC_SHORT_TYPE_E;
                break;
            case 1:
                logicFormatPtr->genCfg.genericType = CPSS_DXCH_TUNNEL_START_GENERIC_MEDIUM_TYPE_E;
                break;
            case 3:
                logicFormatPtr->genCfg.genericType = CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E;
                break;               
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RETAIN_INNER_CRC_E);
            logicFormatPtr->genCfg.retainCrc = BIT2BOOL_MAC(value);
            SIP6_HA_TUNNEL_START_GENERIC_FIELD_MAC_DA_GET_MAC(devNum, hwFormatArray, &logicFormatPtr->genCfg.macDa.arEther[0]);
            HA_TUNNEL_START_GENERIC_FIELD_VALUE_GET_MAC(devNum, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_ETHER_TYPE_E);
            logicFormatPtr->genCfg.etherType = value;
            if(logicFormatPtr->genCfg.genericType == CPSS_DXCH_TUNNEL_START_GENERIC_MEDIUM_TYPE_E ||
                logicFormatPtr->genCfg.genericType == CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E)
            {
                /* get <Data1 bytes> */
                for (i = 0; i < 12; i++)
                    logicFormatPtr->genCfg.data[11-i] = U32_GET_FIELD_MAC(hwFormatArray[3+(i >>2)], 8 * (i & 0x03), 8);
            }
            if (logicFormatPtr->genCfg.genericType == CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E)
            {
                /* get <Data2 bytes> */
                for (i = 0; i < 24; i++)
                    logicFormatPtr->genCfg.data[35-i] = U32_GET_FIELD_MAC(hwFormatArray[6 + (i >> 2)], 8 * (i & 0x03), 8);
            }
            break;
        default:
            return tunnelStartConfigHw2LogicFormat_sip5(devNum,
                hwFormatArray,tunnelTypePtr,logicFormatPtr);
    }

    *tunnelTypePtr = swTunnelType;

    return GT_OK;
}

/**
* @internal tunnelStartProfileConfigLogic2HwFormat function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from logic format
*         to hardware format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to genric tunnel start profile configuration
*
* @param[out] hwFormatArray            - The configuration in HW format (17 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS tunnelStartProfileConfigLogic2HwFormat
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   *logicFormatPtr,
    OUT GT_U32                                      *hwFormatArray
)
{
    GT_U32 i;
    GT_U32 value;
    GT_U32 templateSizeHw;
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
     /* zero out hw format */
    cpssOsMemSet(hwFormatArray,0,sizeof(GT_U32)*TUNNEL_START_PROFILE_HW_SIZE_CNS);

    /* handle template size */
    PRV_TUNNEL_START_GEN_PROFILE_CHECK_TEMPLATE_DATA_SIZE_AND_CONVERT_TO_HW_FORMAT_MAC(logicFormatPtr->templateDataSize, value);
    templateSizeHw = value;
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE_E);
    /*templateDataSize in hw format could be 0,1,2,3,4 :respectively 0,4,8,12,16 byte of template data size*/

    /* handle template data */
    for (i = 0; i < templateSizeHw*4*8; i++)
    {
        PRV_TUNNEL_START_GEN_PROFILE_CHECK_TEMPLATE_DATA_VALUE_AND_CONVERT_TO_HW_FORMAT_MAC(logicFormatPtr->templateDataBitsCfg[i], value);
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,genTunnelStartProfilesFields[i]);
    }
    /* handle hash circular shift left */
    value = logicFormatPtr->hashShiftLeftBitsNumber;
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT_E);

    /* handle UDP source port mode */
    PRV_TUNNEL_START_GEN_PROFILE_CHECK_UDP_SRC_MODE_AND_CONVERT_TO_HW_FORMAT_MAC(logicFormatPtr->udpSrcPortMode, value);
    HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_UDP_SRC_PORT_MODE_E);

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* handle Service-Id Circular Shift Size */
        value = logicFormatPtr->serviceIdCircularShiftSize;
        if (value > GENERIC_IP_PROFILE_SERVICE_ID_CIRCULAR_SHIFT_SIZE_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_SERVICE_ID_E);

        /* handle MAC DA Mode */
        value = logicFormatPtr->macDaMode;
        if (value > GENERIC_IP_PROFILE_MAC_DA_MODE_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_MAC_DA_MODE_E);

        /* handle DIP Mode */
        value = logicFormatPtr->dipMode;
        if (value > GENERIC_IP_PROFILE_DIP_MODE_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE_E);

        /* handle MPLS control word index */
        value = logicFormatPtr->controlWordIndex;
        if (value > GENERIC_IP_PROFILE_CONTROL_WORD_INDEX_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        HA_GEN_TUNNEL_START_PROFILE_FIELD_VALUE_SET_MAC(devNum,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX_E);
    }

    return GT_OK;
}



/**
* @internal tunnelStartConfigLogic2HwFormat function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from logic format
*         to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] tunnelType               - type of the tunnel
* @param[in] logicFormatPtr           - points to tunnel start configuration
*
* @param[out] hwFormatArray            - The configuration in HW format (6 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS tunnelStartConfigLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr,
    OUT GT_U32                              *hwFormatArray
)
{
    GT_U32      upMarkMode;         /* UP marking mode in hw format   */
    GT_U32      dscpMarkMode;       /* DSCP marking mode in hw format */
    GT_U32      exp1MarkMode;       /* EXP1 marking mode in hw format */
    GT_U32      exp2MarkMode;       /* EXP2 marking mode in hw format */
    GT_U32      exp3MarkMode;       /* EXP3 marking mode in hw format */
    GT_U32      ttlMode;            /* TTL mode in hw format          */
    GT_U32      iUpMarkMode;        /* iUP marking mode in hw format  */
    GT_U32      iDpMarkMode;        /* iDP marking mode in hw format  */
    GT_U32      controlWordIndex= 0;/* control word index in hw format*/

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

     /* zero out hw format */
    cpssOsMemSet(hwFormatArray,0,sizeof(GT_U32)*TUNNEL_START_HW_SIZE_CNS);

    /********************/
    /* check parameters */
    /********************/

    switch (tunnelType)
    {
    case CPSS_TUNNEL_X_OVER_IPV4_E:
    case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
    case CPSS_TUNNEL_GENERIC_IPV4_E:
        /********************************/
        /* check IPv4 config parameters */
        /********************************/
        if (logicFormatPtr->ipv4Cfg.tagEnable == GT_TRUE)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->ipv4Cfg.vlanId);
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv4Cfg.upMarkMode);
            if (logicFormatPtr->ipv4Cfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->ipv4Cfg.up);
        }
        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv4Cfg.dscpMarkMode);
        if (logicFormatPtr->ipv4Cfg.dscpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(logicFormatPtr->ipv4Cfg.dscp);
        PRV_TUNNEL_START_TTL_CHECK_MAC(logicFormatPtr->ipv4Cfg.ttl);
        if (logicFormatPtr->ipv4Cfg.autoTunnel == GT_TRUE)
            PRV_TUNNEL_START_AUTO_TUNNEL_OFFSET_CHECK_MAC(logicFormatPtr->ipv4Cfg.autoTunnelOffset);
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
            PRV_TUNNEL_START_CFI_CHECK_MAC(logicFormatPtr->ipv4Cfg.cfi);
        break;
    case CPSS_TUNNEL_GENERIC_IPV6_E:
        /* check ipv6 config parameters*/
        if (logicFormatPtr->ipv6Cfg.tagEnable == GT_TRUE)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->ipv6Cfg.vlanId);
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv6Cfg.upMarkMode);
            if (logicFormatPtr->ipv6Cfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->ipv6Cfg.up);
        }
        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->ipv6Cfg.dscpMarkMode);
        if (logicFormatPtr->ipv6Cfg.dscpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(logicFormatPtr->ipv6Cfg.dscp);
        PRV_TUNNEL_START_TTL_CHECK_MAC(logicFormatPtr->ipv6Cfg.ttl);
        break;
    case CPSS_TUNNEL_X_OVER_MPLS_E:
        /********************************/
        /* check MPLS config parameters */
        /********************************/
        if (logicFormatPtr->mplsCfg.tagEnable == GT_TRUE)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->mplsCfg.vlanId);
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.upMarkMode);
            if (logicFormatPtr->mplsCfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->mplsCfg.up);
        }
        PRV_TUNNEL_START_TTL_CHECK_MAC(logicFormatPtr->mplsCfg.ttl);
        if ((logicFormatPtr->mplsCfg.ttl == 0)
                && PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            PRV_TUNNEL_START_CHECK_TTL_MODE_MAC(devNum,
                logicFormatPtr->mplsCfg.ttlMode);
        }
        switch (logicFormatPtr->mplsCfg.numLabels)
        {
        case 3:
        case 2:
        case 1:
             /* 3 labels mode */
            if (logicFormatPtr->mplsCfg.numLabels >= 3)
            {
                if(! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                PRV_TUNNEL_START_MPLS_LABEL_CHECK_MAC(logicFormatPtr->mplsCfg.label3);
                PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp3MarkMode);
                if (logicFormatPtr->mplsCfg.exp3MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                    PRV_TUNNEL_START_MPLS_EXP_CHECK_MAC(logicFormatPtr->mplsCfg.exp3);
            }
            if (logicFormatPtr->mplsCfg.numLabels >= 2)
            {
                PRV_TUNNEL_START_MPLS_LABEL_CHECK_MAC(logicFormatPtr->mplsCfg.label2);
                PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp2MarkMode);
                if (logicFormatPtr->mplsCfg.exp2MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                    PRV_TUNNEL_START_MPLS_EXP_CHECK_MAC(logicFormatPtr->mplsCfg.exp2);
            }
            PRV_TUNNEL_START_MPLS_LABEL_CHECK_MAC(logicFormatPtr->mplsCfg.label1);
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mplsCfg.exp1MarkMode);
            if (logicFormatPtr->mplsCfg.exp1MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                PRV_TUNNEL_START_MPLS_EXP_CHECK_MAC(logicFormatPtr->mplsCfg.exp1);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
            PRV_TUNNEL_START_CFI_CHECK_MAC(logicFormatPtr->mplsCfg.cfi);

        /* FEr#2785: The passenger Ethernet packet CRC cannot be retained in some cases */
         if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                                    PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
         {
             if (logicFormatPtr->mplsCfg.retainCRC == GT_TRUE)
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
         }

        controlWordIndex = (logicFormatPtr->mplsCfg.controlWordEnable == GT_TRUE ? logicFormatPtr->mplsCfg.controlWordIndex + 1 : 0);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if (controlWordIndex > CONTROL_WORD_INDEX_XCAT_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (controlWordIndex > CONTROL_WORD_INDEX_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }


        break;

    case CPSS_TUNNEL_MAC_IN_MAC_E:
        /*******************************/
        /* check MIM config parameters */
        /*******************************/        
        if (logicFormatPtr->mimCfg.tagEnable == GT_TRUE)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->mimCfg.vlanId);
            PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.upMarkMode);
            if (logicFormatPtr->mimCfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->mimCfg.up);
        }
        PRV_TUNNEL_START_ISID_CHECK_MAC(logicFormatPtr->mimCfg.iSid);
        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.iUpMarkMode);
        if (logicFormatPtr->mimCfg.iUpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->mimCfg.iUp);
        PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->mimCfg.iDpMarkMode);
        if (logicFormatPtr->mimCfg.iDpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->mimCfg.iDp);
        PRV_TUNNEL_START_ITAG_RES_CHECK_MAC(logicFormatPtr->mimCfg.iTagReserved);

        /* FEr#2785: The passenger Ethernet packet CRC cannot be retained in some cases */
         if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                                    PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
         {
             if (logicFormatPtr->mimCfg.retainCrc == GT_TRUE)
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
         }

         if(logicFormatPtr->mimCfg.bSaAssignMode == CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E)
         {
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
         }

        break;
    case CPSS_TUNNEL_TRILL_E:
        /*********************************/
        /* check TRILL config parameters */
        /*********************************/
        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

            if (logicFormatPtr->trillCfg.tagEnable == GT_TRUE)
            {
                PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->trillCfg.vlanId);
                PRV_TUNNEL_START_QOS_MARK_MODE_CHECK_MAC(logicFormatPtr->trillCfg.upMarkMode);
                if (logicFormatPtr->trillCfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->trillCfg.up);
            }
        if (logicFormatPtr->trillCfg.trillHeader.version >= BIT_2)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (logicFormatPtr->trillCfg.trillHeader.opLength >= BIT_5)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (logicFormatPtr->trillCfg.trillHeader.hopCount >= BIT_6)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (logicFormatPtr->trillCfg.trillHeader.eRbid >= BIT_16)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (logicFormatPtr->trillCfg.trillHeader.iRbid >= BIT_16)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        break;
    case CPSS_TUNNEL_GENERIC_E:
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            return tunnelStartConfigLogic2HwFormat_sip6(devNum, tunnelType, logicFormatPtr, hwFormatArray);
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*****************************************/
    /* convert paramteres to hardware format */
    /*****************************************/

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return tunnelStartConfigLogic2HwFormat_sip5(devNum,
            tunnelType,logicFormatPtr,hwFormatArray);
    }



    switch (tunnelType)
    {
    case CPSS_TUNNEL_X_OVER_IPV4_E:
    case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv4Cfg.upMarkMode,upMarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->ipv4Cfg.dscpMarkMode,dscpMarkMode);

        /* prepare hw format */
        hwFormatArray[0] = 0x1 | /* X-over-IPv4 tunnel */
            ((upMarkMode & 0x1) << 2) |
            ((logicFormatPtr->ipv4Cfg.up & 0x7) << 3) |
            (((logicFormatPtr->ipv4Cfg.tagEnable == GT_TRUE) ? 1 : 0) << 6) |
            ((logicFormatPtr->ipv4Cfg.vlanId & 0xfff) << 7) |
            ((logicFormatPtr->ipv4Cfg.ttl & 0xff) << 19) |
            ((logicFormatPtr->ipv4Cfg.cfi & 0x1) << 27);

        hwFormatArray[1] = GT_HW_MAC_LOW32((&logicFormatPtr->ipv4Cfg.macDa));

        hwFormatArray[2] = GT_HW_MAC_HIGH16((&logicFormatPtr->ipv4Cfg.macDa));

        hwFormatArray[3] = (logicFormatPtr->ipv4Cfg.autoTunnelOffset & 0xf) |
            (((logicFormatPtr->ipv4Cfg.autoTunnel == GT_TRUE) ? 1 : 0) << 4) |
            (((tunnelType == CPSS_TUNNEL_X_OVER_GRE_IPV4_E) ? 1 : 0) << 5) |
            ((logicFormatPtr->ipv4Cfg.dscp & 0x3f) << 6) |
            ((dscpMarkMode & 0x1) << 12) |
            (((logicFormatPtr->ipv4Cfg.dontFragmentFlag == GT_TRUE) ? 1 : 0) << 13);

        hwFormatArray[4] = (logicFormatPtr->ipv4Cfg.destIp.arIP[0] << 24) |
            (logicFormatPtr->ipv4Cfg.destIp.arIP[1] << 16) |
            (logicFormatPtr->ipv4Cfg.destIp.arIP[2] << 8) |
            (logicFormatPtr->ipv4Cfg.destIp.arIP[3]);

        hwFormatArray[5] = (logicFormatPtr->ipv4Cfg.srcIp.arIP[0] << 24) |
            (logicFormatPtr->ipv4Cfg.srcIp.arIP[1] << 16) |
            (logicFormatPtr->ipv4Cfg.srcIp.arIP[2] << 8) |
            (logicFormatPtr->ipv4Cfg.srcIp.arIP[3]);

        break;

    case CPSS_TUNNEL_X_OVER_MPLS_E:
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.upMarkMode,upMarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp1MarkMode,exp1MarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp2MarkMode,exp2MarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.exp3MarkMode,exp3MarkMode);
        PRV_TUNNEL_START_CONVERT_TTL_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mplsCfg.ttlMode,ttlMode);

        /* prepare hw format */
        hwFormatArray[0] = 0x0 | /* X-over-MPLS tunnel */
            ((upMarkMode & 0x1) << 2) |
            ((logicFormatPtr->mplsCfg.up & 0x7) << 3) |
            (((logicFormatPtr->mplsCfg.tagEnable == GT_TRUE) ? 1 : 0) << 6) |
            ((logicFormatPtr->mplsCfg.vlanId & 0xfff) << 7) |
            ((logicFormatPtr->mplsCfg.ttl & 0xff) << 19) |
            ((logicFormatPtr->mplsCfg.cfi & 0x1) << 27);

        hwFormatArray[1] = GT_HW_MAC_LOW32((&logicFormatPtr->mplsCfg.macDa));

        hwFormatArray[2] = GT_HW_MAC_HIGH16((&logicFormatPtr->mplsCfg.macDa));

        hwFormatArray[3] = ((logicFormatPtr->mplsCfg.label1 & 0xfffff) << 0) |
            ((logicFormatPtr->mplsCfg.exp1 & 0x7) << 20) |
            ((exp1MarkMode & 0x1) << 23) |
            (((logicFormatPtr->mplsCfg.numLabels - 1) & 0x1) << 24) |
            (((logicFormatPtr->mplsCfg.retainCRC == GT_TRUE) ? 1 : 0) << 25) |
            ((((logicFormatPtr->mplsCfg.numLabels - 1) >> 1) & 0x1) << 26) |
            (((logicFormatPtr->mplsCfg.setSBit == GT_TRUE) ? 1 : 0) << 27) |
            ((ttlMode & 0x3) << 28);

        hwFormatArray[4] = ((logicFormatPtr->mplsCfg.label2 & 0xfffff) << 0) |
            ((logicFormatPtr->mplsCfg.exp2 & 0x7) << 20) |
            (exp2MarkMode << 23) |
            ((controlWordIndex & 0x7) << 24);

        hwFormatArray[5] = ((logicFormatPtr->mplsCfg.label3 & 0xfffff) << 0) |
            ((logicFormatPtr->mplsCfg.exp3 & 0x7) << 20) |
            (exp3MarkMode << 23);

        break;

    case CPSS_TUNNEL_MAC_IN_MAC_E:
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.upMarkMode,upMarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.iUpMarkMode,iUpMarkMode);
        PRV_TUNNEL_START_CONVERT_QOS_MARK_MODE_TO_HW_FORMAT_MAC(logicFormatPtr->mimCfg.iDpMarkMode,iDpMarkMode);

        /* prepare hw format */
        hwFormatArray[0] = 0x2 | /* MIM tunnel */
            ((upMarkMode & 0x1) << 2) |
            ((logicFormatPtr->mimCfg.up & 0x7) << 3) |
            (((logicFormatPtr->mimCfg.tagEnable == GT_TRUE) ? 1 : 0) << 6) |
            ((logicFormatPtr->mimCfg.vlanId & 0xfff) << 7);

        hwFormatArray[1] = GT_HW_MAC_LOW32((&logicFormatPtr->mimCfg.macDa));

        hwFormatArray[2] = GT_HW_MAC_HIGH16((&logicFormatPtr->mimCfg.macDa));

        hwFormatArray[3] = ((logicFormatPtr->mimCfg.iSid & 0xffffff) << 0) |
            ((iUpMarkMode & 0x1) << 24) |
            (((logicFormatPtr->mimCfg.retainCrc == GT_TRUE) ? 1 : 0) << 25) |
            (((logicFormatPtr->mimCfg.iUp) & 0x7) << 26) |
            ((logicFormatPtr->mimCfg.iDp & 0x1) << 29) |
            ((iDpMarkMode & 0x1) << 30);

        hwFormatArray[4] = ((logicFormatPtr->mimCfg.iTagReserved & 0xf) << 0);

        hwFormatArray[5] = 0;

        break;

    CPSS_COVERITY_NON_ISSUE_BOOKMARK
    /* coverity[dead_error_begin] */
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal tunnelStartConfigHw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tunnel start configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] hwFormatArray            - The configuration in HW format (6 words).
*
* @param[out] tunnelTypePtr            - points to tunnel type
* @param[out] logicFormatPtr           - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS tunnelStartConfigHw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_TUNNEL_TYPE_ENT                *tunnelTypePtr,
    OUT CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *logicFormatPtr
)
{
    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(tunnelTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    /* clear the info that should be returned to the caller , because the 'get'
       logic may skip assign of fields that not relevant , like :
       for MPLS when 'num of labels' is less then 3 , then the info about 'label 3'
       is not assigned */
    cpssOsMemSet(logicFormatPtr,0,sizeof(CPSS_DXCH_TUNNEL_START_CONFIG_UNT));

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return tunnelStartConfigHw2LogicFormat_sip6(devNum,
                hwFormatArray,tunnelTypePtr,logicFormatPtr);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return
            tunnelStartConfigHw2LogicFormat_sip5(devNum,
                hwFormatArray,tunnelTypePtr,logicFormatPtr);
    }

    /* get tunnel type */
    switch (hwFormatArray[0] & 0x3)
    {
    case 0:
        *tunnelTypePtr = CPSS_TUNNEL_X_OVER_MPLS_E;
        break;
    case 1:
        if (((hwFormatArray[3] >> 5) & 0x1) == 0)
            *tunnelTypePtr = CPSS_TUNNEL_X_OVER_IPV4_E;
        else
            *tunnelTypePtr = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
        break;
    case 2:
        /* MIM */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            *tunnelTypePtr = CPSS_TUNNEL_MAC_IN_MAC_E;
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* convert to logical format */
    switch (*tunnelTypePtr)
    {
    case CPSS_TUNNEL_X_OVER_IPV4_E:
    case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        if (((hwFormatArray[0] >> 6) & 0x1) == 1)
            logicFormatPtr->ipv4Cfg.tagEnable = GT_TRUE;
        else
            logicFormatPtr->ipv4Cfg.tagEnable = GT_FALSE;

        if (logicFormatPtr->ipv4Cfg.tagEnable == GT_TRUE)
        {
            if (((hwFormatArray[0] >> 2) & 0x1) == 1)
                logicFormatPtr->ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

            if (logicFormatPtr->ipv4Cfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                logicFormatPtr->ipv4Cfg.up = ((hwFormatArray[0] >> 3) & 0x7);

            logicFormatPtr->ipv4Cfg.vlanId = (GT_U16)((hwFormatArray[0] >> 7) & 0xfff);
        }

        logicFormatPtr->ipv4Cfg.ttl = ((hwFormatArray[0] >> 19) & 0xff);

        /* cfi bit is relevant only for xCat3 */
        if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
            logicFormatPtr->ipv4Cfg.cfi = ((hwFormatArray[0] >> 27) & 0x1);

        logicFormatPtr->ipv4Cfg.macDa.arEther[5] = (GT_U8)((hwFormatArray[1] >>  0) & 0xff);
        logicFormatPtr->ipv4Cfg.macDa.arEther[4] = (GT_U8)((hwFormatArray[1] >>  8) & 0xff);
        logicFormatPtr->ipv4Cfg.macDa.arEther[3] = (GT_U8)((hwFormatArray[1] >> 16) & 0xff);
        logicFormatPtr->ipv4Cfg.macDa.arEther[2] = (GT_U8)((hwFormatArray[1] >> 24) & 0xff);
        logicFormatPtr->ipv4Cfg.macDa.arEther[1] = (GT_U8)((hwFormatArray[2] >>  0) & 0xff);
        logicFormatPtr->ipv4Cfg.macDa.arEther[0] = (GT_U8)((hwFormatArray[2] >>  8) & 0xff);

        if (((hwFormatArray[3] >> 4) & 0x1) == 1)
            logicFormatPtr->ipv4Cfg.autoTunnel = GT_TRUE;
        else
            logicFormatPtr->ipv4Cfg.autoTunnel = GT_FALSE;

        if (logicFormatPtr->ipv4Cfg.autoTunnel == GT_TRUE)
        {
            logicFormatPtr->ipv4Cfg.autoTunnelOffset = ((hwFormatArray[3]) & 0xf);
        }

        if (((hwFormatArray[3] >> 12) & 0x1) == 1)
            logicFormatPtr->ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        else
            logicFormatPtr->ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

        if (logicFormatPtr->ipv4Cfg.dscpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
        {
            logicFormatPtr->ipv4Cfg.dscp = ((hwFormatArray[3] >> 6) & 0x3f);
        }

        if (((hwFormatArray[3] >> 13) & 0x1) == 1)
            logicFormatPtr->ipv4Cfg.dontFragmentFlag = GT_TRUE;
        else
            logicFormatPtr->ipv4Cfg.dontFragmentFlag = GT_FALSE;

        logicFormatPtr->ipv4Cfg.destIp.arIP[3] = (GT_U8)((hwFormatArray[4] >>  0) & 0xff);
        logicFormatPtr->ipv4Cfg.destIp.arIP[2] = (GT_U8)((hwFormatArray[4] >>  8) & 0xff);
        logicFormatPtr->ipv4Cfg.destIp.arIP[1] = (GT_U8)((hwFormatArray[4] >> 16) & 0xff);
        logicFormatPtr->ipv4Cfg.destIp.arIP[0] = (GT_U8)((hwFormatArray[4] >> 24) & 0xff);

        logicFormatPtr->ipv4Cfg.srcIp.arIP[3] = (GT_U8)((hwFormatArray[5] >>  0) & 0xff);
        logicFormatPtr->ipv4Cfg.srcIp.arIP[2] = (GT_U8)((hwFormatArray[5] >>  8) & 0xff);
        logicFormatPtr->ipv4Cfg.srcIp.arIP[1] = (GT_U8)((hwFormatArray[5] >> 16) & 0xff);
        logicFormatPtr->ipv4Cfg.srcIp.arIP[0] = (GT_U8)((hwFormatArray[5] >> 24) & 0xff);

        break;

    case CPSS_TUNNEL_X_OVER_MPLS_E:
        if (((hwFormatArray[0] >> 6) & 0x1) == 1)
            logicFormatPtr->mplsCfg.tagEnable = GT_TRUE;
        else
            logicFormatPtr->mplsCfg.tagEnable = GT_FALSE;

        if (logicFormatPtr->mplsCfg.tagEnable == GT_TRUE)
        {
            if (((hwFormatArray[0] >> 2) & 0x1) == 1)
                logicFormatPtr->mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

            if (logicFormatPtr->mplsCfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            {
                logicFormatPtr->mplsCfg.up = ((hwFormatArray[0] >> 3) & 0x7);
            }

            logicFormatPtr->mplsCfg.vlanId = (GT_U16)((hwFormatArray[0] >> 7) & 0xfff);
        }

        logicFormatPtr->mplsCfg.ttl = ((hwFormatArray[0] >> 19) & 0xff);

        if ((logicFormatPtr->mplsCfg.ttl == 0)
            && PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            switch ((hwFormatArray[3] >> 28) & 0x3)
            {
            case 0:
                logicFormatPtr->mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
                break;
            case 1:
                logicFormatPtr->mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E;
                break;
            case 2:
                logicFormatPtr->mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E;
                break;
            case 3:
                if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                else
                {
                    logicFormatPtr->mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E;
                }
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            if (((hwFormatArray[3] >> 27) & 0x1) == 1)
                logicFormatPtr->mplsCfg.setSBit = GT_TRUE;
            else
                logicFormatPtr->mplsCfg.setSBit = GT_FALSE;
        }

        /* cfi bit is relevant only for DxCh3; xCat3 */
        if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
        {
            logicFormatPtr->mplsCfg.cfi = ((hwFormatArray[0] >> 27) & 0x1);
        }

        logicFormatPtr->mplsCfg.macDa.arEther[5] = (GT_U8)((hwFormatArray[1] >>  0) & 0xff);
        logicFormatPtr->mplsCfg.macDa.arEther[4] = (GT_U8)((hwFormatArray[1] >>  8) & 0xff);
        logicFormatPtr->mplsCfg.macDa.arEther[3] = (GT_U8)((hwFormatArray[1] >> 16) & 0xff);
        logicFormatPtr->mplsCfg.macDa.arEther[2] = (GT_U8)((hwFormatArray[1] >> 24) & 0xff);
        logicFormatPtr->mplsCfg.macDa.arEther[1] = (GT_U8)((hwFormatArray[2] >>  0) & 0xff);
        logicFormatPtr->mplsCfg.macDa.arEther[0] = (GT_U8)((hwFormatArray[2] >>  8) & 0xff);

        logicFormatPtr->mplsCfg.numLabels = (((hwFormatArray[3] >> 24) & 0x1) |
            (((hwFormatArray[3] >> 26) & 0x1) << 1)) + 1;

        switch (logicFormatPtr->mplsCfg.numLabels)
        {
        case 3:
            /* 3 labels mode */
            if(! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            logicFormatPtr->mplsCfg.label3 = ((hwFormatArray[5]) & 0xfffff);
            if (((hwFormatArray[5] >> 23) & 0x1) == 1)
                logicFormatPtr->mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            if (logicFormatPtr->mplsCfg.exp3MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                logicFormatPtr->mplsCfg.exp3 = ((hwFormatArray[5] >> 20) & 0x7);
            /* note fall through to check other labels too */
        case 2:
            logicFormatPtr->mplsCfg.label2 = ((hwFormatArray[4]) & 0xfffff);
            if (((hwFormatArray[4] >> 23) & 0x1) == 1)
                logicFormatPtr->mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            if (logicFormatPtr->mplsCfg.exp2MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                logicFormatPtr->mplsCfg.exp2 = ((hwFormatArray[4] >> 20) & 0x7);
            /* note fall through to check other labels too */
        case 1:
            logicFormatPtr->mplsCfg.label1 = ((hwFormatArray[3]) & 0xfffff);
            if (((hwFormatArray[3] >> 23) & 0x1) == 1)
                logicFormatPtr->mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            if (logicFormatPtr->mplsCfg.exp1MarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                logicFormatPtr->mplsCfg.exp1 = ((hwFormatArray[3] >> 20) & 0x7);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if (((hwFormatArray[3] >> 25) & 0x1) == 1)
            logicFormatPtr->mplsCfg.retainCRC = GT_TRUE;
        else
            logicFormatPtr->mplsCfg.retainCRC = GT_FALSE;

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if ((hwFormatArray[4] >> 24) & 0x7)
            {
                logicFormatPtr->mplsCfg.controlWordEnable = GT_TRUE;
                logicFormatPtr->mplsCfg.controlWordIndex = ((hwFormatArray[4] >> 24) & 0x7) - 1;
            }
        }

        break;

    case CPSS_TUNNEL_MAC_IN_MAC_E:
        if (((hwFormatArray[0] >> 6) & 0x1) == 1)
            logicFormatPtr->mimCfg.tagEnable = GT_TRUE;
        else
            logicFormatPtr->mimCfg.tagEnable = GT_FALSE;

        if (logicFormatPtr->mimCfg.tagEnable == GT_TRUE)
        {
            if (((hwFormatArray[0] >> 2) & 0x1) == 1)
                logicFormatPtr->mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            else
                logicFormatPtr->mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

            if (logicFormatPtr->mimCfg.upMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
                logicFormatPtr->mimCfg.up = ((hwFormatArray[0] >> 3) & 0x7);

            logicFormatPtr->mimCfg.vlanId = (GT_U16)((hwFormatArray[0] >> 7) & 0xfff);
        }

        logicFormatPtr->mimCfg.macDa.arEther[5] = (GT_U8)((hwFormatArray[1] >>  0) & 0xff);
        logicFormatPtr->mimCfg.macDa.arEther[4] = (GT_U8)((hwFormatArray[1] >>  8) & 0xff);
        logicFormatPtr->mimCfg.macDa.arEther[3] = (GT_U8)((hwFormatArray[1] >> 16) & 0xff);
        logicFormatPtr->mimCfg.macDa.arEther[2] = (GT_U8)((hwFormatArray[1] >> 24) & 0xff);
        logicFormatPtr->mimCfg.macDa.arEther[1] = (GT_U8)((hwFormatArray[2] >>  0) & 0xff);
        logicFormatPtr->mimCfg.macDa.arEther[0] = (GT_U8)((hwFormatArray[2] >>  8) & 0xff);

        logicFormatPtr->mimCfg.iSid = ((hwFormatArray[3] >> 0) & 0xffffff);

        if (((hwFormatArray[3] >> 25) & 0x1) == 1)
            logicFormatPtr->mimCfg.retainCrc = GT_TRUE;
        else
            logicFormatPtr->mimCfg.retainCrc = GT_FALSE;

        if (((hwFormatArray[3] >> 24) & 0x1) == 1)
            logicFormatPtr->mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        else
            logicFormatPtr->mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

        if (logicFormatPtr->mimCfg.iUpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            logicFormatPtr->mimCfg.iUp = ((hwFormatArray[3] >> 26) & 0x7);

        if (((hwFormatArray[3] >> 30) & 0x1) == 1)
            logicFormatPtr->mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        else
            logicFormatPtr->mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;

        if (logicFormatPtr->mimCfg.iDpMarkMode == CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E)
            logicFormatPtr->mimCfg.iDp = ((hwFormatArray[3] >> 29) & 0x1);

        logicFormatPtr->mimCfg.iTagReserved = ((hwFormatArray[4] >> 0) & 0xf);

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTunnelStartEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start /NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
)
{
    return cpssDxChTunnelStartPortGroupEntrySet(devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                routerArpTunnelStartLineIndex,
                tunnelType,
                configPtr);
}

/**
* @internal cpssDxChTunnelStartEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start /NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerArpTunnelStartLineIndex, tunnelType, configPtr));

    rc = internal_cpssDxChTunnelStartEntrySet(devNum, routerArpTunnelStartLineIndex, tunnelType, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerArpTunnelStartLineIndex, tunnelType, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChTunnelStartPortGroupGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
)
{
    GT_U32      hwConfigArray[TUNNEL_START_PROFILE_HW_SIZE_CNS] = {0}; /* tunnel start profile config in hardware format */
    GT_STATUS   rc;         /* function return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    PRV_TUNNEL_START_PROFILE_INDEX_CHECK_MAC(profileIndex);
    CPSS_NULL_PTR_CHECK_MAC(profileDataPtr);

    /* convert tunnel start entry to hw format while checking parameters */
    rc = tunnelStartProfileConfigLogic2HwFormat(devNum,profileDataPtr,hwConfigArray);
    if (rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E,
                                             profileIndex,
                                             hwConfigArray);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    return rc;
}

/**
* @internal cpssDxChTunnelStartPortGroupGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortGroupGenProfileTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, profileIndex, profileDataPtr));

    rc = internal_cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(devNum, portGroupsBmp, profileIndex, profileDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, profileIndex, profileDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
)
{
    return cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                profileIndex,
                profileDataPtr);

}

/**
* @internal cpssDxChTunnelStartGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartGenProfileTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, profileDataPtr));

    rc = internal_cpssDxChTunnelStartGenProfileTableEntrySet(devNum, profileIndex, profileDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, profileDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPortGroupGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartPortGroupGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
)
{
    GT_U32      hwConfigArray[TUNNEL_START_PROFILE_HW_SIZE_CNS] = {0}; /* tunnel start profile config in hardware format */
    GT_STATUS   rc;         /* function return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    PRV_TUNNEL_START_PROFILE_INDEX_CHECK_MAC(profileIndex);
    CPSS_NULL_PTR_CHECK_MAC(profileDataPtr);

    if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* Get the first active port group */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                            portGroupId);
    }
    else
    {
        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }


    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                            portGroupId,
                                            CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E,
                                            profileIndex,
                                            hwConfigArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert tunnel start entry to sw format */
    rc = tunnelStartProfileConfigHw2LogicFormat(devNum,hwConfigArray,profileDataPtr);
    return rc;
}

/**
* @internal cpssDxChTunnelStartPortGroupGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortGroupGenProfileTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, profileIndex, profileDataPtr));

    rc = internal_cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(devNum, portGroupsBmp, profileIndex, profileDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, profileIndex, profileDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTunnelStartGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
)
{
    return cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(devNum,
                                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                               profileIndex,
                                                               profileDataPtr);
}

/**
* @internal cpssDxChTunnelStartGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartGenProfileTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, profileDataPtr));

    rc = internal_cpssDxChTunnelStartGenProfileTableEntryGet(devNum, profileIndex, profileDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, profileDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChTunnelStartEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
{
    return cpssDxChTunnelStartPortGroupEntryGet(devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                routerArpTunnelStartLineIndex,
                tunnelTypePtr,
                configPtr);
}

/**
* @internal cpssDxChTunnelStartEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr));

    rc = internal_cpssDxChTunnelStartEntryGet(devNum, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPortGroupEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
)
{
    GT_U32      hwConfigArray[TUNNEL_START_HW_SIZE_CNS]; /* tunnel start config in hardware format */
    GT_STATUS   rc;         /* function return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT entryType;
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_TABLE_MAC(devNum,portGroupsBmp,CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E);

    LINE_INDEX_CHECK_MAC(devNum,routerArpTunnelStartLineIndex);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    /* convert tunnel start entry to hw format while checking parameters */
    rc = tunnelStartConfigLogic2HwFormat(devNum,tunnelType,configPtr,hwConfigArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((( PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum)==GT_TRUE ) &&
        (tunnelType == CPSS_TUNNEL_GENERIC_IPV6_E)) ||
        (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && tunnelType == CPSS_TUNNEL_GENERIC_E &&
        configPtr->genCfg.genericType == CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E))
    {
        entryType = PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E;
    }
    else
    {
        entryType = PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
    rc = prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite(devNum,
                                                 portGroupId,
                                                 entryType,
                                                 routerArpTunnelStartLineIndex,
                                                 hwConfigArray);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return rc;
}

/**
* @internal cpssDxChTunnelStartPortGroupEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortGroupEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelType, configPtr));

    rc = internal_cpssDxChTunnelStartPortGroupEntrySet(devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelType, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelType, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPortGroupEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
static GT_STATUS internal_cpssDxChTunnelStartPortGroupEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
{
    GT_U32      hwConfigArray[TUNNEL_START_HW_SIZE_CNS]; /* tunnel start config in hardware format */
    GT_STATUS   rc;     /* function return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT entryType =PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_TABLE_MAC(devNum,portGroupsBmp,CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E);
    LINE_INDEX_CHECK_MAC(devNum,routerArpTunnelStartLineIndex);
    CPSS_NULL_PTR_CHECK_MAC(tunnelTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* Get the first active port group */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                            portGroupId);
    }
    else
    {
        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    if ( PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum)==GT_TRUE )
    {
        if (!(routerArpTunnelStartLineIndex & 0x1))
        {
            entryType = PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E;
        }
    }

    /* read tunnel start entry from the hardware */
    rc = prvCpssDxChHwHaTunnelStartArpNatTableEntryRead(devNum,
                                                        portGroupId,
                                                        entryType,
                                                        routerArpTunnelStartLineIndex,
                                                        hwConfigArray);
    if (rc != GT_OK)
        return rc;

    /* convert tunnel entry from hardware format to logic format */
    rc = tunnelStartConfigHw2LogicFormat(devNum,hwConfigArray,tunnelTypePtr,configPtr);

    return rc;
}

/**
* @internal cpssDxChTunnelStartPortGroupEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortGroupEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr));

    rc = internal_cpssDxChTunnelStartPortGroupEntryGet(devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, routerArpTunnelStartLineIndex, tunnelTypePtr, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChEthernetOverMplsTunnelStartTaggingSet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   -  / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
static GT_STATUS internal_cpssDxChEthernetOverMplsTunnelStartTaggingSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* write enable value */
    value = (enable == GT_TRUE) ? 1 : 0;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,27,1,value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.generalConfig;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,12,1,value);
    }

    return rc;
}

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTaggingSet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   -  / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChEthernetOverMplsTunnelStartTaggingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChEthernetOverMplsTunnelStartTaggingSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChEthernetOverMplsTunnelStartTaggingGet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points to enable / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
static GT_STATUS internal_cpssDxChEthernetOverMplsTunnelStartTaggingGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32      value;      /* register value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* read register value */
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,27,1,&value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.generalConfig;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,12,1,&value);
    }

    if (rc != GT_OK)
        return rc;

    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTaggingGet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points to enable / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChEthernetOverMplsTunnelStartTaggingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChEthernetOverMplsTunnelStartTaggingGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChEthernetOverMplsTunnelStartTagModeSet function
* @endinternal
*
* @brief   Set the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tagMode                  - tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
static GT_STATUS internal_cpssDxChEthernetOverMplsTunnelStartTagModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    switch (tagMode)
    {
    case CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E:
        data = 0;
        break;
    case CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E:
        data = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;

        return prvCpssHwPpSetRegField(devNum,regAddr,26,1,data);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.generalConfig;
        return prvCpssHwPpSetRegField(devNum,regAddr,11,1,data);
    }

}

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTagModeSet function
* @endinternal
*
* @brief   Set the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tagMode                  - tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeSet

(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChEthernetOverMplsTunnelStartTagModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tagMode));

    rc = internal_cpssDxChEthernetOverMplsTunnelStartTagModeSet(devNum, tagMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tagMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChEthernetOverMplsTunnelStartTagModeGet function
* @endinternal
*
* @brief   Get the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tagModePtr               - pointer to tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_STATE             - on bad state.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
static GT_STATUS internal_cpssDxChEthernetOverMplsTunnelStartTagModeGet
(
    IN  GT_U8                                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT *tagModePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_STATUS rc;                /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(tagModePtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueResSharingAndTunnelEgrFltr;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,26,1,&data);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.generalConfig;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,11,1,&data);
    }

    if (rc != GT_OK)
        return rc;

    switch (data)
    {
    case 0:
        *tagModePtr = CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E;
        break;
    case 1:
        *tagModePtr = CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;

}

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTagModeGet function
* @endinternal
*
* @brief   Get the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tagModePtr               - pointer to tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_STATE             - on bad state.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeGet

(
    IN  GT_U8                                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT *tagModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChEthernetOverMplsTunnelStartTagModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tagModePtr));

    rc = internal_cpssDxChEthernetOverMplsTunnelStartTagModeGet(devNum, tagModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tagModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPassengerVlanTranslationEnableSet function
* @endinternal
*
* @brief   Controls Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
static GT_STATUS internal_cpssDxChTunnelStartPassengerVlanTranslationEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
        startBit = 11;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 10;
    }

    data = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit , 1, data);

}

/**
* @internal cpssDxChTunnelStartPassengerVlanTranslationEnableSet function
* @endinternal
*
* @brief   Controls Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPassengerVlanTranslationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTunnelStartPassengerVlanTranslationEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTunnelStartPassengerVlanTranslationEnableGet function
* @endinternal
*
* @brief   Gets the Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points enable state.
*                                      GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
static GT_STATUS internal_cpssDxChTunnelStartPassengerVlanTranslationEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* hardware value */
    GT_STATUS   rc;         /* function return code */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
        startBit = 11;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 10;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &value);

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTunnelStartPassengerVlanTranslationEnableGet function
* @endinternal
*
* @brief   Gets the Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points enable state.
*                                      GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPassengerVlanTranslationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTunnelStartPassengerVlanTranslationEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet function
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
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    IN  GT_BOOL                             enable
)
{
    GT_U32      value;      /* value to write */
    GT_STATUS   rc;         /* function return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      bitOffset;  /* offset of nit from start of register */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);

    /* prepare value to write */
    value = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
                                       port,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                       SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE_E, /* field name */
                                       PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                       value);
    }
    else
    {
        /* write register value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.trunkNumConfigReg[port >> 2];
        bitOffset = (port & 0x3) + 28;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 1, value);
    }

    return rc;

}

/**
* @internal cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet function
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
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, enable));

    rc = internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(devNum, port, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet function
* @endinternal
*
* @brief   The function gets status of the feature which allows overriding the
*         <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - points to enable/disable additional offset to tunnel total length
*                                      GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* value to write */
    GT_STATUS   rc;         /* function return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      bitOffset;  /* offset of nit from start of register */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
                                       port,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                       SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE_E, /* field name */
                                       PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                       &value);
    }
    else
    {
        /* read register value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.trunkNumConfigReg[port >> 2];
        bitOffset = (port & 0x3) + 28;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 1, &value);
    }

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet function
* @endinternal
*
* @brief   The function gets status of the feature which allows overriding the
*         <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - points to enable/disable additional offset to tunnel total length
*                                      GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, enablePtr));

    rc = internal_cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(devNum, port, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet function
* @endinternal
*
* @brief   This API sets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] additionToLength         - Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range values
*/
static GT_STATUS internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              additionToLength
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    if(additionToLength >= BIT_6)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* write register value */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).IPLengthOffset;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,6,additionToLength);
    }
    else
    {
        /* write register value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCpidReg1;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,26,6,additionToLength);
    }

    return rc;
}

/**
* @internal cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet function
* @endinternal
*
* @brief   This API sets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] additionToLength         - Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range values
*/
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              additionToLength
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, additionToLength));

    rc = internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(devNum, additionToLength);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, additionToLength));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet function
* @endinternal
*
* @brief   This API gets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] additionToLengthPtr      - (pointer to) Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *additionToLengthPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    CPSS_NULL_PTR_CHECK_MAC(additionToLengthPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* write register value */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).IPLengthOffset;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,6,additionToLengthPtr);
    }
    else
    {
        /* read register value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCpidReg1;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,26,6,additionToLengthPtr);
    }

    return rc;
}

/**
* @internal cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet function
* @endinternal
*
* @brief   This API gets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] additionToLengthPtr      - (pointer to) Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *additionToLengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, additionToLengthPtr));

    rc = internal_cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(devNum, additionToLengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, additionToLengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelPushEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS PW label push.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelPushEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelPushEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS PW label push.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelPushEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTunnelStartMplsPwLabelPushEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelPushEnableGet function
* @endinternal
*
* @brief   Returns if MPLS PW Label Push is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelPushEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelPushEnableGet function
* @endinternal
*
* @brief   Returns if MPLS PW Label Push is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelPushEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTunnelStartMplsPwLabelPushEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelSet function
* @endinternal
*
* @brief   Set the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] label                    - MPLS  (20 bits)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or label.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               label
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_TUNNEL_START_MPLS_LABEL_CHECK_MAC(label);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   label);
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelSet function
* @endinternal
*
* @brief   Set the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] label                    - MPLS  (20 bits)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or label.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               label
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, label));

    rc = internal_cpssDxChTunnelStartMplsPwLabelSet(devNum, portNum, label);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, label));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelGet function
* @endinternal
*
* @brief   Returns the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] labelPtr                 - (pointer to) MPLS label
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *labelPtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(labelPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *labelPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelGet function
* @endinternal
*
* @brief   Returns the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] labelPtr                 - (pointer to) MPLS label
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *labelPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, labelPtr));

    rc = internal_cpssDxChTunnelStartMplsPwLabelGet(devNum, portNum, labelPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, labelPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsFlowLabelEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS flow label per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsFlowLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChTunnelStartMplsFlowLabelEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS flow label per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsFlowLabelEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTunnelStartMplsFlowLabelEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsFlowLabelEnableGet function
* @endinternal
*
* @brief   Returns if MPLS flow Label is enabled or disabled per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsFlowLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsFlowLabelEnableGet function
* @endinternal
*
* @brief   Returns if MPLS flow Label is enabled or disabled per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsFlowLabelEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTunnelStartMplsFlowLabelEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsFlowLabelTtlSet function
* @endinternal
*
* @brief   Set the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ttl                      - the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsFlowLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_U8                ttl
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig1;
    return prvCpssHwPpSetRegField(devNum, regAddr, 1, 8, ttl);
}

/**
* @internal cpssDxChTunnelStartMplsFlowLabelTtlSet function
* @endinternal
*
* @brief   Set the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ttl                      - the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_U8                ttl
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsFlowLabelTtlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ttl));

    rc = internal_cpssDxChTunnelStartMplsFlowLabelTtlSet(devNum, ttl);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ttl));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsFlowLabelTtlGet function
* @endinternal
*
* @brief   Get the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ttlPtr                   - (pointer to) the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsFlowLabelTtlGet
(
    IN  GT_U8           devNum,
    OUT GT_U8           *ttlPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* field address */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ttlPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig1;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 8, &value);
    *ttlPtr = (GT_U8)value;
    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsFlowLabelTtlGet function
* @endinternal
*
* @brief   Get the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ttlPtr                   - (pointer to) the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlGet
(
    IN  GT_U8           devNum,
    OUT GT_U8           *ttlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsFlowLabelTtlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ttlPtr));

    rc = internal_cpssDxChTunnelStartMplsFlowLabelTtlGet(devNum, ttlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ttlPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelExpSet function
* @endinternal
*
* @brief   Set the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] exp                      - EXP value (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or exp.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelExpSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               exp
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_TUNNEL_START_MPLS_EXP_CHECK_MAC(exp);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   exp);
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelExpSet function
* @endinternal
*
* @brief   Set the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] exp                      - EXP value (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or exp.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               exp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelExpSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, exp));

    rc = internal_cpssDxChTunnelStartMplsPwLabelExpSet(devNum, portNum, exp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, exp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelExpGet function
* @endinternal
*
* @brief   Returns the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] expPtr                   - (pointer to) EXP value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelExpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *expPtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(expPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *expPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelExpGet function
* @endinternal
*
* @brief   Returns the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] expPtr                   - (pointer to) EXP value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *expPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelExpGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, expPtr));

    rc = internal_cpssDxChTunnelStartMplsPwLabelExpGet(devNum, portNum, expPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, expPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelTtlSet function
* @endinternal
*
* @brief   Set the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ttl                      - TTL value (APPLICABLE RANGES: 0...255)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or ttl.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               ttl
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(ttl > TTL_MAX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   ttl);
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelTtlSet function
* @endinternal
*
* @brief   Set the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ttl                      - TTL value (APPLICABLE RANGES: 0...255)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or ttl.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               ttl
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelTtlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ttl));

    rc = internal_cpssDxChTunnelStartMplsPwLabelTtlSet(devNum, portNum, ttl);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ttl));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwLabelTtlGet function
* @endinternal
*
* @brief   Returns the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ttlPtr                   - (pointer to) TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwLabelTtlGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *ttlPtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(ttlPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *ttlPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPwLabelTtlGet function
* @endinternal
*
* @brief   Returns the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ttlPtr                   - (pointer to) TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *ttlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwLabelTtlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ttlPtr));

    rc = internal_cpssDxChTunnelStartMplsPwLabelTtlGet(devNum, portNum, ttlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ttlPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the source ePort of the packet is
*         pushed onto the packet as the inner-most label. This control is accessed
*         with the target ePort.
*         The Label to push is determined by:
*         <Source based MPLS Label>,
*         <Source based MPLS Label EXP> and
*         <Source based MPLS Label TTL> that are accessed with the source ePort.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the source ePort of the packet is
*         pushed onto the packet as the inner-most label. This control is accessed
*         with the target ePort.
*         The Label to push is determined by:
*         <Source based MPLS Label>,
*         <Source based MPLS Label EXP> and
*         <Source based MPLS Label TTL> that are accessed with the source ePort.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push source based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push source based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the packet's eVLAN is pushed onto the
*         packet after the 'Source based Label' (if exists)
*         This control is accessed with the target ePort.
*         The Label to push is determined by:
*         <Service-ID> that is accessed with the eVLAN and
*         the global configuration <eVLAN Based MPLS Label TTL>.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the packet's eVLAN is pushed onto the
*         packet after the 'Source based Label' (if exists)
*         This control is accessed with the target ePort.
*         The Label to push is determined by:
*         <Service-ID> that is accessed with the eVLAN and
*         the global configuration <eVLAN Based MPLS Label TTL>.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push eVLAN based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push eVLAN based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwControlWordSet function
* @endinternal
*
* @brief   Sets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
* @param[in] value                    - the PW word to write
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          value
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum)== GT_TRUE)
    {

        if(index > 6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* write value to hardware register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.mplsControlWord[0];

        /* adding the offset of the register                            */
        /* 0x1C000764 + n*4: where n (0-6) represents word for xCat3    */
        regAddr += index * 4;

        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 32, value);
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (index >= 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PWCtrlWord[index];

    return prvCpssHwPpWriteRegister(devNum, regAddr, value);
}

/**
* @internal cpssDxChTunnelStartMplsPwControlWordSet function
* @endinternal
*
* @brief   Sets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
* @param[in] value                    - the PW word to write
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwControlWordSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, value));

    rc = internal_cpssDxChTunnelStartMplsPwControlWordSet(devNum, index, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwControlWordGet function
* @endinternal
*
* @brief   Gets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
*
* @param[out] valuePtr                 - (pointer to) the read PW word
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *valuePtr
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum)== GT_TRUE)
    {
        if(index > 6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* read value from hardware register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.mplsControlWord[0];

        /* adding the offset of the register                            */
        /* 0x1C000764 + n*4: where n (0-6) represents word for xCat3    */
        regAddr += index * 4;

        return prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, valuePtr);

    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (index >= 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PWCtrlWord[index];

    return prvCpssHwPpReadRegister(devNum, regAddr, valuePtr);
}

/**
* @internal cpssDxChTunnelStartMplsPwControlWordGet function
* @endinternal
*
* @brief   Gets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
*
* @param[out] valuePtr                 - (pointer to) the read PW word
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwControlWordGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, valuePtr));

    rc = internal_cpssDxChTunnelStartMplsPwControlWordGet(devNum, index, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwETreeEnableSet function
* @endinternal
*
* @brief   Enable/Disable E-TREE assignment for a specified PW Control Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
* @param[in] enable                   - enable/disable the E-TREE assignment-TREE assignment
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwETreeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_BOOL         enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    offset;            /* the offset of the field */
    GT_U32    value;             /* the value to write */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (index >= 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = index+1;
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PWETreeEnable;
    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, offset , 1, value);
}

/**
* @internal cpssDxChTunnelStartMplsPwETreeEnableSet function
* @endinternal
*
* @brief   Enable/Disable E-TREE assignment for a specified PW Control Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
* @param[in] enable                   - enable/disable the E-TREE assignment-TREE assignment
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwETreeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enable));

    rc = internal_cpssDxChTunnelStartMplsPwETreeEnableSet(devNum, index, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartMplsPwETreeEnableGet function
* @endinternal
*
* @brief   Return whether E-TREE assignment is enabled for a specified PW Control
*         Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
*
* @param[out] enablePtr                - (pointer to) the E-TREE assignment enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartMplsPwETreeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    offset;            /* the offset of the field */
    GT_U32    value;             /* the read value */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (index >= 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = index+1;
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PWETreeEnable;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTunnelStartMplsPwETreeEnableGet function
* @endinternal
*
* @brief   Return whether E-TREE assignment is enabled for a specified PW Control
*         Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
*
* @param[out] enablePtr                - (pointer to) the E-TREE assignment enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartMplsPwETreeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enablePtr));

    rc = internal_cpssDxChTunnelStartMplsPwETreeEnableGet(devNum, index, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartHeaderTpidSelectSet function
* @endinternal
*
* @brief   Function sets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
static GT_STATUS internal_cpssDxChTunnelStartHeaderTpidSelectSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  GT_U32               tpidEntryIndex
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (tpidEntryIndex >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E, /* field name */
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           tpidEntryIndex);
}

/**
* @internal cpssDxChTunnelStartHeaderTpidSelectSet function
* @endinternal
*
* @brief   Function sets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  GT_U32               tpidEntryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartHeaderTpidSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tpidEntryIndex));

    rc = internal_cpssDxChTunnelStartHeaderTpidSelectSet(devNum, portNum, tpidEntryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tpidEntryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartHeaderTpidSelectGet function
* @endinternal
*
* @brief   Function gets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
static GT_STATUS internal_cpssDxChTunnelStartHeaderTpidSelectGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT GT_U32               *tpidEntryIndexPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(tpidEntryIndexPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);


    rc = prvCpssDxChReadTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                               portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E, /* field name */
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               &hwValue);
    if(rc != GT_OK)
        return rc;

    *tpidEntryIndexPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChTunnelStartHeaderTpidSelectGet function
* @endinternal
*
* @brief   Function gets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT GT_U32               *tpidEntryIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartHeaderTpidSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tpidEntryIndexPtr));

    rc = internal_cpssDxChTunnelStartHeaderTpidSelectGet(devNum, portNum, tpidEntryIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tpidEntryIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartEntryExtensionSet function
* @endinternal
*
* @brief   Set tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tsExtension              - tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartEntryExtensionSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               tsExtension
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   tsExtension);
}

/**
* @internal cpssDxChTunnelStartEntryExtensionSet function
* @endinternal
*
* @brief   Set tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tsExtension              - tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEntryExtensionSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               tsExtension
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEntryExtensionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsExtension));

    rc = internal_cpssDxChTunnelStartEntryExtensionSet(devNum, portNum, tsExtension);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsExtension));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartEntryExtensionGet function
* @endinternal
*
* @brief   Get tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tsExtensionPtr           - (pointer to) tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartEntryExtensionGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tsExtensionPtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(tsExtensionPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *tsExtensionPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChTunnelStartEntryExtensionGet function
* @endinternal
*
* @brief   Get tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tsExtensionPtr           - (pointer to) tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEntryExtensionGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tsExtensionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEntryExtensionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsExtensionPtr));

    rc = internal_cpssDxChTunnelStartEntryExtensionGet(devNum, portNum, tsExtensionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsExtensionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartEgessVlanTableServiceIdSet function
* @endinternal
*
* @brief   Set egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
)
{
    CPSS_DXCH_TABLE_ENT     tableType = CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E;
    GT_U32                      numOfBits = 24;/*number of bit to write*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    PRV_TUNNEL_START_VLAN_SERVICE_ID_CHECK_MAC(vlanServiceId);
    return prvCpssDxChWriteTableEntryField(devNum,
                                           tableType,
                                           vlanId,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,   /* word num */
                                           12,   /* start bit */
                                           numOfBits,
                                           vlanServiceId);
}

/**
* @internal cpssDxChTunnelStartEgessVlanTableServiceIdSet function
* @endinternal
*
* @brief   Set egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEgessVlanTableServiceIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, vlanServiceId));

    rc = internal_cpssDxChTunnelStartEgessVlanTableServiceIdSet(devNum, vlanId, vlanServiceId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, vlanServiceId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTunnelStartEgessVlanTableServiceIdGet function
* @endinternal
*
* @brief   Get egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
)
{
    CPSS_DXCH_TABLE_ENT     tableType = CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E;
    GT_U32                      numOfBits = 24;/*number of bit to write*/
    GT_STATUS                   rc;
    GT_U32                      hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    CPSS_NULL_PTR_CHECK_MAC(vlanServiceIdPtr);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        tableType,
                                        vlanId,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,   /* word num*/
                                        12,   /* start bit */
                                        numOfBits,
                                        &hwValue);

    *vlanServiceIdPtr = hwValue & 0xFFFFFF;

    return rc;
}

/**
* @internal cpssDxChTunnelStartEgessVlanTableServiceIdGet function
* @endinternal
*
* @brief   Get egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEgessVlanTableServiceIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, vlanServiceIdPtr));

    rc = internal_cpssDxChTunnelStartEgessVlanTableServiceIdGet(devNum, vlanId, vlanServiceIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, vlanServiceIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartEcnModeSet function
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
static GT_STATUS internal_cpssDxChTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT  mode
)
{
    GT_STATUS   rc;                 /* return value */
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    switch(mode)
    {
        case CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E:
            value = 0;
            break;
        case CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E:
            value = 1;
            break;
        default :
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* write value to register */
    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKGlobalConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 15, 1, value);
    return rc;
}

/**
* @internal cpssDxChTunnelStartEcnModeSet function
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
GT_STATUS cpssDxChTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEcnModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChTunnelStartEcnModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTunnelStartEcnModeGet function
* @endinternal
*
* @brief   Get Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] modePtr              - (pointer to) ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTunnelStartEcnModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;                 /* return value */
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* read value from register */
    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKGlobalConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 15, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (value == 0) ?
            CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E :
            CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E;

    return rc;
}

/**
* @internal cpssDxChTunnelStartEcnModeGet function
* @endinternal
*
* @brief   Get Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] modePtr              - (pointer to) ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEcnModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTunnelStartEcnModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChTunnelStartEcnModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
