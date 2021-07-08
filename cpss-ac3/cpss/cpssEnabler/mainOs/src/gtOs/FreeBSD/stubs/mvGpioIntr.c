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
* @file mvGpioIntr.c
*
* @brief Interface for using GPIO port interrupts.
*
* @version   1
********************************************************************************
*/

#include "marvell.h"
#include "mvGpioIntr.h"

#ifndef OK
#define OK 0
#endif

#undef INTDEBUG

#define FIRST_GPIO_INTERRUPT_PIN	4
#define LAST_GPIO_INTERRUPT_PIN		15
#define CURRENT_INTERRUPT_PIN		10

#define NULL_VOIDINTFUNCPTR ((GT_VOIDINTFUNCPTR) 0)

/* array to set isr routines for each pin */
static GT_VOIDINTFUNCPTR gpioIsrRoutinesArray[LAST_GPIO_INTERRUPT_PIN + 1] =
{
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR,
	NULL_VOIDINTFUNCPTR
};



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
)
{
	if ((gpio_pin < FIRST_GPIO_INTERRUPT_PIN) || (gpio_pin > LAST_GPIO_INTERRUPT_PIN))
		return;

    /* Unask the interrupt */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_IMR)) |= 1 << gpio_pin;
}

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
)
{
	if ((gpio_pin < FIRST_GPIO_INTERRUPT_PIN) || (gpio_pin > LAST_GPIO_INTERRUPT_PIN))
		return;

    /* Mask the interrupt */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_IMR)) &= ~((GT_U16)(1 << gpio_pin));
}

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
)
{
    GT_U16 reason;
	GT_U16 gpio_pin = FIRST_GPIO_INTERRUPT_PIN;
    
    /* Find the reason */
    reason = *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_ISR));

	if (reason == 0)
		return;

	reason >>= FIRST_GPIO_INTERRUPT_PIN;

	/* found interrupt with highest priority to service */
    while(reason != 1)
	{
		reason >>= 1;

		gpio_pin++;
	}

    /* Acknoledge the interrupt */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_ISR)) &= ~((GT_U16)(1 << gpio_pin));
    
#ifdef INTDEBUG
    printf("\nsysGpioInt cookie 0x%08X reason 0x%04X.\n", parameter, reason);
#endif

    /* Call user callback */
    if (gpioIsrRoutinesArray[gpio_pin])
    {
        (gpioIsrRoutinesArray[gpio_pin])(parameter);
    }
}

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
	IN  GT_U16			 gpio_pin
)
{
    GT_STATUS status;

#ifdef INTDEBUG
    printf("\ngpioIntConnect vector %d.\n", vector);
#endif

	if ((gpio_pin < FIRST_GPIO_INTERRUPT_PIN) || (gpio_pin > LAST_GPIO_INTERRUPT_PIN))
		return GT_FAIL;
    
    /* Disable the interrupt */
	gpioIntDisable(gpio_pin);

    /* Define the pin as input */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_OE)) |= 1 << gpio_pin;

    /* Set Interrupt Edge Trigger Select - 1->0 */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_IER)) |= 1 << gpio_pin;

    /* Set Reset Select Register - Clear on write */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_RSR)) &= ~((GT_U16)(1 << gpio_pin));

    /* Clear the cause */
    *((GT_U16 *)(GPIO_BASE_ADRS + GPIO_ISR)) &= ~((GT_U16)(1 << gpio_pin));

#if 0
    status = intConnect((GT_VOIDFUNCPTR *) vector, gpioInt, parameter);
#else
    status = OK;
#endif
    if (status == OK)
    {
        gpioIsrRoutinesArray[gpio_pin] = (GT_VOIDINTFUNCPTR)routine;

        /* Enable the interrupt */
        gpioIntEnable(gpio_pin);

    	return GT_OK;
	}

   	return GT_FAIL;
}


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
)
{
	return gpioIntConnect(vector, routine, parameter, CURRENT_INTERRUPT_PIN);
}


