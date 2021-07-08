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
* @file prvCpssDrvPpCfgInit.h
*
* @brief Includes initialization functions for the driver level of the CPSS Sw.
*
* @version   16
********************************************************************************
*/
#ifndef __prvCpssDrvPpCfgInith
#define __prvCpssDrvPpCfgInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/generic/cpssHwInfo.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpssDriver/pp/prvCpssDrvErrataMng.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>


/**
* @enum PRV_CPSS_DRV_DEV_ID_LOCATION_ENT
 *
 * @brief the enumeration indicate the location of the DEV_ID for the
 * device family (the specific device is unknown at this stage)
*/
typedef enum{

    /** @brief take info from
     *  address 0x00000000 on the PCI config cycle space
     *  (relevant only to PCI devices (not PEX))
     */
    PRV_CPSS_DRV_DEV_ID_LOCATION_PCI_CONFIG_CYCLE_ADDR_0x00000000_E,

    /** @brief take info from
     *  address 0x00070000 on the PEX config header space
     *  (relevant only to PCI devices (not PCI))
     */
    PRV_CPSS_DRV_DEV_ID_LOCATION_PEX_CONFIGURATION_HEADER_ADDR_0X00070000_E,

    /** @brief take DEV_ID from
     *  register 0x0000004c in the device address space (PCI/PEX/SMI...)
     *  NOTE: the vendorId in register 0x00000050 (0x11AB)
     */
    PRV_CPSS_DRV_DEV_ID_LOCATION_DEVICE_ADDR_0x0000004C_E

} PRV_CPSS_DRV_DEV_ID_LOCATION_ENT;

/**
* @struct PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC
 *
 * @brief hold info about the device received by the cpssDriver, when
 * calling prvCpssDrvPpHwPhase1Init(...)
*/
typedef struct{

    /** @brief the management interface SMI/PCI/TWSI/PEX
     *  hwInfo[]      - HW info (bus type, bus address, mappings, interrupts, etc)
     */
    CPSS_PP_INTERFACE_CHANNEL_ENT mngInterfaceType;

    CPSS_HW_INFO_STC hwInfo[CPSS_MAX_PORT_GROUPS_CNS];

    /** PP's High availability mode : Active / Standby. */
    CPSS_SYS_HA_MODE_ENT ppHAState;

    /** the location of the DEV_ID for those device family */
    PRV_CPSS_DRV_DEV_ID_LOCATION_ENT devIdLocationType;

    /** @brief Number of port groups.
     *  For 'non multi-port-groups' device --> use 1
     *  For Lion (regardless to active port-groups) should be 4
     */
    GT_U32 numOfPortGroups;

    /** @brief Bitmap of configurable address completion regions
     *  used by interrupt hanling routine.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 isrAddrCompletionRegionsBmp;

    /** @brief Bitmap of configurable address completion regions
     *  used by CPSS API.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X PPs that have 8 address completion regions.
     *  Region 0 provide access to the lower addresses that contain also
     *  configuration of address completion regions. It must never be configured.
     *  Lists (coded as ranges) of other regions can be assigned for several CPUs
     *  for interrupt handling (for such CPU that processes the PP interrupts)
     *  and for all other CPSS API using.
     *  These assigned ranges of regions must not have common members.
     *  These assigned ranges of regions must not contain zero range.
     */
    GT_U32 appAddrCompletionRegionsBmp;

} PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC;

/**
* @struct PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC
 *
 * @brief hold info about the device returned by the cpssDriver, when
 * calling prvCpssDrvPpHwPhase1Init(...)
*/
typedef struct{

    /** device type of the PP. */
    CPSS_PP_DEVICE_TYPE devType;

    /** The device's revision number. */
    GT_U8 revision;

    /** CPSS's device family that current device belongs to. */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /** CPSS's device sub */
    CPSS_PP_SUB_FAMILY_TYPE_ENT devSubFamily;

    /** @brief number of ports in this device
     *  For 'multi-port-groups' device , the number of 'global ports'.
     *  For example : Lion --> 60 ports (0..59)
     */
    GT_U32 numOfPorts;

    /** @brief number of tiles.
      * Falcon : 1/2/4
      * Other devices : 0/1 (meaning no tile concept)
      */
     GT_U32                  numOfTiles;
    /** @brief bmp of actual exiting ports
     *  needed for devices that their physical ports not
     *  continues from first port num to last port num.
     *  For 'multi-port-groups' device , the bmp of 'global ports'
     *  For example : Lion --> 0..11 , 16..27 , 32..43 , 48..59
     */
    CPSS_PORTS_BMP_STC existingPorts;

} PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC;


/**
* @struct PRV_CPSS_DRV_GLOBAL_DATA_STC
 *
 * @brief hold info about the device returned by the cpssDriver, when
 * calling prvCpssDrvPpHwPhase1Init(...)
*/
typedef struct{

    /** @brief flag that state the DB of cpssDriver was
     *  released and now we are after that stage
     *  NOTE: the flag start with GT_FALSE , but once
     *  changed to GT_TRUE it will stay that way forever.
     */
    GT_BOOL prvCpssDrvAfterDbRelease;

    /** @brief GT_TRUE
     *  GT_FALSE - otherwise
     */
    GT_BOOL performReadAfterWrite;

} PRV_CPSS_DRV_GLOBAL_DATA_STC;

extern PRV_CPSS_DRV_GLOBAL_DATA_STC drvGlobalInfo;
/**
* @internal prvCpssDrvSysConfigPhase1 function
* @endinternal
*
* @brief   This function sets cpssDriver system level system configuration
*         parameters before any of the device's phase1 initialization .
*
* @note   APPLICABLE DEVICES:      ALL systems must call this function
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDrvSysConfigPhase1
(
    void
);

/**
* @internal prvCpssDrvHwPpPhase1Init function
* @endinternal
*
* @brief   This function is called by cpss "phase 1" family functions,
*         in order to enable PP Hw access,
*         the device number provided to this function may
*         be changed when calling prvCpssDrvPpHwPhase2Init().
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] ppOutInfoPtr             - (pointer to)the info that driver return to caller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPhase1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC *ppOutInfoPtr
);


/**
* @internal prvCpssDrvHwPpPhase2Init function
* @endinternal
*
* @brief   the function set parameters for the driver for the "init Phase 2".
*         the function "renumber" the current device number of the device to a
*         new device number.
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] devNum                   - The PP's current device number .
* @param[in] newDevNum                - The PP's "new" device number to register.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssDrvHwPpPhase2Init
(
    IN GT_U8    devNum,
    IN GT_U8    newDevNum
);

/**
* @internal prvCpssDrvHwPpDevRemove function
* @endinternal
*
* @brief   remove the device from the CPSS driver.
*         the function clean all the internal DB relate to the device , disconnect
*         from interrupts of the device
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvHwPpDevRemove
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDrvHwPpRenumber function
* @endinternal
*
* @brief   the function set parameters for the driver to renumber it's DB.
*         the function "renumber" the current device number of the device to a
*         new device number.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] oldDevNum                - The PP's "old" device number .
* @param[in] newDevNum                - The PP's "new" device number swap the DB to.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssDrvHwPpRenumber
(
    IN GT_U8    oldDevNum,
    IN GT_U8    newDevNum
);

/**
* @internal prvCpssDrvDebugDeviceIdSet function
* @endinternal
*
* @brief   This is debug function.
*         The function overrides device ID by given value.
*         The function should be called before cpssDxChHwPpPhase1Init().
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - device number .
* @param[in] devType                  - device type to store.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvDebugDeviceIdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
);

/**
* @internal prvCpssDrvAddrCheckWaBind function
* @endinternal
*
* @brief   Binds errata db with address check callback
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - the device number
* @param[in] addrCheckFuncPtr         - pointer to callback function that checks addresses.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS prvCpssDrvAddrCheckWaBind
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DRV_ERRATA_ADDR_CHECK_FUNC     addrCheckFuncPtr
);

/**
* @internal prvCpssDrvErrataCallbackBind function
* @endinternal
*
* @brief   Binds errata callback to prepare and complete special read/write operations
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - the device number
* @param[in] callbackFuncPtr          - pointer to prepare/complete special read/write operations function
* @param[in] callbackDataPtr          - pointer to prepare/complete special read/write operations anchor data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS prvCpssDrvErrataCallbackBind
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DRV_ERRATA_CALLBACK_FUNC_PTR   callbackFuncPtr,
    IN  GT_VOID                                 *callbackDataPtr
);

/**
* @internal prvCpssDrvPortsFullMaskGet function
* @endinternal
*
* @brief   get bmp of all phy ports of the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] numOfPorts               - number of physical ports in the device
*
* @param[out] portsBmpPtr              - (pointer to) bmp of ports
*                                       None
*/
void prvCpssDrvPortsFullMaskGet
(
    IN GT_U32                   numOfPorts,
    OUT CPSS_PORTS_BMP_STC       *portsBmpPtr
);

/**
* @internal prvCpssDrvPortsBmpMaskWithMaxPorts function
* @endinternal
*
* @brief   mask bmp ports with the max number of ports of the of the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] portsBmpPtr              - (pointer to) bmp of ports
* @param[in,out] portsBmpPtr              - (pointer to) bmp of ports , after the mask
*                                       None
*/
void prvCpssDrvPortsBmpMaskWithMaxPorts
(
    IN GT_U8                        devNum,
    INOUT CPSS_PORTS_BMP_STC       *portsBmpPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvPpCfgInith */

