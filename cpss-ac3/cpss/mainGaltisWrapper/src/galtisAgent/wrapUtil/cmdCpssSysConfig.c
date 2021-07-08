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
* @file cmdCpssSysConfig.c
*
* @brief System configuration and initialization control.
*
* @version   67
********************************************************************************
*/

#include <cmdShell/common/cmdCommon.h>

#include <galtisAgent/wrapUtil/cmdCpssSysConfig.h>

#include <cmdShell/shell/cmdMain.h>

#include <galtisAgent/wrapUtil/cmdCpssLinkedList.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/version/gtVersion.h>

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID myCmdOsFree
(
    IN GT_VOID* const memblock
)
{
    cmdOsFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/



/*******************************************************************************
* internal definitions
*******************************************************************************/
#ifdef PRESTERA_DEBUG
#undef CMD_INIT_DEBUG
#define CMD_INIT_DEBUG
#endif

#ifdef CMD_INIT_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

extern GT_BOOL systemInitialized;

/* sysConfLl    - Sysconf linked list head.                         */
static  STRUCT_LL_HEAD   *sysConfLl = NULL;
static GT_UINTPTR        sysConfLlIterator;

/*******************************************************************************
* Forward function declaration
*******************************************************************************/
static GT_STATUS cmdBuildSysConf(GT_VOID);


#if (defined ASIC_SIMULATION) && (! (defined RTOS_ON_SIM))
    #define USE_WIN_SYS_CONF_OUT_STR_CNS
    GT_CHAR               winSysconfOutStr[100];
#endif /* (defined ASIC_SIMULATION) && (! (defined RTOS_ON_SIM)) */


/* tool to represent the Id of device, yet not supported by Galtis as another */
static GT_U32 repDevIdsNum = 0;
static GT_U32 repDevIdsSrc[16];
static GT_U32 repDevIdsDst[16];

/**
* @internal cmdCpssSysDevIdAddReplaceId function
* @endinternal
*
* @brief   Adds pair unkhownId / knownId-to-replace.
*
* @param[in] srcId                    - unkhownId
* @param[in] dstId                    - knownId-to-replace
*                                       None.
*/
GT_VOID cmdCpssSysDevIdAddReplaceId
(
    IN GT_U32 srcId,
    IN GT_U32 dstId
)
{
    GT_U32  size = sizeof(repDevIdsSrc) / sizeof(repDevIdsSrc[0]);

    if (repDevIdsNum >= size)
    {
        cmdOsPrintf("cmdCpssSysDevIdAddReplaceId: All %d entries used, cannot add\n",
                 size);
        return;
    }

    repDevIdsSrc[repDevIdsNum] = srcId;
    repDevIdsDst[repDevIdsNum] = dstId;

    repDevIdsNum++;
}

/**
* @internal cmdCpssSysDevIdResetReplaceId function
* @endinternal
*
* @brief   Drops all pairs unkhownId / knownId-to-replace.
*/
GT_VOID cmdCpssSysDevIdResetReplaceId
(
    GT_VOID
)
{
    repDevIdsNum = 0;
}

/**
* @internal mngInitSystem function
* @endinternal
*
* @brief   Initialize the system according to the pp configuration paramters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 
*       GalTis:
*       Command - mngInitSystem
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngInitSystem(GT_VOID)
{
    if (cmdAppIsSystemInitialized() == GT_FALSE)
    {
        return GT_NOT_INITIALIZED;
    }

    /* Prepare the sysconf linked list.             */
    if(cmdBuildSysConf() != GT_OK)
        cmdOsPrintf("@@@0!!!%x!!!###",-1);
    else
        cmdOsPrintf("@@@0!!!###");

    return GT_OK;
}


/**
* @internal mngGetSysFirstElement function
* @endinternal
*
* @brief   Returns the first element of the sysconf.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 
*       Galtis:
*       Command - mngGetSysFirstElement
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngGetSysFirstElement(GT_VOID)
{
    CMD_SYSCONF_INFO    *sysConfInfo;

    sysConfLlIterator = 0;
    sysConfInfo = llGetNext(sysConfLl,&sysConfLlIterator);
    if(sysConfInfo == NULL)
    {
#ifndef USE_WIN_SYS_CONF_OUT_STR_CNS
        cmdOsPrintf("@@@0!!!%x!!!###",-1);
#else  /*USE_WIN_SYS_CONF_OUT_STR_CNS*/
        cmdOsSprintf(winSysconfOutStr,"@@@0!!!%x!!!###",-1);
#endif /*USE_WIN_SYS_CONF_OUT_STR_CNS*/
        return GT_FAIL;
    }

#ifndef USE_WIN_SYS_CONF_OUT_STR_CNS
    cmdOsPrintf("@@@0!!!%x!!!%s!!!%s!!!%x!!!%x!!!%x!!!%x!!!%x!!!###",
             sysConfInfo->id,sysConfInfo->revision,sysConfInfo->subRev,
             sysConfInfo->deviceId,sysConfInfo->fatherId,
             sysConfInfo->fatherType,sysConfInfo->sonIndex,
             sysConfInfo->sonType);

#else /* USE_WIN_SYS_CONF_OUT_STR_CNS*/
    cmdOsSprintf(winSysconfOutStr,
              "@@@0!!!%x!!!%s!!!%s!!!%x!!!%x!!!%x!!!%x!!!%x!!!###",
              sysConfInfo->id,sysConfInfo->revision,sysConfInfo->subRev,
              sysConfInfo->deviceId,sysConfInfo->fatherId,
              sysConfInfo->fatherType,sysConfInfo->sonIndex,
              sysConfInfo->sonType);
#endif /*  USE_WIN_SYS_CONF_OUT_STR_CNS */

    return GT_OK;
}

/**
* @internal mngGetSysNextElement function
* @endinternal
*
* @brief   Returns the next element of the sysconf.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 
*       Galtis:
*       Command - mngGetSysNextElement
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS mngGetSysNextElement(GT_VOID)
{
    CMD_SYSCONF_INFO    *sysConfInfo;
    GT_U32               devId;

    sysConfInfo = llGetNext(sysConfLl,&sysConfLlIterator);
    if(sysConfInfo == NULL)
    {
#ifndef USE_WIN_SYS_CONF_OUT_STR_CNS
        cmdOsPrintf("@@@0!!!%x!!!###",-1);
#else /*USE_WIN_SYS_CONF_OUT_STR_CNS*/
        cmdOsSprintf(winSysconfOutStr,"@@@0!!!%x!!!###",-1);
#endif /*USE_WIN_SYS_CONF_OUT_STR_CNS*/
        return GT_FAIL;
    }

    devId = sysConfInfo->deviceId;
    switch(sysConfInfo->deviceId)
    {
        case 0x0d0411ab:
            /* support managed salsa */
            devId = 0x0d0011ab;
        break;
        case CPSS_98CX8296_CNS:
        case CPSS_98CX8297_CNS:
        case CPSS_98CX8308_CNS:
        case CPSS_98CX8297_1_CNS:
        case CPSS_98CX8121_CNS:
        case CPSS_98CX8123_CNS:
        case CPSS_98CX8124_CNS:
        case CPSS_98CX8129_CNS:
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
        case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
            devId = 0xE01F11AB /* Lion device CPSS_98CX8248_1_CNS*/;
            break;

        default:
            break;
    }

#ifndef USE_WIN_SYS_CONF_OUT_STR_CNS
    cmdOsPrintf("@@@0!!!%x!!!%s!!!%s!!!%x!!!%x!!!%x!!!%x!!!%x!!!###",
             sysConfInfo->id,sysConfInfo->revision,sysConfInfo->subRev,
             devId,sysConfInfo->fatherId,
             sysConfInfo->fatherType,sysConfInfo->sonIndex,
             sysConfInfo->sonType);

#else/*USE_WIN_SYS_CONF_OUT_STR_CNS*/

    cmdOsSprintf(winSysconfOutStr,
              "@@@0!!!%x!!!%s!!!%s!!!%x!!!%x!!!%x!!!%x!!!%x!!!###",
              sysConfInfo->id,sysConfInfo->revision,sysConfInfo->subRev,
              devId,sysConfInfo->fatherId,
              sysConfInfo->fatherType,sysConfInfo->sonIndex,
              sysConfInfo->sonType);
#endif /* USE_WIN_SYS_CONF_OUT_STR_CNS */

    return GT_OK;
}


/*******************************************************************************
 * Internal functions
 ******************************************************************************/
/**
* @internal sysConfLlCompareFunc function
* @endinternal
*
* @brief   The sysconf linked list compare function.
*/
static GT_LL_COMP_RES sysConfLlCompareFunc
(
    IN  CMD_SYSCONF_INFO    *data1,
    IN  CMD_SYSCONF_INFO    *data2
)
{
    if((data1 == NULL) && (data2 == NULL))
        return GT_LL_EQUAL;
    if((data1 == NULL) && (data2 != NULL))
        return GT_LL_SMALLER;
    if((data1 != NULL) && (data2 == NULL))
        return GT_LL_GREATER;

    return GT_LL_SMALLER;
}


/**
* @internal addElement2Sysconf function
* @endinternal
*
* @brief   This function adds a new element to the sysconf linked-list.
*
* @param[in] sysconfElement           - The sysconf element data to add.
*
* @retval GT_OK                    -  on success,
* @retval GT_FAIL                  -  otherwise.
*/
static GT_STATUS addElement2Sysconf
(
    IN  CMD_SYSCONF_INFO    *sysconfElement
)
{
    CMD_SYSCONF_INFO    *pElement;

    if((pElement=(CMD_SYSCONF_INFO*)cmdOsMalloc(sizeof(CMD_SYSCONF_INFO))) == NULL)
        return GT_FAIL;

    cmdOsMemCpy(pElement,sysconfElement,sizeof(CMD_SYSCONF_INFO));
    return llAdd(sysConfLl,pElement);
}

/**
* @internal cmdBuildSysConf function
* @endinternal
*
* @brief   Builds the sysconf linked-list.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cmdBuildSysConf(GT_VOID)
{
    GT_STATUS           rc;
    GT_U8               unitsNum;                   /* units in system.     */
    CMD_SYSCONF_INFO    sysConfData;
    GT_VERSION          version;                    /* Holds version info.  */
    GT_U32              phyId = 0;                      /* Holds the phyId.     */
    GT_U8               i,j;

    GT_U8   fakePortsNum;
    CMD_APP_PP_CONFIG   ppConfig;

    if(sysConfLl != NULL)
    {
        /* Was already initialized                  */
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
        llDestroy(sysConfLl,myCmdOsFree);
#else /*! OS_MALLOC_MEMORY_LEAKAGE_DBG*/
        llDestroy(sysConfLl,cmdOsFree);
#endif /*! OS_MALLOC_MEMORY_LEAKAGE_DBG*/


        sysConfLl = NULL;
    }

    /* Initialize the sysconf linked-list if needed */
    if((sysConfLl = llInit((GT_INTFUNCPTR)sysConfLlCompareFunc,
                           GT_FALSE)) == NULL)
    {
        return GT_FAIL;
    }

    version.version[0] = '1';
    version.version[1] = 0;

    unitsNum = 1;

    /* Add element for Tapi                         */
    sysConfData.id          = 0;
    cmdOsMemCpy(sysConfData.revision,version.version,cmdOsStrlen((GT_CHAR*)version.version)+1);
    cmdOsMemCpy(sysConfData.subRev,"0",cmdOsStrlen("0")+1);
    sysConfData.deviceId    = TAPI_TYPE;
    sysConfData.fatherId    = 0xFFFFFFFF;
    sysConfData.fatherType  = 0xFFFFFFFF;
    sysConfData.sonIndex    = 0;
    sysConfData.sonType     = 0;
    if(addElement2Sysconf(&sysConfData) != GT_OK)
    {
        return GT_FAIL;
    }

    /* Add element for each unit in system          */
    for(i = 0; i < unitsNum; i++)
    {
        cmdOsMemCpy(sysConfData.revision,version.version,
                 cmdOsStrlen((GT_CHAR*)version.version)+1);
        cmdOsMemCpy(sysConfData.subRev,"0",cmdOsStrlen("0")+1);
        sysConfData.deviceId    = UNIT_TYPE;
        sysConfData.fatherId    = TAPI_ID;
        sysConfData.fatherType  = TAPI_TYPE_ID;
        sysConfData.sonIndex    = i;
        sysConfData.sonType     = UNIT_SON_TYPE;
        if(addElement2Sysconf(&sysConfData) != GT_OK)
        {
            return GT_FAIL;
        }
    }

 /*   gtAppDemoDevsMapDbGetEntry(&fakeDevsNum);*/

    i=0; /*Currently we support ONLY 1 device */

    /*fakePortsNum = ppConfigList[i].numOfPorts;*/
    fakePortsNum = PRV_CPSS_PP_MAC(i)->numOfVirtPorts;

    /* Add element for each device in system    */
    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        rc = cmdAppPpConfigGet(i,&ppConfig);
        if(rc != GT_OK || ppConfig.valid/*active*/ == GT_FALSE)
            continue;
        sysConfData.id          = ppConfig.devNum;
        cmdOsMemCpy(sysConfData.revision,"0",cmdOsStrlen("0")+1);
        cmdOsMemCpy(sysConfData.subRev,"0",cmdOsStrlen("0")+1);
        sysConfData.deviceId    = ppConfig.deviceId;
        sysConfData.fatherId    = 0;    /*ppConfigList[i].unitNum;*/
        sysConfData.fatherType  = UNIT_TYPE_ID;
        sysConfData.sonIndex    = 0;/*ppConfigList[i].devNum;*/
        sysConfData.sonType     = PP_SON_TYPE;

        /* change the device Id by another, supported by Galtis */
        {
            GT_U32 ii;

            for (ii = 0; (ii < repDevIdsNum); ii++)
            {
                if (sysConfData.deviceId == repDevIdsSrc[ii])
                {
                    sysConfData.deviceId = repDevIdsDst[ii];
                }
            }
        }

        if(addElement2Sysconf(&sysConfData) != GT_OK)
        {
            return GT_FAIL;
        }

        /* Add element for each phy of the device       */
        for(j = 0; j < fakePortsNum; j++)
        {

            #if defined (ASIC_SIMULATION)
                phyId = 0x01410C10;
            #endif

            if(IS_MARVELL_PHY(phyId))
            {
                sysConfData.id          = j;
                cmdOsMemCpy(sysConfData.revision,"0",cmdOsStrlen("0") + 1);
                cmdOsMemCpy(sysConfData.subRev,"0",cmdOsStrlen("0") + 1);
                sysConfData.deviceId    = GET_PHY_ID(phyId);
                sysConfData.fatherId    = ppConfig.devNum;
                sysConfData.fatherType  = PP_TYPE_ID;
                sysConfData.sonIndex    = j;
                sysConfData.sonType     = PHY_SON_TYPE;
                if(addElement2Sysconf(&sysConfData) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal sys function
* @endinternal
*
* @brief   Prints the sysconf elements.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 
*       Galtis:
*       Command - sys
*       Toolkit:
*       Interface - <prestera/tapi/sysConfig/commands.api>
*
*/
GT_STATUS sys(GT_VOID)
{
    CMD_SYSCONF_INFO    *sysConfInfo;
    GT_UINTPTR          llIterator = 0;

    if(sysConfLl == NULL)
    {
        cmdOsPrintf("Empty list.\n");
        return GT_OK;
    }

    while((sysConfInfo = llGetNext(sysConfLl,&llIterator)) != NULL)
    {
        cmdOsPrintf("id = %d\n  typeId  = 0x%x\n  revision  = %s\n  subRev =%s\n",
                 sysConfInfo->id,sysConfInfo->deviceId,sysConfInfo->revision,
                 sysConfInfo->subRev);
        cmdOsPrintf("  fatherid = %d \n  fatherType = %d \n  sonIndex = %d \n",
                 sysConfInfo->fatherId,sysConfInfo->fatherType,
                 sysConfInfo->sonIndex);
        cmdOsPrintf("  sonType = %d\n\n",sysConfInfo->sonType);
    }

    return GT_OK;
}


