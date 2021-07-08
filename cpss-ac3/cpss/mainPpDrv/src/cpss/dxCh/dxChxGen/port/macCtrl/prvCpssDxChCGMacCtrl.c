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
* @file prvCpssDxChCGMacCtrl.c
*
* @brief SIP5.20 CG mac control API
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChXLGMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChCGMacCtrl.h>



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
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(isSupportedPtr);

    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, mac, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *isSupportedPtr = GT_FALSE;
    }
    else
    {
        *isSupportedPtr = GT_TRUE;
    }
    return GT_OK;

}

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
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    direction = direction; /* avoid warning */

    if ((length != 4) && (length != 8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    regValue = (length == 4) ? 1 : 0;

    /* SHORT_PREAMBLE */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 30, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* RX_SFD_ANY */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 21, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 14, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value to be written */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(!((direction == CPSS_PORT_DIRECTION_RX_E) || (direction == CPSS_PORT_DIRECTION_TX_E)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 30, 1, &regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *lengthPtr = (regValue == 0) ? 8 : 4;

    return GT_OK;

}



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
* *                                     1-8 means -> IPG depend
*                                       on packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12
*                                       any other setting are
*                                       discarded

*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortCGMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*
        For 0 – DIC enabled. Mechanism of IPG is enabled to create average IPG of 12.
        For 1 – DIC disabled. IPG depend on packet size, move between 1byte to 8byte.
    */

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_TX_IPG_LENGTH_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if ((length >=1) && (length <=8))
    {
        regValue = 1;
    }
    else if (length == 12)
    {
        regValue = 0;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


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
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_TX_IPG_LENGTH_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 1, &regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
        For 0 – DIC enabled. Mechanism of IPG is enabled to create average IPG of 12.
        For 1 – DIC disabled. IPG depend on packet size, move between 1byte to 8byte.
    */
    *lengthPtr = (regValue == 0) ? 12 : 1;

    return GT_OK;
}


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
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    direction = direction;/* avoid warning */

    if ((crcBytesNum != 4) && (crcBytesNum != 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regValue = (crcBytesNum == 0) ? 1 : 0;

    /* CRC_FWD */
    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 6, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* disable_tx_crc */
    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 24, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* strip_crc_ind */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 15, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /* CRC_CHECK_DIS */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 16, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


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
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(crcBytesNumPtr);

    /* CRC_FWD */
    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 6, 1, &regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *crcBytesNumPtr = (regValue == 1) ? 0 : 4;

    return GT_OK;
}


