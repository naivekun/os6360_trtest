/*******************************************************************************
*              Copyright 2003, RADLAN Computer Communications, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* GALILEO TECHNOLOGY, INC. (GTI) AND RADLAN Computer Communications, LTD.
********************************************************************************
*/
/**
********************************************************************************
* @file mvGpioIntr.h
*
* @brief Interface for using GPIO port interrupts.
*
* @version   1
********************************************************************************
*/


typedef unsigned long GT_BOOL;
typedef unsigned long GT_STATUS;
typedef unsigned char GT_U8;
typedef unsigned short GT_U16;
typedef unsigned long GT_U32;

typedef void (*GT_VOIDFUNCPTR)(void);
typedef void (*GT_VOIDINTFUNCPTR)(GT_U32);

#define IN
#define GT_FAIL -1
#define GT_OK 0
#ifndef NULL
#define NULL 0
#endif

/**
* @internal gpioIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] gpio_pin                 - pin of GPIO to enable interrupt for.
*/
void gpioIntEnable
(
    IN GT_U16   gpio_pin
);

/**
* @internal gpioIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] gpio_pin                 - pin of GPIO to disable interrupt for.
*/
void gpioIntDisable
(
    IN GT_U16   gpio_pin
);

/**
* @internal gpioInt function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
void gpioInt
(
   GT_U32 parameter
);

/**
* @internal gpioIntConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] vector                   - interrupt  number to attach to.
* @param[in] routine                  -  to be called.
* @param[in] parameter                -  to be passed to routine.
* @param[in] gpio_pin                 - pin of GPIO to connect interrupt to.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS gpioIntConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter,
   IN  GT_U16          gpio_pin
);

/**
* @internal sysGpioIntConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] vector                   - interrupt  number to attach to
* @param[in] routine                  -  to be called
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS sysGpioIntConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
);



