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
* @file appDemoStaticDb.c
*
* @brief App demo database.
*
* @version   1
********************************************************************************
*/

#include <appDemo/sysHwConfig/appDemoDb.h>

/* app demo databse */
static APP_DEMO_DB_ENTRY_STC    appDemoStaticDb[APP_DEMO_DB_MAX_SIZE_CNS];
static GT_U32                   appDemoStaticDbSize = 0;

/**
* @internal appDemoStaticDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appDemoStaticDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
    GT_U32 i;

    /* check parameters */
    if (namePtr == NULL)
        return GT_BAD_PTR;

    /* check database is not full */
    if (appDemoStaticDbSize >= APP_DEMO_DB_MAX_SIZE_CNS)
        return GT_NO_RESOURCE;

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > APP_DEMO_DB_NAME_MAX_LEN_CNS)
        return GT_BAD_PARAM;

    /* search if the name already exists, if so override the value */
    for (i = 0 ; i < appDemoStaticDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDemoStaticDb[i].name) == 0)
        {
            appDemoStaticDb[i].value = value;
            return GT_OK;
        }
    }

    /* the entry wasn't already in database, add it */
    cpssOsStrCpy(appDemoStaticDb[appDemoStaticDbSize].name, namePtr);
    appDemoStaticDb[appDemoStaticDbSize].value = value;
    appDemoStaticDbSize++;

    return GT_OK;
}


/**
* @internal appDemoStaticDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appDemoStaticDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    GT_U32 i;

    /* check parameters */
    if ((namePtr == NULL) || (valuePtr == NULL))
        return GT_BAD_PTR;

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > APP_DEMO_DB_NAME_MAX_LEN_CNS)
        return GT_BAD_PARAM;

    /* search the name in the databse */
    for (i = 0 ; i < appDemoStaticDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDemoStaticDb[i].name) == 0)
        {
            *valuePtr = appDemoStaticDb[i].value;
            return GT_OK;
        }
    }

    /* the entry wasn't found */
    return GT_NO_SUCH;
}


/**
* @internal appDemoStaticDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo database to console.
*         This DB is not reset by the 'system reset' mechanism.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoStaticDbDump
(
    GT_VOID
)
{
    GT_U32 i;

    /* print header */
    cpssOsPrintf("App Demo Static Database Dump:\n");
    cpssOsPrintf("");

    /* check if database is empty */
    if (appDemoStaticDbSize == 0)
    {
        cpssOsPrintf("App Demo Static Database is empty\n");
        return GT_OK;
    }

    /* print database values */
    for (i = 0 ; i < appDemoStaticDbSize ; i++)
    {
        cpssOsPrintf("[%3d] %s = %d\n", i+1, appDemoStaticDb[i].name, appDemoStaticDb[i].value);
    }

    return GT_OK;
}



