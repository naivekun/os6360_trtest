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
* @file appDemoPortInit.c
*
* @brief Initialization functions for port using Port Manager.
*
* @version   1
********************************************************************************
*/

#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#endif

#ifdef PX_CODE
#include <cpss/px/port/cpssPxPortManager.h>
#endif

#include <cpss/common/port/cpssPortManager.h>

#define APP_DEMO_LOG(...)
/*                                    \
    do{                               \
        osPrintf("\n ** APP_DEMO (portInit) "); \
        osPrintf(__VA_ARGS__);        \
        osPrintf("** \n");            \
    }while (0)
*/

#define APP_DEMO_LOG1(...)\
    do{                                          \
        osPrintf("\n APP_DEMO: "); \
        osPrintf(__VA_ARGS__);                   \
        osPrintf("\n");                          \
    }while (0)

#define APP_DEMO_LOG_ERROR(...) \
    do{                                     \
        osPrintf("\n APP_DEMO ERROR: ");    \
        osPrintf("[line %d]",__LINE__);     \
        osPrintf(__VA_ARGS__);              \
        osPrintf("\n");                     \
    }while (0)


/* whether or not multi-threaded usage should be performed. It is recommended to use
   the multi-threaded version, in which a dedicated task exist for the initialization
   of all ports in the system. This file contain examples for both usages */
/* NOTE: currently, port manager does not support the value of zero when managing
   loopbacked ports on a device, i.e when managing ports on a device where the ports
   are connected to each other.*/
static GT_U8 appDemoPortManagerMultiThreadedUsage =  1;

extern GT_BOOL portMgr;
/*****************************************************************
                         Tasks semaphores
*****************************************************************/
CPSS_OS_SIG_SEM     portInitSeqSignalDetSem=0;

/*****************************************************************
                         Forward declarations
*****************************************************************/
unsigned __TASKCONV appDemoPortInitSeqSignalDetectedStage
(
        GT_VOID * param
);

GT_STATUS appDemoPortInitSequentialInit
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

GT_STATUS appDemoPortInitPortParametersSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal appDemoPortInitSeqConfigureStage function
* @endinternal
*
* @brief   Create port using port manager.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqConfigureStage
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Inside first stage: Inside init stage");

    /****************************************
            Create Port
    *****************************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    APP_DEMO_LOG("calling the stage config with stage INIT");
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqPortStatusChangeSignal function
* @endinternal
*
* @brief   Handler for port events. This API will pass
*         CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E (or
*         CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E) event to port manager
*         in order to notify it about a (possibly) change in the port status,
*         and then will initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event (as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state)
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] uniEvent                 - unified event raised on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqPortStatusChangeSignal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32                      uniEvent
)
{
    GT_STATUS                       rc;
    CPSS_PORT_MANAGER_STATUS_STC    portConfigOutParams;
    CPSS_PORT_MANAGER_STC           portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("(port %d)uniEvent: %d",portNum,uniEvent);

    /* AP port is processed in the FW, hence no need dedicated task */
    if (CPSS_SRVCPU_PORT_802_3_AP_E != uniEvent)
    {
        /********************************************
           1. Creating handling task if not created yet and if multi-threaded
            work is desired
          *******************************************/
        /* If handler was not made yet and we are in multi-threaded mode */
        if( (0 == portInitSeqSignalDetSem) && appDemoPortManagerMultiThreadedUsage)
        {
            GT_U32   tuneStartTaskId;

            if(cpssOsSigSemBinCreate("portInitSeqSignalDetSem", CPSS_OS_SEMB_EMPTY_E, &portInitSeqSignalDetSem) != GT_OK)
            {
                APP_DEMO_LOG_ERROR("calling cpssOsSigSemBinCreate returned %d", GT_FAIL);
                return GT_FAIL;
            }

            if (portMgr)
            /* Don't create the task if Port Manager is not supported*/
            {
                rc = cpssOsTaskCreate("portManagerTask",                     /* Task Name      */
                                      500,                                   /* Task Priority  */
                                      _64K,                                  /* Stack Size     */
                                      appDemoPortInitSeqSignalDetectedStage, /* Starting Point */
                                      (GT_VOID*)((GT_UINTPTR)devNum),        /* Arguments list */
                                      &tuneStartTaskId);                     /* task ID        */
                if (rc != GT_OK)
                {
                    APP_DEMO_LOG_ERROR("calling cpssOsTaskCreate returned=%d, portNum=%d", rc, portNum);
                    return rc;
                }
            }
        }
    }

    /************************************************************
        2. Getting port status
    ***********************************************************/
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }
    /* if port is in failure state, not passing the event to cpss, app need to furthur check
       this issue, possible bad connectivity.
       (Alternatively, app can decide to notify port manager with the event and then port will continue the flow).
       also if port is disabled not doing it. */
    if ( portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        portConfigOutParams.portUnderOperDisable == GT_TRUE)
    {
        APP_DEMO_LOG("port is in HW failure state or in disable so abort");
        return GT_OK;
    }

    /************************************************************
        3. Passing event to port manager
    ***********************************************************/
    if (CPSS_SRVCPU_PORT_802_3_AP_E == uniEvent)
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E;
    }
    else
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
    }
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    /************************************************************
        4. Signaling dedicated task if configured, or else,
        performing initialization operations on the main context
    ***********************************************************/
    if (CPSS_SRVCPU_PORT_802_3_AP_E != uniEvent)
    {
        /* if we want to use the multi-threaded version (currently,
           only multi-threaded version is supported in port manager
           when configuring ports on the same device, i.e when a port
           and it's peer are on the same device)*/
        if (appDemoPortManagerMultiThreadedUsage)
        {
            APP_DEMO_LOG("signaling signal detect task");
            /* Signal the handler */
            rc = cpssOsSigSemSignal(portInitSeqSignalDetSem);
        }
        else /* currently not supported */
        {
            rc = appDemoPortInitSequentialInit(devNum, portNum);
        }
    }

    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling cpssOsSigSemSignal returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSequentialInit function
* @endinternal
*
* @brief   Initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event, as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is an alternative implementation for appDemoPortInitSeqSignalDetectedStage
*       API, While appDemoPortInitSeqSignalDetectedStage act as a dedicated task for the
*       init process, it can be implemented straight forward as this API in the same context.
*
*/
GT_STATUS appDemoPortInitSequentialInit
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    CPSS_PORT_MANAGER_STC        portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif

    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling cpssDxChPortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;

    /* port will stay in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E for limited time
       so this 'while' loop on the main execution is safe to use */
    while (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
    {
#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
        }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
        }
#endif
        if(rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent,rc, portNum);
        }
#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
        }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
        }
#endif
        if(rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling PortManagerEventGet returned=%d, portNum=%d", rc, portNum);
        }
    }

    return GT_OK;
}

/*******************************************************************************
* appDemoPortInitSeqSignalDetectedStage
*
* DESCRIPTION:
*       Initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*       event, as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state.
*
* INPUTS:
*       param   - device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - on passing null pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned __TASKCONV appDemoPortInitSeqSignalDetectedStage
(
        GT_VOID * param
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U8                   devNum;
    GT_STATUS               rc;
    GT_U8                   portsExistFlag;
    CPSS_PORT_MANAGER_STATUS_STC   portConfigOutParams;
    CPSS_PORT_MANAGER_STC          portEventStc;
    GT_U32                  maxPort;
    /* Variables initialization */
    devNum = (GT_U8)((GT_UINTPTR)param);

    rc = GT_NOT_APPLICABLE_DEVICE;
    maxPort = CPSS_MAX_PORTS_NUM_CNS;/*appDemoPpConfigList[devNum].maxPortNumber*/
    while(1)
    {
        cpssOsSigSemWait(portInitSeqSignalDetSem, 0);
        /* marking port-exist flag so iteration cycles will be performed at least once */
        portsExistFlag = 1;

        while (portsExistFlag>0)
        {
            /* by here, the next 'for' loop is promised to run, so we can mark this flag with
               zero. The only case in which the 'while' will be iterated again is if there is existing port
               in INIT state (checked in the next scope), at which we want to continue the iterations */
            portsExistFlag = 0;

            for (portNum = 0; (portNum < maxPort)/*CPSS_MAX_PORTS_NUM_CNS*/; portNum++)
            {
                /************************************************************************
                   1. Get port status
                **********************************************************************/
#ifdef PX_CODE
                if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                {
                    rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                }
#endif
#ifdef DXCH_CODE
                if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                {
                    rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                }
#endif

                /* GT_BAD_PARAM will be received upon bad portNum argument. We iterating
                   all possible portNum values so in GT_BAD_PARAM case we will ignore this
                   error code and continue execution, either way we won't enter the 2nd 'if'
                   statement as bad portNum argument will not meet the statement condition */
                if(rc != GT_OK)
                {
                    continue;
                }

                /************************************************************************
                   2. If port state is INIT than continue to call INIT_EVENT
                **********************************************************************/
                if (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
                {
                    APP_DEMO_LOG("calling to launching whole INIT on port");
                    /* marking that there is a port in INIT state so iteration cycles will continue */
                    portsExistFlag = 1;
                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
#ifdef PX_CODE
                    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                    {
                        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
                    }
#endif
#ifdef DXCH_CODE
                    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                    {
                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                    }
#endif
                    if(rc != GT_OK)
                    {
                        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                    }
                }
            } /* for (portNum) */
        cpssOsTimerWkAfter(10);
        } /* while (portsExistFlag>0) */
    } /* while (1) */

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqEnablePortStage function
* @endinternal
*
* @brief   Enable the port. This API is the complement of appDemoPortInitSeqDisablePortStage
*         API, while the later disabled the port, this API will enable it back.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqEnablePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Enabling port bitmaps");
    /**************************
           Enable Port
    **************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_ENABLE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqLinkStatusChangedStage function
* @endinternal
*
* @brief   Handler function for link status change event. This API will pass
*         the event to the port manager and perform actions if needed.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqLinkStatusChangedStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
          1. Notify port manager
    *******************************/
    APP_DEMO_LOG("calling to launching whole tune on ports");
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        portStage.portUnderOperDisable == GT_TRUE)
    {
        APP_DEMO_LOG("port is in HW failure state or in disable so abort mac_level_change notification");
        return GT_OK;
    }
    if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E )
    {
        APP_DEMO_LOG("port is in reset state so abort mac_level_change notification");
        return GT_OK;
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoPortInitSeqDisablePortStage function
* @endinternal
*
* @brief   Disable the port. This API is the complement of appDemoPortInitSeqEnablePortStage
*         API, while the later enables the port, this API will disable it.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqDisablePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
            Disable Port
    *******************************/
    APP_DEMO_LOG("disabling port start");
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DISABLE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif

    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet with PORT_STAGE_DISABLE_E ended-bad and returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    APP_DEMO_LOG("disabling port end");

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqDeletePortStage function
* @endinternal
*
* @brief   Delete port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqDeletePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
            Delete Port
    *******************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/* ##############################################
   #############App Demo commands for port manager ##########
   ############################################## */


#if 0 /* TBD */
GT_STATUS cpssDxChPortManagerMultiPortParamsSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         adaptiveMode,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  trainMode,
    IN  GT_BOOL                              overrideElectricalParams,
    IN  GT_BOOL                              overridePolarityParams,
    IN  char*                           portString
)
{
    GT_U32 ii,jj;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32 temp = 0;
    GT_STATUS rc;
    char *ports = (char*)cpssOsMalloc(CPSS_MAX_PORTS_NUM_CNS*(sizeof(char)));
    cpssOsMemSet(&ports,0,sizeof(ports));
    ports = portString;
    /* start loop on all relevant ports */
    printf("\n ports %s ", ports);

    for (ii = 0; ports[ii] != '\0'; ii++)
    {
        if (ports[ii] == ',')
        {
            continue;
        }
        /* if portNum is 1 digit */
        else if ((ports[ii+1] == ',') || (ports[ii+1] == '\0'))
        {
            portNum = ports[ii] - '0';
        }
        /* if portNum is above 1 digit */
        else
        {
            for (jj = ii; jj < ii+2; jj++)
            {
                temp = temp * 10 + (ports[jj] - '0');
            }
            portNum = temp;
            temp = 0;
            ii++;
        }
        osPrintf("\n setting Port-Manager parameters for port %d ", portNum);

        rc = cpssDxChPortManagerParametersSetExt(devNum,portNum,ifMode,speed,adaptiveMode,trainMode, overrideElectricalParams, overridePolarityParams);

        if (rc!=GT_OK)
        {
            osPrintf("calling cpssDxChPortManagerMultiPortParamsSet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }
}

GT_STATUS cpssDxChPortManagerMultiPortsEventSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_MANAGER_EVENT_ENT portEvent,
    IN  char*                   portString
)
{
    GT_U32 ii,jj;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32 temp = 0;
    GT_STATUS rc;
    char *ports = (char*)cpssOsMalloc(CPSS_MAX_PORTS_NUM_CNS*(sizeof(char)));
    cpssOsMemSet(&ports,0,sizeof(ports));
    CPSS_PORT_MANAGER_STC portEventStc;
    portEventStc.portEvent = portEvent;
    ports = portString;

    /* start loop on all relevant ports */
    osPrintf("\n ports %s ", ports);

    for (ii = 0; ports[ii] != '\0'; ii++)
    {
        if (ports[ii] == ',')
        {
            continue;
        }
        /* if portNum is 1 digit */
        else if ((ports[ii+1] == ',') || (ports[ii+1] == '\0'))
        {
            portNum = ports[ii] - '0';
        }
        /* if portNum is above 1 digit */
        else
        {
            for (jj = ii; jj < ii+2; jj++)
            {
                temp = temp * 10 + (ports[jj] - '0');
            }
            portNum = temp;
            temp = 0;
            ii++;
        }
        osPrintf("\n setEvent on Port-Manager, portNum %d ", portNum);

        rc = cpssDxChPortManagerEventSet(devNum,portNum,&portEventStc);

        if (rc!=GT_OK)
        {
            osPrintf("calling cpssDxChPortManagerMultiPortParamsSet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }
}
#endif /* TBD */


/**
* @internal appDemoPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up port using port manager.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode for the port
* @param[in] speed                    -  for the port
* @param[in] powerUp                  - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqStart
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp
)
{
    GT_STATUS    rc;

    APP_DEMO_LOG("Starting: inside appDemoPortInitSeqStart");

    if (powerUp)
    {
        /* set port parameters */
        rc = appDemoPortInitPortParametersSet(devNum, portNum, ifMode, speed);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitPortParametersSet returned %d", rc);
            return rc;
        }
        /* start the port */
        rc = appDemoPortInitSeqConfigureStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned %d", rc);
            return rc;
        }
    }
    else
    {
        /* delete port */
        rc = appDemoPortInitSeqDeletePortStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned %d", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoApPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up AP port using port manager.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode for the port
* @param[in] speed                    -  for the port
* @param[in] powerUp                  - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoApPortInitSeqStart
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp
)
{
    GT_STATUS    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Starting: inside appDemoPortInitSeqStart");

    if (powerUp)
    {
        CPSS_PM_PORT_PARAMS_STC portParams;

#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &portParams);
        }
#endif
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &portParams);
        }
#endif

        if(rc != GT_OK)
        {
            cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", rc);
            return rc;
        }

        /* filling port supported advertisement speeds and modes,
           only one in this example */
        portParams.portParamsType.apPort.numOfModes = 1;
        portParams.portParamsType.apPort.modesArr[0].ifMode = ifMode;
        portParams.portParamsType.apPort.modesArr[0].speed = speed;

        /*
           (NOTE: the following commented code is an example of using parameters set
           structure and API for AP port. the values in this example are not optimal,
           and presented for the sake of the usage example)

           For an AP port, for BobK and above devices, cpss provides ability to
           configure TX parameters offsets which will take effect in the AP FW,
           the values range are -31 to +31:

           CPSS_PORT_MANAGER_MODE_PARAM_STC params;
           if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
           {
                params.attributeType = CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E;
                params.attribute.serdesTxParams.totalLanes = 2;
                params.attribute.serdesTxParams.txCfg[0].lane = 0;
                params.attribute.serdesTxParams.txCfg[0].laneType = CPSS_PORT_MANAGER_SERDES_LANE_TYPE_LOCAL_E;
                params.attribute.serdesTxParams.txCfg[0].txParams.txAmp = -3;
                params.attribute.serdesTxParams.txCfg[0].txParams.emph1 = -3;
                params.attribute.serdesTxParams.txCfg[0].txParams.emph0 = -3;
                params.attribute.serdesTxParams.txCfg[1].lane = 1;
                params.attribute.serdesTxParams.txCfg[1].laneType = CPSS_PORT_MANAGER_SERDES_LANE_TYPE_LOCAL_E;
                params.attribute.serdesTxParams.txCfg[1].txParams.txAmp = -4;
                params.attribute.serdesTxParams.txCfg[1].txParams.emph1 = 5;
                params.attribute.serdesTxParams.txCfg[1].txParams.emph0 = -5;

                apParams[0].modeParamsArrSize = 1;
                apParams[0].modeParamsArr = &params[0];
            }
        */

        /* ap port filling */

#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
        }
#endif
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
        }
#endif
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitPortParametersSet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }

        /* start the port */
        rc = appDemoPortInitSeqConfigureStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }
    else
    {
        /* delete port */
        rc = appDemoPortInitSeqDeletePortStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSequenceIsPortApplicable function
* @endinternal
*
* @brief   Check whether port is managed by port manager
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_BOOL appDemoPortInitSequenceIsPortApplicable
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STATUS_STC    configureOutputParams;
    CPSS_PP_FAMILY_TYPE_ENT      devFamily;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* device was removed */
        return GT_OK;
    }
    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = GT_NOT_APPLICABLE_DEVICE;

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &configureOutputParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &configureOutputParams);
    }
#endif

    if(rc != GT_OK)
    {
        APP_DEMO_LOG("calling PortManagerStatusGet #2 returned %d", rc);
        return GT_FALSE;
    }

    if (configureOutputParams.portState != CPSS_PORT_MANAGER_STATE_RESET_E)
    {
        return GT_TRUE;
    }

    return GT_FALSE;

}

/**
* @internal appDemoDbgTestMaxPortsInit function
* @endinternal
*
* @brief   DEBUG function.
*/
GT_STATUS appDemoDbgTestMaxPortsInit
(
    GT_U8 up
)
{
    /*
    //SRLR 10
    appDemoPortInitSeqStart 0, 58, 20, 3, 1
    appDemoPortInitSeqStart 0, 56, 20, 3, 1

    //SRLR 40
    appDemoPortInitSeqStart 0, 56, 20, 9, 1
    appDemoPortInitSeqStart 0, 64, 20, 9, 1
    */

    /* 10g srlr*/
    if (up==1)
    {
        appDemoPortInitSeqStart(0, 56, 20, 3, 1);
        appDemoPortInitSeqStart(0, 58, 20, 3, 1);
        appDemoPortInitSeqStart(0, 59, 20, 3, 1);
        appDemoPortInitSeqStart(0, 57, 20, 3, 1);
        appDemoPortInitSeqStart(0, 64, 20, 3, 1);
        appDemoPortInitSeqStart(0, 66, 20, 3, 1);
        appDemoPortInitSeqStart(0, 67, 20, 3, 1);
        appDemoPortInitSeqStart(0, 65, 20, 3, 1);
    }
    else
    {
        appDemoPortInitSeqStart(0, 56, 20, 3, 0);
        appDemoPortInitSeqStart(0, 58, 20, 3, 0);
        appDemoPortInitSeqStart(0, 59, 20, 3, 0);
        appDemoPortInitSeqStart(0, 57, 20, 3, 0);
        appDemoPortInitSeqStart(0, 64, 20, 3, 0);
        appDemoPortInitSeqStart(0, 66, 20, 3, 0);
        appDemoPortInitSeqStart(0, 67, 20, 3, 0);
        appDemoPortInitSeqStart(0, 65, 20, 3, 0);
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitPortParametersSet function
* @endinternal
*
* @brief   Set port parameters.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode for the port
* @param[in] speed                    -  for the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitPortParametersSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{

    CPSS_PM_PORT_PARAMS_STC           portParams;
    GT_STATUS                         rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Starting: inside appDemoPmPortParamsSet");

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    }
#endif
    if (rc!=GT_OK)
    {
        APP_DEMO_LOG_ERROR("internal_cpssPortManagerInitParamsStruct returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    /* standard */
    portParams.portParamsType.regPort.ifMode    = ifMode;
    portParams.portParamsType.regPort.speed     = speed;

    /* in this example we are setting common additional parameters for 10G KR port
       in Sip_5_15 devices (overriding CPSS defaults) */
    if ( PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) &&
         ifMode == CPSS_PORT_INTERFACE_MODE_KR_E &&
         speed == CPSS_PORT_SPEED_10000_E)
    {
        /* overriding CPSS default parameters for the port */

        /*
            For example, default RxTx for Sip_5_15 devices can be viewed for reference on:
                hwsAvagoSerdesTxRxTuneParamsArray
            for non Sip_5_15:
                hwPortTxDefaults
        */

        /*
           first lane in port, given by local index 0  - for example,
           in BobK Cetus, the global serdes index is 27.
           Alternative code:
                mParamsArr[1].attribute.serdesRxParams.rxCfg[0].lane = 27;
                mParamsArr[1].attribute.serdesRxParams.rxCfg[0].laneType = CPSS_PORT_MANAGER_SERDES_LANE_TYPE_GLOBAL_E;
           same principle in the Rx parameters, following this Tx parameters
        */

        /*
        mParamsArr[0].param.serdesTxParams.txCfg[0].lane = 0;
        mParamsArr[0].param.serdesTxParams.txCfg[0].laneType = CPSS_PORT_MANAGER_SERDES_LANE_TYPE_LOCAL_E;
        */
        portParams.portParamsType.regPort.laneParams[0].validLaneParamsBitMask = 0;
        portParams.portParamsType.regPort.portAttributes.validAttrsBitMask = 0;

        portParams.portParamsType.regPort.laneParams[0].globalLaneNum  = 0;

        /* common param - TX params */
        portParams.portParamsType.regPort.laneParams[0].txParams.txAmp = 4;
        portParams.portParamsType.regPort.laneParams[0].txParams.emph0 = 0;
        portParams.portParamsType.regPort.laneParams[0].txParams.emph1 = 0;
        CPSS_PM_SET_VALID_LANE_PARAM (&portParams, 0, CPSS_PM_LANE_PARAM_TX_E);

        /* common param - RX params */
        portParams.portParamsType.regPort.laneParams[0].rxParams.dcGain = 100;
        portParams.portParamsType.regPort.laneParams[0].rxParams.ffeCap = 15;
        portParams.portParamsType.regPort.laneParams[0].rxParams.ffeRes = 3;
        portParams.portParamsType.regPort.laneParams[0].rxParams.bandWidth = 13;
        portParams.portParamsType.regPort.laneParams[0].rxParams.loopBandwidth = 0;
        portParams.portParamsType.regPort.laneParams[0].rxParams.sqlch = 68;
        CPSS_PM_SET_VALID_LANE_PARAM (&portParams, 0, CPSS_PM_LANE_PARAM_RX_E);

        /* Attributes */
        /* common param - FEC params */
        portParams.portParamsType.regPort.portAttributes.fecMode = CPSS_PORT_FEC_MODE_DISABLED_E;
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);

        /* overriding default rx training to be enhanced tune */
        portParams.portParamsType.regPort.portAttributes.trainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E;
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);

        /*
           Example for setting MAC loopback:
           (instead of attributeType CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_TUNE_MODE_E)

           mNonApParamsArr[0].attributeType = CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_LOOPBACK_E;
           mNonApParamsArr[0].attribute.loopback.loopbackType = CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E;
           mNonApParamsArr[0].attribute.loopback.loopbackMode.macLoopbackEnable = GT_TRUE;
        */
    }

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    }
#endif
    if (rc!=GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling internal_cpssPortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitPortParametersGet function
* @endinternal
*
* @brief   Get port parameters and print them.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoPortInitPortParametersGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum
)
{
    GT_U32                                portMacNum     = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS              *curPortParams;
    GT_U32                                bm;
    GT_U32                                lanesArrIdx;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    CPSS_PM_PORT_PARAMS_STC               portParams;
    GT_STATUS                             rc             = GT_NOT_APPLICABLE_DEVICE;
    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams     = NULL;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
    *confiData      = NULL;

    APP_DEMO_LOG("Starting: inside appDemoPmPortParamsGet");

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_LAST_E, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_LAST_E, &portParams);
    }
#endif
  if (rc != GT_OK)
  {
    APP_DEMO_LOG_ERROR("calling internal_cpssPortManagerInitParamsStruct returned %d", rc);
    return rc;
  }

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    }
#endif

    if (rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling internal_cpssPortManagerPortParamsGet returned %d", rc);
        return rc;
    }

    /* prepering data structures for parameters get according to the port type */
    if (portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        APP_DEMO_LOG_ERROR("New PortManager APIs currently supports regular ports alone");
        return rc;
    }

    /* ifMode and speed for non-AP port */
    if (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        APP_DEMO_LOG1("ifMode=%d, speed=%d",
                      portParams.portParamsType.regPort.ifMode, portParams.portParamsType.regPort.speed);

        portAttr    = &(portParams.portParamsType.regPort.portAttributes);
        bm          = portAttr->validAttrsBitMask;

        if ( bm & CPSS_PM_ATTR_TRAIN_MODE_E )
        {
            APP_DEMO_LOG1("trainMode=%d", portAttr->trainMode);
        }

        if ( bm & CPSS_PM_ATTR_RX_TRAIN_SUPP_E )
        {
            APP_DEMO_LOG1("adaptiveRxTrainSupported=%d", portAttr->adaptRxTrainSupp);
        }

        if ( bm & CPSS_PM_ATTR_EDGE_DETECT_SUPP_E )
        {
            APP_DEMO_LOG1("edgeDetectSupported=%d", portAttr->edgeDetectSupported);
        }

        if ( bm & CPSS_PM_ATTR_LOOPBACK_E )
        {
            APP_DEMO_LOG1("loopbackType=%d", portAttr->loopback.loopbackType);

            if (portAttr->loopback.loopbackType ==
                CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)
            {
                APP_DEMO_LOG1("macLoopbackModeEnabled=%d",
                              portAttr->loopback.loopbackMode.macLoopbackEnable);
            }
            else if (portAttr->loopback.loopbackType ==
                     CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E)
            {
                APP_DEMO_LOG1("serdesLoopbackMode=%d",
                              portAttr->loopback.loopbackMode.serdesLoopbackMode);
            }
            else if (portAttr->loopback.loopbackType ==
                     CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E)
            {
                APP_DEMO_LOG1("NO Loopback Mode Configured");
            }
            else
            {
                APP_DEMO_LOG1("INVALID loopback type=%d",
                              portAttr->loopback.loopbackType);
            }
        }

        if ( bm & CPSS_PM_ATTR_ET_OVERRIDE_E )
        {
            APP_DEMO_LOG1("Enhance Tune is Active. minLF=%d, maxLF=%d",
                          portAttr->etOverride.minLF, portAttr->etOverride.maxLF);
        }

        if ( bm & CPSS_PM_ATTR_FEC_MODE_E )
        {
            APP_DEMO_LOG1("FEC mode is %d", portAttr->fecMode);
        }

        if ( bm & CPSS_PM_ATTR_CALIBRATION_E )
        {
            APP_DEMO_LOG1("Enhance calibration mode is %d", portAttr->calibrationMode.calibrationType);
            if (portAttr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                APP_DEMO_LOG1("CI - lfMin = %d, lfMax=%d, hfMin=%d, hfMax=%d, eoMin=%d, eoMax=%d bitmap=0x%08x",
                              confiData->lfMinThreshold, confiData->lfMaxThreshold, confiData->hfMinThreshold,
                              confiData->hfMaxThreshold, confiData->eoMinThreshold, confiData->eoMaxThreshold,
                              confiData->confidenceEnableBitMap);
            }
        }

        if ( bm & CPSS_PM_ATTR_UNMASK_EV_MODE_E )
        {
            APP_DEMO_LOG1("Enhance Unmask Event mode is %d", portAttr->unMaskEventsMode);
        }
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        APP_DEMO_LOG1( "Port %d ppCheckAndGetMacFunc failed=%d", portNum, rc);
        return rc;
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, portParams.portParamsType.regPort.ifMode,
                                               portParams.portParamsType.regPort.speed, &portMode);
    if (rc != GT_OK)
    {
        APP_DEMO_LOG1(
                     "prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                     rc, portParams.portParamsType.regPort.ifMode, portParams.portParamsType.regPort.speed);
        return rc;
    }

    curPortParams = hwsPortModeParamsGet(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
    if (curPortParams == NULL)
    {
        APP_DEMO_LOG1("hwsPortModeParamsGet returned null");
        return rc;
    }

    /* Lane Params */
    for (lanesArrIdx = 0; lanesArrIdx < curPortParams->numOfActLanes; lanesArrIdx++)
    {
        laneParams      = &(portParams.portParamsType.regPort.laneParams[lanesArrIdx]);
        bm              = laneParams->validLaneParamsBitMask;

        if ( bm & CPSS_PM_LANE_PARAM_RX_E )
        {

            APP_DEMO_LOG1(
                         "[Lane:%d] RX params bandWidth %d, dcGain %d,"
                         "ffeCap %d, ffeRes %d, loopBandwidth %d, sqlch %d",
                         lanesArrIdx, laneParams->rxParams.bandWidth, laneParams->rxParams.dcGain, laneParams->rxParams.ffeCap,
                         laneParams->rxParams.ffeRes, laneParams->rxParams.loopBandwidth, laneParams->rxParams.sqlch
                         );
        }

        if ( bm & CPSS_PM_LANE_PARAM_TX_E )
        {
            APP_DEMO_LOG1(
                         "[Lane:%d] TX params txAmp %d, "
                         "emph0 %d, emph1 %d, txAmpAdjEn %d, txAmpShft %d ",
                         lanesArrIdx, laneParams->txParams.txAmp, laneParams->txParams.emph0, laneParams->txParams.emph1,
                         laneParams->txParams.txAmpAdjEn, laneParams->txParams.txAmpShft
                         );
        }

    }
    return GT_OK;
}

