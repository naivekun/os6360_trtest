/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtOsTimer.h
*
* @brief Operating System wrapper. Timer facility.
*
* @version   7
********************************************************************************
*/

#ifndef __gtOsTimerh
#define __gtOsTimerh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/**
* @internal osTimerWkAfter function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of milisecond.
*
* @param[in] mils                     - time to sleep in miliseconds
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTimerWkAfter
(
    IN GT_U32 mils
);

/**
* @internal CPSS_osTickGet function
* @endinternal
*
* @brief   Gets the value of the kernel's tick counter.
*/
#define osTickGet CPSS_osTickGet
GT_U32 CPSS_osTickGet(void);


/**
* @internal osTime function
* @endinternal
*
* @brief   Gets number of seconds passed since system boot
*/
GT_U32 osTime(void);

/**
* @internal osStrftime function
* @endinternal
*
* @brief   convert time value into a formatted string
*
* @param[in] bufferSize               - size of string buffer
* @param[in] format                   -  string, see strftime()
* @param[in] timeValue                - time value returned by osTime()
*
* @param[out] bufferPtr                - String buffer for resulting string
*                                       The second counter value.
*/
GT_U32 osStrftime(
    OUT char *  bufferPtr,
    IN  GT_U32  bufferSize,
    IN  const char* format,
    IN  GT_U32  timeValue
);

/**
* @internal osTimeRT function
* @endinternal
*
* @brief   Get the current time with nanoseconds
*
* @param[out] seconds                  - elapsed time in seconds
* @param[out] nanoSeconds              - elapsed time within a second
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);

/**
* @internal osGetSysClockRate function
* @endinternal
*
* @brief   Get the system clock rate
*
* @param[out] ticks                    - The number of  per second of the system clock.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osGetSysClockRate
(
    OUT GT_U32  *ticks
);

/**
* @internal osDelay function
* @endinternal
*
* @brief   System Delay nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDelay
(
    IN GT_U32 nanosec
);


#ifdef __cplusplus
}
#endif

#endif  /* __gtOsTimerh */
/* Do Not Add Anything Below This Line */




