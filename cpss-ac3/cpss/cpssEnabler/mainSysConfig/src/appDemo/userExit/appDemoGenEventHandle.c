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
* @file appDemoGenEventHandle.c
*
* @brief this library contains the implementation of the event handling functions
* for the Gen device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* @version   26
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#if defined CHX_FAMILY
    #define SUPPORT_AU

    #include <extUtils/auEventHandler/auFdbUpdateLock.h>
    #include <cpss/generic/networkIf/cpssGenNetIfMii.h>
    #include <extUtils/auEventHandler/auEventHandler.h>

    #include <extUtils/cpssEnabler/appDemoFdbIpv6UcLinkedList.h>

    #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>

    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>
    #include <cpss/extServices/private/prvCpssBindFunc.h>
#else
    #include <appDemo/sysHwConfig/nonSwitchStubs.h>
#endif

/****************************************************************************
* Debug                                                                     *
****************************************************************************/
#ifdef ASIC_SIMULATION
static GT_U32 appDemoMaxRetryOnSleep0 = 128;
#endif /*ASIC_SIMULATION*/

/* number of cases that the movement of old entry to other place allowed the new
    entry to take it's place */
static GT_U32 numOfSuccessesInReorder = 0;
static GT_U32 numOfFailesInReorder = 0;
/* indication to trace the first fail of adding an entry into the FDB */
/* when this flag is set to 1 , the first time that the engine can't add entry
   the engine will check how much entries in the FDB. and print in % the capacity.
*/
static GT_U32   traceFirstMiss = 0;
static GT_BOOL  isFirstMissDone = 0;
/* indication that the Cuckoo algorithm succeeded to do it's first reorder of
   entries in order to insert NA that first failed to be inserted but after the
   re order succeeded */
static GT_BOOL  isFirstReorderDone = 0;

#define BUFF_LEN    5
#define AU_SIZE      10


#ifdef IMPL_GALTIS
extern GT_STATUS cmdGenRxPktReceive
(
    IN GT_U8      devNum,
    IN GT_U8      queueIdx,
    IN GT_U32     numOfBuff,
    IN GT_U8     *packetBuffs[],
    IN GT_U32     buffLen[],
    IN void      *rxParamsPtr
);
#endif /* IMPL_GALTIS */

/* trace info for AUQ messages */
/* trace flag */
static GT_U32   traceAuq = 0;

#if (defined CHX_FAMILY)
    #define DXCH_CODE

/* trace indications */
static char* traceAuqArr[CPSS_HR_E + 1] = {
    "+",/*CPSS_NA_E*/
    "qa",/*CPSS_QA_E*//* should not be */
    "qr",/*CPSS_QR_E*/
    "-",/*CPSS_AA_E*/
    "t",/*CPSS_TA_E*/
    "sa",/*CPSS_SA_E*//* should not be */
    "qi",/*CPSS_QI_E*//* should not be */
    "f",/*CPSS_FU_E*/
    "hr" /*CPSS_HR_E*//* should not be */
};

#endif /* (defined CHX_FAMILY) */

/* The number of AU messages per type*/
GT_U32 appDemoAuMessageNum[CPSS_HR_E + 1] = {0};

/* The number of wrong type messages*/
GT_U32 appDemoAuWrongMessageNum = 0;

#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

static  CPSS_DXCH_NET_RX_PARAMS_STC     chRxParams;

#ifdef IMPL_GALTIS
GT_STATUS cpssDxChBrgFdbPortGroupCount_debug
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
);
#endif /* IMPL_GALTIS */

#define PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS 0xFFFFFFFF

/**
* @internal appDemoNetRxPktHandleChPCI function
* @endinternal
*
* @brief   application routine to receive frames . -- Gen function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*
* @note The user is responsible to free the gtBuf structures and rx data buffers.
*       The lport that needs to be transferred to the function netFreeRxGtbuf()
*       is the intLport and not srcLport.
*
*/
static GT_STATUS appDemoNetRxPktHandleChPCI
(
    IN    GT_U8      devNum,
    IN    GT_U8      queueIdx,
    INOUT GT_U32    *numOfBuffPtr,
    OUT   GT_U8     *packetBuffs[],
    OUT   GT_U32     buffLen[],
    OUT   void     **rxParams
)
{
    GT_STATUS                       rc;


    /* get the packet from the device */
    rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx, numOfBuffPtr,
                        packetBuffs, buffLen, &chRxParams);

    if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
    {
        /* no need to free buffers because not got any */
        return rc;
    }
    *rxParams = &chRxParams;


    /* call GalTis Agent for counting and capturing */
#ifdef IMPL_GALTIS
    /* ignore the return code from this function because we want to keep on
       getting RX buffers , regardless to the CMD manager */
    (void) cmdGenRxPktReceive(devNum, queueIdx, *numOfBuffPtr,
                              packetBuffs, buffLen, *rxParams);
#endif /* IMPL_GALTIS */


    /* now you need to free the buffers */
    rc = cpssDxChNetIfRxBufFree(devNum, queueIdx, packetBuffs, *numOfBuffPtr);

    return rc;
}
#endif


/*******************************************************************************
* appDemoGenNetRxPktHandle
*
* DESCRIPTION:
*       application routine to receive frames . -- Gen function
*
* INPUTS:
*       devNum      - Device number.
*       queueIdx    - The queue from which this packet was received.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*     The user is responsible to free the gtBuf structures and rx data buffers.
*     The lport that needs to be transferred to the function netFreeRxGtbuf()
*     is the intLport and not srcLport.
*
*
*******************************************************************************/
#if defined DXCH_CODE
GT_STATUS appDemoGenNetRxPktHandle
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfBuff = BUFF_LEN;
    GT_U8*                              packetBuffs[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];
    void                               *rxParams;

    /* get the packet from the device */
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        if (CPSS_IS_DXCH_FAMILY_MAC(appDemoPpConfigList[devNum].devFamily))
        {
            rc = appDemoNetRxPktHandleChPCI(devNum, queueIdx, &numOfBuff,
                                            packetBuffs, buffLenArr, &rxParams);
        }
        else
        {
            numOfBuff = 1;
            packetBuffs[0]=NULL;
            buffLenArr[0]=1;
            rxParams = NULL;
        }

        return rc;
    }
    else if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxPacketGet(devNum,queueIdx,&numOfBuff,packetBuffs,buffLenArr,&chRxParams);
        if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
        {
            /* no need to free buffers because not got any */
            return rc;
        }
        rxParams = (void*)(&chRxParams);

    /* call GalTis Agent for counting and capturing */
#ifdef IMPL_GALTIS
        /* ignore the return code from this function because we want to keep on
           getting RX buffers , regardless to the CMD manager */
        (void) cmdGenRxPktReceive(devNum,queueIdx,numOfBuff,
                                  packetBuffs,buffLenArr,rxParams);
#endif /* IMPL_GALTIS */

        rc = cpssDxChNetIfMiiRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
    }
    else
    {
        rc = GT_NOT_IMPLEMENTED;
    }

    return rc;
}
#endif /*DXCH_CODE*/


/*******************************************************************************/
/*******************************************************************************/
/********************************* AU MESSAGES HANDLE **************************/
/*******************************************************************************/
/*******************************************************************************/


#if defined DXCH_CODE
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#endif

typedef GT_STATUS fdbAuMsgBlockGetFunc(IN    GT_U8 devNum,
                                       INOUT GT_U32                      *numOfAuPtr,
                                       OUT   CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr);

typedef GT_STATUS fdbEntrySetFunc(IN GT_U8                   devNum,
                                  IN GT_U32                  messagePortGroupId,
                                  IN CPSS_MAC_ENTRY_EXT_STC *fdbEntryPtr);

typedef GT_STATUS fdbEntryDeleteFunc(IN GT_U8                        devNum,
                                     IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr);

typedef GT_STATUS fdbEntryInvalidateFunc(IN GT_U8                       devNum,
                                         IN CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType,
                                         IN GT_U32                      index);

typedef GT_STATUS fdbFromCpuAuMsgStatusGetFunc(IN  GT_U8    devNum,
                                               OUT GT_BOOL  *completedPtr,
                                               OUT GT_BOOL  *succeededPtr);

#if defined DXCH_CODE
static fdbAuMsgBlockGetFunc          *cpssEnFdbBrgFdbAuMsgBlockGet = NULL;
static fdbEntrySetFunc               *cpssEnFdbBrgFdbEntrySetFunc;
static fdbEntryDeleteFunc            *cpssEnFdbBrgFdbEntryDeleteFunc;
static fdbEntryInvalidateFunc        *cpssEnFdbBrgFdbEntryInvalidateFunc;
static fdbFromCpuAuMsgStatusGetFunc  *cpssEnFdbFromCpuAuMsgStatusGetFunc;


GT_STATUS prvDxChBrgFdbFromCpuAuMsgStatusGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  usePortGroupsBmp,
    IN GT_U32   currPortGroupsBmp
)
{
    GT_STATUS   st = GT_OK;
    GT_BOOL completed;
    GT_BOOL succeeded;
    GT_PORT_GROUPS_BMP completedBmp;
    GT_PORT_GROUPS_BMP succeededBmp;
    GT_PORT_GROUPS_BMP completedBmpSummary;
    GT_PORT_GROUPS_BMP succeededBmpSummary;
    #ifdef ASIC_SIMULATION
    GT_U32  sleepTime = 0;
    GT_U32  maxRetry = appDemoMaxRetryOnSleep0;
    GT_U32  max_get_tries = 10000;
    #endif/*ASIC_SIMULATION*/

    if(usePortGroupsBmp == GT_FALSE)
    {
        completed = GT_FALSE;
        succeeded = GT_FALSE;

        do{
            #ifdef ASIC_SIMULATION
            if(max_get_tries-- == 0){
                succeeded = GT_FALSE;
                break;
            }
            if ((sleepTime == 0) && maxRetry)
            {
                /* do max retries with 'busy wait' but with option to other tasks
                   to also run --> reduce run time */
                maxRetry--;
            }
            else if (maxRetry == 0)
            {
                sleepTime = 1;
            }
            osTimerWkAfter(sleepTime);
            #endif /*ASIC_SIMULATION*/
            st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(devNum, &completed, &succeeded,NULL);
            if(st != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        } while (completed == GT_FALSE);

        if(succeeded == GT_FALSE)
        {
            /* operation finished but not succeeded */
            st = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }
    else
    {
        completedBmpSummary = 0;
        succeededBmpSummary = 0;
        do{
            completedBmp = 0;
            succeededBmp = 0;
            #ifdef ASIC_SIMULATION
            if ((sleepTime == 0) && maxRetry)
            {
                /* do max retries with 'busy wait' but with option to other tasks
                   to also run --> reduce run time */
                maxRetry--;
            }
            else if (maxRetry == 0)
            {
                sleepTime = 1;
            }
            osTimerWkAfter(sleepTime);
            #endif /*ASIC_SIMULATION*/
            st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(devNum, currPortGroupsBmp, &completedBmp, &succeededBmp);
            if(st != GT_OK)
            {
                goto exit_cleanly_lbl;
            }

            completedBmpSummary |= completedBmp;
            succeededBmpSummary |= succeededBmp;
        }while (
            (completedBmpSummary & PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp) !=
            (currPortGroupsBmp & PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp));

        if(completedBmpSummary != succeededBmpSummary)
        {
            /* operation finished but not succeeded on all port groups */
            st = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    return st;
}

/* function to be called when adding new entry failed */
GT_STATUS   prvDxChBrgFdbMacEntryNewFailed
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
{
    GT_STATUS rc;
    GT_U32    needStatistics = 0;

    macEntryPtr = macEntryPtr; /* avoid : warning C4100: 'macEntryPtr' : unreferenced formal parameter */

    /* indicate that the bucket is FULL */
    rc = GT_FULL;
    numOfFailesInReorder++;

    if(traceAuq)
    {
        /* put visual indication that NA failed */
        osPrintf("F"/*FAILED*/);/* should be followed by "+" due to traceAuqArr[CPSS_NA_E]*/
    }

    if(isFirstMissDone == 0)
    {
        isFirstMissDone = 1;
        if (traceFirstMiss)
        {
            osPrintf("NA processing: First Miss detected \n");
            needStatistics = 1;
        }
    }

    if(needStatistics)
    {
#ifdef IMPL_GALTIS
        /* call the debug function to print capacity of FDB */
        /* this actually print info from 'first port group' */
        (void) cpssDxChBrgFdbPortGroupCount_debug(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
#endif /* IMPL_GALTIS */
    }

    return rc;
}


/* 'mac set' entry for multi port groups device */
GT_STATUS prvDxChBrgFdbPortGroupMacEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U32                       messagePortGroupId,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
{
    GT_STATUS   rc;
    GT_PORT_GROUPS_BMP  deletedPortGroupsBmp;  /* bitmap of Port Groups - to delete the entry from */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* bitmap of Port Groups */
    GT_U32              devIdx; /* index to appDemoPpConfigList */

    rc = appDemoDevIdxGet(devNum, &devIdx);
    if ( GT_OK != rc )
    {
        return rc;
    }

    if(appDemoPpConfigList[devIdx].numOfPortGroups <= 1)
    {
        rc = cpssDxChBrgFdbMacEntrySet(devNum,macEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc =  prvDxChBrgFdbFromCpuAuMsgStatusGet(devNum,GT_FALSE,0);

        if(rc == GT_FAIL)
        {
            /*adding new entry failed - check to see if we can do something about it.
              or just to 'trace' this fail */
            rc = prvDxChBrgFdbMacEntryNewFailed(devNum,macEntryPtr);
        }

        return rc;
    }

    if(appDemoPpConfigList[devIdx].fdbMode ==
        APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E)
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        /* do NOT care if entry is :
            1. port or trunk
            2. or relate to local/remote device
           because this is ' controlled learn' that actually emulates 'auto learn'.
           so when message come from one of the port groups we add the entry
           to all port groups that associated with this src port group (unifiedFdbPortGroupsBmp)
        */
        if(messagePortGroupId < appDemoPpConfigList[devIdx].numOfPortGroups)
        {
            portGroupsBmp =
                appDemoPpConfigList[devIdx].portGroupsInfo[messagePortGroupId].unifiedFdbPortGroupsBmp;
        }
        else
        {
            /* error -- should not happen */
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
    }

    rc =  cpssDxChBrgFdbPortGroupMacEntrySet(devNum,portGroupsBmp,macEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        rc = prvDxChBrgFdbFromCpuAuMsgStatusGet(devNum,GT_FALSE,0);
        if(rc == GT_FAIL)
        {
            /*adding new entry failed - check to see if we can do something about it.
              or just to 'trace' this fail */
            /*adding new entry failed*/
            rc = prvDxChBrgFdbMacEntryNewFailed(devNum,macEntryPtr);
        }

        return rc;
    }

    /* remove from the active port groups the port groups that we added the entry
       to them */
    deletedPortGroupsBmp =
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (~portGroupsBmp);

    /* we need to delete the mac entry from other port groups that it may be in
       (station movement) */
    rc = cpssDxChBrgFdbPortGroupMacEntryDelete(devNum,deletedPortGroupsBmp,&macEntryPtr->key);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* wait for finish on deleted port groups , but not require success ,
       because mac not necessarily exists there */
    rc =  prvDxChBrgFdbFromCpuAuMsgStatusGet(devNum,GT_TRUE,deletedPortGroupsBmp);
    if(rc != GT_OK)
    {
        /* this is not error !!!! */
        rc = GT_OK;
    }

    /* wait for finish and success on added port groups to see that that added
       entry succeeded */
    rc = prvDxChBrgFdbFromCpuAuMsgStatusGet(devNum,GT_TRUE,portGroupsBmp);

    return rc;
}

GT_STATUS prvDxChBrgFdbMacEntryDelete
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
{
    GT_STATUS   rc;
    GT_U32      devIdx; /* index to appDemoPpConfigList */

    rc = cpssDxChBrgFdbMacEntryDelete(devNum,macEntryKeyPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvDxChBrgFdbFromCpuAuMsgStatusGet(devNum,GT_FALSE,0);

    if(rc != GT_OK)
    {
        rc = appDemoDevIdxGet(devNum, &devIdx);
        if ( GT_OK != rc )
        {
            return rc;
        }

        if(appDemoPpConfigList[devIdx].numOfPortGroups == 0 ||
           (appDemoPpConfigList[devIdx].fdbMode ==
            APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E))
        {
            return rc;
        }

        /* wait for finish on deleted port groups , but not require success ,
           because mac not necessarily exists there */

        /* this is not error !!!! */
        rc = GT_OK;
    }

    return rc;
}

GT_STATUS prvDxChBrgFdbMacEntryInvalidate
(
    IN  GT_U8                           devNum,
    IN  CPSS_MAC_ENTRY_EXT_TYPE_ENT     entryType,
    IN  GT_U32                          index
)
{
    GT_STATUS rc=GT_OK;
    GT_BOOL   valid;
    GT_BOOL   skip;
    GT_U32    pairIndex=0;

    /* get index pair incase of ipv6 key or data */
    if((entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E)||
       (entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E))
    {
        /* need to get second index to delete */
        rc = prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete(devNum, entryType, index, GT_FALSE, &pairIndex);
        if((rc != GT_OK)&&(rc!=GT_NOT_FOUND))
        {
            return rc;
        }
    }

    rc = cpssDxChBrgFdbMacEntryStatusGet(devNum,index,&valid,&skip);
    if(rc != GT_OK)
        return rc;

    /* If the entry is valid need to invalidate and to decrement the bank counter
       If the entry is invalid - do nothing */
    if(valid)
    {
        rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,index);
        if(rc!=GT_OK)
            return rc;

        /* decrement bank counter */
        rc =  cpssDxChBrgFdbBankCounterUpdate(devNum, index%16, GT_FALSE);
        if(rc!=GT_OK)
            return rc;

    }

    if(pairIndex != PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS)
    {
        rc = cpssDxChBrgFdbMacEntryStatusGet(devNum,pairIndex,&valid,&skip);
        if(rc != GT_OK)
            return rc;

        /* If the entry is valid need to invalidate and to decrement the bank counter
           If the entry is invalid - do nothing */
        if(valid)
        {
            rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,pairIndex);
            if(rc!=GT_OK)
                return rc;

            /* decrement bank counter */
            rc =  cpssDxChBrgFdbBankCounterUpdate(devNum, pairIndex%16, GT_FALSE);
            if(rc!=GT_OK)
                return rc;
        }
    }

    /* if we succeed in deleting some or both entries (ipv6 UC addr and data) from HW
       we need to delete the address/data index from the Link List data base */
    rc = prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete(devNum, entryType, index, GT_TRUE, &pairIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}
#endif /*DXCH_CODE*/

#ifdef SUPPORT_AU
/**
* @internal prvAppDemoAaMsgHandleFdbUcRouteEntries function
* @endinternal
*
* @brief   This handle AA message for FDB UC Route Entries
*
* @param[in] entryType                - UC Entry type: ipv4/ipv6 Key/ipv6 data
* @param[in] macEntryIndex            - UC Entry index to invalidate
*
* @param[out] errorCounterPtr          - (pointer to) incremented on every error.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvAppDemoAaMsgHandleFdbUcRouteEntries
(
    IN  CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType,
    IN  GT_U32                      macEntryIndex,
    OUT GT_U32                      *errorCounterPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       dev;
    GT_U8       maxDevNum;

    if(cpssEnFdbBrgFdbEntryInvalidateFunc)
    {
        maxDevNum = (isHirApp) ? APP_DEMO_MAX_HIR_SUPPORTED_PP_CNS : appDemoPpConfigDevAmount;

        for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(maxDevNum); dev++)
        {
            if(appDemoPpConfigList[dev].valid == GT_FALSE)
            {
                continue;
            }

            AU_FDB_UPDATE_LOCK();
            /* delete data from cpss */

            rc = cpssEnFdbBrgFdbEntryInvalidateFunc(appDemoPpConfigList[dev].devNum,
                                                    entryType,macEntryIndex);

            if (rc != GT_OK)
            {
                /* go to next message to handle */
                (*errorCounterPtr)++;
                AU_FDB_UPDATE_UNLOCK();
                continue;
            }

            AU_FDB_UPDATE_UNLOCK();
        }
    }

    return rc;
}

/**
* @internal appDemoAuMsgHandle function
* @endinternal
*
* @brief   This handle batch of AU messages.
*
* @param[in] devNum                   - the device number.
* @param[in] bufferArr[]              - array of AU messages
* @param[in] numOfAu                  - number of AU messages in bufferArr[]
*
* @param[out] errorCounterPtr          - (pointer to) incremented on every error.
* @param[out] isFirstMissDonePtr       - (pointer to) indication to trace the first fail of adding an entry into the FDB
*                                      when this flag is set to 1 , the first time that the engine can't add entry
*                                      the engine will check how much entries in the FDB. and print in % the capacity.
* @param[out] isFirstReorderDonePtr    -  indication that the Cuckoo algorithm succeeded to do it's first reorder of
*                                      entries in order to insert NA that first failed to be inserted but after the
*                                      are order succeeded
*
* @retval GT_OK                    - on success
*/
static GT_STATUS appDemoAuMsgHandle
(
    IN  GT_UINTPTR           auEvHandler,
    IN  GT_U8                devNum,
    IN  CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[],
    IN  GT_U32               numOfAu,
    OUT GT_U32              *errorCounterPtr,
    OUT GT_BOOL             *isFirstMissDonePtr,
    OUT GT_BOOL             *isFirstReorderDonePtr
)
{
    GT_STATUS                   rc;
    GT_U32                      auIndex;
    GT_BOOL                     completed;
    GT_BOOL                     succeeded;
    CPSS_MAC_ENTRY_EXT_STC      cpssMacEntry;
    GT_U8                       dev;
    CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType;
    GT_U8                       maxDevNum;

    for (auIndex = 0; auIndex < numOfAu; auIndex++)
    {
        if(bufferArr[auIndex].updType <= CPSS_FU_E )
        {
            appDemoAuMessageNum[bufferArr[auIndex].updType]++;
        }
        else
        {
            appDemoAuWrongMessageNum++;
        }

        switch (bufferArr[auIndex].updType)
        {
            case CPSS_AA_E:
                if(appDemoSysConfig.supportAaMessage == GT_FALSE)
                {
                    break;
                }

                entryType = bufferArr[auIndex].macEntry.key.entryType;
                if (((entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E)||
                    (entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E)||
                    (entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E))
#if defined DXCH_CODE
                    && (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E))
#endif
                    )
                {
                    rc = prvAppDemoAaMsgHandleFdbUcRouteEntries(entryType,bufferArr[auIndex].macEntryIndex,errorCounterPtr);
                    if(rc != GT_OK)
                        return rc;

                    break;
                }
                /* fall through ... */
            case CPSS_NA_E:
            {
                osMemCpy(&cpssMacEntry, &bufferArr[auIndex].macEntry,
                             sizeof(CPSS_MAC_ENTRY_EXT_STC));
                cpssMacEntry.saCommand = CPSS_MAC_TABLE_FRWRD_E;
                cpssMacEntry.age = GT_TRUE;
                cpssMacEntry.spUnknown = GT_FALSE;/*even if SP came from device , we need to set entry with 'SP false' */

                maxDevNum = (isHirApp) ? APP_DEMO_MAX_HIR_SUPPORTED_PP_CNS : appDemoPpConfigDevAmount;

                for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(maxDevNum); dev++)
                {
                    if(appDemoPpConfigList[dev].valid == GT_FALSE)
                    {
                        continue;
                    }

                    AU_FDB_UPDATE_LOCK();
                    if(bufferArr[auIndex].updType == CPSS_NA_E)
                    {
                        /* write data to cpss */
                        rc = cpssEnFdbBrgFdbEntrySetFunc(appDemoPpConfigList[dev].devNum,
                                                         bufferArr[auIndex].portGroupId,&cpssMacEntry);
                    }
                    else /*AA*/
                    {
                        /* delete data from cpss */
                        rc = cpssEnFdbBrgFdbEntryDeleteFunc(appDemoPpConfigList[dev].devNum,
                                                            &cpssMacEntry.key);
                    }

                    if (rc != GT_OK)
                    {
                        /* go to next message to handle */
                        (*errorCounterPtr)++;
                        AU_FDB_UPDATE_UNLOCK();
                        continue;
                    }

                    if(cpssEnFdbFromCpuAuMsgStatusGetFunc)
                    {
                       completed = GT_FALSE;
                       succeeded = GT_FALSE;/*fix warning: local variable 'succeeded' may be used without having been initialized*/
                       /* wait for the PP to process the action */
                       while(completed == GT_FALSE)
                       {
                            /* simulation task needs CPU time in order
                               to execute the process */
                            #ifdef ASIC_SIMULATION
                            osTimerWkAfter(1);
                            #endif

                            /* Check that the AU message processing has completed */
                            rc = cpssEnFdbFromCpuAuMsgStatusGetFunc(appDemoPpConfigList[dev].devNum,
                                                                    &completed,
                                                                    &succeeded);
                            if (rc != GT_OK)
                            {
                                /* go to next message to handle */
                                (*errorCounterPtr)++;
                                succeeded = GT_FALSE;
                                break;
                            }
                       }
                    }
                    else
                    {
                        /* was checked in other place */
                        succeeded = GT_TRUE;
                    }

                    AU_FDB_UPDATE_UNLOCK();

                    if (succeeded == GT_FALSE)
                    {
                        /* the action competed but without success !? */
                        /* go to next message to handle */
                        (*errorCounterPtr)++;
                        continue;
                    }
                }
                break;

            }
            case CPSS_QA_E:
            case CPSS_QR_E:
            case CPSS_SA_E:
            case CPSS_QI_E:
            case CPSS_FU_E:
            case CPSS_TA_E:
                break;
            default:
                /* the action competed but without success !? */
                /* go to next message to handle */
                (*errorCounterPtr)++;
                continue;
        }

        if(traceAuq)
        {
            osPrintf("%s",traceAuqArr[bufferArr[auIndex].updType]);
        }

        /* check if we have callback function for this type of message */
        auEventHandlerDoCallbacks(auEvHandler,
                (AU_EV_HANDLER_EV_TYPE_ENT)(bufferArr[auIndex].updType),
                devNum, &bufferArr[auIndex]);

    }

    *isFirstMissDonePtr = isFirstMissDone;
    *isFirstReorderDonePtr = isFirstReorderDone;

    return GT_OK;
}

/**
* @internal appDemoDxChAuMsgHandle function
* @endinternal
*
* @brief   DXCH devices : This handle batch of AU messages.
*
* @param[in] devNum                   - the device number.
* @param[in] bufferArr[]              - array of AU messages
* @param[in] numOfAu                  - number of AU messages in bufferArr[]
*
* @param[out] errorCounterPtr          - (pointer to) incremented on every error.
* @param[out] isFirstMissDonePtr       - (pointer to) indication to trace the first fail of adding an entry into the FDB
*                                      when this flag is set to 1 , the first time that the engine can't add entry
*                                      the engine will check how much entries in the FDB. and print in % the capacity.
* @param[out] isFirstReorderDonePtr    -  indication that the Cuckoo algorithm succeeded to do it's first reorder of
*                                      entries in order to insert NA that first failed to be inserted but after the
*                                      are order succeeded
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDxChAuMsgHandle
(
    IN  GT_UINTPTR           auEvHandler,
    IN  GT_U8                devNum,
    IN  CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[],
    IN  GT_U32               numOfAu,
    OUT GT_U32              *errorCounterPtr,
    OUT GT_BOOL             *isFirstMissDonePtr,
    OUT GT_BOOL             *isFirstReorderDonePtr
)
{
#if defined DXCH_CODE
    cpssEnFdbBrgFdbAuMsgBlockGet       = cpssDxChBrgFdbAuMsgBlockGet;
    cpssEnFdbBrgFdbEntrySetFunc        = prvDxChBrgFdbPortGroupMacEntrySet;
    cpssEnFdbBrgFdbEntryDeleteFunc     = prvDxChBrgFdbMacEntryDelete;
    cpssEnFdbBrgFdbEntryInvalidateFunc = prvDxChBrgFdbMacEntryInvalidate;
    cpssEnFdbFromCpuAuMsgStatusGetFunc = NULL;/*the set and delete functions will handle it*/
#endif /*DXCH_CODE*/

    return appDemoAuMsgHandle(auEvHandler,devNum,bufferArr,numOfAu,errorCounterPtr,isFirstMissDonePtr,isFirstReorderDonePtr);
}



/**
* @internal cpssEnGenAuMsgHandle function
* @endinternal
*
* @brief   This routine gets and handles the ChDx Address Update messages.
*
* @param[in] devNum                   - the device number.
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssEnGenAuMsgHandle
(
    IN GT_UINTPTR           auEvHandler,
    IN GT_U8                devNum,
    GT_U32                  evExtData
)
{

    GT_STATUS                   rc = GT_OK;
    GT_STATUS                   rc1;
    GT_U32                      numOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[AU_SIZE];
    GT_U32                      error = 0;/* number of errors */
    GT_BOOL   dummy1,dummy2;

    evExtData = evExtData;

    if(IS_API_CH_DEV_MAC(devNum))/* is device support dxch API ? */
    {
#if defined DXCH_CODE
        cpssEnFdbBrgFdbAuMsgBlockGet       = cpssDxChBrgFdbAuMsgBlockGet;
        cpssEnFdbBrgFdbEntrySetFunc        = prvDxChBrgFdbPortGroupMacEntrySet;
        cpssEnFdbBrgFdbEntryDeleteFunc     = prvDxChBrgFdbMacEntryDelete;
        cpssEnFdbBrgFdbEntryInvalidateFunc = prvDxChBrgFdbMacEntryInvalidate;
        cpssEnFdbFromCpuAuMsgStatusGetFunc = NULL;/*the set and delete functions will handle it*/
#endif /*DXCH_CODE*/
    }

    if(NULL == cpssEnFdbBrgFdbAuMsgBlockGet)
    {
        /* the AU messages processing is not implemented yet for the device */
        return GT_OK;
    }

    /* reset the full buffer ... as CPSS not set all fields and it will cause 'memcmp' to fail. */
    cpssOsMemSet(bufferArr,0,sizeof(bufferArr));

    while(rc != GT_NO_MORE)
    {
        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/
        numOfAu = AU_SIZE;

        AU_FDB_UPDATE_LOCK();
        rc = cpssEnFdbBrgFdbAuMsgBlockGet(devNum, &numOfAu, bufferArr);
        AU_FDB_UPDATE_UNLOCK();

        if((rc != GT_NO_MORE) && (rc != GT_OK))
        {
            return rc;
        }

        /*********************************/
        /* handle all of the AU messages */
        /*********************************/
        rc1 = appDemoAuMsgHandle(auEvHandler,devNum,bufferArr,numOfAu,&error,&dummy1,&dummy2);
        if(rc1 != GT_OK)
        {
            return rc1;
        }

        /* reset the entries that we got */
        cpssOsMemSet(bufferArr,0,numOfAu * sizeof(bufferArr[0]));
    }

    if(error)
    {
        return GT_FAIL;
    }

    return rc;
}

#endif /*SUPPORT_AU*/

/**
* @internal appDemoTraceAuqFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : traceAuq
*
* @param[in] enable                   - enable/disable the printings of indication of AUQ/FUQ messages:
*                                      "+",    CPSS_NA_E
*                                      "qa",   CPSS_QA_E should not be
*                                      "qr",   CPSS_QR_E
*                                      "-",    CPSS_AA_E
*                                      "t",    CPSS_TA_E
*                                      "sa",   CPSS_SA_E should not be
*                                      "qi",   CPSS_QI_E should not be
*                                      "f"     CPSS_FU_E
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTraceAuqFlagSet
(
    IN GT_U32   enable
)
{
    traceAuq = enable;
    return GT_OK;
}

#ifdef ASIC_SIMULATION
GT_STATUS appDemoMaxRetryOnSleep0Set
(
    IN GT_U32   maxRetry
)
{
    appDemoMaxRetryOnSleep0 = maxRetry;
    return GT_OK;
}
#endif /*ASIC_SIMULATION*/

/**
* @internal appDemoAuMessageNumberDump function
* @endinternal
*
* @brief   Get number of Au messages.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoAuMessageNumberDump
(
    GT_VOID
)
{
    cpssOsPrintf("The number NA messages is %d\n", appDemoAuMessageNum[CPSS_NA_E]);
    cpssOsPrintf("The number QA messages is %d\n", appDemoAuMessageNum[CPSS_QA_E]);
    cpssOsPrintf("The number QR messages is %d\n", appDemoAuMessageNum[CPSS_QR_E]);
    cpssOsPrintf("The number AA messages is %d\n", appDemoAuMessageNum[CPSS_AA_E]);
    cpssOsPrintf("The number TA messages is %d\n", appDemoAuMessageNum[CPSS_TA_E]);
    cpssOsPrintf("The number SA messages is %d\n", appDemoAuMessageNum[CPSS_SA_E]);
    cpssOsPrintf("The number QI messages is %d\n", appDemoAuMessageNum[CPSS_QI_E]);
    cpssOsPrintf("The number FU messages is %d\n", appDemoAuMessageNum[CPSS_FU_E]);
    if(appDemoAuWrongMessageNum != 0)
    {
        cpssOsPrintf("The number AU wrong messages is %d\n", appDemoAuWrongMessageNum);
    }

    if(numOfSuccessesInReorder)
    {
        cpssOsPrintf("number of cases that the movement of old entry to other place allowed the new \n"
                     "entry to take it's place is [%d] \n",
                     numOfSuccessesInReorder);
    }

    if(numOfFailesInReorder)
    {
        cpssOsPrintf("number of cases that NA not succeeded [%d] \n",
                    numOfFailesInReorder);
    }




    return GT_OK;
}

/**
* @internal appDemoAuProcessingTraceFirstMiss function
* @endinternal
*
* @brief   function to allow set the flag of : traceFirstMiss
*         NOTE: this trace will happen only ONCE.
*         in order to get this print again , call appDemoAuProcessingFirstMissIndicationReset()
*         to state that the FDB did 'flush' so we can again look for 'first miss'
*         NOTE: another trace that relate to this flag is 'first reorder'
*         in order to get this print again , call appDemoAuProcessingFirstReorderIndicationReset()
*         to state that the FDB did 'flush' so we can again look for 'first reorder'
* @param[in] enable                   - enable/disable the printings during AU processing of indication
*                                      of 'first miss'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoAuProcessingTraceFirstMiss
(
    IN GT_U32   enable
)
{
    traceFirstMiss = enable;
    return GT_OK;
}
/**
* @internal appDemoAuProcessingFirstMissIndicationReset function
* @endinternal
*
* @brief   function state the 'first miss' indication should be reset.
*         meaning that the next fail to insert new FDB entry would be considered
*         'first miss'
*         use this function to state that the FDB did 'flush' so we can again look
*         for 'first mniss'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoAuProcessingFirstMissIndicationReset
(
    GT_VOID
)
{
    isFirstMissDone = 0;
    return GT_OK;
}

/**
* @internal appDemoAuProcessingFirstReorderIndicationReset function
* @endinternal
*
* @brief   function state that the Cuckoo algorithm succeeded to do it's first reorder of
*         entries in order to insert NA that first failed to be inserted but after the
*         re order succeeded
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoAuProcessingFirstReorderIndicationReset
(
    GT_VOID
)
{
    isFirstReorderDone = 0;
    return GT_OK;
}


