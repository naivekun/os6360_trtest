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
* @file prvCpssDxChPortIfModeCfgResource.h
*
* @brief CPSS implementation for Port resource configuration.
*
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_RESOURCE_H
#define __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_RESOURCE_H

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* sizes of resource tables */

/* Aldrin2 */
/* size of tables per clock-rate starting from index == 1 */
#define PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS 4
/* Aldrin2 datapath 0 and 1 */
/* size of tables per speed starting from index == 1 */
/* added 107G as 100 and 11800M as 12500 */
#define PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS 21
/* Aldrin2 datapath 2 and 3 */
/* size of tables per speed starting from index == 1 */
/* added 11800M as 12500 */
/* added 29090M as 25000 */
#define PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS 15

/* Bobcat3 */
/* size of tables per clock-rate starting from index == 1 */
#define PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS 6
/* size of tables per speed starting from index == 1 */
#define PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS 21

/**
* @internal prvCpssDxChPortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesInit
(
    IN  GT_U8   devNum
);

/**
* @internal prvCpssDxChPortResourcesConfigDbInit function
* @endinternal
*
* @brief   Resource data structure initialization
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] maxDescCredits           - physical device number
* @param[in] maxHeaderCredits         - physical device number
* @param[in] maxPayloadCredits        - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbInit
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  maxDescCredits,
    IN  GT_U32  maxHeaderCredits,
    IN  GT_U32  maxPayloadCredits
);


/**
* @internal prvCpssDxChPortResourcesConfigDbAvailabilityCheck function
* @endinternal
*
* @brief   Resource data structure limitations checks
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*                                      txQDescCredits      - physical device number
* @param[in] txFifoHeaderCredits      - physical device number
* @param[in] txFifoPayloadCredits     - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbAvailabilityCheck
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
);


/**
* @internal prvCpssDxChPortResourcesConfigDbAdd function
* @endinternal
*
* @brief   Resource data structure DB add operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbAdd
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
);


/**
* @internal prvCpssDxChPortResourcesConfigDbDelete function
* @endinternal
*
* @brief   Resource data structure DB delete operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      txQDescCredits      - descriptor credit
* @param[in] txFifoHeaderCredits      - header credit
* @param[in] txFifoPayloadCredits     - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesConfigDbDelete
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits
);


/*-------------------------------------------------*
 *                                                 *
 *-------------------------------------------------*/
GT_STATUS prvCpssDxChPortResourcesConfigDbCoreOverallSpeedAdd
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  bandwidthMbps
);

GT_STATUS prvCpssDxChPortResourcesConfigDbCoreOverallSpeedDelete
(
    IN  GT_U8   devNum,
    IN GT_U32   dpIndex,
    IN  GT_U32  bandwidthMbps
);


/*-------------------------------------------------*
 *                                                 *
 *-------------------------------------------------*/

GT_STATUS prvCpssDxChPortResourcesConfigDbTMBWSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps,
    IN  GT_U32  dpIndex,
    IN  GT_U32  txQDescrCredits,
    IN  GT_U32  txFifoHeaderCredits,
    IN  GT_U32  txFifoPayloadCredits

);

GT_STATUS prvCpssDxChPortResourcesConfigDbTMBWGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  *tmBandwidthMbpsPtr
);

/**
* @internal prvCpssDxChPort_Bcat2A0_TM_ResourcesConfig function
* @endinternal
*
* @brief   Bcat A0 TM port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*
*/
GT_STATUS prvCpssDxChPort_Bcat2A0_TM_ResourcesConfig
(
    IN GT_U8       devNum
);

/**
* @internal prvCpssDxChPortApHighLevelConfigurationSet function
* @endinternal
*
* @brief   Allocate/release high level port data path resources (pizza etc.) for
*         port where AP enabled, while physical port interface configuration made
*         by HWS in co-processor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] portCreate               - create or delete port
* @param[in] ifMode                   - port intrface
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/

GT_STATUS prvCpssDxChPortApHighLevelConfigurationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         portCreate,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChPortApPortModeAckSet function
* @endinternal
*
* @brief   Send Ack to Service CPU indicate pizza resources allocate
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*                                      mode       - mode in CPSS format
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS prvCpssDxChPortApPortModeAckSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

#ifdef __cplusplus
}
#endif /* __cplusplus */





#endif


