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
* @file cpssDxChTti.c
*
* @brief CPSS tunnel termination implementation.
*
* @version   158
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiLog.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/generic/log/prvCpssLog.h>

/* the size of tti macToMe table entry in words */
#define TTI_MAC2ME_SIZE_CNS             6   /* in Bobcat2; Caelum; Bobcat3; Aldrin2 the 'entry' is 6 words */

/* the size of tti action type 2 in words */
#define TTI_ACTION_TYPE_2_SIZE_CNS      8

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

/* maximum value for QOS profile field */
#define QOS_PROFILE_MAX_CNS             127

/* maximum value for QOS profile field for E-arch devices */
#define QOS_PROFILE_E_ARCH_MAX_CNS       1023

/* maximum value for VRF ID field */
#define VRF_ID_MAX_CNS                  4095

/* maximum value for IPCL index */
#define IPCL_INDEX_MAX_CNS              8191

/* maximum value for central counter index */
#define CENTRAL_COUNTER_MAX_CNS         16383

/* maximum value for DP field */
#define DP_MAX_CNS                      1

/* maximum value for MIM ISID field */
#define MIM_ISID_MAX_CNS                16777215

/* maximum value for MIM iRES field */
#define MIM_IRES_MAX_CNS                3

/* maximum value for time to live (TTL) field */
#define TTL_MAX_CNS                     255

/* maximum value for Adjacency index */
#define ADJACENCY_MAX_CNS              127

/* minimum value for cpu code base field */
#define CPU_CODE_BASE_MIN_CNS                192

/* maximum value for PWE3 cpu code base field */
#define PRV_CPSS_DXCH_TTI_PWE3_CPU_CODE_BASE_MAX_CNS    251

/* maximum value for TRILL cpu code base field */
#define PRV_CPSS_DXCH_TTI_TRILL_CPU_CODE_BASE_MAX_CNS   244

/* maximum value for qos mapping table index */
#define QOS_MAPPING_TABLE_INDEX_MAX_CNS     12

/* HW values for offset types in UDBs */
#define UDB_HW_L2_CNS                               0
#define UDB_HW_MPLS_CNS                             1
#define UDB_HW_L3_CNS                               2
#define UDB_HW_L4_CNS                               3
#define UDB_HW_METADATA_CNS                         6

/* maximum value for udb index field */
#define UDB_CONFIG_NUM_WORDS_MAX_CNS   30

#define KEY_UDB_IPV4_TCP_OFFSET_IN_ARRAY_CNS        0
#define KEY_UDB_IPV4_UDP_OFFSET_IN_ARRAY_CNS        1
#define KEY_UDB_MPLS_OFFSET_IN_ARRAY_CNS            2
#define KEY_UDB_IPV4_FRAGMENT_OFFSET_IN_ARRAY_CNS   3
#define KEY_UDB_IPV4_OTHER_OFFSET_IN_ARRAY_CNS      4
#define KEY_UDB_ETHERNET_OTHER_OFFSET_IN_ARRAY_CNS  5
#define KEY_UDB_IPV6_OFFSET_IN_ARRAY_CNS            6
#define KEY_UDB_IPV6_TCP_OFFSET_IN_ARRAY_CNS        7
#define KEY_UDB_IPV6_UDP_OFFSET_IN_ARRAY_CNS        8
#define KEY_UDB_UDE_OFFSET_IN_ARRAY_CNS             9
#define KEY_UDB_UDE1_OFFSET_IN_ARRAY_CNS            10
#define KEY_UDB_UDE2_OFFSET_IN_ARRAY_CNS            11
#define KEY_UDB_UDE3_OFFSET_IN_ARRAY_CNS            12
#define KEY_UDB_UDE4_OFFSET_IN_ARRAY_CNS            13
#define KEY_UDB_UDE5_OFFSET_IN_ARRAY_CNS            14
#define KEY_UDB_UDE6_OFFSET_IN_ARRAY_CNS            15

/**
* @struct TTI_ACTION_HW_VALUES_STC
 *
 * @brief A structure to tti action info in HW format between function that
 * do 'SW format check' + 'convert to HW value' to function that use
 * HW values to build the actual full HW entry.
*/
typedef struct{

    GT_U32 ttPassengerPktType;

    GT_U32 tsPassengerPacketType;

    GT_U32 mplsCommand;

    GT_U32 pktCommand;

    GT_U32 redirectCommand;

    GT_U32 userDefinedCpuCode;

    GT_U32 tag0VlanPrecedence;

    GT_U32 qosPrecedence;

    GT_U32 modifyDSCP;

    GT_U32 modifyTag0Up;

    GT_U32 tag1UpCommand;

    GT_U32 tag0VlanCmd;

    GT_U32 tag1VlanCmd;

    GT_U32 pcl0OverrideConfigIndex;

    GT_U32 pcl0_1OverrideConfigIndex;

    GT_U32 pcl1OverrideConfigIndex;

    GT_U32 routerLttPtr;

    GT_U32 tempAnalyzerIndex;

    GT_HW_DEV_NUM hwDev;

    GT_PORT_NUM hwPort;

} TTI_ACTION_HW_VALUES_STC;

/* macro to set value to field of tti ACTION entry format in buffer */
#define TTI_ACTION_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

/* macro to Get value to field of tti ACTION entry format in buffer */
#define TTI_ACTION_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)





/* Converts rule type to rule size value */
#define PRV_CPSS_DXCH_TTI_CONVERT_RULE_TYPE_TO_TCAM_RULE_SIZE_VAL_MAC(_ruleSize, _ruleType)     \
    switch (_ruleType)                                                                      \
    {                                                                                       \
        case CPSS_DXCH_TTI_RULE_UDB_10_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                    \
            break;                                                                          \
        case CPSS_DXCH_TTI_RULE_UDB_20_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                    \
            break;                                                                          \
        case CPSS_DXCH_TTI_RULE_UDB_30_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                    \
            break;                                                                          \
        default:                                                                            \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                    \
            break;                                                                          \
    }

typedef enum {
    TTI_META_DATA_FIELDS_IS_ARP_E,
    TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E,
    TTI_META_DATA_FIELDS_RESERVED_0_E,
    TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,
    TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,
    TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,
    TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,
    TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,
    TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,
    TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,
    TTI_META_DATA_FIELDS_UP0_B_UP_E,
    TTI_META_DATA_FIELDS_CFI0_B_DEI__E,
    TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,
    TTI_META_DATA_FIELDS_RESERVED_1_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,
    TTI_META_DATA_FIELDS_VLAN1_7_0_E,
    TTI_META_DATA_FIELDS_VLAN1_11_8_E,
    TTI_META_DATA_FIELDS_UP1_E,
    TTI_META_DATA_FIELDS_CFI1_E,
    TTI_META_DATA_FIELDS_VLAN1_EXIST_E,
    TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,
    TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,
    TTI_META_DATA_FIELDS_RESERVED_2_E,
    TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,
    TTI_META_DATA_FIELDS_RESERVED_3_E,
    TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,
    TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,
    TTI_META_DATA_FIELDS_NUMBER_OF_MPLS_LABELS_14_12_E,
    TTI_META_DATA_FIELDS_RESERVED_4_E,
    TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,
    TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,
    TTI_META_DATA_FIELDS_TRILL_M_BIT_E,
    TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,
    TTI_META_DATA_FIELDS_REP_LAST_E,
    TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,
    TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,
    TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,
    TTI_META_DATA_FIELDS_PCLID_4_0_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,
    TTI_META_DATA_FIELDS_MAC_TO_ME_E,
    TTI_META_DATA_FIELDS_PCLID_9_5_E,
    TTI_META_DATA_FIELDS_SRC_COREID_E,
    TTI_META_DATA_FIELDS_EVLAN_7_0_E,
    TTI_META_DATA_FIELDS_EVLAN_12_8_E,
    TTI_META_DATA_FIELDS_RESERVED_5_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,
    TTI_META_DATA_FIELDS_RESERVED_6_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,
    TTI_META_DATA_FIELDS_RESERVED_7_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,
    TTI_META_DATA_FIELDS_RESERVED_8_E,
    TTI_META_DATA_FIELDS_MUST_BE_1_E,
    TTI_META_DATA_FIELDS_SIP5_LAST_VALUE_E,
    /* SIP6 META_DATA_FIELDS */
    TTI_META_DATA_FIELDS_RESERVED_9_E,
    TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,
    TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E,
    TTI_META_DATA_FIELDS_RESERVED_10_E,
    TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E,

    TTI_META_DATA_FIELDS___LAST_VALUE___E/* used for array size */
}TTI_META_DATA_FIELDS_ENT;

/* number of bytes that metadata hold */
#define NUM_OF_META_DATA_BYTES_MAC(_devNum) \
    (PRV_CPSS_SIP_6_CHECK_MAC(_devNum))?                                                            \
        (((ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS___LAST_VALUE___E - 1].startBit +            \
        ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS___LAST_VALUE___E - 1].numOfBits) + 7) / 8)     \
        :(((ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS_SIP5_LAST_VALUE_E - 1].startBit +          \
        ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS_SIP5_LAST_VALUE_E - 1].numOfBits) + 7) / 8)

/* maximum number of metadata bytes */
#define NUM_OF_MAX_META_DATA_BYTES_MAC(_devNum) \
    (PRV_CPSS_SIP_6_CHECK_MAC(_devNum)) ? 37:32

static PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS___LAST_VALUE___E] =
{ /* byte 0 */
    /*TTI_META_DATA_FIELDS_IS_ARP_E,                                    */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E,                   */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_RESERVED_0_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
/* byte 1..4 */
    /*TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,                              */
    PRV_CPSS_STANDARD_FIELD_MAC(32 ),
/* byte 5..8 */
    /*TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,                              */
    PRV_CPSS_STANDARD_FIELD_MAC(32 ),
/* byte 9 */
    /*TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,                       */
    PRV_CPSS_STANDARD_FIELD_MAC(2  ),
    /*TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,                     */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,                    */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
/* byte 10 */
    /*TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,                      */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,         */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
/* byte 11 */
    /*TTI_META_DATA_FIELDS_UP0_B_UP_E,                                  */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_CFI0_B_DEI__E,                               */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,                          */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_RESERVED_1_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(2  ),
    /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,                       */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 12 */
    /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,               */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 13 */
    /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,              */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,                    */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 14 */
    /*TTI_META_DATA_FIELDS_VLAN1_7_0_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 15 */
    /*TTI_META_DATA_FIELDS_VLAN1_11_8_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_UP1_E,                                       */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_CFI1_E,                                      */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 16 */
    /*TTI_META_DATA_FIELDS_VLAN1_EXIST_E,                               */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,                           */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,                           */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_RESERVED_2_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 17 */
    /*TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,                           */
    PRV_CPSS_STANDARD_FIELD_MAC(7  ),
    /*TTI_META_DATA_FIELDS_RESERVED_3_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 18 */
    /*TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 19 */
    /*TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,                       */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_NUMBER_OF_MPLS_LABELS_14_12_E,               */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_RESERVED_4_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 20 */
    /*TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,                  */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,                       */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_TRILL_M_BIT_E,                               */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,              */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_REP_LAST_E,                                  */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 21 */
    /*TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,                   */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,                */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,                             */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 22 */
    /*TTI_META_DATA_FIELDS_PCLID_4_0_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,              */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_MAC_TO_ME_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 23 */
    /*TTI_META_DATA_FIELDS_PCLID_9_5_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_META_DATA_FIELDS_SRC_COREID_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 24 */
    /*TTI_META_DATA_FIELDS_EVLAN_7_0_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 25 */
    /*TTI_META_DATA_FIELDS_EVLAN_12_8_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_META_DATA_FIELDS_RESERVED_5_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 26 */
    /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,     */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 27 */
    /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,    */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_META_DATA_FIELDS_RESERVED_6_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 28 */
    /*TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,      */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 29 */
    /*TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,     */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_META_DATA_FIELDS_RESERVED_7_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 30 */
    /*TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 31 */
    /*TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_META_DATA_FIELDS_RESERVED_8_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_META_DATA_FIELDS_MUST_BE_1_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_SIP5_LAST_VALUE_E*/
    PRV_CPSS_STANDARD_FIELD_MAC(0  ),
/* SIP6 META_DATA_FIELDS */
/* byte 32..33 */
    /*TTI_META_DATA_FIELDS_RESERVED_9_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(16  ),
/* byte 34..35*/
    /*TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,                          */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
    /*TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E*/
    PRV_CPSS_STANDARD_FIELD_MAC(2  ),
    /*TTI_META_DATA_FIELDS_RESERVED_10_E*/
    PRV_CPSS_STANDARD_FIELD_MAC(6  ),
/* byte 36 */
    /*TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E,                             */
    PRV_CPSS_STANDARD_FIELD_MAC(8  )
};
/* macro used for array and it's size as function parameters */
#define PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(arrayName)   \
    arrayName , sizeof(arrayName) / sizeof(arrayName[0])

/* fields from metadata that located at bytes 22..29 */
static TTI_META_DATA_FIELDS_ENT  legacyKey_common_metadata22_29_fieldsArr[]={
    TTI_META_DATA_FIELDS_PCLID_4_0_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,
    TTI_META_DATA_FIELDS_MAC_TO_ME_E,
    TTI_META_DATA_FIELDS_PCLID_9_5_E,
    TTI_META_DATA_FIELDS_SRC_COREID_E,
    TTI_META_DATA_FIELDS_EVLAN_7_0_E,
    TTI_META_DATA_FIELDS_EVLAN_12_8_E,
    TTI_META_DATA_FIELDS_RESERVED_5_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,
    TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,
    TTI_META_DATA_FIELDS_RESERVED_6_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E
};

/* fields from metadata that located at bytes 11 */
static TTI_META_DATA_FIELDS_ENT  legacyKey_common_metadata11_fieldsArr[]=
{
    TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E
};

/* fields from metadata that located at bytes 30..31 */
static TTI_META_DATA_FIELDS_ENT  legacyKey_common_metadata30_31_fieldsArr[]={
    TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,
    TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E
};

/* fields from metadata that located at bytes 30..31 */
static TTI_META_DATA_FIELDS_ENT  legacyKey_ip_metadata0_fieldsArr[]={
    TTI_META_DATA_FIELDS_IS_ARP_E,
    TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_ipv4_sip_metadata_fieldsArr[]={
    TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_ipv4_dip_metadata_fieldsArr[]={
    TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E
};


static TTI_META_DATA_FIELDS_ENT  legacyKey_mpls_metadata10_fieldsArr[]={
    TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,
    TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_mpls_metadata9_fieldsArr[]={
    TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,
    TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,
    TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_ethernet_metadata14_15_fieldsArr[]={
    TTI_META_DATA_FIELDS_VLAN1_7_0_E,
    TTI_META_DATA_FIELDS_VLAN1_11_8_E,
    TTI_META_DATA_FIELDS_UP1_E,
    TTI_META_DATA_FIELDS_CFI1_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_ethernet_metadata16_19_fieldsArr[]={
    TTI_META_DATA_FIELDS_VLAN1_EXIST_E,
    TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,
    TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,
    TTI_META_DATA_FIELDS_RESERVED_2_E,
    TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,
    TTI_META_DATA_FIELDS_RESERVED_3_E,
    TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,
    TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,
    TTI_META_DATA_FIELDS_RESERVED_4_E
};

static TTI_META_DATA_FIELDS_ENT  legacyKey_mim_metadata12_13_fieldsArr[]={
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,
    TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E
};




/* the index (byte index) in the legacy key of Bobcat2 and above that should represent the mac SA/DA */
static GT_U32   sip5legacyMacSaDaIndexInKey = 0;

typedef enum {
    TTI_BYTES_FROM_PACKET_L2_MAC_DA_E,        /*6 bytes in network order*/  /*L2 offset 0..5 */
    TTI_BYTES_FROM_PACKET_L2_MAC_SA_E,        /*6 bytes in network order*/  /*L2 offset 6..11 */

    TTI_BYTES_FROM_PACKET_L3_ETHERTYPE_E,     /*16 bits */ /*L3 offset 0..1 */

    TTI_BYTES_FROM_PACKET_L3_IP_PROTOCOL_E,   /*8 bits */  /*L3 offset 1 */

    TTI_BYTES_FROM_PACKET_L3_IPV4_SIP_E,      /*4 bytes in network order*/ /*L3 offset 14..17 */
    TTI_BYTES_FROM_PACKET_L3_IPV4_DIP_E,      /*4 bytes in network order*/ /*L3 offset 18..21 */

    TTI_BYTES_FROM_PACKET_MPLS_TAG_1_LABEL_E,   /*20 bits */  /*MPLS offset 2..4  from bit 4 in byte4 towards byte 2 */
    TTI_BYTES_FROM_PACKET_MPLS_TAG_1_EXP_E,     /* 3 bits */  /*MPLS offset 4 bit 1*/
    TTI_BYTES_FROM_PACKET_MPLS_TAG_1_S_BIT_E,   /* 1 bit  */  /*MPLS offset 4 bit 0*/

    TTI_BYTES_FROM_PACKET_MPLS_TAG_2_LABEL_E,   /*20 bits */  /*MPLS offset 6..8  from bit 4 in byte8 towards byte 6 */
    TTI_BYTES_FROM_PACKET_MPLS_TAG_2_EXP_E,     /* 3 bits */  /*MPLS offset 8 bit 1*/
    TTI_BYTES_FROM_PACKET_MPLS_TAG_2_S_BIT_E,   /* 1 bit  */  /*MPLS offset 8 bit 0*/

    TTI_BYTES_FROM_PACKET_MPLS_TAG_3_LABEL_E,   /*20 bits */  /*MPLS offset 10..12 from bit 4 in byte12 towards byte 10 */
    TTI_BYTES_FROM_PACKET_MPLS_TAG_3_EXP_E,     /* 3 bits */  /*MPLS offset 12 bit 1*/
    TTI_BYTES_FROM_PACKET_MPLS_TAG_3_S_BIT_E,   /* 1 bit  */  /*MPLS offset 12 bit 0*/

    TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_PRI_E,   /* 3 bits */  /*L3 offset 2 bit 5 */
    TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_DEI_E,   /* 1 bit  */  /*L3 offset 2 bit 4 */
    TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_NO_CUSTOMER_ADDRESS_INDICATION_E,   /* 4 bits */ /*L3 offset 2 bit 0 */

    TTI_BYTES_FROM_PACKET_L3_MIM_I_SID_E,          /*24 bits */

    TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_0_E,     /*32 bits */  /*L4 offset 0..3 */
    TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_1_E,     /*32 bits */  /*L4 offset 4..7 */
    TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_2_E,     /*32 bits */  /*L4 offset 8..11 */

    TTI_BYTES_FROM_PACKET___LAST_VALUE___E/* used for array size */
}TTI_BYTES_FROM_PACKET_ENT;

typedef enum{
    FLAG_BYTES_ARRAY_E     ,
    FLAG_VALUE_E           ,
    FLAG_VALUE_REVERED_E
}PACKET_PART_INFO_ENT;

typedef struct{
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT   partType;
    GT_U32                          startByteOffset; /*when FLAG_VALUE_REVERED_E indicate 'end byte' */
    GT_U32                          numOfBits;
    GT_U32                          startBitInByte;  /*when FLAG_VALUE_REVERED_E indicate 'end bit' */
    PACKET_PART_INFO_ENT            flag;
}PACKET_PART_INFO_STC;

#define BYTES_TO_BITS_MAC(numBytes) (numBytes * 8)

/* convert labels to start offset from 'mpls-2' : 2,6,10 */
#define MPLS_LABEL_OFFSET_MAC(label)    (2 + (label * 4) )

static PACKET_PART_INFO_STC ttiWellKnownPacketPartsArr[TTI_BYTES_FROM_PACKET___LAST_VALUE___E] =
{
    /*TTI_BYTES_FROM_PACKET_L2_MAC_DA_E,        *//*6 bytes in network order  L2 offset 0..5  */
    {CPSS_DXCH_TTI_OFFSET_L2_E , 0 , BYTES_TO_BITS_MAC(6) , 0 , FLAG_BYTES_ARRAY_E},
    /*TTI_BYTES_FROM_PACKET_L2_MAC_SA_E,        *//*6 bytes in network order  L2 offset 6..11 */
    {CPSS_DXCH_TTI_OFFSET_L2_E , 6 , BYTES_TO_BITS_MAC(6) , 0 , FLAG_BYTES_ARRAY_E},

    /*TTI_BYTES_FROM_PACKET_L3_ETHERTYPE_E,     *//*16 bits  L3 offset 0..1                   */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 0 , 16 , 7/*end at bit 7 byte 0*/ , FLAG_VALUE_REVERED_E},

    /*TTI_BYTES_FROM_PACKET_L3_IP_PROTOCOL_E,   *//*8 bits   L3 offset 1                      */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 1 , 8 , 0 , FLAG_VALUE_E},

    /*TTI_BYTES_FROM_PACKET_L3_IPV4_SIP_E,      *//*4 bytes in network order L3 offset 14..17                  */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 14 , BYTES_TO_BITS_MAC(4) , 0 , FLAG_BYTES_ARRAY_E},
    /*TTI_BYTES_FROM_PACKET_L3_IPV4_DIP_E,      *//*4 bytes in network order L3 offset 18..21                  */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 18 , BYTES_TO_BITS_MAC(4) , 0 , FLAG_BYTES_ARRAY_E},

    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_1_LABEL_E, *//*  20 bits   MPLS offset 2..4              */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(0) , 20 , 7/*end at bit 7 byte 2*/ , FLAG_VALUE_REVERED_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_1_EXP_E,   *//*   3 bits   MPLS offset 4 bit 1           */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(0)+2 , 3 , 4 , FLAG_VALUE_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_1_S_BIT_E, *//*   1 bit    MPLS offset 4 bit 0           */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(0)+2 , 1 , 7 , FLAG_VALUE_E},

    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_2_LABEL_E, *//*  20 bits   MPLS offset 6..8              */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(1) , 20 , 7/*end at bit 7 byte 6*/ , FLAG_VALUE_REVERED_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_2_EXP_E,   *//*   3 bits   MPLS offset 8 bit 1           */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(1)+2 , 3 , 4 , FLAG_VALUE_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_2_S_BIT_E, *//*   1 bit    MPLS offset 8 bit 0           */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(1)+2 , 1 , 7 , FLAG_VALUE_E},

    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_3_LABEL_E, *//*  20 bits   MPLS offset 10..12            */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(2) , 20 , 7/*end at bit 7 byte 10*/ , FLAG_VALUE_REVERED_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_3_EXP_E,   *//*   3 bits   MPLS offset 12 bit 1          */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(2)+2 , 3 , 4 , FLAG_VALUE_E},
    /*TTI_BYTES_FROM_PACKET_MPLS_TAG_3_S_BIT_E, *//*   1 bit    MPLS offset 12 bit 0          */
    {CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E , MPLS_LABEL_OFFSET_MAC(2)+2 , 1 , 7 , FLAG_VALUE_E},

    /*TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_PRI_E,    *//*3 bits   L3 offset 2 bit 5         */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 2 , 3 , 5 , FLAG_VALUE_E},
    /*TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_DEI_E,    *//*1 bit    L3 offset 2 bit 4         */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 2 , 1 , 4 , FLAG_VALUE_E},
    /*TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_NO_CUSTOMER_ADDRESS_INDICATION_E,    *//*4 bits  L3 offset 2 bit 0*/
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 2 , 4 , 0 , FLAG_VALUE_E},

    /*TTI_BYTES_FROM_PACKET_L3_MIM_I_SID_E,          *//*24 bits  L3 offset 3..5 */
    {CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E , 3 , 24 , 7/*end at bit 7 byte 3*/ , FLAG_VALUE_REVERED_E},

    /*TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_0_E,     *//*32 bits   L4 offset 0..3             */
    {CPSS_DXCH_TTI_OFFSET_L4_E , 0 , 32 , 7/*end at bit 7 byte 0*/ , FLAG_VALUE_REVERED_E},
    /*TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_1_E,     *//*32 bits   L4 offset 4..7             */
    {CPSS_DXCH_TTI_OFFSET_L4_E , 4 , 32 , 7/*end at bit 7 byte 4*/ , FLAG_VALUE_REVERED_E},
    /*TTI_BYTES_FROM_PACKET_L4_IP_DATA_WORD_2_E,     *//*32 bits   L4 offset 8..11            */
    {CPSS_DXCH_TTI_OFFSET_L4_E , 8 , 32 , 7/*end at bit 7 byte 8*/ , FLAG_VALUE_REVERED_E},

};

static GT_STATUS  wellKnownPacketPartsUsedSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType,
    IN TTI_BYTES_FROM_PACKET_ENT  field,
    IN GT_U32              startIndexInKey,
    OUT GT_U32             *numOfUdbsPtr
);

typedef enum{
    /*common to all*/
    TTI_LEGACY_KEY_FIELDS_PCLID_4_0_E,
    TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,
    TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCISTRUNK_E,
    TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E,
    TTI_LEGACY_KEY_FIELDS_PCLID_9_5_E,
    TTI_LEGACY_KEY_FIELDS_SRC_COREID_E,
    TTI_LEGACY_KEY_FIELDS_EVLAN_7_0_E,
    TTI_LEGACY_KEY_FIELDS_EVLAN_12_8_E,
    TTI_LEGACY_KEY_FIELDS_RESERVED_0_E,
    TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,
    TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,
    TTI_LEGACY_KEY_FIELDS_RESERVED_1_E,
    TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,
    TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,
    TTI_LEGACY_KEY_FIELDS_RESERVED_2_E,

    /* mim and trill and ethernet */
    TTI_LEGACY_KEY_FIELDS_UP0_E,
    TTI_LEGACY_KEY_FIELDS_CFI0_E,
    TTI_LEGACY_KEY_FIELDS_VLAN_TAG0_EXISTS_E,
    TTI_LEGACY_KEY_FIELDS_RESERVED_4_E,
    TTI_LEGACY_KEY_FIELDS_PASSENGER_OUTER_CFI_E,
    /* end mim and trill */

    TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E,

    TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_7_0_E,
    TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_9_8_E,
    TTI_LEGACY_KEY_FIELDS_RESERVED_3_E,
    TTI_LEGACY_KEY_FIELDS_MUST_BE_1_E,

    /* ipv4 */
    TTI_LEGACY_KEY_FIELDS_IPV4_SIP_E,
    TTI_LEGACY_KEY_FIELDS_IPV4_DIP_E,
    TTI_LEGACY_KEY_FIELDS_IPV4_IS_ARP_E,
    TTI_LEGACY_KEY_FIELDS_IPV4_TUNNELING_PROTOCOL_E,

    /* mpls */
    TTI_LEGACY_KEY_FIELDS_MPLS_S0_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_EXP0_E    ,
    TTI_LEGACY_KEY_FIELDS_MPLS_LABEL0_E  ,
    TTI_LEGACY_KEY_FIELDS_MPLS_S1_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_EXP1_E    ,
    TTI_LEGACY_KEY_FIELDS_MPLS_LABEL1_E  ,
    TTI_LEGACY_KEY_FIELDS_MPLS_S2_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_EXP2_E    ,
    TTI_LEGACY_KEY_FIELDS_MPLS_LABEL2_E  ,
    TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_VALUE_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_PROTOCOL_AFTER_MPLS_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_EXISTS_E      ,
    TTI_LEGACY_KEY_FIELDS_MPLS_DATA_AFTER_INNER_LABEL_E      ,

    /* mim (part2)*/
    TTI_LEGACY_KEY_FIELDS_MIM_I_SID_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_I_TAG_OCTET1_BITS_1_4_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_I_DP_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_I_UP_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_VID_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_UP_E      ,
    TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_EXISTS_E      ,

    /*ethernet*/
    TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN1_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_UP1_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_CFI1_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_ETHERTYPE_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN_TAG1_EXISTS_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG0_TPID_INDEX_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG1_TPID_INDEX_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_QOS_PROFILE_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED1_E      ,
    TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_SOURCE_ID_11_0_E      ,

    TTI_LEGACY_KEY_FIELDS___LAST_VALUE___E/* used for array size */
}TTI_LEGACY_KEY_FIELDS_ENT;

static PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiLegacyKeyFieldsFormat[TTI_LEGACY_KEY_FIELDS___LAST_VALUE___E] =
{
/* byte 0 */
    /*TTI_LEGACY_KEY_FIELDS_PCLID_4_0_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,              */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCISTRUNK_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 1 */
    /*TTI_LEGACY_KEY_FIELDS_PCLID_9_5_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_LEGACY_KEY_FIELDS_SRC_COREID_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 2 */
    /*TTI_LEGACY_KEY_FIELDS_EVLAN_7_0_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 3 */
    /*TTI_LEGACY_KEY_FIELDS_EVLAN_12_8_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_LEGACY_KEY_FIELDS_RESERVED_0_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 4 */
    /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,     */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 5 */
    /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,    */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_LEGACY_KEY_FIELDS_RESERVED_1_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
/* byte 6 */
    /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,      */
    PRV_CPSS_STANDARD_FIELD_MAC(8  ),
/* byte 7 */
    /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,     */
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),
    /*TTI_LEGACY_KEY_FIELDS_RESERVED_2_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),

    /* mim and trill and ethernet */
/* byte 8 */
    /*TTI_LEGACY_KEY_FIELDS_UP0_E,           */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_CFI0_E,        */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_VLAN_TAG0_EXISTS_E,   */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_RESERVED_4_E,         */
    PRV_CPSS_STANDARD_FIELD_MAC(2  ),
    /*TTI_LEGACY_KEY_FIELDS_PASSENGER_OUTER_CFI_E,*/
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),

/* byte 9..14 */
    /*TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E,     */
    PRV_CPSS_STANDARD_FIELD_MAC(48  ),

/* byte 28 */
    /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_7_0_E,                        */
    {(28*8),/*explicit byte 28*/
     8,
     0/*not care when startBit is explicit*/},
/* byte 29 */
    /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_9_8_E,                        */
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_LEGACY_KEY_FIELDS_RESERVED_3_E,                                */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_MUST_BE_1_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),

/* IPV4 specific fields */
/* byte 15..18 */
    /*TTI_LEGACY_KEY_FIELDS_IPV4_SIP_E,                                 */
    {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
     32,
     TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
/* byte 19..22 */
    /*TTI_LEGACY_KEY_FIELDS_IPV4_DIP_E,                                 */
    PRV_CPSS_STANDARD_FIELD_MAC(32  ),
/* byte 23 */
    /*TTI_LEGACY_KEY_FIELDS_IPV4_IS_ARP_E,                              */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_IPV4_TUNNELING_PROTOCOL_E,                  */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),

/* MPLS specific fields */
/* byte 15..17 */
    /*TTI_LEGACY_KEY_FIELDS_MPLS_S0_E      ,*/
    {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
     1,
     TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
    /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP0_E    ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL0_E  ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(20  ),

/* byte 18..20 */
    /*TTI_LEGACY_KEY_FIELDS_MPLS_S1_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP1_E    ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL1_E  ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(20  ),
/* byte 21..23 */
    /*TTI_LEGACY_KEY_FIELDS_MPLS_S2_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP2_E    ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL2_E  ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(20  ),
/* byte 24 */
    /*TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_VALUE_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
/* byte 25 */
    /*TTI_LEGACY_KEY_FIELDS_MPLS_PROTOCOL_AFTER_MPLS_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(2  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_EXISTS_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_MPLS_DATA_AFTER_INNER_LABEL_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(5  ),


/* MIM specific fields */
/* byte 15..17 */
    /*TTI_LEGACY_KEY_FIELDS_MIM_I_SID_E      ,                */
    {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
     24,
     TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
/* byte 18 */
    /*TTI_LEGACY_KEY_FIELDS_MIM_I_TAG_OCTET1_BITS_1_4_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(4  ),
    /*TTI_LEGACY_KEY_FIELDS_MIM_I_DP_E      ,                 */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_MIM_I_UP_E      ,                 */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),

/* byte 19..20*/
    /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_VID_E      ,   */
    PRV_CPSS_STANDARD_FIELD_MAC(12  ),
    /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_UP_E      ,    */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_EXISTS_E      ,*/
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),

/* ethernet specific fields */
/* byte 15..16 */
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN1_E      ,               */
    {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
     12,
     TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_UP1_E      ,                 */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_CFI1_E      ,                */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 17..18 */
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_ETHERTYPE_E      ,           */
    PRV_CPSS_STANDARD_FIELD_MAC(16  ),
/* byte 19 */
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN_TAG1_EXISTS_E      ,    */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG0_TPID_INDEX_E      ,     */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG1_TPID_INDEX_E      ,     */
    PRV_CPSS_STANDARD_FIELD_MAC(3  ),
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED_E      ,            */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 20 */
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_QOS_PROFILE_E      ,     */
    PRV_CPSS_STANDARD_FIELD_MAC(7  ),
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED1_E      ,            */
    PRV_CPSS_STANDARD_FIELD_MAC(1  ),
/* byte 21..22 */
    /*TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_SOURCE_ID_11_0_E      ,  */
    PRV_CPSS_STANDARD_FIELD_MAC(12  )
};
/* fill _hwValue with value from 'fieldName' in hwFormatArray according to format of ttiLegacyKeyFieldsFormat */
#define GET_FIELD_FROM_LEGACY_KEY_MAC(_hwValue,fieldName)  \
    U32_GET_FIELD_BY_ID_MAC(hwFormatArray,ttiLegacyKeyFieldsFormat,fieldName,_hwValue)

/* fill hwFormatArray with value of _hwValue into field 'fieldName' according to format of ttiLegacyKeyFieldsFormat */
#define SET_FIELD_TO_LEGACY_KEY_MAC(_hwValue,fieldName)  \
    U32_SET_FIELD_BY_ID_MAC(hwFormatArray,ttiLegacyKeyFieldsFormat,fieldName,_hwValue)



/**
* @internal ttiConfigCheckCommon function
* @endinternal
*
* @brief   checks rule common fields.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] commonPtr                - type of the tunnel
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
*/
static GT_STATUS ttiConfigCheckCommon
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_RULE_COMMON_STC       *commonPtr
)
{
    GT_BOOL                 srcIsTrunk      = GT_FALSE;
    GT_U32                  srcPortTrunk    = GT_FALSE;
    GT_BOOL                 isTagged        = GT_FALSE;
    GT_BOOL                 dsaSrcIsTrunk   = GT_FALSE;
    GT_U32                  dsaSrcPortTrunk = 0;
    GT_U32                  dsaSrcDevice    = 0;
    GT_U32                  sourcePortGroupId = 0;

    CPSS_NULL_PTR_CHECK_MAC(commonPtr);

    srcIsTrunk = BOOL2BIT_MAC(commonPtr->srcIsTrunk);
    srcPortTrunk = commonPtr->srcPortTrunk;
    isTagged = BOOL2BIT_MAC(commonPtr->isTagged);
    dsaSrcIsTrunk = BOOL2BIT_MAC(commonPtr->dsaSrcIsTrunk);
    dsaSrcPortTrunk = commonPtr->dsaSrcPortTrunk;
    dsaSrcDevice = commonPtr->dsaSrcDevice;
    sourcePortGroupId = commonPtr->sourcePortGroupId;

    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum,commonPtr->vid);

    if (commonPtr->pclId >= BIT_10)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (sourcePortGroupId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (srcIsTrunk)
    {
        case GT_FALSE: /* src is port */
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,srcPortTrunk);
            break;
        case GT_TRUE: /* src is trunk */
            if(srcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
    }

    if (isTagged)
    {
        switch (dsaSrcIsTrunk)
        {
            case GT_FALSE: /* src is port */
                if(dsaSrcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case GT_TRUE: /* src is trunk */
                if(dsaSrcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
        }

        if(dsaSrcDevice > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal ttiConfigSetLogic2HwFormat function
* @endinternal
*
* @brief   Write a given logic format to hw format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI rule type
* @param[in] logicFormatPtr           - points to tunnel termination configuration in
*                                      logic format
*
* @param[out] hwFormatArray            - the configuration in HW format (6 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiConfigSetLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT             *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:

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

        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],28,3,logicFormatPtr->ipv4.common.sourcePortGroupId);
        }
        else
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],29,2,logicFormatPtr->ipv4.common.sourcePortGroupId);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */

        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:

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
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],8,4,logicFormatPtr->mpls.cwFirstNibble);
        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],28,3,logicFormatPtr->mpls.common.sourcePortGroupId);
        }
        else
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],29,2,logicFormatPtr->mpls.common.sourcePortGroupId);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */

        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:

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
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],28,4,logicFormatPtr->eth.vid1);

        /* handle word 3 (bits 96-127) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,8,(logicFormatPtr->eth.vid1 >> 4));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],8,3,logicFormatPtr->eth.up1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],11,1,logicFormatPtr->eth.cfi1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],12,16,logicFormatPtr->eth.etherType);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],28,1,BOOL2BIT_MAC(logicFormatPtr->eth.macToMe));

        /* handle word 5 (bits 160-191) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],0,7,logicFormatPtr->eth.common.dsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],7,1,BOOL2BIT_MAC(logicFormatPtr->eth.common.dsaSrcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],8,5,logicFormatPtr->eth.srcId);

        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],13,7,logicFormatPtr->eth.dsaQosProfile);

        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],20,2,logicFormatPtr->eth.eTagGrp);
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],28,3,logicFormatPtr->eth.common.sourcePortGroupId);
        }
        else
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],29,2,logicFormatPtr->eth.common.sourcePortGroupId);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */

        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:

        /* handle word 0 (bits 0-31) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],0,10,logicFormatPtr->mim.common.pclId);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],10,8,logicFormatPtr->mim.common.srcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],18,1,BOOL2BIT_MAC(logicFormatPtr->mim.common.srcIsTrunk));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],19,12,logicFormatPtr->mim.common.vid);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,BOOL2BIT_MAC(logicFormatPtr->mim.common.isTagged));

        /* handle word 1 (bits 32-63) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,8,logicFormatPtr->mim.common.mac.arEther[5]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],8,8,logicFormatPtr->mim.common.mac.arEther[4]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],16,8,logicFormatPtr->mim.common.mac.arEther[3]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],24,8,logicFormatPtr->mim.common.mac.arEther[2]);

        /* handle word 2 (bits 64-95) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],0,8,logicFormatPtr->mim.common.mac.arEther[1]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],8,8,logicFormatPtr->mim.common.mac.arEther[0]);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],16,7,logicFormatPtr->mim.common.dsaSrcDevice);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],23,3,logicFormatPtr->mim.bUp);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],26,1,logicFormatPtr->mim.bDp);

        /* note bit 27 is reserved */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],28,4,(logicFormatPtr->mim.iSid & 0xf));

        /* handle word 3 (bits 96-127) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,20,(logicFormatPtr->mim.iSid >> 4));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],20,3,logicFormatPtr->mim.iUp);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],23,1,logicFormatPtr->mim.iDp);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],24,2,logicFormatPtr->mim.iRes1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],26,2,logicFormatPtr->mim.iRes2);
        /* handle word 4 (bits 128-159) */

        /* handle word 5 (bits 160-191) */
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],0,7,logicFormatPtr->mim.common.dsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],7,1,BOOL2BIT_MAC(logicFormatPtr->mim.common.dsaSrcIsTrunk));
        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],28,3,logicFormatPtr->mim.common.sourcePortGroupId);
        }
        else
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],29,2,logicFormatPtr->mim.common.sourcePortGroupId);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[5],31,1,1); /* must be 1 */

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal sip5TtiConfigSetLogic2HwUdbFormat function
* @endinternal
*
* @brief   Write a given logic format to hw format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2;
*
* @param[in] ruleType                 - TTI rule type
* @param[in] logicFormatPtr           - points to tunnel termination configuration in
*                                      logic format
* @param[in] isPattern                - this is pattern
*
* @param[out] hwFormatArray            - the configuration in HW format (8 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS sip5TtiConfigSetLogic2HwUdbFormat
(
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  GT_BOOL                             isPattern,
    IN  CPSS_DXCH_TTI_RULE_UNT             *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    /* In order to support backward compatibility for numberOfLabels*/
    /* only "full don't care" (mask 0) and  "full care" (mask 3) are supported.*/
    /* The numberOfLabels pattern and mask are translated to corresponding s-bits.*/
    /* The pattern s-bit decisions are needed to make correct s-bit masks.*/
    static GT_U32 patternS0 = 0;
    static GT_U32 patternS1 = 0;
    static GT_U32 patternS2 = 0;

    GT_U32 s0=0,s1=0,s2=0; /* stop bit for mpls label */
    GT_U32 i,length = 0;
    GT_U32 hwValue;
    GT_U32 tmpValue;
    CPSS_DXCH_TTI_RULE_COMMON_STC *commonPtr = NULL;
    CPSS_DXCH_TTI_IPV4_RULE_STC   *ipv4Ptr;
    CPSS_DXCH_TTI_MPLS_RULE_STC   *mplsPtr;
    CPSS_DXCH_TTI_ETH_RULE_STC    *ethPtr;
    CPSS_DXCH_TTI_MIM_RULE_STC    *mimPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        commonPtr = &logicFormatPtr->ipv4.common;
        ipv4Ptr = &logicFormatPtr->ipv4;

        hwValue =
            (ipv4Ptr->srcIp.arIP[3] <<  0) |
            (ipv4Ptr->srcIp.arIP[2] <<  8) |
            (ipv4Ptr->srcIp.arIP[1] << 16) |
            (ipv4Ptr->srcIp.arIP[0] << 24) ;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_SIP_E);


        hwValue =
            (ipv4Ptr->destIp.arIP[3] <<  0) |
            (ipv4Ptr->destIp.arIP[2] <<  8) |
            (ipv4Ptr->destIp.arIP[1] << 16) |
            (ipv4Ptr->destIp.arIP[0] << 24) ;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_DIP_E);

        hwValue = BOOL2BIT_MAC(ipv4Ptr->isArp);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_IS_ARP_E);

        hwValue = ipv4Ptr->tunneltype;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_TUNNELING_PROTOCOL_E);

        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        commonPtr = &logicFormatPtr->mpls.common;
        mplsPtr = &logicFormatPtr->mpls;

        if (isPattern)
        {
            switch (logicFormatPtr->mpls.numOfLabels & 0x3)
            {
                case 0:
                    s0 = 1;
                    patternS0 = s0;
                    break;
                case 1:
                    s1 = 1;
                    patternS1 = s1;
                    break;
                case 2:
                    s2 = 1;
                    patternS2 = s2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    break;
            }
        }
        else
        {
            switch (logicFormatPtr->mpls.numOfLabels & 0x3)
            {
            case 0:
                s0 = s1 = s2 = 0;
                break;
            case 3:
                if (patternS0 == 1)
                {
                    s0 = 1;
                    s1 = 0;
                    s2 = 0;
                }
                if (patternS1 == 1)
                {
                    s1 = 1;
                    /* in order to prevent match with illegal packet contained more than 1 sbit*/
                    /* s0 mask is set to check s0 bit as well                                  */
                    s0 = 1;
                    s2 = 0;
                }
                if (patternS2 == 1)
                {
                    s2 = 1;
                    /* in order to prevent match with illegal packet contained more than 1 sbit*/
                    /* s0,s1 masks are set to check s0,s1 bits as well                                  */
                    s0 = 1;
                    s1 = 1;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                break;
            }
            patternS0 = patternS1 = patternS2 = 0;
        }

        hwValue = s0;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S0_E);
        hwValue = mplsPtr->exp0;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP0_E);
        hwValue = mplsPtr->label0;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL0_E);

        hwValue = s1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S1_E);
        hwValue = mplsPtr->exp1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP1_E);
        hwValue = mplsPtr->label1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL1_E);

        hwValue = s2;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S2_E);
        hwValue = mplsPtr->exp2;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP2_E);
        hwValue = mplsPtr->label2;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL2_E);


        hwValue = mplsPtr->reservedLabelValue;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_VALUE_E);
        hwValue = mplsPtr->channelTypeProfile;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E);
        hwValue = mplsPtr->protocolAboveMPLS;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_PROTOCOL_AFTER_MPLS_E);
        hwValue = BOOL2BIT_MAC(mplsPtr->reservedLabelExist);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_EXISTS_E);
        hwValue = mplsPtr->dataAfterInnerLabel;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_DATA_AFTER_INNER_LABEL_E);


        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        commonPtr = &logicFormatPtr->eth.common;
        ethPtr = &logicFormatPtr->eth;

        hwValue = ethPtr->up0;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_UP0_E);
        hwValue = ethPtr->cfi0;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_CFI0_E);
        hwValue = BOOL2BIT_MAC(ethPtr->macToMe);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E);
        hwValue = ethPtr->vid1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN1_E);
        hwValue = ethPtr->up1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_UP1_E);
        hwValue = ethPtr->cfi1;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_CFI1_E);
        hwValue = ethPtr->etherType;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_ETHERTYPE_E);
        hwValue = BOOL2BIT_MAC(ethPtr->isVlan1Exists);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN_TAG1_EXISTS_E);
        hwValue = ethPtr->tag0TpidIndex;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG0_TPID_INDEX_E);
        hwValue = ethPtr->tag1TpidIndex;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG1_TPID_INDEX_E);

        hwValue = ethPtr->dsaQosProfile;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_QOS_PROFILE_E);
        hwValue = ethPtr->srcId;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_SOURCE_ID_11_0_E);

        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:
        commonPtr = &logicFormatPtr->mim.common;
        mimPtr = &logicFormatPtr->mim;

        hwValue = mimPtr->bUp;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_UP0_E);
        hwValue = mimPtr->bDp;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_CFI0_E);
        hwValue = BOOL2BIT_MAC(mimPtr->macToMe);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E);
        hwValue = mimPtr->passengerPacketOuterTagDei;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PASSENGER_OUTER_CFI_E);
        hwValue = mimPtr->iSid;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_SID_E);
        hwValue = (mimPtr->iRes1 & 0x3) |         /*2 bits : 0..1*/
                  ((mimPtr->iRes2 & 0x3) << 2);   /*2 bits : 2..3*/
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_TAG_OCTET1_BITS_1_4_E);
        hwValue = mimPtr->iDp;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_DP_E);
        hwValue = mimPtr->iUp;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_UP_E);
        hwValue = mimPtr->passengerPacketOuterTagVid;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_VID_E);
        hwValue = mimPtr->passengerPacketOuterTagUp;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_UP_E);
        hwValue = BOOL2BIT_MAC(mimPtr->passengerPacketOuterTagExists);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_EXISTS_E);


        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
        length = 10;

        break;

    case CPSS_DXCH_TTI_RULE_UDB_20_E:
        length = 20;

        break;

    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        length = 30;

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(commonPtr)/*legacy keys only */
    {
        /* convert the common part */

        /* the PCL-ID is split field */
        tmpValue = commonPtr->pclId;
        hwValue = tmpValue & 0x1f;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PCLID_4_0_E);
        hwValue = tmpValue >> 5;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PCLID_9_5_E);

        hwValue = BOOL2BIT_MAC(commonPtr->srcIsTrunk);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E);

        hwValue = BOOL2BIT_MAC(commonPtr->dsaSrcIsTrunk);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCISTRUNK_E);

        hwValue = BOOL2BIT_MAC(commonPtr->isTagged);
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_VLAN_TAG0_EXISTS_E);

        hwValue = commonPtr->sourcePortGroupId;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_SRC_COREID_E);

        tmpValue = commonPtr->vid;
        hwValue = tmpValue & 0xff;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_EVLAN_7_0_E);
        hwValue = tmpValue >> 8;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_EVLAN_12_8_E);

        tmpValue = commonPtr->srcPortTrunk;
        hwValue = tmpValue & 0xff;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E);
        hwValue = tmpValue >> 8;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E);

        tmpValue = commonPtr->dsaSrcPortTrunk;
        hwValue = tmpValue & 0xff;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E);
        hwValue = tmpValue >> 8;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E);

        MAC_ADDR_SET_FIELD_BY_ID_MAC(hwFormatArray,
            ttiLegacyKeyFieldsFormat,
            TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E,
            &commonPtr->mac.arEther[0]);

        tmpValue = commonPtr->dsaSrcDevice;
        hwValue = tmpValue & 0xff;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_7_0_E);
        hwValue = tmpValue >> 8;
        SET_FIELD_TO_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_9_8_E);

    }
    else
    {
        /* setting UDBs */
        for (i =0 ; i < length; i++)
        {
            U32_SET_FIELD_IN_ENTRY_MAC(hwFormatArray,(8*i),8,logicFormatPtr->udbArray[i]);
        }

        /* if rule is less then 30 bytes mask the irrelevant bytes */
        for (/* continue i */; i < 30; i++)
        {
            U32_SET_FIELD_IN_ENTRY_MAC(hwFormatArray,(8*i),8,0);
        }
    }

    return GT_OK;
}

/**
* @internal ttiConfigLogic2HwFormat function
* @endinternal
*
* @brief   Converts a given tunnel termination configuration from logic format
*         to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI rule type
* @param[in] patternLogicFormatPtr    - points to tunnel termination configuration in
*                                      pattern logic format
* @param[in] maskLogicFormatPtr       - points to tunnel termination configuration in
*                                      mask logic format
*
* @param[out] patternHwFormatArray     - the pattern configuration in HW format (6 words).
* @param[out] maskHwFormatArray        - the mask configuration in HW format (6 words).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiConfigLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT             *patternLogicFormatPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT             *maskLogicFormatPtr,
    OUT GT_U32                             *patternHwFormatArray,
    OUT GT_U32                             *maskHwFormatArray
)
{
    CPSS_DXCH_TTI_RULE_COMMON_STC       *commonPtr = NULL;
    CPSS_DXCH_TTI_RULE_COMMON_STC       *commonMaskPtr = NULL;
    GT_U32   dsaDevLsb;           /* dsa device LSBit */
    GT_U32   dsaDevMaskLsb;       /* dsa device mask LSBit */
    GT_U32   newDsaSrcPortTrunk;  /* dsa trunk/port */
    GT_U32   save_dsaSrcPortTrunk = 0;
    GT_U32   save_PatternDsaSrcDevice = 0;
    GT_U32   save_maskDsaSrcDevice = 0;
    GT_STATUS rc;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(patternLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(patternHwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(maskHwFormatArray);

     /* zero out hw format */
    cpssOsMemSet(patternHwFormatArray,0,sizeof(GT_U32)*TTI_RULE_SIZE_CNS);
    cpssOsMemSet(maskHwFormatArray,0,sizeof(GT_U32)*TTI_RULE_SIZE_CNS);

    /* check ipv4/mpls/eth configuration pararmeters only for config, not for mask */
    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        commonPtr = &(patternLogicFormatPtr->ipv4.common);
        commonMaskPtr = &(maskLogicFormatPtr->ipv4.common);

        if (patternLogicFormatPtr->ipv4.tunneltype > IPV4_TUNNEL_TYPE_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        commonPtr = &(patternLogicFormatPtr->mpls.common);
        commonMaskPtr = &(maskLogicFormatPtr->mpls.common);

        PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(patternLogicFormatPtr->mpls.exp0);
        PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(patternLogicFormatPtr->mpls.exp1);
        PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(patternLogicFormatPtr->mpls.exp2);
        if (patternLogicFormatPtr->mpls.label0 > MPLS_LABEL_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
         /* the check is needed only when we have more then 1 label*/
        if ((patternLogicFormatPtr->mpls.numOfLabels >= 1) && (patternLogicFormatPtr->mpls.label1 > MPLS_LABEL_MAX_CNS))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if ((patternLogicFormatPtr->mpls.numOfLabels >= 2) && (patternLogicFormatPtr->mpls.label2 > MPLS_LABEL_MAX_CNS))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->mpls.numOfLabels > MPLS_LABELS_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->mpls.numOfLabels < MPLS_LABELS_MAX_CNS)
        {
            if (patternLogicFormatPtr->mpls.protocolAboveMPLS > MPLS_PROTOCOL_ABOVE_MAX_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
          if (patternLogicFormatPtr->mpls.cwFirstNibble >= BIT_4)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if (patternLogicFormatPtr->mpls.reservedLabelExist && patternLogicFormatPtr->mpls.reservedLabelValue >= BIT_4)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if (patternLogicFormatPtr->mpls.channelTypeProfile >= BIT_4)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if (patternLogicFormatPtr->mpls.dataAfterInnerLabel >= BIT_5)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        commonPtr = &(patternLogicFormatPtr->eth.common);
        commonMaskPtr = &(maskLogicFormatPtr->eth.common);

        PRV_CPSS_DXCH_COS_CHECK_UP_MAC(patternLogicFormatPtr->eth.up0);
        if (patternLogicFormatPtr->eth.cfi0 > ETH_CFI_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->eth.isVlan1Exists)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(patternLogicFormatPtr->eth.vid1);
            PRV_CPSS_DXCH_COS_CHECK_UP_MAC(patternLogicFormatPtr->eth.up1);
            if (patternLogicFormatPtr->eth.cfi1 > ETH_CFI_MAX_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (patternLogicFormatPtr->eth.dsaQosProfile > QOS_PROFILE_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->eth.srcId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if ((patternLogicFormatPtr->eth.tag0TpidIndex >= BIT_3)||
                (patternLogicFormatPtr->eth.tag1TpidIndex >= BIT_3))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            if (patternLogicFormatPtr->eth.eTagGrp >= BIT_2)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:        
        commonPtr = &(patternLogicFormatPtr->mim.common);
        commonMaskPtr = &(maskLogicFormatPtr->mim.common);

        PRV_CPSS_DXCH_COS_CHECK_UP_MAC(patternLogicFormatPtr->mim.bUp);
        if (patternLogicFormatPtr->mim.bDp > DP_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->mim.iSid > MIM_ISID_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        PRV_CPSS_DXCH_COS_CHECK_UP_MAC(patternLogicFormatPtr->mim.iUp);
        if (patternLogicFormatPtr->mim.iDp > DP_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->mim.iRes1 > MIM_IRES_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (patternLogicFormatPtr->mim.iRes2 > MIM_IRES_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(patternLogicFormatPtr->mim.passengerPacketOuterTagVid);
            PRV_CPSS_DXCH_COS_CHECK_UP_MAC(patternLogicFormatPtr->mim.passengerPacketOuterTagUp);
            PRV_CPSS_DXCH3_CHECK_CFI_DEI_BIT_MAC(patternLogicFormatPtr->mim.passengerPacketOuterTagDei);
        }

        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        /* do nothing */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((sysGenGlobalInfo.supportDualDeviceId == GT_TRUE) && (commonPtr != NULL) && (commonMaskPtr != NULL))
    {
        newDsaSrcPortTrunk = commonPtr->dsaSrcPortTrunk;

        /*
         *       A. If <isTrunk> is exact match = port
         *           Then Cpss do :
         *           < hwDevNum >[0] .Pattern = <port/trunk>[6].Pattern
         *           < hwDevNum >[0] .Mask = <port/trunk>[6].Mask
         *           <port/trunk>[6]. Pattern = 0
         *           <port/trunk>[6]. Mask = no change
         */
        if ((commonMaskPtr->dsaSrcIsTrunk == 1) && (commonPtr->dsaSrcIsTrunk == 0))
        {
            dsaDevLsb     = U32_GET_FIELD_MAC(commonPtr->dsaSrcPortTrunk,6,1);
            dsaDevMaskLsb = U32_GET_FIELD_MAC(commonMaskPtr->dsaSrcPortTrunk,6,1);
            U32_SET_FIELD_MASKED_MAC(newDsaSrcPortTrunk,6,1,0);
        }
        /*
         *       B. Else If <isTrunk> is exact match = trunk
         *           Then Cpss do :
         *           < hwDevNum >[0] .Mask = dont care
         */
        else if ((commonMaskPtr->dsaSrcIsTrunk == 1) && (commonPtr->dsaSrcIsTrunk == 1))
        {
            dsaDevLsb     = 0;
            dsaDevMaskLsb = 0;
        }
        /*
         *      C. Else // <isTrunk> is dont care
         *          1) if <port/trunk>[6] is dont care
         *              Then Cpss do :
         *              < hwDevNum >[0] .Mask = dont care
         *          2) else //exact match  0 or 1
         *              return bad state //because trunk can come from 2 hemispheres
         */
        else
        {
             if(U32_GET_FIELD_MAC(commonMaskPtr->dsaSrcPortTrunk,6,1) == 1)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            dsaDevLsb     = 0;
            dsaDevMaskLsb = 0;
        }

        /* save values for restore */
        save_dsaSrcPortTrunk = commonPtr->dsaSrcPortTrunk;
        save_PatternDsaSrcDevice = commonPtr->dsaSrcDevice;
        save_maskDsaSrcDevice = commonMaskPtr->dsaSrcDevice;

        /* update the fields */
        U32_SET_FIELD_MASKED_MAC(commonPtr->dsaSrcPortTrunk,0,7,newDsaSrcPortTrunk);
        U32_SET_FIELD_MASKED_MAC(commonPtr->dsaSrcDevice,0,1,dsaDevLsb);
        U32_SET_FIELD_MASKED_MAC(commonMaskPtr->dsaSrcDevice,0,1,dsaDevMaskLsb);
    }

    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
    case CPSS_DXCH_TTI_RULE_MPLS_E:
    case CPSS_DXCH_TTI_RULE_ETH_E:
    case CPSS_DXCH_TTI_RULE_MIM_E:
        if (ttiConfigCheckCommon(devNum,commonPtr) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    case CPSS_DXCH_TTI_RULE_UDB_10_E:
    case CPSS_DXCH_TTI_RULE_UDB_20_E:
    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        rc = ttiConfigSetLogic2HwFormat(devNum,ruleType,patternLogicFormatPtr,patternHwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = ttiConfigSetLogic2HwFormat(devNum,ruleType,maskLogicFormatPtr,maskHwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* convert 'old keys' as udbs */
        rc = sip5TtiConfigSetLogic2HwUdbFormat(ruleType,GT_TRUE,patternLogicFormatPtr,patternHwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = sip5TtiConfigSetLogic2HwUdbFormat(ruleType,GT_FALSE,maskLogicFormatPtr,maskHwFormatArray);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* restore the values */
    if ((sysGenGlobalInfo.supportDualDeviceId == GT_TRUE) && (commonPtr != NULL) && (commonMaskPtr != NULL))
    {
        commonPtr->dsaSrcPortTrunk = save_dsaSrcPortTrunk;
        commonPtr->dsaSrcDevice = save_PatternDsaSrcDevice;
        commonMaskPtr->dsaSrcDevice = save_maskDsaSrcDevice;
    }

    return GT_OK;
}

/**
* @internal ttiConfigHw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI rule type
* @param[in] hwFormatArray            - tunnel termination configuration in HW format (6 words).
*
* @param[out] logicFormatPtr           - points to tti configuration in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS ttiConfigHw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_DXCH_TTI_RULE_UNT              *logicFormatPtr
)
{
    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:

        logicFormatPtr->ipv4.common.pclId        = U32_GET_FIELD_MAC(hwFormatArray[0],0,10);
        logicFormatPtr->ipv4.common.srcPortTrunk = U32_GET_FIELD_MAC(hwFormatArray[0],10,8);
        logicFormatPtr->ipv4.common.srcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],18,1));
        logicFormatPtr->ipv4.common.vid          = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],19,12);
        logicFormatPtr->ipv4.common.isTagged     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],31,1));

        logicFormatPtr->ipv4.common.mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],0,8);
        logicFormatPtr->ipv4.common.mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],8,8);
        logicFormatPtr->ipv4.common.mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],16,8);
        logicFormatPtr->ipv4.common.mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],24,8);

        logicFormatPtr->ipv4.common.mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],0,8);
        logicFormatPtr->ipv4.common.mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],8,8);
        logicFormatPtr->ipv4.common.dsaSrcDevice   = U32_GET_FIELD_MAC(hwFormatArray[2],16,7);
        logicFormatPtr->ipv4.tunneltype            = U32_GET_FIELD_MAC(hwFormatArray[2],23,3);
        logicFormatPtr->ipv4.srcIp.arIP[3]         = (GT_U8)(U32_GET_FIELD_MAC(hwFormatArray[2],26,6) |
                                                            (U32_GET_FIELD_MAC(hwFormatArray[3],0,2) << 6));

        logicFormatPtr->ipv4.srcIp.arIP[2]  = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[3],2,8);
        logicFormatPtr->ipv4.srcIp.arIP[1]  = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[3],10,8);
        logicFormatPtr->ipv4.srcIp.arIP[0]  = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[3],18,8);
        logicFormatPtr->ipv4.destIp.arIP[3] = (GT_U8)(U32_GET_FIELD_MAC(hwFormatArray[3],26,6) |
                                                     (U32_GET_FIELD_MAC(hwFormatArray[4],0,2) << 6));

        logicFormatPtr->ipv4.destIp.arIP[2] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[4],2,8);
        logicFormatPtr->ipv4.destIp.arIP[1] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[4],10,8);
        logicFormatPtr->ipv4.destIp.arIP[0] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[4],18,8);
        logicFormatPtr->ipv4.isArp          = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],26,1));

        logicFormatPtr->ipv4.common.dsaSrcPortTrunk = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[5],0,7);

        logicFormatPtr->ipv4.common.dsaSrcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[5],7,1));

        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            /* To allow support for larger amounts of cores in a single device,
               an additional 1 bits must be added to all TTI keys */
            logicFormatPtr->ipv4.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],28,3);
        }
        else
        {
            logicFormatPtr->ipv4.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],29,2);
        }
        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:

        logicFormatPtr->mpls.common.pclId        = U32_GET_FIELD_MAC(hwFormatArray[0],0,10);
        logicFormatPtr->mpls.common.srcPortTrunk = U32_GET_FIELD_MAC(hwFormatArray[0],10,8);
        logicFormatPtr->mpls.common.srcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],18,1));
        logicFormatPtr->mpls.common.vid          = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],19,12);
        logicFormatPtr->mpls.common.isTagged     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],31,1));

        logicFormatPtr->mpls.common.mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],0,8);
        logicFormatPtr->mpls.common.mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],8,8);
        logicFormatPtr->mpls.common.mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],16,8);
        logicFormatPtr->mpls.common.mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],24,8);

        logicFormatPtr->mpls.common.mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],0,8);
        logicFormatPtr->mpls.common.mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],8,8);
        logicFormatPtr->mpls.common.dsaSrcDevice   = U32_GET_FIELD_MAC(hwFormatArray[2],16,7);
        logicFormatPtr->mpls.exp0                  = U32_GET_FIELD_MAC(hwFormatArray[2],23,3);
        logicFormatPtr->mpls.exp1                  = U32_GET_FIELD_MAC(hwFormatArray[2],26,3);
        logicFormatPtr->mpls.label0                = U32_GET_FIELD_MAC(hwFormatArray[2],29,3) |
                                                     (U32_GET_FIELD_MAC(hwFormatArray[3],0,17) << 3);

        logicFormatPtr->mpls.label1 = U32_GET_FIELD_MAC(hwFormatArray[3],17,15) |
                                      (U32_GET_FIELD_MAC(hwFormatArray[4],0,5) << 15);

        logicFormatPtr->mpls.exp2              = U32_GET_FIELD_MAC(hwFormatArray[4],5,3);
        logicFormatPtr->mpls.label2            = U32_GET_FIELD_MAC(hwFormatArray[4],8,20);
        logicFormatPtr->mpls.numOfLabels       = U32_GET_FIELD_MAC(hwFormatArray[4],28,2);
        logicFormatPtr->mpls.protocolAboveMPLS = U32_GET_FIELD_MAC(hwFormatArray[4],30,2);

        logicFormatPtr->mpls.common.dsaSrcPortTrunk = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[5],0,7);
        logicFormatPtr->mpls.common.dsaSrcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[5],7,1));
        logicFormatPtr->mpls.cwFirstNibble = U32_GET_FIELD_MAC(hwFormatArray[5],8,4);
         if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            /* To allow support for larger amounts of cores in a single device,
               an additional 1 bits must be added to all TTI keys */
            logicFormatPtr->mpls.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],28,3);
        }
        else
        {
            logicFormatPtr->mpls.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],29,2);
        }

        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:

        logicFormatPtr->eth.common.pclId         = U32_GET_FIELD_MAC(hwFormatArray[0],0,10);
        logicFormatPtr->eth.common.srcPortTrunk = U32_GET_FIELD_MAC(hwFormatArray[0],10,8);
        logicFormatPtr->eth.common.srcIsTrunk    = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],18,1));
        logicFormatPtr->eth.common.vid       = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],19,12);
        logicFormatPtr->eth.common.isTagged      = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],31,1));

        logicFormatPtr->eth.common.mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],0,8);
        logicFormatPtr->eth.common.mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],8,8);
        logicFormatPtr->eth.common.mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],16,8);
        logicFormatPtr->eth.common.mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],24,8);

        logicFormatPtr->eth.common.mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],0,8);
        logicFormatPtr->eth.common.mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],8,8);
        logicFormatPtr->eth.common.dsaSrcDevice   = U32_GET_FIELD_MAC(hwFormatArray[2],16,7);

        logicFormatPtr->eth.up0           = U32_GET_FIELD_MAC(hwFormatArray[2],23,3);
        logicFormatPtr->eth.cfi0          = U32_GET_FIELD_MAC(hwFormatArray[2],26,1);
        logicFormatPtr->eth.isVlan1Exists = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],27,1));
        logicFormatPtr->eth.vid1          = (GT_U16)(U32_GET_FIELD_MAC(hwFormatArray[2],28,4) |
                                                    (U32_GET_FIELD_MAC(hwFormatArray[3],0,8) << 4));

        logicFormatPtr->eth.up1       = U32_GET_FIELD_MAC(hwFormatArray[3],8,3);
        logicFormatPtr->eth.cfi1      = U32_GET_FIELD_MAC(hwFormatArray[3],11,1);
        logicFormatPtr->eth.etherType = U32_GET_FIELD_MAC(hwFormatArray[3],12,16);
        logicFormatPtr->eth.macToMe   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[3],28,1));

        logicFormatPtr->eth.common.dsaSrcPortTrunk = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[5],0,7);
        logicFormatPtr->eth.common.dsaSrcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[5],7,1));
        logicFormatPtr->eth.srcId                  = U32_GET_FIELD_MAC(hwFormatArray[5],8,5);

        logicFormatPtr->eth.dsaQosProfile          = U32_GET_FIELD_MAC(hwFormatArray[5],13,7);

        if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            logicFormatPtr->eth.eTagGrp = U32_GET_FIELD_MAC(hwFormatArray[5],20,2);
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            /* To allow support for larger amounts of cores in a single device,
               an additional 1 bits must be added to all TTI keys */
            logicFormatPtr->eth.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],28,3);

        }
        else
        {
            logicFormatPtr->eth.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],29,2);
        }

        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:        
        logicFormatPtr->mim.common.pclId         = U32_GET_FIELD_MAC(hwFormatArray[0],0,10);
        logicFormatPtr->mim.common.srcPortTrunk = U32_GET_FIELD_MAC(hwFormatArray[0],10,8);
        logicFormatPtr->mim.common.srcIsTrunk    = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],18,1));
        logicFormatPtr->mim.common.vid       = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],19,12);
        logicFormatPtr->mim.common.isTagged      = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],31,1));

        logicFormatPtr->mim.common.mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],0,8);
        logicFormatPtr->mim.common.mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],8,8);
        logicFormatPtr->mim.common.mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],16,8);
        logicFormatPtr->mim.common.mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[1],24,8);

        logicFormatPtr->mim.common.mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],0,8);
        logicFormatPtr->mim.common.mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[2],8,8);
        logicFormatPtr->mim.common.dsaSrcDevice   = U32_GET_FIELD_MAC(hwFormatArray[2],16,7);

        logicFormatPtr->mim.bUp   = U32_GET_FIELD_MAC(hwFormatArray[2],23,3);
        logicFormatPtr->mim.bDp   = U32_GET_FIELD_MAC(hwFormatArray[2],26,1);

        logicFormatPtr->mim.iSid  = U32_GET_FIELD_MAC(hwFormatArray[2],28,4) |
                                    (U32_GET_FIELD_MAC(hwFormatArray[3],0,20) << 4);

        logicFormatPtr->mim.iUp   = U32_GET_FIELD_MAC(hwFormatArray[3],20,3);
        logicFormatPtr->mim.iDp   = U32_GET_FIELD_MAC(hwFormatArray[3],23,1);
        logicFormatPtr->mim.iRes1 = U32_GET_FIELD_MAC(hwFormatArray[3],24,2);
        logicFormatPtr->mim.iRes2 = U32_GET_FIELD_MAC(hwFormatArray[3],26,2);

        logicFormatPtr->mim.common.dsaSrcPortTrunk = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[5],0,7);
        logicFormatPtr->mim.common.dsaSrcIsTrunk   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[5],7,1));

        if(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).portGroupId > 2)
        {
            /* To allow support for larger amounts of cores in a single device,
               an additional 1 bits must be added to all TTI keys */
            logicFormatPtr->mim.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],28,3);
        }
        else
        {
            logicFormatPtr->mim.common.sourcePortGroupId = U32_GET_FIELD_MAC(hwFormatArray[5],29,2);
        }

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal ttiConfigHwUdb2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti configuration from hardware format
*         to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] ruleType                 - TTI rule type
* @param[in] hwFormatArray            - tunnel termination configuration in HW format (6 words).
* @param[in] isPattern                - this is pattern
*
* @param[out] logicFormatPtr           - points to tti configuration in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS ttiConfigHwUdb2LogicFormat
(
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  GT_BOOL                             isPattern,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_DXCH_TTI_RULE_UNT              *logicFormatPtr
)
{

    GT_U32 sbits[3]={0};   /* stop bit for mpls label */

    GT_U32 i;
    GT_U32 length = 0;
    GT_U32 maskedSbitCounter = 0;
    GT_U32 hwValue;
    GT_U32 tmpValue;
    CPSS_DXCH_TTI_RULE_COMMON_STC *commonPtr = NULL;
    CPSS_DXCH_TTI_IPV4_RULE_STC   *ipv4Ptr;
    CPSS_DXCH_TTI_MPLS_RULE_STC   *mplsPtr;
    CPSS_DXCH_TTI_ETH_RULE_STC    *ethPtr;
    CPSS_DXCH_TTI_MIM_RULE_STC    *mimPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        commonPtr = &logicFormatPtr->ipv4.common;
        ipv4Ptr = &logicFormatPtr->ipv4;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_SIP_E);

        ipv4Ptr->srcIp.arIP[3]              = (GT_U8)(hwValue >>  0);
        ipv4Ptr->srcIp.arIP[2]              = (GT_U8)(hwValue >>  8);
        ipv4Ptr->srcIp.arIP[1]              = (GT_U8)(hwValue >> 16);
        ipv4Ptr->srcIp.arIP[0]              = (GT_U8)(hwValue >> 24);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_DIP_E);

        ipv4Ptr->destIp.arIP[3]             = (GT_U8)(hwValue >>  0);
        ipv4Ptr->destIp.arIP[2]             = (GT_U8)(hwValue >>  8);
        ipv4Ptr->destIp.arIP[1]             = (GT_U8)(hwValue >> 16);
        ipv4Ptr->destIp.arIP[0]             = (GT_U8)(hwValue >> 24);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_IS_ARP_E);
        ipv4Ptr->isArp                      = BIT2BOOL_MAC(hwValue);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_IPV4_TUNNELING_PROTOCOL_E);
        ipv4Ptr->tunneltype                 = hwValue;

        break;

    case CPSS_DXCH_TTI_RULE_MPLS_E:
        commonPtr = &logicFormatPtr->mpls.common;
        mplsPtr = &logicFormatPtr->mpls;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S0_E);
        sbits[0]                            = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP0_E);
        mplsPtr->exp0                       = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL0_E);
        mplsPtr->label0                     = hwValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S1_E);
        sbits[1]                            = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP1_E);
        mplsPtr->exp1                       = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL1_E);
        mplsPtr->label1                     = hwValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_S2_E);
        sbits[2]                            = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_EXP2_E);
        mplsPtr->exp2                       = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_LABEL2_E);
        mplsPtr->label2                     = hwValue;


        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_VALUE_E);
        mplsPtr->reservedLabelValue         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E);
        mplsPtr->channelTypeProfile         = hwValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_PROTOCOL_AFTER_MPLS_E);
        mplsPtr->protocolAboveMPLS          = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_EXISTS_E);
        mplsPtr->reservedLabelExist         = BIT2BOOL_MAC(hwValue);
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MPLS_DATA_AFTER_INNER_LABEL_E);
        mplsPtr->dataAfterInnerLabel        = hwValue;

        if (isPattern)
        {
            if (sbits[0] == 1)
            {
                mplsPtr->numOfLabels = 0; /* One label*/
            }
            else if (sbits[1] == 1)
            {
                mplsPtr->numOfLabels = 1; /* two labels*/
            }
            else if (sbits[2] == 1)
            {
                mplsPtr->numOfLabels = 2; /* three labels*/
            }
        }
        else /* mask */
        {
            for (i = 0; i < 3; i++)
            {
                if (sbits[i] == 1)
                {
                    maskedSbitCounter++;
                }
            }
            if (maskedSbitCounter == 0)
            {
                mplsPtr->numOfLabels = 0;
            }
            else
            {
                mplsPtr->numOfLabels = 3;
            }
        }

        break;

    case CPSS_DXCH_TTI_RULE_ETH_E:
        commonPtr = &logicFormatPtr->eth.common;
        ethPtr = &logicFormatPtr->eth;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_UP0_E);
        ethPtr->up0                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_CFI0_E);
        ethPtr->cfi0                        = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E);
        ethPtr->macToMe                     = BIT2BOOL_MAC(hwValue);
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN1_E);
        ethPtr->vid1                        = (GT_U16)hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_UP1_E);
        ethPtr->up1                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_CFI1_E);
        ethPtr->cfi1                        = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_ETHERTYPE_E);
        ethPtr->etherType                   = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN_TAG1_EXISTS_E);
        ethPtr->isVlan1Exists               = BIT2BOOL_MAC(hwValue);
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG0_TPID_INDEX_E);
        ethPtr->tag0TpidIndex               = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG1_TPID_INDEX_E);
        ethPtr->tag1TpidIndex               = hwValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_QOS_PROFILE_E);
        ethPtr->dsaQosProfile               = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_SOURCE_ID_11_0_E);
        ethPtr->srcId                       = hwValue;

        break;

    case CPSS_DXCH_TTI_RULE_MIM_E:
        commonPtr = &logicFormatPtr->mim.common;
        mimPtr = &logicFormatPtr->mim;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_UP0_E);
        mimPtr->bUp                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_CFI0_E);
        mimPtr->bDp                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E);
        mimPtr->macToMe                     = BIT2BOOL_MAC(hwValue);
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PASSENGER_OUTER_CFI_E);
        mimPtr->passengerPacketOuterTagDei          = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_SID_E);
        mimPtr->iSid                        = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_TAG_OCTET1_BITS_1_4_E);
        mimPtr->iRes1                       = hwValue & 0x3;/*2 bits : 0..1*/
        mimPtr->iRes2                       = (hwValue >> 2) & 0x3;/*2 bits : 2..3*/
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_DP_E);
        mimPtr->iDp                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_I_UP_E);
        mimPtr->iUp                         = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_VID_E);
        mimPtr->passengerPacketOuterTagVid          = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_UP_E);
        mimPtr->passengerPacketOuterTagUp           = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_EXISTS_E);
        mimPtr->passengerPacketOuterTagExists       = BIT2BOOL_MAC(hwValue);


        break;

    case CPSS_DXCH_TTI_RULE_UDB_10_E:
        length = 10;

        break;

    case CPSS_DXCH_TTI_RULE_UDB_20_E:
        length = 20;

        break;

    case CPSS_DXCH_TTI_RULE_UDB_30_E:
        length = 30;

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(commonPtr)/*legacy keys only */
    {
        /* convert the common part */

        /* the PCL-ID is split field */
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PCLID_4_0_E);
        tmpValue = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_PCLID_9_5_E);
        tmpValue += hwValue << 5;

        commonPtr->pclId                = tmpValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E);
        commonPtr->srcIsTrunk           = BIT2BOOL_MAC(hwValue);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCISTRUNK_E);
        commonPtr->dsaSrcIsTrunk        = BIT2BOOL_MAC(hwValue);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_VLAN_TAG0_EXISTS_E);
        commonPtr->isTagged             = BIT2BOOL_MAC(hwValue);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_SRC_COREID_E);
        commonPtr->sourcePortGroupId    = hwValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_EVLAN_7_0_E);
        tmpValue = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_EVLAN_12_8_E);
        tmpValue += hwValue << 8;

        commonPtr->vid                  = (GT_U16)tmpValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E);
        tmpValue = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E);
        tmpValue += hwValue << 8;

        commonPtr->srcPortTrunk         = tmpValue;

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E);
        tmpValue = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E);
        tmpValue += hwValue << 8;

        commonPtr->dsaSrcPortTrunk      = tmpValue;

        MAC_ADDR_GET_FIELD_BY_ID_MAC(hwFormatArray,
            ttiLegacyKeyFieldsFormat,
            TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E,
            &commonPtr->mac.arEther[0]);

        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_7_0_E);
        tmpValue = hwValue;
        GET_FIELD_FROM_LEGACY_KEY_MAC(hwValue,TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_9_8_E);
        tmpValue += hwValue << 8;

        commonPtr->dsaSrcDevice        = tmpValue;

    }
    else
    {
        /* getting UDBs */
        for (i=0; i<length; i++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwFormatArray,(8*i),8,hwValue);
            logicFormatPtr->udbArray[i] = (GT_U8)hwValue;
        }
    }

    return GT_OK;
}

/**
* @internal ttiConfigLogicFormatForDualDeviceId function
* @endinternal
*
* @brief   Converts a given tti configuration from hardware format
*         to logic format for dual device id.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] ruleType                 - TTI rule type
* @param[in,out] patternLogicFormatPtr    - points to tunnel termination configuration in
*                                      pattern logic format
* @param[in,out] maskLogicFormatPtr       - points to tunnel termination configuration in
*                                      mask logic format
* @param[in,out] patternLogicFormatPtr    - points to tunnel termination configuration in
*                                      pattern logic format
* @param[in,out] maskLogicFormatPtr       - points to tunnel termination configuration in
*                                      mask logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*
* @note bit '0' of dsaSrcDevice is always returned as 'don't care'
*
*/
static GT_STATUS ttiConfigLogicFormatForDualDeviceId
(
    IN     CPSS_DXCH_TTI_RULE_TYPE_ENT        ruleType,
    INOUT  CPSS_DXCH_TTI_RULE_UNT             *patternLogicFormatPtr,
    INOUT  CPSS_DXCH_TTI_RULE_UNT             *maskLogicFormatPtr
)
{
    CPSS_DXCH_TTI_RULE_COMMON_STC   *commonPatternPtr;
    CPSS_DXCH_TTI_RULE_COMMON_STC   *commonMaskPtr;

    if (sysGenGlobalInfo.supportDualDeviceId == GT_FALSE)
    {
        return GT_OK;
    }

    if ((ruleType == CPSS_DXCH_TTI_RULE_UDB_10_E) ||
        (ruleType == CPSS_DXCH_TTI_RULE_UDB_20_E) ||
        (ruleType == CPSS_DXCH_TTI_RULE_UDB_30_E))
    {
        /* nothing to do for UDB rules */
        return GT_OK;
    }

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(patternLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskLogicFormatPtr);

    switch (ruleType)
    {
    case CPSS_DXCH_TTI_RULE_IPV4_E:
        commonPatternPtr = &patternLogicFormatPtr->ipv4.common;
        commonMaskPtr    = &maskLogicFormatPtr->ipv4.common;
        break;
    case CPSS_DXCH_TTI_RULE_MPLS_E:
        commonPatternPtr = &patternLogicFormatPtr->ipv4.common;
        commonMaskPtr    = &maskLogicFormatPtr->ipv4.common;
        break;
    case CPSS_DXCH_TTI_RULE_ETH_E:
        commonPatternPtr = &patternLogicFormatPtr->ipv4.common;
        commonMaskPtr    = &maskLogicFormatPtr->ipv4.common;
        break;
    case CPSS_DXCH_TTI_RULE_MIM_E:
        commonPatternPtr = &patternLogicFormatPtr->ipv4.common;
        commonMaskPtr    = &maskLogicFormatPtr->ipv4.common;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(commonMaskPtr->dsaSrcIsTrunk == 1 && commonPatternPtr->dsaSrcIsTrunk == 0)
    {
        /*<isTrunk> is exact match = port*/
        U32_SET_FIELD_MASKED_MAC(commonPatternPtr->dsaSrcPortTrunk,6,1,
                                 U32_GET_FIELD_MAC(commonPatternPtr->dsaSrcDevice,0,1));
        U32_SET_FIELD_MASKED_MAC(commonMaskPtr->dsaSrcPortTrunk,6,1,
                                 U32_GET_FIELD_MAC(commonMaskPtr->dsaSrcDevice,0,1));
    }
    else if(commonMaskPtr->dsaSrcIsTrunk == 1 && commonPatternPtr->dsaSrcIsTrunk == 1)
    {
        /*<isTrunk> is exact match = trunk*/

        /* no manipulation needed */
    }
    else
    {
        /*<isTrunk> is dont care*/

        /* no manipulation needed */
    }

    /* bit '0' of dsaSrcDevice is always returned as 'don't care' */
    U32_SET_FIELD_MASKED_MAC(commonPatternPtr->dsaSrcDevice,0,1,0);
    U32_SET_FIELD_MASKED_MAC(commonMaskPtr->dsaSrcDevice,0,1,0);

    return GT_OK;
}

/**
* @internal ttiActionXcatLogic2HwFormat function
* @endinternal
*
* @brief   Xcat and above (not sip5) Converts a given tti action (type 2) from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] hwValuesInfoPtr          - the parameters that already set in hw values during
*                                      'parameter check'  --> this to avoid duplication of
*                                      large code of checking parameters
*                                      (shared with xCat3,lion2 devices).
*
* @param[out] hwFormatArray            - tti action in hardware format (8 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiActionXcatLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr,
    IN  TTI_ACTION_HW_VALUES_STC           *hwValuesInfoPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    GT_U32 tempRouterLttPtr; /* pointer to the Router Lookup Translation Table entry (hw format) */
    GT_U32      pwTagMode = 0;                  /* Pseudowire Tag Mode (hw format) */
    GT_U32      unknownSaCommand = 0;           /* unkown macSA command (hw format) */
    GT_U32      sourceMeshId = 0;               /* source mesh ID (hw format) */
    GT_U32      cwBasedPw = 0;                  /* Pseudowire has control word (hw format) */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwValuesInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(logicFormatPtr->pwTagMode)
        {
        case CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E:
            pwTagMode = 0;
            break;
        case CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E:
            pwTagMode = 1;
            break;
        case CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E:
            pwTagMode = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (logicFormatPtr->sourceMeshIdSetEnable == GT_TRUE)
        {
            if (logicFormatPtr->sourceMeshId >= BIT_2)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            sourceMeshId = logicFormatPtr->sourceMeshId;
        }

        cwBasedPw = ((logicFormatPtr->cwBasedPw == GT_TRUE) ? 1 : 0);

        if (logicFormatPtr->unknownSaCommandEnable == GT_TRUE)
        {
            switch (logicFormatPtr->unknownSaCommand)
            {
            case CPSS_PACKET_CMD_FORWARD_E:
                unknownSaCommand = 0;
                break;
            case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                unknownSaCommand = 1;
                break;
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                unknownSaCommand = 2;
                break;
            case CPSS_PACKET_CMD_DROP_HARD_E:
                unknownSaCommand = 3;
                break;
            case CPSS_PACKET_CMD_DROP_SOFT_E:
                unknownSaCommand = 4;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else /* logicFormatPtr->unknownSaCommandEnable == GT_FALSE */
        {
            unknownSaCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        }
    }

    /* handle word 0 (bits 0-31) and also some of word 1 (bits 32-63) */
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],0,3,hwValuesInfoPtr->pktCommand);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],3,8,hwValuesInfoPtr->userDefinedCpuCode);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],11,1,BOOL2BIT_MAC(logicFormatPtr->mirrorToIngressAnalyzerEnable));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],12,3,hwValuesInfoPtr->redirectCommand);

    switch (logicFormatPtr->redirectCommand)
    {
    case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
        switch (logicFormatPtr->egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,1,0); /* target is not trunk */
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],16,6,hwValuesInfoPtr->hwPort);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],22,5,hwValuesInfoPtr->hwDev);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],27,1,0); /* target not VIDX */
            break;
        case CPSS_INTERFACE_TRUNK_E:
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,1,1); /* target is trunk */
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],16,7,logicFormatPtr->egressInterface.trunkId);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],27,1,0); /* target not VIDX */
            break;
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            if(logicFormatPtr->egressInterface.type == CPSS_INTERFACE_VID_E)
            {
                U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,12,0xFFF);/* special VIDX */
            }
            else
            {
                U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,12,logicFormatPtr->egressInterface.vidx);
            }
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],27,1,1); /* target is VIDX */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],28,1,BOOL2BIT_MAC(logicFormatPtr->tunnelStart));
        if (logicFormatPtr->tunnelStart == GT_TRUE)
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],29,3,logicFormatPtr->tunnelStartPtr & 0x7);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,10,logicFormatPtr->tunnelStartPtr >> 3);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],10,1,hwValuesInfoPtr->tsPassengerPacketType);
        }
        else
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],29,3,logicFormatPtr->arpPtr & 0x7);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,11,logicFormatPtr->arpPtr >> 3);
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],11,1,BOOL2BIT_MAC(logicFormatPtr->vntl2Echo));
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))
        {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],22,1,BOOL2BIT_MAC(logicFormatPtr->modifyMacDa));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],23,1,BOOL2BIT_MAC(logicFormatPtr->modifyMacSa));
        }
        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
       /* LTT entry is row based however in this field the LTT is treated as column based.
          Bits [11:0] indicate row while bits [14:13] indicate column, bit 12 is not used.
          The Formula for translating the LTT entry to column based is as follow:
          [11:0] << 2 + [14:13]   (Note: bit 12 not used). */
        tempRouterLttPtr = (((hwValuesInfoPtr->routerLttPtr & 0x3FFC) >> 2) | ((hwValuesInfoPtr->routerLttPtr & 0x3) << 13));
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,15,tempRouterLttPtr);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,logicFormatPtr->iPclConfigIndex & 0x1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,12,logicFormatPtr->iPclConfigIndex >> 1);
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],22,1,BOOL2BIT_MAC(logicFormatPtr->ResetSrcPortGroupId));
        }
        break;
    case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,12,logicFormatPtr->vrfId);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,logicFormatPtr->iPclConfigIndex & 0x1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,12,logicFormatPtr->iPclConfigIndex >> 1);
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],22,1,BOOL2BIT_MAC(logicFormatPtr->ResetSrcPortGroupId));
        }
        break;

    case CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E:
        /* redirect to logical port is supported only for Lion2-B0 and above; xCat3*/
        if((PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum)) ||
           (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE))
        {
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            hwValuesInfoPtr->hwDev = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            hwValuesInfoPtr->hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);


            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,1,0); /* target is not trunk */
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],16,6,hwValuesInfoPtr->hwPort);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],22,5,hwValuesInfoPtr->hwDev);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],27,1,0); /* target not VIDX */

            U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,logicFormatPtr->iPclConfigIndex & 0x1);
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,12,logicFormatPtr->iPclConfigIndex >> 1);
            if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],22,1,BOOL2BIT_MAC(logicFormatPtr->ResetSrcPortGroupId));
            }

            if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
            {

                U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],27,2,pwTagMode);
                U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],29,2,sourceMeshId);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    case CPSS_DXCH_TTI_NO_REDIRECT_E:
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],15,1,BOOL2BIT_MAC(logicFormatPtr->multiPortGroupTtiEnable));
        }
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[0],31,1,logicFormatPtr->iPclConfigIndex & 0x1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],0,12,logicFormatPtr->iPclConfigIndex >> 1);
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],22,1,BOOL2BIT_MAC(logicFormatPtr->ResetSrcPortGroupId));
        }
    default:
        /* do noting */
        break;
    }

    /* handle word 1 (bits 32-63) */
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],12,1,BOOL2BIT_MAC(logicFormatPtr->bindToCentralCounter));
    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_TRUE)
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],13,12,logicFormatPtr->centralCounterIndex);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],25,1,cwBasedPw);
    }
    else
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],13,14,logicFormatPtr->centralCounterIndex);
    }
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],27,1,BOOL2BIT_MAC(logicFormatPtr->bindToPolicerMeter));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],28,1,BOOL2BIT_MAC(logicFormatPtr->bindToPolicer));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],29,3,logicFormatPtr->policerIndex & 0x7);

    /* handle word 2 (bits 64-95) */
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],0,9,logicFormatPtr->policerIndex >> 3);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],9,1,BOOL2BIT_MAC(logicFormatPtr->sourceIdSetEnable));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],10,5,logicFormatPtr->sourceId);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],15,1,BOOL2BIT_MAC(logicFormatPtr->actionStop));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],16,1,BOOL2BIT_MAC(logicFormatPtr->bridgeBypass));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],17,1,BOOL2BIT_MAC(logicFormatPtr->ingressPipeBypass));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],18,1,hwValuesInfoPtr->pcl1OverrideConfigIndex);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],19,1,hwValuesInfoPtr->pcl0_1OverrideConfigIndex);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],20,1,hwValuesInfoPtr->pcl0OverrideConfigIndex);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],21,1,hwValuesInfoPtr->tag0VlanPrecedence);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],22,1,BOOL2BIT_MAC(logicFormatPtr->nestedVlanEnable));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],23,2,hwValuesInfoPtr->tag0VlanCmd);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[2],25,7,logicFormatPtr->tag0VlanId & 0x7F);

    /* handle word 3 (bits 96-127) */
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],0,5,logicFormatPtr->tag0VlanId >> 7);
    if (logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],5,1,hwValuesInfoPtr->tag1VlanCmd);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],6,12,logicFormatPtr->tag1VlanId);
    }
    else
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],5,8,logicFormatPtr->mplsTtl);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],13,1,BOOL2BIT_MAC(logicFormatPtr->enableDecrementTtl));
    }
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],18,1,hwValuesInfoPtr->qosPrecedence);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],19,7,logicFormatPtr->qosProfile);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],26,2,hwValuesInfoPtr->modifyDSCP);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],28,2,hwValuesInfoPtr->modifyTag0Up);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],30,1,BOOL2BIT_MAC(logicFormatPtr->keepPreviousQoS));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[3],31,1,BOOL2BIT_MAC(logicFormatPtr->trustUp));

    /* handle word 4 (bits 128-159) */
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],0,1,BOOL2BIT_MAC(logicFormatPtr->trustDscp));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],1,1,BOOL2BIT_MAC(logicFormatPtr->trustExp));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],2,1,BOOL2BIT_MAC(logicFormatPtr->remapDSCP));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],3,3,logicFormatPtr->tag0Up);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],6,2,hwValuesInfoPtr->tag1UpCommand);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],8,3,logicFormatPtr->tag1Up);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],11,2,hwValuesInfoPtr->ttPassengerPktType);
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],13,1,BOOL2BIT_MAC(logicFormatPtr->copyTtlExpFromTunnelHeader));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],14,1,BOOL2BIT_MAC(logicFormatPtr->tunnelTerminate));
    U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],15,3,hwValuesInfoPtr->mplsCommand);
    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_TRUE)
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[1],26,1,unknownSaCommand & 0x1);
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],18,2,unknownSaCommand >> 1);
    }
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        U32_SET_FIELD_MASKED_MAC(hwFormatArray[4],18,4,logicFormatPtr->hashMaskIndex & 0xF);
    }


    return GT_OK;
}

/**
* @internal ttiActionSip5Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action (type 2)
*         from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] hwValuesInfoPtr          - the parameters that already set in hw values during
*                                      'parameter check'  --> this to avoid duplication of
*                                      large code of checking parameters
*
* @param[out] hwFormatArray            - tti action in hardware format (8 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiActionSip5Logic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr,
    IN  TTI_ACTION_HW_VALUES_STC           *hwValuesInfoPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    GT_U32      value, value2;
    GT_BOOL     enablePtr;
    GT_STATUS   rc;
    GT_U32      lpmIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E |  CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwValuesInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    /********************************************************/
    /* check params and translate params to hardware format */
    /********************************************************/

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/

    value = hwValuesInfoPtr->pktCommand;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E,
        value);

    value = hwValuesInfoPtr->userDefinedCpuCode;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E,
        value);

    if(logicFormatPtr->mirrorToIngressAnalyzerEnable == GT_TRUE)
    {
        value = logicFormatPtr->mirrorToIngressAnalyzerIndex + 1;
        if(0 == CHECK_BITS_DATA_RANGE_MAC(logicFormatPtr->mirrorToIngressAnalyzerIndex + 1, 3))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        value = 0;
    }
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
        value);

    value = hwValuesInfoPtr->redirectCommand;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E,
        value);


    switch (logicFormatPtr->redirectCommand)
    {
    case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
        switch (logicFormatPtr->egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            /* target is not trunk */
            value = 0;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                value);

            value = hwValuesInfoPtr->hwPort;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E,
                value);

            value = hwValuesInfoPtr->hwDev;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E,
                value);

            /* target not VIDX */
            value = 0;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                value);

            break;
        case CPSS_INTERFACE_TRUNK_E:
            /* target is trunk */
            value = 1;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                value);

            value = logicFormatPtr->egressInterface.trunkId;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E,
                value);

            /* target not VIDX */
            value = 0;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                value);
            break;
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            if(logicFormatPtr->egressInterface.type == CPSS_INTERFACE_VID_E)
            {
                value = 0xfff;
            }
            else
            {
                value = logicFormatPtr->egressInterface.vidx;
            }

            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E,
                value);

            /* target is VIDX */
            value = 1;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                value);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        value = BOOL2BIT_MAC(logicFormatPtr->tunnelStart);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E,
            value);


        if (logicFormatPtr->tunnelStart == GT_TRUE)
        {
            value = logicFormatPtr->tunnelStartPtr;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E,
                value);

            value = hwValuesInfoPtr->tsPassengerPacketType;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E,
                value);
        }
        else
        {
            value = logicFormatPtr->arpPtr;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E,
                value);
        }

        value = BOOL2BIT_MAC(logicFormatPtr->vntl2Echo);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E,
            value);

        value = BOOL2BIT_MAC(logicFormatPtr->modifyMacDa);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E,
            value);

        value = BOOL2BIT_MAC(logicFormatPtr->modifyMacSa);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E,
            value);

        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if(hwValuesInfoPtr->routerLttPtr >=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
                    hwValuesInfoPtr->routerLttPtr, PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
            }
            value = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(hwValuesInfoPtr->routerLttPtr);
        }
        else
        {
            rc = prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex(devNum,hwValuesInfoPtr->routerLttPtr,&lpmIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
            value = lpmIndex - PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
        }
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E,
            value);

        break;
    case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
        value = logicFormatPtr->vrfId;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E,
            value);
        break;
    case CPSS_DXCH_TTI_NO_REDIRECT_E:
        /*logicFormatPtr->multiPortGroupTtiEnable --> not applicable for Bobcat2; Caelum; Bobcat3; Aldrin2 ! */

        /* set flow id field*/
        value = logicFormatPtr->flowId;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E,
                                value);

        if (logicFormatPtr->iPclUdbConfigTableEnable == GT_TRUE)
        {
            switch (logicFormatPtr->iPclUdbConfigTableIndex)
            {
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:
                    value = 1;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:
                    value = 2;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:
                    value = 3;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:
                    value = 4;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:
                    value = 5;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:
                    value = 6;
                    break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:
                    value = 7;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            value = 0;
        }

        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E,
                                value);
        break;
    default:
        /* do nothing */
        break;
    }

    if(logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            value = hwValuesInfoPtr->pcl1OverrideConfigIndex;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
                value);

            value = hwValuesInfoPtr->pcl0_1OverrideConfigIndex;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                value);

            value = hwValuesInfoPtr->pcl0OverrideConfigIndex;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E,
                value);
        }
        else
        {
            /* IPCL-0 is bypassed */
            if (hwValuesInfoPtr->pcl1OverrideConfigIndex)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            value = hwValuesInfoPtr->pcl0_1OverrideConfigIndex;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
                value);

            value = hwValuesInfoPtr->pcl0OverrideConfigIndex;
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                value);
        }

        value = logicFormatPtr->iPclConfigIndex;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E,
            value);

        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            /* no such field any more logicFormatPtr->ResetSrcPortGroupId*/
        }
    }

    value = BOOL2BIT_MAC(logicFormatPtr->bindToCentralCounter);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);


    value = logicFormatPtr->centralCounterIndex;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        value);


    value = BOOL2BIT_MAC(logicFormatPtr->bindToPolicerMeter);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        value);


    value = BOOL2BIT_MAC(logicFormatPtr->bindToPolicer);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        value);

    value = logicFormatPtr->policerIndex;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->sourceIdSetEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        value);

    value = logicFormatPtr->sourceId;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->actionStop);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->bridgeBypass);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->ingressPipeBypass);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        value);

    value = hwValuesInfoPtr->tag0VlanPrecedence;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->nestedVlanEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E,
        value);

    value = hwValuesInfoPtr->tag0VlanCmd;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E,
        value);

    value = logicFormatPtr->tag0VlanId;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E,
        value);

    value = hwValuesInfoPtr->tag1VlanCmd;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E,
        value);

    value = logicFormatPtr->tag1VlanId;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E,
        value);

    value = logicFormatPtr->mplsTtl;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->enableDecrementTtl);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E,
        value);

    value = hwValuesInfoPtr->qosPrecedence;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        value);

    value = logicFormatPtr->qosProfile;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
        value);

    value = hwValuesInfoPtr->modifyDSCP;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        value);

    value = hwValuesInfoPtr->modifyTag0Up;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->keepPreviousQoS);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->trustUp);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->trustDscp);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->trustExp);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->remapDSCP);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E,
        value);

    value = logicFormatPtr->tag0Up;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E,
        value);

    value = hwValuesInfoPtr->tag1UpCommand;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E,
        value);

    value = logicFormatPtr->tag1Up;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E,
        value);

    value = hwValuesInfoPtr->ttPassengerPktType;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->copyTtlExpFromTunnelHeader);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->tunnelTerminate);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E,
        value);

    value = hwValuesInfoPtr->mplsCommand;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E,
        value);

    value = logicFormatPtr->hashMaskIndex;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E,
        value);

    /* Source ePort Assignment Enable */
    value = BOOL2BIT_MAC(logicFormatPtr->sourceEPortAssignmentEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E,
        value);

    /* Source ePort: assigned by TTI entry when
      <Source ePort Assignment Enable> = Enabled */
    value = logicFormatPtr->sourceEPort;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->mplsLLspQoSProfileEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E,
        value);

    value = logicFormatPtr->ttHeaderLength;
    /* this is the Tunnel header length in units of Bytes. Granularity is in 2 Bytes. */
    if ((value % 2) != 0)/* length of field is checked by TTI_ACTION_FIELD_SET_MAC(...) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    value = value / 2;

    /* in case of Aldrin or AC3X device, ttHeaderLength can be 6 bit long if RedirectCommand != EgressInterface:
       MSB of ttHeaderLength is stored in the LSB of TTI Reserved field.
       5 LSB's of ttHeaderLength are stored in the 5 bit Tunnel Header Length field.
       Otherwise, it is 5 bit long and all 5 are stored in the Tunnel Header Length field. */
    if (  ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
         && logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E )
    {
        /* TTI Header Length is represented with 6 bits. */
        if ( value > 0x3F )
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        /* save current TTI Reserved field in value2*/
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E,
            value2);

        /* change LSB of the TTI Reserved field according to the MSB of ttHeaderLength. */
        if ( (value & 0x20) != 0 )
        {
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E,
                value2 | 0x1);
        }
        else
        {
            TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E,
                value2 & ~0x1);
        }
        /* the 5 LSB bits will be written to SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E */
        value &= 0x1F;
    }
    else
    {
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* TTI Header Length is represented with 6 bits. */
            if ( value > 0x3F )
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            /* TTI Header Length is represented with 5 bits. */
            if ( value > 0x1F )
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E,
            value);

    value = BOOL2BIT_MAC(logicFormatPtr->qosUseUpAsIndexEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E,
        value);

    if (logicFormatPtr->qosUseUpAsIndexEnable == GT_TRUE)
    {
        if (logicFormatPtr->qosMappingTableIndex >= QOS_MAPPING_TABLE_INDEX_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    value = logicFormatPtr->qosMappingTableIndex;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E,
        value);


    value = BOOL2BIT_MAC(logicFormatPtr->setMacToMe);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->isPtpPacket);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E,
        value);

    if (logicFormatPtr->isPtpPacket == GT_TRUE)
    {
        switch (logicFormatPtr->ptpTriggerType)
        {
        case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E:
            value = 0;
            break;
        case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E:
            value = 1;
            break;
        case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E,
            value);

        value = logicFormatPtr->ptpOffset;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E,
            value);
    }
    else
    {
        value = BOOL2BIT_MAC(logicFormatPtr->oamTimeStampEnable);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E,
            value);

        value = logicFormatPtr->oamOffsetIndex;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E,
            value);
    }

    value = BOOL2BIT_MAC(logicFormatPtr->oamProcessEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        value);

    value = logicFormatPtr->oamProfile;
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->oamChannelTypeToOpcodeMappingEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        value);

    /* The global enable bit is retained for BWC.  If enabled, it overrides the TTI Action */
    rc = cpssDxChPclL3L4ParsingOverMplsEnableGet(devNum,&enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (enablePtr == GT_FALSE)
    {
        switch (logicFormatPtr->passengerParsingOfTransitMplsTunnelMode)
        {
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E:
            value = 0;
            break;
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E:
            value = 1;
            break;
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E:
            value = 2;
            break;
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E,
            value);

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_MPLS_TRANSIT_TUNNEL_PARSING_WA_E))
        {
            /* FEr#4300730: MPLS Transit Tunnel Parsing requires setting the Tunnel Header Length in the TTI Action Entry */
            if ((value != 0) && (logicFormatPtr->tunnelTerminate == GT_FALSE) && (logicFormatPtr->ttHeaderLength == 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    value = BOOL2BIT_MAC(logicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable);

    /* When setting <Passenger Parsing of Non-MPLS Transit tunnel Enable> == 1 and
       <TT Passenger Type> == MPLS --> unexpected behavior */
    if (value && logicFormatPtr->ttPassengerPacketType == CPSS_DXCH_TTI_PASSENGER_MPLS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->rxIsProtectionPath);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->rxProtectionSwitchEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->continueToNextTtiLookup);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->cwBasedPw);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->ttlExpiryVccvEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->pwe3FlowLabelExist);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->pwCwBasedETreeEnable);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E,
        value);

    value = BOOL2BIT_MAC(logicFormatPtr->applyNonDataCwCommand);
    TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E,
        value);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        value = BOOL2BIT_MAC(logicFormatPtr->tunnelHeaderStartL4Enable);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E,
            value);

        value = BOOL2BIT_MAC(logicFormatPtr->skipFdbSaLookupEnable);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E,
            value);

        value = BOOL2BIT_MAC(logicFormatPtr->ipv6SegmentRoutingEndNodeEnable);
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E,
            value);

        value = BOOL2BIT_MAC(logicFormatPtr->exactMatchOverTtiEn);
        value ^= 1;
        TTI_ACTION_FIELD_SET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_TTI_ENABLE_E,
            value);
    }

    return GT_OK;
}

/**
* @internal ttiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiActionType2Logic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    TTI_ACTION_HW_VALUES_STC    hwValues;/* hw values to set to hw */
    GT_STATUS   rc;                             /* return status */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* zero out hw format */
    cpssOsMemSet(hwFormatArray,0,sizeof(GT_U32)*TTI_ACTION_TYPE_2_SIZE_CNS);
    cpssOsMemSet(&hwValues,0,sizeof(hwValues));

    /********************************************************/
    /* check params and translate params to hardware format */
    /********************************************************/

    /* check and convert tt passenger packet type */
    switch (logicFormatPtr->ttPassengerPacketType)
    {
    case CPSS_DXCH_TTI_PASSENGER_IPV4V6_E:
        hwValues.ttPassengerPktType = 0;
        break;
    case CPSS_DXCH_TTI_PASSENGER_MPLS_E:
        hwValues.ttPassengerPktType = 1;
        break;
    case CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E:
        hwValues.ttPassengerPktType = 2;
        break;
    case CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E:
        hwValues.ttPassengerPktType = 3;
        break;
    default:
        /* if not TT then TT Passenger Type is not relevant */
        if(logicFormatPtr->tunnelTerminate == GT_FALSE)
        {
            hwValues.ttPassengerPktType = 0;
            break;
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* in MPLS LSR functionality, tt passenger type must be MPLS */
    if ((logicFormatPtr->tunnelTerminate == GT_FALSE) && (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE))
    {
        if ((logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_POP1_CMD_E ||
            logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_POP2_CMD_E ||
            logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E))
        {
            if (logicFormatPtr->ttPassengerPacketType != CPSS_DXCH_TTI_PASSENGER_MPLS_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* check and convert ts passenger type */
    if (logicFormatPtr->redirectCommand == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E &&
        logicFormatPtr->tunnelStart == GT_TRUE)
    {
        if ((logicFormatPtr->tunnelTerminate == GT_FALSE) &&
            (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E))
        {
            if (logicFormatPtr->tsPassengerPacketType != CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* modifyMacDa == GT_TRUE requires that tunnelStart == GT_FALSE*/
        if (logicFormatPtr->modifyMacDa == GT_TRUE &&
            ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
             (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (logicFormatPtr->tsPassengerPacketType)
        {
        case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
            hwValues.tsPassengerPacketType = 0;
            break;
        case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
            hwValues.tsPassengerPacketType = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* check and convert mpls command */
    switch (logicFormatPtr->mplsCommand)
    {
    case CPSS_DXCH_TTI_MPLS_NOP_CMD_E:
        hwValues.mplsCommand = 0;
        break;
    case CPSS_DXCH_TTI_MPLS_SWAP_CMD_E:
        hwValues.mplsCommand = 1;
        break;
    case CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E:
        hwValues.mplsCommand = 2;
        break;
    case CPSS_DXCH_TTI_MPLS_POP1_CMD_E:
        hwValues.mplsCommand = 3;
        break;
    case CPSS_DXCH_TTI_MPLS_POP2_CMD_E:
        hwValues.mplsCommand = 4;
        break;
    case CPSS_DXCH_TTI_MPLS_POP3_CMD_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* POP3 supported only for eArch */
            hwValues.mplsCommand = 5;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E:
        hwValues.mplsCommand = 7;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        if (logicFormatPtr->mplsTtl > TTL_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate tunnel termination action forwarding command parameter */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValues.pktCommand,logicFormatPtr->command);

    /* translate tunnel termination action redirect command parameter */
    switch (logicFormatPtr->redirectCommand)
    {
    case CPSS_DXCH_TTI_NO_REDIRECT_E:
        hwValues.redirectCommand = 0;
        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
        hwValues.redirectCommand = 1;
        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
        hwValues.redirectCommand = 2;
        if(logicFormatPtr->routerLttPtr >= BIT_15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        hwValues.routerLttPtr = logicFormatPtr->routerLttPtr;
        if (logicFormatPtr->routerLttPtr > (fineTuningPtr->tableSize.tunnelTerm * 4))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        /* FEr#2018: Limited number of Policy-based routes */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH3_LIMITED_NUMBER_OF_POLICY_BASED_ROUTES_WA_E))
        {
            /* xCat devices support index 0,4,8,12(max IP TCAM row * 4) only */
            if ((logicFormatPtr->routerLttPtr % 4) != 0)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        break;
    case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        hwValues.redirectCommand = 4;
        if (logicFormatPtr->vrfId > VRF_ID_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    case CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E:
        /* redirect to logical port is supported only for Lion2-B0 and above; xCat3*/
        if((PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum)) ||
           (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE))
        {
            hwValues.redirectCommand = 5;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(logicFormatPtr->redirectCommand == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        switch (logicFormatPtr->egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            hwValues.hwDev = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            hwValues.hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            if(hwValues.hwPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
            {
                /* 6 bits for the port number */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
         case CPSS_INTERFACE_TRUNK_E:
            if(logicFormatPtr->egressInterface.trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                /* 7 bits for the trunkId */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_INTERFACE_VIDX_E:
            if (logicFormatPtr->egressInterface.vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        case CPSS_INTERFACE_VID_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (logicFormatPtr->tunnelStart == GT_TRUE)
        {
            if (logicFormatPtr->tunnelStartPtr > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TS_PTR_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            if (logicFormatPtr->arpPtr > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_ARP_PTR_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }

    if (logicFormatPtr->sourceIdSetEnable == GT_TRUE)
    {
        if (logicFormatPtr->sourceId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* translate tag 0 VLAN command from action parameter */
    switch (logicFormatPtr->tag0VlanCmd)
    {
    case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
        hwValues.tag0VlanCmd = 0;
        break;
    case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
        hwValues.tag0VlanCmd = 1;
        break;
    case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
        hwValues.tag0VlanCmd = 2;
        break;
    case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
        hwValues.tag0VlanCmd = 3;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, logicFormatPtr->tag0VlanId);

    /* translate tag 1 VLAN command from action parameter */
    switch (logicFormatPtr->tag1VlanCmd)
    {
    case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
        hwValues.tag1VlanCmd = 0;
        break;
    case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
        hwValues.tag1VlanCmd = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* the tag1VlanId  is checked with PRV_CPSS_VLAN_VALUE_CHECK_MAC  and
       not with PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC , because tag1VlanId not
       supporting enhanced vid */
    PRV_CPSS_VLAN_VALUE_CHECK_MAC(logicFormatPtr->tag1VlanId);

    /* translate VLAN precedence from action parameter */
    switch (logicFormatPtr->tag0VlanPrecedence)
    {
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
        hwValues.tag0VlanPrecedence = 1;
        break;
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
        hwValues.tag0VlanPrecedence = 0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->bindToPolicerMeter == GT_TRUE ||
        logicFormatPtr->bindToPolicer == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if (logicFormatPtr->policerIndex >= BIT_16)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            if (logicFormatPtr->policerIndex >= BIT_12)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* translate QoS precedence from action parameter */
    switch (logicFormatPtr->qosPrecedence)
    {
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
        hwValues.qosPrecedence = 1;
        break;
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
        hwValues.qosPrecedence = 0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->keepPreviousQoS == GT_FALSE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if (logicFormatPtr->qosProfile > QOS_PROFILE_E_ARCH_MAX_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            if (logicFormatPtr->qosProfile > QOS_PROFILE_MAX_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    /* translate tunnel termination action modify User Priority parameter */
    switch (logicFormatPtr->modifyTag0Up)
    {
    case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E:
        hwValues.modifyTag0Up = 0;
        break;
    case CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E:
        hwValues.modifyTag0Up = 1;
        break;
    case CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E:
        hwValues.modifyTag0Up = 2;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate tunnel termination tag1 up command parameter */
    switch (logicFormatPtr->tag1UpCommand)
    {
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
        hwValues.tag1UpCommand = 0;
        break;
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
        hwValues.tag1UpCommand = 1;
        break;
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E:
        hwValues.tag1UpCommand = 2;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate tunnel termination action modify DSCP parameter */
    switch (logicFormatPtr->modifyDscp)
    {
    case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
        hwValues.modifyDSCP = 0;
        break;
    case CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E:
        hwValues.modifyDSCP = 1;
        break;
    case CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E:
        hwValues.modifyDSCP = 2;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->tag0Up);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->tag1Up);

    if (logicFormatPtr->hashMaskIndex >= BIT_4 &&
        ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
         (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        switch (logicFormatPtr->pcl0OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            hwValues.pcl0OverrideConfigIndex = 0;
            break;
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            hwValues.pcl0OverrideConfigIndex = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (logicFormatPtr->pcl0_1OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            hwValues.pcl0_1OverrideConfigIndex = 0;
            break;
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            hwValues.pcl0_1OverrideConfigIndex = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (logicFormatPtr->pcl1OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            hwValues.pcl1OverrideConfigIndex = 0;
            break;
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            hwValues.pcl1OverrideConfigIndex = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (logicFormatPtr->iPclConfigIndex > IPCL_INDEX_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate user defined cpu code from action parameter */
    /* note that cpu code is relevant only for trap or mirror commands */
    if ((logicFormatPtr->command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ||
        (logicFormatPtr->command == CPSS_PACKET_CMD_TRAP_TO_CPU_E))
    {
        if ((logicFormatPtr->userDefinedCpuCode >= CPSS_NET_FIRST_USER_DEFINED_E) &&
            (logicFormatPtr->userDefinedCpuCode <= CPSS_NET_LAST_USER_DEFINED_E))
        {
            rc = prvCpssDxChNetIfCpuToDsaCode(logicFormatPtr->userDefinedCpuCode,
                                         (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT*)(GT_UINTPTR)&hwValues.userDefinedCpuCode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
        hwValues.userDefinedCpuCode = 0;

    if (logicFormatPtr->bindToCentralCounter == GT_TRUE)
    {
        if (logicFormatPtr->centralCounterIndex > CENTRAL_COUNTER_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* all common checks done */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* call the Bobcat2; Caelum; Bobcat3; Aldrin2 ; Falcon format */
        return ttiActionSip5Logic2HwFormat(devNum, logicFormatPtr, &hwValues, hwFormatArray);
    }
    else
    {
        /* call the xCat3, Lion2 format */
        return ttiActionXcatLogic2HwFormat(devNum, logicFormatPtr, &hwValues, hwFormatArray);
    }

}

/**
* @internal ttiActionType2Hw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from hardware format to
*         logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] hwFormatArray            - tti action in hardware format (5 words)
*
* @param[out] logicFormatPtr           - points to tti action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS ttiActionType2Hw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_2_STC          *logicFormatPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    tempRouterLttPtr=0;


    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(logicFormatPtr->command, U32_GET_FIELD_MAC(hwFormatArray[0],0,3));

    /* translate user defined cpu code from action parameter */
    /* note that cpu code is relevant only for trap or mirror commands */
    if ((logicFormatPtr->command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ||
        (logicFormatPtr->command == CPSS_PACKET_CMD_TRAP_TO_CPU_E))
    {
        rc = prvCpssDxChNetIfDsaToCpuCode((U32_GET_FIELD_MAC(hwFormatArray[0],3,8)),
                                   &logicFormatPtr->userDefinedCpuCode);
        if(rc != GT_OK)
            return rc;
    }

    logicFormatPtr->mirrorToIngressAnalyzerEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],11,1));

    switch (U32_GET_FIELD_MAC(hwFormatArray[0],12,3))
    {
    case 0:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            logicFormatPtr->multiPortGroupTtiEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],15,1));
        }
        logicFormatPtr->iPclConfigIndex = U32_GET_FIELD_MAC(hwFormatArray[0],31,1) |
                                          (U32_GET_FIELD_MAC(hwFormatArray[1],0,12) << 1);
        break;
    case 1:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        if (U32_GET_FIELD_MAC(hwFormatArray[0],27,1) == 0) /* target is not VIDX */
        {
            if (U32_GET_FIELD_MAC(hwFormatArray[0],15,1) == 0) /* target is not trunk */
            {
                logicFormatPtr->egressInterface.devPort.portNum = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[0],16,6);
                logicFormatPtr->egressInterface.devPort.hwDevNum  = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[0],22,5);
                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(logicFormatPtr->egressInterface));
            }
            else /* target is trunk */
            {
                logicFormatPtr->egressInterface.trunkId = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],16,7);
                logicFormatPtr->egressInterface.type    = CPSS_INTERFACE_TRUNK_E;
            }
        }
        else /* target is VIDX */
        {
            logicFormatPtr->egressInterface.vidx = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[0],15,12);
            if(logicFormatPtr->egressInterface.vidx == 0xFFF)
            {
                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_VID_E;
                logicFormatPtr->egressInterface.vidx = 0;
            }
            else
            {
                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_VIDX_E;
            }
        }

        logicFormatPtr->tunnelStart = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[0],28,1));
        if (logicFormatPtr->tunnelStart == GT_TRUE)
        {
            logicFormatPtr->tunnelStartPtr = U32_GET_FIELD_MAC(hwFormatArray[0],29,3) |
                                             (U32_GET_FIELD_MAC(hwFormatArray[1],0,10) << 3);
            switch (U32_GET_FIELD_MAC(hwFormatArray[1],10,1))
            {
            case 0:
                logicFormatPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
                break;
            case 1:
                logicFormatPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            logicFormatPtr->arpPtr = U32_GET_FIELD_MAC(hwFormatArray[0],29,3) |
                                     (U32_GET_FIELD_MAC(hwFormatArray[1],0,11) << 3);
        }
        logicFormatPtr->vntl2Echo = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[1],11,1));
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))
        {
        logicFormatPtr->modifyMacDa = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],22,1));
        logicFormatPtr->modifyMacSa = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],23,1));
        }
        break;
    case 2:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
        /* LTT entry is row based however in this field the LTT is treated as column based.
          Bits [11:0] indicate row while bits [14:13] indicate column, bit 12 is not used.
          The Formula for translating the LTT entry to column based is as follow:
          [11:0] << 2 + [14:13]   (Note: bit 12 not used). */
        tempRouterLttPtr = U32_GET_FIELD_MAC(hwFormatArray[0],15,15);
        logicFormatPtr->routerLttPtr = (((tempRouterLttPtr & 0xFFF) << 2) |
                                        ((tempRouterLttPtr & 0x6000) >> 13));

        logicFormatPtr->iPclConfigIndex = U32_GET_FIELD_MAC(hwFormatArray[0],31,1) |
                                          (U32_GET_FIELD_MAC(hwFormatArray[1],0,12) << 1);
        break;
    case 4:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
        logicFormatPtr->vrfId           = U32_GET_FIELD_MAC(hwFormatArray[0],15,12);
        logicFormatPtr->iPclConfigIndex = U32_GET_FIELD_MAC(hwFormatArray[0],31,1) |
                                          (U32_GET_FIELD_MAC(hwFormatArray[1],0,12) << 1);
        break;
    case 5:
        /* redirect to logical port is supported only for Lion2-B0 and above; xCat3*/
        if((PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum)) ||
           (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE))
        {
            logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E;
            logicFormatPtr->egressInterface.devPort.portNum = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[0],16,6);
            logicFormatPtr->egressInterface.devPort.hwDevNum  = (GT_U8)U32_GET_FIELD_MAC(hwFormatArray[0],22,5);
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(logicFormatPtr->egressInterface));

            if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_TRUE)
            {
                switch (U32_GET_FIELD_MAC(hwFormatArray[0],27,2))
                {
                    case 0:
                        logicFormatPtr->pwTagMode = CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E;
                        break;
                    case 1:
                        logicFormatPtr->pwTagMode = CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E;
                        break;
                    case 2:
                        logicFormatPtr->pwTagMode = CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if (U32_GET_FIELD_MAC(hwFormatArray[0],29,2))
                {
                    logicFormatPtr->sourceMeshIdSetEnable = GT_TRUE;
                    logicFormatPtr->sourceMeshId = U32_GET_FIELD_MAC(hwFormatArray[0],29,2);
                }
                else
                {
                    logicFormatPtr->sourceMeshIdSetEnable = GT_FALSE;
                    logicFormatPtr->sourceMeshId = 0;
                }

                logicFormatPtr->iPclConfigIndex = U32_GET_FIELD_MAC(hwFormatArray[0],31,1) |
                                              (U32_GET_FIELD_MAC(hwFormatArray[1],0,10) << 1);
            }
            else
            {
                logicFormatPtr->iPclConfigIndex = U32_GET_FIELD_MAC(hwFormatArray[0],31,1) |
                    (U32_GET_FIELD_MAC(hwFormatArray[1],0,12) << 1);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->bindToCentralCounter = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[1],12,1));
     if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (logicFormatPtr->bindToCentralCounter == GT_TRUE)
        {
            logicFormatPtr->centralCounterIndex   = U32_GET_FIELD_MAC(hwFormatArray[1],13,12);
        }
        logicFormatPtr->cwBasedPw = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[1],25,1));
        switch (U32_GET_FIELD_MAC(hwFormatArray[1],26,1)  | (U32_GET_FIELD_MAC(hwFormatArray[4],18,2) << 1))
        {
            case 0:
                logicFormatPtr->unknownSaCommandEnable = GT_TRUE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_FORWARD_E;
                break;
            case 1:
                logicFormatPtr->unknownSaCommandEnable = GT_TRUE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
                break;
            case 2:
                logicFormatPtr->unknownSaCommandEnable = GT_TRUE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                break;
            case 3:
                logicFormatPtr->unknownSaCommandEnable = GT_TRUE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_DROP_HARD_E;
                break;
            case 4:
                logicFormatPtr->unknownSaCommandEnable = GT_TRUE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                break;
            case 7:
                logicFormatPtr->unknownSaCommandEnable = GT_FALSE;
                logicFormatPtr->unknownSaCommand = CPSS_PACKET_CMD_FORWARD_E; /* dummy */
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (logicFormatPtr->bindToCentralCounter == GT_TRUE)
        {
            logicFormatPtr->centralCounterIndex   = U32_GET_FIELD_MAC(hwFormatArray[1],13,14);
        }
    }

    logicFormatPtr->bindToPolicerMeter = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[1],27,1));
    logicFormatPtr->bindToPolicer = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[1],28,1));
    if ((logicFormatPtr->bindToPolicerMeter == GT_TRUE) ||
        (logicFormatPtr->bindToPolicer == GT_TRUE))
    {
        logicFormatPtr->policerIndex = U32_GET_FIELD_MAC(hwFormatArray[1],29,3) |
                                       (U32_GET_FIELD_MAC(hwFormatArray[2],0,9) << 3);
    }

    logicFormatPtr->sourceIdSetEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],9,1));
    if (logicFormatPtr->sourceIdSetEnable == GT_TRUE)
    {
        logicFormatPtr->sourceId   = U32_GET_FIELD_MAC(hwFormatArray[2],10,5);
    }

    logicFormatPtr->actionStop        = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],15,1));
    logicFormatPtr->bridgeBypass      = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],16,1));
    logicFormatPtr->ingressPipeBypass = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],17,1));

    if (logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        switch (U32_GET_FIELD_MAC(hwFormatArray[2],18,1))
        {
            case 0:
                logicFormatPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
                break;
            case 1:
                logicFormatPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch (U32_GET_FIELD_MAC(hwFormatArray[2],19,1))
        {
            case 0:
                logicFormatPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
                break;
            case 1:
                logicFormatPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch (U32_GET_FIELD_MAC(hwFormatArray[2],20,1))
        {
            case 0:
                logicFormatPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
                break;
            case 1:
                logicFormatPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            logicFormatPtr->ResetSrcPortGroupId = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],22,1));
        }
    }

    switch (U32_GET_FIELD_MAC(hwFormatArray[2],21,1))
    {
    case 0:
        logicFormatPtr->tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        break;
    case 1:
        logicFormatPtr->tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->nestedVlanEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[2],22,1));

    switch (U32_GET_FIELD_MAC(hwFormatArray[2],23,2))
    {
    case 0:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
        break;
    case 1:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        break;
    case 2:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E;
        break;
    case 3:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->tag0VlanId = (GT_U16)(U32_GET_FIELD_MAC(hwFormatArray[2],25,7) |
                                          (U32_GET_FIELD_MAC(hwFormatArray[3],0,5) << 7));

    if(logicFormatPtr->egressInterface.type == CPSS_INTERFACE_VID_E)
    {
        logicFormatPtr->egressInterface.vlanId = logicFormatPtr->tag0VlanId;
    }

    switch (U32_GET_FIELD_MAC(hwFormatArray[4],15,3))
    {
    case 0:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
        break;
    case 1:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
        break;
    case 2:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
        break;
    case 3:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
        break;
    case 4:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
        break;
    case 5:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    case 7:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        switch (U32_GET_FIELD_MAC(hwFormatArray[3],5,1))
        {
        case 0:
            logicFormatPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;
        case 1:
            logicFormatPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        logicFormatPtr->tag1VlanId = (GT_U16)U32_GET_FIELD_MAC(hwFormatArray[3],6,12);
    }
    else
    {
        if (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
        {
            logicFormatPtr->mplsTtl = U32_GET_FIELD_MAC(hwFormatArray[3],5,8);
        }
        logicFormatPtr->enableDecrementTtl = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[3],13,1));
    }

    switch (U32_GET_FIELD_MAC(hwFormatArray[3],18,1))
    {
    case 0:
        logicFormatPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        break;
    case 1:
        logicFormatPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->keepPreviousQoS = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[3],30,1));
    if (logicFormatPtr->keepPreviousQoS == GT_FALSE)
    {
        logicFormatPtr->qosProfile   = U32_GET_FIELD_MAC(hwFormatArray[3],19,7);
    }

    switch (U32_GET_FIELD_MAC(hwFormatArray[3],26,2))
    {
    case 0:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
        break;
    case 1:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
        break;
    case 2:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch (U32_GET_FIELD_MAC(hwFormatArray[3],28,2))
    {
    case 0:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
        break;
    case 1:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E;
        break;
    case 2:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->trustUp   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[3],31,1));
    logicFormatPtr->trustDscp = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],0,1));
    logicFormatPtr->trustExp  = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],1,1));
    logicFormatPtr->remapDSCP = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],2,1));
    logicFormatPtr->tag0Up    = U32_GET_FIELD_MAC(hwFormatArray[4],3,3);

    switch (U32_GET_FIELD_MAC(hwFormatArray[4],6,2))
    {
    case 0:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
        break;
    case 1:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
        break;
    case 2:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->tag1Up    = U32_GET_FIELD_MAC(hwFormatArray[4],8,3);

    switch (U32_GET_FIELD_MAC(hwFormatArray[4],11,2))
    {
    case 0:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        break;
    case 1:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_MPLS_E;
        break;
    case 2:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E;
        break;
    case 3:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    logicFormatPtr->copyTtlExpFromTunnelHeader = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],13,1));
    logicFormatPtr->tunnelTerminate         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormatArray[4],14,1));
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        logicFormatPtr->hashMaskIndex           = U32_GET_FIELD_MAC(hwFormatArray[4],18,4);
    }

    return GT_OK;
}

/**
* @internal ttiActionSip5Hw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action (type 2)
*         from hardware format to logic format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwFormatArray            - tti action in hardware format (8 words)
*
* @param[out] logicFormatPtr           - points to tti action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
static GT_STATUS ttiActionSip5Hw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_2_STC          *logicFormatPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    value, value2;
    GT_BOOL   enablePtr;
    GT_U32    leafSwIndex;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E,
        value);

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(logicFormatPtr->command, value);

    /* translate user defined cpu code from action parameter */
    /* note that cpu code is relevant only for trap or mirror commands */
    if ((logicFormatPtr->command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ||
        (logicFormatPtr->command == CPSS_PACKET_CMD_TRAP_TO_CPU_E))
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E,
            value);

        rc = prvCpssDxChNetIfDsaToCpuCode(value,
                                   &logicFormatPtr->userDefinedCpuCode);
        if(rc != GT_OK)
            return rc;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
        value);

    if(value)
    {
        logicFormatPtr->mirrorToIngressAnalyzerEnable = GT_TRUE;
        logicFormatPtr->mirrorToIngressAnalyzerIndex  = value - 1;
    }
    else
    {
        logicFormatPtr->mirrorToIngressAnalyzerEnable = GT_FALSE;
        logicFormatPtr->mirrorToIngressAnalyzerIndex = 0;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
/*  not supported ???
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            logicFormatPtr->multiPortGroupTtiEnable = ???;
        }
*/
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E,
            value);
        logicFormatPtr->flowId = value;

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E,
            value);

        if (value)
        {
            logicFormatPtr->iPclUdbConfigTableEnable = GT_TRUE;
            switch (value)
            {
            case 1:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE_E;
                break;
            case 2:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E;
                break;
            case 3:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E;
                break;
            case 4:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E;
                break;
            case 5:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E;
                break;
            case 6:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
                break;
            case 7:
                logicFormatPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            logicFormatPtr->iPclUdbConfigTableEnable = GT_FALSE;
        }
        break;
    case 1:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
            value);

        if (value == 0) /* target is not VIDX */
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                value);

            if (value == 0) /* target is not trunk */
            {
                TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                    SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E,
                    value);

                logicFormatPtr->egressInterface.devPort.portNum = value;

                TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                    SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E,
                    value);

                logicFormatPtr->egressInterface.devPort.hwDevNum  = value;

                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
                PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(logicFormatPtr->egressInterface));
            }
            else /* target is trunk */
            {
                TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                    SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E,
                    value);

                logicFormatPtr->egressInterface.trunkId = (GT_U16)value;
                logicFormatPtr->egressInterface.type    = CPSS_INTERFACE_TRUNK_E;
            }
        }
        else /* target is VIDX */
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E,
                value);

            logicFormatPtr->egressInterface.vidx = (GT_U16)value;

            if(logicFormatPtr->egressInterface.vidx == 0xFFF)
            {
                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_VID_E;
                logicFormatPtr->egressInterface.vidx = 0;
            }
            else
            {
                logicFormatPtr->egressInterface.type = CPSS_INTERFACE_VIDX_E;
            }
        }

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E,
            value);

        logicFormatPtr->tunnelStart = BIT2BOOL_MAC(value);
        if (logicFormatPtr->tunnelStart == GT_TRUE)
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E,
                value);
            logicFormatPtr->tunnelStartPtr = value;

            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E,
                value);
            switch (value)
            {
            case 0:
                logicFormatPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
                break;
            case 1:
                logicFormatPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E,
                value);
            logicFormatPtr->arpPtr = value;
        }

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E,
            value);
        logicFormatPtr->vntl2Echo = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E,
            value);
        logicFormatPtr->modifyMacDa = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E,
            value);
        logicFormatPtr->modifyMacSa = BIT2BOOL_MAC(value);

        break;
    case 2:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E,
            value);
        /* get the HW index and translate it to SW index that take into consideration
           "holes" that we might have in the LPM RAM*/
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            leafSwIndex = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_HW_INDEX_TO_LEAF_SW_INDEX_MAC(value);
        }
        else
        {
            prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex(devNum,value,&leafSwIndex);
        }
        logicFormatPtr->routerLttPtr = leafSwIndex;

        break;
    case 4:
        logicFormatPtr->redirectCommand = CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E,
            value);
        logicFormatPtr->vrfId           = value;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E,
            value);

        logicFormatPtr->iPclConfigIndex = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        value);
    logicFormatPtr->bindToCentralCounter = BIT2BOOL_MAC(value);
    if (logicFormatPtr->bindToCentralCounter == GT_TRUE)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
            value);
        logicFormatPtr->centralCounterIndex   = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        value);
    logicFormatPtr->bindToPolicerMeter = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        value);
    logicFormatPtr->bindToPolicer = BIT2BOOL_MAC(value);

    if ((logicFormatPtr->bindToPolicerMeter == GT_TRUE) ||
        (logicFormatPtr->bindToPolicer == GT_TRUE))
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
            value);

        logicFormatPtr->policerIndex = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        value);
    logicFormatPtr->sourceIdSetEnable = BIT2BOOL_MAC(value);

    if (logicFormatPtr->sourceIdSetEnable == GT_TRUE)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E,
            value);
        logicFormatPtr->sourceId   = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        value);
    logicFormatPtr->actionStop        = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E,
        value);
    logicFormatPtr->bridgeBypass      = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        value);
    logicFormatPtr->ingressPipeBypass = BIT2BOOL_MAC(value);

    if (logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
            value);

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            switch (value)
            {
            case 0:
                logicFormatPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
                break;
            case 1:
                logicFormatPtr->pcl1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                value);
        }

        switch (value)
        {
        case 0:
            logicFormatPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;
        case 1:
            logicFormatPtr->pcl0_1OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E,
                value);
        }
        else
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                value);
        }
        switch (value)
        {
        case 0:
            logicFormatPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            break;
        case 1:
            logicFormatPtr->pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            /*no such field any more logicFormatPtr->ResetSrcPortGroupId*/
        }
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        break;
    case 1:
        logicFormatPtr->tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E,
        value);
    logicFormatPtr->nestedVlanEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
        break;
    case 1:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        break;
    case 2:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E;
        break;
    case 3:
        logicFormatPtr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E,
        value);

    logicFormatPtr->tag0VlanId = (GT_U16)value;

    if(logicFormatPtr->egressInterface.type == CPSS_INTERFACE_VID_E)
    {
        logicFormatPtr->egressInterface.vlanId = logicFormatPtr->tag0VlanId;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
        break;
    case 1:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
        break;
    case 2:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
        break;
    case 3:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
        break;
    case 4:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
        break;
    case 5:
        /* POP3 supported only for eArch */
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP3_CMD_E;
        break;
    case 7:
        logicFormatPtr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E,
            value);

        switch (value)
        {
        case 0:
            logicFormatPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
            break;
        case 1:
            logicFormatPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E,
            value);
        logicFormatPtr->tag1VlanId = (GT_U16)value;
    }
    else
    {
        if (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
        {
            TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
                SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E,
                value);
            logicFormatPtr->mplsTtl = value;
        }

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E,
            value);
        logicFormatPtr->enableDecrementTtl = BIT2BOOL_MAC(value);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        break;
    case 1:
        logicFormatPtr->qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E,
        value);
    logicFormatPtr->keepPreviousQoS = BIT2BOOL_MAC(value);
    if (logicFormatPtr->keepPreviousQoS == GT_FALSE)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
            value);
        logicFormatPtr->qosProfile   = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
        break;
    case 1:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
        break;
    case 2:
        logicFormatPtr->modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
        break;
    case 1:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E;
        break;
    case 2:
        logicFormatPtr->modifyTag0Up = CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E,
        value);
    logicFormatPtr->trustUp   = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E,
        value);
    logicFormatPtr->trustDscp = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E,
        value);
    logicFormatPtr->trustExp  = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E,
        value);
    logicFormatPtr->remapDSCP = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E,
        value);
    logicFormatPtr->tag0Up    = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
        break;
    case 1:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E;
        break;
    case 2:
        logicFormatPtr->tag1UpCommand = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E,
        value);
    logicFormatPtr->tag1Up    = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        break;
    case 1:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_MPLS_E;
        break;
    case 2:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E;
        break;
    case 3:
        logicFormatPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E,
        value);
    logicFormatPtr->copyTtlExpFromTunnelHeader = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E,
        value);
    logicFormatPtr->tunnelTerminate         = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E,
        value);
    logicFormatPtr->hashMaskIndex           = value;

    /* Source ePort Assignment Enable bit */
    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E,
        value);
    logicFormatPtr->sourceEPortAssignmentEnable = BIT2BOOL_MAC(value);

    /* Source ePort: assigned by TTI entry when
      <Source ePort Assignment Enable> = Enabled */
    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E,
        value);
    logicFormatPtr->sourceEPort = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E,
        value);
    logicFormatPtr->mplsLLspQoSProfileEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E,
        value);

    /* in case of Aldrin or AC3X device, ttHeaderLength can be 6 bit long if RedirectCommand != EgressInterface:
       MSB of ttHeaderLength is stored in the LSB of TTI Reserved field.
       5 LSB's of ttHeaderLength are stored in the 5 bit Tunnel Header Length field.
       Otherwise, it is 5 bit long and all 5 are stored in the Tunnel Header Length field. */
    if (  ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
         && logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E )
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E,
            value2);

        /* if LSB of TTI Reserved field is '1' value is updated accordingly. */
        if ( ( value2 & 0x1 ) == 1 )
            value |= 0x20;
    }

    /* this is the Tunnel header length in units of Bytes. Granularity is in 2 Bytes. */
    value *= 2;
    logicFormatPtr->ttHeaderLength = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E,
        value);
    logicFormatPtr->qosUseUpAsIndexEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E,
        value);
    logicFormatPtr->qosMappingTableIndex = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        value);
    logicFormatPtr->setMacToMe = BIT2BOOL_MAC(value);


    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E,
        value);
    logicFormatPtr->isPtpPacket = BIT2BOOL_MAC(value);

    if (logicFormatPtr->isPtpPacket == GT_TRUE)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E,
            value);

        switch (value)
        {
        case 0:
            logicFormatPtr->ptpTriggerType = CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E;
            break;
        case 1:
            logicFormatPtr->ptpTriggerType = CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E;
            break;
        case 2:
            logicFormatPtr->ptpTriggerType = CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E,
            value);
        logicFormatPtr->ptpOffset = value;
    }
    else
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E,
            value);
        logicFormatPtr->oamTimeStampEnable = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E,
            value);
        logicFormatPtr->oamOffsetIndex = value;
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        value);
    logicFormatPtr->oamProcessEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        value);
    logicFormatPtr->oamProfile = value;

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        value);
    logicFormatPtr->oamChannelTypeToOpcodeMappingEnable = BIT2BOOL_MAC(value);

    /* The global enable bit is retained for BWC.  If enabled, it overrides the TTI Action */
    rc = cpssDxChPclL3L4ParsingOverMplsEnableGet(devNum,&enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (enablePtr == GT_FALSE)
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E,
            value);

        switch (value)
        {
        case 0:
            logicFormatPtr->passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E;
            break;
        case 1:
            logicFormatPtr->passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E;
            break;
        case 2:
            logicFormatPtr->passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E;
            break;
        case 3:
            logicFormatPtr->passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E,
        value);
    logicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E,
        value);
    logicFormatPtr->rxIsProtectionPath = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E,
        value);
    logicFormatPtr->rxProtectionSwitchEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PW_TAG_MODE_E,
        value);

    switch (value)
    {
    case 0:
        logicFormatPtr->pwTagMode = CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E,
        value);
    logicFormatPtr->continueToNextTtiLookup = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E,
        value);
    logicFormatPtr->cwBasedPw = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E,
        value);
    logicFormatPtr->ttlExpiryVccvEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E,
        value);
    logicFormatPtr->pwe3FlowLabelExist = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E,
        value);
    logicFormatPtr->pwCwBasedETreeEnable = BIT2BOOL_MAC(value);

    TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
        SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E,
        value);
    logicFormatPtr->applyNonDataCwCommand = BIT2BOOL_MAC(value);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E,
            value);
        logicFormatPtr->tunnelHeaderStartL4Enable = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E,
            value);
        logicFormatPtr->skipFdbSaLookupEnable = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E,
            value);
        logicFormatPtr->ipv6SegmentRoutingEndNodeEnable = BIT2BOOL_MAC(value);

        TTI_ACTION_FIELD_GET_MAC(devNum,hwFormatArray,
            SIP6_TTI_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_TTI_ENABLE_E,
            value);
        value ^= 1;
        logicFormatPtr->exactMatchOverTtiEn = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}


/**
* @internal ttiActionLogic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] actionType               - type of the action to use
* @param[in] logicFormatPtr           - points to tti action in logic format
*
* @param[out] hwFormatArray            - tti action in hardware format (4 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiActionLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT           *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    switch (actionType)
    {
    case CPSS_DXCH_TTI_ACTION_TYPE2_ENT:
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            return ttiActionType2Logic2HwFormat(devNum,
                                                &logicFormatPtr->type2,
                                                hwFormatArray);
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    default:
        break;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

}

/**
* @internal prvCpssDxChTtiActionHw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from hardware format to
*         logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] hwFormatArray            - tti action in hardware format (4 words)
* @param[in] actionType               - type of the action to use
*
* @param[out] logicFormatPtr           - points to tti action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid tunnel type
*/
GT_STATUS prvCpssDxChTtiActionHw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *logicFormatPtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    cpssOsMemSet(logicFormatPtr, 0, sizeof(*logicFormatPtr));
    switch (actionType)
    {
    case CPSS_DXCH_TTI_ACTION_TYPE2_ENT:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            return ttiActionSip5Hw2LogicFormat(devNum, hwFormatArray,
                                                &logicFormatPtr->type2);
        }
        else
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            return ttiActionType2Hw2LogicFormat(devNum, hwFormatArray,
                                                &logicFormatPtr->type2);
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    default:
        break;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    cpssOsMemSet(hwFormatArray, 0, sizeof(*hwFormatArray));
    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        return ttiActionType2Logic2HwFormat(devNum,
                                            logicFormatPtr,
                                            hwFormatArray);
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal ttiAction2ReducedMask function
* @endinternal
*
* @brief  Converts a given tti action (type 2) from logic format
*         to exact match reduced mask.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] ttiExpandedActionOriginPtr - (pointer to) Whether
*                   to use the action attributes from the Exact
*                   Match rule action or from the profile
* @param[out] exactMatchReducedHwMaskArray  - tti reduced action
*       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS ttiAction2ReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC                           *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiExpandedActionOriginPtr,
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
    CPSS_NULL_PTR_CHECK_MAC(ttiExpandedActionOriginPtr);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchReducedHwMaskArray);

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCommand==GT_TRUE)
    {
        value=0x7; /*bits 0-2*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E,
                                 value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_TRUE)
    {
        value=0xFF; /*bits 3-10*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E,
                                 value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMirrorToIngressAnalyzerIndex==GT_TRUE)
    {
        value=0x7; /*bits 11-13*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
                                 value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_TRUE)
    {
        value=0x7; /*bits 14-16*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E,
                                 value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_TRUE)
    {
        /* bits 17 - target is lag
        * bits 17-32 eVidx
        * bits 18-29 trunk ID
        * bits 18-31 target ePort
        * bits 32-41 target device
        * bit 42 useVidx */

        value=1; /*bits 17*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                                 value);
        value=0x3FFF; /*bits 18-31 target eport */
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E,
                                 value);
        value=0xFFF; /*bits 18-29*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E,
                                 value);
        value=0xFFFF; /*bit 17-32*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E,
                                 value);

        value=0x3FF; /*bits 32-41*/
        TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                 SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E,
                                 value);
         value = 1;/*bit 42*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_TRUE)
    {
         value = 0xFFF;/*bits 17-28*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                     SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E,
                                     value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_TRUE)
    {
         value = 0x1FFF;/*bits 17-29*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_TRUE)
    {
         value = 0x3FFFF;/*bits 17-34*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclUdbConfigTable==GT_TRUE)
    {
         value = 0x7;/*bits 30-32*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_TRUE)
    {
         value = 1;/*bits 43*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_TRUE)
    {
         value = 1;/*bits 44*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionArpPtr==GT_TRUE)
    {
         value = 0x3FFFF;/*bits 45-62*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex==GT_TRUE)
    {
         value = 0xFFFF;/*bits 45-60*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl2OverrideConfigIndex==GT_TRUE)
    {
         value = 1;/*bits 49*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl1OverrideConfigIndex==GT_TRUE)
    {
         value = 1;/*bits 50*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl0OverrideConfigIndex==GT_TRUE)
    {
         value = 1;/*bits 51*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclConfigIndex==GT_TRUE)
    {
         value = 0x1FFF;/*bits 52-64*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_TRUE)
    {
         value = 1;/*bits 61*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacDa==GT_TRUE)
    {
         value = 1;/*bits 63*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacSa==GT_TRUE)
    {
         value = 1;/*bits 64*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToCentralCounter==GT_TRUE)
    {
         value = 1;/*bits 65*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCentralCounterIndex==GT_TRUE)
    {
         value = 0x3FFF;/*bits 66-79*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicerMeter==GT_TRUE)
    {
         value = 1;/*bits 80*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicer==GT_TRUE)
    {
         value = 1;/*bits 81*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_TRUE)
    {
         value = 0x3FFF;/*bits 82-95*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceIdSetEnable==GT_TRUE)
    {
         value = 1;/*bits 96*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceId==GT_TRUE)
    {
         value = 0xFFF;/*bits 97-108*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_TRUE)
    {
         value = 1;/*bits 109 */
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBridgeBypass==GT_TRUE)
    {
         value = 1;/*bits 110 */
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIngressPipeBypass==GT_TRUE)
    {
         value = 1;/*bits 111*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanPrecedence==GT_TRUE)
    {
         value = 1;/*bits 112*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_TRUE)
    {
         value = 1;/*bits 113*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanCmd==GT_TRUE)
    {
         value = 0x7;/*bits 114-116*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanId==GT_TRUE)
    {
         value = 0x1FFF;/*bits 117-129*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanCmd==GT_TRUE)
    {
         value = 1;/*bits 130*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanId==GT_TRUE)
    {
         value = 0xFFF;/*bits 131-142*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_TRUE)
    {
         value = 1;/*bits 143*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosProfile==GT_TRUE)
    {
         value = 0x7F;/*bits 144-150*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyDscp==GT_TRUE)
    {
         value = 0x3;/*bits 151-152*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyTag0==GT_TRUE)
    {
         value = 0x3;/*bits 153-154*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionKeepPreviousQoS==GT_TRUE)
    {
         value = 1;/*bits 155*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustUp==GT_TRUE)
    {
         value = 1;/*bits 156*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustDscp==GT_TRUE)
    {
         value = 1;/*bits 157*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustExp==GT_TRUE)
    {
         value = 1;/*bits 158*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRemapDscp==GT_TRUE)
    {
         value = 1;/*bits 159*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0Up==GT_TRUE)
    {
         value = 0x7;/*bits 160-162*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1UpCommand==GT_TRUE)
    {
         value = 0x3;/*bits 163-164*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1Up==GT_TRUE)
    {
         value = 0x7;/*bits 165-167*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtPassengerPacketType==GT_TRUE)
    {
         value = 0x3;/*bits 168-169*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCopyTtlExpFromTunnelHeader==GT_TRUE)
    {
         value = 1;/*bits 170*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelTerminate==GT_TRUE)
    {
         value = 1;/*bits 171*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsCommand==GT_TRUE)
    {
         value = 0x7;/*bits 172-174*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionHashMaskIndex==GT_TRUE)
    {
         value = 0xF;/*bits 175-178*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosMappingTableIndex==GT_TRUE)
    {
         value = 0xF;/*bits 179-182*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsLLspQoSProfileEnable==GT_TRUE)
    {
         value = 1;/*bits 183*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsTtl==GT_TRUE)
    {
         value = 0xFF;/*bits 184-191*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionEnableDecrementTtl==GT_TRUE)
    {
         value = 1;/*bits 192*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPortAssignmentEnable==GT_TRUE)
    {
         value = 1;/*bits 193*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPort==GT_TRUE)
    {
         value = 0x3FFF;/*bits 194-207*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtHeaderLength==GT_TRUE)
    {
         value = 0x3F;/*bits 208-213*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIsPtpPacket==GT_TRUE)
    {
         value = 1;/*bits 214*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamTimeStampEnable==GT_TRUE)
    {
         value = 1;/*bits 215*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpTriggerType==GT_TRUE)
    {
         value = 0x3;/*bits 215-216*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpOffset==GT_TRUE)
    {
         value = 0x3F;/*bits 217-222*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_TRUE)
    {
         value = 0x7F;/*bits 216-222*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable==GT_TRUE)
    {
         value = 1;/*bits 223*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessEnable==GT_TRUE)
    {
         value = 1;/*bits 224*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCwBasedPw==GT_TRUE)
    {
         value = 1;/*bits 225*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtlExpiryVccvEnable==GT_TRUE)
    {
         value = 1;/*bits 226*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwe3FlowLabelExist==GT_TRUE)
    {
         value = 1;/*bits 227*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwCwBasedETreeEnable==GT_TRUE)
    {
         value = 1;/*bits 228*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosUseUpAsIndexEnable==GT_TRUE)
    {
         value = 1;/*bits 229*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxIsProtectionPath==GT_TRUE)
    {
         value = 1;/*bits 230*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxProtectionSwitchEnable==GT_TRUE)
    {
         value = 1;/*bits 231*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_TRUE)
    {
         value = 1;/*bits 232*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_TRUE)
    {
         value = 1;/*bits 233*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionApplyNonDataCwCommand==GT_TRUE)
    {
         value = 1;/*bits 234*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionContinueToNextTtiLookup==GT_TRUE)
    {
         value = 1;/*bits 237*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable==GT_TRUE)
    {
         value = 1;/*bits 238*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode==GT_TRUE)
    {
         value = 0x3;/*bits 239-240*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,

                                  SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable==GT_TRUE)
    {
         value = 1;/*bits 241*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookupEnable==GT_TRUE)
    {
         value = 1;/*bits 242*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable==GT_TRUE)
    {
         value = 1;/*bits 243*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E,
                                  value);
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelHeaderStartL4Enable==GT_TRUE)
    {
         value = 1;/*bits 244*/
         TTI_ACTION_FIELD_SET_MAC(devNum,exactMatchReducedHwMaskArray,
                                  SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E,
                                  value);
    }

    return rc;
}

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:           Falcon.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] ttiExpandedActionOriginPtr - (pointer to) Whether
*                   to use the action attributes from the Exact
*                   Match rule action or from the profile
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - tti reduced action
*       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC                           *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiExpandedActionOriginPtr,
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
    CPSS_NULL_PTR_CHECK_MAC(ttiExpandedActionOriginPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchReducedHwMaskArray);

    cpssOsMemSet(hwFormatArray, 0, sizeof(*hwFormatArray));
    cpssOsMemSet(exactMatchReducedHwMaskArray, 0, sizeof(*exactMatchReducedHwMaskArray));
    rc = ttiActionType2Logic2HwFormat(devNum,logicFormatPtr,hwFormatArray);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = ttiAction2ReducedMask(devNum, logicFormatPtr, ttiExpandedActionOriginPtr, exactMatchReducedHwMaskArray);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

/**
* @internal prvCpssDxChTtiActionType2Hw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from hardware
*          format to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum          - device number
* @param[in] hwFormatArray   - tti action in hardware format
*                              (5 words)
*
* @param[out] logicFormatPtr - points to tti action in logic
*                              format
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Hw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                             *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwFormatArray);

    cpssOsMemSet(logicFormatPtr, 0, sizeof(*logicFormatPtr));
    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        return ttiActionSip5Hw2LogicFormat(devNum,
                                            hwFormatArray,
                                            logicFormatPtr);
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal ttiGetIndexForKeyType function
* @endinternal
*
* @brief   Gets the index used in the HW for a specific key type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] keyType                  - the key type
* @param[in] isLegacyKeyTypeValid     - GT_TRUE: a legacy keyType is valid
*                                      GT_FALSE: a legacy keyType is not valid
*
* @param[out] indexPtr                 - (pointer to) the index of the key type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong key type
*/
GT_STATUS ttiGetIndexForKeyType
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             isLegacyKeyTypeValid,
    OUT GT_U32                              *indexPtr
)
{
    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
        *indexPtr = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
        *indexPtr = 1;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
        *indexPtr = 2;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
        *indexPtr = 3;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
        *indexPtr = 4;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
        *indexPtr = 5;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
        *indexPtr = 6;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
        *indexPtr = 7;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
        *indexPtr = 8;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
        *indexPtr = 9;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
        *indexPtr = 10;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
        *indexPtr = 11;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
        *indexPtr = 12;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
        *indexPtr = 13;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
        *indexPtr = 14;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        *indexPtr = 15;
        break;
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        if (isLegacyKeyTypeValid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *indexPtr = 16;
        break;
    case CPSS_DXCH_TTI_KEY_MPLS_E:
        if (isLegacyKeyTypeValid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *indexPtr = 17;
        break;
    case CPSS_DXCH_TTI_KEY_MIM_E:
        if (isLegacyKeyTypeValid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *indexPtr = 18;
        break;
    case CPSS_DXCH_TTI_KEY_ETH_E:
        if (isLegacyKeyTypeValid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *indexPtr = 19;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal ttiInitUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offset                   - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type. .(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS ttiInitUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U32                               offset
)
{
    GT_STATUS rc;              /* return code              */
    GT_U32    regValue;        /* value to update register */
    GT_U32    startBit;        /* start Bit                */
    GT_U32    udbEntryIndex;   /* HW UDB entry index       */
    GT_U32    portGroupId;     /*the port group Id         */
    GT_U32    maxNumMetaDataBytes = NUM_OF_META_DATA_BYTES_MAC(devNum);/* max number of bytes of metadata  */
    GT_U32    numBits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    numBits = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? BIT_8 : BIT_7;
    if ((offset >= numBits) || (udbIndex >= UDB_CONFIG_NUM_WORDS_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(offsetType == CPSS_DXCH_TTI_OFFSET_METADATA_E && offset > maxNumMetaDataBytes)
    {
        /* the metadata not supports more then 32 bytes of info */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = ttiGetIndexForKeyType(keyType, GT_TRUE, &udbEntryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    regValue = (offset << 4);

    /* bit 0 means "valid" */
    switch (offsetType)
    {
        case CPSS_DXCH_TTI_OFFSET_L2_E:
            regValue |= (UDB_HW_L2_CNS << 1) | 1;
            break;
        case CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E:
            regValue |= (UDB_HW_L3_CNS << 1) | 1;
            break;
        case CPSS_DXCH_TTI_OFFSET_L4_E:
            regValue |= (UDB_HW_L4_CNS << 1) | 1;
            break;
        case CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E:
            regValue |= (UDB_HW_MPLS_CNS << 1) | 1;
            break;
        case CPSS_DXCH_TTI_OFFSET_METADATA_E:
            regValue |= (UDB_HW_METADATA_CNS << 1) | 1;
            break;
        case CPSS_DXCH_TTI_OFFSET_INVALID_E:
            /* L2 without valid bit*/
            regValue |= (UDB_HW_L2_CNS << 1);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numBits = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 12 : 11;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        startBit  =  (numBits * udbIndex);
        /* Update the needed 11 bits in the packet type related 23-UDB entry */
        rc =  prvCpssDxChPortGroupWriteTableEntryField(
            devNum, portGroupId,
            CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E,
            udbEntryIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            startBit,
            numBits,
            regValue);
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
* @internal ttiInitUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] keyType                  - key Type
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
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
static GT_STATUS ttiInitUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U32                              *offsetPtr
)
{
    GT_STATUS rc;               /* return code              */
    GT_U32    regValue;         /* value to update register */
    GT_U32    startBit;         /* start Bit                */
    GT_U32    udbEntryIndex;    /* HW UDB entry index       */
    GT_U32    portGroupId;      /*the port group Id         */
    GT_U32    udbEntryArray[UDB_CONFIG_NUM_WORDS_MAX_CNS]; /* HW UDB entry */
    GT_U32    numBits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(offsetTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (udbIndex >= UDB_CONFIG_NUM_WORDS_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = ttiGetIndexForKeyType(keyType, GT_TRUE, &udbEntryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read all packet type related 30-UDB entry */
    rc = prvCpssDxChPortGroupReadTableEntry(
        devNum, portGroupId,
        CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E,
        udbEntryIndex, udbEntryArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    numBits = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 12 : 11;
    startBit = (numBits * udbIndex);
    /* get the needed bits */
    U32_GET_FIELD_IN_ENTRY_MAC(udbEntryArray,startBit,numBits,regValue);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *offsetPtr = ((regValue >> 4) & 0xFF);
    }
    else
    {
        *offsetPtr = ((regValue >> 4) & 0x7F);
    }

    if ((regValue & 1) == 0)
    {
        /* invalid UDB */
        *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_INVALID_E;
    }
    else
    {
        switch ((regValue >> 1) & 7)
        {
         case UDB_HW_L2_CNS:
             *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_L2_E;
             break;
         case UDB_HW_L3_CNS:
             *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E;
             break;
         case UDB_HW_L4_CNS:
             *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_L4_E;
             break;
         case UDB_HW_MPLS_CNS:
             *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E;
             break;
         case UDB_HW_METADATA_CNS:
             *offsetTypePtr = CPSS_DXCH_TTI_OFFSET_METADATA_E;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal ttiKeyLookupBitLengthGet function
* @endinternal
*
* @brief   This function gets field name and sub bit offset for the specified key
*         type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] keyType                  - TTI key type
*
* @param[out] fieldNamePtr             - (pointer to) field name (type) in table
* @param[out] subFieldOffsetPtr        - (pointer to) sub field offset within the (primary) field.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ttiKeyLookupBitLengthGet
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT *fieldNamePtr,
    OUT GT_U32                              *subFieldOffsetPtr
)
{
     /* check parameters */
    GT_U32 bit;
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT fieldName;

    CPSS_NULL_PTR_CHECK_MAC(fieldNamePtr);
    CPSS_NULL_PTR_CHECK_MAC(subFieldOffsetPtr);

    bit = 0;/* by default start from bit 0 of the primary bit */

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        fieldName = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN_E;
        break;
    case CPSS_DXCH_TTI_KEY_MPLS_E:
        fieldName = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN_E;
        break;
    case CPSS_DXCH_TTI_KEY_ETH_E:
        fieldName = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN_E;
        break;
    case CPSS_DXCH_TTI_KEY_MIM_E:
        fieldName = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP_E;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        fieldName = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE_E;
        bit = keyType - CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *subFieldOffsetPtr = bit;
    *fieldNamePtr = fieldName;

    return GT_OK;
}

/**
* @internal internal_cpssDxChTtiMacToMeSet function
* @endinternal
*
* @brief   function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    return cpssDxChTtiPortGroupMacToMeSet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,entryIndex,valuePtr,maskPtr,interfaceInfoPtr);
}

/**
* @internal cpssDxChTtiMacToMeSet function
* @endinternal
*
* @brief   function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMacToMeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));

    rc = internal_cpssDxChTtiMacToMeSet(devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    return cpssDxChTtiPortGroupMacToMeGet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,entryIndex,valuePtr,maskPtr,interfaceInfoPtr);
}

/**
* @internal cpssDxChTtiMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMacToMeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));

    rc = internal_cpssDxChTtiMacToMeGet(devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
* @param[in] enable                   - GT_TRUE:  TTI lookup
*                                      GT_FALSE: disable TTI lookup
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             enable
)
{

    GT_U32      value;      /* value to write */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc = GT_OK; /* function return code */
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT fieldName;
    GT_U32      subFiledOffset; /* the sub field offset */

     /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        rc = ttiKeyLookupBitLengthGet(keyType, &fieldName, &subFiledOffset);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS,/* use of sub field ! */
                                            fieldName,   /* field name */
                                            PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(subFiledOffset,1),/* sub field info {offset,length}*/
                                            value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* DxCh3 */
        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            switch (keyType)
            {
            case CPSS_DXCH_TTI_KEY_IPV4_E:
                bit = 49;
                break;
            case CPSS_DXCH_TTI_KEY_MPLS_E:
                bit = 50;
                break;
            case CPSS_DXCH_TTI_KEY_ETH_E:
                bit = 51;
                break;
            default:
                if ((keyType == CPSS_DXCH_TTI_KEY_MIM_E) && (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))) /* xCat A0 */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                else
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else  /* xCat3; Lion2 */
        {
            switch (keyType)
            {
            case CPSS_DXCH_TTI_KEY_IPV4_E:
                bit = 51;
                break;
            case CPSS_DXCH_TTI_KEY_MPLS_E:
                bit = 55;
                break;
            case CPSS_DXCH_TTI_KEY_ETH_E:
                bit = 57;
                break;
            case CPSS_DXCH_TTI_KEY_MIM_E:
                bit = 60;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /* write value to hardware table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,/* start bit */
                                             1,             /* length */
                                             value);
    }

    return rc;

}

/**
* @internal cpssDxChTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
* @param[in] enable                   - GT_TRUE:  TTI lookup
*                                      GT_FALSE: disable TTI lookup
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortLookupEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, keyType, enable));

    rc = internal_cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, keyType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
*
* @param[out] enablePtr                - points to enable/disable TTI lookup
*                                      GT_TRUE: TTI lookup is enabled
*                                      GT_FALSE: TTI lookup is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id, port or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc = GT_OK; /* function return code */
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT fieldName;
    GT_U32      subFiledOffset; /* the sub field offset */

     /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        rc = ttiKeyLookupBitLengthGet(keyType, &fieldName, &subFiledOffset);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS,/* use of sub field ! */
                                            fieldName,   /* field name */
                                            PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(subFiledOffset,1),/* sub field info {offset,length}*/
                                            &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* DxCh3 */
        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            switch (keyType)
            {
            case CPSS_DXCH_TTI_KEY_IPV4_E:
                bit = 49;
                break;
            case CPSS_DXCH_TTI_KEY_MPLS_E:
                bit = 50;
                break;
            case CPSS_DXCH_TTI_KEY_ETH_E:
                bit = 51;
                break;
            default:
                if ((keyType == CPSS_DXCH_TTI_KEY_MIM_E) && (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))) /* xCat A0 */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                else
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            switch (keyType)
            {
            case CPSS_DXCH_TTI_KEY_IPV4_E:
                bit = 51;
                break;
            case CPSS_DXCH_TTI_KEY_MPLS_E:
                bit = 55;
                break;
            case CPSS_DXCH_TTI_KEY_ETH_E:
                bit = 57;
                break;
            case CPSS_DXCH_TTI_KEY_MIM_E:
                bit = 60;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /* read value from hardware table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,     /* offset */
                                             1,             /* length */
                                             &value);
    }
    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
*
* @param[out] enablePtr                - points to enable/disable TTI lookup
*                                      GT_TRUE: TTI lookup is enabled
*                                      GT_FALSE: TTI lookup is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id, port or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortLookupEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, keyType, enablePtr));

    rc = internal_cpssDxChTtiPortLookupEnableGet(devNum, portNum, keyType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, keyType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for tunneled packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc=GT_OK;   /* function return code */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 54;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 52;
        }

        /* write value to hardware table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,    /* offset */
                                             1,             /* length */
                                             value);
    }

    return rc;
}

/**
* @internal cpssDxChTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for tunneled packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpv4OnlyTunneledEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortIpv4OnlyTunneledEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortIpv4OnlyTunneledEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only tunneled packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      tunneled packets
*                                      GT_TRUE: IPv4 TTI lookup only for tunneled packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for tunneled packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc=GT_OK;   /* function return code */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);

    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 54;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 52;
        }

        /* read hardware value */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bit,/* start bit */
                                            1,              /* length */
                                            &value);
    }

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortIpv4OnlyTunneledEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only tunneled packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      tunneled packets
*                                      GT_TRUE: IPv4 TTI lookup only for tunneled packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for tunneled packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpv4OnlyTunneledEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortIpv4OnlyTunneledEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc=GT_OK;   /* function return code */

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 53;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 54;
        }

        /* write value to hardware table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,/* start bit */
                                             1,             /* length */
                                             value);
    }
    return rc;
}

/**
* @internal cpssDxChTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpv4OnlyMacToMeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: IPv4 TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc=GT_OK;   /* function return code */

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);   
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 53;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 54;
        }

        /* read hardware value */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bit,/* start bit */
                                            1,              /* length */
                                            &value);
    }
    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortIpv4OnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: IPv4 TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpv4OnlyMacToMeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortIpv4OnlyMacToMeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTtiIpv4McEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for IPv4 multicast
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  TTI lookup for IPv4 MC
*                                      GT_FALSE: disable TTI lookup for IPv4 MC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiIpv4McEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{

    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    /* write value */
    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,23,1,value);
    }
    else /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIEngineConfig;
            rc = prvCpssHwPpSetRegField(devNum,regAddr,2,1,value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiEngineConfig;
            rc = prvCpssHwPpSetRegField(devNum,regAddr,5,1,value);
        }
    }

    return rc;


}

/**
* @internal cpssDxChTtiIpv4McEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for IPv4 multicast
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  TTI lookup for IPv4 MC
*                                      GT_FALSE: disable TTI lookup for IPv4 MC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiIpv4McEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIpv4McEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTtiIpv4McEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiIpv4McEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of TTI lookup for
*         IPv4 multicast (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable TTI lookup for IPv4 MC
*                                      GT_TRUE: TTI lookup for IPv4 MC enabled
*                                      GT_FALSE: TTI lookup for IPv4 MC disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiIpv4McEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      value = 0;  /* hardware write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* read value */
    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,23,1,&value);
    }
    else /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIEngineConfig;
            rc = prvCpssHwPpGetRegField(devNum,regAddr,2,1,&value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiEngineConfig;
            rc = prvCpssHwPpGetRegField(devNum,regAddr,5,1,&value);
        }
    }

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiIpv4McEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of TTI lookup for
*         IPv4 multicast (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable TTI lookup for IPv4 MC
*                                      GT_TRUE: TTI lookup for IPv4 MC enabled
*                                      GT_FALSE: TTI lookup for IPv4 MC disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiIpv4McEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIpv4McEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTtiIpv4McEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortMplsOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MPLS TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc;         /* function return code */

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 52;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 56;
        }

        /* write value to hardware table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,/* start bit */
                                             1,             /* length */
                                             value);
    }

    return rc;
}

/**
* @internal cpssDxChTtiPortMplsOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MPLS TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortMplsOnlyMacToMeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortMplsOnlyMacToMeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortMplsOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MPLS TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: MPLS TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MPLS TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortMplsOnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_U32      bit;        /* bit in table   */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            bit = 52;
        }
        else  /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            bit = 56;
        }

        /* read hardware value */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bit,/* start bit */
                                            1,              /* length */
                                            &value);
    }

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortMplsOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MPLS TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: MPLS TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MPLS TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortMplsOnlyMacToMeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortMplsOnlyMacToMeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MIM TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:   MIM TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      bit = 0;    /* bit in table   */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        bit = 61;

        /* write value to hardware table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             bit,/* start bit */
                                             1,             /* length */
                                             value);
    }

    return rc;
}

/**
* @internal cpssDxChTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MIM TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:   MIM TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortMimOnlyMacToMeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortMimOnlyMacToMeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortMimOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MIM TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable MIM TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE:  MIM TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MIM TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      value=0;    /* hardware value */
    GT_U32      bit = 0;    /* bit in table   */
    GT_STATUS   rc=GT_OK;   /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        bit = 61;

        /* read hardware value */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bit,/* start bit */
                                            1,              /* length */
                                            &value);
    }

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortMimOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MIM TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable MIM TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE:  MIM TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MIM TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortMimOnlyMacToMeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortMimOnlyMacToMeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    return cpssDxChTtiPortGroupRuleSet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
        ruleType,patternPtr,maskPtr,actionType,actionPtr);
}

/**
* @internal cpssDxChTtiRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiRuleSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));

    rc = internal_cpssDxChTtiRuleSet(devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    return cpssDxChTtiPortGroupRuleGet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,ruleType,
        patternPtr,maskPtr,actionType,actionPtr);
}

/**
* @internal cpssDxChTtiRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiRuleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));

    rc = internal_cpssDxChTtiRuleGet(devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    return cpssDxChTtiPortGroupRuleActionUpdate(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
        actionType,actionPtr);
}

/**
* @internal cpssDxChTtiRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiRuleActionUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, actionType, actionPtr));

    rc = internal_cpssDxChTtiRuleActionUpdate(devNum, index, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
static GT_STATUS internal_cpssDxChTtiRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    return cpssDxChTtiPortGroupRuleValidStatusSet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,valid);
}

/**
* @internal cpssDxChTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS cpssDxChTtiRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiRuleValidStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, valid));

    rc = internal_cpssDxChTtiRuleValidStatusSet(devNum, index, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
)
{
    return cpssDxChTtiPortGroupRuleValidStatusGet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,validPtr);
}

/**
* @internal cpssDxChTtiRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiRuleValidStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, validPtr));

    rc = internal_cpssDxChTtiRuleValidStatusGet(devNum, index, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, validPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
static GT_STATUS internal_cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
{
    return cpssDxChTtiPortGroupMacModeSet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,keyType,macMode);
}

/**
* @internal cpssDxChTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMacModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, macMode));

    rc = internal_cpssDxChTtiMacModeSet(devNum, keyType, macMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, macMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
static GT_STATUS internal_cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
{
    return cpssDxChTtiPortGroupMacModeGet(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,keyType,macModePtr);
}

/**
* @internal cpssDxChTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMacModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, macModePtr));

    rc = internal_cpssDxChTtiMacModeGet(devNum, keyType, macModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, macModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
static GT_STATUS internal_cpssDxChTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      offset;         /* offset in register */
    GT_STATUS   rc;             /* function return code */
    GT_U32      entryIndex;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(pclId >= BIT_10)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 0;
        break;
    case CPSS_DXCH_TTI_KEY_MPLS_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 10;
        break;
    case CPSS_DXCH_TTI_KEY_ETH_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 20;
        break;
    case CPSS_DXCH_TTI_KEY_MIM_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig1;
        }
        offset  = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 0;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 1;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 2;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 3;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 4;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 5;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 6;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 7;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 8;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 9;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 10;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 11;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 12;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 13;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 14;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 15;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write value */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,10,pclId);

    return rc;
}

/**
* @internal cpssDxChTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPclIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, pclId));

    rc = internal_cpssDxChTtiPclIdSet(devNum, keyType, pclId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, pclId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
static GT_STATUS internal_cpssDxChTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in register */
    GT_STATUS   rc;         /* function return code */
    GT_U32      entryIndex;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pclIdPtr);

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_IPV4_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 0;
        break;
    case CPSS_DXCH_TTI_KEY_MPLS_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 10;
        break;
    case CPSS_DXCH_TTI_KEY_ETH_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig0;
        }
        offset  = 20;
        break;
    case CPSS_DXCH_TTI_KEY_MIM_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPCLIDConfig1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.pclIdConfig1;
        }
        offset  = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 0;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 1;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 2;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 3;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 4;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 5;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 6;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 7;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 8;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 9;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 10;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 11;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 12;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 13;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 14;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 0;
        break;
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        entryIndex = 15;
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];
        offset = 16;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read value */
    rc = prvCpssHwPpGetRegField(devNum,regAddr,offset,10,pclIdPtr);

    return rc;
}

/**
* @internal cpssDxChTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPclIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, pclIdPtr));

    rc = internal_cpssDxChTtiPclIdGet(devNum, keyType, pclIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, pclIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ttiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command register address, offset and legnth.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
*
* @param[out] regAddressPtr            - (pointer to) register address
* @param[out] offsetPtr                - (pointer to) offset in the register
* @param[out] lenPtr                   - (pointer to) length in the register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ttiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT GT_U32                              *regAddressPtr,
    OUT GT_U32                              *offsetPtr,
    OUT GT_U32                              *lenPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        *lenPtr = 3;
    }
    else
    {
        *lenPtr = 1;
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* find register offset according to the exception type */
        switch (exceptionType)
        {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
            *offsetPtr = 0;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:
            *offsetPtr = 3;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:
            *offsetPtr = 9;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:
            *offsetPtr = 12;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E:
            *offsetPtr = 15;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E:
            *offsetPtr = 18;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E:
            *offsetPtr = 21;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E:
            *offsetPtr = 24;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E:
            *offsetPtr = 27;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:
            *offsetPtr = 0;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupMPLSExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:
            *offsetPtr = 3;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupMPLSExceptionCommands;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E:
            *offsetPtr = 0;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E:
            *offsetPtr = 3;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E:
            *offsetPtr = 6;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E:
            *offsetPtr = 9;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E:
            *offsetPtr = 12;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E:
            *offsetPtr = 15;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E:
            *offsetPtr = 18;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E:
            *offsetPtr = 21;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E:
            *offsetPtr = 24;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E:
            *offsetPtr = 27;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E:
            *offsetPtr = 0;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands1;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E:
            *offsetPtr = 3;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands1;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E:
            *offsetPtr = 6;
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLExceptionCommands1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* DxCh3 */
    else if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* find register offset according to the exception type */
        switch (exceptionType)
        {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
            *offsetPtr = 19;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:
            *offsetPtr = 20;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:
            *offsetPtr = 21;
            break;
        default:
            if (((exceptionType == CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E) ||
                (exceptionType == CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E)) &&
                (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        *regAddressPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
    }
    else /* xCat3; Lion2 */
    {
        /* find register offset according to the exception type */
        switch (exceptionType)
        {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
            *offsetPtr = 4;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:
            *offsetPtr = 9;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:
            *offsetPtr = 6;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:
            *offsetPtr = 7;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:
            *offsetPtr = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        *regAddressPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiEngineConfig;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
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
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static GT_STATUS internal_cpssDxChTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
)
{
    GT_STATUS   rc = GT_OK; /* function return code */
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in the register */
    GT_U32      len;        /* length */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* note the commands for all exception types are the same  */
    /* get the value to write according to command             */
    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:
            switch (command)
            {
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
                    {
                        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);
                    }
                    else
                    {
                        value = 0;
                    }
                    break;
                case CPSS_PACKET_CMD_DROP_HARD_E:
                    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
                    {
                        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);
                    }
                    else
                    {
                        value = 1;
                    }
                    break;
                default:
                    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
                    {
                        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
            }
            break;

        case CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E:
        case CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = ttiExceptionCmdGet(devNum,exceptionType,&regAddr,&offset,&len);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write command value */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,len,value);

    return rc;
}

/**
* @internal cpssDxChTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
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
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiExceptionCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, command));

    rc = internal_cpssDxChTtiExceptionCmdSet(devNum, exceptionType, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
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
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static GT_STATUS internal_cpssDxChTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_STATUS   rc;         /* function return code */
    GT_U32      value;      /* register value */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in the register */
    GT_U32      len;        /* length */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    rc = ttiExceptionCmdGet(devNum,exceptionType,&regAddr,&offset,&len);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* read command value */
    rc = prvCpssHwPpGetRegField(devNum,regAddr,offset,len,&value);
    if (rc != GT_OK)
        return rc;

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E:
        case CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC((*commandPtr),value);
            }
            else
            {
                switch (value)
                {
                case 0:
                    *commandPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                    break;
                case 1:
                    *commandPtr = CPSS_PACKET_CMD_DROP_HARD_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
            break;

        default:
            PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr, value);
    }

    return rc;
}

/**
* @internal cpssDxChTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
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
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiExceptionCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, commandPtr));

    rc = internal_cpssDxChTtiExceptionCmdGet(devNum, exceptionType, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ttiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code register address and offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
*
* @param[out] regAddressPtr            - (pointer to) register address
* @param[out] offsetPtr                - (pointer to) offset in the register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ttiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT GT_U32                              *regAddressPtr,
    OUT GT_U32                              *offsetPtr
)
{
    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(regAddressPtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes0;
            *offsetPtr = 0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes0;
            *offsetPtr = 8;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes0;
            *offsetPtr = 16;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes0;
            *offsetPtr = 24;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes1;
            *offsetPtr = 0;
            break;
        case CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E:
            *regAddressPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTILookupIPExceptionCodes1;
            *offsetPtr = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
* @param[in] code                     - the  for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            code
)
{
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_U32 regAddr, offset;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = ttiExceptionCpuCodeGet(devNum, exceptionType, &regAddr, &offset);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChNetIfCpuToDsaCode(code, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 8, (GT_U32)dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
* @param[in] code                     - the  for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            code
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, code));

    rc = internal_cpssDxChTtiExceptionCpuCodeSet(devNum, exceptionType, code);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, code));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to get CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*
* @param[out] codePtr                  - (points to) the code for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *codePtr
)
{
    GT_U32 regAddr, offset, value;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(codePtr);

    rc = ttiExceptionCpuCodeGet(devNum, exceptionType, &regAddr, &offset);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 8, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)value,
                                      codePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to get CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*
* @param[out] codePtr                  - (points to) the code for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *codePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, codePtr));

    rc = internal_cpssDxChTtiExceptionCpuCodeGet(devNum, exceptionType, codePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, codePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function globally enables/disables bypassing IPv4 header length criteria checks as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:   bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
)
{
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | 
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* check revision */
    if (GT_FALSE == PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

     /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    /* write value to hardware register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiMetalFix;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, value);

    return rc;
}

/**
* @internal cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function globally enables/disables bypassing IPv4 header length criteria checks as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:   bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function gets the globally bypassing IPv4 header length criteria check as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable bypass header length check
*                                      GT_TRUE:  enable bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | 
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check revision */
   if (GT_FALSE == PRV_CPSS_DXCH_XCAT_B1_ONLY_CHECK_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiMetalFix;
    rc = prvCpssHwPpGetRegField(devNum,regAddr,18,1,&value);

    if (rc != GT_OK)
        return rc;

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function gets the globally bypassing IPv4 header length criteria check as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable bypass header length check
*                                      GT_TRUE:  enable bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
static GT_STATUS internal_cpssDxChTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32    hwData;      /* value to write */

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    /* set Pre-TTI-Lookup-Ingress-ePort table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
}

/**
* @internal cpssDxChTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpTotalLengthDeductionEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortIpTotalLengthDeductionEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
static GT_STATUS internal_cpssDxChTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData;

     /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* read from Pre-TTI-Lookup-Ingress-ePort table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortIpTotalLengthDeductionEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortIpTotalLengthDeductionEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
* @param[in] value                    - IPv4 or IPv6 Total Length Deduction Value .(APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
static GT_STATUS internal_cpssDxChTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
)
{
    GT_STATUS rc=GT_OK;    /* function return code */
    GT_U32    regAddr;     /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(value >= BIT_6)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPLengthChecks;

    switch (ipType)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,6,value);
        break;

    case CPSS_IP_PROTOCOL_IPV6_E:
        rc = prvCpssHwPpSetRegField(devNum,regAddr,6,6,value);
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
* @param[in] value                    - IPv4 or IPv6 Total Length Deduction Value .(APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIpTotalLengthDeductionValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ipType, value));

    rc = internal_cpssDxChTtiIpTotalLengthDeductionValueSet(devNum, ipType, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ipType, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData;
    GT_U32    regAddr;    /* register address */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPLengthChecks;

    switch (ipType)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,6,&hwData);
        break;

    case CPSS_IP_PROTOCOL_IPV6_E:
        rc = prvCpssHwPpGetRegField(devNum,regAddr,6,6,&hwData);
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(rc!=GT_OK)
        return rc;

    *valuePtr = hwData;

    return rc;
}

/**
* @internal cpssDxChTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIpTotalLengthDeductionValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ipType, valuePtr));

    rc = internal_cpssDxChTtiIpTotalLengthDeductionValueGet(devNum, ipType, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ipType, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
static GT_STATUS internal_cpssDxChTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              ethertype
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in register */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    
    if (ethertype >= BIT_16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch (ethertypeType)
    {
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E)
            offset = 0;
        else
            offset = 16;
        /* DxCh3 */
        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* write value */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vntReg.vntGreEtherTypeConfReg;
            rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);
        }
        else /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            /* write value */
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIIPv4GREEthertype;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiIpv4GreEthertype;
            }
            rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);
        }
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE1_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E)
            offset = 0;
        else
            offset = 16;
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIIPv6GREEtherType;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E)
            offset = 0;
        else
            offset = 16;
        /* write value */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MPLSEtherTypes;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.mplsEthertypes;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);
        if(rc != GT_OK)
            return rc;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).MPLSEtherType;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltMplsEthertypes;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* write value */
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).specialEtherTypes;
            rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,ethertype);
            if(rc != GT_OK)
                return rc;
        }
        else
        {
            /* write value */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.specialEthertypes;
            rc = prvCpssHwPpSetRegField(devNum,regAddr,16,16,ethertype);
            if(rc != GT_OK)
                return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).IEtherType;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltIEthertype;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,ethertype);
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* write value */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,ethertype);
        if(rc != GT_OK)
            return rc;

        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).TRILLEtherType;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,ethertype);

        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[0];
        }
        else if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[1];
        }
        else if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[2];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[3];
        }

        if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E))
        {
            offset = 0;
        }
        else
        {
            offset = 16;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,16,ethertype);

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              ethertype
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEthernetTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ethertypeType, ethertype));

    rc = internal_cpssDxChTtiEthernetTypeSet(devNum, ethertypeType, ethertype);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ethertypeType, ethertype));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
static GT_STATUS internal_cpssDxChTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    OUT GT_U32                              *ethertypePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in register */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ethertypePtr);

    switch (ethertypeType)
    {
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E)
            offset = 0;
        else
            offset = 16;
        /* DxCh3 */
        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* read value */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vntReg.vntGreEtherTypeConfReg;
        }
        else /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        {
            /* read value */
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIIPv4GREEthertype;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiIpv4GreEthertype;
            }
        }
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE1_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E)
            offset = 0;
        else
            offset = 16;
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIIPv6GREEtherType;
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E:
        if (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E)
            offset = 0;
        else
            offset = 16;
        /* we are reading only mplsEthertypes register because it reflects the
           value in the hdrAltMplsEthertypes as well */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MPLSEtherTypes;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.mplsEthertypes;
        }
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            offset = 0;
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).specialEtherTypes;
        }
        else
        {
            offset = 16;
            /* we are reading only specialEthertypes register because it reflects the
               value in the hdrAltIEthertype as well */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.specialEthertypes;
        }
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            offset = 0;
            /* we are reading only trillEthertype register because it reflects the
               value in the hdrAltTrillEthertype as well */
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E:
    case CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[0];
        }
        else if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[1];
        }
        else if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[2];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUserDefinedEtherTypes[3];
        }

        if ((ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E) ||
            (ethertypeType == CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E))
        {
            offset = 0;
        }
        else
        {
            offset = 16;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpGetRegField(devNum,regAddr,offset,16,ethertypePtr);
}

/**
* @internal cpssDxChTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    OUT GT_U32                              *ethertypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEthernetTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ethertypeType, ethertypePtr));

    rc = internal_cpssDxChTtiEthernetTypeGet(devNum, ethertypeType, ethertypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ethertypeType, ethertypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillCpuCodeBaseSet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - The base CPU code value for the TRILL engine .(APPLICABLE RANGES: 192..244)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if ((cpuCodeBase < CPU_CODE_BASE_MIN_CNS) ||
        (cpuCodeBase > PRV_CPSS_DXCH_TTI_TRILL_CPU_CODE_BASE_MAX_CNS))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
    rc = prvCpssHwPpSetRegField(devNum,regAddr,18,8,cpuCodeBase);

    return rc;

}

/**
* @internal cpssDxChTtiTrillCpuCodeBaseSet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - The base CPU code value for the TRILL engine .(APPLICABLE RANGES: 192..244)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillCpuCodeBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodeBase));

    rc = internal_cpssDxChTtiTrillCpuCodeBaseSet(devNum, cpuCodeBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodeBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillCpuCodeBaseGet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - Points to the base CPU code value for the TRILL engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
)
{
    GT_U32      regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodeBasePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* read value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
    return prvCpssHwPpGetRegField(devNum,regAddr,18,8,cpuCodeBasePtr);

}

/**
* @internal cpssDxChTtiTrillCpuCodeBaseGet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - Points to the base CPU code value for the TRILL engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillCpuCodeBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodeBasePtr));

    rc = internal_cpssDxChTtiTrillCpuCodeBaseGet(devNum, cpuCodeBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodeBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillAdjacencyCheckEntrySet function
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
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
* @param[in] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
)
{
    GT_STATUS rc;
    GT_U32    entryKey[3];        /* entry to write (key)         */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(ttiTrillAdjacencyPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (entryIndex > ADJACENCY_MAX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (ttiTrillAdjacencyPtr->trillMBit >= BIT_1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (ttiTrillAdjacencyPtr->trillEgressRbid >= BIT_16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (ttiTrillAdjacencyPtr->srcHwDevice > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (ttiTrillAdjacencyPtr->srcIsTrunk >= BIT_1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if ((ttiTrillAdjacencyPtr->srcIsTrunk == 0 && ttiTrillAdjacencyPtr->srcPortTrunk > (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK_MAC(15))) ||
        (ttiTrillAdjacencyPtr->srcIsTrunk == 1 && ttiTrillAdjacencyPtr->srcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum)))
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* clear output data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));

     /* handle word 0 (bits 0-31) */
    U32_SET_FIELD_MASKED_MAC(entryKey[0],0,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[5]);
    U32_SET_FIELD_MASKED_MAC(entryKey[0],8,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[4]);
    U32_SET_FIELD_MASKED_MAC(entryKey[0],16,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[3]);
    U32_SET_FIELD_MASKED_MAC(entryKey[0],24,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[2]);

    /* handle word 1 (bits 32-63) */
    U32_SET_FIELD_MASKED_MAC(entryKey[1],0,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[1]);
    U32_SET_FIELD_MASKED_MAC(entryKey[1],8,8,ttiTrillAdjacencyPtr->outerMacSa.arEther[0]);
    U32_SET_FIELD_MASKED_MAC(entryKey[1],16,16,ttiTrillAdjacencyPtr->trillEgressRbid);

    /* handle word 2 (bits 64-95) */
    U32_SET_FIELD_MASKED_MAC(entryKey[2],0,1,BOOL2BIT_MAC(ttiTrillAdjacencyPtr->trillMBit));
    U32_SET_FIELD_MASKED_MAC(entryKey[2],1,10,ttiTrillAdjacencyPtr->srcHwDevice);
    U32_SET_FIELD_MASKED_MAC(entryKey[2],11,15,ttiTrillAdjacencyPtr->srcPortTrunk);
    U32_SET_FIELD_MASKED_MAC(entryKey[2],26,1,BOOL2BIT_MAC(ttiTrillAdjacencyPtr->srcIsTrunk));

    /* write adjacency entry to the hardware */
    rc = prvCpssDxChWriteTableEntry(
        devNum,CPSS_DXCH_SIP5_TABLE_ADJACENCY_E,entryIndex,entryKey);

    return rc;
}

/**
* @internal cpssDxChTtiTrillAdjacencyCheckEntrySet function
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
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
* @param[in] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillAdjacencyCheckEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, ttiTrillAdjacencyPtr));

    rc = internal_cpssDxChTtiTrillAdjacencyCheckEntrySet(devNum, entryIndex, ttiTrillAdjacencyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, ttiTrillAdjacencyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillAdjacencyCheckEntryGet function
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
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
*
* @param[out] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
)
{

    GT_STATUS   rc;
    GT_U32      entryKey[3];        /* entry to write (key)         */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(ttiTrillAdjacencyPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (entryIndex > ADJACENCY_MAX_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* read adjacency entry from the hardware */
    rc = prvCpssDxChReadTableEntry(
        devNum,CPSS_DXCH_SIP5_TABLE_ADJACENCY_E,entryIndex,entryKey);
    if (rc != GT_OK)
        return rc;

    ttiTrillAdjacencyPtr->outerMacSa.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(entryKey[0], 0, 8);
    ttiTrillAdjacencyPtr->outerMacSa.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(entryKey[0], 8, 8);
    ttiTrillAdjacencyPtr->outerMacSa.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(entryKey[0], 16, 8);
    ttiTrillAdjacencyPtr->outerMacSa.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(entryKey[0], 24, 8);
    ttiTrillAdjacencyPtr->outerMacSa.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(entryKey[1], 0, 8);
    ttiTrillAdjacencyPtr->outerMacSa.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(entryKey[1], 8, 8);
    ttiTrillAdjacencyPtr->trillEgressRbid = (GT_U16)U32_GET_FIELD_MAC(entryKey[1], 16, 16);
    ttiTrillAdjacencyPtr->trillMBit = BIT2BOOL_MAC(U32_GET_FIELD_MAC(entryKey[2], 0, 1));
    ttiTrillAdjacencyPtr->srcHwDevice = (GT_HW_DEV_NUM)U32_GET_FIELD_MAC(entryKey[2], 1, 10);
    ttiTrillAdjacencyPtr->srcPortTrunk = U32_GET_FIELD_MAC(entryKey[2], 11, 15);
    ttiTrillAdjacencyPtr->srcIsTrunk = BIT2BOOL_MAC(U32_GET_FIELD_MAC(entryKey[2], 26, 1));

    return GT_OK;
}

/**
* @internal cpssDxChTtiTrillAdjacencyCheckEntryGet function
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
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
*
* @param[out] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillAdjacencyCheckEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, ttiTrillAdjacencyPtr));

    rc = internal_cpssDxChTtiTrillAdjacencyCheckEntryGet(devNum, entryIndex, ttiTrillAdjacencyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, ttiTrillAdjacencyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillMaxVersionSet function
* @endinternal
*
* @brief   This function sets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxTrillVersion          - max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (maxTrillVersion >= BIT_2)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
    rc = prvCpssHwPpSetRegField(devNum,regAddr,16,2,maxTrillVersion);

    return rc;
}

/**
* @internal cpssDxChTtiTrillMaxVersionSet function
* @endinternal
*
* @brief   This function sets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxTrillVersion          - max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillMaxVersionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxTrillVersion));

    rc = internal_cpssDxChTtiTrillMaxVersionSet(devNum, maxTrillVersion);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxTrillVersion));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiTrillMaxVersionGet function
* @endinternal
*
* @brief   This function gets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
)
{
    GT_U32      regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E |  CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(maxTrillVersionPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* read value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TRILLConfigs.TRILLGlobalConfigs0;
    return prvCpssHwPpGetRegField(devNum,regAddr,16,2,maxTrillVersionPtr);
}

/**
* @internal cpssDxChTtiTrillMaxVersionGet function
* @endinternal
*
* @brief   This function gets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTrillMaxVersionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxTrillVersionPtr));

    rc = internal_cpssDxChTtiTrillMaxVersionGet(devNum, maxTrillVersionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxTrillVersionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortTrillEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TRILL engine on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  TRILL engine on port
*                                      GT_FALSE: disable TRILL engine on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configure TRILL Interface Enable bit for Egress ePort in table HA Egress ePort Attribute Table 1
*       Configure TRILL Engine Enable bit for Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
static GT_STATUS internal_cpssDxChTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS   rc;         /* function return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_TBD_BOOKMARK_EARCH /* need to add 'direction' for this API .
        because ingress write 'Fefault eport' table that hold much less entries then the HA table.
        this also implies that the configuration is not symmetric . */

    if(portNum < PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write value to Pre-TTI Lookup Ingress ePort Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_ENGINE_EN_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             BOOL2BIT_MAC(enable));

        if (rc != GT_OK)
            return rc;
    }

    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

        /* write value to HA Egress ePort Attribute Table 1 */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             BOOL2BIT_MAC(enable));
    }

    return rc;
}

/**
* @internal cpssDxChTtiPortTrillEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TRILL engine on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  TRILL engine on port
*                                      GT_FALSE: disable TRILL engine on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configure TRILL Interface Enable bit for Egress ePort in table HA Egress ePort Attribute Table 1
*       Configure TRILL Engine Enable bit for Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
GT_STATUS cpssDxChTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortTrillEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiPortTrillEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortTrillEnableGet function
* @endinternal
*
* @brief   This function gets the current state enables/disables of TRILL engine
*         on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
*
* @note Same value is configured for Egress ePort in table HA Egress ePort Attribute Table 1
*       and Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
static GT_STATUS internal_cpssDxChTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
)
{
    GT_U32    value;
    GT_STATUS rc;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    CPSS_TBD_BOOKMARK_EARCH /* need to add 'direction' for this API .
        because ingress write 'Fefault eport' table that hold much less entries then the HA table.
        this also implies that the configuration is not symmetric . */

    if(portNum < PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read value from hardware table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_ENGINE_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);

        if (rc != GT_OK)
            return rc;
    }

    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

        /* read value to HA Egress ePort Attribute Table 1 */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             &value);
    }

    /* decode enable/disable state from hardware value */
    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortTrillEnableGet function
* @endinternal
*
* @brief   This function gets the current state enables/disables of TRILL engine
*         on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
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
*
* @note Same value is configured for Egress ePort in table HA Egress ePort Attribute Table 1
*       and Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
GT_STATUS cpssDxChTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortTrillEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiPortTrillEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortTrillOuterVid0Set function
* @endinternal
*
* @brief   This function sets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] vlanId                   - TRILL Outer Tag0 VID .(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
)
{
    GT_STATUS   rc;         /* function return code */

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(vlanId >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* write value from hardware table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         vlanId);

    return rc;
}

/**
* @internal cpssDxChTtiPortTrillOuterVid0Set function
* @endinternal
*
* @brief   This function sets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] vlanId                   - TRILL Outer Tag0 VID .(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortTrillOuterVid0Set);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanId));

    rc = internal_cpssDxChTtiPortTrillOuterVid0Set(devNum, portNum, vlanId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortTrillOuterVid0Get function
* @endinternal
*
* @brief   This function gets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] vlanIdPtr                - (pointer to) TRILL Outer Tag0 VID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
)
{
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(vlanIdPtr);

    /* read value from hardware table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *vlanIdPtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChTtiPortTrillOuterVid0Get function
* @endinternal
*
* @brief   This function gets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] vlanIdPtr                - (pointer to) TRILL Outer Tag0 VID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortTrillOuterVid0Get);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanIdPtr));

    rc = internal_cpssDxChTtiPortTrillOuterVid0Get(devNum, portNum, vlanIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupMacToMeSet function
* @endinternal
*
* @brief   This function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_U32      hwMacToMeArray[TTI_MAC2ME_SIZE_CNS]; /* macToMe config in hardware format */
    GT_U32      hwMacToMeSrcInterface = 0;           /* macToMe Src Interface info in hardware format */
    GT_U32      hwMacToMeSrcInterfaceSelect = 0;     /* macToMe Src Interface select info in hardware format */
    GT_U32      portGroupId;                         /* port group Id */
    GT_U32      rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(interfaceInfoPtr);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (entryIndex > 127)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (entryIndex > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (valuePtr->vlanId >= BIT_13)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if (valuePtr->vlanId >= BIT_12) /* MAC_TO_ME table supports only 12 bits for VLAN */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (interfaceInfoPtr->includeSrcInterface)
        {
        case CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E:
            hwMacToMeSrcInterfaceSelect = 0;
            break;
        case CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                hwMacToMeSrcInterfaceSelect = 1;/*compare with device */

                if(interfaceInfoPtr->srcHwDevice > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                if ((interfaceInfoPtr->srcIsTrunk == GT_TRUE && interfaceInfoPtr->srcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum)) ||
                    (interfaceInfoPtr->srcIsTrunk == GT_FALSE && interfaceInfoPtr->srcPortTrunk > (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK_MAC(15))))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

                U32_SET_FIELD_MASKED_MAC(hwMacToMeSrcInterface,0,10,interfaceInfoPtr->srcHwDevice);
                U32_SET_FIELD_MASKED_MAC(hwMacToMeSrcInterface,10,1,BOOL2BIT_MAC(interfaceInfoPtr->srcIsTrunk));
                U32_SET_FIELD_MASKED_MAC(hwMacToMeSrcInterface,11,15,interfaceInfoPtr->srcPortTrunk);
            }
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        case CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E:
            hwMacToMeSrcInterfaceSelect = 2;/*compare without device */

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                if((interfaceInfoPtr->srcIsTrunk == GT_TRUE && interfaceInfoPtr->srcPortTrunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum)) ||
                    (interfaceInfoPtr->srcIsTrunk == GT_FALSE && interfaceInfoPtr->srcPortTrunk > (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) & BIT_MASK_MAC(15))))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

                U32_SET_FIELD_MASKED_MAC(hwMacToMeSrcInterface,10,1,BOOL2BIT_MAC(interfaceInfoPtr->srcIsTrunk));
                U32_SET_FIELD_MASKED_MAC(hwMacToMeSrcInterface,11,15 ,interfaceInfoPtr->srcPortTrunk);
            }
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* clear output data */
    cpssOsMemSet(hwMacToMeArray, 0, sizeof(GT_U32) * TTI_MAC2ME_SIZE_CNS);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        hwMacToMeArray[0] = (GT_HW_MAC_LOW16(&(valuePtr->mac)) << 16) | (valuePtr->vlanId & 0x1fff);
        hwMacToMeArray[1] =  GT_HW_MAC_HIGH32(&(valuePtr->mac));
        hwMacToMeArray[2] = (GT_HW_MAC_LOW16(&(maskPtr->mac)) << 16)  | (maskPtr->vlanId & 0x1fff);
        hwMacToMeArray[3] =  GT_HW_MAC_HIGH32(&(maskPtr->mac));
        hwMacToMeArray[4] =  hwMacToMeSrcInterface;
        hwMacToMeArray[5] =  hwMacToMeSrcInterfaceSelect;
    }
    else
    {
        hwMacToMeArray[0] = (GT_HW_MAC_LOW16(&(valuePtr->mac)) << 16) | ((valuePtr->vlanId & 0xfff) << 4);
        hwMacToMeArray[1] = GT_HW_MAC_HIGH32(&(valuePtr->mac));
        hwMacToMeArray[2] = (GT_HW_MAC_LOW16(&(maskPtr->mac)) << 16) | ((maskPtr->vlanId & 0xfff) << 4);
        hwMacToMeArray[3] = GT_HW_MAC_HIGH32(&(maskPtr->mac));
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* write mac to me entry to the hardware */
        rc =  prvCpssDxChPortGroupWriteTableEntry(
            devNum,portGroupId,CPSS_DXCH3_TABLE_MAC2ME_E,entryIndex,hwMacToMeArray);

        if (rc != GT_OK)
            return rc;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupMacToMeSet function
* @endinternal
*
* @brief   This function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupMacToMeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC         *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupMacToMeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));

    rc = internal_cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_U32      hwMacToMeArray[TTI_MAC2ME_SIZE_CNS]; /* macToMe config in hardware format */
    GT_U32      hwMacToMeSrcInterface;           /* macToMe Src Interface info in hardware format */
    GT_U32      hwMacToMeSrcInterfaceSelect;     /* macToMe Src Interface select info in hardware format */
    GT_U32      portGroupId;                         /* port group Id */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(interfaceInfoPtr);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (entryIndex > 127)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (entryIndex > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read mac to me entry from the hardware */
    if (prvCpssDxChPortGroupReadTableEntry(
        devNum,portGroupId,CPSS_DXCH3_TABLE_MAC2ME_E,entryIndex,hwMacToMeArray)!= GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    valuePtr->mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[0], 16, 8);
    valuePtr->mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[0], 24, 8);
    valuePtr->mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[1],  0, 8);
    valuePtr->mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[1],  8, 8);
    valuePtr->mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[1], 16, 8);
    valuePtr->mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[1], 24, 8);

    maskPtr->mac.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[2], 16, 8);
    maskPtr->mac.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[2], 24, 8);
    maskPtr->mac.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[3],  0, 8);
    maskPtr->mac.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[3],  8, 8);
    maskPtr->mac.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[3], 16, 8);
    maskPtr->mac.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(hwMacToMeArray[3], 24, 8);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        valuePtr->vlanId = (GT_U16)U32_GET_FIELD_MAC(hwMacToMeArray[0], 0, 13);
        maskPtr->vlanId  = (GT_U16)U32_GET_FIELD_MAC(hwMacToMeArray[2], 0, 13);

        hwMacToMeSrcInterface = hwMacToMeArray[4];
        hwMacToMeSrcInterfaceSelect = hwMacToMeArray[5];

         switch (hwMacToMeSrcInterfaceSelect & 0x3) /* includeSrcInterface */
         {
         case 0:
             interfaceInfoPtr->includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;
             break;
         case 1:
             interfaceInfoPtr->includeSrcInterface = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E;
             interfaceInfoPtr->srcHwDevice = U32_GET_FIELD_MAC(hwMacToMeSrcInterface,0,10);
             interfaceInfoPtr->srcIsTrunk = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwMacToMeSrcInterface,10,1));
             interfaceInfoPtr->srcPortTrunk = U32_GET_FIELD_MAC(hwMacToMeSrcInterface,11,15);
             break;
         case 2:
             interfaceInfoPtr->includeSrcInterface = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
             interfaceInfoPtr->srcIsTrunk = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwMacToMeSrcInterface,10,1));
             interfaceInfoPtr->srcPortTrunk = U32_GET_FIELD_MAC(hwMacToMeSrcInterface,11,15);
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
         }
    }
    else
    {
        valuePtr->vlanId = (GT_U16)U32_GET_FIELD_MAC(hwMacToMeArray[0], 4, 12);
        maskPtr->vlanId  = (GT_U16)U32_GET_FIELD_MAC(hwMacToMeArray[2], 4, 12);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupMacToMeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC          *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupMacToMeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));

    rc = internal_cpssDxChTtiPortGroupMacToMeGet(devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, entryIndex, valuePtr, maskPtr, interfaceInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
static GT_STATUS internal_cpssDxChTtiPortGroupMacModeSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
{
    GT_U32      value;          /* value to write       */
    GT_U32      regAddr;        /* register address     */
    GT_U32      offset;         /* offset in register   */
    GT_STATUS   rc;             /* function return code */
    GT_U32      portGroupId;    /* port group Id        */
    GT_U32      index;          /* udb index */
    GT_U32      numOfUdbs;      /* number of UDBs needed for 'mac SA/DA'*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (macMode)
        {
        case CPSS_DXCH_TTI_MAC_MODE_DA_E:
        case CPSS_DXCH_TTI_MAC_MODE_SA_E:
            /* do nothing */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
        case CPSS_DXCH_TTI_KEY_MPLS_E:
        case CPSS_DXCH_TTI_KEY_ETH_E:
        case CPSS_DXCH_TTI_KEY_MIM_E:
            /* do nothing */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* UDB8-13: Anchor=L2; Offset=0-5/6-11 */
        /* MAC_DA/MAC_SA
         */
        /* set the bytes of the SA/DA in little endian order */
        index = sip5legacyMacSaDaIndexInKey;

        rc = wellKnownPacketPartsUsedSet(devNum,keyType,
            (macMode == CPSS_DXCH_TTI_MAC_MODE_DA_E) ?
                TTI_BYTES_FROM_PACKET_L2_MAC_DA_E :
                TTI_BYTES_FROM_PACKET_L2_MAC_SA_E ,
            index,
            &numOfUdbs);
        if (rc != GT_OK)
        {
            return rc;
        }

        return GT_OK;
    }

    /* prepare value to write */
    switch (macMode)
    {
    case CPSS_DXCH_TTI_MAC_MODE_DA_E:
        value  = 0;
        break;
    case CPSS_DXCH_TTI_MAC_MODE_SA_E:
        value  = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
            offset = 25;
            break;
        case CPSS_DXCH_TTI_KEY_MPLS_E:
            offset = 24;
            break;
        case CPSS_DXCH_TTI_KEY_ETH_E:
            offset = 26;
            break;
        default:
            if ((keyType == CPSS_DXCH_TTI_KEY_MIM_E) && (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* write value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupSetRegField(
                devNum,portGroupId,regAddr,offset,1,value);

            if (rc != GT_OK)
                return rc;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)

    }
    else /* xCat3; Lion2 */
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
            offset = 0;
            break;
        case CPSS_DXCH_TTI_KEY_MPLS_E:
            offset = 1;
            break;
        case CPSS_DXCH_TTI_KEY_ETH_E:
            offset = 2;
            break;
        case CPSS_DXCH_TTI_KEY_MIM_E:
            offset = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* write value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiEngineConfig;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,offset,1,value);

            if (rc != GT_OK)
                return rc;

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiPortGroupMacModeSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupMacModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, keyType, macMode));

    rc = internal_cpssDxChTtiPortGroupMacModeSet(devNum, portGroupsBmp, keyType, macMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, keyType, macMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
static GT_STATUS internal_cpssDxChTtiPortGroupMacModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
{
    GT_U32      value;      /* value to write       */
    GT_U32      regAddr;    /* register address     */
    GT_U32      offset;     /* offset in register   */
    GT_STATUS   rc;         /* function return code */
    GT_U32      portGroupId;/* port group Id        */
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT  dummyOffsetType;
    GT_U32      udbOffset;
    GT_U32      index;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macModePtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
        case CPSS_DXCH_TTI_KEY_MPLS_E:
        case CPSS_DXCH_TTI_KEY_ETH_E:
        case CPSS_DXCH_TTI_KEY_MIM_E:
            /* do nothing */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* UDB8-13: Anchor=L2; Offset=0-5/6-11 */
        /* MAC_DA/MAC_SA
         */
        /* Get the bytes of the SA/DA in little endian order */
        index = sip5legacyMacSaDaIndexInKey;

        rc = ttiInitUserDefinedByteGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,keyType,index,&dummyOffsetType,&udbOffset);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (udbOffset == 5)
            *macModePtr = CPSS_DXCH_TTI_MAC_MODE_DA_E; /* MSB byte of MAC DA is in offset 5 */
        else
            *macModePtr = CPSS_DXCH_TTI_MAC_MODE_SA_E; /* MSB byte of MAC SA is in offset 11 */

        return GT_OK;
    }

    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
            offset = 25;
            break;
        case CPSS_DXCH_TTI_KEY_MPLS_E:
            offset = 24;
            break;
        case CPSS_DXCH_TTI_KEY_ETH_E:
            offset = 26;
            break;
        default:
            if ((keyType == CPSS_DXCH_TTI_KEY_MIM_E) && (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* read value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
        rc = prvCpssHwPpPortGroupGetRegField(
            devNum,portGroupId,regAddr,offset,1,&value);
    }
    else /* xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2 */
    {
        switch (keyType)
        {
        case CPSS_DXCH_TTI_KEY_IPV4_E:
            offset = 0;
            break;
        case CPSS_DXCH_TTI_KEY_MPLS_E:
            offset = 1;
            break;
        case CPSS_DXCH_TTI_KEY_ETH_E:
            offset = 2;
            break;
        case CPSS_DXCH_TTI_KEY_MIM_E:
            offset = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* read value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiEngineConfig;
        rc = prvCpssHwPpPortGroupGetRegField(
            devNum,portGroupId,regAddr,offset,1,&value);
    }

    if (rc != GT_OK)
        return rc;

    switch (value)
    {
    case 0:
        *macModePtr = CPSS_DXCH_TTI_MAC_MODE_DA_E;
        break;
    case 1:
        *macModePtr = CPSS_DXCH_TTI_MAC_MODE_SA_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiPortGroupMacModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupMacModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, keyType, macModePtr));

    rc = internal_cpssDxChTtiPortGroupMacModeGet(devNum, portGroupsBmp, keyType, macModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, keyType, macModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS   rc;                                           /* function return code    */
    GT_U32      hwTtiKeyArray[TTI_RULE_SIZE_CNS];             /* TTI key in hw format    */
    GT_U32      hwTtiMaskArray[TTI_RULE_SIZE_CNS];            /* TTI mask in hw format   */
    GT_U32      hwTtiActionArray[TTI_ACTION_TYPE_2_SIZE_CNS]; /* TTI action in hw format */
    GT_U32      portGroupId;                                  /* port group Id           */
    GT_U32      entryNumber = 0;                              /* Rule's index in TCAM    */
    GT_U32      dummy;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E; /* default */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(patternPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    /* convert tti configuration pattern and mask to hardware format */
    rc = ttiConfigLogic2HwFormat(devNum,
                                 ruleType,
                                 patternPtr,
                                 maskPtr,
                                 hwTtiKeyArray,
                                 hwTtiMaskArray);
    if (rc != GT_OK)
        return rc;

    /* convert tti action to hardware format */
    rc = ttiActionLogic2HwFormat(devNum,
                                 actionType,
                                 actionPtr,
                                 hwTtiActionArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* convert rule's type to rule's size */
        PRV_CPSS_DXCH_TTI_CONVERT_RULE_TYPE_TO_TCAM_RULE_SIZE_VAL_MAC(ruleSize,ruleType);

        /* Check if an global TCAM index is valid for a TCAM rule's size */
        rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* write action */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                     (entryNumber/2),
                                                     hwTtiActionArray);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* write rule to the TCAM */
            rc = cpssDxChTcamPortGroupRuleWrite(devNum,1<<portGroupId,index,GT_TRUE,ruleSize,hwTtiKeyArray,hwTtiMaskArray);

            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* now write entry to the router / tunnel termination TCAM */
            rc = prvCpssDxChRouterTunnelTermTcamSetLine(
                devNum,portGroupId,index,hwTtiKeyArray,
                hwTtiMaskArray,hwTtiActionArray);

            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
     PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
           devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupRuleSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));

    rc = internal_cpssDxChTtiPortGroupRuleSet(devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{

    GT_STATUS   rc;                                                           /* function return code    */
    GT_U32      validArr[5];                                                  /* TCAM line valid bits    */
    GT_U32      compareModeArr[5];                                            /* TCAM line compare mode  */
    GT_U32      portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;               /* port group Id           */
    GT_U32      hwTtiKeyArray[TCAM_RULE_SIZE_CNS];                            /* TTI key in hw format    */
    GT_U32      hwTtiMaskArray[TCAM_RULE_SIZE_CNS];                           /* TTI mask in hw format   */
    GT_U32      hwTtiActionArray[TTI_ACTION_TYPE_2_SIZE_CNS];                 /* TTI action in hw format */
    GT_BOOL     validFound;                                                   /* is rule found valid     */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSize;                                   /* rule size for ruleType  */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSizeFound;                              /* rule size found in TCAM */
    GT_U32      entryNumber;                                                  /* Rule's index in TCAM    */
    GT_U32      dummy;

    /* check pararmeters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(patternPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

     /* zero out hw format */
    cpssOsMemSet(hwTtiKeyArray,0,sizeof(hwTtiKeyArray));
    cpssOsMemSet(hwTtiMaskArray,0,sizeof(hwTtiMaskArray));
    cpssOsMemSet(hwTtiActionArray,0,sizeof(hwTtiActionArray));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* convert rule's type to rule's size */
        PRV_CPSS_DXCH_TTI_CONVERT_RULE_TYPE_TO_TCAM_RULE_SIZE_VAL_MAC(ruleSize,ruleType);

        /* Check if an global TCAM index is valid for a TCAM rule's size */
        rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* get the first port group in the BMP */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);
        /* read action */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                                 portGroupId,
                                                 CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                 (entryNumber/2),
                                                 hwTtiActionArray);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* read rule from the TCAM */
        rc = cpssDxChTcamPortGroupRuleRead(devNum,portGroupsBmp,index,&validFound,&ruleSizeFound,hwTtiKeyArray,hwTtiMaskArray);

        if ((ruleSizeFound != ruleSize) && (validFound == GT_TRUE))
        {
            /* the rule is valid but has another size */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        rc = prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine(devNum,
                                                                   portGroupsBmp,
                                                                   index,
                                                                   &validArr[0],
                                                                   &compareModeArr[0],
                                                                   hwTtiKeyArray,
                                                                   hwTtiMaskArray,
                                                                   hwTtiActionArray);
    }
    else
    {
        /* read tunnel termination configuration and action from hardware */
        rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                    portGroupId,
                                                    index,
                                                    &validArr[0],
                                                    &compareModeArr[0],
                                                    hwTtiKeyArray,
                                                    hwTtiMaskArray,
                                                    hwTtiActionArray);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* convert tti configuration from hw format to logic format */
        rc = ttiConfigHw2LogicFormat(devNum,ruleType,hwTtiKeyArray,patternPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* convert tti configuration mask from hw format to logic format */
        rc = ttiConfigHw2LogicFormat(devNum,ruleType,hwTtiMaskArray,maskPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* convert tti configuration 'old keys' as udbs from hw format to logic format */
        rc = ttiConfigHwUdb2LogicFormat(ruleType,GT_TRUE,hwTtiKeyArray,patternPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* convert tti configuration mask 'old keys' as udbs from hw format to logic format */
        rc = ttiConfigHwUdb2LogicFormat(ruleType,GT_FALSE,hwTtiMaskArray,maskPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* convert tti configuration for dual device id from hw format to logic format */
    rc = ttiConfigLogicFormatForDualDeviceId(ruleType, patternPtr, maskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert tti action from hw format to logic format */
    rc = prvCpssDxChTtiActionHw2LogicFormat(devNum,hwTtiActionArray,actionType,actionPtr);

    return rc;
}

/**
* @internal cpssDxChTtiPortGroupRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupRuleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));

    rc = internal_cpssDxChTtiPortGroupRuleGet(devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, ruleType, patternPtr, maskPtr, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Index of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS   rc;                                            /* return status                  */
    GT_U32      hwTtiActionArray[TTI_ACTION_TYPE_2_SIZE_CNS];  /* TTI action in hw format        */
    GT_U32      portGroupId;                                   /* port group Id                  */
    GT_U32      entryNumber = 0;                               /* Rule's index in TCAM           */
    GT_U32      dummy;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    /* convert tti action to hardware format */
    rc = ttiActionLogic2HwFormat(devNum,actionType,actionPtr,hwTtiActionArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* write action */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                     (entryNumber/2),
                                                     hwTtiActionArray);
        }
        else
        {
            /* write action */
            rc =  prvCpssDxChPortGroupWriteTableEntry(
                devNum,portGroupId,CPSS_DXCH3_LTT_TT_ACTION_E,
                index,hwTtiActionArray);
        }

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
* @internal cpssDxChTtiPortGroupRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Index of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupRuleActionUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, actionType, actionPtr));

    rc = internal_cpssDxChTtiPortGroupRuleActionUpdate(devNum, portGroupsBmp, index, actionType, actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, actionType, actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
static GT_STATUS internal_cpssDxChTtiPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_U32      keyArr[TTI_RULE_SIZE_CNS];          /* TCAM key in hw format            */
    GT_U32      maskArr[TTI_RULE_SIZE_CNS];         /* TCAM mask in hw format           */
    /* TCAM action (rule) in hw format  */
    GT_U32      actionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32      validArr[5];        /* TCAM line valid bits             */
    GT_U32      compareModeArr[5];  /* TCAM line compare mode           */
    /* port group Id                    */
    GT_U32      portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32      entryKey[4];        /* dummy                        */
    GT_U32      entryMask[4];       /* dummy                        */
    GT_BOOL     validFound;         /* is rule found valid          */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSizeFound; /* rule size found */

    GT_STATUS   rc = GT_OK;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

     /* zero out hw format */
    cpssOsMemSet(keyArr,0,sizeof(keyArr));
    cpssOsMemSet(maskArr,0,sizeof(maskArr));
    cpssOsMemSet(actionArr,0,sizeof(actionArr));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssDxChTcamPortGroupRuleReadEntry(devNum,1<<portGroupId,index,&validFound,&ruleSizeFound,entryKey,entryMask);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (valid == validFound)
            {
                /* the rule is already in required valid state */
            }
            else
            {
                rc = cpssDxChTcamPortGroupRuleValidStatusSet(devNum,1<<portGroupId,index,valid);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
           devNum, portGroupsBmp, portGroupId)

        return GT_OK;
    }


    if (valid == GT_TRUE)
    {
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            rc = prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine(devNum,
                                                                       portGroupsBmp,
                                                                       index,
                                                                       &validArr[0],        /* dummy */
                                                                       &compareModeArr[0],  /* dummy */
                                                                       keyArr,
                                                                       maskArr,
                                                                       actionArr);
        }
        else
        {
            /* read tunnel termination configuration and action from hardware */
            rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                        portGroupId,
                                                        index,
                                                        &validArr[0],       /* dummy */
                                                        &compareModeArr[0], /* dummy */
                                                        keyArr,
                                                        maskArr,
                                                        actionArr);
        }

        if (rc != GT_OK)
            return rc;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            /* write the line (note it also set the line as valid) */
            rc = prvCpssDxChRouterTunnelTermTcamSetLine(devNum,
                                                        portGroupId,
                                                        index,
                                                        keyArr,
                                                        maskArr,
                                                        actionArr);
            if (rc != GT_OK)
                return rc;
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
           devNum, portGroupsBmp, portGroupId)
    }
    else /* (valid == GT_FALSE) */
    {
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            /* just invalidate the line */
            rc = prvCpssDxChRouterTunnelTermTcamInvalidateLine(
                devNum,portGroupId,index);

            if (rc != GT_OK)
                return rc;
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupRuleValidStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, valid));

    rc = internal_cpssDxChTtiPortGroupRuleValidStatusSet(devNum, portGroupsBmp, index, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
)
{
    GT_U32      keyArr[TTI_RULE_SIZE_CNS];          /* TCAM key in hw format            */
    GT_U32      maskArr[TTI_RULE_SIZE_CNS];         /* TCAM mask in hw format           */
      /* TCAM action (rule) in hw format  */
    GT_U32      actionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32      validArr[6];        /* TCAM line valid bits             */
    GT_U32      compareModeArr[6];  /* TCAM line compare mode           */
    GT_U32      portGroupId;        /* port group Id                    */
    GT_STATUS   rc;                 /* function return code             */
    GT_U32      i;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize; /* dummy */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum,index);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return cpssDxChTcamPortGroupRuleValidStatusGet(devNum,portGroupsBmp,index,validPtr,&ruleSize);
    }

    rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                portGroupId,
                                                index,
                                                validArr,
                                                compareModeArr,
                                                keyArr,
                                                maskArr,
                                                actionArr);
    if (rc != GT_OK)
        return rc;

    /* line holds valid TTI entry if the following applies:
         - all entries are valid
         - the compare mode or all entries is row compare
         - keyArr[5] bit 31 must be 1 (to indicate TTI entry and not IPv6 entry) */
    *validPtr = GT_TRUE;
    for (i = 0 ; i < 4; i++)
    {
        /* if entry is not valid or is single compare mode, whole line is not valid */
        if ((validArr[i] == 0) || (compareModeArr[i] == 0))
        {
            *validPtr = GT_FALSE;
            break;
        }
    }
    /* if whole line is valid, verify it is indeed TTI entry and not IPv6 entry */
    if ((*validPtr == GT_TRUE) && (((keyArr[5] >> 31) & 0x1) != 1))
    {
        *validPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupRuleValidStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, validPtr));

    rc = internal_cpssDxChTtiPortGroupRuleValidStatusGet(devNum, portGroupsBmp, index, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, validPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type. .(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChTtiPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if ((offset >= BIT_7) || (udbIndex >= UDB_CONFIG_NUM_WORDS_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return ttiInitUserDefinedByteSet(devNum,portGroupsBmp,keyType,udbIndex,offsetType,offset);

}

/**
* @internal cpssDxChTtiPortGroupUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type. .(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupUserDefinedByteSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, keyType, udbIndex, offsetType, offset));

    rc = internal_cpssDxChTtiPortGroupUserDefinedByteSet(devNum, portGroupsBmp, keyType, udbIndex, offsetType, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, keyType, udbIndex, offsetType, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortGroupUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
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
static GT_STATUS internal_cpssDxChTtiPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
{
    GT_STATUS   rc;
    GT_U32  offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(offsetTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);


    if (udbIndex >= UDB_CONFIG_NUM_WORDS_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (keyType)
    {
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_MPLS_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E:
    case CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE1_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE2_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE3_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE4_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE5_E:
    case CPSS_DXCH_TTI_KEY_UDB_UDE6_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = ttiInitUserDefinedByteGet(devNum,portGroupsBmp,keyType,udbIndex,offsetTypePtr,&offset);
    *offsetPtr = (GT_U8)offset;

    return rc;
}

/**
* @internal cpssDxChTtiPortGroupUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
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
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortGroupUserDefinedByteGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, keyType, udbIndex, offsetTypePtr, offsetPtr));

    rc = internal_cpssDxChTtiPortGroupUserDefinedByteGet(devNum, portGroupsBmp, keyType, udbIndex, offsetTypePtr, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, keyType, udbIndex, offsetTypePtr, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Set function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] mode                     - recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port , mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;    /* hw value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

    switch(mode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            hwValue = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            hwValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write to pre-tti-lookup-ingress-eport table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwValue);
    return rc;

}

/**
* @internal cpssDxChTtiPortPassengerOuterIsTag0Or1Set function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] mode                     - recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port , mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortPassengerOuterIsTag0Or1Set);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Set(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Get function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] modePtr                  - (pointer to)recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Get
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT  CPSS_ETHER_MODE_ENT          *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;    /* hw value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* read from pre-tti-lookup-ingress-eport table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);

    *modePtr = (hwValue == 1) ?
                CPSS_VLAN_ETHERTYPE1_E :
                CPSS_VLAN_ETHERTYPE0_E;

    return rc;

}

/**
* @internal cpssDxChTtiPortPassengerOuterIsTag0Or1Get function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] modePtr                  - (pointer to)recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Get
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT  CPSS_ETHER_MODE_ENT          *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPortPassengerOuterIsTag0Or1Get);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChTtiPortPassengerOuterIsTag0Or1Get(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal metaDataFieldInfoGet function
* @endinternal
*
* @brief   get the metadata field info for given field.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] metadataField            - meta data field
*
* @param[out] fieldInfoPtrPtr          - (pointer to) (pointer to) field info (start bit , length)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS  metaDataFieldInfoGet
(
    IN TTI_META_DATA_FIELDS_ENT  metadataField,
    OUT PRV_CPSS_ENTRY_FORMAT_TABLE_STC **fieldInfoPtrPtr
)
{
    if(metadataField >= TTI_META_DATA_FIELDS___LAST_VALUE___E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *fieldInfoPtrPtr = &(ttiMetaDataFieldsFormat[metadataField]);

    return GT_OK;
}

/**
* @internal metaDataUdbsGet function
* @endinternal
*
* @brief   get the number of UDBs needed for given list of fields.
*         and get a BMP of offset from the metadata start
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fieldsArrPtr             - (pointer to) array of meta data fields
* @param[in] numOfFields              - number of fields in fieldsArrPtr
*
* @param[out] neededBytesBmpPtr        - (pointer to) bmp of the needed bytes (each needed byte represented by single bit)
*                                      since TTI metadata hold exactly 32 bytes , we can use single BMP of 32 bits.
* @param[out] numOfUdbsPtr             - (pointer to) the number of UDBs needed for the fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS  metaDataUdbsGet
(
    IN GT_U8                    devNum,
    IN TTI_META_DATA_FIELDS_ENT *fieldsArrPtr,
    IN GT_U32                   numOfFields,
    OUT GT_U32                  *neededBytesBmpPtr,
    OUT GT_U32                  *numOfUdbsPtr
)
{
    GT_U32  ii;
    GT_STATUS   rc;
    GT_U32  neededBytesBmp = 0;/* since TTI metadata hold exactly 32 bytes , we can use single BMP of 32 bits */
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *fieldInfoPtr;
    GT_U32  startByte , endByte , numBytes;
    GT_U32  maxNumMetaDataBytes = NUM_OF_META_DATA_BYTES_MAC(devNum); /* max number of bytes of metadata  */
    GT_U32  numSupportedMetaDataBytes = NUM_OF_MAX_META_DATA_BYTES_MAC(devNum); /* number of supported bytes of metadata */

    if(maxNumMetaDataBytes > numSupportedMetaDataBytes)
    {
        /* the current code written to support up to 32 bytes */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }


    for(ii = 0 ; ii < numOfFields ; ii++)
    {
        rc = metaDataFieldInfoGet(fieldsArrPtr[ii],&fieldInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        startByte = (fieldInfoPtr->startBit / 8);
        endByte = (fieldInfoPtr->startBit + fieldInfoPtr->numOfBits - 1) / 8;
        numBytes = (endByte - startByte) + 1;

        /* state the needed bytes (each needed byte represented by single bit)*/
        U32_SET_FIELD_MASKED_MAC(neededBytesBmp ,
            startByte ,      /* start byte */
            numBytes,  /* number of bytes */
            0xFFFFFFFF);
    }

    if(neededBytesBmpPtr)
    {
        *neededBytesBmpPtr = neededBytesBmp;
    }

    if(numOfUdbsPtr)
    {
        /* get the number of bits that are set in neededBytesBmp */
        *numOfUdbsPtr = prvCpssPpConfigBitmapNumBitsGet(neededBytesBmp);
    }

    if(neededBytesBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal wellKnownPacketPartUdbsGet function
* @endinternal
*
* @brief   get the number of UDBs needed for given packet part
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] field                    - packet part
*
* @param[out] numOfUdbsPtr             - (pointer to) the number of UDBs needed for the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS  wellKnownPacketPartUdbsGet
(
    IN TTI_BYTES_FROM_PACKET_ENT  field,
    OUT GT_U32          *numOfUdbsPtr
)
{
    PACKET_PART_INFO_STC *currentPartPtr;
    GT_U32  startByte , endByte , numBytes;

    if(field >= TTI_BYTES_FROM_PACKET___LAST_VALUE___E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    currentPartPtr = & ttiWellKnownPacketPartsArr[field];
    switch(currentPartPtr->partType)
    {
        case CPSS_DXCH_TTI_OFFSET_L2_E:
        case CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E:
        case CPSS_DXCH_TTI_OFFSET_L4_E:
        case CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E:
            break;
        case CPSS_DXCH_TTI_OFFSET_INVALID_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    startByte = (currentPartPtr->startByteOffset / 8);
    /* state the needed bytes (each needed byte represented by single bit)*/
    if(currentPartPtr->flag != FLAG_VALUE_REVERED_E)
    {
        endByte = startByte + ((0 + (currentPartPtr->startBitInByte + currentPartPtr->numOfBits - 1)) / 8);
    }
    else /*FLAG_VALUE_REVERED_E*/
    {
        endByte = startByte + ((7 + ((currentPartPtr->numOfBits - 1) - currentPartPtr->startBitInByte)) / 8);
    }

    numBytes = (endByte - startByte) + 1;

    (*numOfUdbsPtr) = numBytes;

    return GT_OK;
}



/**
* @internal metaDataFeildsUsedSet function
* @endinternal
*
* @brief   init common values for specific keys in TTI UDBs Configuration Table.
*         for list of fields.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type
* @param[in] fieldsArrPtr             - (pointer to) array of meta data fields
* @param[in] numOfFields              - number of fields in fieldsArrPtr
* @param[in] startIndexInKey          - the first index in the key that should be bound to the fields.
*
* @param[out] numOfUdbsPtr             - (pointer to) the number of UDBs needed for the fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS  metaDataFeildsUsedSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType,
    IN TTI_META_DATA_FIELDS_ENT  *fieldsArrPtr,
    IN GT_U32              numOfFields,
    IN GT_U32              startIndexInKey,
    OUT GT_U32              *numOfUdbsPtr
)
{
    GT_STATUS   rc;
    GT_U32  neededBytesBmp;
    GT_U32  offset;/* byte offset in the metadata */
    GT_U32  index ;/* index in the key */
    GT_U32  maxNumMetaDataBytes = NUM_OF_META_DATA_BYTES_MAC(devNum);/* max number of bytes of metadata  */

    rc = metaDataUdbsGet(devNum, fieldsArrPtr,
            numOfFields,&neededBytesBmp,numOfUdbsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    index = startIndexInKey;/* index in the key */

    for (offset = 0; offset < maxNumMetaDataBytes; offset++)
    {
        if(0 == ((1 << offset) & neededBytesBmp))
        {
            continue;
        }

        rc = ttiInitUserDefinedByteSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            keyType,index,CPSS_DXCH_TTI_OFFSET_METADATA_E,offset);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* start to the next index in the key */
        index ++;
    }

    return GT_OK;
}



/**
* @internal wellKnownPacketPartsUsedSet function
* @endinternal
*
* @brief   set packets part for specific keys in TTI UDBs Configuration Table.
*         function bind parts in 'little endian' order.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type
* @param[in] field                    - single part
* @param[in] startIndexInKey          - the first index in the key that should be bound to the parts
*
* @param[out] numOfUdbsPtr             - (pointer to) the number of UDBs needed for the part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS  wellKnownPacketPartsUsedSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType,
    IN TTI_BYTES_FROM_PACKET_ENT  field,
    IN GT_U32              startIndexInKey,
    OUT GT_U32             *numOfUdbsPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  index ;/* index in the key */
    GT_U32  anchorIndex;/*index in the anchor*/
    PACKET_PART_INFO_STC *currentPartPtr;

    rc = wellKnownPacketPartUdbsGet(field,numOfUdbsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    index = startIndexInKey;/* index in the key */

    currentPartPtr = & ttiWellKnownPacketPartsArr[field];

    /* anchor is in reverse because 'little endian' order */
    anchorIndex = currentPartPtr->startByteOffset + (*numOfUdbsPtr) - 1;

    for(ii = 0; ii < (*numOfUdbsPtr) ;index++ ,ii++)
    {
        rc = ttiInitUserDefinedByteSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                keyType,index,
                currentPartPtr->partType,
                anchorIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* anchor is in reverse because 'little endian' order */
        anchorIndex--;
    }

    return GT_OK;
}


/**
* @internal ttiInitUserDefinedByteCommonSet function
* @endinternal
*
* @brief   init common values for specific keys in TTI User Defined bytes Configuration Table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type
*
* @retval GT_OK                    - on success
*/
static GT_STATUS  ttiInitUserDefinedByteCommonSet
(
    IN GT_U8                       devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType
)
{
    GT_STATUS  rc;
    GT_U32  numOfUdbs;
    GT_U32  index ;/* index in the key */

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    /* NOTE: this function sets

        UDBs 0..8 (9 UDBs) for metadata
        UDBs 9..13 (6 UDBs) for DA mac (place holder for SA/DA)
        UDBs 28..29 (2 UDBs) for metadata
        and another 2 lsb UDBS
        */



    /*  metadata bytes : 22..29 (8 UDBs)
        legacyKey_common_metadata22_29_fieldsArr[]
        PCL ID
        Local Source is Trunk
        DSA SrcIsTrunk
        CoreID
        eVLAN
        Local Source ePort/TrunkID[15:0]
        DSA Tag Source Port/Trunk[15:0]
    */
    index = 0;
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_common_metadata22_29_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    /*  metadata bytes : 11 (1 UDB)
        legacyKey_common_metadata11_fieldsArr[]
        VLAN Tag0 Exists
    */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_common_metadata11_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    /* UDB8-13: Anchor=L2; Offset=0-5/6-11 */
    /* MAC_DA/MAC_SA
     */

    /* save the actual index */
    sip5legacyMacSaDaIndexInKey = index;
    /* for BWC we set MAC DA */
    rc = wellKnownPacketPartsUsedSet(devNum,keyType,
        TTI_BYTES_FROM_PACKET_L2_MAC_DA_E,
        index,
        &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    /*  metadata bytes : 30..31 (2 UDBs)
        legacyKey_common_metadata30_31_fieldsArr[]
       DSA Tag Source Device
    */
    index = 28;
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_common_metadata30_31_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    return GT_OK;
}

/**
* @internal sip5BuildUdbsNeededForLegacyKey_ipv4 function
* @endinternal
*
* @brief   build legacy key needed UDBs - ip key
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - the key type that need ipv4 legacy style
*
* @retval GT_OK                    - on success
*/
GT_STATUS sip5BuildUdbsNeededForLegacyKey_ipv4
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT keyType
)
{
    GT_STATUS  rc;
    GT_U32 index;
    GT_U32  startIndex;
    GT_U32  numOfUdbs;

    /* init UDBs for CPSS_DXCH_TTI_KEY_IPV4_E */
    ttiInitUserDefinedByteCommonSet(devNum,keyType);
    startIndex = sip5legacyMacSaDaIndexInKey + 6;/* start after the 6 UDBs of mac sa/da*/

    index = startIndex;

/*
    IPV4_ARP_SIP,
*/
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_ipv4_sip_metadata_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }
    index += numOfUdbs;

/*
    IPV4_ARP_DIP,
*/
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_ipv4_dip_metadata_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

/*
    IS_ARP,
    TUNNELING_PROTOCOL
*/
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_ip_metadata0_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    return GT_OK;
}


/**
* @internal sip5BuildUdbsNeededForLegacyKey_mpls function
* @endinternal
*
* @brief   build legacy key needed UDBs - mpls key
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - the key type that need mpls legacy style
*
* @retval GT_OK                    - on success
*/
GT_STATUS sip5BuildUdbsNeededForLegacyKey_mpls
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT keyType
)
{
    GT_STATUS  rc;
    GT_U32 index;
    GT_U32  startIndex;
    GT_U32  numOfUdbs;
    TTI_BYTES_FROM_PACKET_ENT partsArr[]=
    {
        TTI_BYTES_FROM_PACKET_MPLS_TAG_1_LABEL_E,
        TTI_BYTES_FROM_PACKET_MPLS_TAG_2_LABEL_E,
        TTI_BYTES_FROM_PACKET_MPLS_TAG_3_LABEL_E
    };
    GT_U32  ii;

    /* init UDBs for CPSS_DXCH_TTI_KEY_MPLS_E */
    ttiInitUserDefinedByteCommonSet(devNum,keyType);
    startIndex = sip5legacyMacSaDaIndexInKey + 6;/* start after the 6 UDBs of mac sa/da*/

    index = startIndex;


    for(ii = 0 ; ii < (sizeof(partsArr)/sizeof(partsArr[0])) ; ii++)
    {
        rc = wellKnownPacketPartsUsedSet(devNum,keyType,
            partsArr[ii],
            index,
            &numOfUdbs);
        if (rc != GT_OK)
        {
            return rc;
        }

        index += numOfUdbs;
    }

    /* Reserved Label Value
       Control Word Channel Type Profile */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_mpls_metadata10_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    /* Protocol after MPLS
       Reserved Label Exists
       Data after inner label */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_mpls_metadata9_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    return GT_OK;
}

/**
* @internal sip5BuildUdbsNeededForLegacyKey_ethernet function
* @endinternal
*
* @brief   build legacy key needed UDBs - ethernet key
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - the key type that need ethernet legacy style
*
* @retval GT_OK                    - on success
*/
GT_STATUS sip5BuildUdbsNeededForLegacyKey_ethernet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT keyType
)
{
    GT_STATUS  rc;
    GT_U32 index;
    GT_U32  startIndex;
    GT_U32  numOfUdbs;
    TTI_BYTES_FROM_PACKET_ENT partsArr[]=
    {
        TTI_BYTES_FROM_PACKET_L3_ETHERTYPE_E
    };
    GT_U32  ii;

    /* init UDBs for CPSS_DXCH_TTI_KEY_ETH_E */
    ttiInitUserDefinedByteCommonSet(devNum,keyType);
    startIndex = sip5legacyMacSaDaIndexInKey + 6;/* start after the 6 UDBs of mac sa/da*/

    index = startIndex;

    /* UDB15-16: Anchor=Desc; Offset=6,7 */
    /* VLAN1
       UP1
       CFI1 */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_ethernet_metadata14_15_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    for(ii = 0 ; ii < (sizeof(partsArr)/sizeof(partsArr[0])) ; ii++)
    {
        rc = wellKnownPacketPartsUsedSet(devNum,keyType,
            partsArr[ii],
            index,
            &numOfUdbs);
        if (rc != GT_OK)
        {
            return rc;
        }

        index += numOfUdbs;
    }

    /* UDB19-22: Anchor=Desc; Offset=8-11 */
    /* VLAN Tag1 Exists
       Tag0 TPID Index
       Tag1 TPID Index
       DSA QoS Profile
       DSA Source ID [11:0] */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_ethernet_metadata16_19_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    return GT_OK;
}

/**
* @internal sip5BuildUdbsNeededForLegacyKey_mim function
* @endinternal
*
* @brief   build legacy key needed UDBs - mim key (802.1ah)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - the key type that need mim legacy style
*
* @retval GT_OK                    - on success
*/
GT_STATUS sip5BuildUdbsNeededForLegacyKey_mim
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT keyType
)
{
    GT_STATUS  rc;
    GT_U32 index;
    GT_U32  startIndex;
    GT_U32  numOfUdbs;
    TTI_BYTES_FROM_PACKET_ENT partsArr[]=
    {
        TTI_BYTES_FROM_PACKET_L3_MIM_I_SID_E,
        TTI_BYTES_FROM_PACKET_L3_MIM_I_SERVICE_PRI_E
    };
    GT_U32  ii;

    /* init UDBs for CPSS_DXCH_TTI_KEY_MIM_E */
    ttiInitUserDefinedByteCommonSet(devNum,keyType);
    startIndex = sip5legacyMacSaDaIndexInKey + 6;/* start after the 6 UDBs of mac sa/da*/

    index = startIndex;

    /* UDB14: Anchor=Desc; Offset=3 */
    /* UP0 (B-UP)
       CFI0 (B-DEI)
       MAC2ME
       Passenger S-tag DEI */
    for(ii = 0 ; ii < (sizeof(partsArr)/sizeof(partsArr[0])) ; ii++)
    {
        rc = wellKnownPacketPartsUsedSet(devNum,keyType,
            partsArr[ii],
            index,
            &numOfUdbs);
        if (rc != GT_OK)
        {
            return rc;
        }

        index += numOfUdbs;
    }

    /* UDB19-20: Anchor=Desc; Offset=4,5 */
    /* Passenger S-tag VID
       Passenger S-tag UP
       Passenger S-tag exists */
    rc = metaDataFeildsUsedSet(devNum,
            keyType,
            PRV_CPSS_DXCH_TTI_ARRAY_AND_SIZE_MAC(legacyKey_mim_metadata12_13_fieldsArr),
            index,
            &numOfUdbs);
    if (rc != GT_OK)
    {
        return rc;
    }

    index += numOfUdbs;

    return GT_OK;
}


/**
* @internal prvCpssDxChTtiDbTablesInit function
* @endinternal
*
* @brief   init TTI tables internal DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS  prvCpssDxChTtiDbTablesInit
(
    IN GT_U8   devNum
)
{
    GT_STATUS  rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssFillFieldsStartBitInfo(TTI_META_DATA_FIELDS___LAST_VALUE___E,
                ttiMetaDataFieldsFormat);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssFillFieldsStartBitInfo(TTI_LEGACY_KEY_FIELDS___LAST_VALUE___E,
                ttiLegacyKeyFieldsFormat);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* state the needed parts in packet,metadata that needed for legacy IPv4 key */
        rc = sip5BuildUdbsNeededForLegacyKey_ipv4(devNum,CPSS_DXCH_TTI_KEY_IPV4_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* state the needed parts in packet,metadata that needed for legacy mpls key */
        rc = sip5BuildUdbsNeededForLegacyKey_mpls(devNum,CPSS_DXCH_TTI_KEY_MPLS_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* state the needed parts in packet,metadata that needed for legacy ethernet key */
        rc = sip5BuildUdbsNeededForLegacyKey_ethernet(devNum,CPSS_DXCH_TTI_KEY_ETH_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* state the needed parts in packet,metadata that needed for legacy mim key */
        rc = sip5BuildUdbsNeededForLegacyKey_mim(devNum,CPSS_DXCH_TTI_KEY_MIM_E);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN CPSS_DXCH_TTI_KEY_SIZE_ENT           size
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      entryIndex;
    GT_U32      entrySize;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E |CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = ttiGetIndexForKeyType(keyType, GT_FALSE, &entryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (size)
    {
    case CPSS_DXCH_TTI_KEY_SIZE_10_B_E:
        entrySize = 0;
        break;
    case CPSS_DXCH_TTI_KEY_SIZE_20_B_E:
        entrySize = 1;
        break;
    case CPSS_DXCH_TTI_KEY_SIZE_30_B_E:
        entrySize = 2;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];

    return prvCpssHwPpSetRegField(devNum, regAddr, 16 * (entryIndex % 2) + 10, 2, entrySize);

}

/**
* @internal cpssDxChTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN CPSS_DXCH_TTI_KEY_SIZE_ENT           size
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPacketTypeKeySizeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, size));

    rc = internal_cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, size);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, size));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
static GT_STATUS internal_cpssDxChTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    OUT CPSS_DXCH_TTI_KEY_SIZE_ENT           *sizePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      entryIndex;
    GT_U32      entrySize;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);

    rc = ttiGetIndexForKeyType(keyType, GT_FALSE, &entryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPktTypeKeys[entryIndex / 2];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, (16 * (entryIndex % 2)) + 10, 2, &entrySize);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (entrySize)
    {
    case 0:
        *sizePtr = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;
        break;
    case 1:
        *sizePtr = CPSS_DXCH_TTI_KEY_SIZE_20_B_E;
        break;
    case 2:
        *sizePtr = CPSS_DXCH_TTI_KEY_SIZE_30_B_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    OUT CPSS_DXCH_TTI_KEY_SIZE_ENT           *sizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPacketTypeKeySizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, sizePtr));

    rc = internal_cpssDxChTtiPacketTypeKeySizeGet(devNum, keyType, sizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, sizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal tcamSegmentModeInfoGet function
* @endinternal
*
* @brief   Gets the info need to use for tcam segment configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - the key type
*
* @param[out] startBitPtr              - (pointer to) the start bit in HW
* @param[out] numOfBitsPtr             - (pointer to) the number of bits in HW
* @param[out] regAddrPtr               - (pointer to) the register to read/write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong key type
*/
static GT_STATUS tcamSegmentModeInfoGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_U32                              *startBitPtr,
    OUT GT_U32                              *numOfBitsPtr,
    OUT GT_U32                              *regAddrPtr
)
{
    GT_STATUS   rc;
    GT_U32      offset;

    rc = ttiGetIndexForKeyType(keyType, GT_TRUE, &offset);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        *startBitPtr = (offset % 10) * 3;
        *numOfBitsPtr = 3;
        if(offset > 10)
        {
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIKeySegmentMode1;
        }
        else
        {
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIKeySegmentMode0;
        }
    }
    else
    {
        *startBitPtr  = offset;
        *numOfBitsPtr = 1;
        *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIKeySegmentMode0;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
* @param[in] segmentMode              - TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiTcamSegmentModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT               keyType,
    IN CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT  segmentMode
)
{
    GT_U32      regAddr;
    GT_U32      startBit;
    GT_U32      value;
    GT_U32      numOfBits;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = tcamSegmentModeInfoGet(devNum, keyType, &startBit, &numOfBits, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        switch (segmentMode)
        {
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E:
                value = 0;
                break;
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E:
                value = 1;
                break;
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E:
                value = 2;
                break;
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E:
                value = 3;
                break;
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E:
                value = 4;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (segmentMode)
        {
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E:
                value = 0;
                break;
            case CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, numOfBits, value);
}

/**
* @internal cpssDxChTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
* @param[in] segmentMode              - TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTcamSegmentModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT               keyType,
    IN CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT  segmentMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTcamSegmentModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, segmentMode));

    rc = internal_cpssDxChTtiTcamSegmentModeSet(devNum, keyType, segmentMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, segmentMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
*
* @param[out] segmentModePtr           - (pointer to) TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiTcamSegmentModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType,
    OUT CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT *segmentModePtr
)
{
    GT_U32      regAddr;
    GT_U32      startBit;
    GT_U32      value;
    GT_U32      numOfBits;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(segmentModePtr);

    rc = tcamSegmentModeInfoGet(devNum, keyType, &startBit, &numOfBits, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, startBit, numOfBits, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        switch (value)
        {
            case 0:
                *segmentModePtr = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;
                break;
            case 1:
                *segmentModePtr = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E;
                break;
            case 2:
                *segmentModePtr = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E;
                break;
            case 3:
                *segmentModePtr = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E;
                break;
            case 4:
                *segmentModePtr = CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        *segmentModePtr =
            ((value == 0) ?
             CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E :
             CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E);
    }

    return rc;
}

/**
* @internal cpssDxChTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
*
* @param[out] segmentModePtr           - (pointer to) TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTcamSegmentModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType,
    OUT CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT *segmentModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiTcamSegmentModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, segmentModePtr));

    rc = internal_cpssDxChTtiTcamSegmentModeGet(devNum, keyType, segmentModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, segmentModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMcTunnelDuplicationModeSet function
* @endinternal
*
* @brief   Set the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
* @param[in] mode                     - the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMcTunnelDuplicationModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol,
    IN CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode
)
{
    GT_U32      field;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* check mode validity */
    if ((protocol == CPSS_TUNNEL_MULTICAST_IPV4_E) ||
        (protocol == CPSS_TUNNEL_MULTICAST_IPV6_E))
    {
        if (mode > CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if ((protocol == CPSS_TUNNEL_MULTICAST_MPLS_E) ||
        (protocol == CPSS_TUNNEL_MULTICAST_TRILL_E) ||
        (protocol == CPSS_TUNNEL_MULTICAST_PBB_E))
    {
        if ((mode != CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E) &&
            (mode != CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    switch (protocol)
    {
        case CPSS_TUNNEL_MULTICAST_IPV4_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_IPV6_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_MPLS_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_TRILL_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_PBB_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (mode)
    {
        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E:
            hwData = 0;
            break;

        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E:
            hwData = 1;
            break;

        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_GRE_E:
            hwData = 2;
            break;

        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_GRE_E:
            hwData = 3;
            break;

        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_UDP_PORT_E:
            hwData = 4;
            break;

        case CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E:
            hwData = 5;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write to pre-tti-lookup-ingress-eport table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           field,
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           hwData);
}

/**
* @internal cpssDxChTtiMcTunnelDuplicationModeSet function
* @endinternal
*
* @brief   Set the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
* @param[in] mode                     - the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol,
    IN CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMcTunnelDuplicationModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, protocol, mode));

    rc = internal_cpssDxChTtiMcTunnelDuplicationModeSet(devNum, portNum, protocol, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, protocol, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMcTunnelDuplicationModeGet function
* @endinternal
*
* @brief   Get the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
*
* @param[out] modePtr                  - (pointer to) the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMcTunnelDuplicationModeGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN  CPSS_TUNNEL_MULTICAST_TYPE_ENT                  protocol,
    OUT CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT    *modePtr
)
{
    GT_U32      field;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    switch (protocol)
    {
        case CPSS_TUNNEL_MULTICAST_IPV4_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_IPV6_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_MPLS_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_TRILL_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE_E;
            break;

        case CPSS_TUNNEL_MULTICAST_PBB_E:
            field = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read from pre-tti-lookup-ingress-eport table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        field,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    if (rc == GT_OK)
    {
        switch (hwData)
        {
            case 0:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E;
                break;

            case 1:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E;
                break;

            case 2:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_GRE_E;
                break;

            case 3:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_GRE_E;
                break;

            case 4:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_UDP_PORT_E;
                break;

            case 5:
                *modePtr = CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    return rc;
}

/**
* @internal cpssDxChTtiMcTunnelDuplicationModeGet function
* @endinternal
*
* @brief   Get the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
*
* @param[out] modePtr                  - (pointer to) the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationModeGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN  CPSS_TUNNEL_MULTICAST_TYPE_ENT                  protocol,
    OUT CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMcTunnelDuplicationModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, protocol, modePtr));

    rc = internal_cpssDxChTtiMcTunnelDuplicationModeGet(devNum, portNum, protocol, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, protocol, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortSet function
* @endinternal
*
* @brief   Set the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] udpPort                  - the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
static GT_STATUS internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortSet
(
    IN GT_U8    devNum,
    IN GT_U16   udpPort
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.duplicationConfigs1;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, (GT_U32)udpPort);
}

/**
* @internal cpssDxChTtiMcTunnelDuplicationUdpDestPortSet function
* @endinternal
*
* @brief   Set the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] udpPort                  - the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationUdpDestPortSet
(
    IN GT_U8    devNum,
    IN GT_U16   udpPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMcTunnelDuplicationUdpDestPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpPort));

    rc = internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortSet(devNum, udpPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortGet function
* @endinternal
*
* @brief   Get the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] udpPortPtr               - (pointer to) the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
static GT_STATUS internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortGet
(
    IN GT_U8    devNum,
    OUT GT_U16  *udpPortPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udpPortPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.duplicationConfigs1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &value);
    if (rc == GT_OK)
    {
        *udpPortPtr = (GT_U16)value;
    }
    return rc;
}

/**
* @internal cpssDxChTtiMcTunnelDuplicationUdpDestPortGet function
* @endinternal
*
* @brief   Get the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] udpPortPtr               - (pointer to) the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationUdpDestPortGet
(
    IN GT_U8    devNum,
    OUT GT_U16  *udpPortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMcTunnelDuplicationUdpDestPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpPortPtr));

    rc = internal_cpssDxChTtiMcTunnelDuplicationUdpDestPortGet(devNum, udpPortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpPortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaSet function
* @endinternal
*
* @brief   Set the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to) the MAC DA
* @param[in] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaSet
(
    IN GT_U8            devNum,
    IN GT_ETHERADDR     *addressPtr,
    IN GT_ETHERADDR     *maskPtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(addressPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDALow;
    hwData = (addressPtr->arEther[5]) |
             (addressPtr->arEther[4] << 8) |
             (addressPtr->arEther[3] << 16);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDAHigh;
    hwData = (addressPtr->arEther[2]) |
             (addressPtr->arEther[1] << 8) |
             (addressPtr->arEther[0] << 16);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDALowMask;
    hwData = (maskPtr->arEther[5]) |
             (maskPtr->arEther[4] << 8) |
             (maskPtr->arEther[3] << 16);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDAHighMask;
    hwData = (maskPtr->arEther[2]) |
             (maskPtr->arEther[1] << 8) |
             (maskPtr->arEther[0] << 16);
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, hwData);
}

/**
* @internal cpssDxChTtiMplsMcTunnelTriggeringMacDaSet function
* @endinternal
*
* @brief   Set the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to) the MAC DA
* @param[in] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMplsMcTunnelTriggeringMacDaSet
(
    IN GT_U8            devNum,
    IN GT_ETHERADDR     *addressPtr,
    IN GT_ETHERADDR     *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMplsMcTunnelTriggeringMacDaSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addressPtr, maskPtr));

    rc = internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaSet(devNum, addressPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, addressPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaGet function
* @endinternal
*
* @brief   Get the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to) the MAC DA
* @param[out] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaGet
(
    IN GT_U8            devNum,
    OUT GT_ETHERADDR    *addressPtr,
    OUT GT_ETHERADDR    *maskPtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(addressPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDALow;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    addressPtr->arEther[5] = (GT_U8)(hwData & 0xFF);
    addressPtr->arEther[4] = (GT_U8)((hwData >> 8) & 0xFF);
    addressPtr->arEther[3] = (GT_U8)((hwData >> 16) & 0xFF);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDAHigh;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    addressPtr->arEther[2] = (GT_U8)(hwData & 0xFF);
    addressPtr->arEther[1] = (GT_U8)((hwData >> 8) & 0xFF);
    addressPtr->arEther[0] = (GT_U8)((hwData >> 16) & 0xFF);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDALowMask;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    maskPtr->arEther[5] = (GT_U8)(hwData & 0xFF);
    maskPtr->arEther[4] = (GT_U8)((hwData >> 8) & 0xFF);
    maskPtr->arEther[3] = (GT_U8)((hwData >> 16) & 0xFF);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).duplicationConfigs.MPLSMCDAHighMask;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    maskPtr->arEther[2] = (GT_U8)(hwData & 0xFF);
    maskPtr->arEther[1] = (GT_U8)((hwData >> 8) & 0xFF);
    maskPtr->arEther[0] = (GT_U8)((hwData >> 16) & 0xFF);

    return GT_OK;
}

/**
* @internal cpssDxChTtiMplsMcTunnelTriggeringMacDaGet function
* @endinternal
*
* @brief   Get the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to) the MAC DA
* @param[out] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMplsMcTunnelTriggeringMacDaGet
(
    IN GT_U8            devNum,
    OUT GT_ETHERADDR    *addressPtr,
    OUT GT_ETHERADDR    *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiMplsMcTunnelTriggeringMacDaGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addressPtr, maskPtr));

    rc = internal_cpssDxChTtiMplsMcTunnelTriggeringMacDaGet(devNum, addressPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, addressPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
static GT_STATUS internal_cpssDxChTtiPwCwExceptionCmdSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT    exceptionType,
    IN CPSS_PACKET_CMD_ENT                  command
)
{
    GT_U32      regAddr;
    GT_U32      offset;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_LION2_E);

    if ((PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_FALSE) &&
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 0;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 3;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E:
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
                offset = 6;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
                offset = 1;
            }
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 9;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 12;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);
            switch (command)
            {
                case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

                default:
                    /* nothing to do */
                    break;
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
            offset = 12;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);
    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 3, value);
}

/**
* @internal cpssDxChTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
GT_STATUS cpssDxChTtiPwCwExceptionCmdSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT    exceptionType,
    IN CPSS_PACKET_CMD_ENT                  command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwExceptionCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, command));

    rc = internal_cpssDxChTtiPwCwExceptionCmdSet(devNum, exceptionType, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
static GT_STATUS internal_cpssDxChTtiPwCwExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_U32      regAddr;
    GT_U32      offset;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    if ((PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_FALSE) &&
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 0;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 3;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E:
            if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
                offset = 6;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
                offset = 1;
            }
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 9;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E:
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;
            offset = 12;
            break;

        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
            offset = 12;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 3, &value);
    if (rc == GT_OK)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr,value);
    }
    return rc;
}

/**
* @internal cpssDxChTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
GT_STATUS cpssDxChTtiPwCwExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwExceptionCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, commandPtr));

    rc = internal_cpssDxChTtiPwCwExceptionCmdGet(devNum, exceptionType, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..251)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPwCwCpuCodeBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   cpuCodeBase
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if ((cpuCodeBase < CPU_CODE_BASE_MIN_CNS) ||
        (cpuCodeBase > PRV_CPSS_DXCH_TTI_PWE3_CPU_CODE_BASE_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;

    return prvCpssHwPpSetRegField(devNum, regAddr, 15, 8, cpuCodeBase);
}

/**
* @internal cpssDxChTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..251)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwCpuCodeBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   cpuCodeBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwCpuCodeBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodeBase));

    rc = internal_cpssDxChTtiPwCwCpuCodeBaseSet(devNum, cpuCodeBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodeBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - (pointer to) the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPwCwCpuCodeBaseGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *cpuCodeBasePtr
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodeBasePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;

    return prvCpssHwPpGetRegField(devNum, regAddr, 15, 8, cpuCodeBasePtr);
}

/**
* @internal cpssDxChTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - (pointer to) the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwCpuCodeBaseGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *cpuCodeBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwCpuCodeBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodeBasePtr));

    rc = internal_cpssDxChTtiPwCwCpuCodeBaseGet(devNum, cpuCodeBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodeBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChTtiPwCwExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code for packets trapped or mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for. valid values:
*                                      CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E
*                                      CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E
* @param[in] cpuCode                  - CPU code for packets trapped or mirrored to the CPU.
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            cpuCode
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */
    GT_U32      offset;     /* offset in the register */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;   /* DSA  Cpu Code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | 
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    if(cpuCode >= BIT_9)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E:
            offset = 4;
            break;
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E:
            offset = 15;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode,&dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write value to hardware register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 8, (GT_U32)dsaCpuCode);

    return rc;
}

/**
* @internal cpssDxChTtiPwCwExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code for packets trapped or mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for. valid values:
*                                      CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E
*                                      CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E
*
* @param[out] cpuCodePtr               - (pointer to) CPU code for packets trapped or mirrored by the feature.
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChTtiPwCwExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *cpuCodePtr
)
{
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* function return code */
    GT_U32      offset;     /* offset in the register */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | 
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);
    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E:
            offset = 4;
            break;
        case CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E:
            offset = 15;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read value from hardware register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttiPwConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 8, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)value, cpuCodePtr);
    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwSequencingSupportEnableSet function
* @endinternal
*
* @brief   This function enables/disables the check for Pseudo Wire-Control Word
*         Data Word format <Sequence Number>==0 in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: PW-CW sequencing supported
*                                      GT_FALSE: PW-CW sequencing is not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPwCwSequencingSupportEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_U32      regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;

    return prvCpssHwPpSetRegField(devNum,regAddr,23,1,(BOOL2BIT_MAC(enable)));

}

/**
* @internal cpssDxChTtiPwCwSequencingSupportEnableSet function
* @endinternal
*
* @brief   This function enables/disables the check for Pseudo Wire-Control Word
*         Data Word format <Sequence Number>==0 in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: PW-CW sequencing supported
*                                      GT_FALSE: PW-CW sequencing is not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwSequencingSupportEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTtiPwCwSequencingSupportEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiPwCwSequencingSupportEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of the check
*         for Pseudo Wire-Control Word Data Word format <Sequence Number>==0
*         in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable PW-CW sequencing support
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiPwCwSequencingSupportEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value = 0;  /* hardware write */
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* write value */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).pseudoWire.PWE3Config;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,23,1,&value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChTtiPwCwSequencingSupportEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of the check
*         for Pseudo Wire-Control Word Data Word format <Sequence Number>==0
*         in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable PW-CW sequencing support
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiPwCwSequencingSupportEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTtiPwCwSequencingSupportEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*       To disable a configurable header, set its value to one of the above
*       values.
*
*/
static GT_STATUS internal_cpssDxChTtiIPv6ExtensionHeaderSet
(
    IN GT_U8    devNum,
    IN GT_U32   extensionHeaderId,
    IN GT_U32   extensionHeaderValue
)
{
    GT_U32      regAddr;
    GT_U32      offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (extensionHeaderId)
    {
        case 0:
            offset = 0;
            break;

        case 1:
            offset = 8;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (extensionHeaderValue > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6ExtensionValue;

    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 8, extensionHeaderValue);
}

/**
* @internal cpssDxChTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*       To disable a configurable header, set its value to one of the above
*       values.
*
*/
GT_STATUS cpssDxChTtiIPv6ExtensionHeaderSet
(
    IN GT_U8    devNum,
    IN GT_U32   extensionHeaderId,
    IN GT_U32   extensionHeaderValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIPv6ExtensionHeaderSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, extensionHeaderId, extensionHeaderValue));

    rc = internal_cpssDxChTtiIPv6ExtensionHeaderSet(devNum, extensionHeaderId, extensionHeaderValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, extensionHeaderId, extensionHeaderValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] extensionHeaderValuePtr  - the configurable extension header value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*
*/
static GT_STATUS internal_cpssDxChTtiIPv6ExtensionHeaderGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  extensionHeaderId,
    OUT GT_U32  *extensionHeaderValuePtr
)
{
    GT_U32      regAddr;
    GT_U32      offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(extensionHeaderValuePtr);

    switch (extensionHeaderId)
    {
        case 0:
            offset = 0;
            break;

        case 1:
            offset = 8;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6ExtensionValue;

    return prvCpssHwPpGetRegField(devNum, regAddr, offset, 8, extensionHeaderValuePtr);
}

/**
* @internal cpssDxChTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] extensionHeaderValuePtr  - the configurable extension header value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*
*/
GT_STATUS cpssDxChTtiIPv6ExtensionHeaderGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  extensionHeaderId,
    OUT GT_U32  *extensionHeaderValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiIPv6ExtensionHeaderGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, extensionHeaderId, extensionHeaderValuePtr));

    rc = internal_cpssDxChTtiIPv6ExtensionHeaderGet(devNum, extensionHeaderId, extensionHeaderValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, extensionHeaderId, extensionHeaderValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiSourceIdBitsOverrideSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookup,
    IN GT_U32   overrideBitmap
)
{
    GT_U32      regAddr;
    GT_U32      maxNumOfLookups;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    maxNumOfLookups = PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) ? 4 : 2;

    if(lookup > maxNumOfLookups-1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((CHECK_BITS_DATA_RANGE_MAC(overrideBitmap, 12)) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = lookup < 2 ? PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sourceIDAssignment0 :
                           PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sourceIDAssignment1 ;

    return prvCpssHwPpSetRegField(devNum, regAddr, (lookup%2)*12, 12, overrideBitmap);
}

/**
* @internal cpssDxChTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiSourceIdBitsOverrideSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookup,
    IN GT_U32   overrideBitmap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiSourceIdBitsOverrideSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lookup, overrideBitmap));

    rc = internal_cpssDxChTtiSourceIdBitsOverrideSet(devNum, lookup, overrideBitmap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lookup, overrideBitmap));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiSourceIdBitsOverrideGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  lookup,
    OUT GT_U32  *overrideBitmapPtr
)
{
    GT_U32      regAddr;
    GT_U32      maxNumOfLookups;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(overrideBitmapPtr);

    maxNumOfLookups = PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) ? 4 : 2;

    if(lookup > maxNumOfLookups-1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = lookup < 2 ? PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sourceIDAssignment0 :
                           PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sourceIDAssignment1 ;

    return prvCpssHwPpGetRegField(devNum, regAddr, (lookup%2)*12, 12, overrideBitmapPtr);
}

/**
* @internal cpssDxChTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiSourceIdBitsOverrideGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  lookup,
    OUT GT_U32  *overrideBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiSourceIdBitsOverrideGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lookup, overrideBitmapPtr));

    rc = internal_cpssDxChTtiSourceIdBitsOverrideGet(devNum, lookup, overrideBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lookup, overrideBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiGreExtensionsEnableSet function
* @endinternal
*
* @brief   Enable/Disable parsing of extensions (Checksum, Sequence, Key) on
*         IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable GRE extensions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiGreExtensionsEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;

    hwData = ((enable == GT_FALSE) ? 1 : 0);

    return prvCpssHwPpSetRegField(devNum, regAddr, 25, 1, hwData);
}

/**
* @internal cpssDxChTtiGreExtensionsEnableSet function
* @endinternal
*
* @brief   Enable/Disable parsing of extensions (Checksum, Sequence, Key) on
*         IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable GRE extensions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiGreExtensionsEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiGreExtensionsEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTtiGreExtensionsEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiGreExtensionsEnableGet function
* @endinternal
*
* @brief   Gt the enabling status of the parsing of extensions (Checksum, Sequence,
*         Key) on IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GRE extensions enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiGreExtensionsEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 25, 1, &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = ((hwData == 0) ? GT_TRUE : GT_FALSE);
    }
    return rc;
}

/**
* @internal cpssDxChTtiGreExtensionsEnableGet function
* @endinternal
*
* @brief   Gt the enabling status of the parsing of extensions (Checksum, Sequence,
*         Key) on IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GRE extensions enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiGreExtensionsEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiGreExtensionsEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTtiGreExtensionsEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type.(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    return cpssDxChTtiPortGroupUserDefinedByteSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        keyType, udbIndex, offsetType, offset);
}

/**
* @internal cpssDxChTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type.(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiUserDefinedByteSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, udbIndex, offsetType, offset));

    rc = internal_cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, udbIndex, offsetType, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
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
static GT_STATUS internal_cpssDxChTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_U32                              udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
{
    return cpssDxChTtiPortGroupUserDefinedByteGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        keyType, udbIndex, offsetTypePtr, offsetPtr);
}

/**
* @internal cpssDxChTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
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
GT_STATUS cpssDxChTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_U32                              udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiUserDefinedByteGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, udbIndex, offsetTypePtr, offsetPtr));

    rc = internal_cpssDxChTtiUserDefinedByteGet(devNum, keyType, udbIndex, offsetTypePtr, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, udbIndex, offsetTypePtr, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeForwardingEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEGlobalConfig;

    hwData = ((enable == GT_FALSE) ? 0 : 1);

    return prvCpssHwPpSetRegField(devNum, regAddr, 16, 1, hwData);
}

/**
* @internal cpssDxChTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeForwardingEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeForwardingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTtiFcoeForwardingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeForwardingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 1, &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = ((hwData == 1) ? GT_TRUE : GT_FALSE);
    }
    return rc;
}

/**
* @internal cpssDxChTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeForwardingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeForwardingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTtiFcoeForwardingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeEtherTypeSet
(
    IN GT_U8        devNum,
    IN GT_U16       etherType
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEGlobalConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, etherType);
}

/**
* @internal cpssDxChTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeEtherTypeSet
(
    IN GT_U8        devNum,
    IN GT_U16       etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeEtherTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType));

    rc = internal_cpssDxChTtiFcoeEtherTypeSet(devNum, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - (pointer to) EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeEtherTypeGet
(
    IN  GT_U8       devNum,
    OUT GT_U16      *etherTypePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &hwData);
    if (rc == GT_OK)
    {
        *etherTypePtr = (GT_U16)hwData;
    }
    return rc;
}

/**
* @internal cpssDxChTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - (pointer to) EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeEtherTypeGet
(
    IN  GT_U8       devNum,
    OUT GT_U16      *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeEtherTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherTypePtr));

    rc = internal_cpssDxChTtiFcoeEtherTypeGet(devNum, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableSet function
* @endinternal
*
* @brief   Enable FCoE Exception Configuration.
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
* @param[in] enable                   - enable/disable FCoE Exception
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    IN GT_BOOL                              enable
)
{
    GT_STATUS   rc = GT_OK; /* function return code */
    GT_U32      value;      /* value to write */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in the register */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    /* find register offset according to the exception type */
    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VER_E:
            offset = 11;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_SOF_E:
            offset = 12;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_R_CTL_E:
            offset = 13;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_HEADER_LENGTH_E:
            offset = 14;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_S_ID_E:
            offset = 15;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_MC_E:
            offset = 16;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_VER_E:
            offset = 17;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_TYPE_E:
            offset = 18;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E:
            offset = 19;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = ((enable == GT_TRUE) ? 1 : 0);

    /* write configuration value */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,offset,1,value);

    return rc;
}

/**
* @internal cpssDxChTtiFcoeExceptionConfiguratiosEnableSet function
* @endinternal
*
* @brief   Enable FCoE Exception Configuration.
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
* @param[in] enable                   - enable/disable FCoE Exception
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionConfiguratiosEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionConfiguratiosEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, enable));

    rc = internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableSet(devNum, exceptionType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableGet function
* @endinternal
*
* @brief   Return FCoE Exception Configuration status
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS   rc = GT_OK; /* function return code */
    GT_U32      hwData;     /* hw value */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in the register */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    /* find register offset according to the exception type */
    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VER_E:
            offset = 11;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_SOF_E:
            offset = 12;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_R_CTL_E:
            offset = 13;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_HEADER_LENGTH_E:
            offset = 14;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_S_ID_E:
            offset = 15;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_MC_E:
            offset = 16;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_VER_E:
            offset = 17;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_TYPE_E:
            offset = 18;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E:
            offset = 19;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read configuration value */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = ((hwData == 1) ? GT_TRUE : GT_FALSE);
    }
    return rc;
}

/**
* @internal cpssDxChTtiFcoeExceptionConfiguratiosEnableGet function
* @endinternal
*
* @brief   Return FCoE Exception Configuration status
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionConfiguratiosEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionConfiguratiosEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, enablePtr));

    rc = internal_cpssDxChTtiFcoeExceptionConfiguratiosEnableGet(devNum, exceptionType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionCountersGet function
* @endinternal
*
* @brief   Return number of FCoE exceptions of a given type.
*         When the counter reaches 0xFF it sticks to this value, i.e., it does not wrap around
*         The counter is clear on read
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] counterValuePtr          - (pointer to) number of FCoE exceptions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeExceptionCountersGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    OUT GT_U32                              *counterValuePtr
)
{
    GT_STATUS   rc = GT_OK; /* function return code */
    GT_U32      hwData;     /* hw value */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* offset in the register */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(counterValuePtr);

    /* find register offset according to the exception type */
    switch (exceptionType)
    {
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VER_E:
            offset = 0;
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs0;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_SOF_E:
            offset = 8;
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs0;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_R_CTL_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs0;
            offset = 16;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_HEADER_LENGTH_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs0;
            offset = 24;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_S_ID_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs1;
            offset = 0;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_MC_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs1;
            offset = 8;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_VER_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs1;
            offset = 16;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_TYPE_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs1;
            offset = 24;
            break;
        case CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E:
            offset = 0;
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionCntrs2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read configuration value */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 8, &hwData);
    if (rc == GT_OK)
    {
        *counterValuePtr = hwData;
    }
    return rc;
}

/**
* @internal cpssDxChTtiFcoeExceptionCountersGet function
* @endinternal
*
* @brief   Return number of FCoE exceptions of a given type.
*         When the counter reaches 0xFF it sticks to this value, i.e., it does not wrap around
*         The counter is clear on read
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] counterValuePtr          - (pointer to) number of FCoE exceptions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCountersGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT    exceptionType,
    OUT GT_U32                             *counterValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exceptionType, counterValuePtr));

    rc = internal_cpssDxChTtiFcoeExceptionCountersGet(devNum, exceptionType, counterValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, exceptionType, counterValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - enable/disable assignment of VRF-ID with the value VF_ID-ID with the value VF_ID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiFcoeAssignVfIdEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc = GT_OK;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

   /* write to pre-tti-lookup-ingress-eport table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ASSIGN_VF_ID_EN_E,/* field name */
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           BOOL2BIT_MAC(enable));
    return rc;
}

/**
* @internal cpssDxChTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - enable/disable assignment of VRF-ID with the value VF_ID-ID with the value VF_ID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeAssignVfIdEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeAssignVfIdEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChTtiFcoeAssignVfIdEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if VRF-ID is assigned with the value VF_ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for FCoE packets which contain a VF Tag
*
*/
static GT_STATUS internal_cpssDxChTtiFcoeAssignVfIdEnableGet
(
    IN  GT_U8       devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      data=0;
    GT_STATUS   rc = GT_OK;           /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ASSIGN_VF_ID_EN_E,/* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &data);
    if (rc == GT_OK)
    {
        *enablePtr = BIT2BOOL_MAC(data);
    }
    return rc;
}

/**
* @internal cpssDxChTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if VRF-ID is assigned with the value VF_ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for FCoE packets which contain a VF Tag
*
*/
GT_STATUS cpssDxChTtiFcoeAssignVfIdEnableGet
(
    IN  GT_U8       devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeAssignVfIdEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChTtiFcoeAssignVfIdEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static GT_STATUS internal_cpssDxChTtiFcoeExceptionPacketCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
)
{
    GT_U32      regAddr;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    return prvCpssHwPpSetRegField(devNum, regAddr, 8, 3, value);
}

/**
* @internal cpssDxChTtiFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiFcoeExceptionPacketCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionPacketCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, command));

    rc = internal_cpssDxChTtiFcoeExceptionPacketCommandSet(devNum, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static GT_STATUS internal_cpssDxChTtiFcoeExceptionPacketCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 3, &value);
    if (rc == GT_OK)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr,value);
    }
    return rc;

}

/**
* @internal cpssDxChTtiFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiFcoeExceptionPacketCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionPacketCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, commandPtr));

    rc = internal_cpssDxChTtiFcoeExceptionPacketCommandGet(devNum, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChTtiFcoeExceptionCpuCodeSet
(
    IN GT_U8 devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      value;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  hwCpuCode;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &hwCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = hwCpuCode;

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, value);
}

/**
* @internal cpssDxChTtiFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCpuCodeSet
(
    IN GT_U8 devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCode));

    rc = internal_cpssDxChTtiFcoeExceptionCpuCodeSet(devNum, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] cpuCodePtr               - points to the CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChTtiFcoeExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *cpuCodePtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).FCOE.FCoEExceptionConfigs;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &value);
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    return prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)value,
                                      cpuCodePtr);
}

/**
* @internal cpssDxChTtiFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] cpuCodePtr               - points to the CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiFcoeExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodePtr));

    rc = internal_cpssDxChTtiFcoeExceptionCpuCodeGet(devNum, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEcnPacketCommandSet function
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
static GT_STATUS internal_cpssDxChTtiEcnPacketCommandSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  command
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    switch(command)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            value = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            value = 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            value = 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ECNDroppedConfiguration;

    /* write value to register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, value);
}

/**
* @internal cpssDxChTtiEcnPacketCommandSet function
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
GT_STATUS cpssDxChTtiEcnPacketCommandSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEcnPacketCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, command));

    rc = internal_cpssDxChTtiEcnPacketCommandSet(devNum, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEcnPacketCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] commandPtr           - (pointer to) packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_VALUE             - on bad packet command value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiEcnPacketCommandGet
(
    IN GT_U8                                devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
)
{
    GT_STATUS   rc;                 /* return value */
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    /* read value from register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ECNDroppedConfiguration;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 3, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            *commandPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            *commandPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            *commandPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            *commandPtr = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            *commandPtr = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChTtiEcnPacketCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] commandPtr           - (pointer to) packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_VALUE             - on bad packet command value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnPacketCommandGet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEcnPacketCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, commandPtr));

    rc = internal_cpssDxChTtiEcnPacketCommandGet(devNum, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
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
static GT_STATUS internal_cpssDxChTtiEcnCpuCodeSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS   rc;                 /* return value */
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  hwCpuCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &hwCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = hwCpuCode;

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ECNDroppedConfiguration;

    /* write value to register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 8, value);
}

/**
* @internal cpssDxChTtiEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
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
GT_STATUS cpssDxChTtiEcnCpuCodeSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEcnCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCode));

    rc = internal_cpssDxChTtiEcnCpuCodeSet(devNum, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTtiEcnCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] cpuCodePtr           - (pointer to) CPU codecommand.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTtiEcnCpuCodeGet
(
    IN GT_U8                        devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
{
    GT_STATUS   rc;                 /* return value */
    GT_U32      regAddr;            /* register address */
    GT_U32      value;              /* value into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
         CPSS_CAELUM_E| CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    /* read value from register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ECNDroppedConfiguration;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 3, 8, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)value,
                                      cpuCodePtr);
}

/**
* @internal cpssDxChTtiEcnCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] cpuCodePtr           - (pointer to) CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnCpuCodeGet
(
    IN GT_U8                      devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT   *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTtiEcnCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodePtr));

    rc = internal_cpssDxChTtiEcnCpuCodeGet(devNum, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
