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
* @file cmdBase.h
*
* @brief commander database api header
*
* @version   7
********************************************************************************
*/

#ifndef __cmdBase_h__
#define __cmdBase_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/
#include <cmdShell/common/cmdCommon.h>
#ifdef CPSS_PRODUCT
#  include <cpss/extServices/os/gtOs/gtGenTypes.h>
#endif

/* commander compile-time configuration macros */
#define CMD_MAX_ARGS    256     /* maximum # of arguments       */
#define CMD_MAX_FIELDS  256     /* maximum # of fields          */

/*
 * Typedef: function cmdWrapperFuncPtr
 *
 * Description: definition of the prototype of all the commands wrappper
 *              functions. This prototype should be used when implementing
 *              wrapper functions.
 *
 * INPUTS:
 *      inArgs - an array of command line input parameters
 *               for numeric values it holds the value
 *               for string values it holds a pointer to the input string
 *
 * OUTPUTS:
 *      outArgs - a poiter to a string that should be field with the function
 *                Galtis output.
 */
typedef CMD_STATUS (*cmdWrapperFuncPtr)
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @struct CMD_COMMAND
 *
 * @brief definition of single command (api function) used by database
 * description tables.
*/
typedef struct{

    GT_CHAR *commandName;

    /** Pointer to the function */
    cmdWrapperFuncPtr funcReference;

    /** Number of aruments */
    GT_U32 funcArgs;

    /** Number of fields for table related commands */
    GT_U32 funcFields;

} CMD_COMMAND;

/*******************************************************************************
* cmdInitDatabase
*
* DESCRIPTION:
*       create the commands database by calling the libraries
*       initialization functions
*
* INPUTS:
*       none
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_STATUS
*
* COMMENTS:
*       none
*
*******************************************************************************/
#if (defined CPSS_PRODUCT) || (defined PSS_PRODUCT)
GT_STATUS cmdInitDatabase(GT_VOID);
#define cmdCpssInitDatabase cmdInitDatabase
#else
GT_STATUS cmdCpssInitDatabase(GT_VOID);
#endif

/**
* @internal cmdInitLibrary function
* @endinternal
*
* @brief   add library commands to the global command database
*
* @param[in] list[]                   - an array of commands data structure
* @param[in] amount                   - the number of command in the commands list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note none
*
*/
GT_STATUS cmdInitLibrary
(
    IN CMD_COMMAND list[],
    IN GT_U32 amount
);

/**
* @internal cmdGetCommand function
* @endinternal
*
* @brief   performs binary search on the cmdDatabase for finding the right
*         command entry
* @param[in] commandName              - null terminated string holding command name
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if command database not initialized.
* @retval GT_NOT_FOUND             - if the command was not found
*/
GT_STATUS cmdGetCommand
(
    IN const GT_CHAR *commandName,
    OUT CMD_COMMAND** command
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__cmdBase_h__*/


