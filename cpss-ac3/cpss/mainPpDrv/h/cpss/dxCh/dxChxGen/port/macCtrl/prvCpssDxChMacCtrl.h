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
* @file prvCpssDxChMacCtrl.h
*
* @brief bobcat2 mac control
*
* @version   2
********************************************************************************
*/

#ifndef __PRV_CPSS_DXCH_MAC_CTRL_H
#define __PRV_CPSS_DXCH_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @struct PRV_CPSS_DXCH_PORT_REG_CONFIG_STC
 *
 * @brief A structure to describe data to write to register
*/
typedef struct{

    /** register address */
    GT_U32 regAddr;

    /** bit number inside register */
    GT_U32 fieldOffset;

    /** bit number inside register */
    GT_U32 fieldLength;

    /** value to write into the register */
    GT_U32 fieldData;

} PRV_CPSS_DXCH_PORT_REG_CONFIG_STC;


#define PRV_CPSS_DXCH_BOBCAT2_MAX_MAC_D 72
#define PRV_CPSS_DXCH_BOBCAT3_MAX_MAC_D 74

/*--------------------------------------------------------*/
/* GIGA MAC                                               */
/*--------------------------------------------------------*/
#define GIGA_MAC_CTRL0_PORT_TYPE_FLD_OFFS_D      1
#define GIGA_MAC_CTRL0_PORT_TYPE_FLD_LEN_D       1

#define GIGA_MAC_CTRL1_PREAMBLE_LEN_4_D          4
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_8_D          8

/* Mac Control Get 1 */
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_OFFS_D   15
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_LEN_D    1


/* Mac Control Get 3 */
#define GIGA_MAC_CTRL3_IPG_MIN_LEN_D             0
#define GIGA_MAC_CTRL3_IPG_MAX_LEN_D             511

#define GIGA_MAC_CTRL3_IPG_LEN_FLD_OFFS_D        6
#define GIGA_MAC_CTRL3_IPG_LEN_FLD_LEN_D         9


/*--------------------------------------------------------*/
/* XLG MAC                                                */
/*--------------------------------------------------------*/
#define XLG_MAC_CTRL5_TX_IPG_MIN_VAL_D          8
#define XLG_MAC_CTRL5_TX_IPG_MAX_VAL_D          15

#define XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D       0
#define XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D        4

#define XLG_MAC_CTRL5_PREAMBLE_MIN_VAL_D          1
#define XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D          8


#define XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D          3
#define XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D  4
#define XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D   XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D
#define XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D  7
#define XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D   XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D


#define XLG_MAC_CTRL5_NUM_CRC_MIN_VAL_D           1
#define XLG_MAC_CTRL5_NUM_CRC_MAX_VAL_D           4

#define XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D     3
#define XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D 10
#define XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D  XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D
#define XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D 13
#define XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D  XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D

/**
* @internal prvCpssDxChBobcat2PortMacIsSupported function
* @endinternal
*
* @brief   check whether XLG mac is supported for specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type (mac)
*
* @param[out] isSupportedPtr           - pointer to is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacIsSupported
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN PRV_CPSS_PORT_TYPE_ENT            portType,
    OUT GT_BOOL                         *isSupportedPtr
);


/**
* @internal prvCpssDxChBobcat2PortMacIPGLengthSet function
* @endinternal
*
* @brief   set XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   = ipg  in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChBobcat2PortMacIPGLengthGet function
* @endinternal
*
* @brief   get XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
*
* @param[out] lengthPtr                = pointer to length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssDxChBobcat2PortMacPreambleLengthSet function
* @endinternal
*
* @brief   set XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                -  (RX/TX/both)
* @param[in] length                   = ipg  in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChBobcat2PortMacPreambleLengthGet function
* @endinternal
*
* @brief   get XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
* @param[in] direction                -  (RX/TX/both)
*
* @param[out] lengthPtr                - preable ipg length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssDxChBobcat2PortMacCrcModeSet function
* @endinternal
*
* @brief   Set XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
* @param[in] numCrcBytes              - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortMacCrcModeSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
);

/**
* @internal prvCpssDxChBobcat2PortMacCrcModeGet function
* @endinternal
*
* @brief   Get XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
*
* @param[out] numCrcBytesPtr           - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacCrcModeGet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                           *numCrcBytesPtr
);


/**
* @internal prvCpssDxChPortMacConfiguration function
* @endinternal
*
* @brief   Write value to register field and duplicate it to other members of SW
*         combo if needed
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] regDataArray             - array of register's address/offset/field lenght/value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortMacConfiguration
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,     
    IN  const PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssDxChPortMacConfigurationClear function
* @endinternal
*
* @brief   Clear array of registers data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] regDataArray             - "clean" array of register's data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChPortMacConfigurationClear
(
    INOUT PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssDxChMacByPhysPortGet function
* @endinternal
*
* @brief   get mac by port number
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] macPtr                   - pointer to is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChMacByPhysPortGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                  *macPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

