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
* @file appDemoDb.c
*
* @brief App demo database.
*
* @version   3
********************************************************************************
*/

#include <appDemo/sysHwConfig/appDemoDb.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#ifdef IMPL_GALTIS
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#endif /*IMPL_GALTIS*/

/* app demo databse */
static APP_DEMO_DB_ENTRY_STC    appDemoDb[APP_DEMO_DB_MAX_SIZE_CNS];
static GT_U32                   appDemoDbSize = 0;
static GT_U32                   initDone = 0;


#ifdef IMPL_GALTIS
/**
* @internal cmdLibResetAppDemoDb function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetAppDemoDb
(
    GT_VOID
)
{
    /* must not reset the DB ... to keep runtime parameters for next run */
}
#endif /*IMPL_GALTIS*/

/**
* @internal appDemoDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
#ifdef IMPL_GALTIS
    GT_STATUS   rc;
#endif /*IMPL_GALTIS*/
    GT_U32 i;

    if(initDone == 0)
    {
#ifdef IMPL_GALTIS
        /* register function to reset DB during system reset */
        rc = wrCpssRegisterResetCb(cmdLibResetAppDemoDb);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif /*IMPL_GALTIS*/
        initDone = 1;
    }

    /* check parameters */
    if (namePtr == NULL)
        return GT_BAD_PTR;

    /* check database is not full */
    if (appDemoDbSize >= APP_DEMO_DB_MAX_SIZE_CNS)
        return GT_NO_RESOURCE;

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > APP_DEMO_DB_NAME_MAX_LEN_CNS)
        return GT_BAD_PARAM;

    /* search if the name already exists, if so override the value */
    for (i = 0 ; i < appDemoDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDemoDb[i].name) == 0)
        {
            appDemoDb[i].value = value;
            return GT_OK;
        }
    }

    /* the entry wasn't already in database, add it */
    cpssOsStrCpy(appDemoDb[appDemoDbSize].name, namePtr);
    appDemoDb[appDemoDbSize].value = value;
    appDemoDbSize++;

    return GT_OK;
}


/**
* @internal appDemoDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appDemoDbEntryGet
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
    for (i = 0 ; i < appDemoDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDemoDb[i].name) == 0)
        {
            *valuePtr = appDemoDb[i].value;
            return GT_OK;
        }
    }

    /* the entry wasn't found */
    return GT_NO_SUCH;
}


/**
* @internal appDemoDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo database to console.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDbDump
(
    GT_VOID
)
{
    GT_U32 i;

    /* print header */
    cpssOsPrintf("App Demo Database Dump:\n");
    cpssOsPrintf("");

    /* check if database is empty */
    if (appDemoDbSize == 0)
    {
        cpssOsPrintf("App Demo Database is empty\n");
        return GT_OK;
    }

    /* print database values */
    for (i = 0 ; i < appDemoDbSize ; i++)
    {
        cpssOsPrintf("[%3d] %s = %d\n", i+1, appDemoDb[i].name, appDemoDb[i].value);
    }

    return GT_OK;
}



