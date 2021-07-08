#include <Copyright.h>

/**
********************************************************************************
* @file gtDrvSwRegs.h
*
* @brief definitions of the register map of Switch Device
*
*/
/********************************************************************************
* gtDrvSwRegs.h
*
* DESCRIPTION:
*       definitions of the register map of Switch Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#ifndef __prvCpssDrvGtDrvSwRegs_h
#define __prvCpssDrvGtDrvSwRegs_h
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/* Block: Switch Registers                                                    */
/*                                                                            */
/******************************************************************************/

/* Define Switch SMI Device Address */
#define PRV_CPSS_PORT_START_ADDR               0x00    /* Start Port Address */
#define PRV_CPSS_GLOBAL1_DEV_ADDR              0x1B    /* Global 1 Device Address */
#define PRV_CPSS_GLOBAL2_DEV_ADDR              0x1C    /* Global 2 Device Address */
#define PRV_CPSS_CPU_PORT_ADDR                 0x1E    /* CPU Port Address */
#define PRV_CPSS_TCAM_DEV_ADDR                 0x1F    /* TCAM Device Address */

/* Switch Per Port Registers */
#define PRV_CPSS_QD_REG_PORT_STATUS            0x00    /* Port Status Register */
#define PRV_CPSS_QD_REG_PHY_CONTROL            0x01    /* Physical Control Register */
#define PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL    0x02    /* Flow Control Register */
#define PRV_CPSS_QD_REG_SWITCH_ID              0x03    /* Switch Identifier Register */
#define PRV_CPSS_QD_REG_PORT_CONTROL           0x04    /* Port Control Register */
#define PRV_CPSS_QD_REG_PORT_CONTROL1          0x05    /* Port Control 1 Register */
#define PRV_CPSS_QD_REG_PORT_VLAN_MAP          0x06    /* Port based VLAN Map Register */
#define PRV_CPSS_QD_REG_PVID                   0x07    /* Port VLAN ID & Priority Register */
#define PRV_CPSS_QD_REG_PORT_CONTROL2          0x08    /* Port Control 2 Register */
#define PRV_CPSS_QD_REG_EGRESS_RATE_CTRL       0x09    /* Egress Rate Control Register */
#define PRV_CPSS_QD_REG_EGRESS_RATE_CTRL2      0x0A    /* Egress Rate Control 2 Register */
#define PRV_CPSS_QD_REG_PAV                    0x0B    /* Port Association Vector Register */
#define PRV_CPSS_QD_REG_PORT_ATU_CONTROL       0x0C    /* Port ATU Control Register */
#define PRV_CPSS_QD_REG_PRI_OVERRIDE           0x0D    /* Override Register */
#define PRV_CPSS_QD_REG_POLICY_CONTROL         0x0E    /* Policy Control Register */
#define PRV_CPSS_QD_REG_PORT_ETH_TYPE          0x0F    /* Port E Type Register */
#define PRV_CPSS_QD_REG_LED_CONTROL            0x16    /* LED Control Register */
#define PRV_CPSS_QD_REG_IP_PRI_MAPPING_TBL     0x17    /* IP Priority Mapping Table Register */
#define PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL   0x18    /* IEEE Priority Mapping Table Register */
#define PRV_CPSS_QD_REG_PORT_CONTROL3          0x19    /* Port Control 3 Register */
#define PRV_CPSS_QD_REG_Q_COUNTER              0x1B    /* Queue Counter Register */
#define PRV_CPSS_QD_REG_Q_CONTROL              0x1C    /* Port Queue Control Register */
#define PRV_CPSS_QD_REG_CT_CONTROL             0x1E    /* Cut Through Control Register */
#define PRV_CPSS_QD_REG_DBG_COUNTER            0x1F    /* Debug Counters Register */

/* Switch Global Registers */
#define PRV_CPSS_QD_REG_GLOBAL_STATUS          0x00    /* Global Status Register */
#define PRV_CPSS_QD_REG_ATU_FID_REG            0x01    /* ATU FID Register */
#define PRV_CPSS_QD_REG_VTU_FID_REG            0x02    /* VTU FID Register */
#define PRV_CPSS_QD_REG_STU_SID_REG            0x03    /* VTU SID Register */
#define PRV_CPSS_QD_REG_GLOBAL_CONTROL         0x04    /* Global Control Register */
#define PRV_CPSS_QD_REG_VTU_OPERATION          0x05    /* VTU Operation Register */
#define PRV_CPSS_QD_REG_VTU_VID_REG            0x06    /* VTU VID Register */
#define PRV_CPSS_QD_REG_VTU_DATA1_REG          0x07    /* VTU/STU Data Register Ports 0 to 7 */
#define PRV_CPSS_QD_REG_VTU_DATA2_REG          0x08    /* VTU/STU Data Register Ports 8 to 10 and Priority */
#define PRV_CPSS_QD_REG_ATU_CTRL_REG           0x0A    /* ATU Control Register */
#define PRV_CPSS_QD_REG_ATU_OPERATION          0x0B    /* ATU Operation Register */
#define PRV_CPSS_QD_REG_ATU_DATA_REG           0x0C    /* ATU Data Register */
#define PRV_CPSS_QD_REG_ATU_MAC_BASE           0x0D    /* ATU MAC Address Register Base */
#define PRV_CPSS_QD_REG_MONITOR_MGMT_CTRL      0x1A    /* Monitor & MGMT Control Register */
#define PRV_CPSS_QD_REG_TOTAL_FREE_COUNTER     0x1B    /* Total Free Counter Register */
#define PRV_CPSS_QD_REG_GLOBAL_CONTROL2        0x1C    /* Global Control 2 Register */
#define PRV_CPSS_QD_REG_STATS_OPERATION        0x1D    /* Stats Operation Register */
#define PRV_CPSS_QD_REG_STATS_COUNTER3_2       0x1E    /* Stats Counter Register Bytes 3 & 2 */
#define PRV_CPSS_QD_REG_STATS_COUNTER1_0       0x1F    /* Stats Counter Register Bytes 1 & 0 */

/* Switch Global 2 Registers */
#define PRV_CPSS_QD_REG_INT_SOURCE             0x00    /* Interrupt Source Register */
#define PRV_CPSS_QD_REG_INT_MASK               0x01    /* Interrupt Mask Register */
#define PRV_CPSS_QD_REG_FLOWCTRL_DELAY         0x04    /* Flow Control Delay Register */
#define PRV_CPSS_QD_REG_MANAGEMENT             0x05    /* Switch Management Register */
#define PRV_CPSS_QD_REG_ROUTING_TBL            0x06    /* Device Mapping Table Register */
#define PRV_CPSS_QD_REG_TRUNK_MASK_TBL         0x07    /* Trunk Mask Table Register */
#define PRV_CPSS_QD_REG_TRUNK_ROUTING          0x08    /* Trunk Mapping Register */
#define PRV_CPSS_QD_REG_IGR_RATE_COMMAND       0x09    /* Ingress Rate Command Register */
#define PRV_CPSS_QD_REG_IGR_RATE_DATA          0x0A    /* Ingress Rate Data Register */
#define PRV_CPSS_QD_REG_PVT_ADDR               0x0B    /* Cross Chip Port VLAN Addr Register */
#define PRV_CPSS_QD_REG_PVT_DATA               0x0C    /* Cross Chip Port VLAN Data Register */
#define PRV_CPSS_QD_REG_SWITCH_MAC             0x0D    /* Switch MAC/WoL/WoF Register */
#define PRV_CPSS_QD_REG_ATU_STATS              0x0E    /* ATU Stats Register */
#define PRV_CPSS_QD_REG_PRIORITY_OVERRIDE      0x0F    /* Priority Override Table Register */
#define PRV_CPSS_QD_REG_ENERGY_MNT             0x12    /* Energy Management Register */
#define PRV_CPSS_QD_REG_IMP_COMM_DBG           0x13    /* IMP Comm/Debug Register */
#define PRV_CPSS_QD_REG_EEPROM_COMMAND         0x14    /* EEPROM Command Register */
#define PRV_CPSS_QD_REG_EEPROM_DATA            0x15    /* EEPROM Address Register */
#define PRV_CPSS_QD_REG_AVB_COMMAND            0x16    /* AVB Command Register */
#define PRV_CPSS_QD_REG_AVB_DATA               0x17    /* AVB Data Register */
#define PRV_CPSS_QD_REG_SMI_PHY_CMD            0x18    /* SMI PHY Command Register */
#define PRV_CPSS_QD_REG_SMI_PHY_DATA           0x19    /* SMI PHY Data Register */
#define PRV_CPSS_QD_REG_SCRATCH_MISC           0x1A    /* Scratch & Misc Register */
#define PRV_CPSS_QD_REG_WD_CONTROL             0x1B    /* Watch Dog Control Register */
#define PRV_CPSS_QD_REG_QOS_WEIGHT             0x1C    /* QoS Weights Register */
#define PRV_CPSS_QD_REG_MISC                   0x1D    /* Misc Register */
#define PRV_CPSS_QD_REG_CUT_THROU_CTRL         0x1F    /* Cut Through Control Register */

/* TCAM Registers - Page 0 */
#define PRV_CPSS_QD_REG_TCAM_OPERATION         0x00    /* TCAM Operation Register */
#define PRV_CPSS_QD_REG_TCAM_P0_KEYS_1         0x02    /* TCAM Key Register 1 */
#define PRV_CPSS_QD_REG_TCAM_P0_KEYS_2         0x03    /* TCAM Key Register 2 */
#define PRV_CPSS_QD_REG_TCAM_P0_KEYS_3         0x04    /* TCAM Key Register 3 */
#define PRV_CPSS_QD_REG_TCAM_P0_KEYS_4         0x05    /* TCAM Key Register 4 */
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_1   0x06    /* TCAM Match Data Register 1 */
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_2   0x07    /* TCAM Match Data Register 2 */
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_3   0x08
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_4   0x09
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_5   0x0A
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_6   0x0B
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_7   0x0C
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_8   0x0D
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_9   0x0E
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_10  0x0F
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_11  0x10
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_12  0x11
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_13  0x12
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_14  0x13
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_15  0x14
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_16  0x15
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_17  0x16
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_18  0x17
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_19  0x18
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_20  0x19
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_21  0x1A
#define PRV_CPSS_QD_REG_TCAM_P0_MATCH_DATA_22  0x1B

/* TCAM Registers - Page 1 */
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_23  0x02    /* TCAM Match Data Register 23 */
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_24  0x03    /* TCAM Match Data Register 24 */
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_25  0x04
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_26  0x05
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_27  0x06
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_28  0x07
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_29  0x08
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_30  0x09
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_31  0x0A
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_32  0x0B
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_33  0x0C
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_34  0x0D
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_35  0x0E
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_36  0x0F
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_37  0x10
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_38  0x11
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_39  0x12
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_40  0x13
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_41  0x14
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_42  0x15
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_43  0x16
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_44  0x17
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_45  0x18
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_46  0x19
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_47  0x1A
#define PRV_CPSS_QD_REG_TCAM_P1_MATCH_DATA_48  0x1B

/* TCAM Registers - Page 2 */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_1       0x02    /* TCAM Action 1 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_2       0x03    /* TCAM Action 2 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_3       0x04    /* TCAM Action 3 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_5       0x06    /* TCAM Action 5 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_6       0x07    /* TCAM Action 6 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ACTION_7       0x08    /* TCAM Action 7 Register */
#define PRV_CPSS_QD_REG_TCAM_P2_DEBUG_PORT     0x1C    /* TCAM Debug Register */
#define PRV_CPSS_QD_REG_TCAM_P2_ALL_HIT        0x1F    /* TCAM Debug Register 2 */

/* TCAM Registers - Page 3 */
#define PRV_CPSS_QD_REG_TCAM_EGR_PORT          0x01    /* Egress TCAM Port Register */
#define PRV_CPSS_QD_REG_TCAM_EGR_ACTION_1      0x02    /* Egress TCAM Action 1 Register */
#define PRV_CPSS_QD_REG_TCAM_EGR_ACTION_2      0x03    /* Egress TCAM Action 2 Register */
#define PRV_CPSS_QD_REG_TCAM_EGR_ACTION_3      0x04    /* Egress TCAM Action 3 Register */


/******************************************************************************/
/*                                                                            */
/* Block: INTERNAL PHY Registers                                              */
/*                                                                            */
/******************************************************************************/

/* Internal Copper PHY registers*/
#define PRV_CPSS_QD_PHY_CONTROL_REG            0
#define PRV_CPSS_QD_PHY_AUTONEGO_AD_REG        4
#define PRV_CPSS_QD_PHY_AUTONEGO_1000AD_REG    9
#define PRV_CPSS_QD_PHY_XMDIO_ACCESS_CONTROL_REG         13
#define PRV_CPSS_QD_PHY_XMDIO_ACCESS_ADDRESS_DATA_REG    14
#define PRV_CPSS_QD_PHY_SPEC_CONTROL_REG       16
#define PRV_CPSS_QD_PHY_PAGE_ANY_REG           22

/* Definitions for VCT registers */
#define PRV_CPSS_QD_REG_MDI0_VCT_STATUS        16
#define PRV_CPSS_QD_REG_MDI1_VCT_STATUS        17
#define PRV_CPSS_QD_REG_MDI2_VCT_STATUS        18
#define PRV_CPSS_QD_REG_MDI3_VCT_STATUS        19
#define PRV_CPSS_QD_REG_ADV_VCT_CONTROL_5      23
#define PRV_CPSS_QD_REG_ADV_VCT_SMPL_DIST      24
#define PRV_CPSS_QD_REG_ADV_VCT_CONTROL_8      20
#define PRV_CPSS_QD_REG_PAIR_SKEW_STATUS       20
#define PRV_CPSS_QD_REG_PAIR_SWAP_STATUS       21

/* Bit Definition for PRV_CPSS_QD_PHY_CONTROL_REG */
#define PRV_CPSS_QD_PHY_RESET                  0x8000
#define PRV_CPSS_QD_PHY_LOOPBACK               0x4000
#define PRV_CPSS_QD_PHY_SPEED                  0x2000
#define PRV_CPSS_QD_PHY_AUTONEGO               0x1000
#define PRV_CPSS_QD_PHY_POWER                  0x0800
#define PRV_CPSS_QD_PHY_ISOLATE                0x0400
#define PRV_CPSS_QD_PHY_RESTART_AUTONEGO       0x0200
#define PRV_CPSS_QD_PHY_DUPLEX                 0x0100
#define PRV_CPSS_QD_PHY_SPEED_MSB              0x0040

/* Bit Definition for PRV_CPSS_QD_PHY_AUTONEGO_AD_REG */
#define PRV_CPSS_QD_PHY_NEXTPAGE               0x8000
#define PRV_CPSS_QD_PHY_REMOTEFAULT            0x4000
#define PRV_CPSS_QD_PHY_PAUSE                  0x0400
#define PRV_CPSS_QD_PHY_100_FULL               0x0100
#define PRV_CPSS_QD_PHY_100_HALF               0x0080
#define PRV_CPSS_QD_PHY_10_FULL                0x0040
#define PRV_CPSS_QD_PHY_10_HALF                0x0020

#define PRV_CPSS_QD_PHY_MODE_AUTO_AUTO         (PRV_CPSS_QD_PHY_100_FULL | PRV_CPSS_QD_PHY_100_HALF | PRV_CPSS_QD_PHY_10_FULL | PRV_CPSS_QD_PHY_10_HALF)
#define PRV_CPSS_QD_PHY_MODE_100_AUTO          (PRV_CPSS_QD_PHY_100_FULL | PRV_CPSS_QD_PHY_100_HALF)
#define PRV_CPSS_QD_PHY_MODE_10_AUTO           (PRV_CPSS_QD_PHY_10_FULL | PRV_CPSS_QD_PHY_10_HALF)
#define PRV_CPSS_QD_PHY_MODE_AUTO_FULL         (PRV_CPSS_QD_PHY_100_FULL | PRV_CPSS_QD_PHY_10_FULL)
#define PRV_CPSS_QD_PHY_MODE_AUTO_HALF         (PRV_CPSS_QD_PHY_100_HALF | PRV_CPSS_QD_PHY_10_HALF)

#define PRV_CPSS_QD_PHY_MODE_100_FULL          PRV_CPSS_QD_PHY_100_FULL
#define PRV_CPSS_QD_PHY_MODE_100_HALF          PRV_CPSS_QD_PHY_100_HALF
#define PRV_CPSS_QD_PHY_MODE_10_FULL           PRV_CPSS_QD_PHY_10_FULL
#define PRV_CPSS_QD_PHY_MODE_10_HALF           PRV_CPSS_QD_PHY_10_HALF

/* Gigabit Phy related definition */
#define PRV_CPSS_QD_GIGPHY_1000X_FULL_CAP      0x08
#define PRV_CPSS_QD_GIGPHY_1000X_HALF_CAP      0x04
#define PRV_CPSS_QD_GIGPHY_1000T_FULL_CAP      0x02
#define PRV_CPSS_QD_GIGPHY_1000T_HALF_CAP      0x01

#define PRV_CPSS_QD_GIGPHY_1000X_CAP           (PRV_CPSS_QD_GIGPHY_1000X_FULL_CAP | PRV_CPSS_QD_GIGPHY_1000X_HALF_CAP)
#define PRV_CPSS_QD_GIGPHY_1000T_CAP           (PRV_CPSS_QD_GIGPHY_1000T_FULL_CAP | PRV_CPSS_QD_GIGPHY_1000T_HALF_CAP)

#define PRV_CPSS_QD_GIGPHY_1000X_FULL          0x0020
#define PRV_CPSS_QD_GIGPHY_1000X_HALF          0x0040

#define PRV_CPSS_QD_GIGPHY_1000T_FULL          0x0200
#define PRV_CPSS_QD_GIGPHY_1000T_HALF          0x0100

#ifdef __cplusplus
}
#endif
#endif /* __prvCpssDrvGtDrvSwRegs_h */

