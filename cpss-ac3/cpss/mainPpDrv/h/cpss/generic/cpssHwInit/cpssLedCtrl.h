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
* @file cpssLedCtrl.h
*
* @brief Includes LEDs control data structures and enumerations definitions
*
* @version   2
********************************************************************************
*/
#ifndef __cpssLedCtrlh
#define __cpssLedCtrlh

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_LED_CLASS_5_SELECT_ENT
 *
 * @brief The indication displayed on class5 (for dual-media port\phy).
*/
typedef enum{

    /** Half Duplex is displayed on class5. */
    CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E,

    /** @brief If port is a dual media port,
     *  Fiber Link Up is displayed on class5.
     */
    CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E

} CPSS_LED_CLASS_5_SELECT_ENT;

/**
* @enum CPSS_LED_CLASS_13_SELECT_ENT
 *
 * @brief The indication displayed on class13 (for dual-media port\phy).
*/
typedef enum{

    /** Link Down is displayed on class13. */
    CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E,

    /** @brief If port is a dual media port,
     *  Copper Link Up is displayed on class13.
     */
    CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E

} CPSS_LED_CLASS_13_SELECT_ENT;

/**
* @enum CPSS_LED_DATA_ENT
 *
 * @brief Types of data reflected by the XG-PCS lane LEDs
*/
typedef enum{

    /** LED low */
    CPSS_LED_DATA_LOW_E,

    /** LED high */
    CPSS_LED_DATA_HIGH1_E,

    /** LED high */
    CPSS_LED_DATA_HIGH2_E,

    /** Sync indication for the lane */
    CPSS_LED_DATA_SYNC_E,

    /** PPM FIFO overrun */
    CPSS_LED_DATA_OVERRUN_E,

    /** PPM FIFO underrun */
    CPSS_LED_DATA_UNDERRUN_E,

    /** /E/ code or invalid code */
    CPSS_LED_DATA_BYTE_ERROR_E,

    /** Disparity error, invalid code */
    CPSS_LED_DATA_DISPARITY_ERROR_E,

    /** Link and data on RX XGMII */
    CPSS_LED_DATA_RX_E,

    /** Link and data on RX XGMII */
    CPSS_LED_DATA_TX_E,

    /** RX or TX */
    CPSS_LED_DATA_ACTIVITY_E,

    /** RX or Link */
    CPSS_LED_DATA_RX_OR_LINK_E,

    /** Lane's link is up */
    CPSS_LED_DATA_LINK_E,

    /** Local fault indication on the lane */
    CPSS_LED_DATA_LOCAL_FAULT_E

} CPSS_LED_DATA_ENT;

/*
 * typedef: struct CPSS_LED_CONF_STC
 *
 * Description: 
 *      LED stream configuration parameters.
 *
 * Fields:
 *      ledOrganize         - The order of the signals driven in the LED stream.
 *      disableOnLinkDown   - How to display indications when the link is down:
 *                            GT_FALSE - No effect, Link status has no effect 
 *                                       on other indications.
 *                            GT_TRUE - When link is down, the following indications: 
 *                                      speed, duplex, activity are forced down.
 *                              (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *      blink0DutyCycle     - The duty cycle of the Blink0 signal.
 *      blink0Duration      - The period of the Blink0 signal.
 *      blink1DutyCycle     - The duty cycle of the Blink1 signal.
 *      blink1Duration      - The period of the Blink1 signal.
 *      pulseStretch        - The length of stretching for dynamic signals.
 *      ledStart            - The first bit in the LED stream to be driven, range 0..255
 *      ledEnd              - The last bit in the LED stream to be driven, range 0..255
 *      clkInvert           - Inverts the LEDClk pin, GT_FALSE - disable, GT_TRUE - enable.
 *                              (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *      class5select        - Selects the indication displayed on class5 (relevant only for 
 *                            GE ports LED stream).
 *                              (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *      class13select       - Selects the indication displayed on class13 (relevant only for 
 *                            GE ports LED stream).
 *                              (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *      invertEnable        - LED data polarity
 *                            GT_TRUE - active low: LED light is in low indication.
 *                            GT_FALSE - active high: LED light is in high indication.
 *                              (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
 *      ledClockFrequency   - LED output clock frequency
 *                              (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
 * 
 */
typedef struct CPSS_LED_CONF_STCT
{
    CPSS_LED_ORDER_MODE_ENT         ledOrganize;
    GT_BOOL                         disableOnLinkDown;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blink0DutyCycle;
    CPSS_LED_BLINK_DURATION_ENT     blink0Duration;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blink1DutyCycle;
    CPSS_LED_BLINK_DURATION_ENT     blink1Duration;
    CPSS_LED_PULSE_STRETCH_ENT      pulseStretch;
    GT_U32                          ledStart;
    GT_U32                          ledEnd;
    GT_BOOL                         clkInvert;
    CPSS_LED_CLASS_5_SELECT_ENT     class5select;
    CPSS_LED_CLASS_13_SELECT_ENT    class13select;
    GT_BOOL                         invertEnable;
    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT ledClockFrequency;
} CPSS_LED_CONF_STC;

/*
 * typedef: struct CPSS_LED_CLASS_MANIPULATION_STC
 *
 * Description:
 *      LED class manipulation configuration.
 *
 * Fields:
 *      invertEnable    - Enables a NOT operation on Class data.
 *                        GT_TRUE - data inversion enabled.
 *                        GT_FALSE - data is not inverted.
 *                          (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *      blinkEnable     - Enables an AND operation on Class with selected Blink signals.
 *                        GT_TRUE - blinking enabled: High data displayed as blinking.
 *                        GT_FALSE - blinking feature not enabled.
 *      blinkSelect     - Selects between Blink 0 or Blink 1 signals.
 *      forceEnable     - Enables forcing Class data.
 *                        GT_TRUE - forceData is the data displayed.
 *                        GT_FALSE - no forcing on dispalyed data.
 *      forceData       - The data to be forced on Class data.
 *                        For GE devices: range 0x0..0xFFF (12 bits length)
 *                        For XG devices: range 0x0..0x3   (2 bits length)
 *      pulseStretchEnable
 *                      - enable the stretch affect on a specific class
 *                          (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
 *      disableOnLinkDown
 *                      - disable the LED indication classes when the link is down
 *                          (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
 */
typedef struct CPSS_LED_CLASS_MANIPULATION_STCT
{
    GT_BOOL                     invertEnable;
    GT_BOOL                     blinkEnable;
    CPSS_LED_BLINK_SELECT_ENT   blinkSelect;
    GT_BOOL                     forceEnable;
    GT_U32                      forceData;
    GT_BOOL                     pulseStretchEnable;
    GT_BOOL                     disableOnLinkDown;
} CPSS_LED_CLASS_MANIPULATION_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssLedCtrlh */


