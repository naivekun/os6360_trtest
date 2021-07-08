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
* @file osFreeBsdTimer.c
*
* @brief FreeBsd User Mode Operating System wrapper. Queue facility.
*
* @version   8
********************************************************************************
*/

#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <gtOs/gtOsTimer.h>

#ifndef CPSS_CLOCK
#define CPSS_CLOCK CLOCK_REALTIME
#endif

/************ Public Functions ************************************************/

/**
* @internal osTickGet function
* @endinternal
*
* @brief   Gets the value of the kernel's tick counter.
*/
GT_U32 osTickGet(void)
{
    struct tms tp;
    return (GT_U32)(times(&tp));
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
    struct tm tm_val;
    time_t tim_val = (time_t)timeValue;
    return (GT_U32)strftime(bufferPtr, bufferSize, format, localtime_r(&tim_val, &tm_val));
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

    if (clock_gettime(CPSS_CLOCK, &tv) < 0)
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
#ifdef _FreeBSD
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
#endif

    return GT_OK;
}




