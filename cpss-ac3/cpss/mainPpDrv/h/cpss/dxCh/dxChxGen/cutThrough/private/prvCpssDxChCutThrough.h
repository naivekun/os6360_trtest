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
* @file prvCpssDxChCutThrough.h
*
* @brief internal CPSS DXCH Cut Through facility API.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChCutThroughh
#define __prvCpssDxChCutThroughh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssDxChCutThroughBuffresUsingModeSet function
* @endinternal
*
* @brief   Set Buffers using mode.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] cutThroughEnable         – GT_TRUE – at least one port will be used in CutThrough mode.
*                                      GT_FALSE – all ports used in Store and Forward mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCutThroughBuffresUsingModeSet
(
    IN GT_U8                            devNum,
    IN GT_BOOL                          cutThroughEnable
);

/**
* @internal prvCpssDxChCutThroughRxErrorConfigSet function
* @endinternal
*
* @brief   Set Error Configuration of RX when obtained actual packet size different from calculated.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  paddedPacketEofWithError    - When a CT packet is padded it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
* @param[in]  truncatedPacketEofWithError - When a CT packet is truncated it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChCutThroughRxErrorConfigSet
(
    IN  GT_U8                 devNum,
    IN  GT_BOOL               paddedPacketEofWithError,
    IN  GT_BOOL               truncatedPacketEofWithError
);

/**
* @internal prvCpssDxChCutThroughPortMinCtByteCountDefaultValueSet function
* @endinternal
*
* @brief   Set minimal Cut Through packet byte count value for all RX_DMA channels.
*          Not depends of mapping physical ports to MAC/RX/TX
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X  Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] minCtByteCount           - minimal Cut Through packet byte count.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on wrong minCtByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChCutThroughPortMinCtByteCountDefaultValueSet
(
    IN GT_U8                 devNum,
    IN GT_U32                minCtByteCount
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCutThroughh */





