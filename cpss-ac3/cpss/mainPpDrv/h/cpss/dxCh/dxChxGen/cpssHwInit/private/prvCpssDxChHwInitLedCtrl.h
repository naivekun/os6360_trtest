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
* @file prvCpssDxChHwInitLedCtrl.h
*
* @brief Includes Leds units control declaraioons
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChHwInitLedCtrlh
#define __prvCpssDxChHwInitLedCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>


/*---------------------------------------------------------------------------------------------------------
 *
 *   +-----------+-------------------------+-----------------------------+-----------------------------+-----------------------------+
 *   |    Led    |                         |   BobK/Caelum               |    Aldrin                   |       BC3                   |
 *   | Interface +------------+------------+------------+------+---------+------------+------+---------+------------+------+---------+
 *   |   Number  |   BC2 B0   | BobK/Cetus | base addr  | leds | macs    |  base addr | leds | macs    | base addr  | leds | macs    |
 *   +-----------+------------+------------+------------+------+---------+------------+------+---------+------------+------+---------+
 *   |   0       | 0x21000000 | 0x21000000 | 0x21000000 | 12   |         | 0x21000000 | 32   |         | 0x27000000 | 13   |  0-11,72|
 *   |   1       | 0x22000000 | 0x22000000 | 0x22000000 | 12   |         | 0x22000000 |  1   |         | 0x28000000 | 24   | 12-35   |
 *   |   2       | 0x23000000 |            | 0x23000000 | 12   |         |            |      |         | 0xA7000000 | 13   | 36-47,73|
 *   |   3       | 0x24000000 |            | 0x24000000 | 12   |         |            |      |         | 0xA8000000 | 24   | 48-71   |
 *   |   4       | 0x50000000 |            | 0x50000000 | 13   |         |            |      |         |            |      |         |
 *   +-----------+------------+------------+------------+------+---------+------------+------+---------+------------+------+---------+
 *
 *   LED interface:
 *
 *   +--------------------------+----------+-----------------------------+-----------------------------+--------------------+--------------------------+
 *   |                          |  Fields  |     BC2 B0                  |     BobK                    |       Aldrin       |          BC3             |
 *   |                          +-----+----+------------+----------------+------------+----------------+------------+-------+------------+-------------+
 *   |                          | per |size|  baseAddr  |   regN         |  baseAddr  |   regN         |  baseAddr  | regN  |   baseAddr |  regN       |
 *   |                          | reg |    |            |  0  1  2  3  4 |            |  0  1  2  3  4 |            |  0  1 |            |  0  1  2  3 |
 *   +--------------------------+-----+----+------------+----------------+------------+----------------+------------+-------+------------+-------------+
 *   | LEDControl               |          | 0x21000000 |  -             | 0x21000000 |  -             | 0x21000000 |  -    | 0x27000000 |  -          |
 *   | blinkGlobalControl       |          | 0x21000004 |  -             | 0x21000004 |  -             | 0x21000004 |  -    | 0x27000004 |  -          |
 *   | classGlobalControl       |          | 0x21000008 |  -             | 0x21000008 |  -             | 0x21000008 |  -    | 0x27000008 |  -          |
 *   | classesAndGroupConfig    |          | 0x2100000C |  -             | 0x2100000C |  -             | 0x2100000C |  -    | 0x2700000C |  -          |
 *   +--------------------------+-----+----+------------+----------------+------------+----------------+------------+-------+------------+-------------+
 *   | classForcedData[N]       |  1  | 32 | 0x21000010 |  6  6  6  6  6 | 0x21000010 |  6  6  6  6  6 | 0x21000010 |  6  6 | 0x27000010 |  6  6  6  6 |
 *   | portIndicationSelect[N]  |  6  |  5 | 0x21000028 | 12 12 12 12 12 | 0x21000028 | 12 12 12 12 13 | 0x21000028 | 32  1 | 0x27000028 | 13 24 13 24 |
 *   | portTypeConfig[1];       | 16  |  2 | 0x21000120 |  1  1  1  1  1 | 0x21000120 |  1  1  1  1  1 | 0x21000120 |  2  1 | 0x27000120 |  1  2  1  2 |
 *   | portIndicationPolarity[N]|  4  |  6 | 0x21000128 |  3  3  3  3  3 | 0x21000128 |  3  3  3  3  4 | 0x21000128 |  8  1 | 0x27000128 |  4  6  4  6 |
 *   | classRawData[N];         |  1  | 32 | 0x21000148 |  6  6  6  6  6 | 0x21000148 |  6  6  6  6  6 | 0x21000148 |  6  6 | 0x27000148 |  6  6  6  6 |
 *   +--------------------------+-----+----+------------+----------------+------------+----------------+------------+-------+------------+-------------+
 *
 *    Comment : Each led stream has its own number of registers
 *
 *---------------------------------------------------------------------------------------------------------*/

/* Up to 6 valid LED indications per port at a time */
#define PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS     6
/* Max number of led groups                         */
#define PRV_CPSS_DXCH_BOBCAT2_LED_GROUP_CNS                       2



/*---------------------------------------------------------------*
 * this constant defines invalid port LED position               *
 *---------------------------------------------------------------*/

#define PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS               0x3f
/*---------------------------------------------------------------*
 * Max number of LED ports in LED interface                      *
 *---------------------------------------------------------------*/

#define PRV_CPSS_DXCH_BOBCAT2_LED_IF_NUN_CNS                           5
#define PRV_CPSS_DXCH_BOBCAT2_LED_IF_PORTS_NUM_CNS                    12

#define PRV_CPSS_DXCH_CETUS_LED_IF_NUM_CNS                             2
#define PRV_CPSS_DXCH_CAELUM_LED_IF_NUM_CNS                            5



#define PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS                            2

#define PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_NUM_CNS                   32
#define PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_NUM_CNS                    1

#define PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_TYPE_CONF_REG_NUM_CNS      2
#define PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_TYPE_CONF_REG_NUM_CNS      1

#define PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_IND_POLARITY_REG_NUM_CNS   8
#define PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_IND_POLARITY_REG_NUM_CNS   1


#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS                           4

#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_NUM_CNS                  13
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_NUM_CNS                  24
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_NUM_CNS                  13
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_NUM_CNS                  24

#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_TYPE_CONF_REG_NUM_CNS     1
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_TYPE_CONF_REG_NUM_CNS     2
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_TYPE_CONF_REG_NUM_CNS     1
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_TYPE_CONF_REG_NUM_CNS     2


#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_IND_POLARITY_REG_NUM_CNS  4
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_IND_POLARITY_REG_NUM_CNS  6
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_IND_POLARITY_REG_NUM_CNS  4
#define PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_IND_POLARITY_REG_NUM_CNS  6

#define PRV_CPSS_DXCH_ALDRIN2_LED_IF_NUM_CNS                        4



/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain for specific MAC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portMacType              - macType
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  portMacType,
    IN  GT_U32                  position
);

/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain for specific MAC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portMacType              - macType
*
* @param[out] positionPtr              - port position index in LED stream
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  portMacType,
    IN  GT_U32                 *positionPtr
);


/*******************************************************************************
* prvCpssDxChLedErrataPortLedInit
*
* APPLICABLE DEVICES:
*       Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* INPUTS:
*       devNum          - physical device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - one of the parameters value is wrong
*       GT_OUT_OF_RANGE          - position out of range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLedErrataPortLedInit
(
    IN  GT_U8                   devNum
);


/**
* @internal prvCpssDxChLedErrataPortLedMacTypeSet function
* @endinternal
*
* @brief   Init FE-3763116 Port Led Mac type set and if postion is configured
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] macType                  -  , ( PRV_CPSS_PORT_NOT_EXISTS_E, if MAC is deacivated)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChLedErrataPortLedMacTypeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  macType
);

/**
* @internal prvCpssDxChLedErrataPortLedPositionSet function
* @endinternal
*
* @brief   Init FE-3763116 Port Led Position set and MAC type specified
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] position                 - led  on LED interface corresponding to the port (MAC)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChLedErrataPortLedPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHwInitLedCtrlh */

