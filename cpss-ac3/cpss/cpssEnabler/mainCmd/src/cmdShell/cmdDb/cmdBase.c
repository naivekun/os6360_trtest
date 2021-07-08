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
* @file cmdBase.c
*
* @brief API commander database functionality implementation.
*
* @version   8
********************************************************************************
*/

#include <cmdShell/cmdDb/cmdBase.h>
#include <stdlib.h>

/***** Command Registration and Initialization *************************/

#if (defined DX_FAMILY) || (defined CHX_FAMILY)
#define DX_FAMILY_NUM_OF_CMD 1224
#else
#define DX_FAMILY_NUM_OF_CMD 0
#endif

#ifdef PX_FAMILY
#define PX_FAMILY_NUM_OF_CMD 1024
#else
#define PX_FAMILY_NUM_OF_CMD 0
#endif

#define NUM_OF_CMD_COMMAND (3072 \
                                + DX_FAMILY_NUM_OF_CMD  \
                                + PX_FAMILY_NUM_OF_CMD)

static CMD_COMMAND *dbCommands[NUM_OF_CMD_COMMAND];
static GT_U32 numCommands = 0;

/*******************************************************************************
* databaseSorted
*
* DESCRIPTION:
*                Database sort is delayed.
*                This helps to avoid database sorting for each
*                cmdInitLibrary call
*******************************************************************************/
static GT_BOOL databaseSorted = GT_FALSE;

/**
* @internal qsortRecordCompareFunc function
* @endinternal
*
* @brief   Compares two CMD_COMMAND records by command name
*         Required by qsort()
* @param[in] a                        - pointer to record
* @param[in] b                        - pointer to record
*                                       integer less than, equal to, or greater than zero if
*                                       a is less than, equal to, or greater than b
*
* @note none
*
*/
static int qsortRecordCompareFunc(
        const GT_VOID* a,
        const GT_VOID* b
)
{
    return cmdOsStrCmp(
            (*((const CMD_COMMAND**)a))->commandName,
            (*((const CMD_COMMAND**)b))->commandName);
}

/**
* @internal cmdInitLibrary function
* @endinternal
*
* @brief   insert into    the command database (dbCommands) the recived list of
*         commands
* @param[in] list[]                   -  of commands each entry is function name, function
*                                      reference and number of function arguments.
* @param[in] amount                   - the number of commands to add.
*                                       GT_STATUS
*
* @note none
*
*/
GT_STATUS cmdInitLibrary
(
        IN CMD_COMMAND list[],
        IN GT_U32 amount
)
{
    unsigned int index;

    if( numCommands + amount > NUM_OF_CMD_COMMAND)
        return GT_NO_RESOURCE;

    /* store command references */
    for (index = 0; index < amount; index++)
        dbCommands[numCommands++] = &list[index];

    databaseSorted = GT_FALSE;

    return GT_OK;
}

/**
* @internal cmdGetCommand function
* @endinternal
*
* @brief   performs binery search on the cmdDatabase for finding the right cmd
*         entry
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
)
{
        GT_32 iFirst = 0;
        GT_32 iLast = numCommands;
        GT_32 iMiddle, iRet;

        /* dbCommands never has NULL value. It is pointer to static array
        if (dbCommands == NULL)
            return GT_FAIL;
        */

        if (databaseSorted == GT_FALSE)
        {
            qsort(dbCommands, numCommands, sizeof(dbCommands[0]), qsortRecordCompareFunc);
            databaseSorted = GT_TRUE;
        }

        while (iLast >= iFirst)
        {
                /*iMiddle = (iLast + iFirst) / 2;*/
                iMiddle = iFirst + (iLast - iFirst) / 2;
                if (iMiddle >= (GT_32) numCommands || iMiddle < 0)
                        return GT_NOT_FOUND;

                iRet = cmdOsStrCmp(dbCommands[iMiddle]->commandName, commandName);

                if (iRet > 0)
                        iLast = iMiddle - 1;
                else if (iRet < 0)
                        iFirst = iMiddle + 1;
                else if (iRet == 0)
                {
                        *command = dbCommands[iMiddle];
                        return GT_OK;
                }
        }

        return GT_NOT_FOUND;
}




