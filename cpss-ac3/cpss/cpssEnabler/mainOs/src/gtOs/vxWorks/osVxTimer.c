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
* @file osVxTimer.c
*
* @brief VxWorks Operating System wrapper. Timer facility.
*
* @version   6
********************************************************************************
*/

#include <vxWorks.h>
#include <time.h>
#include <tickLib.h>
#include <sysLib.h>
#include <taskLib.h>

#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsGen.h>

/************ Public Functions ************************************************/

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
)
{
    GT_U32 delay, time;

    IS_WRAPPER_OPEN_STATUS;

    time = sysClkRateGet() * mils;

    delay = (time % 1000) ? (time / 1000 + 1) : (time / 1000);

    taskDelay (delay);

    return GT_OK;
}

/**
* @internal osTickGet function
* @endinternal
*
* @brief   Gets the value of the kernel's tick counter.
*/
GT_U32 osTickGet(void)
{
    IS_WRAPPER_OPEN_PTR;
    return tickGet();
}


/**
* @internal osTime function
* @endinternal
*
* @brief   Gets number of seconds passed since system boot
*/
GT_U32 osTime(void)
{
   time_t t ;

   IS_WRAPPER_OPEN_PTR;
   return (GT_U32) time(&t);
}

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
)
{
    struct tm tm_val;
    time_t tim_val = (time_t)timeValue;
    IS_WRAPPER_OPEN_PTR;
    localtime_r(&tim_val, &tm_val);
    return (GT_U32)strftime(bufferPtr, bufferSize, format, &tm_val);
}

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
)
{
    struct timespec tv;

    if (clock_gettime(CLOCK_REALTIME, &tv) != OK)
        return GT_FAIL;

    if (seconds != NULL)
        *seconds = (GT_U32)tv.tv_sec;
    if (nanoSeconds != NULL)
        *nanoSeconds = (GT_U32)tv.tv_nsec;

    return GT_OK;
}

/**
* @internal osDelay function
* @endinternal
*
* @brief   System Delay nanoseconds.
*
* @param[in] delay                    - non operational time in nanoseconds
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The values for calculation of iterations number, achieved by tests.
*
*/
GT_STATUS osDelay
(
    IN GT_U32 delay
)
{
    volatile GT_U32 i;                /* loop iterator */
    GT_U32          iterationsNum;    /* num of iterations for busy wait */


/* FireFox CPU 88e6218 */
iterationsNum =  (delay / 10000) * 122;

#if (CPU == PPC603)
    iterationsNum = (delay / 10000) * 190;
#endif

#ifdef CPU_926EJ
  /* Orion CPU 88f5181 */
    iterationsNum =  (delay / 10000) * 362;
#endif


    for(i = 0; i < iterationsNum; i++);

    return GT_OK;

}

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
)
{
	IS_WRAPPER_OPEN_STATUS;
    *ticks = sysClkRateGet();
    return GT_OK;
}

#ifdef DEBUG_OS_TIMER
/**
* @internal timerDebug function
* @endinternal
*
* @brief   System Delay nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS timerDebug(GT_U32 delay, GT_U32 loops)
{
    GT_U32 start, stop, sysClkRate;
    GT_U32 time,ii;


#if (CPU == PPC603)
   osPrintf("inside PPC603\n");
#endif

#ifdef CPU_926EJ
  /* Orion CPU 88f5181 */
   osPrintf("inside Orion CPU 88f5181\n");
#endif

#ifdef CPU_946ES
  /* Firefox CPU 88e6828 */
   osPrintf("inside Firefox CPU 88e6828\n");
#endif

    ii = 0;
    start = tickGet ();
    for (; ii < loops; ii++)
        osDelay(delay);

    stop = tickGet ();
    sysClkRate = sysClkRateGet();
    osPrintf("ii=%d sysClkRate=%d\n", ii, sysClkRate);

    time =  (stop - start) * 1000 / sysClkRate;

    osPrintf("Ticks: = %d,  Mili Seconds: %d\n",((stop - start)), time);

    return GT_OK;


}

#endif





