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
* @file prvCpssDxChPortSlices.h
*
* @brief interface for slices manipulation from Pizza Arbiter point of view
*
*
* @version   6
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PRT_SLICES_H
#define __PRV_CPSS_DXCH_PRT_SLICES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>

/**
* @internal prvLion2PortPizzaArbiterConfigureSlicesInit function
* @endinternal
*
* @brief   Init all arbiter' unit (sets number of slice to use , strict priority to CPU bit
*         and disables all slices of the device.)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] slicesNumberPerGop       - number of slices to be used
* @param[in] txQDefPtr                - tx pizza definiton (repetition counter how many times the pizza shall be repeated
*                                      for port group with CPU connected)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - sliceNum2Init is greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterConfigureSlicesInit
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNumberPerGop,
    IN CPSS_DXCH_TXQDEF_STC *txQDefPtr
);

/**
* @internal prvLion2PortPizzaArbiterConfigureSliceOccupy function
* @endinternal
*
* @brief   The function occupy the slice (i.e. set the spicific slice
*         be asigned to specific port for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceIdx_inGop           - slice to be occupied
* @param[in] localPort                - local port id (inside portGroup) to which slice is assigned
* @param[in] slicesNumberPerGop       - slice number to be configured
* @param[in] txQDefPtr                - txQ definition
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_ABORT                 - on verification error
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterConfigureSliceOccupy
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceIdx_inGop,
    IN GT_PHYSICAL_PORT_NUM localPort,
    IN GT_U32 slicesNumberPerGop,
    IN CPSS_DXCH_TXQDEF_STC *txQDefPtr
);

/**
* @internal prvLion2PortPizzaArbiterConfigureSliceRelease function
* @endinternal
*
* @brief   The function release the slice (i.e. set the spicific slice
*         to be disable to any port) for BM unit)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceIdx_inGop           - slice to be occupied
* @param[in] slicesNumberPerGop       - slice number to be configured
* @param[in] txQDefPtr                - txQ definition
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_ABORT                 - on verification error
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterConfigureSliceRelease
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceIdx_inGop,
    IN GT_U32 slicesNumberPerGop,
    IN CPSS_DXCH_TXQDEF_STC *txQDefPtr
);

/**
* @internal prvLion2PortPizzaArbiterConfigureSliceGetState function
* @endinternal
*
* @brief   The function gets the state of the slice (i.e. if slace is disable or enable
*         and if it enbaled to which port it is assigned) for BM unit.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceIdx_inGop           - slice to be released
*
* @param[out] isOccupiedPtr            - place to store whether slice is occupied
* @param[out] portNumPtr               - plavce to store to which port slice is assigned
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId are greater than available
* @retval GT_BAD_PTR               - isOccupiedPtr or portNumPtr are NULL
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterConfigureSliceGetState
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceIdx_inGop,
    OUT GT_BOOL              *isOccupiedPtr, 
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



