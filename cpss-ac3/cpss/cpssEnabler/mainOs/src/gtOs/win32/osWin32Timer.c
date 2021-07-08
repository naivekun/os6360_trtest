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
* @file osWin32Timer.c
*
* @brief Win32 Operating System Simulation. Timer facility.
*
* @version   5
********************************************************************************
*/

#include <windows.h>
#include <process.h>
#include <time.h>

#include <gtOs/gtOsTimer.h>

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
    Sleep(mils);

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
    clock_t clockT = clock();

    if(clockT==-1)
    {
        return 0;
    }

    return clockT;
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
    time_t tim_val = (time_t)timeValue;
    return (GT_U32)strftime(bufferPtr, bufferSize, format, localtime(&tim_val));
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
    static int      timer_type = 0;
    static LONGLONG frequency = 0;
    static LONGLONG start;
    static GT_U32   start_sec = 0;

    LARGE_INTEGER   now;
    time_t          t;

    if (timer_type == 0)
    {
        /* uninitialized */
        if (QueryPerformanceFrequency(&now))
        {
            /* timer supported */
            timer_type = 2; /* QueryPerformanceCounter() */
            frequency = now.QuadPart;
            QueryPerformanceCounter(&now);
            start = now.QuadPart;
            start_sec = (GT_U32)time(&t);
        } else {
            timer_type = 1; /* time() */
        }
    }
    if (timer_type == 1)
    {
        /* time() */
        if (seconds != NULL)
            *seconds = (GT_U32)time(&t);
        if (nanoSeconds != NULL)
            *nanoSeconds = 0;
        return GT_OK;
    }
    /* timer_type == 2, QueryPerformanceCounter() */
    QueryPerformanceCounter(&now);
    now.QuadPart -= start;
    if (seconds != NULL)
        *seconds = start_sec + (GT_U32)(now.QuadPart / frequency);
    if (nanoSeconds != NULL)
    {
        now.QuadPart %= frequency;
        now.QuadPart *= 1000000000;
        *nanoSeconds = (GT_U32)(now.QuadPart / frequency);
    }
    return GT_OK;
}

/**
* @internal osGetSysClockRate function
* @endinternal
*
* @brief   Get the system clock rate
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osGetSysClockRate
(
    GT_U32      *ticksPtr
)
{
    if (ticksPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    *ticksPtr = 1;
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
*/
GT_STATUS osDelay
(
    IN GT_U32 delay
)
{
    /* the minimal delay in the Windows is 1 milisecond */
    Sleep(1);

    return GT_OK;

}


