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
* @file cpssCommonLedCtrl.h
*
* @brief Includes LEDs control data structures and enumerations definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssCommonLedCtrlh
#define __cpssCommonLedCtrlh

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_LED_ORDER_MODE_ENT
 *
 * @brief LED stream ordering mode
*/
typedef enum{

    /** the indication order is arranged by port */
    CPSS_LED_ORDER_MODE_BY_PORT_E,

    /** the indication order is arranged by class */
    CPSS_LED_ORDER_MODE_BY_CLASS_E

} CPSS_LED_ORDER_MODE_ENT;

/**
* @enum CPSS_LED_BLINK_DUTY_CYCLE_ENT
 *
 * @brief Duty cycle of LED blink signal
*/
typedef enum{

    /** 25% on, 75% off */
    CPSS_LED_BLINK_DUTY_CYCLE_0_E,

    /** 50% on, 50% off */
    CPSS_LED_BLINK_DUTY_CYCLE_1_E,

    /** 50% on, 50% off */
    CPSS_LED_BLINK_DUTY_CYCLE_2_E,

    /** 75% on, 25% off */
    CPSS_LED_BLINK_DUTY_CYCLE_3_E

} CPSS_LED_BLINK_DUTY_CYCLE_ENT;

/**
* @enum CPSS_LED_BLINK_DURATION_ENT
 *
 * @brief The period of the LED Blink signal
 * (see HW spec doc for meaning of values).
*/
typedef enum{

    CPSS_LED_BLINK_DURATION_0_E,

    CPSS_LED_BLINK_DURATION_1_E,

    CPSS_LED_BLINK_DURATION_2_E,

    CPSS_LED_BLINK_DURATION_3_E,

    CPSS_LED_BLINK_DURATION_4_E,

    CPSS_LED_BLINK_DURATION_5_E,

    CPSS_LED_BLINK_DURATION_6_E,

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe) */
    CPSS_LED_BLINK_DURATION_7_E

} CPSS_LED_BLINK_DURATION_ENT;

/**
* @enum CPSS_LED_PULSE_STRETCH_ENT
 *
 * @brief The length of stretching for dynamic signals
 * (see HW spec doc for meaning of values).
*/
typedef enum{

    /** no stretching */
    CPSS_LED_PULSE_STRETCH_0_NO_E,

    CPSS_LED_PULSE_STRETCH_1_E,

    CPSS_LED_PULSE_STRETCH_2_E,

    CPSS_LED_PULSE_STRETCH_3_E,

    CPSS_LED_PULSE_STRETCH_4_E,

    CPSS_LED_PULSE_STRETCH_5_E,

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe) */
    CPSS_LED_PULSE_STRETCH_6_E,

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe) */
    CPSS_LED_PULSE_STRETCH_7_E

} CPSS_LED_PULSE_STRETCH_ENT;

/**
* @enum CPSS_LED_CLOCK_OUT_FREQUENCY_ENT
 *
 * @brief The LED clock out frequency
*/
typedef enum{

    /** 500 KHz LED clock frequency. */
    CPSS_LED_CLOCK_OUT_FREQUENCY_500_E,

    /** 1 MHz LED clock frequency. */
    CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E,

    /** 2 MHz LED clock frequency. */
    CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E,

    /** 3 MHz LED clock frequency. */
    CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E

} CPSS_LED_CLOCK_OUT_FREQUENCY_ENT;

/**
* @enum CPSS_LED_BLINK_SELECT_ENT
 *
 * @brief Blink types to select
*/
typedef enum{

    /** Blink 0 signal */
    CPSS_LED_BLINK_SELECT_0_E,

    /** Blink 1 signal */
    CPSS_LED_BLINK_SELECT_1_E

} CPSS_LED_BLINK_SELECT_ENT;

/*
 * typedef: struct CPSS_LED_GROUP_CONF_STC
 *
 * Description:
 *      LED group classes used for the combinational logic.
 *      The combinational logic = (Class A AND Class B) OR (Class C AND Class D).
 *
 * Fields:
 *      classA  - Selects which class is used as A in the group combinational logic.
 *      classB  - Selects which class is used as B in the group combinational logic. 
 *      classC  - Selects which class is used as C in the group combinational logic. 
 *      classD  - Selects which class is used as D in the group combinational logic.
 */
typedef struct CPSS_LED_GROUP_CONF_STCT
{
    GT_U32  classA;
    GT_U32  classB;
    GT_U32  classC;
    GT_U32  classD;
} CPSS_LED_GROUP_CONF_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssCommonLedCtrlh */


