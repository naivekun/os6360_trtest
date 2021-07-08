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
* @file cpssGenCfg.h
*
* @brief CPSS generic configuration types.
*
* @version   6
********************************************************************************
*/
#ifndef __cpssGenCfgh
#define __cpssGenCfgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>

/**
* @enum CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT
 *
 * @brief defines device's HW device usage
*/
typedef enum{

    /** device uses single HW device number */
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E,

    /** device uses dual HW device number */
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E

} CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT;


/**
* @enum CPSS_GEN_SIP_ENT
 *
 * @brief enumerator for the 'names' of SIP that added to the device
*/
typedef enum{

    /** legacy features. */
    CPSS_GEN_SIP_LEGACY_E = 0,

    /** @brief features that added in sip 5.0
     *  also known as 'eBridge' introducing eports and evlans.
     *  devices like : bobcat2_A0
     */
    CPSS_GEN_SIP_5_E,

    /** @brief features that added in generation 5.10
     *  devices like : bobcat2_B0
     */
    CPSS_GEN_SIP_5_10_E,

    /** @brief features that added in generation 5.15
     *  devices like : Caelum (Bobk)
     */
    CPSS_GEN_SIP_5_15_E,

    /** @brief features that added in generation 5.16
     *  devices like : Aldrin
     */
    CPSS_GEN_SIP_5_16_E,

    /** @brief features that added in generation 5.20
     *  devices like : Bobcat3
     */
    CPSS_GEN_SIP_5_20_E,

    /** @brief features that added in generation 5.25
     *  devices like : Aldrin2
     */
    CPSS_GEN_SIP_5_25_E,

    /** @brief features that added in generation 6.0
     *  devices like : Falcon
     */
    CPSS_GEN_SIP_6_E,

    /** the MAX value to be used for array sizes */
    CPSS_GEN_SIP_MAX_NUM_E

} CPSS_GEN_SIP_ENT;

/* number GT_U32 bmps needed to hold the 'features' bmp */
#define CPSS_GEN_SIP_BMP_MAX_NUM_CNS \
                (1 + (((CPSS_GEN_SIP_MAX_NUM_E)+1) >> 5))

/**
* @struct CPSS_GEN_CFG_DEV_INFO_STC
 *
 * @brief Generic device info structure
*/
typedef struct{

    /** device type of the PP. */
    CPSS_PP_DEVICE_TYPE devType;

    /** the device's revision number. */
    GT_U8 revision;

    /** CPSS's device family that device belongs to. */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /** CPSS's device sub */
    CPSS_PP_SUB_FAMILY_TYPE_ENT devSubFamily;

    /** maximal port's number not including CPU one. */
    GT_U32 maxPortNum;

    /** @brief number of virtual ports.
     *  Relevant only for devices with virtual ports support.
     */
    GT_U32 numOfVirtPorts;

    /** bitmap of actual exiting ports not including CPU one. */
    CPSS_PORTS_BMP_STC existingPorts;

    /** HW device number mode. */
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevNumMode;

    /** CPU port mode. */
    CPSS_NET_CPU_PORT_MODE_ENT cpuPortMode;

    /** @brief a number of LED interfaces per Port Group
     *  If a device is not multi-port-group this value is the
     *  same as a number of LED Interfaces per device.
     */
    GT_U32 numOfLedInfPerPortGroup;

    GT_U32 supportedSipBmp[CPSS_GEN_SIP_BMP_MAX_NUM_CNS];

} CPSS_GEN_CFG_DEV_INFO_STC;

/**
* @internal cpssPpCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
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
GT_STATUS cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenCfgh */

