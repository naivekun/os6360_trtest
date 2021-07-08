#include <Copyright.h>
/**
********************************************************************************
* @file gtSysCtrl.h
*
* @brief API/Structure definitions for Marvell SysCtrl functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtSysCtrl.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell SysCtrl functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtSysCtrl_h
#define __prvCpssDrvGtSysCtrl_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported SysCtrl Types                                                   */
/****************************************************************************/

/* 0x00 to 0x01: Scratch Bytes  */
#define GT_CPSS_SCRAT_MISC_REG_SCRAT_0    0x00 /* Scratch Byte 0 */
#define GT_CPSS_SCRAT_MISC_REG_SCRAT_1    0x01 /* Scratch Byte 1 */
/* 0x02 to 0x1F: Reserved for future use  */
/* 0x20 to 0x3F: GPIO Port Stall Vectors  */
#define GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC0     0x20  /* GPIO 0 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC1     0x21  /* GPIO 0 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO1_STALL_VEC0     0x22  /* GPIO 1 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO1_STALL_VEC1     0x23  /* GPIO 1 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO2_STALL_VEC0     0x24  /* GPIO 2 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO2_STALL_VEC1     0x25  /* GPIO 2 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO3_STALL_VEC0     0x26  /* GPIO 3 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO3_STALL_VEC1     0x27  /* GPIO 3 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO4_STALL_VEC0     0x28  /* GPIO 4 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO4_STALL_VEC1     0x29  /* GPIO 4 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO5_STALL_VEC0     0x2A  /* GPIO 5 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO5_STALL_VEC1     0x2B  /* GPIO 5 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO6_STALL_VEC0     0x2C  /* GPIO 6 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO6_STALL_VEC1     0x2D  /* GPIO 6 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO7_STALL_VEC0     0x2E  /* GPIO 7 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO7_STALL_VEC1     0x2F  /* GPIO 7 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO8_STALL_VEC0     0x30  /* GPIO 8 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO8_STALL_VEC1     0x31  /* GPIO 8 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO9_STALL_VEC0     0x32  /* GPIO 9 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO9_STALL_VEC1     0x33  /* GPIO 9 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO10_STALL_VEC0    0x34  /* GPIO 10 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO10_STALL_VEC1    0x35  /* GPIO 10 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO11_STALL_VEC0    0x36  /* GPIO 11 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO11_STALL_VEC1    0x37  /* GPIO 11 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO12_STALL_VEC0    0x38  /* GPIO 12 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO12_STALL_VEC1    0x39  /* GPIO 12 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO13_STALL_VEC0    0x3A  /* GPIO 13 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO13_STALL_VEC1    0x3B  /* GPIO 13 Port Stall Vector 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO14_STALL_VEC0    0x3C  /* GPIO 14 Port Stall Vector 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO14_STALL_VEC1    0x3D  /* GPIO 14 Port Stall Vector 1 */
/* 0x60 to 0x6F: GPIO registers data and configuration  */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CFG_1           0x60  /* GPIO Configuration 0-7 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CFG_2           0x61  /* GPIO configuration 8-14 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_1           0x62  /* GPIO Direction 0-7 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_2           0x63  /* GPIO Direction 8-14 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_1           0x64  /* GPIO Data 0-7 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_2           0x65  /* GPIO Data 8-14 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_0          0x68  /* GPIO Control 0 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_1          0x69  /* GPIO Control 1 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_2          0x6A  /* GPIO Control 2 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_3          0x6B  /* GPIO Control 3 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_4          0x6C  /* GPIO Control 4 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_5          0x6D  /* GPIO Control 5 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_6          0x6E  /* GPIO Control 6 */
#define GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_7          0x6F  /* GPIO Control 7 */
/* 0x70 to 0x7F: CONFIG reads and Pad Drive  */
#define GT_CPSS_SCRAT_MISC_REG_CFG_DAT0             0x70  /* CONFIG Data 0 */
#define GT_CPSS_SCRAT_MISC_REG_CFG_DAT1             0x71  /* CONFIG Data 1 */
#define GT_CPSS_SCRAT_MISC_REG_CFG_DAT2             0x72  /* CONFIG Data 2 */
#define GT_CPSS_SCRAT_MISC_REG_CFG_DAT3             0x73  /* CONFIG Data 3 */
#define GT_CPSS_SCRAT_MISC_REG_MAX                  0x80  /* Maximun register pointer */

/****************************************************************************/
/* Exported SysCtrl Functions                                               */
/****************************************************************************/

/**
* @internal prvCpssDrvGsysSetDuplexPauseMac function
* @endinternal
*
* @brief   This routine sets the full duplex pause src Mac Address.
*         MAC address should be an Unicast address.
*         For different MAC Addresses per port operation,
*         use gsysSetPerPortDuplexPauseMac API.
* @param[in] mac                      - The Mac address to be set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysSetDuplexPauseMac
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_ETHERADDR    *mac
);

/**
* @internal prvCpssDrvGsysSetFloodBC function
* @endinternal
*
* @brief   Flood Broadcast.
*         When Flood Broadcast is enabled, frames with the Broadcast destination
*         address will flood out all the ports regardless of the setting of the
*         port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and
*         other switch policy still applies to these Broadcast frames.
*         When this feature is disabled, frames with the Broadcast destination
*         address are considered Multicast frames and will be affected by port's
*         Egress Floods mode.
* @param[in] en                       - GT_TRUE to enable Flood Broadcast, GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysSetFloodBC
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         en
);

/**
* @internal prvCpssDrvGsysGetFloodBC function
* @endinternal
*
* @brief   Flood Broadcast.
*         When Flood Broadcast is enabled, frames with the Broadcast destination
*         address will flood out all the ports regardless of the setting of the
*         port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and
*         other switch policy still applies to these Broadcast frames.
*         When this feature is disabled, frames with the Broadcast destination
*         address are considered Multicast frames and will be affected by port's
*         Egress Floods mode.
*
* @param[out] en                       - GT_TRUE if Flood Broadcast is enabled, GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetFloodBC
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_BOOL         *en
);

/**
* @internal prvCpssDrvGsysSetTrunkMaskTable function
* @endinternal
*
* @brief   This function sets Trunk Mask for the given Mask Number.
*
* @param[in] maskNum                  - Mask Number.
* @param[in] trunkMask                - Trunk mask bits. Bit 0 controls trunk masking for port 0,
*                                      bit 1 for port 1 , etc.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetTrunkMaskTable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          maskNum,
    IN  GT_U32          trunkMask
);

/**
* @internal prvCpssDrvGsysGetTrunkMaskTable function
* @endinternal
*
* @brief   This function gets Trunk Mask for the given Trunk Number.
*
* @param[in] maskNum                  - Mask Number.
*
* @param[out] trunkMask                - Trunk mask bits. Bit 0 controls trunk masking for port 0,
*                                      bit 1 for port 1 , etc.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given (if maskNum > 0x7)
*/
GT_STATUS prvCpssDrvGsysGetTrunkMaskTable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          maskNum,
    OUT GT_U32          *trunkMask
);

/**
* @internal prvCpssDrvGsysSetHashTrunk function
* @endinternal
*
* @brief   Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished
*         by using the frame's DA and SA fields to access one of eight Trunk Masks.
*         When this bit is set to a one the hashed computed for address table
*         lookups is used for the TrunkMask selection. When this bit is cleared to
*         a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to
*         select the TrunkMask to use.
* @param[in] en                       - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysSetHashTrunk
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         en
);

/**
* @internal prvCpssDrvGsysGetHashTrunk function
* @endinternal
*
* @brief   Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished
*         by using the frame's DA and SA fields to access one of eight Trunk Masks.
*         When this bit is set to a one the hashed computed for address table
*         lookups is used for the TrunkMask selection. When this bit is cleared to
*         a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to
*         select the TrunkMask to use.
*
* @param[out] en                       - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetHashTrunk
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_BOOL         *en
);

/**
* @internal prvCpssDrvGsysSetTrunkRouting function
* @endinternal
*
* @brief   This function sets routing information for the given Trunk ID.
*
* @param[in] trunkId                  - Trunk ID.
* @param[in] trunkRoute               - Trunk route bits. Bit 0 controls trunk routing for port 0,
*                                      bit 1 for port 1 , etc.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given (if trunkId > 0x1F)
*/
GT_STATUS prvCpssDrvGsysSetTrunkRouting
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          trunkId,
    IN  GT_U32          trunkRoute
);

/**
* @internal prvCpssDrvGsysGetTrunkRouting function
* @endinternal
*
* @brief   This function retrieves routing information for the given Trunk ID.
*
* @param[in] trunkId                  - Trunk ID.
*
* @param[out] trunkRoute               - Trunk route bits. Bit 0 controls trunk routing for port 0,
*                                      bit 1 for port 1 , etc.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given (if trunkId > 0x1F)
*/
GT_STATUS prvCpssDrvGsysGetTrunkRouting
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          trunkId,
    OUT GT_U32          *trunkRoute
);

/**
* @internal prvCpssDrvGsysSetMonitorMgmtCtrl function
* @endinternal
*
* @brief   Set Monitor and mgmt control data.
*
* @param[in] point                    - Pointer to the Monitor and mgmt control register.
*                                      The Pointer bits are used to access the Index registers as follows:
*                                      0x00 to 0x01: Rsvd2Cpu Enables for 01:C2:80:00:00:0x
*                                      0x02 to 0x03: Rsvd2Cpu Enables for 01:C2:80:00:00:2x
*                                      0x04 to 0x05: Rsvd2IntCpu Enables for 01:C2:80:00:00:0x
*                                      0x06 to 0x07: Rsvd2IntCpu Enables for 01:C2:80:00:00:2x
*                                      0x04 to 0x1F: Reserved for future use.
*                                      0x20 to 0x2F: Mirror Destination Port Settings
*                                      0x30 to 0x3F: Trap Destination Port Settings
* @param[in] data                     - Monitor and mgmt Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetMonitorMgmtCtrl
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           point,
    IN  GT_U8           data
);

/**
* @internal prvCpssDrvGsysGetMonitorMgmtCtrl function
* @endinternal
*
* @brief   Get Monitor and mgmt control data.
*
* @param[in] point                    - Pointer to the Monitor and mgmt control register.
*                                      The Pointer bits are used to access the Index registers as follows:
*                                      0x00 to 0x01: Rsvd2Cpu Enables for 01:C2:80:00:00:0x
*                                      0x02 to 0x03: Rsvd2Cpu Enables for 01:C2:80:00:00:2x
*                                      0x04 to 0x05: Rsvd2IntCpu Enables for 01:C2:80:00:00:0x
*                                      0x06 to 0x07: Rsvd2IntCpu Enables for 01:C2:80:00:00:2x
*                                      0x04 to 0x1F: Reserved for future use.
*                                      0x20 to 0x2F: Mirror Destination Port Settings
*                                      0x30 to 0x3F: Trap Destination Port Settings
*
* @param[out] data                     - Monitor and mgmt Control  written to the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysGetMonitorMgmtCtrl
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           point,
    OUT GT_U8           *data
);

/**
* @internal prvCpssDrvGsysSetCPUDest function
* @endinternal
*
* @brief   This routine sets CPU Destination Port. CPU Destination port indicates the
*         port number on this device where the CPU is connected (either directly or
*         indirectly through another Marvell switch device).
*         Many modes of frame processing need to know where the CPU is located.
*         These modes are:
*         1. When IGMP/MLD frame is received and Snooping is enabled
*         2. When the port is configured as a DSA port and it receives a To_CPU frame
*         3. When a Rsvd2CPU frame enters the port
*         4. When the port's SA Filtering mode is Drop to CPU
*         5. When any of the port's Policy Options trap the frame to the CPU
*         6. When the ingressing frame is an ARP and ARP mirroring is enabled in the
*         device
*         In all cases, except for ARP, the frames that meet the enabled criteria
*         are mapped to the CPU Destination port, overriding where the frame would
*         normally go. In the case of ARP, the frame will be mapped normally and it
*         will also get copied to this port.
*         Frames that filtered or discarded will not be mapped to the CPU Destination
*         port with the exception of the Rsvd2CPU and DSA Tag cases.
*         If CPUDest = 0x1F, the remapped frames will be discarded, no ARP mirroring
*         will occur and ingressing To_CPU frames will be discarded.
* @param[in] port                     - the logical  number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetCPUDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port
);

/**
* @internal prvCpssDrvGsysGetCPUDest function
* @endinternal
*
* @brief   This routine gets CPU Destination Port. CPU Destination port indicates the
*         port number on this device where the CPU is connected (either directly or
*         indirectly through another Marvell switch device).
*         Many modes of frame processing need to know where the CPU is located.
*         These modes are:
*         1. When IGMP/MLD frame is received and Snooping is enabled
*         2. When the port is configured as a DSA port and it receives a To_CPU frame
*         3. When a Rsvd2CPU frame enters the port
*         4. When the port's SA Filtering mode is Drop to CPU
*         5. When any of the port's Policy Options trap the frame to the CPU
*         6. When the ingressing frame is an ARP and ARP mirroring is enabled in the
*         device
*         In all cases, except for ARP, the frames that meet the enabled criteria
*         are mapped to the CPU Destination port, overriding where the frame would
*         normally go. In the case of ARP, the frame will be mapped normally and it
*         will also get copied to this port.
*         Frames that filtered or discarded will not be mapped to the CPU Destination
*         port with the exception of the Rsvd2CPU and DSA Tag cases.
*         If CPUDest = 0x1F, the remapped frames will be discarded, no ARP mirroring
*         will occur and ingressing To_CPU frames will be discarded.
*
* @param[out] port                     - the logical  number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetCPUDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_CPSS_LPORT   *port
);

/**
* @internal prvCpssDrvGsysSetIngressMonitorDest function
* @endinternal
*
* @brief   This routine sets Ingress Monitor Destination Port. Frames that are
*         targeted toward an Ingress Monitor Destination go out the port number
*         indicated in these bits. This includes frames received on a Marvell Tag port
*         with the Ingress Monitor type, and frames received on a Network port that
*         is enabled to be the Ingress Monitor Source Port.
*         If the Ingress Monitor Destination Port resides in this device these bits
*         should point to the Network port where these frames are to egress. If the
*         Ingress Monitor Destination Port resides in another device these bits
*         should point to the Marvell Tag port in this device that is used to get
*         to the device that contains the Ingress Monitor Destination Port.
* @param[in] port                     - the logical  number.when  = 0x1F,IMD is disabled.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetIngressMonitorDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port
);

/**
* @internal prvCpssDrvGsysGetIngressMonitorDest function
* @endinternal
*
* @brief   This routine gets Ingress Monitor Destination Port.
*
* @param[out] port                     - the logical  number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetIngressMonitorDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_CPSS_LPORT   *port
);

/**
* @internal prvCpssDrvGsysSetEgressMonitorDest function
* @endinternal
*
* @brief   This routine sets Egress Monitor Destination Port. Frames that are
*         targeted toward an Egress Monitor Destination go out the port number
*         indicated in these bits. This includes frames received on a Marvell Tag port
*         with the Egress Monitor type, and frames transmitted on a Network port that
*         is enabled to be the Egress Monitor Source Port.
*         If the Egress Monitor Destination Port resides in this device these bits
*         should point to the Network port where these frames are to egress. If the
*         Egress Monitor Destination Port resides in another device these bits
*         should point to the Marvell Tag port in this device that is used to get
*         to the device that contains the Egress Monitor Destination Port.
* @param[in] port                     - the logical  number.If  = 0x1F, EMD is disabled
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetEgressMonitorDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port
);

/**
* @internal prvCpssDrvGsysGetEgressMonitorDest function
* @endinternal
*
* @brief   This routine gets Egress Monitor Destination Port.
*
* @param[out] port                     - the logical  number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetEgressMonitorDest
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_CPSS_LPORT   *port
);

/**
* @internal prvCpssDrvGsysSetRsvd2CpuEnables function
* @endinternal
*
* @brief   Reserved DA Enables. When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one,
*         the 16 reserved multicast DA addresses, whose bit in this register are
*         also set to a one, are treadted as MGMT frames. All the reserved DA's
*         take the form 01:80:C2:00:00:0x. When x = 0x0, bit 0 of this register is
*         tested. When x = 0x2, bit 2 of this field is tested and so on.
*         If the tested bit in this register is cleared to a zero, the frame will
*         be treated as a normal (non-MGMT) frame.
* @param[in] enBits                   - bit vector of enabled Reserved Multicast.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGsysSetRsvd2CpuEnables
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          enBits
);

/**
* @internal prvCpssDrvGsysGetRsvd2CpuEnables function
* @endinternal
*
* @brief   Reserved DA Enables. When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one,
*         the 16 reserved multicast DA addresses, whose bit in this register are
*         also set to a one, are treadted as MGMT frames. All the reserved DA's
*         take the form 01:80:C2:00:00:0x. When x = 0x0, bit 0 of this register is
*         tested. When x = 0x2, bit 2 of this field is tested and so on.
*         If the tested bit in this register is cleared to a zero, the frame will
*         be treated as a normal (non-MGMT) frame.
*
* @param[out] enBits                   - bit vector of enabled Reserved Multicast.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetRsvd2CpuEnables
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_U16          *enBits
);

/**
* @internal prvCpssDrvGsysSetMGMTPri function
* @endinternal
*
* @brief   These bits are used as the PRI[2:0] bits on Rsvd2CPU frames.
*
* @param[in] pri                      - PRI[2:0] bits (should be less than 8)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given (If pri is not less than 8)
*/
GT_STATUS prvCpssDrvGsysSetMGMTPri
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          pri
);

/**
* @internal prvCpssDrvGsysGetMGMTPri function
* @endinternal
*
* @brief   These bits are used as the PRI[2:0] bits on Rsvd2CPU frames.
*
* @param[out] pri                      - PRI[2:0] bits (should be less than 8)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysGetMGMTPri
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_U16          *pri
);


/**
* @internal prvCpssDrvGsysSetScratchMiscCtrl function
* @endinternal
*
* @brief   Set Scratch and Misc control data to the Scratch and Misc Control register.
*         The registers of Scratch and Misc control are.
*         Scratch Byte 0
*         Scratch Byte 1
*         GPIO Configuration
*         Reserved for future use
*         GPIO Direction
*         GPIO Data
*         CONFIG Data 0
*         CONFIG Data 1
*         CONFIG Data 2
*         CONFIG Data 3
* @param[in] point                    - Pointer to the Scratch and Misc. Control register.
* @param[in] data                     - Scratch and Misc. Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetScratchMiscCtrl
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U8             point,
    IN  GT_U8             data
);

/**
* @internal prvCpssDrvGsysGetScratchMiscCtrl function
* @endinternal
*
* @brief   Get Scratch and Misc control data from the Scratch and Misc Control register.
*         The register of Scratch and Misc control are.
*         Scratch Byte 0
*         Scratch Byte 1
*         GPIO Configuration
*         Reserved for future use
*         GPIO Direction
*         GPIO Data
*         CONFIG Data 0
*         CONFIG Data 1
*         CONFIG Data 2
*         CONFIG Data 3
* @param[in] point                    - Pointer to the Scratch and Misc. Control register.
*
* @param[out] data                     - Scratch and Misc. Control  read from the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetScratchMiscCtrl
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            point,
    OUT GT_U8            *data
);

/**
* @internal prvCpssDrvGsysSetScratchBits function
* @endinternal
*
* @brief   Set bits to the Scratch and Misc Control register <scratch byte 0 and 1>.
*         These bits are 100% available to software for whatever purpose desired.
*         These bits do not connect to any hardware function.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetScratchBits
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U16          scratch
);

/**
* @internal prvCpssDrvGsysGetScratchBits function
* @endinternal
*
* @brief   Get bits from the Scratch and Misc Control register <scratch byte 0 and 1>.
*         These bits are 100% available to software for whatever purpose desired.
*         These bits do not connect to any hardware function.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetScratchBits
(
    IN  GT_CPSS_QD_DEV      *dev,
    OUT GT_U16              *scratch
);

/**
* @internal prvCpssDrvGsysGetGpioConfigMod function
* @endinternal
*
* @brief   Get mode from the Scratch and Misc Control register <GPIO Configuration>.
*         The bits are shared General Purpose Input Output mode Bits:
*         Bit 15 - GT_GPIO_BIT_14:  1:GPIO[14]  0:P5_OUTCLK
*         ...
*         Bit 6 - GT_GPIO_BIT_6:  1:GPIO[6]  0:SE_RCLK1
*         Bit 5 - GT_GPIO_BIT_5:  1:GPIO[5]  0:SE_RCLK0
*         Bit 4 - GT_GPIO_BIT_4:  1:GPIO[4]  0:
*         Bit 3 - GT_GPIO_BIT_3:  1:GPIO[3]  0:
*         Bit 2 - GT_GPIO_BIT_2:  1:GPIO[2]  0:
*         Bit 1 - GT_GPIO_BIT_1:  1:GPIO[1]  0:P6_COL
*         Bit 0 - GT_GPIO_BIT_0:  1:GPIO[0]  0:P6_CRS
*         Now, bits are read only, except for 0, and 7.
*
* @param[out] mode                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioConfigMod
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              *mode
);

/**
* @internal prvCpssDrvGsysSetGpioPinControl function
* @endinternal
*
* @brief   Set GPIO pin control to the Scratch and Misc Control register <GPIO pin control>
*         General Purpose Input Output Interface pin x Control. This control is
*         used to control alternate functions of the GPIO[x] pin when it is not
*         being used as part of some other interface. This control has an effect
*         only if the pin is enabled to be a GPIO pin, i.e., the GPIO[x] Mode bit
*         is a one (Register Index 0x60 of Scratch and Misc., Control).
*         The options are as follows:
*         GT_GPIO_PIN_GPIO
*         GT_GPIO_PIN_PTP_TRIG
*         GT_GPIO_PIN_PTP_EVREQ
*         GT_GPIO_PIN_PTP_EXTCLK
*         GT_GPIO_PIN_RX_CLK0
*         GT_GPIO_PIN_RX_CLK1
*         GT_GPIO_PIN_SDET
*         GT_GPIO_PIN_CLK125
* @param[in] pinNum                   - pin number.
* @param[in] pinCtrl                  - pin control.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioPinControl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               pinNum,
    IN  GT_U8               pinCtrl
);

/**
* @internal prvCpssDrvGsysGetGpioPinControl function
* @endinternal
*
* @brief   Get GPIO pin control to the Scratch and Misc Control register <GPIO pin control>.
*         General Purpose Input Output Interface pin x Control. This control is
*         used to control alternate functions of the GPIO[x] pin when it is not
*         being used as part of some other interface. This control has an effect
*         only if the pin is enabled to be a GPIO pin, i.e., the GPIO[x] Mode bit
*         is a one (Register Index 0x60 of Scratch and Misc., Control).
*         The options are as follows:
*         GT_GPIO_PIN_GPIO
*         GT_GPIO_PIN_PTP_TRIG
*         GT_GPIO_PIN_PTP_EVREQ
*         GT_GPIO_PIN_PTP_EXTCLK
*         GT_GPIO_PIN_RX_CLK0
*         GT_GPIO_PIN_RX_CLK1
*         GT_GPIO_PIN_SDET
*         GT_GPIO_PIN_CLK125
* @param[in] pinNum                   - pin number.
*
* @param[out] pinCtrl                  - pin control.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioPinControl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               pinNum,
    OUT GT_U8               *pinCtrl
);

/**
* @internal prvCpssDrvGsysSetGpioDirection function
* @endinternal
*
* @brief   Set Gpio direction to the Scratch and Misc Control register <GPIO Direction>.
*         The bits are used to control the direction of GPIO[6:0] or GPIO(14:0].
*         When a GPIO bit is set to a one that GPIO will become an input. When a
*         GPIO bit is cleared to a zero that GPIO will become an output
*         General Purpose Input Output direction bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
* @param[in] dir                      - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioDirection
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U32          dir
);

/**
* @internal prvCpssDrvGsysGetGpioDirection function
* @endinternal
*
* @brief   Get Gpio direction from the Scratch and Misc Control register <GPIO Direction>.
*         The bits are used to control the direction of GPIO[6:0] or GPIO(14:0].
*         When a GPIO bit is set to a one that GPIO will become an input. When a
*         GPIO bit is cleared to a zero that GPIO will become an output
*         General Purpose Input Output direction bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
*
* @param[out] dir                      - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioDirection
(
    IN  GT_CPSS_QD_DEV      *dev,
    OUT GT_U32              *dir
);

/**
* @internal prvCpssDrvGsysSetGpioSMI function
* @endinternal
*
* @brief   Set Normal SMI to the Scratch and Misc Control register <GPIO Direction>.
*         The bit is used to control the Normal SMI vs. GPIO mode.
*         When P5_MODE is not equal to 0x1 or 0x2 the P5_COL and P5_CRS pins are
*         not needed. In this case, when this bit is set to a one, the P5_COL and
*         P5_CRS pins become MDIO_PHY and MDC_PHY, respectively, if the NO_CPU
*         configuration pin was a one during reset. Else the pins become GPIO pins 7 and 8.
* @param[in] smi                      - SMI OR GT_GPIO
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioSMI
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_BOOL             smi
);

/**
* @internal prvCpssDrvGsysGetGpioSMI function
* @endinternal
*
* @brief   Get Normal SMI from the Scratch and Misc Control register <GPIO Direction>.
*         The bit is used to control the Normal SMI vs. GPIO mode.
*         When P5_MODE is not equal to 0x1 or 0x2 the P5_COL and P5_CRS pins are
*         not needed. In this case, when this bit is set to a one, the P5_COL and
*         P5_CRS pins become MDIO_PHY and MDC_PHY, respectively, if the NO_CPU
*         configuration pin was a one during reset. Else the pins become GPIO pins 7 and 8.
*
* @param[out] smi                      - SMI OR GT_GPIO
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioSMI
(
    IN  GT_CPSS_QD_DEV      *dev,
    OUT GT_BOOL             *smi
);

/**
* @internal prvCpssDrvGsysSetGpioData function
* @endinternal
*
* @brief   Set Gpio data to the Scratch and Misc Control register <GPIO data>.
*         When a GPIO bit is set to be an input, data written to this bit will go
*         to a holding register but will not appear on the pin nor in this register.
*         Reads of this register will return the actual, real-time, data that is
*         appearing on the GPIO pin.
*         When a GPIO bit is set to be an output, data written to this bit will go
*         to a holding register and will appear on the GPIO pin. Reads of this register
*         will return the actual, real-time, data that is appearing on the GPIO pin
*         (which in this case should be the data written, but if its isn't that would
*         be an indication of a conflict).
*         When a pin direction changes from input to output, the data last written
*         to the holding register appears on the GPIO pin
*         General Purpose Input Output data bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 5 - GT_GPIO_BIT_5
*         Bit 4 - GT_GPIO_BIT_4
*         Bit 3 - GT_GPIO_BIT_3
*         Bit 2 - GT_GPIO_BIT_2
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
* @param[in] data                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioData
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              data
);

/**
* @internal prvCpssDrvGsysGetGpioData function
* @endinternal
*
* @brief   get Gpio data to the Scratch and Misc Control register <GPIO data>.
*         When a GPIO bit is set to be an input, data written to this bit will go
*         to a holding register but will not appear on the pin nor in this register.
*         Reads of this register will return the actual, real-time, data that is
*         appearing on the GPIO pin.
*         When a GPIO bit is set to be an output, data written to this bit will go
*         to a holding register and will appear on the GPIO pin. Reads of this register
*         will return the actual, real-time, data that is appearing on the GPIO pin
*         (which in this case should be the data written, but if its isn't that would
*         be an indication of a conflict).
*         When a pin's direction changes from input to output, the data last written
*         to the holding register appears on the GPIO pin
*         General Purpose Input Output data bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 5 - GT_GPIO_BIT_5
*         Bit 4 - GT_GPIO_BIT_4
*         Bit 3 - GT_GPIO_BIT_3
*         Bit 2 - GT_GPIO_BIT_2
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
*
* @param[out] data                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioData
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              *data
);

/**
* @internal prvCpssDrvGsysSetGpioxPortStallVect function
* @endinternal
*
* @brief   Set GPIO X Port Stall Vector. The value in this register is sent to the transmit
*         portion of all the ports in the device when GPIO X Port Stall En (below) is set
*         to a one and the value on the device's GPIO pin matches the value in GPIO
*         X Port Stall Value (below) and the GPIO pin is a GPIO input. The assertion
*         of this vector to the ports persists as long as the function is enabled (i.e., GPIO
*         X Port Stall En equals a one) and as long as the value on the pin matches the
*         programmed stall value (GPIO X Port Stall Value) and as long as the pin
*         remains a GPIO input.
*         When a port's bit is set to a one in this vector, that port or ports will stop
*         tranmitting their next frame (the current frame is not affected) until this vector
*         is deasserted. Port 0 is controlled by bit 0, port 1 by bit 1, etc.
* @param[in] gpioNum                  - GPIO number.
* @param[in] portStallVec             - GPIO Num Port Stall Vector.
* @param[in] portStallEn              - GPIO Num Port Stall Enable.
* @param[in] portStallValue           - GPIO Num Port Stall Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioxPortStallVect
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               gpioNum,
    IN  GT_U8               portStallVec,
    IN  GT_U8               portStallEn,
    IN  GT_U8               portStallValue
);

/**
* @internal prvCpssDrvGsysGetGpioxPortStallVect function
* @endinternal
*
* @brief   Get GPIO X Port Stall Vector. The value in this register is sent to the transmit
*         portion of all the ports in the device when GPIO X Port Stall En (below) is set
*         to a one and the value on the device's GPIO pin matches the value in GPIO
*         X Port Stall Value (below) and the GPIO pin is a GPIO input. The assertion
*         of this vector to the ports persists as long as the function is enabled (i.e., GPIO
*         X Port Stall En equals a one) and as long as the value on the pin matches the
*         programmed stall value (GPIO X Port Stall Value) and as long as the pin
*         remains a GPIO input.
*         When a port's bit is set to a one in this vector, that port or ports will stop
*         tranmitting their next frame (the current frame is not affected) until this vector
*         is deasserted. Port 0 is controlled by bit 0, port 1 by bit 1, etc.
* @param[in] gpioNum                  - GPIO number.
*
* @param[out] portStallVec             - GPIO Num Port Stall Vector.
* @param[out] portStallEn              - GPIO Num Port Stall Enable.
* @param[out] portStallValue           - GPIO Num Port Stall Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioxPortStallVect
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               gpioNum,
    OUT GT_U8               *portStallVec,
    OUT GT_U8               *portStallEn,
    OUT GT_U8               *portStallValue
);

/**
* @internal prvCpssDrvGdevSetQosWeightCtrl function
* @endinternal
*
* @brief   Set QoS Weight control data from the QoS Weight Control register.
*         The register of QoS Weight control are one of 64 possible QoS Weight
*         Data registers and the QoS Weight Length register.
* @param[in] point                    - Pointer to the QoS Weight register.
* @param[in] data                     - QoS Weight Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGdevSetQosWeightCtrl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               point,
    IN  GT_U8               data
);

/**
* @internal prvCpssDrvGdevGetQosWeightCtrl function
* @endinternal
*
* @brief   Set QoS Weight control data from the QoS Weight Control register.
*         The register of QoS Weight control are one of 64 possible QoS Weight
*         Data registers and the QoS Weight Length register.
* @param[in] point                    - Pointer to the QoS Weight register.
*
* @param[out] data                     - QoS Weight Control  written to the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGdevGetQosWeightCtrl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U8               point,
    OUT GT_U8               *data
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtSysCtrl_h */

