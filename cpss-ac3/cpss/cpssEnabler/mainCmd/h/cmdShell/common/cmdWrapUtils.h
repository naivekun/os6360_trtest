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
* @file cmdWrapUtils.h
*
* @brief Commander Wrappers utility function implementation
*
* @version   4
********************************************************************************
*/

#ifndef __cmdWrapUtils_h__
#define __cmdWrapUtils_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/
#include <cmdShell/common/cmdCommon.h>

/*******************************************************************************
* strDuplicate
*
* DESCRIPTION:
*       Cyclic string duplicator
*
* INPUTS:
*       src - Null terminated string
*
* OUTPUTS:
*       none
*
* RETURNS:
*       Null terminated string
*
* COMMENTS:
*
*******************************************************************************/
GT_CHAR* strDuplicate
(
    IN const GT_CHAR *src
);

/**
* @internal galtisOutput function
* @endinternal
*
* @brief   builds galtis output string
*
* @param[in] status                   -  that will be added to resultString
* @param[in] format                   - the incoming arguments  (%d%c%x%s...). The %f
*                                      can be used for adding a pre-prepared fields output
*                                      string (by using fieldOutput function)
*                                      ...             - list of arguments to put in the resultString. For %f
* @param[in] format                   no argument is needed
*
* @param[out] resultString             - the result string to write to
*                                       GT_VOID
*
* @note the output looks like: @@@status!!!o1!!!o2!!!o3!!! ... on!!!###
*
*/
GT_VOID galtisOutput
(
    OUT GT_8 *resultString,
    IN GT_STATUS status,
    IN GT_CHAR *format,
    IN ...
);

/**
* @internal fieldOutput function
* @endinternal
*
* @brief   builds table field output from input to the internal field output
*         string
* @param[in] format                   - the incoming arguments  (%d%c%x%s...)
*                                      ...     - list of arguments to put in the fields output string
*                                       GT_VOID
*
* @note the output looks like: f1???f2!!!f3!!!f4!!! ... fn???
*
*/
GT_VOID fieldOutput
(
    IN GT_CHAR* format,
    IN ...
);

/**
* @internal fieldOutputSetAppendMode function
* @endinternal
*
* @brief   causes fieldOutput to continue the output string
*/
GT_VOID fieldOutputSetAppendMode
(
    GT_VOID
);

/**
* @internal genericTableGetLast function
* @endinternal
*
* @brief   The generic table "GetLast" function.
*         Used for one-line tables as GetNext function that notifies Galtis GUI
*         that the table has no lines more.
*
* @note   APPLICABLE DEVICES:      All Devices
*
* @retval GT_OK                    - on success any case
*
* @note NONE
*
*/
CMD_STATUS genericTableGetLast
(
    IN  GT_UINTPTR inArgs[],
    IN  GT_UINTPTR inFields[],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[]
);

/**
* @internal genericDummyCommand function
* @endinternal
*
* @brief   The generic dummy command.
*         Only notifies Galtis GUI the successfull execution of the command
*         Used for tables as EndSet function that at the case when
*         all real API calls done in SetFirst and SetNext functions.
*
* @note   APPLICABLE DEVICES:      All Devices
*
* @retval GT_OK                    - on success any case
*
* @note NONE
*
*/
CMD_STATUS genericDummyCommand
(
    IN  GT_UINTPTR inArgs[],
    IN  GT_UINTPTR inFields[],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdWrapUtils_h__ */



