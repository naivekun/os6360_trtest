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
* @file cpssDxChHwInitLedCtrl.h
*
* @brief Includes Leds control functions declarations.
*
* @version   17
********************************************************************************
*/
#ifndef __cpssDxChHwInitLedCtrlh
#define __cpssDxChHwInitLedCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>

#define CPSS_DXCH_MAX_LED_CLASS_CNS       7   /* max number of led indication classes */
#define CPSS_DXCH_MAX_LED_GROUP_CNS       4   /* max number of led groups             */




/*
 * typedef: struct CPSS_DXCH_LED_CLASS_CONF_STC
 *
 * Description:
 *      led class indication configuration.
 *
 * Fields:
 *      classNum    - the class number.
 *      classConf   - the class manipulation register value for classNum.
 */
typedef struct CPSS_DXCH_LED_CLASS_CONF_STCT
{
    GT_U32      classNum;
    GT_U32      classConf;
} CPSS_DXCH_LED_CLASS_CONF_STC;


/*
 * typedef: struct CPSS_DXCH_LED_GROUP_CONF_STC
 *
 * Description:
 *      led group configuration.
 *
 * Fields:
 *      groupNum    - the group number.
 *      groupConf   - the group logic register value for groupNum.
 */
typedef struct CPSS_DXCH_LED_GROUP_CONF_STC
{
    GT_U32      groupNum;
    GT_U32      groupConf;
} CPSS_DXCH_LED_GROUP_CONF_STC;

/**
* @enum CPSS_DXCH_LED_CPU_OR_PORT27_ENT
 *
 * @brief CPU port or Port 27 indication display on
 * LED interface 0.
*/
typedef enum{

    /** select cpu port indication. */
    CPSS_DXCH_LED_CPU_E,

    /** select port27 indication. */
    CPSS_DXCH_LED_PORT27_E

} CPSS_DXCH_LED_CPU_OR_PORT27_ENT;

/**
* @enum CPSS_DXCH_LED_PORT_TYPE_ENT
 *
 * @brief Port type to which the class manipulation
 * or group configuration is relevant.
*/
typedef enum{

    /** @brief Tri-speed port.
     *  For xCat, xCat3: Network port.
     *  For Lion: GE or XG port.
     */
    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,

    /** @brief XG port.
     *  For xCat, xCat3: Flex port.
     *  For Lion: Reserved.
     */
    CPSS_DXCH_LED_PORT_TYPE_XG_E

} CPSS_DXCH_LED_PORT_TYPE_ENT;

/**
* @enum CPSS_DXCH_LED_INDICATION_ENT
 *
 * @brief Ports (debug) data indications.
*/
typedef enum{

    /** @brief primary indication
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_PRIMARY_E,

    /** @brief Link Enable
     */
    CPSS_DXCH_LED_INDICATION_LINK_E,

    /** @brief receive activity.
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_RX_ACT_E,

    /** @brief transmit activity.
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_TX_ACT_E,

    /** @brief XG mode: local fault
     *  SGMII mode: PCS link
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_LOCAL_FAULT_OR_PCS_LINK_E,

    /** @brief XG mode: remote fault
     *  SGMII mode: reserved (set to 0x0)
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_REMOTE_FAULT_OR_RESERVED_E,

    /** @brief XG mode: unknown sequence
     *  SGMII mode: duplex
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_UKN_SEQ_OR_DUPLEX_E,

    /** @brief p rej.
     *  In case Tri speed indications are enabled for flex port, activity indication
     *  will replace the p rej one.
     *  Refer to cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet API.
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_P_REJ_E,

    /** @brief receive error.
     */
    CPSS_DXCH_LED_INDICATION_RX_ERROR_E,

     /** @brief jabber.
     *  In case Tri speed indications are enabled for flex port, GMII speed indication
     *  will replace the jabber one.
     *  Refer to cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet API.
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_JABBER_E,

    /** @brief fragment.
     *  In case Tri speed indications are enabled for flex port, MII speed indication
     *  will replace the fragment one.
     *  Refer to cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet API.
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_FRAGMENT_E,

    /** @brief CRC error
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_CRC_ERROR_E,

    /** @brief flow control received
     */
    CPSS_DXCH_LED_INDICATION_FC_RX_E,

    /** @brief XG mode: transmit bad CRC
     *  SGMII mode: late collision
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_TX_BAD_CRC_OR_LATE_COL_E,

    /** @brief XG mode: receive buffer full
     *  SGMII mode: back pressure
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_RX_BUFFER_FULL_OR_BACK_PR_E,

    /** @brief watchdog expired sample
     *  (APPLICABLE DEVICES: xCat; xCat3; xCat2)
     */
    CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E,

    /** @brief RX + TX act
     *  (APPLICABLE DEVICES: xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E,

    /** @brief flow control transmitted
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_FC_TX_E,

    /** @brief Port Enabled
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E,

    /** @brief GMII Speed
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_GMII_SPEED_E,

    /** @brief MII Speed
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_MII_SPEED_E,

    /** @brief full duplex
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E,

    /** @brief Indication Collision
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    CPSS_DXCH_LED_INDICATION_COLLISION_E

} CPSS_DXCH_LED_INDICATION_ENT;

/**
* @internal cpssDxChLedStreamClassAndGroupConfig function
* @endinternal
*
* @brief   This routine configures the control, the class manipulation and the
*         group configuration registers for both led interfaces in Dx.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledControl               - The user interface control register value.
* @param[in] ledClassConfPtr          (pointer to) a list of class manipulation registers
* @param[in] ledClassNum              - the class list length
* @param[in] ledGroupConfPtr          (pointer to) a list of group configuration registers
* @param[in] ledGroupNum              - the group list length
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledClassNum value or ledGroupNum value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. See the Led Serial Interface in the device's data-sheet for full
*       detail of the leds configuration, classes and groups.
*
*/
GT_STATUS cpssDxChLedStreamClassAndGroupConfig
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ledControl,
    IN  CPSS_DXCH_LED_CLASS_CONF_STC        *ledClassConfPtr,
    IN  GT_U32                              ledClassNum,
    IN  CPSS_DXCH_LED_GROUP_CONF_STC        *ledGroupConfPtr,
    IN  GT_U32                              ledGroupNum
);


/**
* @internal cpssDxChLedStreamHyperGStackTxQStatusEnableSet function
* @endinternal
*
* @brief   Enables the display to HyperGStack Ports Transmit Queue Status via LED
*         Stream.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);


/**
* @internal cpssDxChLedStreamHyperGStackTxQStatusEnableGet function
* @endinternal
*
* @brief   Gets HyperGStack Ports Transmit Queue Status via LED Stream.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableGet
(
    IN  GT_U8       devNum,
    OUT  GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChLedStreamCpuOrPort27ModeSet function
* @endinternal
*
* @brief   This routine configures the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] indicatedPort            - the selected port indication: CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort
);

/**
* @internal cpssDxChLedStreamCpuOrPort27ModeGet function
* @endinternal
*
* @brief   This routine gets the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] indicatedPortPtr         - (pointer to) the selected port indication:
*                                      CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_LED_CPU_OR_PORT27_ENT *indicatedPortPtr
);

/**
* @internal cpssDxChLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
* @internal cpssDxChLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
* @internal cpssDxChLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
);

/**
* @internal cpssDxChLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
);

/**
* @internal cpssDxChLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
);

/**
* @internal cpssDxChLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
);

/**
* @internal cpssDxChLedStreamClassIndicationSet function
* @endinternal
*
* @brief   This routine configures classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
* @param[in] indication               - port (debug)  data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
);

/**
* @internal cpssDxChLedStreamClassIndicationGet function
* @endinternal
*
* @brief   This routine gets classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
*
* @param[out] indicationPtr            - (pointer to) port (debug) indication data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassIndicationGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         ledInterfaceNum,
    IN  GT_U32                         classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT   *indicationPtr
);

/**
* @internal cpssDxChLedStreamDirectModeEnableSet function
* @endinternal
*
* @brief   This routine enables\disables LED Direct working mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] enable                   - GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 - indicates stack port 24
*       LEDSTB1 - indicates stack port 25
*       LEDDATA0 - indicates stack port 26
*       LEDSTB0 - indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
GT_STATUS cpssDxChLedStreamDirectModeEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChLedStreamDirectModeEnableGet function
* @endinternal
*
* @brief   This routine gets LED pin indication direct mode enabling status.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 - indicates stack port 24
*       LEDSTB1 - indicates stack port 25
*       LEDDATA0 - indicates stack port 26
*       LEDSTB0 - indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
GT_STATUS cpssDxChLedStreamDirectModeEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChLedStreamPortGroupConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
);

/**
* @internal cpssDxChLedStreamPortGroupConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
);

/**
* @internal cpssDxChLedStreamPortGroupClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
* @internal cpssDxChLedStreamPortGroupClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2,  Lion, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
* @internal cpssDxChLedStreamPortGroupGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
);

/**
* @internal cpssDxChLedStreamPortGroupGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, xCat2 0..1;
*                                      Lion, Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
);

/**
* @internal cpssDxChLedStreamSyncLedsEnableSet function
* @endinternal
*
* @brief   This routine enables\disables the global state in which the two LED
*         streams are synchronized.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE: LED streams are synchronized
*                                      GT_FALSE: LED streams are not synchronized
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamSyncLedsEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChLedStreamSyncLedsEnableGet function
* @endinternal
*
* @brief   This routine gets the global state indicating if the two LED streams
*         are synchronized.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: LED streams are synchronized
*                                      GT_FALSE: LED streams are not synchronized
* @param[out] inSyncTrigPtr            - (pointer to) GT_TRUE: synchronization triggering
*                                      not finished yet.
*                                      GT_FALSE: synchronization triggering
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamSyncLedsEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_BOOL *inSyncTrigPtr
);

/**
* @internal cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet function
* @endinternal
*
* @brief   This routine enables having some LED indications of Giga Ethernet MAC
*         for a selected stack\flex port.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - flex port number
* @param[in] enable                   - GT_TRUE:  having some Giga Ethernet MAC indications.
*                                      GT_FALSE: use the legacy indications.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Giga Ethernet mac of the stack port is not sending out on the LED
*       stream interface few important indications: Activity, GMII & MII
*       speeds.
*       The logic which decided which indications to send out was modified for
*       the STACK PORT case: the indications are manipulated to resemble the XG
*       stream - as a result few important indications for Giga Ethernet mode
*       are not in the stream by default.
*       When enabling by this API, the following indication will be replaced:
*       Old indication                   New Indication
*       ==============                   ==============
*       p rej  (CPSS_DXCH_LED_INDICATION_P_REJ_E)     Activity
*       jabber  (CPSS_DXCH_LED_INDICATION_JABBER_E)    GMII speed
*       fragment (CPSS_DXCH_LED_INDICATION_FRAGMENT_E)   MII speed
*       Refer to cpssDxChLedStreamClassIndicationSet\Get APIs and the
*       CPSS_DXCH_LED_INDICATION_ENT structure definition.
*
*/
GT_STATUS cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet function
* @endinternal
*
* @brief   This routine gets enabling status of having some LED indications of
*         Giga Ethernet MAC for a selected stack\flex port.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - flex port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable having some Giga Ethernet MAC indications.
*                                      GT_FALSE: use the legacy indications.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See comment for cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet.
*
*/
GT_STATUS cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
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
GT_STATUS cpssDxChLedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
);

/**
* @internal cpssDxChLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortPositionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
);

/**
* @internal cpssDxChLedStreamPortClassIndicationSet function
* @endinternal
 *
* @brief   This routine set the port class indications
 *
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
 *
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] indication               - value of port class indication
*                                      (APPLICABLE VALUES:
*                                       CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E
*                                       CPSS_DXCH_LED_INDICATION_LINK_E
*                                       CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E
*                                       CPSS_DXCH_LED_INDICATION_RX_ERROR_E
*                                       CPSS_DXCH_LED_INDICATION_FC_TX_E
*                                       CPSS_DXCH_LED_INDICATION_FC_RX_E
*                                       CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E
*                                       CPSS_DXCH_LED_INDICATION_GMII_SPEED_E
*                                       CPSS_DXCH_LED_INDICATION_MII_SPEED_E)
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
GT_STATUS cpssDxChLedStreamPortClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
);

/**
* @internal cpssDxChLedStreamPortClassIndicationGet function
* @endinternal
 *
* @brief   This routine gets the port class indications
 *
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
 *
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[out] indicationPtr           - (pointer to) value of the port class indication.
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
GT_STATUS cpssDxChLedStreamPortClassIndicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT    *indicationPtr
);

/**
* @internal cpssDxChLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] invertEnable             - status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
);

/**
* @internal cpssDxChLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
*
* @param[out] invertEnablePtr          - (pointer to) status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT GT_BOOL                         *invertEnablePtr
);


/**
* @enum CPSS_DXCH_LED_PHY_SELECT_ENT
 *
 * @brief Options for LED selection on remote PHY.
*/
typedef enum{

    /** off = no link, on = link, blink = activity, blink speed = link speed */
    CPSS_DXCH_LED_PHY_LINK_ACT_SPEED = 1,

    /** off = no link, on = link, blink = activity */
    CPSS_DXCH_LED_PHY_LINK_ACT,

    /** off = no link, on = link */
    CPSS_DXCH_LED_PHY_LINK,

    /** off = no link, on = 10, blink = activity */
    CPSS_DXCH_LED_PHY_10_LINK_ACT,

    /** off = no link, on = 10 */
    CPSS_DXCH_LED_PHY_10_LINK,

    /** off = no link, on = 100 link, blink = activity */
    CPSS_DXCH_LED_PHY_100_LINK_ACT,

    /** off = no link, on = 100 link */
    CPSS_DXCH_LED_PHY_100_LINK,

    /** off = no link, on = 1000 link, blink = activity */
    CPSS_DXCH_LED_PHY_1000_LINK_ACT,

    /** off = no link, on = 1000 link */
    CPSS_DXCH_LED_PHY_1000_LINK,

    /** off = no link, on = 10 or 100 link, blink = activity */
    CPSS_DXCH_LED_PHY_10_100_LINK_ACT,

    /** off = no link, on = 10 or 100 link */
    CPSS_DXCH_LED_PHY_10_100_LINK,

    /** off = no link, on = 10 or 1000 link, blink = activity */
    CPSS_DXCH_LED_PHY_10_1000_LINK_ACT,

    /** off = no link, on = 10 or 1000 link */
    CPSS_DXCH_LED_PHY_10_1000_LINK,

    /** off = no link, on = 100 or 1000 link, blink = activity */
    CPSS_DXCH_LED_PHY_100_1000_LINK_ACT,

    /** off = no link, on = 100 or 1000 link */
    CPSS_DXCH_LED_PHY_100_1000_LINK,

    /** special leds */
    CPSS_DXCH_LED_PHY_SPECIAL,

    /** off = half duplx, on = full duplex, blink = collision */
    CPSS_DXCH_LED_PHY_DUPLEX_COL,

    /** off = no link, blink on = activity */
    CPSS_DXCH_LED_PHY_ACTIVITY,

    CPSS_DXCH_LED_PHY_RESERVED,

    /** force blink */
    CPSS_DXCH_LED_PHY_FORCE_BLINK,

    /** force off */
    CPSS_DXCH_LED_PHY_FORCE_OFF,

    /** force on */
    CPSS_DXCH_LED_PHY_FORCE_ON,

    CPSS_DXCH_LED_PHY_LAST

} CPSS_DXCH_LED_PHY_SELECT_ENT;

/**
* @struct CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC
 *
 * @brief LED global (per-PHY) configuration.
*/
typedef struct{

    /** GT_FALSE: use multiplexed mode, GT_TRUE: use direct mode */
    GT_BOOL directMode;

    /** bitmap of columns to skip in multiplexed mode */
    GT_U32 skipColumns;

    /** bitmap of local ports (1..9) to include in "Special LED 1" */
    GT_U32 specialLed1Bitmap;

    /** bitmap of local ports (1..9) to include in "Special LED 2" */
    GT_U32 specialLed2Bitmap;

    /** bitmap of local ports (1..9) to include in "Special LED 3" */
    GT_U32 specialLed3Bitmap;

} CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC;

/**
* @struct CPSS_DXCH_LED_PHY_PORT_CONF_STC
 *
 * @brief LED per-port configuration.
*/
typedef struct{

    /** @brief LED 0 selection
     *  (APPLICABLE VALUES:
     *  CPSS_DXCH_LED_PHY_LINK_ACT_SPEED,
     *  CPSS_DXCH_LED_PHY_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_LINK,
     *  CPSS_DXCH_LED_PHY_10_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_10_LINK,
     *  CPSS_DXCH_LED_PHY_1000_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_10_1000_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_100_1000_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_100_1000_LINK,
     *  CPSS_DXCH_LED_PHY_SPECIAL,
     *  CPSS_DXCH_LED_PHY_DUPLEX_COL,
     *  CPSS_DXCH_LED_PHY_FORCE_BLINK,
     *  CPSS_DXCH_LED_PHY_FORCE_OFF,
     *  CPSS_DXCH_LED_PHY_FORCE_ON
     *  )
     */
    CPSS_DXCH_LED_PHY_SELECT_ENT led0Select;

    /** @brief LED 1 selection:
     *  (APPLICABLE VALUES:
     *  CPSS_DXCH_LED_PHY_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_100_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_100_LINK,
     *  CPSS_DXCH_LED_PHY_1000_LINK,
     *  CPSS_DXCH_LED_PHY_10_100_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_10_100_LINK,
     *  CPSS_DXCH_LED_PHY_10_1000_LINK_ACT,
     *  CPSS_DXCH_LED_PHY_10_1000_LINK,
     *  CPSS_DXCH_LED_PHY_SPECIAL,
     *  CPSS_DXCH_LED_PHY_ACTIVITY,
     *  CPSS_DXCH_LED_PHY_FORCE_BLINK,
     *  CPSS_DXCH_LED_PHY_FORCE_OFF,
     *  CPSS_DXCH_LED_PHY_FORCE_ON
     *  )
     */
    CPSS_DXCH_LED_PHY_SELECT_ENT led1Select;

    /** @brief pulse strech selection
     *  0x0 - no pulse stretching
     *  0x1 - 21 mSec
     *  0x2 - 42 mSec
     *  0x3 - 84 mSec
     *  0x4 - 168 mSec
     */
    GT_U32 pulseStretch;

    /** @brief blink rate selection
     *  0x0 - 21 mSec
     *  0x1 - 42 mSec
     *  0x2 - 84 mSec
     *  0x3 - 168 mSec
     *  0x4 - 336 mSec
     *  0x5 - 672 mSec
     */
    GT_U32 blinkRate;

} CPSS_DXCH_LED_PHY_PORT_CONF_STC;

/**
* @internal cpssDxChLedPhyControlGlobalSet function
* @endinternal
*
* @brief   Set global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
* @param[in] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlGlobalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
);

/**
* @internal cpssDxChLedPhyControlGlobalGet function
* @endinternal
*
* @brief   Get global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
*
* @param[out] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlGlobalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
);

/**
* @internal cpssDxChLedPhyControlPerPortSet function
* @endinternal
*
* @brief   Set per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
* @param[in] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlPerPortSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
);

/**
* @internal cpssDxChLedPhyControlPerPortGet function
* @endinternal
*
* @brief   Get per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
*
* @param[out] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlPerPortGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHwInitLedCtrlh */

