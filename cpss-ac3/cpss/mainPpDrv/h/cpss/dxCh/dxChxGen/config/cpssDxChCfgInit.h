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
* @file cpssDxChCfgInit.h
*
* @brief CPSS DxCh initialization of PPs and shadow data structures, and
* declarations of global variables.
*
* @version   40
********************************************************************************
*/
#ifndef __cpssDxChCfgInith
#define __cpssDxChCfgInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/phy/cpssGenPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/generic/port/cpssPortTx.h>

/**
* @enum CPSS_DXCH_CFG_TABLES_ENT
 *
 * @brief the type of tables that the DXCH devices hold.
 * NOTE: some tables may shared on same memory space (like router and IPCL)
*/
typedef enum{

    /** table type represent the VLAN table */
    CPSS_DXCH_CFG_TABLE_VLAN_E,

    /** table type represent the FDB table */
    CPSS_DXCH_CFG_TABLE_FDB_E,

    /** table type represent the PCL action table */
    CPSS_DXCH_CFG_TABLE_PCL_ACTION_E,

    /** table type represent the PCL Tcam table */
    CPSS_DXCH_CFG_TABLE_PCL_TCAM_E,

    /** table type represent the router next hop table */
    CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E,

    /** table type represent the router lookup translation table (LTT) */
    CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E,

    /** @brief table type represent the Router Tcam table
     *  Note: take in account that cpssDxChCfgTableNumEntriesGet
     *  function will return number of entries for IPv6,
     *  while actual number of IPv4 entries is 4(5 for DxCh2) times bigger.
     */
    CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E,

    /** table type represent the L3 ECMP/QoS table */
    CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E,

    /** table type represent the TTI table */
    CPSS_DXCH_CFG_TABLE_TTI_TCAM_E,

    /** @brief table type represent the MLL pair table
     *  (the MLLs reside as pair in each entry)
     */
    CPSS_DXCH_CFG_TABLE_MLL_PAIR_E,

    /** table type represent the policer metering table */
    CPSS_DXCH_CFG_TABLE_POLICER_METERS_E,

    /** table type represent the policer billing counters table */
    CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E,

    /** table type represent the VIDX (multicast groups) table */
    CPSS_DXCH_CFG_TABLE_VIDX_E,

    /** table type represent the ARP entries in ARP/Tunnel Start table */
    CPSS_DXCH_CFG_TABLE_ARP_E,

    /** table type represent Tunnel Start entries in the ARP/Tunnel Start table */
    CPSS_DXCH_CFG_TABLE_TUNNEL_START_E,

    /** table type represent the STG (spanning tree groups) table */
    CPSS_DXCH_CFG_TABLE_STG_E,

    /** table type represent the QOS profile table */
    CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E,

    /** table type represent the Mac to me table */
    CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E,

    /** @brief table type represent the centralized counters (CNC) table
     *  (the number of counters (X per block) , so 8 blocks means 8X counters).
     *  Refer to device datasheet to see number of CNC blocks and the number of
     *  counters per CNC block.
     */
    CPSS_DXCH_CFG_TABLE_CNC_E,

    /** @brief table type represent CNC block (the number of conters per CNC block).
     *  Refer to device datasheet to see number of counters per CNC block.
     */
    CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E,

    /** table type represent trunk table. */
    CPSS_DXCH_CFG_TABLE_TRUNK_E,

    /** table type represent LPM RAM */
    CPSS_DXCH_CFG_TABLE_LPM_RAM_E,

    /** table type represent router ECMP table */
    CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E,

    /** table type represent L2 MLL LTT */
    CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E,

    /** table type represent ePorts table */
    CPSS_DXCH_CFG_TABLE_EPORT_E,

    /** table type represent default ePorts table */
    CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E,

    /** table type represent physical Ports table */
    CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E,

    /** table type represent the Exact Match table
     *  (APPLICABLE DEVICES : FALCON.) */
    CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,

    /** indication of the last table (not represents a table) */
    CPSS_DXCH_CFG_TABLE_LAST_E

} CPSS_DXCH_CFG_TABLES_ENT;

/**
* @enum CPSS_DXCH_CFG_ROUTING_MODE_ENT
 *
 * @brief Routing mode
*/
typedef enum{

    /** @brief PBR (Policy based routing)
     *  using PCL action as next hop.
     */
    CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E = 0,

    /** @brief Regular routing using router engine
     *  or PBR using next hop table.
     */
    CPSS_DXCH_TCAM_ROUTER_BASED_E         = 1

} CPSS_DXCH_CFG_ROUTING_MODE_ENT;

/**
* @enum CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT
 *
 * @brief how egress pipe get 6 bits hash from the ingress 12 bits hash. (EQ-->TXQ)
*/
typedef enum{

    /** @brief Use 6 Lsbits
     *  egressHash[5:0] = ingressHash[5:0]
     */
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E,

    /** @brief Use 6 Msbits
     *  egressHash[5:0] = ingressHash[11:6]
     */
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E,

    /** @brief Use 12 bits; The 6 lsbits are XORed with the 6 msbits.
     *  egressHash[5:0] = ingressHash[5:0] ^ ingressHash[11:6]
     */
    CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E

} CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT;

/**
* @enum CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT
 *
 * @brief Ingress Drop Counter Mode types.
*/
typedef enum{

    /** count all ingress dropped packets. */
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E,

    /** @brief Count all ingress dropped packets assigned with
     *  a specific VLAN id.
     */
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E,

    /** @brief Count all ingress dropped packets received
     *  on a specific port
     */
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E

} CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT;

/**
* @enum CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT
 *
 * @brief shared tables configuration modes for following clients:
 * L3 (LPM), L2 (FDB), EM (Exact Match)
 * (APPLICABLE DEVICES : FALCON)
*/
typedef enum{

    /** @brief This mode :
     * 1. Maximum L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Minimum L3 table size (LPM)
     * 2. Maximum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Medium L2 table size (FDB)
     * 3. Minimum EM table size Exact Match)
     */
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. Maximum EM table size (Exact Match)
     */
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E

} CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT;


/**
* @enum CPSS_DXCH_LPM_RAM_MEM_MODE_ENT
 *
 * @brief Defines the mode of LPM MEM module
*/
typedef enum{

    /** @brief each RAM pool will be fully utulized
     *  This can cause violation of full wire-speed.
     */
    CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,

    /** @brief each RAM pool will be half utulized in order to support full wirespeed.
     *  The second half of each RAM pool will be identical to first.
     */
    CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E

} CPSS_DXCH_LPM_RAM_MEM_MODE_ENT;

/**
* @struct CPSS_DXCH_PP_CONFIG_INIT_STC
 *
 * @brief DxCh Packet Processor (PP) init configuration Parameters
 * structure
*/
typedef struct{

    /** @brief PP routing mode: Policy based routing Or TCAM based Routing
     *  (APPLICABLE DEVICES: xCat3; Lion2)
     */
    CPSS_DXCH_CFG_ROUTING_MODE_ENT routingMode;

    /** @brief max number of LPM leafs that can be allocated in
     *  the RAM for policy based routing
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 maxNumOfPbrEntries;

    /** @brief Lpm memory mode. Half memory mode can provide full wire speed at SIP plus DIP lookup.
     *  In this mode each memory bank is divided by two. Only half of bank memory capacity is used.
     *  Each line written to first half is duplicated to second half by CPSS.
     *  Full memory mode use full capacity of each memory bank ,but can not ensure full wire speed at SIP plus DIP lookup
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2)
     *  COMMENTS:
     */
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT lpmMemoryMode;

    /** @brief select the proper shared tables mode, to meet system needs
     * (APPLICABLE DEVICES : FALCON)
     */
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT sharedTableMode;

} CPSS_DXCH_PP_CONFIG_INIT_STC;

/**
* @struct CPSS_DXCH_CFG_DEV_INFO_STC
 *
 * @brief DxCh device info structure
*/
typedef struct{

    /** generic device information */
    CPSS_GEN_CFG_DEV_INFO_STC genDevInfo;

} CPSS_DXCH_CFG_DEV_INFO_STC;

/**
* @struct CPSS_DXCH_CFG_GLOBAL_EPORT_STC
 *
 * @brief DxCh structure for Global ePort range configuration.
*/
typedef struct{

    /** @brief GT_TRUE :enable Global ePort configuration based on
     *  the <pattern> & <mask> fields.
     *  GT_FALSE: no Global ePort configured. HW values of all 0's
     *  and all 1's will be used for <mask> and < pattern>
     *  respectively.
     */
    GT_BOOL enable;

    /** @brief Global ePort range value. All bits which are not set to 1 in
     *  <mask> field must be cleared.
     *  (relevant only if enable == GT_TRUE).
     */
    GT_U32 pattern;

    /** @brief A bitmap that determines which bits in pattern are used for
     *  calculating Global ePort value.
     *  (relevant only if enable == GT_TRUE).
     */
    GT_U32 mask;

} CPSS_DXCH_CFG_GLOBAL_EPORT_STC;



/**
* @struct CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC
 *
 * @brief A Structure to hold per remote MAC port 'management' info about
 * connection between (PHY port number) remote MAC port number and it's
 * (DX port number) remote physical port number.
*/
typedef struct{

    /** @brief the (DX port number) remote physical port number on the local device.
     *  for example: 12..19 , 20..27 , 28..35
     */
    GT_U32 remotePhysicalPortNum;

    /** @brief the (PHY port number) MAC port number on the remote device.
     *  for example: 1..8  , 1..8  , 1..8 (3 instances of 88e1690 PHYs)
     */
    GT_U32 remoteMacPortNum;

} CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC;

/**
* @enum CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT
 *
 * @brief An enumeration of PHY-MAC devices , that can be bound to the DX device.
*/
typedef enum{

    /** 88e1690 PHY. */
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E

    /** must be last */
    ,CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE__LAST___E

} CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT;


/**
* @struct CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC
 *
 * @brief A Structure to hold 'management' info about connection between
 * remote device and local device.
 * this info allow the SW on top of local device to configure a remote
 * device (for example over SMI bus).
*/
typedef struct{

    /** the phy */
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT phyMacType;

    /** Management interface type (PCI/SMI/TWSI/PEX...). */
    CPSS_PP_INTERFACE_CHANNEL_ENT mngInterfaceType;

    /** @brief Management bus number.
     *  (for SMI : SMI master interface Id :DX SMI bus_number (0/1) :
     *  CPSS_PHY_SMI_INTERFACE_0_E / 1/2/3)
     */
    GT_U32 mngBusNumber;

    /** @brief address of configured device on bus (management interface)
     *  (for SMI: SMI address (on the bus) : 0..31)
     */
    GT_UINTPTR busBaseAddr;

    /** @brief index of the multi
     *  used for the interrupt line from the remote device to
     *  the local device.
     *  value of 0xFFFFFFFF meaning 'no interrupts bind'
     */
    GT_U32 interruptMultiPurposePinIndex;

    GT_U32 internalFcTxqPort;

    /** @brief number of ports in remotePortInfoArr[]
     *  remotePortInfoArr[] - array of info about the ports
     */
    GT_U32 numOfRemotePorts;

    CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC *remotePortInfoArr;

    /** whether to use MIB counters read by packet method */
    GT_BOOL useMibCounterReadByPacket;

    /** @brief CPU code to use in case of MIB counters read by packet.
     *  Recommended value in User Defined range, however
     *  any code which can be translated to HW CPU Code is allowed.
     */
    CPSS_NET_RX_CPU_CODE_ENT mibCounterReadByPacketCpuCode;

} CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC;

/**
 * @enum CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT
 *
 * @brief An enumeration of illegal sources of access into a memory of
 *      one of the shared table.
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef enum{
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E,
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E,
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E,
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E,
    /* @brief the LPM aging table is located at the 'shared memory' unit ,
       but it is not part of the 'shared tables' */
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E,

    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS___LAST___E
}CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT;

/**
 * @struct CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC
 *
 * @brief A Structure to hold the info about illegal access into a memory of
 *      one of the shared table.
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief the table that caused error.*/
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT        clientId;
    /** @brief the portGroupId that caused error.*/
    GT_U32                portGroupId;
    /** @brief the bank Index that caused error:
            LPM : this is the bank Index.
            FDB , EM : this is the MTH index.
            ARP , LPM-aging : ignored. (always 0)
    */
    GT_U32                bankIndex;
    /** @brief the line Index that caused error (in the bank) */
    GT_U32                lineIndex;
}CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC;

/**
 * @enum CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT
 *
 * @brief Egress Replication Counter types
 * (APPLICABLE DEVICES: Falcon).
*/
typedef enum{

    /** @brief QCN replication packet */
    CPSS_DXCH_CFG_QCN_REPLICATION_E,

    /** @brief Analyzer replication packet **/
    CPSS_DXCH_CFG_SNIFF_REPLICATION_E,

    /** @brief TRAP replication packet **/
    CPSS_DXCH_CFG_TRAP_REPLICATION_E,

    /** @brief Mirror replication packet **/
    CPSS_DXCH_CFG_MIRROR_REPLICATION_E,

    /** @brief HBU Outgoing forward packet **/
    CPSS_DXCH_CFG_OUTGOING_FORWARD_E,

      /** @brief QCN fifo dropped packets**/
    CPSS_DXCH_CFG_QCN_FIFO_DROP_E,

    /** @brief Analyzer fifo dropped packets**/
    CPSS_DXCH_CFG_SNIFF_FIFO_DROP_E,

    /** @brief TRAP fifo dropped packets**/
    CPSS_DXCH_CFG_TRAP_FIFO_DROP_E,

    /** @brief Mirror fifo dropped packets**/
    CPSS_DXCH_CFG_MIRROR_FIFO_DROP_E

} CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT;

/**
 *  @struct CPSS_DXCH_CFG_PROBE_PACKET_STC
 *
 * @brief Enable/Disable drop code latching at the ingress/egress
 * (APPLICABLE DEVICES: Falcon).
*/
typedef struct{
    /** Source-ID bit location for probe packet assignment
        APPLICABLE RANGE: 0-11. */
    GT_U32                bitLocation;
    /** @brief enable/disable drop code latch of probe packet */
    GT_BOOL               enable;
} CPSS_DXCH_CFG_PROBE_PACKET_STC;

/**
* @internal cpssDxChCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU , and
*         messages (AUQ) , prior to this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssDxChCfgDevRemove
(
    IN GT_U8   devNum
);

/**
* @internal cpssDxChCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDevEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCfgDevEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
GT_STATUS cpssDxChCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
);

/**
* @internal cpssDxChCfgPpLogicalInit function
* @endinternal
*
* @brief   This function Performs PP RAMs divisions to memory pools, according to
*         the supported modules in system.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] ppConfigPtr              - (pointer to)includes PP specific logical initialization
*                                      params.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_PP_MEM         - If the given configuration can't fit into the given
*                                       PP's memory capabilities, or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function should perform all PP initializations, which
*       includes:
*       -  Init the Driver's part (by calling driverPpInit()).
*       -  Ram structs init, and Ram size check.
*       -  Initialization of the interrupt mechanism data structure.
*       2. When done, the function should leave the PP in disable state, and
*       forbid any packets forwarding, and any interrupts generation.
*       3. The execution flow for preparing for the initialization of core
*       modules is described below:
*       +-----------------+
*       | Init the 3 RAMs |  This initialization includes all structs
*       | conf. structs  |  fields but the base address field.
*       |         |
*       +-----------------+
*       |
*       (Ram is big    |         +-------------------------+
*       enough to hold  +--(No)-----------| return GT_OUT_OF_PP_MEM |
*       all tables?)   |         +-------------------------+
*       |
*       (Yes)
*       |
*       V
*       +-------------------+
*       | Set the Rams base |
*       | addr. according  |
*       | to the location  |
*       | fields.      |
*       +-------------------+
*       |
*       |
*       V
*       +----------------------------------+
*       | Init the             |
*       | PRV_CPSS_DXCH_MODULE_CONFIG_STC |
*       | struct, according        |
*       | to the info in RAM        |
*       | conf. struct.          |
*       +----------------------------------+
*       |
*       |
*       |
*       V
*       +---------------------------------+
*       | set the prepared        |
*       | module Cfg struct        |
*       | in the appropriate       |
*       | PRV_CPSS_DXCH_PP_CONFIG_STC   |
*       | struct.             |
*       +---------------------------------+
*
*/
GT_STATUS cpssDxChCfgPpLogicalInit

(
    IN      GT_U8                           devNum,
    IN      CPSS_DXCH_PP_CONFIG_INIT_STC    *ppConfigPtr
);

/**
* @internal cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
);

/**
* @internal cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet function
* @endinternal
*
* @brief   Gets device ID modification for Routed packets.
*         See Set Api comments.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
*
* @param[out] modifyEnablePtr          - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *modifyEnablePtr
);

/**
* @internal cpssDxChCfgDsaTagSrcDevPortModifySet function
* @endinternal
*
* @brief   Enables/Disables Device ID Modification upon packet sending to another
*         stack unit.
*         When Connecting DxCh Devices to SOHO in a Value Blade model to enable
*         DxCh grade Capabilities for FE Ports, in a staking system, we must be
*         able to overcome the 32 devices limitation.
*         To do that, SOHO Device Numbers are not unique and packets received
*         from the SOHO by the DxCh and are relayed to other stack units
*         have their Device ID changed to the DxCh Device ID.
*         On Upstream (packet from SOHO to DxCh):
*         The SOHO Sends a packet to the DxCh and the packet is sent back
*         to the SOHO. In this case we don't want to change the Device ID in
*         the DSA Tag so that the SOHO is able to filter the Src Port and is
*         able to send back the packet to its source when doing cross chip
*         flow control.
*         On Downsteam (packet from SOHO to SOHO):
*         The SOHO receives a packet from another SOHO in this case we
*         want to change the Device ID so that the packet is not filtered.
*         On top of the above, for forwarding packets between the DxCh devices
*         and for Auto Learning, the Port Number must also be changed.
*         In addition Changing the Device ID is needed only for FORWARD DSA Tag.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] modifedDsaSrcDev         - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortModifySet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     modifedDsaSrcDev
);

/**
* @internal cpssDxChCfgDsaTagSrcDevPortModifyGet function
* @endinternal
*
* @brief   Get status of Device ID Modification upon packet sending to another
*         stack unit. See Set Api comments.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modifedDsaSrcDevPtr      - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortModifyGet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     *modifedDsaSrcDevPtr
);

/**
* @internal cpssDxChCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*         when several type of entries like ARP and tunnelStart resize in the same
*         table (memory) , the function returns the number of entries for the least
*         denominator type of entry --> in this case number of ARP entries.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgTableNumEntriesGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_CFG_TABLES_ENT    table,
    OUT GT_U32                      *numEntriesPtr
);

/**
* @internal cpssDxChTableValidIndexGetNext function
* @endinternal
*
* @brief   the function return the index of next valid table entry
*         starting a given index (inclusive)
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; BobK; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
* @param[in,out] entryIndexPtr            - pointer to search start index
* @param[in,out] entryIndexPtr            - pointer to valid index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE               - no more valid indices
*/
GT_STATUS cpssDxChTableValidIndexGetNext
(
    IN      GT_U8                       devNum,
    IN      CPSS_DXCH_CFG_TABLES_ENT    table,
    INOUT   GT_U32                      *entryIndexPtr
);

/**
* @internal cpssDxChCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. it is the responsibility of application to update the 'devNum' of
*         HW entries from 'oldDevNum' to 'newDevNum' using appropriate API's
*         such as 'Trunk members','Fdb entries','NH entries','PCE entries'...
*         2. it's application responsibility to update the HW device number !
*         see API cpssDxChCfgHwDevNumSet
*         3. no HW operations involved in this API
*         NOTE:
*         this function MUST be called under 'Interrupts are locked' and under
*         'Tasks lock'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum value > 0x1f (0..31)
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssDxChCfgReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
);

/**
* @internal cpssDxChCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_CFG_DEV_INFO_STC   *devInfoPtr
);


/**
* @internal cpssDxChCfgBindPortPhymacObject function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChCfgBindPortPhymacObject(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
);

/**
* @internal cpssDxChCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mode                     - Ingress Drop Counter Mode.
* @param[in] portNum                  - port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[in] vlan                     - VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_OUT_OF_RANGE          - on wrong portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
);

/**
* @internal cpssDxChCfgIngressDropCntrModeGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modePtr                  - pointer to Ingress Drop Counter Mode.
* @param[out] portNumPtr               - pointer to port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[out] vlanPtr                  - pointer to VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - illegal state of configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrModeGet
(
    IN   GT_U8                                       devNum,
    OUT  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT *modePtr,
    OUT  GT_PORT_NUM                                 *portNumPtr,
    OUT  GT_U16                                      *vlanPtr
);


/**
* @internal cpssDxChCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] counter                  - Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      counter
);


/**
* @internal cpssDxChCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
);

/**
* @internal cpssDxChCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ePort                    -  number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number.
*       2. Can be overridden by TTI Action/RBridge.
*       3. The 'default eport table' will be accessed at (source ePort)%512.
*          (APPLICABLE DEVICES: FALCON. %1K)
*       But the rest of eport tables are accessed by source ePort.
*
*/
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
);

/**
* @internal cpssDxChCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ePortPtr                 - pointer to ePort number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Can be overridden by TTI Action/RBridge
*
*/
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
);

/**
* @internal cpssDxChCfgEgressHashSelectionModeSet function
* @endinternal
*
* @brief   Set how egress pipe get 6 bits hash from the ingress 12 bits hash. (EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgEgressHashSelectionModeSet
(
    IN GT_U8                                                        devNum,
    IN CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT    selectionMode
);

/**
* @internal cpssDxChCfgEgressHashSelectionModeGet function
* @endinternal
*
* @brief   Return how egress pipe get 6 bits hash from the ingress 12 bits hash. (EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to)hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgEgressHashSelectionModeGet
(
    IN  GT_U8                                                       devNum,
    OUT CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT   *selectionModePtr
);

/**
* @internal cpssDxChCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgGlobalEportSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);

/**
* @internal cpssDxChCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgGlobalEportGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);

/**
* @internal cpssDxChCfgLearnOnRingPortEnableSet function
* @endinternal
*
* @brief   function enable/disable learning on 'Ring port'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: Learning is disabled on ring ports.
*                                      In addition NA messages are not send to the CPU
*                                      GT_TRUE: Learning is enabled on ring ports
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgLearnOnRingPortEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChCfgLearnOnRingPortEnableGet function
* @endinternal
*
* @brief   function gets whether learning is enabled/disabled on 'Ring port'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Learning is disabled on ring ports.
*                                      In addition NA messages are not send to the CPU
*                                      GT_TRUE: Learning is enabled on ring ports
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgLearnOnRingPortEnableGet
(
    IN  GT_U8                   devNum,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry read callback function for Hw access.
 *
 * INPUTS:
 *       devNum               - the device number
 *       convertedPortGroupId - the port group Id after conversion if needed, to support
 *                              multi-port-group devices that need to access specific port group
 *       tableType            - the specific table name
 *       convertedEntryIndex  - index in the table after conversion if needed.
 *       inPortGroupId        - original incoming port group.
 *       stage                - stage for callback run.
 *       status               - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       entryValuePtr - (pointer to) the data read from the table
 *                       may be NULL in the case of indirect table.
 *
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  convertedPortGroupId,
    IN  CPSS_DXCH_TABLE_ENT tableType,
    IN  GT_U32                  convertedEntryIndex,
    OUT GT_U32                 *entryValuePtr,
    IN  GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry read callback function for Hw access.
 *
 * INPUTS:
 *       devNum               - the device number
 *       convertedPortGroupId - the port group Id after conversion if needed, to support
 *                              multi-port-group devices that need to access specific port group
 *       tableType            - the specific table name
 *       convertedEntryIndex  - index in the table after conversion if needed.
 *       entryValuePtr        - (pointer to) the data that will be written to the table
 *                               may be NULL in the case of indirect table.
 *       entryMaskPtr         - (pointer to) the mask for entryValuePtr. Only bits raised
 *                              in the entryMaskPtr will be written from entryValuePtr
 *                              into a table.
 *                              If NULL - entryValuePtr will be written as is.
 *       inPortGroupId        - original incoming port group.
 *       stage                - stage for callback run.
 *       status               - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC)
(
    IN GT_U8                         devNum,
    IN GT_U32                        convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT           tableType,
    IN GT_U32                        convertedEntryIndex,
    IN GT_U32                       *entryValuePtr,
    IN GT_U32                       *entryMaskPtr,
    IN GT_U32                        inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT  stage,
    IN  GT_STATUS                    status
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry field read callback function for Hw access.
 *
 * INPUTS:
 *       devNum                - the device number
 *       convertedPortGroupId  - the port group Id after conversion if needed, to support
 *                               multi-port-group devices that need to access specific port group
 *       tableType             - the specific table name
 *       convertedEntryIndex   - index in the table after conversion if needed.
 *       fieldWordNum          - field word number
 *                               use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
 *                               if need global offset in the field of fieldOffset
 *       fieldOffset           - field offset
 *       fieldLength           - field length
 *       inPortGroupId         - original incoming port group.
 *       stage                 - stage for callback run.
 *       status                - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       entryValuePtr - (pointer to) the data read from the table
 *                       may be NULL in the case of indirect table.
 *
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC)
(
    IN GT_U8                   devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                  fieldWordNum,
    IN GT_U32                  fieldOffset,
    IN GT_U32                  fieldLength,
    OUT GT_U32                *fieldValuePtr,
    IN GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry field write callback function for Hw access.
 *
 * INPUTS:
 *       devNum                - the device number
 *       convertedPortGroupId  - the port group Id after conversion if needed, to support
 *                               multi-port-group devices that need to access specific port group
 *       tableType             - the specific table name
 *       convertedEntryIndex   - index in the table after conversion if needed.
 *       fieldWordNum          - field word number
 *                               use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
 *                               if need global offset in the field of fieldOffset
 *       fieldOffset           - field offset
 *       fieldLength           - field length
 *       entryValuePtr         - (pointer to) the data that will be written to the table
 *                                may be NULL in the case of indirect table.
 *       inPortGroupId         - original incoming port group.
 *       stage                 - stage for callback run.
 *       status                - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC)
(
    IN GT_U8                   devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue,
    IN GT_U32                 inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN GT_STATUS                       status
);


/**
* @struct CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC
 *
 * @brief the structure holds the tables access callback routines.
*/
typedef struct{

    /** table entry read callback function */
    CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC hwAccessTableEntryReadFunc;

    /** table entry write callback function */
    CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC hwAccessTableEntryWriteFunc;

    /** table entry field read callback function */
    CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC hwAccessTableEntryFieldReadFunc;

    /** @brief table entry field write callback function
     *  COMMENTS:
     *  ----------------------------------------------------------------------------------------
     *  CPSS Function                Hw access Callback
     *  ------------------------------------------------------------------------------------------
     *  1  prvCpssDxChPortGroupReadTableEntry       hwAccessTableEntryReadFunc;
     *  2  prvCpssDxChPortGroupWriteTableEntry       hwAccessTableEntryWriteFunc;
     *  3  prvCpssDxChPortGroupReadTableEntryField    hwAccessTableEntryFieldReadFunc;
     *  4  prvCpssDxChPortGroupWriteTableEntryField    hwAccessTableEntryFieldWriteFunc
     *  ---------------------------------------------------------------------------------------------
     */
    CPSS_DXCH_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC hwAccessTableEntryFieldWriteFunc;

} CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC;

/**
* @internal cpssDxChCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChCfgHwAccessObjectBind
(
    IN CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
);

/**
* @internal cpssDxChCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write methood.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write methood is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write methood is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgHitlessWriteMethodEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChCfgPortRemotePhyMacBind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC device (like: PHY '88E1690') to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgPortRemotePhyMacBind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
);

/**
* @internal cpssDxChCfgRemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChCfgRemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
);


/**
* @internal cpssDxChCfgSharedTablesIllegalClientAccessInfoGet function
* @endinternal
*
* @brief   a debug API to check and get errors caused in the shared memory engine ,
*          by wrong access to it from one or more of it's clients : LPM , LPM aging ,
*          FDB , EM , ARP.
*          NOTEs:
*           if the function     found 'illegal access' it will return GT_OK and the info will be in (*clientInfoPtr)
*           if the function not found 'illegal access' it will return GT_NO_MORE.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum               - device number
* @param[out] clientInfoPtr        - (pointer to) the client info to retrieve.
*
* @retval GT_OK                    - on success , and there is 'illegal' info to retrieve.
* @retval GT_NO_MORE               - on success , but there are no more 'illegal' info to retrieve.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgSharedTablesIllegalClientAccessInfoGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC        *clientInfoPtr
);

/**
* @internal cpssDxChCfgReplicationCountersGet function
* @endinternal
*
* @brief   Get the value of replication engine counters.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] counterType              - type of replication counter
* @param[out] counterPtr              - (pointer to) value of counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS cpssDxChCfgReplicationCountersGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT counterType,
    OUT GT_U32                                        *counterPtr
);

/**
* @internal cpssDxChCfgProbePacketConfigSet function
*
* @brief Set the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] probeCfgPtr           - (pointer to)Probe packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketConfigSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_CFG_PROBE_PACKET_STC *probeCfgPtr
);

/**
* @internal cpssDxChCfgProbePacketConfigGet function
*
* @brief Get the Probe packet Configuration
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3;
*                                Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number.
* @param[out] probeCfgPtr          - (pointer to) Probe packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketConfigGet
(
    IN GT_U8                           devNum,
    OUT CPSS_DXCH_CFG_PROBE_PACKET_STC *probeCfgPtr
);

/**
*
* @internal cpssDxChCfgProbePacketDropCodeGet function
*
* @brief Get the last probe packet Drop Code
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3;
*                                Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number
* @param[in] direction             - ingress/egress stage
* @param[out] dropCodePtr          - (pointer to) last probe packet drop code
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketDropCodeGet
(
    IN GT_U8                     devNum,
    IN CPSS_DIRECTION_ENT        direction,
    OUT CPSS_NET_RX_CPU_CODE_ENT *dropCodePtr
);

/**
* @internal cpssDxChCfgDevDbInfoGet function
* @endinternal
*
* @brief   function to get the init parameters given during initialization.
*         it is needed for application that need to do re-init without HW reset ,
*         and want to use same init parameters, that relate to DMAs .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_SUPPORTED         - device not hold DMA info
* @retval GT_NOT_INITIALIZED       - device was not yet initialized with DMA info
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS cpssDxChCfgDevDbInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC        **prevCpssInit_netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_auqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_fuqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC  **prevCpssInit_multiNetIfCfgPtrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCfgInith */

