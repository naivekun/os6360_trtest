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
* @file prvCpssDxChHwInit.h
*
* @brief Includes Core level basic Hw initialization functions, and data
* structures.
*
* @version   41
********************************************************************************
*/
#ifndef __prvCpssDxChHwInith
#define __prvCpssDxChHwInith

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_CPSS_DXCH_PROFILES_NUM_CNS       4
#define PRV_CPSS_DXCH2_PROFILES_NUM_CNS      8

#define PRV_CPSS_XCAT_NETWORK_PORTS_SERDES_NUM_CNS  6
#define PRV_CPSS_XCAT_SERDES_NUM_CNS        22
#define PRV_CPSS_XCAT3_SERDES_NUM_CNS       12
#define PRV_CPSS_LION_SERDES_NUM_CNS        24
#define PRV_CPSS_LION2_SERDES_NUM_CNS       24

#define PRV_CPSS_XCAT2_SERDES_NUM_CNS       10

#define PRV_CPSS_BOBCAT2_SERDES_NUM_CNS     36
#define PRV_CPSS_ALDRIN_SERDES_NUM_CNS      33
#define PRV_CPSS_BOBCAT3_SERDES_NUM_CNS     74  /* 6 DP x 12 serdeses + 2 CPU */
#define PRV_CPSS_ALDRIN2_SERDES_NUM_CNS  73  /* DP[0]->12+1 CPU , DP[1]->12 , DP[2,3]-->24 each : total 73 */
/* Falcon as 3.2 Tera device */
#define PRV_CPSS_FALCON_3_2_SERDES_NUM_CNS     65  /* 8 DP x 8 serdeses + 1 CPU */
/* Falcon as 6.4 Tera device */
#define PRV_CPSS_FALCON_6_4_SERDES_NUM_CNS     130 /*16 DP x 8 serdeses + 2 CPU */
/* Falcon as 12.8 Tera device */
#define PRV_CPSS_FALCON_12_8_SERDES_NUM_CNS    258 /*32 DP x 8 serdeses + 2 CPU */

/* Target local port to MAC mapping for CPU port */
#define PRV_CPSS_DXCH_TARGET_LOCAL_LINK_STATUS_CPU_PORT_NUM_CNS 255

/**
* @enum PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT
 *
 * @brief An enum for fields of the EGF_SHT table/register that represent the
 * physical port and/or the eport.
 * Notes:
*/
typedef enum{
     PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_STP_STATE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EGRESS_VLAN_FILTER_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_UC_SRC_ID_FILTER_E          /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_MC_LOCAL_ENABLE_E           /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IGNORE_PHY_SRC_MC_FILTER_E  /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_STP_STATE_MODE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ODD_ONLY_SRC_ID_FILTER_ENABLE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ROUTED_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IPMC_ROUTED_FILTER_ENABLE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_FROM_CPU_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_BRIDGED_FORWARD_RESTRICTED_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_PORT_ISOLATION_MODE_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EGRESS_EPORT_EVLAN_FILTER_ENABLE_E /* only per EPort */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_MESH_ID_E /* only per EPort */
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_EPORT_ASSOCIATED_VID1_E
    ,PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_DROP_ON_EPORT_VID1_MISMATCH_E
}PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT;

/**
* @enum PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT
 *
 * @brief An enum for fields of the EGF_EFT register that represent the
 * physical port .
 * Notes:
*/
typedef enum{
    /* MC_FIFO */
     PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_MC_FIFO_PORT_TO_MC_FIFO_ATTRIBUTION_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_MC_FIFO_PORT_TO_HEMISPHERE_MAP_E                     /* only per physical port */

    /* EGR_FILTER */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_FC_TRIGGER_BY_CN_FRAME_ON_PORT_ENABLE_E   /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_CN_FRAME_TX_ON_PORT_ENABLE_E              /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNREGED_BC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNKNOWN_UC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_UNREGED_MC_FILTER_ENABLE_E                /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_OAM_LOOPBACK_FILTER_ENABLE_E              /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_VIRTUAL_2_PHYSICAL_PORT_REMAP_E           /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E          /* only per physical port */

    /* DEV_MAP_TABLE */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_ADDR_CONSTRUCT_PROFILE_E               /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_SRC_PORT_MAP_OWN_DEV_ENABLE_E    /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_TRG_PORT_MAP_OWN_DEV_ENABLE_E    /* only per physical port */
    ,PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_PORT_ADDR_CONSTRUCT_MODE_ADDRESS_PROFILE_E /* only per physical port */

}PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT;

/**
* @enum PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT
 *
 * @brief An enum for types of entries that occupy the TS/ARP/NAT memory
 * values -
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E : ARP (48 bits)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E : Tunnel start (192 bits)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E : Tunnel start - double length (384 bits)
 * (for IPv6 TS)
 * PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_NAT_E : NAT (Network Address Translation. 192 bits)
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 * Notes:
*/
typedef enum{

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ARP_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_REGULAR_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_TUNNEL_START_DOUBLE_E,

    PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_NAT_E

} PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT;


/* For init system with HW access and register defaults */
extern GT_BOOL dxChInitRegDefaults;

/* For init system with no HW write to the device Device_ID within a Prestera chipset.*/
extern GT_BOOL dxChFastBootSkipOwnDeviceInit;


/**
* @internal prvCpssDxChHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah devices.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrInit
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah2 devices.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh2HwRegAddrInit
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh3HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for cheetah-3 devices.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh3HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChXcatHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh xCat devices.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcatHwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat3HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh xCat3 devices.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChLionHwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Lion devices.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLionHwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChLion2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Lion2 devices.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLion2HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChBobcat2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh Bobcat2 devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBobcat2HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat2HwRegAddrInit function
* @endinternal
*
* @brief   This function initializes the registers struct for DxCh xCat2 devices.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat2HwRegAddrInit
(
    IN GT_U32 devNum
);

/**
* @internal prvCpssDxChHwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh2HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxCh2HwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxCh3HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxCh3HwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxCh3HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChXcatHwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChXcatHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcatHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat3HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChXcat3HwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChXcat2HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChXcat2HwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChLionHwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChLionHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLionHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChLion2HwRegAddrRemove function
* @endinternal
*
* @brief   This function release the memory that was allocated by the function
*         prvCpssDxChLionHwRegAddrInit(...)
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLion2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/**
* @internal prvCpssDxChHwRegAddrPortMacUpdate function
* @endinternal
*
* @brief   This function updates mac registers addresses for given port accordingly
*         to currently used MAC Unit
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - new interface mode used with this MAC
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrPortMacUpdate
(
    IN GT_U32 devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);

/**
* @internal prvCpssDxCh3HwPpPllUnlockWorkaround function
* @endinternal
*
* @brief   Workaround to eliminate the PLL unlocking issue.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] networkSerdesPowerUpBmp  - bitmap of network SERDES to be power UP
* @param[in] xgDevice                 - GT_TRUE for XG device, GT_FALSE for GE device
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
*
* @note PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC usage has actual influence
*       only for XG devices, where it is used to get the port number from
*       the power up SERDES bitmap (networkSerdesPowerUpBmp).
*       For GE devices the macro does nothing usefull since previouse one
*       (PRV_CPSS_DXCH_SKIP_NOT_EXIST_SERDES_MAC) make all necessary checks.
*
*/
GT_STATUS prvCpssDxCh3HwPpPllUnlockWorkaround
(
    IN GT_U8    devNum,
    IN GT_U32   networkSerdesPowerUpBmp,
    IN GT_BOOL  xgDevice
);


/**
* @internal prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert function
* @endinternal
*
* @brief   convert from 'drop mode' to : packet command and to HW value
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] dropMode                 - drop mode
*
* @param[out] packetCommandPtr         - (pointer to) packet command (can be NULL --> ignored)
* @param[out] fieldValuePtr            - (pointer to) HW field value (can be NULL --> ignored)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert
(
    IN GT_U8        devNum,
    IN CPSS_DROP_MODE_TYPE_ENT dropMode,
    OUT CPSS_PACKET_CMD_ENT *packetCommandPtr,
    OUT GT_U32              *fieldValuePtr
);

/**
* @internal prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert function
* @endinternal
*
* @brief   convert from HW value : to 'drop mode' and to packet command
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] fieldValue               - HW field value
*
* @param[out] packetCommandPtr         - (pointer to) packet command (can be NULL --> ignored)
* @param[out] dropModePtr              - (pointer to) drop mode (can be NULL --> ignored)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert
(
    IN GT_U8        devNum,
    IN GT_U32       fieldValue,
    OUT CPSS_PACKET_CMD_ENT     *packetCommandPtr,
    OUT CPSS_DROP_MODE_TYPE_ENT *dropModePtr
);


/**
* @internal prvCpssDxChHwPpPortToGopConvert function
* @endinternal
*
* @brief   Private (internal) function converts Port number to Group Of Ports number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] phyPortNum               - physical port number
*
* @param[out] portGroupPtr             - (pointer to) Port Group Number
*                                      for not multi port group devices
*                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[out] gopNumPtr                - (pointer to) Group of Ports Number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
*/
GT_STATUS prvCpssDxChHwPpPortToGopConvert
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         phyPortNum,
    OUT GT_U32                       *portGroupPtr,
    OUT GT_U32                       *gopNumPtr
);

/**
* @internal prvCpssDxChHwEgfShtFieldSet function
* @endinternal
*
* @brief   Write a field to the EGF_SHT table/register that represent the
*         physical port and/or the eport.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port and/or the eport)
* @param[in] fieldType                - the field type
* @param[in] accessPhysicalPort       - indication if to access to the physical ports registers
*                                      relevant only for the portNum < 256
* @param[in] accessEPort              - indication if to access to the EPort table
* @param[in] value                    - the data write to the register and/or table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfShtFieldSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT  fieldType,
    IN GT_BOOL                  accessPhysicalPort,
    IN GT_BOOL                  accessEPort,
    IN GT_U32                   value
);

/**
* @internal prvCpssDxChHwEgfShtFieldGet function
* @endinternal
*
* @brief   Read a field to the EGF_SHT table/register that represent the
*         physical port or the eport.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port or the eport)
* @param[in] fieldType                - the field type
* @param[in] accessPhysicalPort       - GT_TRUE : indication if to access to the physical ports registers
*                                      relevant only for the portNum < 256
*                                      GT_FALSE : access to the ePort table
*
* @param[out] valuePtr                 - (pointer to)the data read from the register or table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfShtFieldGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT  fieldType,
    IN GT_BOOL                  accessPhysicalPort,
    OUT GT_U32                   *valuePtr
);


/**
* @internal prvCpssDxChHwEgfEftFieldSet function
* @endinternal
*
* @brief   Write a field to the EGF_EFT register that represent the
*         physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port)
* @param[in] fieldType                - the field type
* @param[in] value                    - the data write to the register
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfEftFieldSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT  fieldType,
    IN GT_U32                   value
);

/**
* @internal prvCpssDxChHwEgfEftFieldGet function
* @endinternal
*
* @brief   Read a field from the EGF_EFT register that represent the
*         physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (physical port)
* @param[in] fieldType                - the field type
*
* @param[out] valuePtr                 - (pointer to)the data read from the register
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssDxChHwEgfEftFieldGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_ENT  fieldType,
    OUT GT_U32                   *valuePtr
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntryWrite function
* @endinternal
*
* @brief   Write a whole entry to the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] portsMembersPtr          - (pointer to) the bmp of ports that will be written to
*                                      the table.
* @param[in] isCpuMember              - indication if to use 'isCpuMember'
* @param[in] isCpuMember              - indication to add the CPU port to the BMP of ports
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntryWrite
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr,
    IN GT_BOOL              isCpuMember_supported,
    IN GT_BOOL              isCpuMember
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntryRead function
* @endinternal
*
* @brief   Read a whole entry of the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] isCpuMember_supported    - indication if to use 'isCpuMemberPtr'
*
* @param[out] portsMembersPtr          - (pointer to) the bmp of ports that will be written to
*                                      the table.
* @param[out] isCpuMemberPtr           - (pointer to) indication to add the CPU port to the BMP of ports
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntryRead
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr,
    IN GT_BOOL              isCpuMember_supported,
    OUT GT_BOOL              *isCpuMemberPtr
);

/**
* @internal prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort function
* @endinternal
*
* @brief   set (enable/disable) single port (bit index) in entry to the table. - for 'bmp of ports' - EGF_SHT tables
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*                                      entryIndex   - index in the table
* @param[in] portNum                  - the port number
* @param[in] enable                   - enable/disable the port in the entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort
(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32               tableIndex,
    IN GT_U32               portNum,
    IN GT_BOOL              enable
);

/**
* @internal prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite function
* @endinternal
*
* @brief   Write a whole entry to the table. HA memory of 'TunnelStart/ARP/NAT(for NAT capable devices)'
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] entryType                - entry type (arp/TS/TS_double)
* @param[in] tableIndex               - index in the table.
*                                      the ratio is 4 ARP per 1 TS ('regular')
*                                      so for 1K TS the indexes are:
*                                      for ARP : (APPLICABLE RANGES: 0,1,2,3,4..(4K-1))
*                                      for TS  : (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      for TS_double : (APPLICABLE RANGES: 0,2,4,6,8..(1K-2))
*                                      for NAT: (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      NOTE: the 'double' TS must use only 'even' indexes (0,2,4,6,...)
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHaTunnelStartArpNatTableEntryWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       portGroupId,
    IN PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT   entryType,
    IN GT_U32                                       tableIndex,
    IN GT_U32                                       *entryValuePtr
);

/**
* @internal prvCpssDxChHwHaTunnelStartArpNatTableEntryRead function
* @endinternal
*
* @brief   Read a whole entry from the table. HA memory of 'TunnelStart/ARP/NAT(for NAT capable devices)'
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] entryType                - entry type (arp/TS/TS_double)
* @param[in] tableIndex               - index in the table.
*                                      the ratio is 4 ARP per 1 TS ('regular')
*                                      so for 1K TS the indexes are:
*                                      for ARP : (APPLICABLE RANGES: 0,1,2,3,4..(4K-1))
*                                      for TS  : (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      for TS_double : (APPLICABLE RANGES: 0,2,4,6,8..(1K-2))
*                                      for NAT: (APPLICABLE RANGES: 0,1,2,3,4..(1K-1))
*                                      NOTE: the 'double' TS must use only 'even' indexes (0,2,4,6,...)
*
* @param[out] entryValuePtr            - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwHaTunnelStartArpNatTableEntryRead
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       portGroupId,
    IN PRV_CPSS_DXCH_HA_TS_ARP_NAT_ENTRY_TYPE_ENT   entryType,
    IN GT_U32                                       tableIndex,
    OUT GT_U32                                      *entryValuePtr
);

/**
* @internal prvCpssDxChXcat3PortMacRegAddrSwap function
* @endinternal
*
* @brief   MAC address DB update, in order to support extended MAC feature.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*                                      portsBmp  - physical port number
* @param[in] enable                   - extended MAC mode enabled
*                                      GT_TRUE - update registers to work in extended mode
*                                      GT_FALSE - update registers to work in regular mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3PortMacRegAddrSwap
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);


/**
* @internal prvCpssDxChHwXcat3RegAddrPortMacUpdate function
* @endinternal
*
* @brief   This function updates mac registers addresses for given port accordingly
*         to currently used MAC Unit.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwXcat3RegAddrPortMacUpdate
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet function
* @endinternal
*
* @brief   This function returns supported bandwidth by dev family/reviosion/corecolock
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBWCapacityInGbpsPtr  - pointer to bw coresponding to dev family/reviosion/corecolock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on error
* @retval GT_NOT_SUPPORTED         - on device family/revison/coreclock not found
*/
GT_U32 hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *pipeBWCapacityInGbpsPtr
);



/**
* @internal prvCpssDxChPortLedInterfaceGet function
* @endinternal
*
* @brief   Get LED interface number by port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*                                       RETURNS:
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - not supported device
*/
GT_STATUS prvCpssDxChPortLedInterfaceGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32              *interfaceNumberPtr
);


/**
* @internal prvCpssLedStreamNumOfInterfacesInPortGroupGet function
* @endinternal
*
* @brief   Get the number of LED interfaces that the device's core supports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                       Number of LED interfaces that the device's core supports
*
* @note Multi-core device will return information per core !!!
*
*/
GT_U32   prvCpssLedStreamNumOfInterfacesInPortGroupGet
(
    IN GT_U8 devNum
);


#ifdef ASIC_SIMULATION
GT_VOID simCoreClockOverwrite
(
    GT_U32 simUserDefinedCoreClockMHz
);

#endif

/**
* @internal prvCpssDxChHwRegAddrDbRemove function
* @endinternal
*
* @brief   remove the DB of 'register address'
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChHwRegAddrDbRemove
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChHwInitNumOfSerdesGet function
* @endinternal
*
* @brief   Get number of SERDES lanes in device.
*         Function returns number of SERDES in port group for Lion and Lion2
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
*
* @retval 0                        - for not applicable device
*                                       Number of number of SERDES lanes in device or port group
*/
GT_U32   prvCpssDxChHwInitNumOfSerdesGet
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDxChHwPortGroupBetweenUnitsConvert function
* @endinternal
*
* @brief   Convert port group of 'unit A' to port group in 'unit B'
*         NOTE:
*         1. some units hold 'relations' between themselves , like:
*         a. MT to MG on FDB upload or on NA to CPU.
*         b. CNC to MG on CNC upload
*         2. such function needed for Bobcat3 where single MG unit serves the CNC
*         units of both 2 port groups. (MT is also single anyway)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] unitA                    - the type of unit A
* @param[in] portGroupInUnitA         - the port group in unit A that needs conversion.
* @param[in] unitB                    - the type of unit B
*
* @param[out] portGroupInUnitBPtr      - (pointer to) the port group of unit B that
*                                      relates to port group in unit A.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssDxChHwPortGroupBetweenUnitsConvert
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitA,
    IN GT_U32       portGroupInUnitA,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitB,
    OUT GT_U32      *portGroupInUnitBPtr
);

/**
* @internal prvCpssDxChHwPpPhase1ShadowInit function
* @endinternal
*
* @brief   This function performs basic cpss shadow configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwPpPhase1ShadowInit
(
    IN CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *ppPhase1ParamsPtr
);

/**
* @internal prvCpssDxChDevLedInterfacePortCheck function
* @endinternal
*
* @brief   Function checks device LED port for out of range.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] ledInterfaceNum          - led stream (interface)
* @param[in] ledPort                  - led port
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - on not initialized led port
*/
GT_STATUS prvCpssDxChDevLedInterfacePortCheck
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          ledPort
);

/**
* @internal prvCpssLionDuplicatedMultiPortGroupsGet function
* @endinternal
*
* @brief   Check if register address is duplicated in multi-port groups device
*         for Lion B device
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssLionDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssLion2DuplicatedMultiPortGroupsGet function
* @endinternal
*
* @brief   Check if register address is duplicated in multi-port groups device
*         for Lion2 device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssLion2DuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobk device.
*
* @note   APPLICABLE DEVICES:      BobK.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobcat3 device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);


/**
* @internal prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Aldrin2 device.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in aldrin device.
*
* @note   APPLICABLE DEVICES:      Aldrin.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; ExMxPm.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

/**
* @internal prvCpssDxChCgMacUnitDisable function
* @endinternal
*
* @brief   CG MAC unit enable/disable for power reduction purposes.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] disable                  - CG MAC unit disable
*                                      GT_TRUE - CG Unit disabled
*                                      GT_FALSE - CG Unit enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgMacUnitDisable
(
    IN  GT_U8 devNum,
    IN  GT_BOOL disable
);

/**
* @internal prvCpssDxChPhaClockEnable function
* @endinternal
*
* @brief   enable/disable the clock to PPA in the PHA unit in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum   - device number.
* @param[in] enable   - Enable/Disable the clock to PPA in the PHA unit in the device.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhaClockEnable
(
    IN  GT_U8   devNum,
    IN GT_BOOL  enable
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #define __prvCpssDxChHwInith  */

