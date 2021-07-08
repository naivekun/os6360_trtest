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
* @file prvCpssDxChPortSlicesBM.h
*
* @brief Buffer Management Unit Slices Manipulation Interface
*
* @version   6
********************************************************************************
*/

#ifndef __CPSSDXCH_PORTPIZZA_SLICES_BM_H
#define __CPSSDXCH_PORTPIZZA_SLICES_BM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h> 
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/*--------------------------------------------------*/
/* BM slices manipulation                        */
/*--------------------------------------------------*/
/* BM control register
**
** 0-1 bits  enable the strict priority for CPU port
** 2-5 number of slices to use, 1-31 
**                                    0 if equal to 32
*/
/* BM Slot enable register field
** 0-31  corresponding slice(0-31) is enable/disable
**
** BM Slot configuration register
** 0-3   slice is enable to port N (4 bit for port number per slice) 
*/
#define BM_TOTAL_SLICES_CNS                  32

#define BM_CTRL_SLICE_NUM_BITS_CNS           0x3E
#define BM_CTRL_STRICT_PRIORITY_BIT_CNS      0x01

#define BM_CTRL_SLICE_NUM_OFFS_CNS           0x01
#define BM_CTRL_STRICT_PRIORITY_OFFS_CNS     0x00

#define BM_SLICE_PORT_BITS_CNS               0x0F


/**
* @internal prvLion2PortPizzaArbiterBMSlicesNumSet function
* @endinternal
*
* @brief   The function sets slice number used in Pizza Arbiter for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] slicesNum                - slices number to be set in pizza arbiter
* @param[in] enableStrictPriority4CPU - strict priority to CPU bit shall be set or not
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
*/
GT_STATUS prvLion2PortPizzaArbiterBMSlicesNumSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNum,
    IN GT_BOOL enableStrictPriority4CPU
);


/**
* @internal prvLion2PortPizzaArbiterBMSlicesNumGet function
* @endinternal
*
* @brief   The function get slice number used in Pizza Arbiter and strict priority state bit
*         for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
*
* @param[out] slicesNumPtr             - place to store slices number used in pizza arbiter
* @param[out] enableStrictPriority4CPUPtr - where to store strict priority to CPU bit
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - bad slicesNumPtr, enableStrictPriority4CPUPtr
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSlicesNumGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    OUT GT_U32  *slicesNumPtr,
    OUT GT_BOOL *enableStrictPriority4CPUPtr
);


/**
* @internal prvLion2PortPizzaArbiterBMSliceOccupy function
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
* @param[in] sliceId                  - slice to be occupied
* @param[in] portId                   - local port id (inside portGroup) to which slice is assigned
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceOccupy
(   
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceId,
    IN GT_PHYSICAL_PORT_NUM portId
);


/**
* @internal prvLion2PortPizzaArbiterBMSliceRelease function
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
* @param[in] sliceId                  - slice to be released
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceRelease
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceId
);


/**
* @internal prvLion2PortPizzaArbiterBMSliceStateSet function
* @endinternal
*
* @brief   The function sets the state of the slice (i.e. if slace is disable or enable
*         and if it enbaled to which port it is assigned) for BM unit.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be released
* @param[in] isEnabled                - is slace enabled
* @param[in] portId                   - pord id slace is enabled to
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId are greater than available
* @retval GT_BAD_PTR               - isOccupiedPtr or portNumPtr are NULL
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceStateSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceId,
    IN GT_BOOL              isEnabled, 
    IN GT_PHYSICAL_PORT_NUM portId
);

/**
* @internal prvLion2PortPizzaArbiterBMSliceGetState function
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
* @param[in] sliceId                  - slice to be released
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
GT_STATUS prvLion2PortPizzaArbiterBMSliceGetState
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceId,
    OUT GT_BOOL              *isOccupiedPtr, 
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);


/**
* @internal prvLion2PortPizzaArbiterBMInit function
* @endinternal
*
* @brief   Init BM unit (sets number of slice to use , strict priority to CPU bit
*         and disables all slices of the device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceNum2Init            - number of slices to be used
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceNum2Init is greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMInit
(
     IN GT_U8  devNum,
     IN GT_U32 portGroupId,
     IN GT_U32 sliceNum2Init
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


