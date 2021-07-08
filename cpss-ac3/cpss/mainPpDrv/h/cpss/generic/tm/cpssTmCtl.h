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
* @file cpssTmCtl.h
* @version   1
********************************************************************************
*/

#ifndef __cpssTmCtlh
#define __cpssTmCtlh

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>

#define WAIT_LOOP   10
#define SLEEP_MS   100

/**
* @struct CPSS_TM_CTL_LAD_INF_PARAM_STC
 *
 * @brief This struct defines custom LAD interface parameters,
 * its used when needed to overwrite LADs Default configuration.
*/
typedef struct{

    /** @brief Minimal DRAM Package size in Chunk (64 bytes) units.
     *  value range is 0x1(64 Bytes) - 0x80 (8k Bytes).
     *  pagesPerbank     - Number of DRAM pages per bank that are used by the TM Queue Manager.
     *  PkgesPerbank     - Number of DRAM packeges per bank that are used by the TM Queue Manager.
     *  portChuncksEmitPerSel - Maximal number of chunks (16 bytes) emitted from port per single
     *  Scheduler selection.
     */
    GT_U32 minPkgSize;

    GT_U32 pagesPerBank;

    GT_U32 pkgesPerBank;

    GT_U32 portChunksEmitPerSel;

    GT_U8 bapIDs[2];

} CPSS_TM_CTL_LAD_INF_PARAM_STC;

#define CPSS_TM_CTL_MAX_NUM_OF_LADS_CNS 5
#define CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS 10

/**
* @internal cpssTmInit function
* @endinternal
*
* @brief   Initialize the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmInit
(
    IN GT_U8 devNum
);
/**
* @internal cpssTmInitExt function
* @endinternal
*
* @brief   Customize Initialize of the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] tmLibInitParams          - TM LIB initialize parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmInitExt
(
    IN GT_U8 devNum,
    IN CPSS_TM_LIB_INIT_PARAMS_STC *tmLibInitParams
);

/**
* @internal cpssTmClose function
* @endinternal
*
* @brief   Close the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmClose
(
    IN GT_U8 devNum
);

/**
* @internal cpssTmCtlReadRegister function
* @endinternal
*
* @brief   Read register.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddr                  - Register address.
* @param[in] dataPtr                  - Pointer to read data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCtlReadRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  regAddr,
    OUT GT_U64                 *dataPtr
);
/**
* @internal cpssTmCtlWriteRegister function
* @endinternal
*
* @brief   Write register.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddr                  - Register address.
* @param[in] dataPtr                  - Pointer to write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCtlWriteRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  regAddr,
    IN GT_U64                  *dataPtr
);


/**
* @internal cpssTmCtlLadParamsSet function
* @endinternal
*
* @brief   Set TM LAD parameters to its DB when neccessary to overwrite
*         its default configuration, parameters are used from DB
*         at cpssTmCtlLadInit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLads                - number of LADs.
* @param[in] ladParamsPtr             - (pointer of) CPSS_TM_CTL_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssTmCtlLadParamsSet
(
    IN  GT_U8                         devNum,
    IN  GT_U8                         numOfLads,
    IN  CPSS_TM_CTL_LAD_INF_PARAM_STC *ladParamsPtr
);

/**
* @internal cpssTmCtlLadParamsGet function
* @endinternal
*
* @brief   Get TM LAD parameters from its DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLads                - number of LADs.
* @param[in] ladParamsPtr             - (pointer of) CPSS_TM_CTL_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssTmCtlLadParamsGet
(
    IN  GT_U8                         devNum,
    IN  GT_U8                         numOfLads,
    OUT CPSS_TM_CTL_LAD_INF_PARAM_STC *ladParamsPtr
);

/**
* @internal cpssTmTreeParamsGet function
* @endinternal
*
* @brief   Get TM Tree Hw properties.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] tmTreeParamsPtr          - pointer to CPSS_TM_TREE_PARAMS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmTreeParamsGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_TREE_PARAMS_STC *tmTreeParamsPtr
);


/**
* @internal cpssTmUnitsErrorStatusGet function
* @endinternal
*
* @brief   Get TM Units Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] errorStatusInfoPtr       pointer to CPSS_TM_UNITS_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmUnitsErrorStatusGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_UNITS_ERROR_STATUS_STC *errorStatusInfoPtr
);

/**
* @internal cpssTmBapUnitErrorStatusGet function
* @endinternal
*
* @brief   Get TM BAP Unit Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bapNum                   - BAP interface number. (APPLICABLE RANGES: 0..CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS).
*
* @param[out] errorStatusInfoPtr       pointer to CPSS_TM_BAP_UNIT_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NO_RESOURCE           - on not exist BAP interface.
*/
GT_STATUS cpssTmBapUnitErrorStatusGet
(
    IN GT_U8 devNum,
    IN GT_U8 bapNum,
    OUT CPSS_TM_BAP_UNIT_ERROR_STATUS_STC *errorStatusInfoPtr
);


#endif 	    /* __cpssTmCtlh */

