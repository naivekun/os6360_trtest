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
* @file cpssOsLog.h
*
* @brief Operating System wrapper, log.
*
* @version   2
********************************************************************************
*/

#ifndef __cpssOsLogh
#define __cpssOsLogh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/log/cpssLog.h>

/*******************************************************************************
* CPSS_OS_LOG_FUNC
*
* DESCRIPTION:
*       a callback function for output of the CPSS log.
*
* INPUTS:
*       lib - lib identifier.
*       type - type of the log output.
*       format - usual printf format string.
*       ... - additional parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
* 
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_VOID (*CPSS_OS_LOG_FUNC)
(
    IN    CPSS_LOG_LIB_ENT      lib,
    OUT   CPSS_LOG_TYPE_ENT     type,
    IN    const char*           format,
    ...
);
/**
* @struct CPSS_OS_LOG_BIND_STC
 *
 * @brief Structure of "os log" funcation, needed be bound to cpss.
*/
typedef struct{

    /** @brief pointer to function that printing the logs
     *  of cpss log
     *  Comments:
     *  None.
     */
    CPSS_OS_LOG_FUNC osLogFunc;

} CPSS_OS_LOG_BIND_STC;


#ifdef __cplusplus
}
#endif

#endif  /* __cpssOsLogh */
/* Do Not Add Anything Below This Line */


