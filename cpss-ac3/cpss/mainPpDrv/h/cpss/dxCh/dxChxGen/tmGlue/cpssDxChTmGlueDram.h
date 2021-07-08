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
* @file cpssDxChTmGlueDram.h
*
* @brief TM related DRAM APIs H file
*
* @version   6
********************************************************************************
*/

#ifndef __cpssDxChTmGlueDramh
#define __cpssDxChTmGlueDramh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>

/* Number of buses per interface */
#define CPSS_DXCH_TM_GLUE_DRAM_INF_BUS_CNS  4

/* External Memory Initialization flags         */
/* Initialize DRAM and PHY only, don't touch TM */
#define CPSS_DXCH_TM_GLUE_DRAM_INIT_DDR_PHY_CNS  1

/**
* @enum CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT
 *
 * @brief TM DDR configuration type.
*/
typedef enum{

    /** Dynamic configuration. */
    CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E,

    /** Static configuration. */
    CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E

} CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT;

/**
* @struct CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC
 *
 * @brief This struct defines the DRAM configuration algorithm parameters.
*/
typedef struct{

    /** static or dynamic configuration. */
    CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT algoType;

    /** @brief GT_TRUE: enable performing write leveling.
     *  GT_FALSE: disable performing write leveling.
     *  Comment:
     */
    GT_BOOL performWriteLeveling;

} CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC;


/**
* @struct CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC
 *
 * @brief This struct defines specific DRAM bus interface parameters.
*/
typedef struct{

    /** @brief Chip Select (CS) bitmask, bit<n> for CS<n>.
     *  mirrorEnBitmask - Mirror enabling.
     */
    GT_U32 csBitmask;

    GT_BOOL mirrorEn;

    /** DQS Swap (polarity) enabling. */
    GT_BOOL dqsSwapEn;

    /** @brief CK Swap (polarity) enabling.
     *  Comment:
     */
    GT_BOOL ckSwapEn;

} CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC;

/**
* @struct CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC
 *
 * @brief This struct defines specific DRAM interface parameters.
*/
typedef struct{

    CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC busParams[CPSS_DXCH_TM_GLUE_DRAM_INF_BUS_CNS];

    /** Speed bining (JEDEC standard name). */
    CPSS_DRAM_SPEED_BIN_ENT speedBin;

    /** Memory bus width. */
    CPSS_DRAM_BUS_WIDTH_ENT busWidth;

    /** Memory Size (in MByte). */
    CPSS_DRAM_SIZE_ENT memorySize;

    /** Memory interface frequency. */
    CPSS_DRAM_FREQUENCY_ENT memoryFreq;

    /** @brief Delay CAS Write Latency.
     *  (0 for using default value - jedec suggested).
     */
    GT_U32 casWL;

    /** @brief Delay CAS Latency.
     *  (0 for using default value - jedec suggested).
     */
    GT_U32 casL;

    /** @brief operating temperature.
     *  Comment:
     */
    CPSS_DRAM_TEMPERATURE_ENT interfaceTemp;

} CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC;

/**
* @struct CPSS_DXCH_TM_GLUE_DRAM_CFG_STC
 *
 * @brief This struct defines specific DRAM intrface hardware parameters relevant
 * for the Traffic Manager.
*/
typedef struct{

    /** @brief Number of active DDR interfaces.
     *  (APPLICABLE RANGES: 2..5.)
     */
    GT_U32 activeInterfaceNum;

    /** @brief Custom Order of active DDR interfaces.
     *  APPLICABLE RANGES:
     *  0x0    - mask is calculated from activeInterfaceNum.
     *  0x01..0x1F - custom mask is specified.
     */
    GT_U32 activeInterfaceMask;

    /** @brief Controller interface configurations.
     *  Comment:
     */
    CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC interfaceParams;

} CPSS_DXCH_TM_GLUE_DRAM_CFG_STC;

/**
* @internal cpssDxChTmGlueDramInit function
* @endinternal
*
* @brief   Traffic Manager DRAM related configurations
*         (DDR, PHY & TM BAP registers).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] dramCfgPtr               - (pointer to) DRAM interface configuration parameters
* @param[in] dramAlgoPtr              - (pointer to) DRAM configuration algorithm parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_CPU_MEM        - no CPU memory allocation fail
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDramInit
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_TM_GLUE_DRAM_CFG_STC         *dramCfgPtr,
    IN  CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC   *dramAlgoPtr
);

/**
* @internal cpssDxChTmGlueDramInitFlagsSet function
* @endinternal
*
* @brief   Set flags for Traffic Manager DRAM related configurations
*         (DDR, PHY & TM BAP registers).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] externalMemoryInitFlags  - External Memory Init Flags
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDramInitFlagsSet
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                externalMemoryInitFlags
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTmGlueDramh */



