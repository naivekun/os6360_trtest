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
* @file prvCpssDxChXLGMacCtrl.h
*
* @brief bobcat2 XLG mac control reg 5 API
*
* @version   3
********************************************************************************
*/

#ifndef PRV_CPSS_DXCH_MAC_CTRL_H
#define PRV_CPSS_DXCH_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvCpssDxChBobcat2PortMacXLGIsSupported function
* @endinternal
*
* @brief   check whether XLG mac is supported for specific mac
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      -  number
*
* @param[out] isSupportedPtr           is mac supported result
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacXLGIsSupported
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    OUT GT_BOOL                *isSupportedPtr
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacCtrl5FldSet function
* @endinternal
*
* @brief   XLG mac control register 5 set specific field
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] fldOffs                  - fld offset
* @param[in] fldLen                   - fld len
* @param[in] fldVal                   - value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacCtrl5FldSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   fldOffs,
    IN GT_U32                   fldLen,
    IN GT_U32                   fldVal
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacCtrl5FldGet function
* @endinternal
*
* @brief   XLG mac control register 5 get specific field
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
* @param[in] fldOffs                  - fld offset
* @param[in] fldLen                   - fld len
*
* @param[out] fldValPtr                - value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacCtrl5FldGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     mac,
    IN  GT_U32                   fldOffs,
    IN  GT_U32                   fldLen,
    OUT GT_U32                  *fldValPtr
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacIPGLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN GT_U32                   length
);


/**
* @internal prvCpssDxChBobcat2PortXLGMacIPGLengthGet function
* @endinternal
*
* @brief   XLG mac control register 5 get IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
*
* @param[out] lengthPtr                - pointer to length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssDxChBobcat2PortXLGMacPreambleLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set Preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - max number
* @param[in] direction                - RX/TX/both
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacPreambleLengthGet function
* @endinternal
*
* @brief   XLG mac control register 5 get Preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - max number
* @param[in] direction                - RX/TX/both
*
* @param[out] lengthPtr                - pointer to preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacNumCRCByteshSet function
* @endinternal
*
* @brief   XLG mac control register 5 set number of CRC bytes
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacNumCRCByteshSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   crcBytesNum
);

/**
* @internal prvCpssDxChBobcat2PortXLGMacNumCRCByteshGet function
* @endinternal
*
* @brief   XLG mac control register 5 get number of CRC bytes
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - max number
* @param[in] direction                - RX/TX/both
*
* @param[out] crcBytesNumPtr           - pointer to number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacNumCRCByteshGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *crcBytesNumPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

