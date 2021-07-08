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
* @file prvCpssDxChCGMacCtrl.h
*
* @brief SIP5.20 CG mac control API
*
* @version   3
********************************************************************************
*/

#ifndef PRV_CPSS_DXCH_CG_MAC_CTRL_H
#define PRV_CPSS_DXCH_CG_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvCpssDxChPortCGMacIsSupported function
* @endinternal
*
* @brief   check whether CG mac is supported for specific mac
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mac                      -  number
*
* @param[out] isSupportedPtr           is mac supported result
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChPortCGMacIsSupported
(
    IN GT_U8                    devNum,
    IN GT_U32                   mac,
    OUT GT_BOOL                *isSupportedPtr
);

/**
* @internal prvCpssDxChPortCGMacIPGLengthSet function
* @endinternal
*
* @brief   Set IPG length for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortCGMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
);


/**
* @internal prvCpssDxChPortCGMacIPGLengthGet function
* @endinternal
*
* @brief   Set IPG length for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] lengthPtr                - pointer to length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortCGMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssDxChPortCGMacPreambleLengthSet function
* @endinternal
*
* @brief   Set Preamble length for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortCGMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChPortCGMacPreambleLengthGet function
* @endinternal
*
* @brief   Get Preamble length for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
*
* @param[out] lengthPtr                - pointer to preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChPortCGMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
);

/**
* @internal prvCpssDxChPortCGMacNumCRCByteshSet function
* @endinternal
*
* @brief   Set number of CRC bytes for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortCGMacNumCRCByteshSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   crcBytesNum
);

/**
* @internal prvCpssDxChPortCGMacNumCRCByteshGet function
* @endinternal
*
* @brief   Get number of CRC bytes for CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] crcBytesNumPtr           - pointer to number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortCGMacNumCRCByteshGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                  *crcBytesNumPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

