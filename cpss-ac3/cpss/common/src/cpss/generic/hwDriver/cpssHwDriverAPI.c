/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssHwDriverAPI.h
*
* DESCRIPTION:
*       generic HW driver APIs declaration and common functions
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
* IGNORE_CPSS_LOG_RETURN_SCRIPT
*******************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>


static CPSS_OS_MUTEX prvCpssHwDriverListMtx = 0;
static GT_BOOL prvCpssHwDriverInitialized = GT_FALSE;

typedef struct PRV_CPSS_HW_DRIVER_LIST_NODE_STCT {
    char               *name;
    CPSS_HW_DRIVER_STC *drv;
    struct PRV_CPSS_HW_DRIVER_LIST_NODE_STCT *alias;
    struct PRV_CPSS_HW_DRIVER_LIST_NODE_STCT *next;
    struct PRV_CPSS_HW_DRIVER_LIST_NODE_STCT *childs;
} PRV_CPSS_HW_DRIVER_LIST_NODE_STC;
static PRV_CPSS_HW_DRIVER_LIST_NODE_STC *cpssHwDriverListPtr = NULL;


static char *prvStrDup(const char *str)
{
    char *buf;
    buf = cpssOsMalloc(cpssOsStrlen(str)+1);
    if (buf != NULL)
        cpssOsStrCpy(buf, str);
    return buf;
}
static PRV_CPSS_HW_DRIVER_LIST_NODE_STC* prvNewNode(void)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC *node;
    node = (PRV_CPSS_HW_DRIVER_LIST_NODE_STC*)cpssOsMalloc(sizeof(*node));
    if (node != NULL)
        cpssOsMemSet(node, 0, sizeof(*node));
    return node;
}

static void prvCpssHwDriverListCheckLock(void)
{
    if (prvCpssHwDriverInitialized == GT_FALSE)
    {
        cpssOsMutexCreate("cpssHwDriverList", &prvCpssHwDriverListMtx);
        prvCpssHwDriverInitialized = GT_TRUE;
    }
    cpssOsMutexLock(prvCpssHwDriverListMtx);
}


static PRV_CPSS_HW_DRIVER_LIST_NODE_STC** prvDrvListGetNodeChildPtrPtr(CPSS_HW_DRIVER_STC *drv)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC **baseNode, *node;
    if (drv == NULL)
    {
        return &cpssHwDriverListPtr;
    }
    if (drv->parent == NULL)
    {
        baseNode = &cpssHwDriverListPtr;
    }
    else
    {
        baseNode = prvDrvListGetNodeChildPtrPtr(drv->parent);
    }
    if (baseNode == NULL)
        return NULL;
    for (node = *baseNode; node; node = node->next)
    {
        if (node->drv == drv)
            break;
    }
    if (!node)
    {
        return NULL;
    }
    return &node->childs;
}

/**
* @internal cpssHwDriverRegister function
* @endinternal
*
* @brief   Add driver instance to registry
*         The full path will be constructed as parent.path + '/' + name
* @param[in] drv                      - driver pointer
* @param[in] name                     - driver name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the parent is not registered
*                                       GT_OUT_OF_CPU_MEM
*                                       GT_ALREADY_EXIST
*/
GT_STATUS cpssHwDriverRegister(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  const char         *name
)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC **baseNode, *node;
    prvCpssHwDriverListCheckLock();

/* TODO: check for duplicate name */
    drv->name = prvStrDup(name);
    if (drv->name == NULL)
    {
        cpssOsMutexUnlock(prvCpssHwDriverListMtx);
        return GT_OUT_OF_CPU_MEM;
    }

    baseNode = prvDrvListGetNodeChildPtrPtr(drv->parent);

    if (baseNode != NULL)
    {
        node = prvNewNode();
        node->name = prvStrDup(name);
        node->drv = drv;
        while (*baseNode != NULL)
            baseNode = &((*baseNode)->next);
        *baseNode = node;
    }

    if (drv->parent)
        drv->parent->numRefs++;
    drv->numRefs = 1;
    cpssOsMutexUnlock(prvCpssHwDriverListMtx);
    return (baseNode != NULL) ? GT_OK : GT_BAD_PARAM;
}

/**
* @internal cpssHwDriverAddAlias function
* @endinternal
*
* @brief   Add alias for the driver
*
* @param[in] drv                      - driver pointer
*                                      alias       - alias path: relative ("smi0") or absolute ("/smi0")
*
* @retval GT_OK                    - on success
*                                       GT_OUT_OF_CPU_MEM
*/
GT_STATUS cpssHwDriverAddAlias(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  const char         *path
)
{
/* TODO: check for duplicate name */
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC **baseNode, *node, *driverNode;

    prvCpssHwDriverListCheckLock();

    baseNode = prvDrvListGetNodeChildPtrPtr(drv->parent);
    if (baseNode == NULL)
    {
        cpssOsMutexUnlock(prvCpssHwDriverListMtx);
        return GT_FAIL;
    }

    for (driverNode = *baseNode; driverNode; driverNode = driverNode->next)
    {
        if (driverNode->drv == drv)
            break;
    }
    if (driverNode == NULL)
    {
        cpssOsMutexUnlock(prvCpssHwDriverListMtx);
        return GT_FAIL;
    }
    node = prvNewNode();
    if (node == NULL)
    {
        cpssOsMutexUnlock(prvCpssHwDriverListMtx);
        return GT_OUT_OF_CPU_MEM;
    }
    node->alias = driverNode;
    if (path[0] == '/')
    {
        /* absolute path */
        node->name = prvStrDup(path+1);
        baseNode = &cpssHwDriverListPtr;
        while (*baseNode != NULL)
            baseNode = &((*baseNode)->next);
        *baseNode = node;
    }
    else
    {
        /* relative path */
        node->name = prvStrDup(path);
        node->next = driverNode->next;
        driverNode->next = node;
    }

    cpssOsMutexUnlock(prvCpssHwDriverListMtx);
    return GT_OK;
}

/**
* @internal cpssHwDriverDestroy function
* @endinternal
*
* @brief   Remove driver instance from registry
*         Destroy it if refCount == 0
* @param[in] drv                      - driver pointer
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssHwDriverDestroy(
    IN  CPSS_HW_DRIVER_STC *drv
)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC **baseNode, *node, *alias;

    prvCpssHwDriverListCheckLock();
    /* remove from list */
    baseNode = prvDrvListGetNodeChildPtrPtr(drv->parent);
    if (baseNode)
    {
        while (*baseNode != NULL)
        {
            if ((*baseNode)->drv == drv)
                break;
            baseNode = &((*baseNode)->next);
        }
        if (*baseNode)
        {
            node = *baseNode;
            *baseNode = node->next;
            while (node->next != NULL && node->next->alias == node)
            {
                alias = node->next;
                node->next = alias->next;
                *baseNode = alias->next;
                cpssOsFree(alias->name);
                cpssOsFree(alias);
            }
            cpssOsFree(node->name);
            cpssOsFree(node);
            /* delete absolute aliases */
            baseNode = &cpssHwDriverListPtr;
            while (*baseNode != NULL)
            {
                alias = *baseNode;
                if (alias->alias == node)
                {
                    *baseNode = alias->next;
                    cpssOsFree(alias->name);
                    cpssOsFree(alias);
                }
                else
                    baseNode = &((*baseNode)->next);
            }
        }
    }
    cpssOsMutexUnlock(prvCpssHwDriverListMtx);

    if (drv->numRefs)
        drv->numRefs--;
    if (drv->numRefs == 0)
    {
        if (drv->parent)
        {
            if (drv->parent->numRefs)
                drv->parent->numRefs--;
            if (drv->parent->numRefs == 0)
            {
                cpssHwDriverDestroy(drv->parent);
            }
        }
        if (drv->name)
            cpssOsFree(drv->name);
        drv->name = NULL;
        if (drv->destroy)
            drv->destroy(drv);
    }
    return GT_OK;
}

static CPSS_HW_DRIVER_STC* prvCpssHwDriverLookup(
    IN  PRV_CPSS_HW_DRIVER_LIST_NODE_STC *node,
    IN  const char *path
)
{
    GT_U32 stepCount = 0; /* to prevent loops */
    while (node && stepCount < 100)
    {
        GT_U32 nameLen;
        stepCount++;
        nameLen = cpssOsStrlen(node->name);
        if (cpssOsStrNCmp(node->name, path, nameLen) != 0)
        {
            node = node->next;
            continue;
        }
        if (path[nameLen] != 0 && path[nameLen] != '/')
        {
            node = node->next;
            continue;
        }
        if (node->alias)
            node = node->alias;
        if (path[nameLen] == 0)
            return node->drv;
        node = node->childs;
        path = path + nameLen+1;
    }
    return NULL;
}

/*******************************************************************************
* cpssHwDriverLookup
*
* DESCRIPTION:
*       Lookup the driver in registry using full path
*
* INPUTS:
*       path        - registry path
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Driver pointer or NULL
*
* COMMENTS:
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* cpssHwDriverLookup(
    IN  const char *path
)
{
    CPSS_HW_DRIVER_STC *drv;
    if (path[0] != '/')
        return NULL;
    prvCpssHwDriverListCheckLock();
    drv = prvCpssHwDriverLookup(cpssHwDriverListPtr, path+1);
    cpssOsMutexUnlock(prvCpssHwDriverListMtx);
    return drv;
}

/*******************************************************************************
* cpssHwDriverLookupRelative
*
* DESCRIPTION:
*       Lookup the driver in registry using relative path
*
* INPUTS:
*       parent      - the base for relative search
*       path        - registry relative path
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Driver pointer or NULL
*
* COMMENTS:
*       For example:
*       prestera_drv_path == "/PEX01:00.0"
*       smi_drv_path == "/PEX01:00.0/smi0/smi0"
*       cpssHwDriverLookupRelative(prestera_drv, "smi0/smi0");
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* cpssHwDriverLookupRelative(
    IN  CPSS_HW_DRIVER_STC *parent,
    IN  const char         *path
)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC **baseNode;
    CPSS_HW_DRIVER_STC *drv;

    prvCpssHwDriverListCheckLock();
    baseNode = prvDrvListGetNodeChildPtrPtr(parent);
    if (!baseNode || *baseNode == NULL)
    {
        cpssOsMutexUnlock(prvCpssHwDriverListMtx);
        return NULL;
    }

    drv = prvCpssHwDriverLookup(*baseNode, path);

    cpssOsMutexUnlock(prvCpssHwDriverListMtx);

    return drv;

}

/**
* @internal cpssHwDriverPrintList function
* @endinternal
*
* @brief   Print driver registry
*/
GT_STATUS cpssHwDriverPrintList(void)
{
    PRV_CPSS_HW_DRIVER_LIST_NODE_STC *c, *stack[16];
    int sp = 0, i;

    prvCpssHwDriverListCheckLock();
    c = cpssHwDriverListPtr;
    while (c)
    {
        for (i=0; i < sp; i++)
            cpssOsPrintf("/%s",stack[i]->name);
        cpssOsPrintf("/%s",c->name);
        if (c->alias)
        {
            cpssOsPrintf(" -> ");
            if (c->alias->next == c)
            {
                /* relative alias */
                cpssOsPrintf("%s", c->alias->name);
            }
            else
            {
                /* absolute alias */
                CPSS_HW_DRIVER_STC *stack1[16], *n;
                int j = 0;
                for (n = c->alias->drv; n; n = n->parent)
                    stack1[j++] = n;
                for (;j;j--)
                    cpssOsPrintf("/%s",stack1[j-1]->name);
            }
        }
        cpssOsPrintf("\n");
        if (c->childs)
        {
            stack[sp++] = c;
            c = c->childs;
            continue;
        }
        c = c->next;
        while (c == NULL && sp)
        {
            c = stack[--sp]->next;
        }
    }

    cpssOsMutexUnlock(prvCpssHwDriverListMtx);
    return GT_OK;
}

#if 0
static CPSS_HW_DRIVER_STC a, b, c, d, e;

GT_STATUS tryHwDriver(void)
{
    CPSS_HW_DRIVER_STC *n;
    cpssOsMemSet(&a, 0, sizeof(a));
    cpssOsMemSet(&b, 0, sizeof(a));
    cpssOsMemSet(&c, 0, sizeof(a));
    cpssOsMemSet(&d, 0, sizeof(a));
    cpssOsMemSet(&e, 0, sizeof(a));

    cpssHwDriverRegister(&a, "PEX00:01.0");
    cpssHwDriverRegister(&b, "PEX01:02.0");
    c.parent=&a;
    cpssHwDriverRegister(&c, "smi_c");
    cpssHwDriverAddAlias(&c, "smi0");
    cpssHwDriverAddAlias(&c, "/smi0");
    d.parent=&a;
    cpssHwDriverRegister(&d, "smi_d");
    e.parent=&c;
    cpssHwDriverRegister(&e, "smi_e");
    cpssHwDriverAddAlias(&e, "smi0");

    cpssHwDriverList();

    n = cpssHwDriverLookup("fdkshgds");
    n = cpssHwDriverLookup("/PEX00:01.0/smi0");
    n = cpssHwDriverLookup("/smi0");
    n = cpssHwDriverLookupRelative(&a, "smi0");
    n = cpssHwDriverLookupRelative(&a, "smi0/smi0");
    n = cpssHwDriverLookupRelative(NULL, "PEX00:01.0");

    cpssHwDriverDestroy(&c);
    cpssHwDriverDestroy(&e);

    cpssHwDriverList();

    return GT_OK;
}
#endif

