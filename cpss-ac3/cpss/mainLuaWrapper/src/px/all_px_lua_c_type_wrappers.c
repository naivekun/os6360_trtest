/*************************************************************************
* /local/store/yedidiaf/git/temp_embede_files/tmp_bld_cpssAPI/all_px_lua_c_type_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           struct  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC
*           enum    CPSS_PX_PORT_MAC_COUNTER_ENT
*           struct  CPSS_PX_PORT_AP_STATS_STC
*           struct  CPSS_PX_LOGICAL_TABLE_INFO_STC
*           struct  CPSS_PX_EXTENDED_DSA_FORWARD_STC
*           struct  CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC
*           struct  CPSS_PX_UDB_PAIR_DATA_STC
*           enum    CPSS_PX_PTP_TAI_TOD_TYPE_ENT
*           struct  CPSS_PX_PORT_CN_PROFILE_CONFIG_STC
*           enum    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT
*           struct  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC
*           enum    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT
*           enum    CPSS_PX_PORT_AP_FLOW_CONTROL_ENT
*           enum    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT
*           enum    CPSS_PX_PORT_PFC_ENABLE_ENT
*           enum    CPSS_PX_PORT_MAPPING_TYPE_ENT
*           enum    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT
*           enum    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT
*           struct  CPSS_PX_DIAG_BIST_RESULT_STC
*           struct  CPSS_PX_LOGICAL_TABLES_SHADOW_STC
*           struct  CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC
*           struct  CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT
*           struct  CPSS_PX_INGRESS_HASH_UDBP_STC
*           union   CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT
*           struct  CPSS_PX_PORT_ECN_ENABLERS_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC
*           enum    CPSS_PX_PP_SERDES_REF_CLOCK_ENT
*           struct  CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC
*           struct  CPSS_PX_UDB_KEY_STC
*           enum    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT
*           struct  CPSS_PX_PORT_AP_INTROP_STC
*           enum    CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT
*           enum    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT
*           enum    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
*           struct  CPSS_PX_LED_CLASS_MANIPULATION_STC
*           struct  CPSS_PX_PORT_MAP_STC
*           struct  CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC
*           struct  CPSS_PX_PTP_TYPE_KEY_STC
*           enum    CPSS_PX_CNC_COUNTER_FORMAT_ENT
*           enum    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC
*           enum    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT
*           struct  CPSS_PX_HW_INDEX_INFO_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT
*           struct  CPSS_PX_DETAILED_PORT_MAP_STC
*           struct  CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT
*           struct  CPSS_PX_COS_FORMAT_ENTRY_STC
*           union   CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT
*           struct  CPSS_PX_UDB_PAIR_KEY_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT
*           struct  CPSS_PX_REGULAR_DSA_FORWARD_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT
*           enum    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT
*           enum    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT
*           union   CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT
*           enum    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT
*           enum    CPSS_PX_COS_MODE_ENT
*           struct  CPSS_PX_SHADOW_PORT_MAP_STC
*           struct  CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC
*           enum    CPSS_PX_INGRESS_ETHERTYPE_ENT
*           struct  CPSS_PX_PTP_TAI_TOD_STEP_STC
*           enum    CPSS_PX_PORT_FC_MODE_ENT
*           enum    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT
*           enum    CPSS_PX_LOCATION_ENT
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT
*           struct  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC
*           enum    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT
*           enum    CPSS_PX_DIAG_TRANSMIT_MODE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC
*           union   CPSS_PX_LOCATION_UNT
*           struct  CPSS_PX_PORT_EGRESS_CNTR_STC
*           struct  CPSS_PX_LED_CONF_STC
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT
*           enum    CPSS_PX_CNC_CLIENT_ENT
*           struct  CPSS_PX_CNC_COUNTER_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
*           struct  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
*           struct  CPSS_PX_PTP_TSU_CONTROL_STC
*           enum    CPSS_PX_DIAG_BIST_STATUS_ENT
*           struct  CPSS_PX_INGRESS_TPID_ENTRY_STC
*           struct  CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC
*           struct  CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC
*           struct  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC
*           enum    CPSS_PX_INGRESS_HASH_MODE_ENT
*           struct  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC
*           enum    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT
*           enum    CPSS_PX_PA_UNIT_ENT
*           struct  CPSS_PX_COS_ATTRIBUTES_STC
*           struct  CPSS_PX_EGRESS_SRC_PORT_INFO_STC
*           enum    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT
*           enum    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT
*           struct  CPSS_PX_PORT_SERDES_EYE_INPUT_STC
*           struct  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC
*           struct  CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC
*           enum    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT
*           struct  CPSS_PX_CFG_DEV_INFO_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC
*           struct  CPSS_PX_PTP_TAI_TOD_COUNT_STC
*           struct  CPSS_PX_VLAN_TAG_STC
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT
*           struct  CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC
*           struct  CPSS_PX_PORT_ALIGN90_PARAMS_STC
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC
*           enum    CPSS_PX_MIN_SPEED_ENT
*           struct  CPSS_PX_NET_TX_PARAMS_STC
*           enum    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC
*           enum    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT
*           enum    CPSS_PX_SHADOW_TYPE_ENT
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT
*           enum    CPSS_PX_PORT_TX_SHAPER_MODE_ENT
*           enum    CPSS_PX_POWER_SUPPLIES_NUMBER_ENT
*           enum    CPSS_PX_PHA_FIRMWARE_TYPE_ENT
*           enum    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT
*           enum    CPSS_PX_PORT_PFC_COUNT_MODE_ENT
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC
*           enum    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT
*           enum    CPSS_PX_PORT_CTLE_BIAS_MODE_ENT
*           struct  CPSS_PX_LOCATION_FULL_INFO_STC
*           struct  CPSS_PX_RAM_INDEX_INFO_STC
*           struct  CPSS_PX_NET_SDMA_RX_COUNTERS_STC
*           enum    CPSS_PX_CFG_CNTR_ENT
*           union   CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT
*           enum    CPSS_PX_LOGICAL_TABLE_ENT
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC
*           struct  CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC
*           enum    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT
*           struct  CPSS_PX_PORT_AP_PARAMS_STC
*           struct  CPSS_PX_PORT_AP_STATUS_STC
*           enum    CPSS_PX_TABLE_ENT
*           enum    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT
*           struct  CPSS_PX_PORT_MAC_COUNTERS_STC
*           enum    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT
*
* DEPENDENCIES:
*
* COMMENTS:
*       Generated at Thu Aug 30 21:13:55 2018
*       /local/store/yedidiaf/git/src/linux_cpss.super/cpss.super/cpss/mainLuaWrapper/scripts/make_type_wrapper.py -i /local/store/yedidiaf/git/temp_embede_files/tmp_bld_cpssAPI/type_info_file_px -p /local/store/yedidiaf/git/src/linux_cpss.super/cpss.super/cpss/mainLuaWrapper/scripts/already_implemented.lst -X /local/store/yedidiaf/git/src/linux_cpss.super/cpss.super/cpss/mainLuaWrapper/scripts/excludelist -C /local/store/yedidiaf/git/src/linux_cpss.super/cpss.super/cpss -M /local/store/yedidiaf/git/src/linux_cpss.super/cpss.super/cpss/mainLuaWrapper/scripts/lua_C_mapping -N -F px -o /local/store/yedidiaf/git/temp_embede_files/tmp_bld_cpssAPI/all_px_lua_c_type_wrappers.c
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
**************************************************************************/
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/px/ptp/cpssPxPtp.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortCn.h>
#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/px/port/cpssPxPortEcn.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/diag/cpssDiagPacketGenerator.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/diag/cpssPxDiagPacketGenerator.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/extServices/os/gtOs/gtOs.h>

/***** declarations ********/

use_prv_struct(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC);
use_prv_enum(CPSS_PX_PORT_MAC_COUNTER_ENT);
use_prv_struct(CPSS_PX_PORT_AP_STATS_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLE_INFO_STC);
use_prv_struct(CPSS_PX_EXTENDED_DSA_FORWARD_STC);
use_prv_struct(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
use_prv_struct(CPSS_PX_UDB_PAIR_DATA_STC);
use_prv_enum(CPSS_PX_PTP_TAI_TOD_TYPE_ENT);
use_prv_struct(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC);
use_prv_enum(CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT);
use_prv_enum(CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
use_prv_struct(CPSS_PORT_MODE_SPEED_STC);
use_prv_struct(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC);
use_prv_struct(CPSS_PX_INGRESS_IP2ME_ENTRY_STC);
use_prv_enum(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);
use_prv_enum(CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
use_prv_enum(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);
use_prv_enum(CPSS_PX_PORT_PFC_ENABLE_ENT);
use_prv_enum(CPSS_PX_PORT_MAPPING_TYPE_ENT);
use_prv_enum(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);
use_prv_enum(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);
use_prv_struct(CPSS_PX_DIAG_BIST_RESULT_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLES_SHADOW_STC);
use_prv_struct(CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
use_prv_struct(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_enum(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
use_prv_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);
use_prv_struct(CPSS_PX_INGRESS_HASH_UDBP_STC);
use_prv_struct(CPSS_PX_PORT_ECN_ENABLERS_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
use_prv_enum(CPSS_PX_PP_SERDES_REF_CLOCK_ENT);
use_prv_struct(CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
use_prv_struct(CPSS_PX_UDB_KEY_STC);
use_prv_enum(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
use_prv_struct(CPSS_PX_PORT_AP_INTROP_STC);
use_prv_enum(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);
use_prv_enum(CPSS_PORT_DUPLEX_ENT);
use_prv_enum(CPSS_PP_FAMILY_TYPE_ENT);
use_prv_enum(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);
use_prv_struct(CPSS_802_1BR_ETAG_STC);
use_prv_enum(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
use_prv_struct(CPSS_PX_LED_CLASS_MANIPULATION_STC);
use_prv_struct(CPSS_PX_PORT_MAP_STC);
use_prv_struct(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC);
use_prv_struct(CPSS_PX_PTP_TYPE_KEY_STC);
use_prv_enum(CPSS_DP_LEVEL_ENT);
use_prv_enum(CPSS_PX_CNC_COUNTER_FORMAT_ENT);
use_prv_enum(CPSS_DIAG_PG_IF_SIZE_ENT);
use_prv_enum(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);
use_prv_enum(CPSS_LED_BLINK_SELECT_ENT);
use_prv_enum(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);
use_prv_struct(CPSS_PX_HW_INDEX_INFO_STC);
use_prv_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);
use_prv_struct(CPSS_PX_DETAILED_PORT_MAP_STC);
use_prv_struct(CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
use_prv_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);
use_prv_struct(CPSS_PX_COS_FORMAT_ENTRY_STC);
use_prv_struct(CPSS_PX_UDB_PAIR_KEY_STC);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);
use_prv_enum(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);
use_prv_struct(CPSS_PX_REGULAR_DSA_FORWARD_STC);
use_prv_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);
use_prv_enum(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);
use_prv_struct(CPSS_PX_DIAG_PG_CONFIGURATIONS_STC);
use_prv_enum(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);
use_prv_enum(CPSS_LED_PULSE_STRETCH_ENT);
use_prv_enum(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);
use_prv_enum(CPSS_PX_COS_MODE_ENT);
use_prv_struct(CPSS_PX_SHADOW_PORT_MAP_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC);
use_prv_enum(CPSS_PX_INGRESS_ETHERTYPE_ENT);
use_prv_struct(CPSS_PX_PTP_TAI_TOD_STEP_STC);
use_prv_enum(CPSS_PX_PORT_FC_MODE_ENT);
use_prv_enum(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);
use_prv_enum(CPSS_PX_LOCATION_ENT);
use_prv_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);
use_prv_struct(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC);
use_prv_enum(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
use_prv_enum(CPSS_PX_DIAG_TRANSMIT_MODE_ENT);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_PORT_EGRESS_CNTR_STC);
use_prv_struct(CPSS_PX_LED_CONF_STC);
use_prv_enum(CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC);
use_prv_enum(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);
use_prv_enum(CPSS_PX_CNC_CLIENT_ENT);
use_prv_struct(CPSS_PX_CNC_COUNTER_STC);
use_prv_enum(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
use_prv_struct(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC);
use_prv_struct(CPSS_PX_PTP_TSU_CONTROL_STC);
use_prv_struct(CPSS_PX_INGRESS_TPID_ENTRY_STC);
use_prv_struct(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC);
use_prv_struct(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
use_prv_enum(CPSS_LED_BLINK_DURATION_ENT);
use_prv_enum(CPSS_PX_DIAG_BIST_STATUS_ENT);
use_prv_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
use_prv_enum(CPSS_PX_INGRESS_HASH_MODE_ENT);
use_prv_struct(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC);
use_prv_enum(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
use_prv_enum(CPSS_LED_ORDER_MODE_ENT);
use_prv_enum(CPSS_PX_PA_UNIT_ENT);
use_prv_struct(CPSS_PX_COS_ATTRIBUTES_STC);
use_prv_struct(CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
use_prv_enum(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
use_prv_enum(CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);
use_prv_enum(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);
use_prv_struct(CPSS_PX_PORT_SERDES_EYE_INPUT_STC);
use_prv_struct(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC);
use_prv_struct(GT_ETHERADDR);
use_prv_struct(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC);
use_prv_enum(CPSS_PORT_FEC_MODE_ENT);
use_prv_enum(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
use_prv_struct(CPSS_GEN_CFG_DEV_INFO_STC);
use_prv_struct(CPSS_PX_CFG_DEV_INFO_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_PTP_TAI_TOD_COUNT_STC);
use_prv_struct(CPSS_PX_VLAN_TAG_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
use_prv_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
use_prv_struct(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC);
use_prv_struct(CPSS_PX_PORT_ALIGN90_PARAMS_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
use_prv_enum(CPSS_PX_MIN_SPEED_ENT);
use_prv_struct(CPSS_PX_NET_TX_PARAMS_STC);
use_prv_enum(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC);
use_prv_enum(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);
use_prv_enum(CPSS_PX_SHADOW_TYPE_ENT);
use_prv_enum(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
use_prv_enum(CPSS_PX_PORT_TX_SHAPER_MODE_ENT);
use_prv_enum(CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);
use_prv_enum(CPSS_DIAG_PG_TRANSMIT_MODE_ENT);
use_prv_enum(CPSS_PX_PHA_FIRMWARE_TYPE_ENT);
use_prv_enum(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);
use_prv_enum(CPSS_PX_PORT_PFC_COUNT_MODE_ENT);
use_prv_enum(CPSS_LED_BLINK_DUTY_CYCLE_ENT);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC);
use_prv_enum(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);
use_prv_enum(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);
use_prv_enum(CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);
use_prv_struct(CPSS_PX_LOCATION_FULL_INFO_STC);
use_prv_struct(CPSS_PX_RAM_INDEX_INFO_STC);
use_prv_struct(CPSS_PX_NET_SDMA_RX_COUNTERS_STC);
use_prv_enum(CPSS_PX_CFG_CNTR_ENT);
use_prv_enum(CPSS_PX_LOGICAL_TABLE_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
use_prv_struct(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC);
use_prv_enum(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);
use_prv_struct(GT_U64);
use_prv_struct(CPSS_PX_PORT_AP_PARAMS_STC);
use_prv_struct(CPSS_PX_PORT_AP_STATUS_STC);
use_prv_enum(CPSS_PX_TABLE_ENT);
use_prv_enum(CPSS_PORT_SPEED_ENT);
use_prv_enum(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);
use_prv_struct(CPSS_PX_PORT_MAC_COUNTERS_STC);
use_prv_enum(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);

/***** declarations ********/

void prv_lua_to_c_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC *val
)
{
    F_BOOL(val, -1, isUdp);
    F_NUMBER(val, -1, srcPortProfile, GT_U32);
    F_ARRAY_START(val, -1, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    F_ARRAY_END(val, -1, udbPairsArr);
    F_ARRAY_START(val, -1, udbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, udbArr, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, udbArr);
}

void prv_c_to_lua_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, isUdp);
    FO_NUMBER(val, t, srcPortProfile, GT_U32);
    FO_ARRAY_START(val, t, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    FO_ARRAY_END(val, t, udbPairsArr);
    FO_ARRAY_START(val, t, udbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, udbArr, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, udbArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC);

enumDescr enumDescr_CPSS_PX_PORT_MAC_COUNTER_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_JABBER_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_COLLISION_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_COUNTER____LAST____E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_MAC_COUNTER_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_STATS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATS_STC *val
)
{
    F_NUMBER(val, -1, txDisCnt, GT_U16);
    F_NUMBER(val, -1, abilityCnt, GT_U16);
    F_NUMBER(val, -1, abilitySuccessCnt, GT_U16);
    F_NUMBER(val, -1, linkFailCnt, GT_U16);
    F_NUMBER(val, -1, linkSuccessCnt, GT_U16);
    F_NUMBER(val, -1, hcdResoultionTime, GT_U32);
    F_NUMBER(val, -1, linkUpTime, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_STATS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txDisCnt, GT_U16);
    FO_NUMBER(val, t, abilityCnt, GT_U16);
    FO_NUMBER(val, t, abilitySuccessCnt, GT_U16);
    FO_NUMBER(val, t, linkFailCnt, GT_U16);
    FO_NUMBER(val, t, linkSuccessCnt, GT_U16);
    FO_NUMBER(val, t, hcdResoultionTime, GT_U32);
    FO_NUMBER(val, t, linkUpTime, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_STATS_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLE_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_INFO_STC *val
)
{
    F_ENUM(val, -1, logicalTableType, CPSS_PX_LOGICAL_TABLE_ENT);
    F_NUMBER(val, -1, logicalTableEntryIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLE_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableType, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_NUMBER(val, t, logicalTableEntryIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLE_INFO_STC);

void prv_lua_to_c_CPSS_PX_EXTENDED_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *val
)
{
    F_BOOL(val, -1, srcTagged);
    F_NUMBER(val, -1, hwSrcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, srcPortOrTrunk, GT_U32);
    F_BOOL(val, -1, srcIsTrunk);
    F_NUMBER(val, -1, cfi, GT_U32);
    F_NUMBER(val, -1, up, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
    F_BOOL(val, -1, egrFilterRegistered);
    F_BOOL(val, -1, dropOnSource);
    F_BOOL(val, -1, packetIsLooped);
    F_BOOL(val, -1, wasRouted);
    F_NUMBER(val, -1, srcId, GT_U32);
    F_NUMBER(val, -1, qosProfileIndex, GT_U32);
    F_BOOL(val, -1, useVidx);
    F_NUMBER(val, -1, trgVidx, GT_U32);
    F_NUMBER(val, -1, trgPort, GT_U32);
    F_NUMBER(val, -1, hwTrgDev, GT_HW_DEV_NUM);
}

void prv_c_to_lua_CPSS_PX_EXTENDED_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, srcTagged);
    FO_NUMBER(val, t, hwSrcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, srcPortOrTrunk, GT_U32);
    FO_BOOL(val, t, srcIsTrunk);
    FO_NUMBER(val, t, cfi, GT_U32);
    FO_NUMBER(val, t, up, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    FO_BOOL(val, t, egrFilterRegistered);
    FO_BOOL(val, t, dropOnSource);
    FO_BOOL(val, t, packetIsLooped);
    FO_BOOL(val, t, wasRouted);
    FO_NUMBER(val, t, srcId, GT_U32);
    FO_NUMBER(val, t, qosProfileIndex, GT_U32);
    FO_BOOL(val, t, useVidx);
    FO_NUMBER(val, t, trgVidx, GT_U32);
    FO_NUMBER(val, t, trgPort, GT_U32);
    FO_NUMBER(val, t, hwTrgDev, GT_HW_DEV_NUM);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EXTENDED_DSA_FORWARD_STC);

void prv_lua_to_c_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC *val
)
{
    F_ENUM(val, -1, eventsType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    F_STRUCT(val, -1, location, CPSS_PX_LOCATION_FULL_INFO_STC);
    F_ENUM(val, -1, memoryUseType, CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);
    F_ENUM(val, -1, correctionMethod, CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);
}

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, eventsType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    FO_STRUCT(val, t, location, CPSS_PX_LOCATION_FULL_INFO_STC);
    FO_ENUM(val, t, memoryUseType, CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);
    FO_ENUM(val, t, correctionMethod, CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC *val
)
{
    F_NUMBER(val, -1, srcPortNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcPortNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);

void prv_lua_to_c_CPSS_PX_UDB_PAIR_DATA_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_DATA_STC *val
)
{
    F_ARRAY_START(val, -1, udb);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            F_ARRAY_NUMBER(val, udb, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, udb);
}

void prv_c_to_lua_CPSS_PX_UDB_PAIR_DATA_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, udb);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            FO_ARRAY_NUMBER(val, udb, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, udb);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_PAIR_DATA_STC);

enumDescr enumDescr_CPSS_PX_PTP_TAI_TOD_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TAI_TOD_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC *val
)
{
    F_BOOL(val, -1, cnAware);
    F_NUMBER(val, -1, threshold, GT_U32);
    F_ENUM(val, -1, alpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, cnAware);
    FO_NUMBER(val, t, threshold, GT_U32);
    FO_ENUM(val, t, alpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC);

enumDescr enumDescr_CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT[] = {
    ENUM_ENTRY(CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, tokensRate, GT_U32);
    F_NUMBER(val, -1, slowRateRatio, GT_U32);
    F_ENUM(val, -1, tokensRateGran, CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
    F_NUMBER(val, -1, portsPacketLength, GT_U32);
    F_NUMBER(val, -1, cpuPacketLength, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tokensRate, GT_U32);
    FO_NUMBER(val, t, slowRateRatio, GT_U32);
    FO_ENUM(val, t, tokensRateGran, CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
    FO_NUMBER(val, t, portsPacketLength, GT_U32);
    FO_NUMBER(val, t, cpuPacketLength, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_AP_FLOW_CONTROL_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E),
    ENUM_ENTRY(CPSS_PX_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);

enumDescr enumDescr_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_PFC_ENABLE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E),
    ENUM_ENTRY(CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_PFC_ENABLE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_MAPPING_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAPPING_TYPE_MAX_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAPPING_TYPE_INVALID_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_MAPPING_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E),
    ENUM_ENTRY(CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E),
    ENUM_ENTRY(CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E),
    ENUM_ENTRY(CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_DIAG_BIST_RESULT_STC(
    lua_State *L,
    CPSS_PX_DIAG_BIST_RESULT_STC *val
)
{
    F_ENUM(val, -1, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    F_STRUCT(val, -1, location, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_DIAG_BIST_RESULT_STC(
    lua_State *L,
    CPSS_PX_DIAG_BIST_RESULT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    FO_STRUCT(val, t, location, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_BIST_RESULT_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLES_SHADOW_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLES_SHADOW_STC *val
)
{
    F_ENUM(val, -1, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    F_ENUM(val, -1, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLES_SHADOW_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLES_SHADOW_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_ENUM(val, t, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLES_SHADOW_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC *val
)
{
    F_NUMBER(val, -1, tpid, GT_U16);
    F_NUMBER(val, -1, pcid, GT_U32);
    F_NUMBER(val, -1, egressDelay, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tpid, GT_U16);
    FO_NUMBER(val, t, pcid, GT_U32);
    FO_NUMBER(val, t, egressDelay, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);

void prv_lua_to_c_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    F_ENUM(val, -1, devType, CPSS_PP_FAMILY_TYPE_ENT);
    lua_getfield(L, -1, "devState");
    if (lua_istable(L, -1))
    {
        F_STRUCT(&(val->devState), -1, pipe, CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
    }
    lua_pop(L, 1);
}

void prv_c_to_lua_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, devType, CPSS_PP_FAMILY_TYPE_ENT);
    {
        int t1;
        lua_newtable(L);
        t1 = lua_gettop(L);
            FO_STRUCT(&(val->devState), t1, pipe, CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
        lua_setfield(L, t, "devState");
    }
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_3_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_5_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_6_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_9_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_10_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_11_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_12_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_13_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_14_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);

enumDescr enumDescr_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_HASH_UDBP_STC(
    lua_State *L,
    CPSS_PX_INGRESS_HASH_UDBP_STC *val
)
{
    F_ENUM(val, -1, anchor, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, offset, GT_U32);
    F_ARRAY_START(val, -1, nibbleMaskArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_BOOL(val, nibbleMaskArr, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, nibbleMaskArr);
}

void prv_c_to_lua_CPSS_PX_INGRESS_HASH_UDBP_STC(
    lua_State *L,
    CPSS_PX_INGRESS_HASH_UDBP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, anchor, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, offset, GT_U32);
    FO_ARRAY_START(val, t, nibbleMaskArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_BOOL(val, nibbleMaskArr, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, nibbleMaskArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_HASH_UDBP_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT *val
)
{
    /* just do nothing */
}

void prv_c_to_lua_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_common, CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
    FO_NUMBER(val, t, notNeeded, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT);

void prv_lua_to_c_CPSS_PX_PORT_ECN_ENABLERS_STC(
    lua_State *L,
    CPSS_PX_PORT_ECN_ENABLERS_STC *val
)
{
    F_BOOL(val, -1, tcDpLimit);
    F_BOOL(val, -1, portLimit);
    F_BOOL(val, -1, tcLimit);
    F_BOOL(val, -1, sharedPoolLimit);
}

void prv_c_to_lua_CPSS_PX_PORT_ECN_ENABLERS_STC(
    lua_State *L,
    CPSS_PX_PORT_ECN_ENABLERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, tcDpLimit);
    FO_BOOL(val, t, portLimit);
    FO_BOOL(val, t, tcLimit);
    FO_BOOL(val, t, sharedPoolLimit);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_ECN_ENABLERS_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macSa, GT_ETHERADDR);
    F_STRUCT(val, -1, dsaExtForward, CPSS_PX_EXTENDED_DSA_FORWARD_STC);
    F_NUMBER(val, -1, cnmTpid, GT_U16);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macSa, GT_ETHERADDR);
    FO_STRUCT(val, t, dsaExtForward, CPSS_PX_EXTENDED_DSA_FORWARD_STC);
    FO_NUMBER(val, t, cnmTpid, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);

enumDescr enumDescr_CPSS_PX_PP_SERDES_REF_CLOCK_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E),
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E),
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E),
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E),
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E),
    ENUM_ENTRY(CPSS_PX_PP_SERDES_REF_CLOCK_INTERNAL_125_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PP_SERDES_REF_CLOCK_ENT);

void prv_lua_to_c_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC(
    lua_State *L,
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *val
)
{
    F_NUMBER(val, -1, totalConfiguredSlices, GT_U32);
    F_NUMBER(val, -1, totalSlicesOnUnit, GT_U32);
    F_BOOL(val, -1, workConservingBit);
    F_BOOL(val, -1, pizzaMapLoadEnBit);
    F_ARRAY_START(val, -1, slice_enable);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            F_ARRAY_BOOL(val, slice_enable, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, slice_enable);
    F_ARRAY_START(val, -1, slice_occupied_by);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            F_ARRAY_NUMBER(val, slice_occupied_by, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, slice_occupied_by);
}

void prv_c_to_lua_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC(
    lua_State *L,
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, totalConfiguredSlices, GT_U32);
    FO_NUMBER(val, t, totalSlicesOnUnit, GT_U32);
    FO_BOOL(val, t, workConservingBit);
    FO_BOOL(val, t, pizzaMapLoadEnBit);
    FO_ARRAY_START(val, t, slice_enable);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            FO_ARRAY_BOOL(val, slice_enable, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, slice_enable);
    FO_ARRAY_START(val, t, slice_occupied_by);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            FO_ARRAY_NUMBER(val, slice_occupied_by, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, slice_occupied_by);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);

void prv_lua_to_c_CPSS_PX_UDB_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_KEY_STC *val
)
{
    F_ENUM(val, -1, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, udbByteOffset, GT_U32);
}

void prv_c_to_lua_CPSS_PX_UDB_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, udbByteOffset, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_KEY_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_INTROP_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_INTROP_STC *val
)
{
    F_NUMBER(val, -1, attrBitMask, GT_U16);
    F_NUMBER(val, -1, txDisDuration, GT_U16);
    F_NUMBER(val, -1, abilityDuration, GT_U16);
    F_NUMBER(val, -1, abilityMaxInterval, GT_U16);
    F_NUMBER(val, -1, abilityFailMaxInterval, GT_U16);
    F_NUMBER(val, -1, apLinkDuration, GT_U16);
    F_NUMBER(val, -1, apLinkMaxInterval, GT_U16);
    F_NUMBER(val, -1, pdLinkDuration, GT_U16);
    F_NUMBER(val, -1, pdLinkMaxInterval, GT_U16);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_INTROP_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_INTROP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, attrBitMask, GT_U16);
    FO_NUMBER(val, t, txDisDuration, GT_U16);
    FO_NUMBER(val, t, abilityDuration, GT_U16);
    FO_NUMBER(val, t, abilityMaxInterval, GT_U16);
    FO_NUMBER(val, t, abilityFailMaxInterval, GT_U16);
    FO_NUMBER(val, t, apLinkDuration, GT_U16);
    FO_NUMBER(val, t, apLinkMaxInterval, GT_U16);
    FO_NUMBER(val, t, pdLinkDuration, GT_U16);
    FO_NUMBER(val, t, pdLinkMaxInterval, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_INTROP_STC);

enumDescr enumDescr_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_25M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_156M_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);

enumDescr enumDescr_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_MODE_OUTPUT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);

enumDescr enumDescr_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E),
    ENUM_ENTRY(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E),
    ENUM_ENTRY(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_LED_CLASS_MANIPULATION_STC(
    lua_State *L,
    CPSS_PX_LED_CLASS_MANIPULATION_STC *val
)
{
    F_BOOL(val, -1, blinkEnable);
    F_ENUM(val, -1, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    F_BOOL(val, -1, forceEnable);
    F_NUMBER(val, -1, forceData, GT_U32);
    F_BOOL(val, -1, pulseStretchEnable);
    F_BOOL(val, -1, disableOnLinkDown);
}

void prv_c_to_lua_CPSS_PX_LED_CLASS_MANIPULATION_STC(
    lua_State *L,
    CPSS_PX_LED_CLASS_MANIPULATION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, blinkEnable);
    FO_ENUM(val, t, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    FO_BOOL(val, t, forceEnable);
    FO_NUMBER(val, t, forceData, GT_U32);
    FO_BOOL(val, t, pulseStretchEnable);
    FO_BOOL(val, t, disableOnLinkDown);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LED_CLASS_MANIPULATION_STC);

void prv_lua_to_c_CPSS_PX_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_PORT_MAP_STC *val
)
{
    F_NUMBER(val, -1, physicalPortNumber, GT_PHYSICAL_PORT_NUM);
    F_ENUM(val, -1, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    F_NUMBER(val, -1, interfaceNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, physicalPortNumber, GT_PHYSICAL_PORT_NUM);
    FO_ENUM(val, t, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    FO_NUMBER(val, t, interfaceNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_MAP_STC);

void prv_lua_to_c_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, xonThreshold, GT_U32);
    F_NUMBER(val, -1, xoffThreshold, GT_U32);
    F_ENUM(val, -1, xonAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, xoffAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, xonThreshold, GT_U32);
    FO_NUMBER(val, t, xoffThreshold, GT_U32);
    FO_ENUM(val, t, xonAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, xoffAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC);

void prv_lua_to_c_CPSS_PX_PTP_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_STC *val
)
{
    F_ARRAY_START(val, -1, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            F_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbPairArr);
    F_ARRAY_START(val, -1, portUdbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_STRUCT(val, portUdbArr, idx, CPSS_PX_UDB_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbArr);
}

void prv_c_to_lua_CPSS_PX_PTP_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            FO_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbPairArr);
    FO_ARRAY_START(val, t, portUdbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_STRUCT(val, portUdbArr, idx, CPSS_PX_UDB_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TYPE_KEY_STC);

enumDescr enumDescr_CPSS_PX_CNC_COUNTER_FORMAT_ENT[] = {
    ENUM_ENTRY(CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E),
    ENUM_ENTRY(CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E),
    ENUM_ENTRY(CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E),
    ENUM_ENTRY(CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E),
    ENUM_ENTRY(CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_CNC_COUNTER_FORMAT_ENT);

enumDescr enumDescr_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E),
    ENUM_ENTRY(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *val
)
{
    F_ARRAY_START(val, -1, pcid);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            F_ARRAY_NUMBER(val, pcid, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, pcid);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, pcid);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            FO_ARRAY_NUMBER(val, pcid, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, pcid);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);

enumDescr enumDescr_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E),
    ENUM_ENTRY(CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E),
    ENUM_ENTRY(CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E),
    ENUM_ENTRY(CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E),
    ENUM_ENTRY(CPSS_PX_PORT_SERDES_LOOPBACK_MAX_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);

void prv_lua_to_c_CPSS_PX_HW_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_HW_INDEX_INFO_STC *val
)
{
    F_ENUM(val, -1, hwTableType, CPSS_PX_TABLE_ENT);
    F_NUMBER(val, -1, hwTableEntryIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_HW_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_HW_INDEX_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, hwTableType, CPSS_PX_TABLE_ENT);
    FO_NUMBER(val, t, hwTableEntryIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_HW_INDEX_INFO_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_128_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_256_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_512_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_1K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_4K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_8K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_16K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_32K_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);

void prv_lua_to_c_CPSS_PX_DETAILED_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_DETAILED_PORT_MAP_STC *val
)
{
    F_BOOL(val, -1, valid);
    F_STRUCT(val, -1, portMap, CPSS_PX_SHADOW_PORT_MAP_STC);
}

void prv_c_to_lua_CPSS_PX_DETAILED_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_DETAILED_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, valid);
    FO_STRUCT(val, t, portMap, CPSS_PX_SHADOW_PORT_MAP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DETAILED_PORT_MAP_STC);

void prv_lua_to_c_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC *val
)
{
    F_ARRAY_START(val, -1, unitList);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_ENUM(val, unitList, idx, CPSS_PX_PA_UNIT_ENT);
        }
    }
    F_ARRAY_END(val, -1, unitList);
    F_ARRAY_START(val, -1, unitState);
    {
        int idx;
        for (idx = 0; idx < 5; idx++) {
            F_ARRAY_STRUCT(val, unitState, idx, CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
        }
    }
    F_ARRAY_END(val, -1, unitState);
}

void prv_c_to_lua_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, unitList);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_ENUM(val, unitList, idx, CPSS_PX_PA_UNIT_ENT);
        }
    }
    FO_ARRAY_END(val, t, unitList);
    FO_ARRAY_START(val, t, unitState);
    {
        int idx;
        for (idx = 0; idx < 5; idx++) {
            FO_ARRAY_STRUCT(val, unitState, idx, CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
        }
    }
    FO_ARRAY_END(val, t, unitState);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);

void prv_lua_to_c_CPSS_PX_COS_FORMAT_ENTRY_STC(
    lua_State *L,
    CPSS_PX_COS_FORMAT_ENTRY_STC *val
)
{
    F_ENUM(val, -1, cosMode, CPSS_PX_COS_MODE_ENT);
    F_NUMBER(val, -1, cosByteOffset, GT_U32);
    F_NUMBER(val, -1, cosBitOffset, GT_U32);
    F_NUMBER(val, -1, cosNumOfBits, GT_U32);
    F_STRUCT(val, -1, cosAttributes, CPSS_PX_COS_ATTRIBUTES_STC);
}

void prv_c_to_lua_CPSS_PX_COS_FORMAT_ENTRY_STC(
    lua_State *L,
    CPSS_PX_COS_FORMAT_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, cosMode, CPSS_PX_COS_MODE_ENT);
    FO_NUMBER(val, t, cosByteOffset, GT_U32);
    FO_NUMBER(val, t, cosBitOffset, GT_U32);
    FO_NUMBER(val, t, cosNumOfBits, GT_U32);
    FO_STRUCT(val, t, cosAttributes, CPSS_PX_COS_ATTRIBUTES_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_COS_FORMAT_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT *val
)
{
    /* just do nothing */
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_802_1br_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_802_1br_U2E_MC, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);
    FO_STRUCT(val, t, info_dsa_ET2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_dsa_EU2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_dsa_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
    FO_NUMBER(val, t, notNeeded, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT);

void prv_lua_to_c_CPSS_PX_UDB_PAIR_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_KEY_STC *val
)
{
    F_ENUM(val, -1, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, udbByteOffset, GT_U32);
}

void prv_c_to_lua_CPSS_PX_UDB_PAIR_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, udbByteOffset, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_PAIR_KEY_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);

void prv_lua_to_c_CPSS_PX_REGULAR_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_REGULAR_DSA_FORWARD_STC *val
)
{
    F_BOOL(val, -1, srcTagged);
    F_NUMBER(val, -1, hwSrcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, srcPortOrTrunk, GT_U32);
    F_BOOL(val, -1, srcIsTrunk);
    F_NUMBER(val, -1, cfi, GT_U32);
    F_NUMBER(val, -1, up, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_REGULAR_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_REGULAR_DSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, srcTagged);
    FO_NUMBER(val, t, hwSrcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, srcPortOrTrunk, GT_U32);
    FO_BOOL(val, t, srcIsTrunk);
    FO_NUMBER(val, t, cfi, GT_U32);
    FO_NUMBER(val, t, up, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_REGULAR_DSA_FORWARD_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);

enumDescr enumDescr_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E),
    ENUM_ENTRY(CPSS_PX_PORT_CN_LENGTH_1_5_KB_E),
    ENUM_ENTRY(CPSS_PX_PORT_CN_LENGTH_2_KB_E),
    ENUM_ENTRY(CPSS_PX_PORT_CN_LENGTH_10_KB_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);

enumDescr enumDescr_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);

void prv_lua_to_c_CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *val
)
{
    /* just do nothing */
}

void prv_c_to_lua_CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    FO_STRUCT(val, t, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT);

enumDescr enumDescr_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E),
    ENUM_ENTRY(CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);

enumDescr enumDescr_CPSS_PX_COS_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_COS_MODE_PORT_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_PACKET_DSA_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_PACKET_L2_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_PACKET_L3_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_PACKET_MPLS_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_FORMAT_ENTRY_E),
    ENUM_ENTRY(CPSS_PX_COS_MODE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_COS_MODE_ENT);

void prv_lua_to_c_CPSS_PX_SHADOW_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_SHADOW_PORT_MAP_STC *val
)
{
    F_ENUM(val, -1, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    F_NUMBER(val, -1, macNum, GT_U32);
    F_NUMBER(val, -1, dmaNum, GT_U32);
    F_NUMBER(val, -1, txqNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_SHADOW_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_SHADOW_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    FO_NUMBER(val, t, macNum, GT_U32);
    FO_NUMBER(val, t, dmaNum, GT_U32);
    FO_NUMBER(val, t, txqNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_SHADOW_PORT_MAP_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *val
)
{
    F_ENUM(val, -1, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    F_ENUM(val, -1, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    F_NUMBER(val, -1, numOfBytes, GT_U32);
    F_NUMBER(val, -1, isSupported, GT_STATUS);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_ENUM(val, t, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    FO_NUMBER(val, t, numOfBytes, GT_U32);
    FO_NUMBER(val, t, isSupported, GT_STATUS);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_ETHERTYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_IPV4_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_IPV6_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_MPLS1_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_MPLS2_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_IPV6_EH_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_ETHERTYPE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_ETHERTYPE_ENT);

void prv_lua_to_c_CPSS_PX_PTP_TAI_TOD_STEP_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_STEP_STC *val
)
{
    F_NUMBER(val, -1, nanoSeconds, GT_U32);
    F_NUMBER(val, -1, fracNanoSeconds, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PTP_TAI_TOD_STEP_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_STEP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, nanoSeconds, GT_U32);
    FO_NUMBER(val, t, fracNanoSeconds, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TAI_TOD_STEP_STC);

enumDescr enumDescr_CPSS_PX_PORT_FC_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_FC_MODE_802_3X_E),
    ENUM_ENTRY(CPSS_PX_PORT_FC_MODE_PFC_E),
    ENUM_ENTRY(CPSS_PX_PORT_FC_MODE_LL_FC_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_FC_MODE_ENT);

enumDescr enumDescr_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_LOCATION_ENT[] = {
    ENUM_ENTRY(CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE),
    ENUM_ENTRY(CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE),
    ENUM_ENTRY(CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_LOCATION_ENT);

enumDescr enumDescr_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC(
    lua_State *L,
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *val
)
{
    F_BOOL(val, -1, link);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, duplex, CPSS_PORT_DUPLEX_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC(
    lua_State *L,
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, link);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, duplex, CPSS_PORT_DUPLEX_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);

enumDescr enumDescr_CPSS_PX_DIAG_TRANSMIT_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_ZEROS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_ONES_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_CYCLIC_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS9_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS15_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_PRBS31_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_1T_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_2T_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_5T_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_10T_E),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_DFETraining),
    ENUM_ENTRY(CPSS_PX_DIAG_TRANSMIT_MODE_MAX_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_DIAG_TRANSMIT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, eTag, CPSS_802_1BR_ETAG_STC);
    F_NUMBER(val, -1, vlanTagTpid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, eTag, CPSS_802_1BR_ETAG_STC);
    FO_NUMBER(val, t, vlanTagTpid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);

void prv_lua_to_c_CPSS_PX_LOCATION_UNT(
    lua_State *L,
    CPSS_PX_LOCATION_UNT *val
)
{
    /* just do nothing */
}

void prv_c_to_lua_CPSS_PX_LOCATION_UNT(
    lua_State *L,
    CPSS_PX_LOCATION_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    FO_STRUCT(val, t, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    FO_STRUCT(val, t, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOCATION_UNT);

void prv_lua_to_c_CPSS_PX_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PX_PORT_EGRESS_CNTR_STC *val
)
{
    F_NUMBER(val, -1, outFrames, GT_U32);
    F_NUMBER(val, -1, txqFilterDisc, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PX_PORT_EGRESS_CNTR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, outFrames, GT_U32);
    FO_NUMBER(val, t, txqFilterDisc, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_EGRESS_CNTR_STC);

void prv_lua_to_c_CPSS_PX_LED_CONF_STC(
    lua_State *L,
    CPSS_PX_LED_CONF_STC *val
)
{
    F_ENUM(val, -1, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    F_ENUM(val, -1, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    F_ENUM(val, -1, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    F_ENUM(val, -1, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    F_ENUM(val, -1, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    F_ENUM(val, -1, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    F_NUMBER(val, -1, ledStart, GT_U32);
    F_NUMBER(val, -1, ledEnd, GT_U32);
    F_BOOL(val, -1, invertEnable);
    F_ENUM(val, -1, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
}

void prv_c_to_lua_CPSS_PX_LED_CONF_STC(
    lua_State *L,
    CPSS_PX_LED_CONF_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    FO_ENUM(val, t, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    FO_ENUM(val, t, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    FO_ENUM(val, t, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    FO_ENUM(val, t, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    FO_ENUM(val, t, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    FO_NUMBER(val, t, ledStart, GT_U32);
    FO_NUMBER(val, t, ledEnd, GT_U32);
    FO_BOOL(val, t, invertEnable);
    FO_ENUM(val, t, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LED_CONF_STC);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC *val
)
{
    F_BOOL(val, -1, tcDpLimit);
    F_BOOL(val, -1, portLimit);
    F_BOOL(val, -1, tcLimit);
    F_BOOL(val, -1, sharedPoolLimit);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, tcDpLimit);
    FO_BOOL(val, t, portLimit);
    FO_BOOL(val, t, tcLimit);
    FO_BOOL(val, t, sharedPoolLimit);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_ONLY_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);

enumDescr enumDescr_CPSS_PX_CNC_CLIENT_ENT[] = {
    ENUM_ENTRY(CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E),
    ENUM_ENTRY(CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E),
    ENUM_ENTRY(CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_CNC_CLIENT_ENT);

void prv_lua_to_c_CPSS_PX_CNC_COUNTER_STC(
    lua_State *L,
    CPSS_PX_CNC_COUNTER_STC *val
)
{
    F_STRUCT(val, -1, byteCount, GT_U64);
    F_STRUCT(val, -1, packetCount, GT_U64);
}

void prv_c_to_lua_CPSS_PX_CNC_COUNTER_STC(
    lua_State *L,
    CPSS_PX_CNC_COUNTER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, byteCount, GT_U64);
    FO_STRUCT(val, t, packetCount, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_CNC_COUNTER_STC);

enumDescr enumDescr_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC *val
)
{
    F_NUMBER(val, -1, srcPortProfile, GT_U32);
    F_ARRAY_START(val, -1, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbPairArr);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcPortProfile, GT_U32);
    FO_ARRAY_START(val, t, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbPairArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC);

void prv_lua_to_c_CPSS_PX_PTP_TSU_CONTROL_STC(
    lua_State *L,
    CPSS_PX_PTP_TSU_CONTROL_STC *val
)
{
    F_BOOL(val, -1, unitEnable);
}

void prv_c_to_lua_CPSS_PX_PTP_TSU_CONTROL_STC(
    lua_State *L,
    CPSS_PX_PTP_TSU_CONTROL_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, unitEnable);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TSU_CONTROL_STC);

enumDescr enumDescr_CPSS_PX_DIAG_BIST_STATUS_ENT[] = {
    ENUM_ENTRY(CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E),
    ENUM_ENTRY(CPSS_PX_DIAG_BIST_STATUS_PASS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_BIST_STATUS_FAIL_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_DIAG_BIST_STATUS_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_TPID_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_TPID_ENTRY_STC *val
)
{
    F_NUMBER(val, -1, val, GT_U16);
    F_NUMBER(val, -1, size, GT_U32);
    F_BOOL(val, -1, valid);
}

void prv_c_to_lua_CPSS_PX_INGRESS_TPID_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_TPID_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, val, GT_U16);
    FO_NUMBER(val, t, size, GT_U32);
    FO_BOOL(val, t, valid);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_TPID_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, egrMirrorDropCntr, GT_U32);
    F_NUMBER(val, -1, egrStcDropCntr, GT_U32);
    F_NUMBER(val, -1, egrQcnDropCntr, GT_U32);
    F_NUMBER(val, -1, dropPfcEventsCntr, GT_U32);
    F_NUMBER(val, -1, clearPacketsDroppedCounter, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, egrMirrorDropCntr, GT_U32);
    FO_NUMBER(val, t, egrStcDropCntr, GT_U32);
    FO_NUMBER(val, t, egrQcnDropCntr, GT_U32);
    FO_NUMBER(val, t, dropPfcEventsCntr, GT_U32);
    FO_NUMBER(val, t, clearPacketsDroppedCounter, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC);

void prv_lua_to_c_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC *val
)
{
    F_NUMBER(val, -1, byteOffset, GT_U32);
    F_NUMBER(val, -1, startBit, GT_U32);
    F_NUMBER(val, -1, numBits, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, byteOffset, GT_U32);
    FO_NUMBER(val, t, startBit, GT_U32);
    FO_NUMBER(val, t, numBits, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);

void prv_lua_to_c_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC(
    lua_State *L,
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macDa, GT_ETHERADDR);
    F_BOOL(val, -1, macDaIncrementEnable);
    F_NUMBER(val, -1, macDaIncrementLimit, GT_U32);
    F_STRUCT_CUSTOM(val, -1, macSa, GT_ETHERADDR);
    F_BOOL(val, -1, vlanTagEnable);
    F_NUMBER(val, -1, vpt, GT_U8);
    F_NUMBER(val, -1, cfi, GT_U8);
    F_NUMBER(val, -1, vid, GT_U16);
    F_NUMBER(val, -1, etherType, GT_U16);
    F_ENUM(val, -1, payloadType, CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);
    F_ARRAY_START(val, -1, cyclicPatternArr);
    {
        int idx;
        for (idx = 0; idx < 64; idx++) {
            F_ARRAY_NUMBER(val, cyclicPatternArr, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, cyclicPatternArr);
    F_ENUM(val, -1, packetLengthType, CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);
    F_NUMBER(val, -1, packetLength, GT_U32);
    F_BOOL(val, -1, undersizeEnable);
    F_ENUM(val, -1, transmitMode, CPSS_DIAG_PG_TRANSMIT_MODE_ENT);
    F_NUMBER(val, -1, packetCount, GT_U32);
    F_ENUM(val, -1, packetCountMultiplier, CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);
    F_NUMBER(val, -1, ifg, GT_U32);
    F_ENUM(val, -1, interfaceSize, CPSS_DIAG_PG_IF_SIZE_ENT);
}

void prv_c_to_lua_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC(
    lua_State *L,
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macDa, GT_ETHERADDR);
    FO_BOOL(val, t, macDaIncrementEnable);
    FO_NUMBER(val, t, macDaIncrementLimit, GT_U32);
    FO_STRUCT(val, t, macSa, GT_ETHERADDR);
    FO_BOOL(val, t, vlanTagEnable);
    FO_NUMBER(val, t, vpt, GT_U8);
    FO_NUMBER(val, t, cfi, GT_U8);
    FO_NUMBER(val, t, vid, GT_U16);
    FO_NUMBER(val, t, etherType, GT_U16);
    FO_ENUM(val, t, payloadType, CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);
    FO_ARRAY_START(val, t, cyclicPatternArr);
    {
        int idx;
        for (idx = 0; idx < 64; idx++) {
            FO_ARRAY_NUMBER(val, cyclicPatternArr, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, cyclicPatternArr);
    FO_ENUM(val, t, packetLengthType, CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);
    FO_NUMBER(val, t, packetLength, GT_U32);
    FO_BOOL(val, t, undersizeEnable);
    FO_ENUM(val, t, transmitMode, CPSS_DIAG_PG_TRANSMIT_MODE_ENT);
    FO_NUMBER(val, t, packetCount, GT_U32);
    FO_ENUM(val, t, packetCountMultiplier, CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);
    FO_NUMBER(val, t, ifg, GT_U32);
    FO_ENUM(val, t, interfaceSize, CPSS_DIAG_PG_IF_SIZE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_PG_CONFIGURATIONS_STC);

enumDescr enumDescr_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_HASH_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_MODE_CRC32_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_HASH_MODE_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macDa, GT_ETHERADDR);
    F_NUMBER(val, -1, etherType, GT_U16);
    F_BOOL(val, -1, isLLCNonSnap);
    F_NUMBER(val, -1, profileIndex, GT_U32);
    F_ARRAY_START(val, -1, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    F_ARRAY_END(val, -1, udbPairsArr);
    F_NUMBER(val, -1, ip2meIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macDa, GT_ETHERADDR);
    FO_NUMBER(val, t, etherType, GT_U16);
    FO_BOOL(val, t, isLLCNonSnap);
    FO_NUMBER(val, t, profileIndex, GT_U32);
    FO_ARRAY_START(val, t, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    FO_ARRAY_END(val, t, udbPairsArr);
    FO_NUMBER(val, t, ip2meIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC);

enumDescr enumDescr_CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_MGCAM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MUNIT_MUNIT_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_IMEM_BANK_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_PPN_SCRATCHPAD_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_SHARED_DMEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_HA_TABLE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_BC_CT_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CLEAR_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_FREE_BUFFERS_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_DESC_IDDB_FREE_ID_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_DESC_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_INTERNAL_DESC_IDDB_FREE_ID_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_INTERNAL_DESC_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_CT_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_NEXT_TABLE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_NUMBER_OF_BUFFERS_TABLE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PREF_MEM_ID_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_FREE_ENTRY_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_IND_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_PTR_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_CT_BC_IDDB_FREE_ID_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_CT_BC_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_DESCRIPTOR_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_LL_FREE_BUFS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_LL_LINK_LIST_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RXDMA_IBUF_HDR_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_LL_FREE_BUFS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_LL_LINK_LIST_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_CG_RX_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_CG_TX_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_DESKEW_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_RS_FEC_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_RX_PREAMBLE_MEM_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_CNC_IP_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PCP_IP_DST_PORT_MAP_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PCP_IP_SRC_PORT_MAP_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_DDU_FIFO_E),
    ENUM_ENTRY(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PA_UNIT_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PA_UNIT_UNDEFINED_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_RXDMA_0_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_TXQ_0_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_TXQ_1_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_TXDMA_0_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_TX_FIFO_0_E),
    ENUM_ENTRY(CPSS_PX_PA_UNIT_MAX_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PA_UNIT_ENT);

void prv_lua_to_c_CPSS_PX_COS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PX_COS_ATTRIBUTES_STC *val
)
{
    F_NUMBER(val, -1, trafficClass, GT_U32);
    F_ENUM(val, -1, dropPrecedence, CPSS_DP_LEVEL_ENT);
    F_NUMBER(val, -1, userPriority, GT_U32);
    F_NUMBER(val, -1, dropEligibilityIndication, GT_U32);
}

void prv_c_to_lua_CPSS_PX_COS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PX_COS_ATTRIBUTES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, trafficClass, GT_U32);
    FO_ENUM(val, t, dropPrecedence, CPSS_DP_LEVEL_ENT);
    FO_NUMBER(val, t, userPriority, GT_U32);
    FO_NUMBER(val, t, dropEligibilityIndication, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_COS_ATTRIBUTES_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SRC_PORT_INFO_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SRC_PORT_INFO_STC *val
)
{
    F_NUMBER(val, -1, srcFilteringVector, CPSS_PX_PORTS_BMP);
    F_STRUCT(val, -1, vlanTag, CPSS_PX_VLAN_TAG_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SRC_PORT_INFO_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SRC_PORT_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcFilteringVector, CPSS_PX_PORTS_BMP);
    FO_STRUCT(val, t, vlanTag, CPSS_PX_VLAN_TAG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SRC_PORT_INFO_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_2_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_3_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_4_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_5_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_6_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_7_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_8_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_9_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_10_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_11_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_12_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_13_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_14_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);

void prv_lua_to_c_CPSS_PX_PORT_SERDES_EYE_INPUT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC *val
)
{
    F_NUMBER(val, -1, min_dwell_bits, GT_U32);
    F_NUMBER(val, -1, max_dwell_bits, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_SERDES_EYE_INPUT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, min_dwell_bits, GT_U32);
    FO_NUMBER(val, t, max_dwell_bits, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_SERDES_EYE_INPUT_STC);

void prv_lua_to_c_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC *val
)
{
    F_ARRAY_START(val, -1, bitFieldArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, bitFieldArr, idx, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
        }
    }
    F_ARRAY_END(val, -1, bitFieldArr);
    F_NUMBER(val, -1, indexConst, GT_32);
    F_NUMBER(val, -1, indexMax, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, bitFieldArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, bitFieldArr, idx, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
        }
    }
    FO_ARRAY_END(val, t, bitFieldArr);
    FO_NUMBER(val, t, indexConst, GT_32);
    FO_NUMBER(val, t, indexMax, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC);

void prv_lua_to_c_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, packetType, CPSS_PX_PACKET_TYPE);
    F_NUMBER(val, -1, priority, GT_U32);
    F_ENUM(val, -1, dropPrecedence, CPSS_DP_LEVEL_ENT);
    F_NUMBER(val, -1, tc4pfc, GT_U32);
    F_BOOL(val, -1, localGeneratedPacketTypeAssignmentEnable);
    F_NUMBER(val, -1, localGeneratedPacketType, CPSS_PX_PACKET_TYPE);
}

void prv_c_to_lua_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, packetType, CPSS_PX_PACKET_TYPE);
    FO_NUMBER(val, t, priority, GT_U32);
    FO_ENUM(val, t, dropPrecedence, CPSS_DP_LEVEL_ENT);
    FO_NUMBER(val, t, tc4pfc, GT_U32);
    FO_BOOL(val, t, localGeneratedPacketTypeAssignmentEnable);
    FO_NUMBER(val, t, localGeneratedPacketType, CPSS_PX_PACKET_TYPE);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC);

enumDescr enumDescr_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_PX_CFG_DEV_INFO_STC *val
)
{
    F_STRUCT(val, -1, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
}

void prv_c_to_lua_CPSS_PX_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_PX_CFG_DEV_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_CFG_DEV_INFO_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);

void prv_lua_to_c_CPSS_PX_PTP_TAI_TOD_COUNT_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_COUNT_STC *val
)
{
    F_NUMBER(val, -1, nanoSeconds, GT_U32);
    F_STRUCT(val, -1, seconds, GT_U64);
    F_NUMBER(val, -1, fracNanoSeconds, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PTP_TAI_TOD_COUNT_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_COUNT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, nanoSeconds, GT_U32);
    FO_STRUCT(val, t, seconds, GT_U64);
    FO_NUMBER(val, t, fracNanoSeconds, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TAI_TOD_COUNT_STC);

void prv_lua_to_c_CPSS_PX_VLAN_TAG_STC(
    lua_State *L,
    CPSS_PX_VLAN_TAG_STC *val
)
{
    F_NUMBER(val, -1, pcp, GT_U32);
    F_NUMBER(val, -1, dei, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_VLAN_TAG_STC(
    lua_State *L,
    CPSS_PX_VLAN_TAG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pcp, GT_U32);
    FO_NUMBER(val, t, dei, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_VLAN_TAG_STC);

enumDescr enumDescr_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E),
    ENUM_ENTRY(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);

void prv_lua_to_c_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *val
)
{
    F_NUMBER(val, -1, interval, GT_U32);
    F_NUMBER(val, -1, randBitmap, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, interval, GT_U32);
    FO_NUMBER(val, t, randBitmap, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_PORT_ALIGN90_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_ALIGN90_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, startAlign90, GT_U32);
    F_NUMBER(val, -1, rxTrainingCfg, GT_U32);
    F_NUMBER(val, -1, osDeltaMax, GT_U32);
    F_NUMBER(val, -1, adaptedFfeR, GT_U32);
    F_NUMBER(val, -1, adaptedFfeC, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_ALIGN90_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_ALIGN90_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, startAlign90, GT_U32);
    FO_NUMBER(val, t, rxTrainingCfg, GT_U32);
    FO_NUMBER(val, t, osDeltaMax, GT_U32);
    FO_NUMBER(val, t, adaptedFfeR, GT_U32);
    FO_NUMBER(val, t, adaptedFfeC, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_ALIGN90_PARAMS_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *val
)
{
    F_NUMBER(val, -1, vid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, vid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);

enumDescr enumDescr_CPSS_PX_MIN_SPEED_ENT[] = {
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_INVALID_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_500_Mbps_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_1000_Mbps_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_2000_Mbps_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_5000_Mbps_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_10000_Mbps_E),
    ENUM_ENTRY(CPSS_PX_MIN_SPEED_MAX),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_MIN_SPEED_ENT);

void prv_lua_to_c_CPSS_PX_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_PX_NET_TX_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, txQueue, GT_U32);
    F_BOOL(val, -1, recalcCrc);
}

void prv_c_to_lua_CPSS_PX_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_PX_NET_TX_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txQueue, GT_U32);
    FO_BOOL(val, t, recalcCrc);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_NET_TX_PARAMS_STC);

enumDescr enumDescr_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E),
    ENUM_ENTRY(CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC *val
)
{
    F_NUMBER(val, -1, dp0MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp0MaxDescNum, GT_U32);
    F_NUMBER(val, -1, dp1MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp1MaxDescNum, GT_U32);
    F_NUMBER(val, -1, dp2MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp2MaxDescNum, GT_U32);
    F_NUMBER(val, -1, tcMaxBuffNum, GT_U32);
    F_NUMBER(val, -1, tcMaxDescNum, GT_U32);
    F_ENUM(val, -1, dp0QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, dp1QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, dp2QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dp0MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp0MaxDescNum, GT_U32);
    FO_NUMBER(val, t, dp1MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp1MaxDescNum, GT_U32);
    FO_NUMBER(val, t, dp2MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp2MaxDescNum, GT_U32);
    FO_NUMBER(val, t, tcMaxBuffNum, GT_U32);
    FO_NUMBER(val, t, tcMaxDescNum, GT_U32);
    FO_ENUM(val, t, dp0QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, dp1QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, dp2QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC);

enumDescr enumDescr_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E),
    ENUM_ENTRY(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E),
    ENUM_ENTRY(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E),
    ENUM_ENTRY(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_SHADOW_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_SHADOW_TYPE_NONE_E),
    ENUM_ENTRY(CPSS_PX_SHADOW_TYPE_CPSS_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_SHADOW_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_TX_SHAPER_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E),
    ENUM_ENTRY(CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_TX_SHAPER_MODE_ENT);

enumDescr enumDescr_CPSS_PX_POWER_SUPPLIES_NUMBER_ENT[] = {
    ENUM_ENTRY(CPSS_PX_POWER_SUPPLIES_DUAL_E),
    ENUM_ENTRY(CPSS_PX_POWER_SUPPLIES_SINGLE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);

enumDescr enumDescr_CPSS_PX_PHA_FIRMWARE_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PHA_FIRMWARE_TYPE_DEFAULT_E),
    ENUM_ENTRY(CPSS_PX_PHA_FIRMWARE_TYPE_NOT_USED_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PHA_FIRMWARE_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_PORT_PFC_COUNT_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E),
    ENUM_ENTRY(CPSS_PX_PORT_PFC_COUNT_PACKETS_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_PFC_COUNT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *val
)
{
    F_NUMBER(val, -1, tcDp, GT_U32);
    F_NUMBER(val, -1, port, GT_U32);
    F_NUMBER(val, -1, tc, GT_U32);
    F_NUMBER(val, -1, pool, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tcDp, GT_U32);
    FO_NUMBER(val, t, port, GT_U32);
    FO_NUMBER(val, t, tc, GT_U32);
    FO_NUMBER(val, t, pool, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC);

enumDescr enumDescr_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_78M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_156M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_200M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_312M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_161M_E),
    ENUM_ENTRY(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_164M_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);

enumDescr enumDescr_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PORT_CTLE_BIAS_NORMAL_E),
    ENUM_ENTRY(CPSS_PX_PORT_CTLE_BIAS_HIGH_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);

void prv_lua_to_c_CPSS_PX_LOCATION_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_FULL_INFO_STC *val
)
{
    F_STRUCT(val, -1, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    F_STRUCT(val, -1, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    F_STRUCT(val, -1, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    F_BOOL(val, -1, isMppmInfoValid);
    F_STRUCT(val, -1, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_LOCATION_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_FULL_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    FO_STRUCT(val, t, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    FO_STRUCT(val, t, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    FO_BOOL(val, t, isMppmInfoValid);
    FO_STRUCT(val, t, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOCATION_FULL_INFO_STC);

void prv_lua_to_c_CPSS_PX_RAM_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_RAM_INDEX_INFO_STC *val
)
{
    F_ENUM(val, -1, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    F_NUMBER(val, -1, ramRow, GT_U32);
    F_STRUCT(val, -1, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_RAM_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_RAM_INDEX_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    FO_NUMBER(val, t, ramRow, GT_U32);
    FO_STRUCT(val, t, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_RAM_INDEX_INFO_STC);

void prv_lua_to_c_CPSS_PX_NET_SDMA_RX_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, rxInPkts, GT_U32);
    F_NUMBER(val, -1, rxInOctets, GT_U32);
}

void prv_c_to_lua_CPSS_PX_NET_SDMA_RX_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, rxInPkts, GT_U32);
    FO_NUMBER(val, t, rxInOctets, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_NET_SDMA_RX_COUNTERS_STC);

enumDescr enumDescr_CPSS_PX_CFG_CNTR_ENT[] = {
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_RECEIVED_PFC_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_RECEIVED_MC_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_RECEIVED_UC_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_RECEIVED_QCN_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_RECEIVED_MC_QCN_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_INGRESS_DROP_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_MC_PACKETS_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_QCN_TO_PFC_MSG_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_QCN_PACKETS_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_PFC_PACKETS_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_UC_PACKETS_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_OUT_MC_QCN_PACKETS_E),
    ENUM_ENTRY(CPSS_PX_CFG_CNTR_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_CFG_CNTR_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *val
)
{
    /* just do nothing */
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_802_1br, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
    FO_STRUCT(val, t, info_dsa, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
    FO_STRUCT(val, t, info_evb, CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
    FO_NUMBER(val, t, notNeeded, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT);

enumDescr enumDescr_CPSS_PX_LOGICAL_TABLE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_CNC_0_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_CNC_1_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_PHA_SHARED_DMEM_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_PHA_FW_IMAGE_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_CM3_RAM_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_PHA_RAM_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E),
    ENUM_ENTRY(CPSS_PX_LOGICAL_TABLE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_LOGICAL_TABLE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC *val
)
{
    F_NUMBER(val, -1, pcid, GT_U32);
    F_STRUCT(val, -1, srcPortInfo, CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
    F_BOOL(val, -1, upstreamPort);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pcid, GT_U32);
    FO_STRUCT(val, t, srcPortInfo, CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
    FO_BOOL(val, t, upstreamPort);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);

void prv_lua_to_c_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, wExp, GT_32);
    F_NUMBER(val, -1, fbLsb, GT_U32);
    F_BOOL(val, -1, deltaEnable);
    F_NUMBER(val, -1, fbMin, GT_U32);
    F_NUMBER(val, -1, fbMax, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, wExp, GT_32);
    FO_NUMBER(val, t, fbLsb, GT_U32);
    FO_BOOL(val, t, deltaEnable);
    FO_NUMBER(val, t, fbMin, GT_U32);
    FO_NUMBER(val, t, fbMax, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC);

enumDescr enumDescr_CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E),
    ENUM_ENTRY(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_PARAMS_STC *val
)
{
    F_BOOL(val, -1, fcPause);
    F_ENUM(val, -1, fcAsmDir, CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
    F_BOOL(val, -1, fecSupported);
    F_BOOL(val, -1, fecRequired);
    F_BOOL(val, -1, noneceDisable);
    F_NUMBER(val, -1, laneNum, GT_U32);
    F_ARRAY_START(val, -1, modesAdvertiseArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_STRUCT(val, modesAdvertiseArr, idx, CPSS_PORT_MODE_SPEED_STC);
        }
    }
    F_ARRAY_END(val, -1, modesAdvertiseArr);
    F_ARRAY_START(val, -1, fecAbilityArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_ENUM(val, fecAbilityArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    F_ARRAY_END(val, -1, fecAbilityArr);
    F_ARRAY_START(val, -1, fecRequestedArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_ENUM(val, fecRequestedArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    F_ARRAY_END(val, -1, fecRequestedArr);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, fcPause);
    FO_ENUM(val, t, fcAsmDir, CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
    FO_BOOL(val, t, fecSupported);
    FO_BOOL(val, t, fecRequired);
    FO_BOOL(val, t, noneceDisable);
    FO_NUMBER(val, t, laneNum, GT_U32);
    FO_ARRAY_START(val, t, modesAdvertiseArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_STRUCT(val, modesAdvertiseArr, idx, CPSS_PORT_MODE_SPEED_STC);
        }
    }
    FO_ARRAY_END(val, t, modesAdvertiseArr);
    FO_ARRAY_START(val, t, fecAbilityArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_ENUM(val, fecAbilityArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    FO_ARRAY_END(val, t, fecAbilityArr);
    FO_ARRAY_START(val, t, fecRequestedArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_ENUM(val, fecRequestedArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    FO_ARRAY_END(val, t, fecRequestedArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_PARAMS_STC);

void prv_lua_to_c_CPSS_PX_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATUS_STC *val
)
{
    F_NUMBER(val, -1, postApPortNum, GT_U32);
    F_STRUCT(val, -1, portMode, CPSS_PORT_MODE_SPEED_STC);
    F_BOOL(val, -1, hcdFound);
    F_BOOL(val, -1, fecEnabled);
    F_BOOL(val, -1, fcRxPauseEn);
    F_BOOL(val, -1, fcTxPauseEn);
    F_ENUM(val, -1, fecType, CPSS_PORT_FEC_MODE_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, postApPortNum, GT_U32);
    FO_STRUCT(val, t, portMode, CPSS_PORT_MODE_SPEED_STC);
    FO_BOOL(val, t, hcdFound);
    FO_BOOL(val, t, fecEnabled);
    FO_BOOL(val, t, fcRxPauseEn);
    FO_BOOL(val, t, fcTxPauseEn);
    FO_ENUM(val, t, fecType, CPSS_PORT_FEC_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_STATUS_STC);

enumDescr enumDescr_CPSS_PX_TABLE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_TABLE_BMA_PORT_MAPPING_E),
    ENUM_ENTRY(CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E),
    ENUM_ENTRY(CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_CNC_0_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_CNC_1_COUNTERS_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PHA_HA_TABLE_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E),
    ENUM_ENTRY(CPSS_PX_TABLE_PHA_SHARED_DMEM_E),
    ENUM_ENTRY(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E),
    ENUM_ENTRY(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E),
    ENUM_ENTRY(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E),
    ENUM_ENTRY(CPSS_PX_TABLE_PHA_FW_IMAGE_E),
    ENUM_ENTRY(CPSS_PX_TABLE_LAST_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_ECC_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_PARITY_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_CM3_RAM_E),
    ENUM_ENTRY(CPSS_PX_INTERNAL_TABLE_PHA_RAM_E),
    ENUM_ENTRY(CPSS_PX_HW_TABLE_LAST_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_TABLE_ENT);

enumDescr enumDescr_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_DECREMENT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_NOP_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);

void prv_lua_to_c_CPSS_PX_PORT_MAC_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_MAC_COUNTERS_STC *val
)
{
    F_ARRAY_START(val, -1, mibCounter);
    {
        int idx;
        for (idx = 0; idx < 28; idx++) {
            F_ARRAY_STRUCT(val, mibCounter, idx, GT_U64);
        }
    }
    F_ARRAY_END(val, -1, mibCounter);
}

void prv_c_to_lua_CPSS_PX_PORT_MAC_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_MAC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, mibCounter);
    {
        int idx;
        for (idx = 0; idx < 28; idx++) {
            FO_ARRAY_STRUCT(val, mibCounter, idx, GT_U64);
        }
    }
    FO_ARRAY_END(val, t, mibCounter);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_MAC_COUNTERS_STC);

enumDescr enumDescr_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E),
    ENUM_ENTRY(CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);

