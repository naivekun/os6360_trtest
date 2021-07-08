/********************************************************************************
*       (c), Copyright 2001, Marvell International Ltd.         *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE    *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.   *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssCommonPortManager.c
*
* @brief CPSS implementation for Port manager module.
*
*
* @version  1
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE


#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/port/cpssPortSerdes.h>
#include <cpss/common/private/prvCpssCyclicLogger.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/port/private/prvCpssPortPcsCfg.h>

#ifdef CMD_LUA_CLI
#include <cpss/common/port/private/prvCpssPortManagerLuaTypes.h>
#endif /* CMD_LUA_CLI */

#define CPSS_PORT_MANAGER_LOG_INFORMATION_MAC                       CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC
#define CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC                  CPSS_CYCLIC_LOGGER_LOG_ERROR_AND_RETURN_MAC
#define CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC                  CPSS_CYCLIC_LOGGER_LOG_PORT_INFORMATION_MAC

#undef PORT_MGR_DEBUG
#ifdef PORT_MGR_DEBUG
#define CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC(...)      CPSS_CYCLIC_LOGGER_DBG_LOG_INFORMATION_MAC(..)
#else
#define CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC(...)
#endif
#define CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC(...)
#define CPSS_PORT_MANAGER_LOG_MASK_CONVERT_INFORMATION_MAC(...)

/* check NULL pointer */
#define CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(ptr)  if(ptr == NULL) { \
    CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("parameter is null");     \
    CPSS_NULL_PTR_CHECK_MAC(ptr);                    \
}

/* check that the device exists
  return GT_BAD_PARAM on error
*/
#define CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum)               \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))    {          \
        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("device number not exist");  \
        PRV_CPSS_DEV_CHECK_MAC(devNum);                   \
    }

#define PRV_CPSS_PORT_MANAGER_BIT_MAC(bit) (1<<bit)

/* From port manager perspective, "low speed"s are those speeds */
#define PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(_speed) (CPSS_PORT_SPEED_5000_E == _speed || CPSS_PORT_SPEED_1000_E == _speed \
               || CPSS_PORT_SPEED_100_E == _speed || CPSS_PORT_SPEED_10_E == _speed \
               || CPSS_PORT_SPEED_2500_E == _speed)

/* Macro for two lanes speeds/port modes - to ease the use. */
#define PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(_mode) ((_mode == CPSS_PORT_INTERFACE_MODE_KR2_E) || (_mode == CPSS_PORT_INTERFACE_MODE_CR2_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_CR2_C_E) || (_mode == CPSS_PORT_INTERFACE_MODE_KR2_C_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E))

/* From port manager perspective, around 100g are those speeds */
#define PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(_speed) (CPSS_PORT_SPEED_100G_E == _speed || CPSS_PORT_SPEED_102G_E == _speed \
               || CPSS_PORT_SPEED_107G_E == _speed)

/* Macro for port modes that use XPCS - to ease the use. */
#define PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(_mode) ((_mode == CPSS_PORT_INTERFACE_MODE_XGMII_E) || (_mode == CPSS_PORT_INTERFACE_MODE_RXAUI_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E))

/* memory allocation check macros */
#define PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, erFlag)      \
do                                                     \
{                                                      \
    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr == NULL)                   \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                      /*" perPhyPortDbPtr is NULL", portNum, __FUNCTION__);      */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum] == NULL)            \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                  /*" perPhyPortDbPtr[portNum] is NULL", portNum, __FUNCTION__);      */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum, erFlag)     \
do                                                   \
{                                                    \
    if (tmpPortManagerDbPtr->portMngSmDbPerMac == NULL)                         \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("(portMac %d)%s: "                    */\
                    /*" portMngSmDbPerMac is NULL", portMacNum, __FUNCTION__);    */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                       \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
    if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum] == NULL)                   \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                /*" portMngSmDbPerMac[portMacNum] is NULL", portMacNum, __FUNCTION__);  */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, serdesNum, erFlag)      \
do                                                   \
{                                                    \
  if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr == NULL)                 \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                      /*" perSerdesDbPtr is NULL", serdesNum, __FUNCTION__);    */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
  if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[serdesNum] == NULL)            \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                 /*" perSerdesDbPtr[serdesNum] is NULL", serdesNum, __FUNCTION__);   */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, erFlag)   \
do                                                 \
{                                                  \
  if (tmpPortManagerDbPtr->portMngSmDb == NULL)                          \
  {                                                \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                   */\
                      /*" portMngSmDb is NULL", portNum, __FUNCTION__);    */\
    if (erFlag)                                         \
    {                                              \
      return /*stub comment*/ GT_NOT_INITIALIZED;                       \
    }                                              \
    return GT_OK;                                        \
  }                                                \
  if (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL)                     \
  {                                                \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                   */\
                /*" portMngSmDb[portNum] is NULL", portNum, __FUNCTION__);      */\
    if (erFlag)                                         \
    {                                              \
      return /*stub comment*/ GT_NOT_INITIALIZED;                       \
    }                                              \
    return GT_OK;                                        \
  }                                                \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, erFlag)       \
do {                                                  \
  if (tmpPortManagerDbPtr->portsApAttributedDb == NULL)                        \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
        /*" portsApAttributedDb is NULL. erFlag=%d", portNum, __func__, erFlag);         */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
  if (tmpPortManagerDbPtr->portsApAttributedDb[portNum] == NULL)                   \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
          /*" portsApAttributedDb[portNum] is NULL. erFlag=%d", portNum, __func__, erFlag);  */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
} while (0)

#define PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR     CPSS_PORT_AP_FLOW_CONTROL_SYMMETRIC_E
#define PRV_PORT_MANAGER_DEFAULT_FC_PAUSE       GT_TRUE
#define PRV_PORT_MANAGER_DEFAULT_LANE_NUM       0
#define PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE GT_TRUE

static GT_BOOL linkLevel[CPSS_MAX_PORTS_NUM_CNS] = {0};
static GT_BOOL stageInit = GT_FALSE; /* prevent for PM log to print in loops the init stage, now its only print one time for a change */
static GT_U16 lastUpdate[CPSS_MAX_PORTS_NUM_CNS] = {0};

/************************************************
        External References
 ************************************************/
/* array defining serdes speed used in given interface mode for given port data speed */
extern CPSS_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];
extern MV_HWS_SERDES_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[];

extern GT_STATUS prvFindCauseRegAddrByEvent
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *regAddr
);

extern GT_STATUS prvFindIntCauseBit
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *bit
);

extern GT_STATUS prvFindInterruptIdx
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData,
    GT_U32         *interruptIdx
);

#if (defined CHX_FAMILY)
extern CPSS_PORT_AP_PARAMS_STC prvCpssPortApDefaultParams;
#endif

/************************************************
        Code
************************************************/

GT_STATUS prvCpssPortManagerMaskMacLevelEvents
(
    IN GT_U8          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    if (operation == CPSS_EVENT_UNMASK_E)
    {
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_DISABLE_MODE_E) ||
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LEVEL_EVENTS_DISABLE_MODE_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "unMasking MAC LEVEL EVENTS is not allowed, blocked by application");
            return GT_OK;
        }
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"%s mac level events on port",
                                          (operation == CPSS_EVENT_MASK_E) ? "Masking" : "Unmasking");

    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                             CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                             portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d LINK_STATUS_CHANGE returned %d ", portNum, operation, rc);
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerMaskLowLevelEvents
(
    IN GT_U8          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    if (operation == CPSS_EVENT_MASK_E)
    {

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"%s SIGNAL_DETECT,ALIGN_LOCK,GB_LOCK ",
                                              (operation == CPSS_EVENT_MASK_E) ? "Masking" : "Unmasking" );

        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d ALIGN_LOCK_LOST returned %d ", portNum, operation, rc);
        }


        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d GB_LOCK_SYNC_CHANGE returned %d ", portNum, operation, rc);
        }

        /* mask signal detect */
        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d SIGNAL_DETECT_CHANGE returned %d ", portNum, operation, rc);
        }
    }
    else /* CPSS_EVENT_UNMASK_E */
    {
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_DISABLE_MODE_E) ||
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_LOW_LEVEL_EVENTS_DISABLE_MODE_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "unMasking LOW LEVEL EVENTS is not allowed, blocked by application");
            return GT_OK;
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"%s SIGNAL_DETECT, ALIGN_LOCK, GB_LOCK ",
                                              (operation == CPSS_EVENT_MASK_E) ? "Masking" : "Unmasking");
        /* mask signal detect */
        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d SIGNAL_DETECT_CHANGE returned %d ", portNum, operation, rc);
        }



        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d ALIGN_LOCK_LOST %d returned %d ", portNum, operation, rc);
        }


        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d GB_LOCK_SYNC_CHANGE returned %d ", portNum, operation, rc);
        }



    }

    return GT_OK;
}


GT_STATUS prvCpssPortManagerMaskEvents
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_EVENT_MASK_SET_ENT  operation
)
{
    GT_STATUS rc;
    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
    }
    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
    }
    return GT_OK;
}

/**
* @internal prvCpssPortManagerInterruptAdditionals function
* @endinternal
*
* @brief  API to associate all port MACs of a multi-lane interface.
*     For example, 40Gig port interface combined of total 4 ports, 10G port
*     combined of 1 port.
*     This API helps port manager to manage interrupts of a group of ports as if
*     they are raised within the first port in the group which is the port that
*     the interface mode is configured upon.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
*                   portNum  - physical port number
*                   operation - mask operation (mask or unmask)
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerInterruptAdditionals
(
    IN GT_U8  devNum,
    IN GT_U32  macPortNum,
    OUT GT_U32  *arrPtr,
    OUT GT_U8  *arrSize
)
{
    CPSS_PORT_SPEED_ENT       speed;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    GT_STATUS            rc;
    GT_PHYSICAL_PORT_NUM      portNum;
    GT_U32             lane;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_STANDARD      portMode;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;

    *arrSize = 0;

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("[MacPort %d] device not applicable for additionals convertion mac. Returning.",macPortNum);
        return GT_OK;
    }

    /* getting port manager database from the main pp structure */
    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    CPSS_PORT_MANAGER_LOG_MASK_CONVERT_INFORMATION_MAC("[MacPort %d] physical port to real physical port convert",macPortNum);

    /* convert mac to phy */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                           macPortNum, &portNum);

    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling ppMacConvertFunc on port returned %d ", macPortNum, rc);
    }

    /* get port speed */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &speed);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %2d] Calling ppSpeedGetFromExtFunc on port returned %d ", portNum, rc);
    }

    /* get port interface mode */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppIfModeGetFromExtFunc on port returned %d ", portNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);

    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_NA_E
        && tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"InterruptAdditionals: port not managed by port manager");
        return GT_OK;
    }

    /* if the given port is a multi-lane port, the associated ports should
      be also masked\unmasked */
    if (speed != CPSS_PORT_SPEED_NA_E || /* when port created in CPSS db */
        tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb != CPSS_PORT_SPEED_NA_E /* when port only been set with params */ )
    {
        if (speed == CPSS_PORT_SPEED_NA_E)
        {
            ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
            speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
        }

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssLion2CpssIfModeToHwsTranslate returned rc %d",portNum, rc );
        }

        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, macPortNum, portMode, &curPortParams))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] hwsPortModeParamsGetToBuffer returned null", portNum );
        }

        CPSS_PORT_MANAGER_LOG_MASK_CONVERT_INFORMATION_MAC("[MacPort %d] NumOfActLanes(total interrupts) %d, portMode %d, speed %d"
                                                           ,macPortNum,curPortParams.numOfActLanes, ifMode, speed);
        /* Filling OUT data */
        *arrSize = curPortParams.numOfActLanes;
        for (lane=0; lane<curPortParams.numOfActLanes; lane++)
        {
            arrPtr[lane] = macPortNum + lane;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerIntMaskAdditionalConfiguration function
* @endinternal
*
* @brief  API to perform additional configuration when port is being masked
*     and unmasked.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] operation        - mask (mask or unmask)
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerIntMaskAdditionalConfiguration
(
    IN GT_U8          devNum,
    IN GT_U32          portNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    devNum = devNum;
    portNum = portNum;
    operation = operation;
/* code currently not needed */
#if 0
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"configuration additional mask operation=%d", operation);

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] device not applicable for"
                                                  "mask aditional configuration. Returning.",portNum);
        return GT_OK;
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);

    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_NA_E
        && tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port not managed by port manager"
                                              " mask operation=%d", operation);
        return GT_OK;
    }

    rc = prvCpssPortXlgMacMaskConfig(devNum, portNum,(operation == CPSS_EVENT_MASK_E) ? GT_TRUE : GT_FALSE /*restore*/);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"MaskAdditionalConfiguration: Calling prvCpssPortXlgMacMaskConfig"
                                              "returned=%d, so just returning ", rc);
    }
#endif
    return GT_OK;
}

/**
* @internal prvCpssPortMacEventConvertImpl function
* @endinternal
*
* @brief  API to convert MAC extended data for port related unified events.
*     This API is needed in order for a multi-lane interface mode
*     to receive interrupts from all of it's lanes and in order for
*     them to be received as if they are raised from the port related
*     to the multi-lane interface.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in,out] macPortNum        - (pointer to) the extended data of the interrupt
* @param[in,out] macPortNum        - (pointer to) the converted extended data of the interrupt
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR        - on null pointer
*/
static GT_STATUS prvCpssPortMacEventConvertImpl
(
    IN  GT_U8 devNum,
    INOUT GT_U32 *macPortNum
)
{
    CPSS_PORT_SPEED_ENT       speed;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    GT_STATUS            rc;
    GT_BOOL             found;
    GT_PHYSICAL_PORT_NUM      portNumTemp;
    GT_PHYSICAL_PORT_NUM      portNum;
    GT_U32             portMacNumTemp;
    GT_U32             i;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_STANDARD      portMode;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[MacPort %d] device not applicable for convertion mac. Returning.",*macPortNum);
        return GT_OK;
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(macPortNum);

    portMacNumTemp = 0;
    portNumTemp = 0;
    found = GT_FALSE;

    CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC("[MacPort %d] physical port to real physical port convert",*macPortNum);

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                           *macPortNum, &portNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("[MacPort %d] Calling ppMacConvertFunc on port returned %d ", *macPortNum, rc);
    }
#if 0
    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);

    /* if port is not managed by port manager, returning */
    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_NA_E
        && tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"PortMacEventConvert: port not managed by port manager");
        return GT_OK;
    }
#else
    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, *macPortNum, 0);

    if (tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"PortMacEventConvert: port not managed by port manager");
        return GT_OK;
    }
#endif

    /*rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);*/
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &speed);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"[MacPort %d] Calling ppSpeedGetFromExtFunc on port returned %d ", *macPortNum, rc);
    }
    /*rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);*/
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"[MacPort %d] Calling ppIfModeGetFromExtFunc on port returned %d ", *macPortNum, rc);
    }

    /* Maybe this physical port is just a lane on another physical port */
    if (speed==CPSS_PORT_SPEED_NA_E)
    {
        CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC("[MacPort %d] physical port to real physical port convert, searching for parent",*macPortNum);
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E/*PRV_CPSS_EV_CONVERT_TYPE_PHY_TO_MAC_E*/, portNum, &portMacNumTemp);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling ppMacConvertFunc on port returned %d ", rc);
            portMacNumTemp = *macPortNum;
        }

        /* Searching for a physical port which is configured to multi lane speed. If found, and
          this physical port has a mac which is in offset of up to 3 from our port mac,
          than our physical port is must be a lane of it.*/
        for (i=1; i<4; i++)
        {
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                                   (portMacNumTemp - i), &portNumTemp);

            if (rc==GT_OK)
            {
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNumTemp, &speed);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNumTemp,"Calling ppSpeedGetFromExtFunc on port returned %d ", rc);
                }

                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNumTemp, &ifMode);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNumTemp,"Calling ppIfModeGetFromExtFunc on port returned %d ", rc);
                }

                if (speed != CPSS_PORT_SPEED_NA_E && ifMode != CPSS_PORT_INTERFACE_MODE_NA_E && ifMode != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
                {
                    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"prvCpssCommonPortIfModeToHwsTranslate returned rc %d", rc );
                    }

                    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, (portMacNumTemp - i), portMode, &curPortParams))
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"hwsPortModeParamsGetToBuffer returned null" );
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }

                    if (curPortParams.numOfActLanes > 1 &&
                        ( curPortParams.numOfActLanes >= (*macPortNum - (portMacNumTemp - i)) ))
                    {

                        /*CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("(portMac %d) port to real port convert."
                            " NumOfActLanes %d, portMode %d, speed %d, iteration %d, parentPhyPortNum: %d"
                            ,*macPortNum,curPortParams.numOfActLanes, ifMode, speed, i, portNumTemp);*/

                        found = GT_TRUE;
                        *macPortNum = (portMacNumTemp - i);
                        break;
                    }
                }
            }
        }
        if (found == GT_FALSE)
        {
            CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC("[MacPort %d] is not a part of any interface %s ",
                                                                *macPortNum,"probably a part of deleted interface and was triggered by such operation");
        }
    }
    else
    {
        CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC("[MacPort %d] physical port to real physical port convert, no need to search. end.",*macPortNum);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortMacEventConvert function
* @endinternal
*
* @brief  API to convert MAC extended data for port related unified events.
*     This API is needed in order for a multi-lane interface mode
*     to receive interrupts from all of it's lanes and in order for
*     them to be received as if they are raised from the port related
*     to the multi-lane interface.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortMacEventConvert(GT_U8 devNum, GT_U32 *macPortNum)
{
    GT_STATUS rc;

    /* CPSS_API_LOCK_MAC protection done in function where this callback
      is called - drvMacEventExtDataConvert */

    rc = prvCpssPortMacEventConvertImpl(devNum,macPortNum);

    return rc;
}

/**
* @internal prvCpssPortManagerPushEvent function
* @endinternal
*
* @brief  Push artificial event into event queues of CPSS.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup        - port group
* @param[in] portNum         - physical port number
* @param[in] extraData        - extra data relevant for the unified event
* @param[in] uniEv          - unified event to push
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerPushEvent
(
    IN GT_U8          devNum,
    IN GT_U32          portGroup,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32          extraData,
    IN CPSS_UNI_EV_CAUSE_ENT  uniEv
)
{
    GT_U32 interruptIdx;
    GT_STATUS rc;
    PRV_CPSS_DRV_EV_REQ_NODE_STC *evNodesPool;

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"pushing event uniEv %d", uniEv);

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerPushEvent not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    PRV_CPSS_INT_SCAN_LOCK();

    evNodesPool = prvCpssDrvPpConfig[devNum]->intCtrl.
                  portGroupInfo[CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS].intNodesPool;

    /* get interrupt index */
    rc = prvFindInterruptIdx(devNum, portGroup, uniEv/*CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E*/,
                             extraData/*portMacNum*/, &interruptIdx);

    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvFindInterruptIdx failed=%d", portNum, rc);
    }

    /* insert to event queue */
    rc = prvCpssDrvEvReqQInsert(evNodesPool, interruptIdx/*evIndex*/, GT_FALSE/*masked boolean*/);
    if (rc != GT_OK)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssDrvEvReqQInsert failed=%d", portNum, rc);
    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    return GT_OK;
}

/**
* @internal prvCpssPortXlgMacMaskConfig function
* @endinternal
*
* @brief  Configure XLG mac to enable PCS interrrupts to be raised from the port
*     associated with the given MAC index (which is an additional port in a
*     multi-lane interface and not the first port of the interface), and also,
*     mask every other interrupt on the mac.
*     The operations being done are:
*     (1) configuring mac dma mode field in the mac to allow interrupt to flow.
*     (2) masking all mac interrupts in order to get only PCS interrupts from
*     the port.
*     This function acts as a work-around in order to get all PCS interrupts
*     for every serdes in a given multi-lane port.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] restore         - whether or not to XLG mac state to previous
*                   state
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortXlgMacMaskConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL     restore
)
{
    GT_U32   regAddr;
    GT_U32   regValue;
    GT_U32   i;
    GT_U32   portGroupId;
    GT_STATUS  rc;
    GT_U32   portMacNum;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;
    PRV_CPSS_PORT_REG_CONFIG_STC  regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
#if 0
    CPSS_EVENT_MASK_SET_ENT     masked;
#endif
    MV_HWS_PORT_STANDARD      portMode;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    CPSS_PORT_SPEED_ENT       speed;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32             totalMacs;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortXlgMacMaskConfig not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* get port manager database */
    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* getting appropriate mac num for the fiven physical port num */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    /* getting port group id */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);

    /* getting port speed and interface mode */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppIfModeGetFromExtFunc failed=%d ", portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &speed);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %2d] Calling ppSpeedGetFromExtFunc failed=%d ", portNum, rc);
    }

    /* checking port legality */
    if ( ifMode == CPSS_PORT_INTERFACE_MODE_NA_E || speed == CPSS_PORT_SPEED_NA_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port illegal speed or interface mode"
                                                   "maybe not needed now", portNum);
    }

    /* getting hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",portNum, rc );
    }

    /* getting hws params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroupId , portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] hwsPortModeParamsGetToBuffer returned null ", portNum );
    }

    totalMacs = curPortParams.numOfActLanes;

    /* clear mac configuration array */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConfigClearFunc(&regDataArray[0]);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling ppMacConfigClearFunc on port returned %d ", rc);
    }

#if 0
    /* get the current mask configuration on the port. If application masked the port,
      no need to unmask the neighbous ports assosiated with the interface */
    rc = cpssEventDeviceMaskWithEvExtDataGet(devNum, CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                             portNum, &masked);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataGet failed=%d ", portNum, rc);
    }
/* code currently not needed */
    /* if port is masked, not performing work-around. If port is unmasked
      (and already been created while port was unmasked and WA was not
      performed) then performing work-around.

      general cases:
                  |   UNMASKED            | MASKED
       ----------------------|-----------------------------------------------------
      1. restore == GT_FALSE | (called upon creation of general  | not performing WA when
                  |unmasking operation and port is   | port is masked.
                  | already created)          |
                  | => performing WA          |
                  |                  |
      -----------------------|-----------------------------------------------------------------------------
      3. restore == GT_TRUE  | performing restore WA       | (A) if WA performed in port life-cycle
                  |                  | than restoring WA (B) else => not
                  |                  | performing restore WA
      -----------------------------------------------------------------------------------------------------
    */
    if ( (masked == CPSS_EVENT_MASK_E && restore == GT_FALSE) ||
         (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag==0 &&
          restore == GT_TRUE && masked == CPSS_EVENT_MASK_E)
       )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"prvCpssPortXlgMacMaskConfig: masking condition not met, returning. ");
        return GT_OK;
    }
#endif
    if (restore == GT_TRUE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"handle XLG - restoring values  totalMacs %d", totalMacs);
        for (i = 1; i < totalMacs; i++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum+i, 1);

            /* getting xlg mac mask register address */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppXlgMacMaskAddrGetFunc(devNum, portMacNum+i, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling funcXlgMacIntMaskGet ", portNum);
            }

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] regAddr unused", portNum);
            }

            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] handle XLG - restoring first value: 0x%08x, mac %d",portNum,
                                                      tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0], (portMacNum+i));
            /* setting previous xlg mac interrupt mask register values (restoring) */
            rc = prvCpssDrvHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling prvCpssDrvHwPpWriteRegister ", portNum);
            }


            /* setting previous mac dma mode value (restoring ) */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i)/*, PRV_CPSS_PORT_XLG_E*/, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppMacDmaModeRegAddrGetFunc failed %d ", portNum, rc);
            }

            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] handle XLG - restoring second value: 0x%08x, mac %d",portNum,
                                                      tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1], (portMacNum+i));
            if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1];
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 12;
            }

            /* performing mac configuration */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConfigurationFunc(devNum, portNum, &regDataArray[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppMacConfigurationFunc failed- %d", portNum, rc);
            }

            /* marking WA flag */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag = 0;
        }
    }
    else
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"handle XLG - NOT restoring values totalMacs %d", totalMacs);
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum, 1);

        if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag==1)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"handle XLG - NOT restoring values - already loaded. returning ");
            return GT_OK;
        }

        for (i = 1; i < totalMacs; i++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum+i, 1);

            /* getting xlg mac mask register address */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppXlgMacMaskAddrGetFunc(devNum, portMacNum+i, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling funcXlgMacIntMaskGet ", portNum);
            }

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] regAddr unused", portNum);
            }

            /* getting xlg mac interrupt mask register value */
            rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling prvCpssDrvHwPpReadRegister ", portNum);
            }

            /* saving xlg mac interrupt mask register value in order to save for later restore */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0] = regValue;
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] handle XLG - NOT restoring values - first Value: 0x%08x,",
                                                      " mac %d, regAddr: 0x%08x",
                                                      portNum, regValue, (portMacNum+i), regAddr);

            /* overriding xlg mac interrupt mask register value - mask all. Only pcs we want
              (controlled by xlg summary register - another register)*/
            rc = prvCpssDrvHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling prvCpssDrvHwPpWriteRegister ", portNum);
            }

            /* getting mac dma mode register address in order to get interrupts */
            /*regAddr = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i), PRV_CPSS_PORT_XLG_E);*/
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i)/*, PRV_CPSS_PORT_XLG_E*/, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppMacDmaModeRegAddrGetFunc #2 failed %d ", portNum, rc);
            }

            /* getting mac dma mode value in order to save for later restore */
            rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),portGroupId, regAddr,
                                                    12, 1, &regValue);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] handle xlg macs failed calling prvCpssDrvHwPpPortGroupGetRegField ", portNum);
            }

            /* saving mac dma mode value */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1] = regValue;
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] handle XLG - NOT restoring values - second Value: 0x%08x, mac %d",
                                                      portNum, regValue, (portMacNum+i));

            /* overriding mac dma mode value to get interrupts */
            if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = 0;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 12;
            }

            /* performing mac configuration */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConfigurationFunc(devNum, portNum, &regDataArray[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppMacConfigurationFunc failed- %d", portNum, rc);
            }

            /* marking WA flag */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag = 1;

        }
    }

    return GT_OK;

}

static GT_STATUS prvCpssPortStageTrainPerformPhase
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{

    GT_STATUS rc;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT tmpTuneStatus;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    /************************************************
        Training Results
    ************************************************/
    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    *tuneStatusPtr=CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;

    rc = cpssPortSerdesCyclicAutoTuneStatusGet(CAST_SW_DEVNUM(devNum), portNum,
                                               (tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune), &tmpTuneStatus);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"cpssPortSerdesCyclicAutoTuneStatusGet failed %d ", rc );
        return rc;
    }

    /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Training check, status: %d ", portNum, tmpTuneStatus);*/
    /* we will perform next training mode only if we past previous one */
    if (tmpTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_PASS_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling tune2results on port. Training check and next. tune %d. Phase %d. PASSED ",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.phase);

        if (1/*tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune != CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_LAST_E*/)
        {
            /*PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC *ptrR = trainCookiePtrEnclosingDB.cookiePtr;*/
            /* performing next training */
            /*rc = cpssDxChPortSerdesCyclicAutoTune(devNum, portNum,
                               &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);*/

            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.overrideEtParams =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.max_LF =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.min_LF =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF;


            rc = cpssPortSerdesCyclicAutoTune(CAST_SW_DEVNUM(devNum), portNum,
                                              &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"launched tune: %d ", tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune);

            if (rc != GT_OK)
            {
                /* overall status failed */
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
                /*  CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Calling auto tune port %d returned error code %d ",
                               portNum, rc);*/
            }
            else
            {
                /* overall status not yet finished */
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E;
            }
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune
                == CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
            {
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            }
        }
        else
        {
            /* overall status finished successfully */
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
        }

    }
    else if (tmpTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling tune2results. Training check and next. tune %d. Phase %d. FAILED",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.phase);
        /* overall status failed */
        *tuneStatusPtr =CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
    }
    else
    {
        /* overall status not yet finished */
        *tuneStatusPtr =CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E;
    }

    return GT_OK;
}

static GT_STATUS prvCpssPortStageTrainMngPhases
(
    GT_PHYSICAL_PORT_NUM portNum,
    GT_SW_DEV_NUM devNum,
    IN CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{
    /* Variables declarations */
    GT_STATUS rc;
    GT_U32 timerSecondsNow, timerNsecNew;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = prvCpssPortStageTrainPerformPhase(devNum, portNum, tuneStatusPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling prvCpssPortStageTrainPerformPhase returned %d", rc);
        return rc;
    }
    /**************************************************************************************************
       If training passed, check port statuses good then enabling port. Else, not advancing
    **************************************************************************************************/
    /* Train pass */
    if (CPSS_PORT_SERDES_AUTO_TUNE_PASS_E == *tuneStatusPtr)
    {
        /* Checking overall port statuses */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"training passed ");

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;


        return GT_OK;
    }
    /***********************************************************************
        3. If training failed, not advancing
    ***********************************************************************/
    /* Train fail */
    else if (CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E == *tuneStatusPtr)
    {
        /* Thats it. returning to wait for signal detection */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"training failed on port");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;


        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;

        return GT_OK;
    }

    /***********************************************************************
        4. If training timed-out, not advancing
    ***********************************************************************/
    /* Not ready - continue to poll */
    else
    {
        /* Check Timeout */
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] >= 2)
        {
            /* Thats it. returning to wait for signal detection */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"training timed-out");
            /** NEED TO ENSURE ANY TRAINING MODE WILL BE STOPPED NOW */
            /*appDemoPortInitSeqDisablePortStage(devNum, portNum);*/
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

            /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(configureInputParams->portsBmpPtr, portNum);*/
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;

            return GT_OK;
        }
    }


    /* Initialize timers if this is the first iteration in the algorithm */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev==0 &&
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev==0)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] FATAL timer TrainRes #1 ", portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow,&timerNsecNew);
    if (rc!=GT_OK)
    {
        /* Thats it. returning to wait for signal detection */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"training timed-out TrainRes");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;

        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;
    }

    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        ( 1000000000 - (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev) + timerNsecNew );
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev) );
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0]+=
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;

    if(tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] > (4000000000UL))
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1]++;
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer +=
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer >= 10000000)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries++;
    }

    return GT_OK;
}

static GT_STATUS prvCpssPortStatusesCheck
(
    GT_SW_DEV_NUM    devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC *portStatusesPtr,
    GT_U32 unitsBitmapPtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portGroup;
    GT_STATUS rc;
    GT_BOOL signalState;
    GT_BOOL pcsSyncState, anyLb;
    CPSS_PORT_INTERFACE_MODE_ENT ifModeDb;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;

    GT_U32 portMacNum = 0;
    regAddr = 0;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    /*PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);*/
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }
    ifModeDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);
    rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum),portNum,CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,&loopbackCfg, &anyLb);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   portNum, rc);
    }

    portStatusesPtr->gbLock = GT_FALSE;
    portStatusesPtr->sigDet = GT_FALSE;
    portStatusesPtr->alignLock = GT_FALSE;
    portStatusesPtr->overallStatusesOk = GT_FALSE;

    /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"inside port statuses get function ");*/
    /**********************************************************
      Serdes Signal Detect, PCS GB lock, PCS align lock
    **********************************************************/
    if((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) &&
       (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E) &&
       (loopbackCfg.enableRegularTrafic == GT_FALSE))
    {   /* no need to check pcs and mac*/
        portStatusesPtr->gbLock = GT_TRUE;
        portStatusesPtr->alignLock = GT_TRUE;
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "gbLock and alignLock are ignored because loopback mode");

    }
    else
    {
        if (unitsBitmapPtr & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E))
        {
            /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting gb_lock status");*/
            if (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb)))
            {
                /* in case of RS_FEC/ FEC_OFF with R2 we don't have GearBox lock */
                if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_RS_FEC_MODE_ENABLED_E) ||
                    ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) &&
                     PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifModeDb)))
                {
                    portStatusesPtr->gbLock = GT_TRUE;
                }
                else
                {
                    /* rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortGbStatusGetFunc(devNum, portNum, &(portStatusesPtr->gbLock));*/
                    rc = prvCpssPortPcsGearBoxStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum, &(portStatusesPtr->gbLock));
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppPortGbStatusGetFunc:rc=%d\n",
                                                                   portNum, rc);
                    }
                }
            }
            else
            {
                portStatusesPtr->gbLock = GT_TRUE;
            }
        }

        if (unitsBitmapPtr & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E))
        {
            if (PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb))
            {
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCgConvertersStatus2GetFunc(devNum,portNum,&regAddr);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppCgConvertersStatus2GetFunc:rc=%d",
                                                               portNum,rc);
                }

                rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0, 1, &value);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssDrvHwPpPortGroupGetRegField(CG PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E):rc=%d\n",
                                                               portNum, rc);
                }
                if (value&1)
                {
                    portStatusesPtr->alignLock = GT_TRUE;
                }
            }
            else
            {
                /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E status");*/

                /*regAddr*/
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMpcs40GCommonStatusRegGetFunc(devNum,portNum, &regAddr);/*PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].mpcs40GCommonStatus;*/
                /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ADDR: 0x%08x ", regAddr);*/
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppMpcs40GCommonStatusRegGetFunc(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E):rc=%d\n",
                                                               portNum, rc);
                }
                rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0, 1, &value);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E):rc=%d\n",
                                                               portNum, rc);
                }
                if (value&1)
                {
                    portStatusesPtr->alignLock = GT_TRUE;
                }
            }
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] align_lock reg addr: 0x%08x",portNum, regAddr);

        }
    }

    if (unitsBitmapPtr & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E))
    {
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting PRV_CPSS_PORT_MANAGER_SIG_DET_E status");*/
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSigDetGetFunc(devNum, portNum, &(portStatusesPtr->sigDet));
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppSigDetGetFunc(PRV_CPSS_PORT_MANAGER_SIG_DET_E): rc=%d\n",
                                                           portNum, rc);
            }
        }
        else
        {
            signalState = GT_FALSE;
            pcsSyncState = GT_FALSE;
            /* rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortPcsSyncStatusGetFunc(devNum, portNum, &pcsSyncState);*/
            rc = prvCpssPortPcsSyncStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum, &pcsSyncState);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"cpssDxChPortPcsSyncStatusGet:rc=%d", rc);
            }
            if (GT_TRUE == pcsSyncState)
            {
                /* check that Serdes Signal detected on port is OK and stable - on all its serdes lanes */
                rc = prvCpssPortPcsSyncStableStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum,&signalState);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"cpssDxChPortPcsSyncStableStatusGet:rc=%d", rc);
                }
                else
                {
                    if (signalState == GT_TRUE)
                    {
                        portStatusesPtr->sigDet = GT_TRUE;
                    }
                }
            }
        }
    }


    if (portStatusesPtr->gbLock == GT_TRUE && portStatusesPtr->sigDet == GT_TRUE &&
        portStatusesPtr->alignLock == GT_TRUE)
    {
        portStatusesPtr->overallStatusesOk = GT_TRUE;
    }


    return GT_OK;
}

static GT_STATUS prvCpssPortManagerStatusesStabilityCheckPhase
(
    GT_SW_DEV_NUM    devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC *portStatusesPtr,
    GT_U32         unitsBitmapPtr,
    GT_U32 totalPcs
)
{
   GT_STATUS rc;
    GT_U32 SD_Change_Interval;
    GT_U32 SD_Change_Min_Window_Size;
    GT_U32 SD_Check_Time_Expired;
    GT_U32 regAddrMultiple[4];
    GT_U32 dataMultiple[4];
    GT_U32 bitMultiple[4];
    GT_U32 timerNsecNew;
    GT_U32 timerSecondsNow;
    GT_U32 i;
    GT_U8 flag;
    CPSS_UNI_EV_CAUSE_ENT uniEv;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_BOOL signalOkChange = GT_FALSE;
    MV_HWS_PORT_STANDARD      portMode;

    GT_U32 portMacNum = 0;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    /*PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);*/
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    flag = 0;
    cpssOsMemSet(portStatusesPtr, 0, sizeof(PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC));

    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssLion2CpssIfModeToHwsTranslate returned rc %d",portNum, rc );
    }

    if (unitsBitmapPtr & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E))
    {
        SD_Change_Interval = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval;
        SD_Change_Min_Window_Size = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize;
        SD_Check_Time_Expired = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired;

        uniEv = 0;

        for (i=0; i<totalPcs; i++)
        {
            if ( PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed) )
            {
                uniEv = CPSS_PP_PORT_SYNC_STATUS_CHANGED_E;
            }
            else
            {
                uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
            }

            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                /* Keep int cause register address */
                rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), 0, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddrMultiple[i]);
                if (rc!=GT_OK)
                {
                  CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                }
                /* Keep offset in register of uni-event. */
                rc = prvFindIntCauseBit(CAST_SW_DEVNUM(devNum), 0, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &bitMultiple[i]);
                if (rc!=GT_OK)
                {
                  CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling prvFindIntCauseBit %d ", portMacNum+i, rc);
                }

                CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[MacPort %d] signal stability regAddr: 0x%08x, bit: 0x%08x", portMacNum+i, regAddrMultiple[i], bitMultiple[i]);
            }
        }


        /* Initialize timers if this is the first iteration in the algorithm */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev==0 &&
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev==0)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"signal stability numLanes %d - START", totalPcs);

            /* First time: clear cause bits.  */
            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                for (i=0; i<totalPcs; i++)
                {
                    rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), 0, regAddrMultiple[i], 0xFFFFFFFF, &dataMultiple[i]);
                    if (rc!=GT_OK)
                    {
                        rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum),uniEv, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                        }
                    }
                }
            }
            else
            {
                rc = mvHwsPortAvagoSerdesRxSignalOkChangeGet(CAST_SW_DEVNUM(devNum), 0, portMacNum,portMode,&signalOkChange);
                if(rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling mvHwsPortAvagoSerdesRxSignalOkChangeGet failed %d ", portMacNum, rc);
                }
            }

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
            rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev,
                              &tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev);
            if (rc!=GT_OK)
            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum), uniEv, portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet failed=%d ", portNum, rc);
                }
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Unmasking event %d as rollback failed #1", portNum, uniEv);
            }
        }

        /* Get current time, and update port diff-timer and total-time-from-start timer accordingly*/
        rc = cpssOsTimeRT(&timerSecondsNow, &timerNsecNew);
        if (rc!=GT_OK)
        {
            rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum),uniEv, portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet failed=%d ", portNum, rc);
            }
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Unmasking event %d as rollback failed #2", portNum, uniEv);
        }
        /* In case we in different time in meanning of a second, the difference in nano seconds
          is the following equation */
        if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = ( (1000000000 - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev) + timerNsecNew );
        }
        /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
        else
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime =
              ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev) /*/ 1000000*/ );
        }

        /* update the port seconds and nanosecs timers */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = timerSecondsNow;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = timerNsecNew;

        /* Intervals timer */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        /* updating total time spent */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        /* Check if interval of recheck reached */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer >= SD_Change_Interval*1000000)
        {
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] interval passed. Time diff from last interval: %d ", portNum,
                               tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer);
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer =0;

            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                for (i=0; i<totalPcs; i++)
                {
                    rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), 0, regAddrMultiple[i], 0xFFFFFFFF, &dataMultiple[i]);
                    if (rc!=GT_OK)
                    {
                        rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum),uniEv, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling "
                                      "cpssEventDeviceMaskWithEvExtDataSet returned=%d ", portNum, rc);
                        }

                        /* Reset timers */
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
                        return rc;
                    }

                    /* If signal_detect change happened */
                    if (dataMultiple[i] & bitMultiple[i])
                    {
                        flag = 1;
                    }
                }
            } /* for Avago devices read directly the SerDes interrupt register */
            else
            {
                rc = mvHwsPortAvagoSerdesRxSignalOkChangeGet(CAST_SW_DEVNUM(devNum), 0, portMacNum,portMode,&signalOkChange);
                if(rc != GT_OK)
                {
                    rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum),uniEv, portNum, CPSS_EVENT_UNMASK_E);
                    if(rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling "
                                  "cpssEventDeviceMaskWithEvExtDataSet returned=%d ", portNum, rc);
                    }
                    /* Reset timers */
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling mvHwsPortAvagoSerdesRxSignalOkChangeGet failed %d ", portMacNum, rc);
                }
                if (signalOkChange == GT_TRUE)
                {
                    flag = 1;
                }
            }

            if (flag == 1)
            {
                /* there was a change continue measure */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
            }

            /* Finished measure, signal is stable enought */
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange >= SD_Change_Min_Window_Size*1000000)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"signal stability - FINISHED time(%dms)",(tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange/1000000));

                /* Reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
                /* FINISHED :)) */
                portStatusesPtr->sigDet = GT_TRUE;
                return GT_OK;
            }

            /* If we are timed-out */
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart >= SD_Check_Time_Expired*1000000)
            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum),uniEv, portNum, CPSS_EVENT_UNMASK_E);
                if(rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Calling "
                            "cpssEventDeviceMaskWithEvExtDataSet returned=%d ", portNum, rc);
                }

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Overall time expiration: %dms ", (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart/1000000));
                /* Reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer=0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
                /* FINISHED :(( HW ERROR */
                portStatusesPtr->sigDet = GT_FALSE;
                return GT_OK;
            }

        }

    }
    return GT_OK;
}

/**
* @internal prvCpssPortManagerSigStableRxTraining function
* @endinternal
*
* @brief  Runs Rx training in order to stabilize signal detection.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerSigStableRxTraining
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;
    CPSS_PORT_INTERFACE_MODE_ENT ifModeDb;


    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"device not applicable for prvCpssPortManagerSigStableRxTraining. Returning.");
        return GT_OK;
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    ifModeDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Lunch iCal ");

    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) &&
        (PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifModeDb)))
    {
        rc = prvCpssPortSerdesLunchSingleiCal(devNum, portNum);
    }
    else
    {
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum,
                                                                               portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
    }

    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStageTrainMngPhases failed=%d ",
                                                   portNum, rc);
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = 0;

    return GT_OK;

}

/**
* @internal prvCpssPortManagerLoopbackModesHwSet function
* @endinternal
*
* @brief  Set loopback mode/s of the port to the HW.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
*                   portType    - AP or Non-AP port type
* @param[in] ifMode          - port interface mode
* @param[in] speed          - port speed
* @param[in] loopbackCfgPtr      - (pointer to) loopback configuration structure
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad pointer
*/
GT_STATUS prvCpssPortManagerLoopbackModesHwSet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN CPSS_PORT_SPEED_ENT speed,
    IN CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtr
)
{
    GT_U32 laneBmp, laneIdx;
    GT_STATUS rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtr);

    switch (loopbackCfgPtr->loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacLoopbackModeSetFunc(devNum, portNum,
                                                                                             loopbackCfgPtr->loopbackMode.macLoopbackEnable)));
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Could not set MAC Loopback", portNum);
        }
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        laneBmp = 0;
        rc = prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed(CAST_SW_DEVNUM(devNum),0,portNum,ifMode,speed,&curPortParams);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                       "[Port %d] prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed failed=%d",
                                                       portNum, rc);
        }

        /* filling local lanes bitmap */
        for (laneIdx=0; laneIdx<curPortParams.numOfActLanes; laneIdx++)
        {
            {
                laneBmp |= (1<<laneIdx);
            }
        }

        /* applying serdes loopback on all applicable local lanes */
        rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortSerdesLoopbackModeSetFunc(devNum, portNum,
                                                                                                laneBmp,
                                                                                                loopbackCfgPtr->loopbackMode.serdesLoopbackMode)));

        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] ppPortSerdesLoopbackModeSetFunc failed=%d",
                                                       portNum, rc);
        }
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %d] setting HW loopback failed type=%d",
                                                   portNum, loopbackCfgPtr->loopbackType);
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerEnableRemoteLink
(
    GT_SW_DEV_NUM devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT      speed,
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr,
    GT_BOOL enable
)
{
    GT_STATUS rc;
    if ((PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) && !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Aldrin */
        && (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode || CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode))
    {
        /* XAUI/RXAUI use XPCS which does not implement remote fault */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppRemoteFaultSetFunc was skipped since XPCS is used");
    }
    else if ((!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) &&
        (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb)))) /* for 100G we set local fault */
    {
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, (enable == GT_TRUE) ? GT_FALSE : GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppRemoteFaultSetFunc returned error code %d ", rc);
            return rc;
        }
    }

    /* Force link down */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppForceLinkDownSetFunc(devNum, portNum, (enable == GT_TRUE) ? GT_FALSE : GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"disabling force link down returned error code %d ", rc);
        return rc;
    }
    return rc;
}

GT_STATUS prvCpssPortManagerEnableLinkLevel
(
    GT_SW_DEV_NUM devNum,
    GT_U32 portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT      speed,
    MV_HWS_PORT_STANDARD  portMode,
    GT_BOOL enable
)
{
    GT_STATUS rc,rc_tmp;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_BOOL anyLb, doMacEn = GT_TRUE;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;


    portGroup=portGroup;
    portMode=portMode;
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Performing port-enable stage %s ", (enable == GT_TRUE) ? "enable" : "disable");

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(CAST_SW_DEVNUM(devNum), &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum),portNum,CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,&loopbackCfg, &anyLb);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   portNum, rc);
    }

    /* currently, we will only need 'anyLoopback' case and not per loopback mode so just avoid warning for now*/

    /* in loopback mode we will not force link-down nor force peer-link-down and we will enable mac */
    if ((anyLb==GT_TRUE) &&
        !((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E)))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "not performing link-level enabled-disable as there is a loopback on port");
        return GT_OK;
    }

    if (enable == GT_TRUE)
    {
        linkLevel[portNum] = GT_TRUE;
    }
    else
    {
        linkLevel[portNum] = GT_FALSE;
    }

    /*********************************************************
      Enable port [1/2] - Enable port's Work-Arounds
     ********************************************************/
#if 0
    if ( ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) ) ||
         ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) )
       )
    {
        rc = prvCpssDxChLion2PortLinkWa(devNum, portNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Calling WA prvCpssDxChLion2PortLinkWa on port %d returned error code %d ",
                         portNum, rc);
            return rc;
        }
    }
#endif

    /*********************************************************
      Enable port [2/2] - removing force downs and enabling port
     ********************************************************/
    rc = prvCpssPortManagerEnableRemoteLink(devNum,portNum,ifMode,speed,tmpPortManagerDbPtr,enable);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"prvCpssPortManagerEnableRemoteLink returned error code %d ", rc);
        return rc;
    }

    if((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) &&
       (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E))
    {
        if (enable)
        {
            if (loopbackCfg.enableRegularTrafic == GT_FALSE)
            {
                doMacEn = GT_FALSE;
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Port stay disabled because SERDES_LOOPBACK_DIGITAL_RX2TX mode");

            }
        }

    }

    if (doMacEn)
    {
        /* Enable port */
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, enable);
        if (rc != GT_OK)
        {
            if (enable == GT_FALSE)
            {
                rc_tmp = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, GT_TRUE);
                if (rc_tmp != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"enabling port returned error code %d ", rc_tmp);
                }
                rc_tmp = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, GT_FALSE);
                if (rc_tmp != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"enabling port returned error code %d ", rc_tmp);
                }
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssPortManagerLanePolarityHwSet function
* @endinternal
*
* @brief  set serdes lane rx and tx polarity to hw.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] globalSerdesIndex    - lane num
*                   rxInvert       - rx polarity data
*                   txInvert       - tx polarity data
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
#if 0
GT_STATUS prvCpssPortManagerLanePolarityHwSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U16 globalSerdesIndex
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.txInvert == GT_FALSE
        &&
        tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.rxInvert == GT_FALSE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "[SerDes Lane %d] setting serdes polarity: both FALSE so just returning", globalSerdesIndex);
        return GT_OK;
    }

    rc = mvHwsSerdesPolarityConfig(devNum, 0, globalSerdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum),
                                   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.txInvert,
                                   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.rxInvert);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[SerDes Lane %d] mvHwsSerdesPolarityConfig failed=%d", globalSerdesIndex, rc);
    }


    return GT_OK;
}
#endif
/**
* @internal prvCpssPortManagerLaneTuneDbGet function
* @endinternal
*
* @brief  Get serdes lane RxTx parameters from port manager data base.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
*                   globalSerdesIndex  - lane num
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR        - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32 localLaneIndex,
    IN GT_U32 serdesIndex,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    OUT CPSS_PORT_SERDES_TUNE_STC *serdesParamsPtr,
    OUT GT_BOOL *rxValid,
    OUT GT_BOOL *txValid
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC *serdesDbStcPtr;

    CPSS_NULL_PTR_CHECK_MAC(serdesParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(rxValid);
    CPSS_NULL_PTR_CHECK_MAC(txValid);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, serdesIndex, 1);
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        serdesDbStcPtr = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[serdesIndex];
    }
    else
    {

        serdesDbStcPtr = &tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perSerdesDb[localLaneIndex];
        /* TODO add support for AP + mode */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "Could not get Lane tune params in AP");
    }
    serdesParamsPtr->txAmp   = serdesDbStcPtr->portSerdesParams.serdesCfgDb.txAmp;
    serdesParamsPtr->txAmpAdj  = serdesDbStcPtr->portSerdesParams.serdesCfgDb.txAmpAdj;
    serdesParamsPtr->txEmphAmp = serdesDbStcPtr->portSerdesParams.serdesCfgDb.txEmphAmp;
    serdesParamsPtr->txEmph1  = serdesDbStcPtr->portSerdesParams.serdesCfgDb.txEmph1;
    serdesParamsPtr->txAmpShft = serdesDbStcPtr->portSerdesParams.serdesCfgDb.txAmpShft;

    serdesParamsPtr->BW = serdesDbStcPtr->portSerdesParams.serdesCfgDb.BW;
    serdesParamsPtr->HF = serdesDbStcPtr->portSerdesParams.serdesCfgDb.HF;
    serdesParamsPtr->LF = serdesDbStcPtr->portSerdesParams.serdesCfgDb.LF;
    serdesParamsPtr->LB = serdesDbStcPtr->portSerdesParams.serdesCfgDb.LB;
    serdesParamsPtr->DC = serdesDbStcPtr->portSerdesParams.serdesCfgDb.DC;
    serdesParamsPtr->sqlch = serdesDbStcPtr->portSerdesParams.serdesCfgDb.sqlch;
    serdesParamsPtr->align90 = serdesDbStcPtr->portSerdesParams.serdesCfgDb.align90;

    *rxValid = serdesDbStcPtr->portSerdesParams.rxValid;
    *txValid = serdesDbStcPtr->portSerdesParams.txValid;

    /* *portMngSerdesParamsPtrPtr = &(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams); */

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLaneTuneDbSet function
* @endinternal
*
* @brief  Set serdes lane RxTx parameters in port manager data base.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] globalSerdesIndex    - serdes number
*                   rxParams       - whether or not the parameters referred to rx paremeters
* @param[in] tuneValuesPtr      - (pointer to) serdes RxTx parameters
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR        - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 localLaneIndex,
    IN GT_U32 globalSerdesIndex,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    IN PRV_CPSS_PORT_MANAGER_SERDES_PARAMS_TYPE_ENT paramsType,
    IN CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC *serdesDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E)
        {
            if ( tuneValuesPtr->sqlch    > 310 ||
                 tuneValuesPtr->DC      > 255 ||
                 tuneValuesPtr->LF      > 15 ||
                 tuneValuesPtr->HF      > 15 ||
                 tuneValuesPtr->BW      > 15 ||
                 tuneValuesPtr->LB      > 15)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %2d] RX param out of range for device", portNum);
            }
        }
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E)
        {
            tuneValuesPtr->txAmpAdj = GT_FALSE;
            tuneValuesPtr->txAmpShft = GT_FALSE;
            if (tuneValuesPtr->txEmphAmp  > 32 ||
                tuneValuesPtr->txEmph1   > 32 ||
                tuneValuesPtr->txEmphAmp  < -32 ||
                tuneValuesPtr->txEmph1   < -32 ||
                ((GT_32)tuneValuesPtr->txAmp)    > 31 )
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %2d] TX param out of range for device", portNum);
            }
        }
    }
    else
    {
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E)
        {
            if (tuneValuesPtr->ffeC     >= BIT_4 ||
                tuneValuesPtr->ffeR     >= BIT_3 ||
                tuneValuesPtr->align90   >= BIT_7)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "non sip5_15 out of range RX");
            }
        }
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E)
        {
            /* for now just these parameters supported in Lion2 */
            if (tuneValuesPtr->txEmphAmp  >= BIT_4 ||
                tuneValuesPtr->txEmph1   >= BIT_4 ||
                tuneValuesPtr->txAmp    >= BIT_5 ||
                tuneValuesPtr->txAmpAdj   >= BIT_1)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "non sip5_15 out of range TX");
            }
        }
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        serdesDbPtr = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex];
    }
    else
    {
        serdesDbPtr = &tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perSerdesDb[localLaneIndex];
        /* TODO add support for AP + mode */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "Could not get Lane tune params in AP");
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E)
        {
            /* ctle params */
            serdesDbPtr->portSerdesParams.serdesCfgDb.BW = tuneValuesPtr->BW;
            serdesDbPtr->portSerdesParams.serdesCfgDb.DC = tuneValuesPtr->DC;
            serdesDbPtr->portSerdesParams.serdesCfgDb.HF = tuneValuesPtr->HF;
            serdesDbPtr->portSerdesParams.serdesCfgDb.LF = tuneValuesPtr->LF;
            serdesDbPtr->portSerdesParams.serdesCfgDb.LB = tuneValuesPtr->LB;
            serdesDbPtr->portSerdesParams.serdesCfgDb.sqlch = tuneValuesPtr->sqlch;
            serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
        }
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E)
        {
            /* tx params */
            serdesDbPtr->portSerdesParams.serdesCfgDb.txAmp = tuneValuesPtr->txAmp;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txEmphAmp = tuneValuesPtr->txEmphAmp;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txEmph1 = tuneValuesPtr->txEmph1;
            serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
        }
    }
    else
    {
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E)
        {
            /* ctle params */
            serdesDbPtr->portSerdesParams.serdesCfgDb.ffeC = tuneValuesPtr->ffeC;
            serdesDbPtr->portSerdesParams.serdesCfgDb.ffeR = tuneValuesPtr->ffeR;
            serdesDbPtr->portSerdesParams.serdesCfgDb.sqlch = tuneValuesPtr->sqlch;
            serdesDbPtr->portSerdesParams.serdesCfgDb.align90 = tuneValuesPtr->align90;
            serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
        }

        /* 'sampler' field - not for cpss_4.2 - not argument in params set in HWS,
          only ffeC, ffeR, sqlch, dfeEn=TRUE, align90 */
        if (paramsType == PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E ||
            paramsType == PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E)
        {
            /* tx params */
            serdesDbPtr->portSerdesParams.serdesCfgDb.txAmpAdj = tuneValuesPtr->txAmpAdj;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txAmpShft = tuneValuesPtr->txAmpShft;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txAmp = tuneValuesPtr->txAmp;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txEmphAmp = tuneValuesPtr->txEmphAmp;
            serdesDbPtr->portSerdesParams.serdesCfgDb.txEmph1 = tuneValuesPtr->txEmph1;
            serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLaneTuneHwSet function
* @endinternal
*
* @brief  Set serdes lane RxTx parameters from port manager data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup        - port group number
* @param[in] portNum         - physical port number
*                   laneNum       - lane number
* @param[in] rxSet          - whether or not to set the rx parameters
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLaneTuneHwSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_PHYSICAL_PORT_NUM portNum,
    MV_HWS_PORT_STANDARD portMode,
    IN GT_U32 globalSerdesIndex,
    IN GT_BOOL rxSet
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    if (rxSet==GT_TRUE)
    {
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            rc = mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, globalSerdesIndex,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.DC,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.LF,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.HF,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.BW,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.LB,
                                                  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.sqlch);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsAvagoSerdesManualCtleConfig failed=%d", portNum, rc);
            }
        }
        else
        {
            rc = mvHwsSerdesManualRxConfig(devNum, portGroup, globalSerdesIndex,
                                           HWS_DEV_SERDES_TYPE(devNum),
                                           (((CPSS_PORT_INTERFACE_MODE_KR_E ==
                                              tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb) ||
                                             _40GBase_CR4 == portMode) ?
                                            StaticLongReach : StaticShortReach),
                                           tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.sqlch,
                                           tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.ffeR,
                                           tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.ffeC,
                                           GT_TRUE,
                                           tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.align90);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsSerdesManualRxConfig failed=%d", portNum, rc);
            }
        }
    }
    else
    {
        rc = mvHwsSerdesManualTxConfig(devNum, portGroup, globalSerdesIndex,
                                       HWS_DEV_SERDES_TYPE(devNum),
                                       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txAmp,
                                       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txAmpAdj,
                                       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txEmphAmp,
                                       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txEmph1,
                       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txAmpShft,
                                       GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsSerdesManualTxConfig failed=%d", portNum, rc);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerLaneCiRxTraining
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 globalSerdesIndex,
    IN MV_HWS_SERDES_SPEED serdesSpeed,
    IN GT_BOOL rxOverride
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    if (rxOverride)
    {
        rc = mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, globalSerdesIndex,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.DC,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.LF,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.HF,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.BW,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.LB,
                                              tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.sqlch);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsAvagoSerdesManualCtleConfig failed=%d", portNum, rc);
        }
    }
    else
    {
        rc = mvHwsAvagoSerdesManualCtleConfig(devNum,portGroup,globalSerdesIndex,
                hwsAvagoSerdesTxRxTuneParamsArray[serdesSpeed].ctleParams.dcGain,
                hwsAvagoSerdesTxRxTuneParamsArray[serdesSpeed].ctleParams.lowFrequency,
                hwsAvagoSerdesTxRxTuneParamsArray[serdesSpeed].ctleParams.highFrequency,
                hwsAvagoSerdesTxRxTuneParamsArray[serdesSpeed].ctleParams.bandWidth,
                0 /* loopbackBandwidth */,
                hwsAvagoSerdesTxRxTuneParamsArray[serdesSpeed].ctleParams.squelch);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsAvagoSerdesManualCtleConfig failed=%d", portNum, rc);
        }
    }


    rc = mvHwsAvagoSerdesRxAutoTuneStart(devNum,portGroup,globalSerdesIndex, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsAvagoSerdesRxAutoTuneStart failed=%d", portNum, rc);
    }

    return GT_OK;

}


GT_STATUS prvCpssPortManagerResetOpticalCalCounters
(
    GT_PHYSICAL_PORT_NUM  portNum,
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr
)
{
    GT_U32 i;
    for (i = 0; i < PORT_MANAGER_MAX_LANES; i++)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[i] = GT_FALSE;
    }
    tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_FALSE;

    return GT_OK;
}

static GT_STATUS prvCpssPortManagerOpticalCalibrationSet
(
    GT_SW_DEV_NUM    devNum,
    GT_U32 portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U32 localLaneIdx,
    GT_U32 globalSerdesIndex,
    GT_BOOL  *calibrationFinished
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;
    GT_BOOL needTraining = GT_FALSE;
    CPSS_PORT_SERDES_TUNE_STC serdesParams;
    GT_BOOL rxValid, txValid;


    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortSerdesTuneResultGetFunc(devNum,portNum,localLaneIdx,&tuneValues)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get Tuning Result");
    }
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum," [serdes %2d] serdesSpeed %d EO %d HF %d LF %d",serdesSpeed, localLaneIdx, tuneValues.EO, tuneValues.HF, tuneValues.LF);
    *calibrationFinished = GT_FALSE;
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_LF_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_LF_OFFSET_E) & 0x1))
    {
        if ((tuneValues.LF < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minLfThreshold) ||
            (tuneValues.LF > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxLfThreshold))
        {
            /* signal is not in confidence range */
            needTraining = GT_TRUE;
        }

    }
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_HF_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_HF_OFFSET_E) & 0x1))
    {
        if ((tuneValues.HF < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minHfThreshold) ||
            (tuneValues.HF > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxHfThreshold))
        {
            /* signal is not in confidence range */
            needTraining = GT_TRUE;
        }

    }
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_EO_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_EO_OFFSET_E) & 0x1))
    {
        if ((tuneValues.EO < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minEoThreshold) ||
            (tuneValues.EO > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxEoThreshold))
        {
            needTraining = GT_TRUE;
            /* signal is not in confidence range */
        }
    }

    if (needTraining)
    {
        /* check if application overrides default RX data */
        rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, localLaneIdx,
                                             globalSerdesIndex, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &serdesParams, &rxValid, &txValid);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx",portNum,rc);
        }

        prvCpssPortManagerLaneCiRxTraining(CAST_SW_DEVNUM(devNum),portGroup,portNum,globalSerdesIndex, serdesSpeed,rxValid);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneCiRxTraining failed", portNum);
        }
    }
    else
    {
        *calibrationFinished = GT_TRUE;
    }



    return GT_OK;
}

/**
* @internal prvCpssPortManagerOpticalStabilityCheck function
* @endinternal
*
* @brief  Run optical stability Algorithm
*
* @note  APPLICABLE DEVICES:   Pipe; Caelum; Aldrin; AC3X;
*        Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
static GT_STATUS prvCpssPortManagerOpticalStabilityCheck
(
    GT_SW_DEV_NUM         devNum,
    GT_U32                portGroup,
    GT_PHYSICAL_PORT_NUM  portNum,
    MV_HWS_PORT_STANDARD  portMode,
    CPSS_PORT_SPEED_ENT   speed
)
{
        GT_STATUS rc;
        GT_U32 portMacNum;
        GT_BOOL rxSgnlOk = GT_FALSE;
        GT_BOOL rxSgnlDetect = GT_FALSE;
        MV_HWS_PORT_INIT_PARAMS     curPortParams;
        PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
        GT_U32 localLaneIdx;
        GT_BOOL calibrationFinished = GT_FALSE;
        GT_U32 timerSec;
        GT_U32 timerNsec;
        GT_U32 timeOutSec;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

        rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
        }

        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
        }

        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "[Port %2d] hwsPortModeParamsGetToBuffer returned null", portNum );
        }

        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew == 0) &&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev == 0))
        {
            rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev,
                         &tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew);
            if (rc != GT_OK)
            {
                /* reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                }
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Timer issue - moving to link down", portNum);
            }


        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimeoutSec > 0)
        {
            timeOutSec = tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimeoutSec;
        }
        else
        {
            timeOutSec = 10;
        }

        rc = cpssOsTimeRT(&timerSec,&timerNsec);
        if (rc != GT_OK)
        {
            /* reset timers */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
            }
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Timer issue - moving to link down", portNum);
        }

        if (timerSec - tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev > timeOutSec)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
            }
            return GT_OK;
        }

         /* mask low level interrupts */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask == GT_FALSE)
        {
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
            }
            /* avoid spamming the log */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_TRUE;
        }

        if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)))
        {
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc(devNum, portNum,
                                                                                      &rxTuneStatus, &txTuneStatus);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform ppTuneStatusGetFunc operation", portNum);
            }
            if (rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
            {
                return GT_OK;
            }
        }

        rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSgnlOk);
        if (rc != GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] cpssDrvPpHwRegBitMaskRead failed=%d", portNum, rc);
        }

        if (rxSgnlOk == GT_FALSE)
        {
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSigDetGetFunc(devNum, portNum, &rxSgnlDetect);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppSigDetGetFunc(PRV_CPSS_PORT_MANAGER_SIG_DET_E): rc=%d\n",
                                                           portNum, rc);
            }
            if (rxSgnlDetect)
            {
                return GT_OK;
            }
            else
            {
                /* if there is no signal move to link down state, only low_level_event will recover the port */
                if (!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed))
                {
                    rc = prvCpssPortManagerSigStableRxTraining(CAST_SW_DEVNUM(devNum), portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", portNum, rc);
                    }
                }
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"moves to link_down state, no signal detect");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                /* reset optical counters */
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
                /* mask low level interrupts (working on mac lvl only)*/
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                }

                return GT_OK;
            }
        }
        for (localLaneIdx = 0; localLaneIdx < curPortParams.numOfActLanes; localLaneIdx++)
        {
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] == GT_FALSE)
            {
                rc = prvCpssPortManagerOpticalCalibrationSet(devNum, portGroup, portNum,
                        curPortParams.serdesSpeed, localLaneIdx, curPortParams.activeLanesList[localLaneIdx], &calibrationFinished);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] prvCpssPortManagerOpticalCalibrationSet failed=%d", portNum, rc);
                }

                if (calibrationFinished)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] = GT_TRUE;
                }
            }
        }
        for (localLaneIdx = 0; localLaneIdx < curPortParams.numOfActLanes; localLaneIdx++)
        {
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] == GT_FALSE)
            {
                return GT_OK;
            }
        }
        /* in case of cable plug out mask operation will take place*/
        tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished = GT_TRUE;

        /* unmask low level interrupts (working on mac lvl only)*/
        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
        }

        /* if we passed optical calibration successfuly reset timers */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
        }

        return GT_OK;

}

/**
* @internal prvCpssPortManagerLoopbackModeInit function
* @endinternal
*
* @brief  Init loopback configuration structure according to given loopback type.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
*
* @param[out] loopbackCfgPtrOut    - (pointer to) loopback configuration structure to initialize
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLoopbackModeInit
(
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT loopbackType,
    OUT CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtrOut
)
{

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtrOut);

    loopbackCfgPtrOut->loopbackType = loopbackType;
    loopbackCfgPtrOut->enableRegularTrafic = GT_FALSE;
    switch (loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        loopbackCfgPtrOut->loopbackMode.macLoopbackEnable = GT_FALSE;
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        loopbackCfgPtrOut->loopbackMode.serdesLoopbackMode = CPSS_PORT_SERDES_LOOPBACK_DISABLE_E;
        break;

    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %2d] prvCpssPortManagerLoopbackModeInit:bad loopback type=%d",portNum,loopbackType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLoopbackModesDbGet function
* @endinternal
*
* @brief  Get loopback mode of the port.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] portType         - AP or Non-AP port type
*
* @param[out] loopbackCfgPtrOut    - (pointer to) loopback configuration structure
* @param[out] anyLoopbackPtr      - (pointer to) whether or not any loopback is configured
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad pointer
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    OUT CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtrOut,
    OUT GT_BOOL *anyLoopbackPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *tmpLoopbackCfgPtr;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtrOut);
    CPSS_NULL_PTR_CHECK_MAC(anyLoopbackPtr);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }
    else
    {
        /* Loopback is currently not supported in AP - so need to return no Loopback with OK status */
        *anyLoopbackPtr = GT_FALSE;
        return GT_OK;
        /* verify allocation */
        /*PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);*/
    }

    /* first, fill defaults */
    *anyLoopbackPtr = GT_FALSE;
    /* next, fill real data */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        tmpLoopbackCfgPtr = &(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->
                              loopbackDbCfg);
    }
    else
    {
/* TBD - add loopback support for AP port */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                                   "[Port %2d] loopback not suported for AP port", portNum);
        /*tmpLoopbackCfgPtr = &(tmpPortManagerDbPtr->portsApAttributedDb[portNum]->generalPortCfg.
                   loopbackDbCfg);*/
    }

    cpssOsMemCpy(loopbackCfgPtrOut, tmpLoopbackCfgPtr, sizeof(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC));

    switch (tmpLoopbackCfgPtr->loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        if (tmpLoopbackCfgPtr->loopbackMode.macLoopbackEnable==GT_TRUE)
        {
            *anyLoopbackPtr = GT_TRUE;
        }
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        if (loopbackCfgPtrOut->loopbackMode.serdesLoopbackMode != CPSS_PORT_SERDES_LOOPBACK_DISABLE_E)
        {
            *anyLoopbackPtr = GT_TRUE;
        }
        break;

    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
        /* not filling data */
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[Port %2d] loopback type undefined=%d"
                                                   ,portNum, tmpLoopbackCfgPtr->loopbackType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLoopbackModesDbSet function
* @endinternal
*
* @brief  Set loopback mode of the port.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] portType         - AP or Non-AP port type
* @param[in] loopbackCfgPtr      - (pointer to) loopback configuration structure
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    IN CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtr
)
{
    GT_U32 portMacNum;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *portCfgDbPtr;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtr);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        portCfgDbPtr = &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg;
    }
    else
    {
/* TBD - AP port loopback */
        /* verify allocation */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %2d] loopback not suported for AP port", portNum);
        /*
        PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        portCfgDbPtr = &tmpPortManagerDbPtr->portsApAttributedDb[portNum]->
          generalPortCfg.loopbackDbCfg;*/
    }

/* TBD - currently, loopback not supported in AP (portMode needed in this stage in order
  to perform serdes loopback) */
    cpssOsMemSet(portCfgDbPtr, 0, sizeof(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC));
    portCfgDbPtr->loopbackType = loopbackCfgPtr->loopbackType;
    /* MAC loopback */
    if (loopbackCfgPtr->loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)
    {
        portCfgDbPtr->loopbackMode.macLoopbackEnable = loopbackCfgPtr->loopbackMode.macLoopbackEnable;
    }
    /* SERDES loopback */
    else if (loopbackCfgPtr->loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E)
    {
        portCfgDbPtr->loopbackMode.serdesLoopbackMode = loopbackCfgPtr->loopbackMode.serdesLoopbackMode;
    }
    else if (loopbackCfgPtr->loopbackType != CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %2d] loopback type undefined=%d", portNum, loopbackCfgPtr->loopbackType);
    }
    portCfgDbPtr->enableRegularTrafic = loopbackCfgPtr->enableRegularTrafic;

    return GT_OK;
}


/**
* @internal prvCpssPortManagerGetPortHwsParams function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's parameters.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup        - port group number
* @param[in] portNum         - physical port number
*
* @param[out] curPortParamsPtrPtr   - (pointer to pointer to) hws parameters
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR        - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParams
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    MV_HWS_PORT_INIT_PARAMS *curPortParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_STANDARD portMode;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(curPortParamsPtr);

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "%s not supported for %d device",
                                                   __FUNCNAME__, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database from the main pp structure */
    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    /* getting appropriate mac num for the fiven physical port num */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb,
                                               tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssLion2CpssIfModeToHwsTranslate returned rc %d",portNum, rc );
    }

    /* get hws parameters for a given Hws port mode */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, curPortParamsPtr))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] hwsPortModeParamsGetToBuffer returned null ", portNum );
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's ifMode and speed.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup        - port group number
* @param[in] portNum         - physical port number
* @param[in] ifMode          - port interface mode
* @param[in] speed          - port speed
*
* @param[out] curPortParamsPtrPtr   - (pointer to pointer to) hws parameters
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR        - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT speed,
    MV_HWS_PORT_INIT_PARAMS *curPortParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_STANDARD portMode;

    CPSS_NULL_PTR_CHECK_MAC(curPortParamsPtr);

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "%s not supported for %d device",
                                                   __FUNCNAME__, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting appropriate mac num for the fiven physical port num */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                               speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",portNum, rc );
    }

    /* get hws parameters for a given Hws port mode */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, curPortParamsPtr))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] hwsPortModeParamsGet returned null ", portNum );
    }

    return GT_OK;
}
/**
* @internal prvCpssPortManagerCleanPortParameters function
* @endinternal
*
* @brief  Clean all port parameters in Port Manager DB.
*
* @note  APPLICABLE DEVICES:   xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup        - port group number
* @param[in] portNum         - physical port number
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerCleanPortParameters
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_PORT_MANAGER_PORT_TYPE_ENT portType
)
{
    GT_STATUS        rc;
    GT_U8          laneIndex,i;
    MV_HWS_PORT_INIT_PARAMS   curPortParams = {0};
    PRV_CPSS_PORT_MNG_DB_STC  *tmpPortManagerDbPtr;

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerGetPortHwsParams not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database from the main pp structure */
    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* verify allocation */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }
    else
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 1);

    /* getting hws parameters of the port */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = prvCpssPortManagerGetPortHwsParams(devNum, portGroup, portNum, &curPortParams);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerGetPortHwsParams failed=%d", portNum, rc);
        }
    }

    /* full clean-up */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideTrainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideEdgeDetectSupported = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_LAST_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF = 0;


    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
        tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb = CPSS_PORT_SPEED_NA_E;
        for (laneIndex=0; laneIndex<curPortParams.numOfActLanes; laneIndex++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, curPortParams.activeLanesList[laneIndex], 1);
            cpssOsMemSet((&tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[laneIndex]]->portSerdesParams),0,
                         sizeof(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[0]->portSerdesParams));
        }

        /* cleaning loopback db */
        rc = prvCpssPortManagerLoopbackModeInit(portNum,CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
        }
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        i=0;
        /* ap attributes */
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fcAsmDir = PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR;
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fcPause = PRV_PORT_MANAGER_DEFAULT_FC_PAUSE;
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.laneNum = PRV_PORT_MANAGER_DEFAULT_LANE_NUM;
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.noneceDisable = PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE;
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask = 0;
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortApNumOfModes = 0;
        /* in AP, in port manager DB those are the fields for the supported advertisement modes */
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fecAbilityArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fecRequestedArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].speedDb = CPSS_PORT_SPEED_NA_E;
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perSerdesDb, 0,
                     sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC) * CPSS_PORT_AP_IF_ARRAY_SIZE_CNS * PORT_MANAGER_MAX_LANES);

        /* cleaning loopback db */
        /* TBD - add loopback support for AP port */
        /*rc = prvCpssPortManagerLoopbackModeInit(portNum,CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                      &tmpPortManagerDbPtr->portsApAttributedDb[portNum]->generalPortCfg.loopbackDbCfg);
        if (rc!=GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                    "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
        }*/

    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;

    return GT_OK;
}

GT_STATUS prvCpssPortManagerVerifyAllocation
(
    GT_U8 devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ENT allocType
)
{
    GT_STATUS rc;
    GT_U32 portMacNum, i;
    GT_U8 serdesIndex;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* getting appropriate mac num for the fiven physical port num */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocate port */
        if (tmpPortManagerDbPtr->portMngSmDb==NULL)
        {
            tmpPortManagerDbPtr->portMngSmDb =
                (PRV_CPSS_PORT_MNG_PORT_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portMngSmDb==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet( tmpPortManagerDbPtr->portMngSmDb, 0, sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum] =
            (PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC));
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portMngSmDb[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC));

        /* default state for a port is RESET state */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_RESET_E;
        /* default failure for a port is FAILURE_NONE */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
        /* default operative-disabled state for a port is FALSE */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = 1;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_LAST_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType = CPSS_PORT_MANAGER_DEFAULT_CALIBRATION_TYPE_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode = CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_ENABLE_MODE_E;
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_MAC_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        MV_HWS_PORT_INIT_PARAMS curPortParams;

        /* getting hws parameters of the port */
        rc = prvCpssPortManagerGetPortHwsParams(devNum, 0, portNum, &curPortParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerGetPortHwsParams failed=%d", portNum, rc);

        /* mac pointers allocation */
        if (tmpPortManagerDbPtr->portMngSmDbPerMac==NULL)
        {
            tmpPortManagerDbPtr->portMngSmDbPerMac =
                (PRV_CPSS_PORT_MNG_MAC_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portMngSmDbPerMac==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet( tmpPortManagerDbPtr->portMngSmDbPerMac, 0, sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        /* mac allocation */
        for (serdesIndex=0; serdesIndex<curPortParams.numOfActLanes; serdesIndex++)
        {
            if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex]==NULL)
            {
                tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex] =
                    (PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC));
                if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex]==NULL)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);
            }
            cpssOsMemSet(tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex], 0, sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC));
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_SERDES_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        MV_HWS_PORT_INIT_PARAMS curPortParams;

        /* getting hws parameters of the port */
        rc = prvCpssPortManagerGetPortHwsParams(devNum, 0, portNum, &curPortParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerGetPortHwsParams failed=%d", portNum, rc);

        /* serdes pointer allocation */
        if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr =
                (PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS*8);
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr, 0,
                         sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS*8);
        }

        /* per serdes allocation */
        for (serdesIndex=0; serdesIndex<curPortParams.numOfActLanes; serdesIndex++)
        {
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]==NULL)
            {
                tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]] =
                    (PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC));

                if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]==NULL)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);
            }
            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]],
                        0, sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC));

            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]->portSerdesParams.rxValid = GT_FALSE;
            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]->portSerdesParams.txValid = GT_FALSE;
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr =
                (PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC**)
            cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr, 0,
                        sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum] =
                (PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC));

            if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC));

        /*tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;*/
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb = CPSS_PORT_SPEED_NA_E;
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        if (tmpPortManagerDbPtr->portsApAttributedDb == NULL)
        {
            tmpPortManagerDbPtr->portsApAttributedDb =
                (PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsApAttributedDb==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributedDb,0,
                        sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
        }

        if (tmpPortManagerDbPtr->portsApAttributedDb[portNum] == NULL)
        {
            tmpPortManagerDbPtr->portsApAttributedDb[portNum] =
                (PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC));
            if (tmpPortManagerDbPtr->portsApAttributedDb[portNum]==NULL)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);
            }
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributedDb[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC));

        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].speedDb = CPSS_PORT_SPEED_NA_E;
        }
        tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortApNumOfModes = 0;
    }

#if 0
    /* initialize loopback configurations */
    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocation - in AP structures this is allocated statically as the whole AP enclosing structure
          is already allocated per port, and the loopback configuration is global per the port. */

        /* initialization */
        /* TBD - add loopback support for AP port */
        /*rc = prvCpssPortManagerLoopbackModeInit(portNum,
                    CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                    &tmpPortManagerDbPtr->portsApAttributedDb[portNum]->generalPortCfg.loopbackDbCfg);
        if (rc!=GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                    "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
        }*/
    }
#endif
    /* initialize loopback configurations */
    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocation */
        if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr == NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr =
                (PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC**)
            cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr, 0,
                        sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum] =
                (PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC));

            if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %2d] allocation failed", portNum);
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum], 0,
                     sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC));

        /* initialization */
        rc = prvCpssPortManagerLoopbackModeInit(portNum, CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerInit
(
    GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    rc = GT_OK;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerInit not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    if (PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortHwUnresetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortHwResetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppRemoteFaultSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppForceLinkDownSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSigDetGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCgConvertersGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCgConvertersStatus2GetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortModeSpeedSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConvertFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppIfModeGetFromExtFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSerdesNumGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppNumOfSerdesGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMpcs40GCommonStatusRegGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacDmaModeRegAddrGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusChangedNotifyFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppXlgMacMaskAddrGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppFecModeSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLaneTuneSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLaneTuneGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConfigClearFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppMacConfigurationFunc == NULL /*||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApConfigSetFunc == NULL*/ /*AP is not a must*/
        /* PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApStatusGetFunc == NULL */||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacLoopbackModeSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortPcsLoopbackModeSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortSerdesLoopbackModeSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortSerdesPolaritySetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortProprietaryHighSpeedPortsSetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortSerdesTuneResultGetFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppAutoNeg1GSgmiiFunc == NULL ||
        PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMgrInitFunc == NULL)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "function pointer is null");
    }

    return rc;
}

GT_STATUS prvCpssPortManagerResetCounters
(
    GT_PHYSICAL_PORT_NUM  portNum,
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr
)
{
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
    /* Reset timers */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = 1;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

    /* in case of disable need to restart the optical algo */
    prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);

    return GT_OK;
}


GT_STATUS prvCpssPortManagerApPortHCDFound
(
    GT_SW_DEV_NUM      devNum,
    GT_PHYSICAL_PORT_NUM  portNum
)
{
    CPSS_PORT_AP_STATUS_STC apStatus;
    GT_STATUS rc;

    /* query resolution results */
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApStatusGetFunc(CAST_SW_DEVNUM(devNum),portNum,&apStatus);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                   "[Port %2d] CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortApPortStatusGet:rc=%d,portNum=%d\n",
                                                   portNum, rc, portNum);
    }

    /* resolution found - allocate pizza resources*/
    if (apStatus.hcdFound)
    {
        CPSS_PORT_SPEED_ENT speed;

        /* Check current spped */
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSpeedGetFromExtFunc(devNum, portNum, &speed);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %2d] CPSS_SRVCPU_PORT_802_3_AP_E "
                                                       "- ppSpeedGetFromExtFunc:rc=%d\n",
                                                       portNum, rc);
        }

        /*if pizza already configured, make sure it is the same as reported (need to check speed only)*/
        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            if (speed != apStatus.portMode.speed)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] pizza already defined but"
                                                           " not as found in resolution:"
                                                           " current speed =%d, resolution speed = %d\n",
                                                           portNum,speed,apStatus.portMode.speed);
            }
        }
        else
        {
            CPSS_PORTS_BMP_STC portsBmp;
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

            /* allocate pizza resources */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortModeSpeedSetFunc
                 (devNum, portsBmp, GT_TRUE,
                  apStatus.portMode.ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, apStatus.portMode.speed + CPSS_PORT_SPEED_NA_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                           "[Port %2d] CPSS_SRVCPU_PORT_802_3_AP_E - ppPortModeSpeedSetFunc:rc=%d\n",portNum, rc);
            }
        }
    }
    else
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[Port %2d] CPSS_SRVCPU_PORT_802_3_AP_E - no resolution\n",portNum);
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerValidateSerdesLanes
(
    GT_PHYSICAL_PORT_NUM portNum,
    GT_U32 tragetLane,
    GT_U16 *sdVectorPtr,
    GT_U32 sdVecSize
)
{
    GT_U8 iter, sdCounter;

    /* nuill pointer check */
    if (sdVectorPtr == NULL)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "[Port %2d] serdes vector is NULL", portNum);
    }

    /* checking if the serdes lane is valid */
    sdCounter = 0;
    for (iter=0; iter<sdVecSize; iter++)
    {
        if (tragetLane == sdVectorPtr[iter])
            break;
        sdCounter++;
    }
    if (sdCounter==sdVecSize)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[Port %2d] lane %d not exist in port lanes",
                                                   portNum, tragetLane);
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerApEventSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_MANAGER_EVENT_ENT     stage,
    IN PRV_CPSS_PORT_MNG_DB_STC         *tmpPortManagerDbPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      i, portGroup;
    GT_BOOL     linkUp;

    /* AP port will be configured only in this next scope */
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "AP port eventSet: Event=%d", stage);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    switch (stage)
    {
    CPSS_PORT_AP_PARAMS_STC apParams;
#if 0
    CPSS_PORT_AP_SERDES_TX_OFFSETS_STC serdesTxParamsOffset;
    CPSS_PORT_SERDES_TUNE_STC serdesParams;
    GT_BOOL  rxValid ,txValid;
    MV_HWS_PORT_INIT_PARAMS *curPortParamsPtr;
    GT_U8 laneIndex;
#endif

    case CPSS_PORT_MANAGER_EVENT_CREATE_E:
    case CPSS_PORT_MANAGER_EVENT_ENABLE_E:
        /* CPSS_PORT_AP_IF_ARRAY_SIZE_CNS */
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            apParams.modesAdvertiseArr[i].ifMode = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].ifModeDb;
            apParams.modesAdvertiseArr[i].speed = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i].speedDb;

            apParams.fecAbilityArr[i] = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fecAbilityArr[i];
            apParams.fecRequestedArr[i] = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fecRequestedArr[i];

            /* old mode for 10G/40G fec */
            if ((apParams.modesAdvertiseArr[i].speed == CPSS_PORT_SPEED_10000_E) || (apParams.modesAdvertiseArr[i].speed == CPSS_PORT_SPEED_40000_E))
            {
                apParams.fecRequired   = (apParams.fecRequestedArr[i] == CPSS_PORT_FEC_MODE_DISABLED_E) ? GT_FALSE : GT_TRUE;
                apParams.fecSupported  = (apParams.fecAbilityArr[i] == CPSS_PORT_FEC_MODE_DISABLED_E) ? GT_FALSE : GT_TRUE;
            }
        }
        apParams.fcAsmDir = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fcAsmDir;
        apParams.fcPause = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.fcPause;
        apParams.laneNum = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.laneNum;
        apParams.noneceDisable = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->apAttributesStc.noneceDisable;
#if 0
        /* serdes tx parameters offsets for AP */
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            /* searching for ifMode and speed with tx params in db from user */
            if ((tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->ifModeDb != CPSS_PORT_INTERFACE_MODE_NA_E)&&
                (tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->speedDb != CPSS_PORT_SPEED_NA_E) &&
                (tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->speedDb != CPSS_PORT_SPEED_10_E) &&
                (tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->ifModeDb != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
            {
                /* by here we found advMode (mode&speed)
                   now get tx params from our db*/
                rc = prvCpssPortManagerGetPortHwsParams2(devNum,portGroup,portNum,
                                                         tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->ifModeDb,
                                                         tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->speedDb,
                                                         &curPortParamsPtr);
                if (rc!=GT_OK)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerGetPortHwsParams2 faild=%d", portNum, rc);

                /* going through all port mode lanes, for each lane checking if there are tx
                   params in db, then send the values, lane and port mode to AP*/
                for (laneIndex=0; laneIndex < curPortParamsPtr->numOfActLanes; laneIndex++)
                {
                    rc = prvCpssPortManagerLaneTuneDbGet(devNum, portNum, laneIndex,
                                                         CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &serdesParams, &rxValid, &txValid);
                    if (rc!=GT_OK)
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneDbGet faild=%d", portNum, rc);

                    serdesTxParamsOffset.txAmpOffset = 0;
                    serdesTxParamsOffset.txEmph0Offset = 0;
                    serdesTxParamsOffset.txEmph1Offset = 0;

                    if (txValid==GT_TRUE)
                    {
                        serdesTxParamsOffset.txAmpOffset = serdesParams.txAmp;
                        serdesTxParamsOffset.txEmph0Offset = serdesParams.txEmphAmp;
                        serdesTxParamsOffset.txEmph1Offset = serdesParams.txEmph1;
                    }

                    /* if tx offsets not zero, send them to AP with the proper port mode */
                    if (serdesTxParamsOffset.txAmpOffset != 0 ||
                        serdesTxParamsOffset.txEmph0Offset != 0 ||
                        serdesTxParamsOffset.txEmph1Offset != 0)
                    {
                        /* ifmode and speed */
                        serdesTxParamsOffset.ifMode = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->ifModeDb;
                        serdesTxParamsOffset.speed = tmpPortManagerDbPtr->portsApAttributedDb[portNum]->perPhyPortDb[i]->speedDb;
                        /* sending ifmode and speed and offsets */
                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApSerdesTxParamsOffsetSetFunc(devNum, portNum,
                                                                                                                  laneIndex, &serdesTxParamsOffset);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppPortApSerdesTxParamsOffsetSetFunc faild=%d", portNum, rc);
                        }
                    }
                }
            }
        }
#endif
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                    GT_TRUE, &apParams);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] creating ap port faild=%d", portNum, rc);
        }
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;
        if (stage == CPSS_PORT_MANAGER_EVENT_ENABLE_E)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
        }
        break;

    case CPSS_PORT_MANAGER_EVENT_DELETE_E:
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                    GT_FALSE, &apParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] deleting ap port faild=%d", portNum, rc);

        rc = prvCpssPortManagerCleanPortParameters(CAST_SW_DEVNUM(devNum), portGroup,
                                                   portNum, CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerCleanPortParameters faild=%d", portNum, rc);

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_FALSE;
        break;

    case CPSS_PORT_MANAGER_EVENT_DISABLE_E:
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                    GT_FALSE, &apParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] deleting ap port faild=%d", portNum, rc);

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;

        break;

    case CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E:
        rc = prvCpssPortManagerApPortHCDFound(devNum, portNum);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] apHdcFound faild=%d", portNum, rc);
        break;

    case CPSS_PORT_MANAGER_EVENT_INIT_E:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %2d] operation not supported for AP port. operation=%d", portNum, stage);
        break;

    case CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E:
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"operation not needed for AP port. operation=%d", stage);
        break;

    case CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E:
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppLinkStatusGetFunc faild=%d", portNum, rc);

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "AP port mac_level changed, link %d ", linkUp);
        if (linkUp == GT_TRUE)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM =  CPSS_PORT_MANAGER_STATE_LINK_UP_E;
        }
        else
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM =  CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
        }
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] ap port bad switch-case operation=%d", portNum, stage);
        break;
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerRxStabilityTimeCheck
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr
)
{
    GT_STATUS  rc; /* return code */
    GT_U32 timerSecondsNow;
    GT_U32 timerNsecNew;

    /* Timeout check 1Sec */
    if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart > (1000000000) &&
         tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries > 40 )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Training timeout reached, moving to link down");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries = 0;

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            /* reset Optical calibration counters */
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        /* No need to run again rx train, as rx tain is now running on the down serdes
           that caused this timeout. When this serdes will receive signal, then we continue
           and same story.*/

        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        return GT_OK;
    }

    /* Initialize timers if this is the first iteration in the algorithm */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev==0 &&
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev==0)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart = 0;
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev);
        if (rc!=GT_OK)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                /* reset Optical calibration counters */
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            }

            /* No need to run again rx train, as rx train is now running on the down serdes
              that caused this timeout. When this serdes will receive signal, then we continue
              and same story.*/

            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Timer issue - moving to link down", portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow,&timerNsecNew);
    if (rc!=GT_OK)
    {
        /* Thats it. returning to wait for signal detection */
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTotalTimeFromStart = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTries = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            /* reset Optical calibration counters */
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
        }

        /* No need to run again rx train, as rx train is now running on the down serdes
         that caused this timeout. When this serdes will receive signal, then we continue
          and same story.*/
        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Timer issue - moving to link down", portNum);
    }

    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime =
        ( 1000000000 - (tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev) + timerNsecNew );
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev) );
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev = timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTotalTimeFromStart+=
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime;

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTries++;

    return GT_OK;
}

GT_STATUS prvCpssPortManagerResetAlignCounters
(
     IN GT_PHYSICAL_PORT_NUM portNum,
     IN PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr
)
{
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portDbAlignLockDone=1;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimeDiffFromLastTime=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimerSecondsPrev=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimerNsecPrev=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTotalTimeFromStart=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> Align_Num_Of_Events=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimeIntervalTimer=0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTries = 0;
    return GT_OK;
}

GT_STATUS prvCpssPortManagerAlignTimerCheck
(
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr,
    IN GT_BOOL              overallStatusesOk,
    IN GT_BOOL              *isAlign,
    IN GT_BOOL              *timeout
)
{
    GT_STATUS  rc; /* return code */
    GT_U32 timerSecondsNow;
    GT_U32 timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTries++;
    *isAlign = GT_FALSE;
    *timeout = GT_FALSE;
    /* advancing alignlock timers */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev==0
        && tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev==0)
    {
        /*portDbStabilityDone=0;*/
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Alignlock timer issue", portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow, &timerNsecNew);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d] Alignlock timer issue", portNum);
    }
    /* In case we in different time in meanning of a second, the difference in nano seconds
      is the following equation */
    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime =
        ( (1000000000 - tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev) + timerNsecNew );
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev) /*/ 1000000*/ );
    }

    /* update the port seconds and nanosecs timers */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev = timerNsecNew;

    /* updating total time spent */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart+=
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer +=
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime;

    /* if interval time passed, checking the status. We want statuses to be steady
      for X tries but also over Y time*/
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer>=10000000)
    {
        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"asking ");*/
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer=0;
        if (overallStatusesOk == GT_TRUE)
        {
            /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port statuses OK, incrementing ");*/
            tmpPortManagerDbPtr->portMngSmDb[portNum]->Align_Num_Of_Events++;
        }
        else
        {
            /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port statuses NOT OK, zeroing ");*/
            tmpPortManagerDbPtr->portMngSmDb[portNum]->Align_Num_Of_Events = 0;
        }
    }
    else
    {
        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"not yet, returning");*/
        return GT_OK;
    }


    /* if 3 qonsequtive good-all-statuses existed, we are good */
    if ((overallStatusesOk == GT_TRUE) && (tmpPortManagerDbPtr->portMngSmDb[portNum]-> Align_Num_Of_Events >= 3)/*portAlignTotalTimeFromStart>=30*/)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ALIGN: alignLockDone %d, stabilityDone %d, trainDone %d. Align_Num_Of_Events 0x%08x, portAlignTotalTimeFromStart %dms",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->Align_Num_Of_Events,
                                              (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart/1000000));
        prvCpssPortManagerResetAlignCounters(portNum,tmpPortManagerDbPtr);
        *isAlign = GT_TRUE;
    }
    /* maximux timeout */
    else if ( ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart)) >= 4000000000UL &&
              tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTries>10)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"TIMEOUT: alignLockDone %d, stabilityDone %d, trainDone %d. Align_Num_Of_Events 0x%08x, portAlignTotalTimeFromStart %dms",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->Align_Num_Of_Events,
                                              (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart/1000000));
        prvCpssPortManagerResetAlignCounters(portNum,tmpPortManagerDbPtr);
        *timeout = GT_TRUE;
    }
    return GT_OK;
}

GT_STATUS internal_cpssPortManagerEventSet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MANAGER_STC *portEventStc
)
{
    GT_STATUS  rc; /* return code */
    GT_STATUS  rc_tmp; /* return code */
    /*GT_U32 portMacMap;*/
    /*GT_U32 squelchToRestore;*/
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT      speed;
    MV_HWS_PORT_STANDARD     portMode=0;
    GT_U32 portGroup;
    CPSS_PORTS_BMP_STC portModeSpeedSetBitmap;
    CPSS_PORT_MANAGER_EVENT_ENT  stage;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_U32 portMacNum;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_BOOL linkUp;
    GT_U32 i;
    GT_U32 data;
    GT_U32 regAddr;
    GT_BOOL rxSignalOk;
    CPSS_UNI_EV_CAUSE_ENT uniEv;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT portType;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL anyLoopback;
    CPSS_PORT_FEC_MODE_ENT fecAbility;
    GT_U16  newUpdate;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %2d] port manager not supported for %d device",
                                                   portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    stage = portEventStc->portEvent;
    if (stage != CPSS_PORT_MANAGER_EVENT_INIT_E)
    {
        stageInit = GT_FALSE;
    }
    /* if DELETE event called on non allocated port, just return GT_OK */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum,
                                                     (stage == CPSS_PORT_MANAGER_EVENT_DELETE_E) ? 0 : 1);

    portType = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;
    if (portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] **invalid portType %d stage %d",
                                                   portNum, portType, stage);
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }

    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    speed = CPSS_PORT_SPEED_NA_E;

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
        speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    }

    /*squelchToRestore = 0;*/

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] port check and mac num get failed- %d",portNum, rc);
    }


    rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum), portNum, portType, &loopbackCfg, &anyLoopback);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   portNum, rc);
    }

    if (1)
    {
        newUpdate = (GT_U16)((stage<<8) | (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM));
        if (lastUpdate[portNum] != newUpdate)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "stage %d AdminSM %d stabilityDone %d trainDone %d alignLockDone %d rxStabilityTrainDone %d signalDownHandled %d",
                                                  stage, tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM, tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone, tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone, tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled);
            lastUpdate[portNum] = newUpdate;
        }

        if (stage==CPSS_PORT_MANAGER_EVENT_CREATE_E &&
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] event CREATE not applicable when port is not in RESET",portNum);
        }

        if (stage==CPSS_PORT_MANAGER_EVENT_DELETE_E &&
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_RESET_E)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling DELETE when port is in reset, returning GT_OK");
            return GT_OK;
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM==GT_TRUE &&
            !(CPSS_PORT_MANAGER_EVENT_ENABLE_E==stage || CPSS_PORT_MANAGER_EVENT_DELETE_E==stage
              || CPSS_PORT_MANAGER_EVENT_DISABLE_E==stage) )
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] event %d not applicable when port is disabled", portNum, stage);
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM == GT_FALSE
            && CPSS_PORT_MANAGER_EVENT_ENABLE_E==stage)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"enable operation called upon already enabled port, so just returning.");
            return GT_OK;
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM == GT_TRUE
            && CPSS_PORT_MANAGER_EVENT_DISABLE_E == stage)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"disable operation called upon already disabled port, so just returning.");
            return GT_OK;
        }

        portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

        if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
        {
            if ( ifMode == CPSS_PORT_INTERFACE_MODE_NA_E || speed == CPSS_PORT_SPEED_NA_E )
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                                           "[Port %2d] Calling event without on port without mandatory params been set", portNum);
            }

            /* Getting HWS portMode for the port*/
            rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",portNum, rc );
            }

        }

        /* AP port will be configured only in this next scope */
        if (portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
        {
            rc = prvCpssPortManagerApEventSet(CAST_SW_DEVNUM(devNum), portNum, stage, tmpPortManagerDbPtr);
            return rc;
        }

        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "[Port %2d] hwsPortModeParamsGetToBuffer returned null portMacNum %d portMode %d portType %d ifMode %d speed %d stage %d",
                                                        portNum, portMacNum, portMode, portType, ifMode, speed, stage);
        }

        if (stage == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E || stage == CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E)
        {
            if (stage == CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E &&
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED");
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusChangedNotifyFunc(devNum,portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                               "[Port %2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", portNum, rc);
                }

                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling portLinkStatusGet returned %d ", portNum, rc );
                }
                if (linkUp==GT_TRUE)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"mac_level changed, link_up 1 ");
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"mac_level changed, link_up 0 ");
                    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                    }
                }
            }
            else if (stage == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E &&
                     tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED");
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"low_level changed portAdminSM %d", tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM);
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E
                    || tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E)
                {
                    stage = CPSS_PORT_MANAGER_EVENT_INIT_E;

                    if (!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed))
                    {
                        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }

                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
            }
        }
        /************************************************************
            Enable port [0/2]
        ************************************************************/
        if (stage==CPSS_PORT_MANAGER_EVENT_ENABLE_E)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Stage: PORT_MANAGER_EVENT_ENABLE");
            if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
            {
                /* mask low level so false interrupt from serdes power-up won't */
                rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* power-up port */
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortHwUnresetFunc(devNum, portNum);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppPortHwUnresetFunc returned %d ", portNum, rc );
            }

            if (PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(speed))
            {
                if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))
                {
                    rc = mvHwsPortLoopbackSet(devNum, portGroup, portMacNum, portMode, HWS_MAC,TX_2_RX_LB);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] Calling mvHwsPortLoopbackSet"
                                                                   " returned error code %d ",
                                                                   portNum, rc);
                    }
                }
            }

            /* if user configured rx\tx params and polarity*/
            for (i=0; i<curPortParams.numOfActLanes; i++)
            {
                CPSS_PORT_SERDES_TUNE_STC serdesParams;
                GT_BOOL rxValid, txValid;

                rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum),portNum, i, curPortParams.activeLanesList[i], portType,
                                                     &serdesParams, &rxValid, &txValid);
                if (rc!=GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx",portNum,rc);
                }
                if (rxValid == GT_TRUE)
                {
                    rc = prvCpssPortManagerLaneTuneHwSet(CAST_SW_DEVNUM(devNum),portGroup,portNum, portMode,curPortParams.activeLanesList[i], GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Rx",portNum,rc);
                    }
                }
                if (txValid == GT_TRUE)
                {
                    rc = prvCpssPortManagerLaneTuneHwSet(CAST_SW_DEVNUM(devNum),portGroup,portNum,portMode,curPortParams.activeLanesList[i], GT_FALSE);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx",portNum,rc);
                    }
                }
#if 0
                /* if not set by user, then setting default 0. If set by user, setting what was desired */
                rc = prvCpssPortManagerLanePolarityHwSet(CAST_SW_DEVNUM(devNum), portNum, curPortParams.activeLanesList[i]);
                if (rc!=GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx",portNum,rc);
                }
#endif
            }

            if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
            {
                /* clear cause bit of interrupt link_status_change */
                uniEv = CPSS_PP_PORT_LINK_STATUS_CHANGED_E;
                for (i=0; i<curPortParams.numOfActLanes; i++)
                {
                    rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), portGroup, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddr);
                    if (rc!=GT_OK)
                    {
                        rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                        }
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                    }
                    CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] clearing link_status_change interrupt"
                                                              " from register 0x%08x, for mac %d", portNum, regAddr, (portMacNum+i));
                    rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0xFFFFFFFF, &data);
                    if (rc != GT_OK)
                    {
                        rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Could not perform maskUnmask operation", portNum);
                        }
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d ", portNum, rc);
                    }
                }
                /* Reading the cause register of the interrupt */
                uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
                for (i=0; i<curPortParams.numOfActLanes; i++)
                {
                    rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), portGroup, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddr);
                    if (rc!=GT_OK)
                    {
                        rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                        }
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                    }
                    CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] clearing signal_detect interrupt"
                                                              " from register 0x%08x, for mac %d", portNum, regAddr, (portMacNum+i));
                    rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0xFFFFFFFF, &data);
                    if (rc != GT_OK)
                    {
                        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                        }
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d ",
                                                                   portNum, rc);
                    }
                }
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"clearing signal_detect AND link_status_change interrupt for mac %d numOfActLanes %d", (portMacNum), curPortParams.numOfActLanes);

                /* unmask low level interrupts */
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                }

                 /* loopback configuration */
                if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode != CPSS_PORT_SERDES_LOOPBACK_DISABLE_E))
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
                    rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                   portNum, rc);
                    }
                }

                if (((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))  ||
                    ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                {
                    if (loopbackCfg.enableRegularTrafic == GT_FALSE)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"In TX2RX mode serdes Tx is disable ");
                        rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsPortTxEnable failed=%d", portNum, rc);
                        }
                    }

                     stage = CPSS_PORT_MANAGER_EVENT_INIT_E;
                     tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
                     /* rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum, uniEv);
                    if (rc != GT_OK)
                    {
                     CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerPushEvent"
                                                                 " failed=%d", portNum, rc);
                    }*/
                }
                else
                {
                    /* if rx_signal_ok is up, pushing event that might have been missed */
                    rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSignalOk);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d", portNum, rc);
                    }
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"rx_signal_ok state=%d", rxSignalOk);
                    if (rxSignalOk == GT_TRUE)
                    {
                        rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum/*extraData*/, uniEv);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerPushEvent"
                                                                       " failed=%d", portNum, rc);
                        }
                    }

                    /* launching iCal */
                    if (!((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                    {
                        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) &&
                            PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifMode))
                            {
                                rc = prvCpssPortSerdesLunchSingleiCal(CAST_SW_DEVNUM(devNum), portNum);
                            }
                            else
                            {
                                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum,
                                                                                                       portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
                            }

                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppTuneExtSetFunc returned error code %d ",
                                                                       portNum, rc);
                            }
                    }
                }

                /* marking rx train ran */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = 0;
            }

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Stage: PORT_MANAGER_EVENT_ENABLE - END");

        }

        /************************************************
          Training [0/3]
        ************************************************/
        /* Calling INIT on speed not needed INIT is just to check link status */
        if ( ( (PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && ifMode != CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
             && stage==CPSS_PORT_MANAGER_EVENT_INIT_E)
        {
            PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatuses;

            if (stageInit == GT_FALSE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_INIT");
                stageInit = GT_TRUE;
            }

            /* Auto-Negotiation sequence for 1G QSGMII/SGMII */
            if (((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)) && (PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) )
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Disable Auto-Negotiation 1G QSGMII/SGMII");
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppAutoNeg1GSgmiiFunc(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", portNum, rc);
                }
            }

            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting sig det status: sigDet %d", portStatuses.sigDet);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStatusesCheck returned %d \n ", portNum, rc);
            }

            if (portStatuses.sigDet == GT_FALSE)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
            }

            /* enable port high level unit */
            rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum,ifMode,speed,portMode,GT_TRUE);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                           " returned error code %d ",
                                                           portNum, rc);
            }

            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppLinkStatusGetFunc link_up %d", linkUp);
            if (rc != GT_OK)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] FATAL Calling ppLinkStatusGetFunc, rc %d ", portNum, rc);
            }

            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusChangedNotifyFunc(devNum,portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                           "[Port %2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", portNum, rc);
            }

            if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port enable with MAC loopback");
                /* mask low level interrupts (working on mac lvl only)*/
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                }

                /* Enable port */
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Enable returned error code %d ", rc);
                    return rc;
                }
            }

            if (linkUp)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
            }
            else
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
            }


            return GT_OK;
        }

        if (stage == CPSS_PORT_MANAGER_EVENT_INIT_E)
        {
            /* Variables declarations */
            PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatuses;
            PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatusesStability;
            GT_BOOL ignoreTraining = GT_FALSE;

            if (stageInit == GT_FALSE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_INIT");
                stageInit = GT_TRUE;
            }

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;

            if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) &&
                (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E))
            {
                /*ignore training */
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"config SERDES_LOOPBACK TX2TX enableRegularTrafic %d (ignore training)",loopbackCfg.enableRegularTrafic);
                ignoreTraining = GT_TRUE;
            }

            if (!((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable)) &&
                !((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
            {
                if (!(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled == GT_TRUE) &&
                        (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished == GT_FALSE))
                    {
                        /*for Optical calibration check Confidence is in range before starting the init process */
                        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
                        {
                            rc = prvCpssPortManagerOpticalStabilityCheck(devNum,portGroup,portNum,portMode,speed);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerOpticalStabilityCheck failed", portNum);
                            }
                            else
                            {
                                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished == GT_FALSE)
                                {
                                    return GT_OK;
                                }
                            }
                        }
                    }
                }
            }
            {
                /* if port is in stability checking phase (which is prior to train) */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone==0)
                {
                    goto signalStabilityFilerPhase;
                }
                /* if port is in training phase */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone==0)
                {
                    goto tunePase;
                }
                /* if port is in waiting for align lock phase */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone==0)
                {
                    goto alignLockTimerPhase;
                }

                if (((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))  ||
                    ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port enable with MAC loopback");
                    /* mask low level interrupts (working on mac lvl only)*/
                    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                    }

                    /* Enable port */
                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Enable returned error code %d ", rc);
                        return rc;
                    }
                    goto skipStabilityCheck;
                }

                if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone == 0) && (!ignoreTraining))
                {
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc(devNum, portNum,
                                                                                              &rxTuneStatus, &txTuneStatus);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppTuneStatusGetFunc failed %d ", rc );
                        return rc;
                    }
                    if (rxTuneStatus==CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
                    {
                        /* TBD - when signal is lost and also alignment lost or gb lock lost, then more than one event
                          will be received upon the port that goes down (signal_detect_change and gb_lock change or
                          algin_lock_lost). First event will insert us to sigDetTrain, good, but then the second event
                          will make the seuqnce try to finish the sigDetTrain but there is no signal and it will reach
                          timeout. This scenario is not a bug as eventually port will be in state link_down, but there
                          is a bad side affect to this scenario as application will keep initialize (passing event init)
                          for nothing, and CPU time is wasted. We need to think about a better sequence in order to not
                          perform this side effect. */
                        if ( !(curPortParams.numOfActLanes>1) )
                        {
                            /* log this information only once, as this scope will be reached multiple times until
                              rx training will be finished */
                            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries == 0)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Possible signal lost during training - restarting");
                            }
                        }
                        rc = prvCpssPortManagerRxStabilityTimeCheck(CAST_SW_DEVNUM(devNum), portNum, tmpPortManagerDbPtr);
                        return rc;
                    }
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = 1;
                }

                /* we will check stability only in case signal is down and this is the first up.
                  (and also in case we already in the stability process - hence 'goto' lable)*/
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled == GT_TRUE)
                {
signalStabilityFilerPhase:
                    rc = prvCpssPortManagerStatusesStabilityCheckPhase(devNum, portNum, &portStatusesStability,
                                                                       PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E), curPortParams.numOfActLanes);

                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerStatusesStabilityCheckPhase returned %d", portNum, rc);
                    }
                    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == 0)
                    {
                        return GT_OK;
                    }
                    if (!(portStatusesStability.sigDet))
                    {

                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Signal stability failed");
                        /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum);*/
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled = GT_TRUE;

                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Unmasking port caused signal stability issue");

                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                        /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/

                        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                        }

                        return GT_OK;

                        /*appDemoPortInitSeqDisablePortStage(devNum, portNum);
                        goto unmaskSignalDetAndContinue;*/
                    }
                }

                /***************************************
                  1. Getting port statuses
                ****************************************/
                /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting all port statuses ");*/
                /* signal - for all lanes */
                if ( PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed) || PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(ifMode))
                {
                    rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                  PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
                    /* in low speeds (by here only 1000baseX) and in port modes that use XPCs we don't care about gear box or align lock,
                      only signal. So now that we passed signal stability, we can check the signal itself */
                    portStatuses.alignLock = GT_TRUE;
                    portStatuses.gbLock = GT_TRUE;
                }
                else
                {
                    rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                  PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E));
                }

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"getting all port statuses: alignLock %d gbLock %d sigDet %d overallStatusesOk %d ",
                                                      portStatuses.alignLock,
                                                      portStatuses.gbLock,
                                                      portStatuses.sigDet,
                                                      portStatuses.overallStatusesOk);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStatusesCheck returned %d",portNum, rc);
                }

                /***************************************
                  2. GearBox-lock and Align-lock check
                ****************************************/
                /* If signal was up and training launched and finished (which on success
                  can cause those statuses to be good), then, those low leve MAC units can be checked */

                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled==GT_FALSE)
                {

                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"section of gbLock and align lock.");

                    /* Loss of any of those units will cause us to disable port */
                    if (((curPortParams.numOfActLanes > 1) && (portStatuses.alignLock == GT_FALSE || portStatuses.gbLock == GT_FALSE)) ||
                        ((!(curPortParams.numOfActLanes > 1)) && (portStatuses.gbLock == GT_FALSE)))
                    {

                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"section of gbLock and align lock. align %d gb %d",
                                                              portStatuses.alignLock, portStatuses.gbLock);

                        /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum); */
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;

                        /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/

                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Unmasking port cause signal not good so only finish and unmask ");

                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataSet returned %d", portNum, rc);
                        }

                        /* disable port high level unit */
                        rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum,ifMode,speed,portMode,GT_FALSE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                                       " returned error code %d ",
                                                                       portNum, rc);
                        }

                        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                        }

                        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType ==
                                CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
                        {
                            /* reset Optical calibration counters */
                            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
                        }

                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                        if (!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)&& (!ignoreTraining))
                        {
                            rc = prvCpssPortManagerSigStableRxTraining(CAST_SW_DEVNUM(devNum), portNum);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", portNum, rc);
                            }
                        }
                        /* no furthur port handling needed */

                    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                        return GT_OK;
                    }
                    /* Only signal detect event will cause us to launch training. Gb lock or align lock
                      will not cause performing training. */
                }


                /*******************************
                   3. Signal state
                ********************************/
                /* If signal detected and signal was not down before, skip to next port
                  bacause there is nothing more to do since the previous time we got signal detect
                  and perform stability check than training. We want to avoid the situation where
                  we finish train, getting signal event, seing that is up, do another train,
                  getting event again... infinite loop. */
                if (portStatuses.sigDet == GT_TRUE && tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled==GT_FALSE)
                {
                    /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum);*/
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"unmasking port cause signal still up, no change ");
                    /* here do not reset optical counters, as this point means the port finished calibration but the peer is not up yet */

                    /* if we were in MAC_LINK_DOWN state (before INIT_STATE), and we got here, we need to check if we can go to LINK_UP,
                      or back to MAC_LINK_DOWN. */
                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppLinkStatusGetFunc returned %d ", portNum, rc );
                    }
                    if (linkUp == GT_TRUE && linkLevel[portNum]==GT_TRUE)
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
                    }
                    else
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                    }

                    /* no furthur port handling needed */

                    /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/
                    /* state machine should not change */

                    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    return GT_OK;
                }

                /*******************************
                   4. Signal stability
                ********************************/
                /* If signal is detected and signal was down before, continue to signal stability check*/
                if (portStatuses.sigDet == GT_TRUE)
                {
                    /*************************************
                     By here, signal was detected as UP, and its the first UP from the last DOWN.
                     So, from this point and until we finish handle the signal which is after
                     stability check, training, and port enable, the signal interrupt can be masked.
                     This mask is the CPSS way to ensure the port init sequence completeness.
                     It will be unmask only at the end of the whole handling. (also in a fail point between
                     here and the end, when we fail and breaking the handling, we unmask)
                    *************************************/
                }

                if ( portStatuses.sigDet == GT_FALSE )
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"signal or signal stability not good on port");
                    /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum);*/
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;

                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"unmasking port cause signal not good so only finish and unmask ");

                    /* no furthur port handling needed */

                    /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                    if (!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed) && (!ignoreTraining))
                    {
                        rc = prvCpssPortManagerSigStableRxTraining(CAST_SW_DEVNUM(devNum), portNum);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", portNum, rc);
                        }
                    }
                    return GT_OK;
                }

                tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_FALSE;

            }

            /**********************************************************
              Pre-Training [1/3] - set default rx\tx parameters
            **********************************************************/
            /* applying DB defaults to serdes */
            /* INSTEAD OF HIGHTEST SQUELCH => SETTING THE USER DEFAULTS */

            /* 1000baseX should go through signal stability check but thats it, should need to throug training and alignment check */
            if (ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"finished signal integrity for 1000baseX ");
                rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum, ifMode,speed, portMode, GT_TRUE);
                if (rc != GT_OK)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", portNum, rc);
                }

                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppLinkStatusGetFunc link_up %d", linkUp);
                if (rc != GT_OK)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppLinkStatusGetFunc, rc %d ", portNum, rc);
                }

                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusChangedNotifyFunc(devNum,portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                               "[Port %2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", portNum, rc);
                }

                if (linkUp)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
                }
                else
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                }
                return GT_OK;
            }
            if (!ignoreTraining)
            {
                rc = cpssPortSerdesCyclicAutoTuneDefaultsSet(CAST_SW_DEVNUM(devNum), portMode,
                                                             &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling cpssPortSerdesCyclicAutoTuneDefaultsSet returned %d ", rc);
                    return rc;
                }

                /* override tune related per user request before performing the training sequence */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E))
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.tune =
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideTrainMode;
                }
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E))
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.adaptiveEnabled =
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported;
                }
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E))
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.edgeDetectEnabled =
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideEdgeDetectSupported;
                }
                /**********************************************************
                  Pre-Training [3/3] - performing Edge detection
                **********************************************************/
                /* Performing Edge detection */


                /* Launching the auto tune. */
                /**********************************************************
                  Pre-Training [3/3] - Launching the auto tune
                **********************************************************/
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling rx-tune on port ");

                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 0;

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Calling tune2 on port ");
#if 0
                CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  tuneStatus;

               rc = prvCpssPortStageTrainMngPhases(portNum,devNum,&tuneStatus);
                /* If training failed, returning to waiting for signal detect event stage */
                if (rc != GT_OK)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_FAILURE_E;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E;
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Unmasking port, signal_det, cause prvCpssPortStageTrainMngPhases failed ");

                    return rc;
                }
#endif
            }
tunePase:
            {

                CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  tuneStatus;
                GT_BOOL                                align,timeout;

                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && (!ignoreTraining))
                {
                    rc = prvCpssPortStageTrainMngPhases(portNum, devNum, &tuneStatus);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStageTrainMngPhases - rc %d ", portNum, rc);
                    }
                }
                else
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
                    tuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
                }

                if (CPSS_PORT_SERDES_AUTO_TUNE_PASS_E == tuneStatus)
                {
                    if ((PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) && !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Aldrin */
                        && (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode || CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode))
                    {
                        /* XAUI/RXAUI use XPCS which does not implement remote fault */
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppRemoteFaultSetFunc was skipped since XPCS is used");
                    }
                    else
                    {
                        if ((!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) &&
                            (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb)))) /* for 100G we set local fault */
                        {
                            /* i tune passes, remove 'fault send' so partners could start align */
                            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, GT_FALSE);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppRemoteFaultSetFunc returned error code %d ",
                                                                           portNum, rc);
                            }
                        }
                    }

                    if (!PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed))
                    {
                        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                    if (!(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                            /* if application has override this, then get it from override db */
                        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap &
                             PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E) )
                        {
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted =
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported;
                        }
                        else
                        {
                            /* by default Adaptive is running */
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_TRUE;
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Adaptive started speed %d ", speed);
                        }
                    }

                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone=0;
alignLockTimerPhase:
                    if (curPortParams.numOfActLanes>1)
                    {
                        if(PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(ifMode))
                        { /* in modes that use xpcs there is no gearbox thus check only signal */
                            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
                            if (portStatuses.sigDet == GT_TRUE)
                            {
                                portStatuses.overallStatusesOk = GT_TRUE;
                            }
                        }
                        else
                        {
                            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E));
                        }
                    }
                    else
                    {
                        /* in single lane no need to check for align lock */
                        rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E));
                        if (portStatuses.gbLock == GT_TRUE && portStatuses.sigDet == GT_TRUE)
                        {
                            portStatuses.overallStatusesOk = GT_TRUE;
                        }
                    }

                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStatusesCheck returned %d", portNum, rc);
                    }
                    rc = prvCpssPortManagerAlignTimerCheck(portNum, tmpPortManagerDbPtr, portStatuses.overallStatusesOk, &align,&timeout);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    else if ((align == GT_FALSE) && (timeout == GT_FALSE))
                    {
                        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"not yet, returning");*/
                        return rc;
                    }

                    /* if 3 qonsequtive good-all-statuses existed, we are good */
                    if (align == GT_TRUE)
                    {
skipStabilityCheck:
                        prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

                        /* ENABLE */
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"all statuses OK ");
                        rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum, ifMode,speed, portMode, GT_TRUE);
                        if (rc != GT_OK)
                        {
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", portNum, rc);
                        }

                        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                        }
                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusGetFunc(devNum, portNum, &linkUp);
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppLinkStatusGetFunc link_up %s", linkUp?"TRUE":"FALSE");
                        if (rc != GT_OK)
                        {
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                            /* only high speed ports will reach here so no need to check for high speed port before
                              Calling this next function */
                            if (!ignoreTraining)
                            {
                                rc = prvCpssPortManagerSigStableRxTraining(CAST_SW_DEVNUM(devNum), portNum);
                                if (rc != GT_OK)
                                {
                                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", portNum, rc);
                                }
                            }
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppLinkStatusGetFunc - rc %d ", portNum, rc);
                        }

                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLinkStatusChangedNotifyFunc(devNum,portNum);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                                       "[Port %2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", portNum, rc);
                        }
                        if (linkUp)
                        {
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
                        }
                        else
                        {
                            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                        }
                    }
                    /* maximux timeout */
                    else if (timeout == GT_TRUE)
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled = GT_TRUE;

                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;

                        /* disable port high level unit */
                        rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum,ifMode,speed,portMode,GT_FALSE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel "
                                                                       " returned error code %d ",
                                                                       portNum, rc);
                        }

                        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                        }

                    }
                }
                else if (CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E == tuneStatus)
                {
                    prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

                    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
                    /* check if timeout because of failure or link down */
                    if (curPortParams.numOfActLanes>1)
                    {
                        if(PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(ifMode))
                        { /* in modes that use xpcs there is no gearbox thus check only signal */
                           rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
                           if (portStatuses.sigDet == GT_TRUE)
                           {
                               portStatuses.overallStatusesOk = GT_TRUE;
                           }
                        }
                        else
                        {
                            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E));
                        }
                    }
                    else
                    {
                        /* in single lane no need to check for align lock */
                        rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) | PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E));
                        if (portStatuses.gbLock == GT_TRUE && portStatuses.sigDet == GT_TRUE)
                        {
                            portStatuses.overallStatusesOk = GT_TRUE;
                        }
                    }
                    if (portStatuses.overallStatusesOk)
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_FAILURE_E;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E;
                    }
                    else
                    {
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                    }
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"unmasking port, signal_det, cause prvCpssPortStageTrainMngPhases failed ");

                    rc = prvCpssPortManagerEnableLinkLevel(devNum,portGroup,portNum,ifMode,speed, portMode, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", portNum, rc);
                    }
                    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                    }
                }
            }

            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone == 0 ||
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == 0
                || tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone==0)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
            }

        } /* if training */

        /************************************************
                Disable port
        ************************************************/
        if (stage == CPSS_PORT_MANAGER_EVENT_DISABLE_E || stage == CPSS_PORT_MANAGER_EVENT_CREATE_E
            || stage == CPSS_PORT_MANAGER_EVENT_DELETE_E)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Performing common port disable stage");
            if (stage == CPSS_PORT_MANAGER_EVENT_DELETE_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_DELETE");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_FALSE;
            }

            /* disable port high level unit as there will no be signal trigger that will perform stuff then allow link level */

            if (stage == CPSS_PORT_MANAGER_EVENT_DISABLE_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Starting DISABLE");
                if (!(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc(devNum, portNum,
                                                                                              &rxTuneStatus, &txTuneStatus);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppTuneStatusGetFunc failed %d ", rc );
                        return rc;
                    }
                    if (rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
                    {
                        cpssOsTimerWkAfter(100);
                    }

                    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stopping Adaptive speed %d ", speed);
                        /* Stop Adaptive anyways */
                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum,
                                    portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStageTrainMngPhases returned error code %d ",
                                                                       portNum, rc);
                        }
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_FALSE;
                    }
                }

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_DISABLE");
                prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

                if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"[Port %2d] port disable operation not supported for QSGMII ifMode", portNum);
                }
                else
                {
                    rc = prvCpssPortManagerEnableLinkLevel(devNum, portGroup, portNum, ifMode, speed, portMode, GT_FALSE);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                                   " returned error code %d ",
                                                                   portNum, rc);
                    }
                    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                    }
                }

                tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled = GT_TRUE;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"unmasking port, cause we finished training bad and in disable ");

                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortHwResetFunc(devNum, portNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] Calling ppPortHwResetFunc"
                                                               " returned error code %d ",
                                                               portNum, rc);
                }

                if (PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(speed))
                {
                    if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) && (loopbackCfg.loopbackMode.macLoopbackEnable))
                    {
                        rc = mvHwsPortLoopbackSet(devNum, portGroup, portMacNum, portMode, HWS_MAC, DISABLE_LB);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] Calling mvHwsPortLoopbackSet"
                                                                       " returned error code %d ",
                                                                       portNum, rc);
                        }
                    }
                }

                tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Finish DISABLE");
            }
            else if (stage == CPSS_PORT_MANAGER_EVENT_DELETE_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_DELETE");
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Starting DELETE");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_RESET_E;

                if (!(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
                    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc(devNum, portNum,
                                                                                              &rxTuneStatus, &txTuneStatus);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"ppTuneStatusGetFunc failed %d ", rc );
                        return rc;
                    }
                    if (rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
                    {
                        cpssOsTimerWkAfter(100);
                    }

                    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stopping Adaptive speed %d ", speed);
                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum,
                                    portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStageTrainMngPhases returned error code %d ",
                                                                       portNum, rc);
                        }
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_FALSE;
                    }
                }
                /* restore loopback to defaults if enabled */
                /* loopback configuration - get from DB */
                rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum), portNum, portType, &loopbackCfg, &anyLoopback);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                               portNum, rc);
                }

                /* reset to defualt setting for Auto-Negotiation sequence for 1G QSGMII/SGMII */
                if (((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E))&& (PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)))
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Enable Auto-Negotiation 1G QSGMII/SGMII");
                    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppAutoNeg1GSgmiiFunc(devNum, portNum, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", portNum, rc);
                    }
                }

                /* Disable port */
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMacEnableFunc(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Enable returned error code %d ", rc);
                    return rc;
                }

                /* loopback configuration - now set to HW */
                if (anyLoopback == GT_TRUE)
                {
                    if (((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))  ||
                        ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"port enable with MAC loopback");
                        /* mask low level interrupts (working on mac lvl only)*/
                        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    }

                    /* initialize loopback mode db to proper loopback disable which is the initial state of HW */
                    rc = prvCpssPortManagerLoopbackModeInit(portNum,loopbackCfg.loopbackType,&loopbackCfg);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d",
                                                                   portNum, rc);
                    }
                    /* set disable state to HW */
                    rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                   portNum, rc);
                    }
                }
                loopbackCfg.loopbackType = CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E;
                rc = prvCpssPortManagerLoopbackModeInit(portNum,
                                                        CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                        &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d",
                                                               portNum, rc);
                }

                /* when port is deleted, all of his stored attributes are deleted from Port Manager DB */
                rc = prvCpssPortManagerCleanPortParameters(CAST_SW_DEVNUM(devNum), portGroup, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E);
                if (rc!=GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerCleanPortParameters failed=%d",
                                                               portNum, rc);
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portModeSpeedSetBitmap);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portModeSpeedSetBitmap, portNum);

                if (curPortParams.numOfActLanes > 1)
                {
                    rc = prvCpssPortXlgMacMaskConfig(devNum, portNum, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortXlgMacMaskConfig on port %d returned %d ", portNum, rc);
                    }
                }


                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortModeSpeedSetFunc(devNum, portModeSpeedSetBitmap, GT_FALSE, ifMode, speed);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] Calling ppPortModeSpeedSetFunc"
                                                               " returned error code %d ",
                                                               portNum, rc);
                }

                rc = prvCpssPortManagerEnableRemoteLink(devNum,portNum,ifMode,speed,tmpPortManagerDbPtr,GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"prvCpssPortManagerEnableRemoteLink returned error code %d ", rc);
                    return rc;
                }


                rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform mac level maskUnmask operation", portNum);
                }
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Finish DELETE");

            }

            else /* Stage create */
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_MANAGER_EVENT_CREATE *****");
                /* oper enable is the default */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portModeSpeedSetBitmap);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portModeSpeedSetBitmap, portNum);

                tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;

                if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
                {
                    rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                    }
                }

                /* ensuring port will be in link down before portModeSpeedSet, this way when port will
                  be brought up we ensure there will be no link (if we will perform this operation after
                  portModeSpeedSet, then between portModeSpeedSet and a call to this API, peer can establish link) */
                rc = prvCpssPortManagerEnableLinkLevel(devNum, portGroup, portNum, ifMode, speed, portMode, GT_FALSE);
                if (rc!=GT_OK)
                {
                    if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
                    {
                        rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc_tmp != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                        }
                    }
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                               " returned error code %d ",
                                                               portNum, rc);
                }

                /* loopback configuration - get from DB */
                rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum), portNum, portType, &loopbackCfg, &anyLoopback);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                               portNum, rc);
                }

                /* bringing up the port using portModeSpeedSet */
                rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortModeSpeedSetFunc(devNum, portModeSpeedSetBitmap, GT_TRUE, ifMode, speed);
                if (rc != GT_OK)
                {
                    if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
                    {
                        rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                        if (rc_tmp != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                        }
                    }
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppPortModeSpeedSetFunc"
                                                               " returned error code %d ",
                                                               portNum, rc);
                }

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"setting port FEC parameter %d",
                                                      tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility);

                /* if user configured FEC mode, this is the place to set it */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility != CPSS_PORT_FEC_MODE_LAST_E)
                {
                    if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))
                   {
                        fecAbility = CPSS_PORT_FEC_MODE_DISABLED_E;
                    }
                    else
                    {
                        fecAbility = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
                    }

                    if (1/*curPortParams->portFecMode != tmpPortManagerDbPtr->portMngSmDb[portNum]->fecMode*/)
                    {
                        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppFecModeSetFunc(devNum, portNum, fecAbility);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppFecModeSetFunc returned error code returned error code %d ",
                                                                       portNum, rc);
                        }
                    }
                }



                /* if user configured rx\tx params */
                for (i=0; i<curPortParams.numOfActLanes; i++)
                {
                    CPSS_PORT_SERDES_TUNE_STC serdesParams;
                    GT_BOOL rxValid, txValid;

                    rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, i,
                                                         curPortParams.activeLanesList[i], portType, &serdesParams, &rxValid, &txValid);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx",portNum,rc);
                    }
                    if (rxValid == GT_TRUE)
                    {
                        rc = prvCpssPortManagerLaneTuneHwSet(CAST_SW_DEVNUM(devNum),portGroup,portNum, portMode,curPortParams.activeLanesList[i], GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Rx",portNum,rc);
                        }
                    }
                    if (txValid == GT_TRUE)
                    {
                        rc = prvCpssPortManagerLaneTuneHwSet(CAST_SW_DEVNUM(devNum),portGroup,portNum,portMode,curPortParams.activeLanesList[i], GT_FALSE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx",portNum,rc);
                        }
                    }
#if 0
                    /* if not set by user, then setting default 0. If set by user, setting what was desired */
                    rc = prvCpssPortManagerLanePolarityHwSet(CAST_SW_DEVNUM(devNum), portNum,curPortParams.activeLanesList[i]);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx",portNum,rc);
                    }
#endif
                }


                rxSignalOk = GT_FALSE;
                if ( !(PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
                {
                    /* clear cause bit of interrupt link_status_change */
                    uniEv = CPSS_PP_PORT_LINK_STATUS_CHANGED_E;
                    for (i=0; i<curPortParams.numOfActLanes; i++)
                    {
                        rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), portGroup, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddr);
                        if (rc!=GT_OK)
                        {
                            rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                            if (rc_tmp != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                            }
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] calling prvFindCauseRegAddrByEvent %d ", portMacNum+i, rc);
                        }
                        CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] clearing link_status_change interrupt"
                                                                  " from register 0x%08x, for mac %d", portNum, regAddr, (portMacNum+i));
                        rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0xFFFFFFFF, &data);
                        if (rc != GT_OK)
                        {
                            rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                            if (rc_tmp != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                            }
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d ",
                                                                       portNum, rc);
                        }
                    }

                    /*
                      Section for the iCal rx training as a work around for HW signal stabilizaion:
                      (A) Read the signal_detect cause bit in order to clear it, and that is because
                        when port is being powered up it raises false signal_detect interrupt.
                      (B) checking o_core_status[4] for rx_signal_ok, if it is true, then pusing
                        signal_detect interrupt to the event queue, because in such case we might
                        have cleared the real signal_detect interrupt in (A), and also, rx training
                        in stage (C) is not promised to raise signal_detect interrupt (even it it
                        ends sucessfully).
                      (C) lanuching iCal rx tune in order to stabilize signal.
                    */

                    /* (A) we read the signal_detect cause bits in order to lower it */
                    /* Reading the cause register of the interrupt */
                    uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
                    for (i=0; i<curPortParams.numOfActLanes; i++)
                    {
                        rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), portGroup, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddr);
                        if (rc!=GT_OK)
                        {
                            rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                            if (rc_tmp != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                            }
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d ",
                                                                       portNum, rc);
                        }
                        CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %2d] clearing signal_detect interrupt"
                                                                  "from register 0x%08x, for mac %d", portNum, regAddr, (portMacNum+i));
                        rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0xFFFFFFFF, &data);
                        if (rc != GT_OK)
                        {
                            rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                            if (rc_tmp != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %2d] could not perform maskUnmask operation", portNum);
                            }
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d ",
                                                                       portNum, rc);
                        }
                    }
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"clearing LINK_STATUS and SIGNAL_DETECT interrupt for mac %d numOfActLanes %d", (portMacNum), curPortParams.numOfActLanes);

                    /* unmask the mask we did prior to portModeSpeedSet */
                    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] could not perform maskUnmask operation", portNum);
                    }

                    /* loopback configuration - now set to HW (not serdes Rx2Tx)*/
                    if (anyLoopback == GT_TRUE)

                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"config loopback type %d  enableRegularTrafic %d",loopbackCfg.loopbackType,loopbackCfg.enableRegularTrafic);
                        rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                       portNum, rc);
                        }

                        if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) ||
                            ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                        {
                            if (loopbackCfg.enableRegularTrafic == GT_FALSE)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"In TX2RX mode serdes Tx is disable ");
                                rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                                if(rc != GT_OK)
                                {
                                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsPortTxEnable failed=%d", portNum, rc);
                                }
                            }
                        }
                    }

                /* (B) checking o_core_status[4] for rx_signal_ok, if it is true, then pusing
                      signal_detect interrupt to the event queue, because in such case we might
                      have cleared the real signal_detect interrupt in (A). */
                    if (!((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) && (loopbackCfg.loopbackMode.macLoopbackEnable)))
                    {
                        rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSignalOk);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssDrvPpHwRegBitMaskRead failed=%d", portNum, rc);
                        }
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"rx_signal_ok state=%d", rxSignalOk);
                        /* pusing signal_detect interrupt (that we cleared) to the event queue.
                          Only a single event needed in multi and non-multi lanes interface as we only need
                          one interrupt for the port manager to continue job processing*/

                        /* to push event anyways, need to enable macro CPSS_PORT_MANAGER_IGNORE_SIG_OK */
                        if (rxSignalOk == GT_TRUE)
                        {
                            rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum/*extraData*/, uniEv);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerPushEvent"
                                                                           " failed=%d", portNum, rc);
                            }
                        }
                    }

                    /* (C) lanuching iCal */
                    if (!((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) && (loopbackCfg.loopbackMode.macLoopbackEnable)) &&
                        !((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                    /* no need to check iCal for mac loopback */
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"lunching iCal");

                        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) &&
                            PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifMode))
                        {
                            rc = prvCpssPortSerdesLunchSingleiCal(CAST_SW_DEVNUM(devNum), portNum);
                        }
                        else
                        {
                            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc(devNum,
                                                                                                   portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
                        }
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortStageTrainMngPhases returned error code %d ",
                                                                       portNum, rc);
                        }
                    }

                    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = 0;
                }
                else if ((PRV_CPSS_PORT_MANAGER_SPEED_LOW_CHECK_MAC(speed)) && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))/* handling loopback for low speeds */
                {
                    /* loopback configuration - get from DB */
                    rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum), portNum, portType, &loopbackCfg, &anyLoopback);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                                   portNum, rc);
                    }

                    /* loopback configuration - now set to HW (not serdes Rx2Tx)*/
                    if (anyLoopback == GT_TRUE)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"config loopback type %d  enableRegularTrafic %d",loopbackCfg.loopbackType,loopbackCfg.enableRegularTrafic);
                        rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                       portNum, rc);
                        }

                        if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) ||
                            ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E)))
                        {
                            if(loopbackCfg.enableRegularTrafic == GT_FALSE)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"In TX2RX mode serdes Tx is disable ");
                                rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                                if(rc != GT_OK)
                                {
                                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsPortTxEnable failed=%d", portNum, rc);
                                }
                            }
                        }
                    }
                }

                /****************************************
                    3. Unasking events
                *****************************************/

                if (curPortParams.numOfActLanes > 1)
                {
                    CPSS_LOG_INFORMATION_MAC("handling %d XLG macs", curPortParams.numOfActLanes);
                    rc = prvCpssPortXlgMacMaskConfig(devNum, portNum, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortXlgMacMaskConfig returned %d ", portNum, rc);
                    }
                }

                tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDownHandled=GT_TRUE;

                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = 1;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = 1;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = 1;

                if (tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
                {
                    tmpPortManagerDbPtr->portManagerGlobalUseFlag = 1;
                }


                /* if MAC loopback is Enabled, push signal detect event to procede to INIT
                  TODO: add support for PCS and SerDes Loopback */
                if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)&&(loopbackCfg.loopbackMode.macLoopbackEnable))
                {
                     stage = CPSS_PORT_MANAGER_EVENT_INIT_E;
                     tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
                     rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum, uniEv);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerPushEvent"
                                                                   " failed=%d", portNum, rc);
                    }
                }

            }

        } /* if (stage == STAGE_INIT || stage == STAGE_DISABLE || stage == STAGE_DELETE ) */

    }
    return rc;
}

GT_STATUS internal_cpssPortManagerStatusGet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC  *portStagePtr
)
{

    GT_STATUS rc;
    GT_U32 portMacNum;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_AP_STATUS_STC apStatus;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    if (!CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /*PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);*/
    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        /*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] failed ppCheckAndGetMacFunc %d",portNum, rc);*/
        return rc;
    }

    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(portStagePtr);

    portStagePtr->portState = CPSS_PORT_MANAGER_STATE_RESET_E;
    portStagePtr->portUnderOperDisable = GT_FALSE;
    portStagePtr->failure = CPSS_PORT_MANAGER_FAILURE_NONE_E;

    /* verify allocation - if not allocated, returning GT_OK with above initial statuses (RESET...)*/
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 0);
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);
    }

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged == GT_TRUE)
    {
        /*portStagePtr->isPortUnderManagement = GT_TRUE;*/
        portStagePtr->portState = tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM;
        portStagePtr->portUnderOperDisable = tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM;
        portStagePtr->failure = tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason;
    }

    portStagePtr->ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    portStagePtr->speed = CPSS_PORT_SPEED_NA_E;
    portStagePtr->fecType = CPSS_PORT_FEC_MODE_LAST_E;




    if (portStagePtr->portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        switch (tmpPortManagerDbPtr->portMngSmDb[portNum]->portType)
        {
        case CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
            portStagePtr->ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
            portStagePtr->speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
            portStagePtr->fecType = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
            break;
        case CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApStatusGetFunc(CAST_SW_DEVNUM(devNum),portNum,&apStatus);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssPortApPortStatusGet:rc=%d\n",
                                                           portNum, rc);
            }
            portStagePtr->ifMode = apStatus.portMode.ifMode;
            portStagePtr->speed = apStatus.portMode.speed;
            portStagePtr->fecType =  apStatus.fecType;

            break;
        default:
            ;
        }
    }
    return GT_OK;
}

GT_STATUS prvCpssPortManagerDbgInitPorts
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32 timeOut
)
{

    /* Variables declarations */
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_STATUS rc;
    GT_BOOL  portsExistFlag;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    CPSS_PORT_MANAGER_STC portEventStc;
    GT_U32 dbgCounter;

    portsExistFlag = GT_TRUE;
    dbgCounter = 0;

    while (portsExistFlag == GT_TRUE)
    {
        portsExistFlag = GT_FALSE;

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            /************************************************************************
              1. Get port status
            **********************************************************************/

            rc = internal_cpssPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
            if (rc != GT_OK)
            {
                continue;/*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, " Calling internal_cpssPortManagerStatusGet returned %d ", rc);*/
            }

            /************************************************************************
              2. If port state is INIT than continue to call INIT_EVENT
            **********************************************************************/
            if (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
            {
                /*cpssOsPrintf CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"performing DBG INIT");*/
                portsExistFlag = GT_TRUE;
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
                rc = internal_cpssPortManagerEventSet(devNum, portNum, &portEventStc);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling internal_cpssPortManagerEventSet returned %d ",
                                                               portNum, rc);
                }
            }
        }

        dbgCounter++;

        if (timeOut>0)
        {
            cpssOsTimerWkAfter(timeOut);
        }

        /* for preventing GalTis from context hang */
        if (dbgCounter>10000)
        {
            prvCpssCyclicLoggerLogEntriesDump();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %2d] prvCpssPortManagerDbgInitPorts reached time-out", portNum);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerDbgDumpPortState(GT_SW_DEV_NUM devNum, GT_PHYSICAL_PORT_NUM portNum)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    char *stateStr;
    char *failStr;

    rc = internal_cpssPortManagerStatusGet(devNum, portNum, &portConfigOutParams);

    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling internal_cpssPortManagerStatusGet returned %d ", portNum, rc);
    }

    switch (portConfigOutParams.portState)
    {
    case CPSS_PORT_MANAGER_STATE_RESET_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_RESET_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LINK_DOWN_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LINK_DOWN_E";
        break;
    case CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LINK_UP_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LINK_UP_E";
        break;
    case CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E";
        break;
    case CPSS_PORT_MANAGER_STATE_FAILURE_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_FAILURE_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LAST_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LAST_E";
        break;
    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                                   "[Port %2d] portState undefined %d ",portNum, portConfigOutParams.portState);
    }

    switch (portConfigOutParams.failure)
    {
    case CPSS_PORT_MANAGER_FAILURE_NONE_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_NONE_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_SIGNAL_STABILITY_FAILED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_SIGNAL_STABILITY_FAILED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_ALIGNMENT_TIMER_EXPIRED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_ALIGNMENT_TIMER_EXPIRED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_CONFIDENCE_INTERVAL_TIMER_EXPIRED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_CONFIDENCE_INTERVAL_TIMER_EXPIRED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_LAST_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_LAST_E";
    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                                   "[Port %2d] portFail undefined %d ",portNum, portConfigOutParams.failure);
    }
    cpssOsPrintf("[Port %2d] state: %s, fail: %s, operDisabled: %d ",
                 portNum,
                 stateStr,
                 failStr,
                 /*portConfigOutParams.isPortUnderManagement,*/
                 portConfigOutParams.portUnderOperDisable);
    return GT_OK;
}

/**
* @internal internal_cpssPortManagerGlobalParamsOverride
*      function
* @endinternal
*
* @brief  Override global parameters, using flags to check if
*     override or using defualts
*
* @param[in] devNum        - device number
* @param[in] globalParamsStcPtr  - stc with the
*                  parameters to override
*
* @retval GT_OK          - on success
* @retval GT_BAD_PARAM       - on wrong port number or device
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PTR        - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerGlobalParamsOverride
(
    IN GT_U8                  devNum,
    IN CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC   *globalParamsStcPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    /*CPSS_PORT_MANAGER_MODE_PARAMS_STC*/
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (globalParamsStcPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Port manager not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E, globalParamsStcPtr->globalParamsBitmapType))
    {
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval
        = globalParamsStcPtr->signalDetectDbCfg.sdChangeInterval;
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize
        = globalParamsStcPtr->signalDetectDbCfg.sdChangeMinWindowSize;
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired
        = globalParamsStcPtr->signalDetectDbCfg.sdCheckTimeExpired;

    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Valid for Bobcat3 and Aldrin2*/
    {
        if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_HIGH_SPEED_PORTS_E, globalParamsStcPtr->globalParamsBitmapType))
        {
            rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortProprietaryHighSpeedPortsSetFunc(devNum, globalParamsStcPtr->propHighSpeedPortEnabled);
            if (rc != GT_OK)
            {
                if (rc == GT_NOT_APPLICABLE_DEVICE)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "High speed ports configuration is not valid to this device");
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Setting CPU ports resources for High Speed ports failed");
                }
            }

        }
    }

    return GT_OK;
}


/**
* @internal internal_cpssPortManagerInit function
* @endinternal
*
* @brief   Init the portManager
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerInit
(
    IN  GT_U8                                   devNum
)
{
    GT_STATUS rc= GT_OK;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Port manager not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Valid for Bobcat3 and Aldrin2*/
    {
        rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortMgrInitFunc(devNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to INit Port Manager");
        }
    }

    return GT_OK;

}


GT_STATUS internal_cpssPortManagerInitParamsStruct
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParams
)
{

    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(portParams);

    /* Init for the time being */
    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->magic = CPSS_PM_MAGIC;

    portParams->portType = portType;
    switch (portType)
    {
    case  CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
        /* Mandatory params init*/
        portParams->portParamsType.regPort.ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        portParams->portParamsType.regPort.speed = CPSS_PORT_SPEED_NA_E;
        portParams->portParamsType.regPort.portAttributes.fecMode = CPSS_PORT_FEC_MODE_DISABLED_E;
        break;
    case  CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
        /* Mandatory params init*/
        portParams->portParamsType.apPort.modesArr[0].ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        portParams->portParamsType.apPort.modesArr[0].speed = CPSS_PORT_SPEED_NA_E;
        portParams->portParamsType.apPort.modesArr[0].fecRequested = CPSS_PORT_FEC_MODE_DISABLED_E;
        portParams->portParamsType.apPort.modesArr[0].fecSupported = CPSS_PORT_FEC_MODE_DISABLED_E;
        break;
    default:
        CPSS_LOG_INFORMATION_MAC("Port type: %d", portType);
    }

    return GT_OK;
}

GT_STATUS prvCpssPmRegPortParamsSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PM_PORT_PARAMS_STC *portParams
)
{

    /**
     ** Variables
     **/

    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                sdVecSize;
    GT_U16                               *sdVectorPtr;
    GT_U32                                sdMax;

    GT_U32                                lanesArrIdx = 0;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC     *portSmDbPtr;
    GT_U32                                bm;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
    *confiData   = NULL;

    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams  = NULL;
    GT_U32                                targetLane  = 0xffffffff;
    CPSS_PORT_SERDES_TX_CONFIG_STC        serdesTxParams;
    CPSS_PORT_SERDES_RX_CONFIG_STC        serdesRxParams;
    GT_BOOL                               rxParamsOk;
    GT_BOOL                               txParamsOk;
    PRV_CPSS_PORT_MANAGER_SERDES_PARAMS_TYPE_ENT
    sdParamsType = PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E;
    CPSS_PORT_SERDES_TUNE_STC             serdesParams;
    GT_BOOL                               rxValid;
    GT_BOOL                               txValid;

#if 0
    /* TODO: Q */
    GT_U32                              sdLanes;
#endif

    /**
     ** Checks
     **/


    if (   !portParams
           || portParams->magic != CPSS_PM_MAGIC
           || portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL/uninitialised or portType not valid.");
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &pmDB)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    /* check state machine legality */
    if (   (pmDB != NULL)
           && (pmDB->portMngSmDb != NULL)
           && (pmDB->portMngSmDb[portNum] != NULL)
           && (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "(port %d)port not in reset state.", portNum);
    }

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(port %d)prvCpssPortManagerVerifyAllocation SM_DB failed=%d", portNum, rc);
    }

    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(port %d)prvCpssPortManagerVerifyAllocation PHYSICAL failed=%d", portNum, rc);
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d)ppCheckAndGetMacFunc failed=%d", portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), portParams->portParamsType.regPort.ifMode,
                                               portParams->portParamsType.regPort.speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                   rc, portParams->portParamsType.regPort.ifMode, portParams->portParamsType.regPort.speed);
    }

    if ((CPSS_PORT_INTERFACE_MODE_QSGMII_E == portParams->portParamsType.regPort.ifMode) ||
        (CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E == portParams->portParamsType.regPort.ifMode))
    {
        /* provide to HWS first port in quadruplet and it will configure all other ports if needed */
        portMacNum &= 0xFFFFFFFC;
    }

    /* port mode sanity check */
    rc = mvHwsPortValidate(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "port mode not supported=%d",rc);
    }

    /* TODO: Q: Shouldn't we clean-up these DB entries, if the API fails? */

    /**
     ** Basic mandatory params
     **/

    pmDB->portMngSmDb[portNum]->portType = portParams->portType;
    pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb = portParams->portParamsType.regPort.ifMode;
    pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb = portParams->portParamsType.regPort.speed;

    /* now that hws ifMode exist, we can verify alocation of serdes, because serdes lane numbers exist in hws */
    /* verify serdes allocation */

    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_SERDES_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(port %d)prvCpssPortManagerVerifyAllocation SERDES failed=%d",portNum, rc);
    }

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum),portNum,PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_MAC_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(port %d)prvCpssPortManagerVerifyAllocation MAC failed=%d",portNum, rc);
    }

    /**
     ** Lane Params
     **/

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
    }

    pmDB->portMngSmDb[portNum]->fecAbility = curPortParams.portFecMode;
    sdVecSize = curPortParams.numOfActLanes;
    sdVectorPtr = curPortParams.activeLanesList;
    sdMax = sdVecSize;

#if 0
    /* TODO: Q: Do we need to give this option of total lanes?
     * Currently, we do not have totalLanes as a parameter
     */

    sdLanes = portModeParamsPtr->modeParamsArr[i].param.serdesPolarity.totalLanes;

    if (sdLanes != PORT_MANAGER_ALL_PORT_LANES)
    {
        if (sdLanes<=sdVecSize)
        {
            sdMax = sdLanes;
        }
        else
        {
            sdMax = sdVecSize;
        }
    }
    else
    {
        sdMax = sdVecSize;
    }
#endif

    for (lanesArrIdx = 0; lanesArrIdx < sdMax; lanesArrIdx++)
    {
        laneParams      = &(portParams->portParamsType.regPort.laneParams[lanesArrIdx]);
        targetLane      = 0xffffffff;
        rxParamsOk      = GT_FALSE;
        txParamsOk      = GT_FALSE;
        sdParamsType    = PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E;
        bm              = laneParams->validLaneParamsBitMask;


        if (bm)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "(lane %d): SET PARAMS valid_bm=0x%x", lanesArrIdx, bm);
            if (CPSS_PM_LANE_PARAM_IS_VALID_BITMASK (bm, CPSS_PM_LANE_PARAM_GLOBAL_LANE_NUM_E) )
            {
                targetLane = laneParams->globalLaneNum;
            }
            else
            {
                targetLane = sdVectorPtr[lanesArrIdx];
            }

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"(lane %d): SET PARAMS targetLane=0x%x", lanesArrIdx, targetLane);

            /* validate target serdes lane */
            rc = prvCpssPortManagerValidateSerdesLanes(portNum, targetLane, sdVectorPtr, sdVecSize);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"(portNum %d)prvCpssPortManagerValidateSerdesLanes failed=%d"
                                                           "on portMode=%d ", portNum, rc, portMode);
            }

            rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, lanesArrIdx, targetLane,
                                                 CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &serdesParams, &rxValid, &txValid);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortManagerLaneTuneDbGet failed=%d", rc);
            }

            if ( bm & CPSS_PM_LANE_PARAM_RX_E )
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "(lane %d) : setting port serdes RX params bandWidth %d, dcGain %d,"
                                                           "ffeCap %d, ffeRes %d, loopBandwidth %d, sqlch %d",
                                                           lanesArrIdx, laneParams->rxParams.bandWidth, laneParams->rxParams.dcGain, laneParams->rxParams.ffeCap,
                                                           laneParams->rxParams.ffeRes, laneParams->rxParams.loopBandwidth, laneParams->rxParams.sqlch);

                serdesParams.BW     =    laneParams->rxParams.bandWidth;
                serdesParams.DC     =    laneParams->rxParams.dcGain;
                serdesParams.HF     =    laneParams->rxParams.ffeCap;
                serdesParams.LF     =    laneParams->rxParams.ffeRes;
                serdesParams.LB     =    laneParams->rxParams.loopBandwidth;
                serdesParams.sqlch  =    laneParams->rxParams.sqlch;

                rxParamsOk = GT_TRUE;
            }

            if ( bm & CPSS_PM_LANE_PARAM_TX_E )
            {

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "(lane %d) : setting port serdes TX parameters txAmp %d, "
                                                           "emph0 %d, emph1 %d, txAmpAdjEn %d, txAmpShft %d ",
                                                           lanesArrIdx, laneParams->txParams.txAmp, laneParams->txParams.emph0, laneParams->txParams.emph1,
                                                           laneParams->txParams.txAmpAdjEn, laneParams->txParams.txAmpShft);

                serdesParams.txAmp         =    laneParams->txParams.txAmp;
                serdesParams.txEmphAmp     =    laneParams->txParams.emph0;
                serdesParams.txEmph1       =    laneParams->txParams.emph1;

                if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    serdesParams.txAmpAdj    =   laneParams->txParams.txAmpAdjEn;
                    serdesParams.txAmpShft   =   laneParams->txParams.txAmpShft;
                }

                txParamsOk = GT_TRUE;
            }

            if (txParamsOk || rxParamsOk)
            {

                /* Get the Tx & Rx defaults if required - i.e. only if the current Tx or Rx config is not valid */
                if (!txParamsOk && (txValid == GT_FALSE))
                {
                    rc = cpssPortTxRxDefaultsDbEntryGet(CAST_SW_DEVNUM(devNum), portMode, curPortParams.serdesSpeed ,&serdesTxParams, NULL);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"cpssPortTxRxDefaultsDbEntryGet failed=%d", rc);
                    }

                    /* tx params */
                    serdesParams.txAmp      = serdesTxParams.txAmp;
                    serdesParams.txEmphAmp  = serdesTxParams.emph0;
                    serdesParams.txEmph1    = serdesTxParams.emph1;

                    serdesParams.txAmpAdj   = serdesTxParams.txAmpAdjEn;
                    serdesParams.txAmpShft  = serdesTxParams.txAmpShft;

                    txParamsOk = GT_TRUE;
                }

                if (!rxParamsOk && (rxValid == GT_FALSE))
                {
                    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        rc = cpssPortTxRxDefaultsDbEntryGet(CAST_SW_DEVNUM(devNum), portMode, curPortParams.serdesSpeed, NULL, &serdesRxParams);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"cpssPortTxRxDefaultsDbEntryGet failed=%d", rc);
                        }

                        /* TODO: Q: ??*/
                        /* Add spl case of Avago Serdes */
                        /* ctle params */
                        serdesParams.BW     = serdesRxParams.bandWidth;
                        serdesParams.DC     = serdesRxParams.dcGain;
                        serdesParams.HF     = serdesRxParams.ffeCap;
                        serdesParams.LF     = serdesRxParams.ffeRes;
                        serdesParams.LB     = serdesRxParams.loopBandwidth;
                        serdesParams.sqlch  = serdesRxParams.sqlch;

                        serdesParams.ffeC   = serdesRxParams.ffeCap;
                        serdesParams.ffeR   = serdesRxParams.ffeRes;
                        serdesParams.sqlch  = serdesRxParams.sqlch;
                        serdesParams.align90 = serdesRxParams.align90;

                        rxParamsOk = GT_TRUE;
                    }
                }

                if (txParamsOk)
                    if (rxParamsOk)
                        sdParamsType = PRV_CPSS_PORT_MANAGER_SERDES_BOTH_TXRX_PARAMS_E;
                    else
                        sdParamsType = PRV_CPSS_PORT_MANAGER_SERDES_TX_PARAMS_E;
                else if (rxParamsOk)
                    sdParamsType = PRV_CPSS_PORT_MANAGER_SERDES_RX_PARAMS_E;


                rc = prvCpssPortManagerLaneTuneDbSet(CAST_SW_DEVNUM(devNum), portNum, lanesArrIdx, targetLane,
                                                     CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, sdParamsType, &serdesParams);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortManagerLaneTuneDbSet failed=%d", rc);
                }
            }
        }
    }

    /**
     ** Port Attributes
     **/

    portAttr    = &(portParams->portParamsType.regPort.portAttributes);
    bm          = portAttr->validAttrsBitMask;
    portSmDbPtr = pmDB->portMngSmDb[portNum];

    if (bm)
    {
        if ( bm & CPSS_PM_ATTR_TRAIN_MODE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "trainMode = %d", portAttr->trainMode);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E);
            portSmDbPtr->userOverrideTrainMode = portAttr->trainMode;
        }

        if ( bm & CPSS_PM_ATTR_RX_TRAIN_SUPP_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "adaptRxTrainSuppMode = %d", portAttr->adaptRxTrainSupp);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E);
            portSmDbPtr->userOverrideAdaptiveRxTrainSupported = portAttr->adaptRxTrainSupp;
        }

        if ( bm & CPSS_PM_ATTR_EDGE_DETECT_SUPP_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "edgeDetectSuppMode = %d", portAttr->edgeDetectSupported);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E);
            portSmDbPtr->userOverrideEdgeDetectSupported = portAttr->edgeDetectSupported;
        }

        if ( bm & CPSS_PM_ATTR_LOOPBACK_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Port Attr Loopback type = %d", portAttr->loopback);

            rc = prvCpssPortManagerLoopbackModesDbSet(CAST_SW_DEVNUM(devNum), portNum, portParams->portType,
                                                      &(portAttr->loopback));
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d)prvCpssPortManagerLoopbackModesDbSet failed=%d", portNum, rc);
            }
        }

        if ( bm & CPSS_PM_ATTR_ET_OVERRIDE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "minLF = %d", portAttr->etOverride.minLF);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "maxLF = %d", portAttr->etOverride.maxLF);

            portSmDbPtr->overrideEtParams = GT_TRUE;
            portSmDbPtr->min_LF           = portAttr->etOverride.minLF;
            portSmDbPtr->max_LF           = portAttr->etOverride.maxLF;
        }

        if ( bm & CPSS_PM_ATTR_FEC_MODE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "FEC mode = %d", portAttr->fecMode);
            portSmDbPtr->fecAbility = portAttr->fecMode;
        }

        if ( bm & CPSS_PM_ATTR_CALIBRATION_E )
        {
            confiData = &(portAttr->calibrationMode.confidenceCfg);
            portSmDbPtr->calibrationMode.calibrationType = portAttr->calibrationMode.calibrationType;

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "calibration type = %d", portAttr->calibrationMode.calibrationType);

            if (portSmDbPtr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "CI - lfMin = %d, lfMax=%d, hfMin=%d, hfMax=%d, eoMin=%d, eoMax=%d bitmap=0x%08x",
                                                           confiData->lfMinThreshold, confiData->lfMaxThreshold, confiData->hfMinThreshold,
                                                           confiData->hfMaxThreshold, confiData->eoMinThreshold, confiData->eoMaxThreshold,
                                                           confiData->confidenceEnableBitMap);

                portSmDbPtr->calibrationMode.minLfThreshold   = confiData->lfMinThreshold;
                portSmDbPtr->calibrationMode.maxLfThreshold   = confiData->lfMaxThreshold;
                portSmDbPtr->calibrationMode.minHfThreshold   = confiData->hfMinThreshold;
                portSmDbPtr->calibrationMode.maxHfThreshold   = confiData->hfMaxThreshold;
                portSmDbPtr->calibrationMode.minEoThreshold   = confiData->eoMinThreshold;
                portSmDbPtr->calibrationMode.maxEoThreshold   = confiData->eoMaxThreshold;
                portSmDbPtr->calibrationMode.confidenceBitMap = confiData->confidenceEnableBitMap;
            }
        }

        if ( bm & CPSS_PM_ATTR_UNMASK_EV_MODE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"unMask Events Mode  %d",
                                                       portAttr->unMaskEventsMode);
            portSmDbPtr->unMaskEventsMode = portAttr->unMaskEventsMode;
        }


    }
    return rc;
}

GT_STATUS prvCpssPmRegPortParamsGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PM_PORT_PARAMS_STC *portParams
)
{
  /**
   ** Variables
   **/

    CPSS_PORT_INTERFACE_MODE_ENT          ifMode;
    CPSS_PORT_SPEED_ENT                   speed;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB            = NULL;
    GT_BOOL                               anyLb;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
    *confiData       = NULL;
    GT_STATUS                             rc              = GT_OK;
    GT_U32                                portMacNum      = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                sdVecSize;

    GT_U32                                lanesArrIdx     = 0;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC     *portSmDbPtr;

    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams      = NULL;

    CPSS_PORT_SERDES_TX_CONFIG_STC        serdesTxParams;
    CPSS_PORT_SERDES_RX_CONFIG_STC        serdesRxParams;
    CPSS_PORT_SERDES_TUNE_STC             serdesParams;
    GT_BOOL                               rxValid;
    GT_BOOL                               txValid;
    MV_HWS_AUTO_TUNE_RESULTS              results;

    /**
    ** Checks
    **/
    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &pmDB)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d)ppCheckAndGetMacFunc failed=%d", portNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %2d] port parameters not set on port", portNum);
    }

    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    if (portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "PortType is not Regular. Is unexpextedly AP.");
    }

    PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(pmDB, portMacNum, 1);
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(pmDB, portNum, 1);

    portParams->portParamsType.regPort.ifMode = pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portParams->portParamsType.regPort.speed = pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;

    ifMode = portParams->portParamsType.regPort.ifMode;
    speed = portParams->portParamsType.regPort.speed;

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Params Get speed=%d, ifMode=%d ", speed, ifMode);

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
    if (rc != GT_OK || portMode >= LAST_PORT_MODE)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d. portMode = %d",
                                                   portNum, portMode, rc);
    }

    /**
    ** Lane Params
    **/
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
    }

    sdVecSize = curPortParams.numOfActLanes;

#if 0
    /* TODO: Q: Do we need to give this option of total lanes?
    * Currently, we do not have totalLanes as a parameter
    */

    sdLanes = portModeParamsPtr->modeParamsArr[i].param.serdesPolarity.totalLanes;

    if (sdLanes != PORT_MANAGER_ALL_PORT_LANES)
    {
        if (sdLanes<=sdVecSize)
        {
            sdMax = sdLanes;
        }
        else
        {
            sdMax = sdVecSize;
        }
    }
    else
    {
        sdMax = sdVecSize;
    }

#endif

    for (lanesArrIdx = 0; lanesArrIdx < sdVecSize; lanesArrIdx++)
    {
        laneParams        = &(portParams->portParamsType.regPort.laneParams[lanesArrIdx]);
        txValid           = GT_FALSE;
        rxValid           = GT_FALSE;

        if (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E &&
            pmDB->portMngSmDb[portNum]->portOperDisableSM != GT_TRUE)
        {

            rc = mvHwsSerdesAutoTuneResult(CAST_SW_DEVNUM(devNum), 0, curPortParams.activeLanesList[lanesArrIdx],
                                           HWS_DEV_SERDES_TYPE(devNum), &results);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %2d] mvHwsSerdesAutoTuneResult failed=%d", portNum, rc);
            }

            serdesParams.txAmp      = results.txAmp;
            serdesParams.txAmpAdj   = results.txAmpAdj;
            serdesParams.txEmphAmp  = results.txEmph0;
            serdesParams.txEmph1    = results.txEmph1;
            serdesParams.txAmpShft  = results.txAmpShft;
            txValid                 = GT_TRUE;

            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                serdesParams.BW     = results.BW;
                serdesParams.DC     = results.DC;
                serdesParams.HF     = results.HF;
                serdesParams.LB     = results.LB;
                serdesParams.LF     = results.LF;
                serdesParams.sqlch  = results.sqleuch;
                serdesParams.align90= 0;
            }
            else
            {
                serdesParams.DC      = 0;
                serdesParams.LB      = 0;
                serdesParams.BW      = 0;
                serdesParams.HF      = results.ffeC;
                serdesParams.LF      = results.ffeR;
                serdesParams.align90 = results.align90;
                serdesParams.sqlch   = results.sqleuch;
            }
            rxValid = GT_TRUE;


        }
        else
        {
            rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, lanesArrIdx,
                                                 curPortParams.activeLanesList[lanesArrIdx],
                                                 CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &serdesParams, &rxValid, &txValid);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling prvCpssPortManagerLaneTuneDbGet failed."
                                                           " returned error code %d ", portNum, rc);
            }
        }


        /* CPSS_PM_LANE_PARAM_TX_E */
        if (txValid == GT_FALSE)
        {

            rc = cpssPortTxRxDefaultsDbEntryGet(CAST_SW_DEVNUM(devNum), portMode, curPortParams.serdesSpeed , &serdesTxParams, NULL);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssPortTxRxDefaultsDbEntryGet on portMode %d"
                                                           " returned error code %d ", portNum, portMode, rc);
            }

            laneParams->txParams.emph0       = serdesTxParams.emph0;
            laneParams->txParams.emph1       = serdesTxParams.emph1;
            laneParams->txParams.txAmp       = serdesTxParams.txAmp;
            laneParams->txParams.txAmpAdjEn  = serdesTxParams.txAmpAdjEn;
            laneParams->txParams.txAmpShft   = serdesTxParams.txAmpShft;
        }
        else
        {
            laneParams->txParams.emph0       = serdesParams.txEmphAmp;
            laneParams->txParams.emph1       = serdesParams.txEmph1;
            laneParams->txParams.txAmp       = serdesParams.txAmp;
            laneParams->txParams.txAmpAdjEn  = serdesParams.txAmpAdj;
            laneParams->txParams.txAmpShft   = serdesParams.txAmpShft;
        }

        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            laneParams->txParams.txAmpAdjEn   = 0;
            laneParams->txParams.txAmpShft    = 0;
        }
        CPSS_PM_SET_VALID_LANE_PARAM(portParams, lanesArrIdx, CPSS_PM_LANE_PARAM_TX_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "(lane %d): GET port serdes TX parameters txAmp %d, "
                                                   "emph0 %d, emph1 %d, txAmpAdjEn %d, txAmpShft %d ",
                                                   lanesArrIdx, laneParams->txParams.txAmp, laneParams->txParams.emph0, laneParams->txParams.emph1,
                                                   laneParams->txParams.txAmpAdjEn, laneParams->txParams.txAmpShft);


        /* CPSS_PM_LANE_PARAM_RX_E */
        if (rxValid == GT_FALSE)
        {
            if (!(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)))
            {
                laneParams->rxParams.bandWidth         = 0;
                laneParams->rxParams.ffeCap            = 0;
                laneParams->rxParams.ffeRes            = 0;
                laneParams->rxParams.loopBandwidth     = 0;
                laneParams->rxParams.sqlch             = 0;
                laneParams->rxParams.dcGain            = 0;
                laneParams->rxParams.align90           = 0;

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "rx not supported for ComPhy currently - all zeros");

                break;
            }

            rc = cpssPortTxRxDefaultsDbEntryGet(CAST_SW_DEVNUM(devNum), portMode,curPortParams.serdesSpeed, NULL, &serdesRxParams);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssPortRxRxDefaultsDbEntryGet on portMode %d"
                                                           " returned error code %d ", portNum, portMode, rc);
            }

            laneParams->rxParams.ffeCap          = serdesRxParams.ffeCap;
            laneParams->rxParams.ffeRes          = serdesRxParams.ffeRes;
            laneParams->rxParams.loopBandwidth   = serdesRxParams.loopBandwidth;
            laneParams->rxParams.bandWidth       = serdesRxParams.bandWidth;
            laneParams->rxParams.sqlch           = serdesRxParams.sqlch;
            laneParams->rxParams.dcGain          = serdesRxParams.dcGain;
            laneParams->rxParams.align90         = serdesRxParams.align90;

        }
        else
        {
            laneParams->rxParams.ffeCap          = serdesParams.HF;
            laneParams->rxParams.ffeRes          = serdesParams.LF;
            laneParams->rxParams.loopBandwidth   = serdesParams.LB;
            laneParams->rxParams.bandWidth       = serdesParams.BW;
            laneParams->rxParams.sqlch           = serdesParams.sqlch;
            laneParams->rxParams.dcGain          = serdesParams.DC;
            laneParams->rxParams.align90         = serdesParams.align90;

        }
        CPSS_PM_SET_VALID_LANE_PARAM(portParams, lanesArrIdx, CPSS_PM_LANE_PARAM_RX_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "(lane %d) : get port serdes RX params bandWidth %d, dcGain %d,"
                                                   "ffeCap %d, ffeRes %d, loopBandwidth %d, sqlch %d", lanesArrIdx,
                                                   laneParams->rxParams.bandWidth, laneParams->rxParams.dcGain, laneParams->rxParams.ffeCap,
                                                   laneParams->rxParams.ffeRes, laneParams->rxParams.loopBandwidth, laneParams->rxParams.sqlch);



        /* CPSS_PM_LANE_PARAM_GLOBAL_LANE_NUM_E */
        /* TODO: Q NOT IMPLEMENTED*/

    }

    /**
     ** Port Attributes
     **/

    portAttr    = &(portParams->portParamsType.regPort.portAttributes);
    portSmDbPtr = pmDB->portMngSmDb[portNum];

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E))
    {
        portAttr->trainMode = portSmDbPtr->userOverrideTrainMode;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "trainMode = %d", portAttr->trainMode);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E))
    {
        portAttr->adaptRxTrainSupp = portSmDbPtr->userOverrideAdaptiveRxTrainSupported;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "adaptRxTrainSuppMode = %d", portAttr->adaptRxTrainSupp);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E))
    {
        portAttr->edgeDetectSupported = portSmDbPtr->userOverrideEdgeDetectSupported;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_EDGE_DETECT_SUPP_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "edgeDetectSuppMode = %d", portAttr->edgeDetectSupported);
    }

    rc = prvCpssPortManagerLoopbackModesDbGet(CAST_SW_DEVNUM(devNum), portNum, portParams->portType,
                                              &(portAttr->loopback), &anyLb);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                   "(portNum %d)prvCpssPortManagerLoopbackModesDbSet failed=%d", portNum, rc);
    }
    CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_LOOPBACK_E);
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "Port Attr Loopback = %d", portAttr->loopback);

    if ( portSmDbPtr->overrideEtParams == GT_TRUE)
    {
        portAttr->etOverride.minLF = portSmDbPtr->min_LF;
        portAttr->etOverride.maxLF = portSmDbPtr->max_LF;

        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "minLF = %d", portAttr->etOverride.minLF);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "maxLF = %d", portAttr->etOverride.maxLF);
    }

    portAttr->fecMode = portSmDbPtr->fecAbility;
    CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_FEC_MODE_E );
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "FEC mode = %d", portAttr->fecMode);


    portAttr->calibrationMode.calibrationType = portSmDbPtr->calibrationMode.calibrationType;
    confiData = &(portAttr->calibrationMode.confidenceCfg);
    CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_CALIBRATION_E);
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "calibration type = %d", portAttr->calibrationMode.calibrationType);
    if (portSmDbPtr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
    {

        confiData->lfMinThreshold           = portSmDbPtr->calibrationMode.minLfThreshold;
        confiData->lfMaxThreshold           = portSmDbPtr->calibrationMode.maxLfThreshold;
        confiData->hfMinThreshold           = portSmDbPtr->calibrationMode.minHfThreshold;
        confiData->hfMaxThreshold           = portSmDbPtr->calibrationMode.maxHfThreshold;
        confiData->eoMinThreshold           = portSmDbPtr->calibrationMode.minEoThreshold;
        confiData->eoMaxThreshold           = portSmDbPtr->calibrationMode.maxEoThreshold;
        confiData->confidenceEnableBitMap   = portSmDbPtr->calibrationMode.confidenceBitMap;

        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC(
                                             "(port %d) CI - lfMin = %d, lfMax=%d, hfMin=%d, hfMax=%d, eoMin=%d, eoMax=%d bitmap=0x%08x",
                                             confiData->lfMinThreshold, confiData->lfMaxThreshold, confiData->hfMinThreshold,
                                             confiData->hfMaxThreshold, confiData->eoMinThreshold, confiData->eoMaxThreshold,
                                             confiData->confidenceEnableBitMap);

    }
    else
    {
        confiData->confidenceEnableBitMap = 0x00000000;
    }

    portAttr->unMaskEventsMode = portSmDbPtr->unMaskEventsMode;
    CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "unmask events mode = %d", portAttr->unMaskEventsMode);


    if (rc == GT_OK)
    {
        portParams->magic = CPSS_PM_MAGIC;
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "portParamsGet rc = %d", rc);

    return rc;
}

GT_STATUS prvCpssPmApPortParamsSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PM_PORT_PARAMS_STC *portParams
)
{
    /**
     ** Variables
     **/
    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                i,bm;
    CPSS_PM_AP_PORT_ADV_STC              *mode;
    CPSS_PM_AP_PORT_ATTR_STC             *apAttrs;
    GT_U32                                minNumOfLanes = PORT_MANAGER_MAX_LANES;
#if 0
    /* TODO: Q */
    GT_U32                              sdLanes;
#endif

    /**
     ** Checks
     **/

    if ((!portParams) ||
        (portParams->magic != CPSS_PM_MAGIC) ||
        (portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL/uninitialised or portType not valid.");
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &pmDB)));
    if (rc!=GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");

    if ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApConfigSetFunc == NULL) ||
        (PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApStatusGetFunc == NULL) ||
        (PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortApSerdesTxParamsOffsetSetFunc == NULL))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %2d] AP APIs not configured", portNum);
    }

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E);
    if (rc!=GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerVerifyAllocation AP failed=%d", portNum, rc);

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E);
    if (rc!=GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(port %d)prvCpssPortManagerVerifyAllocation SM_DB failed=%d", portNum, rc);

    /* check state machine legality */
    if (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "(port %d)port not in reset state.", portNum);

    if (CPSS_PORT_AP_IF_ARRAY_SIZE_CNS < portParams->portParamsType.apPort.numOfModes)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[Port %2d] AP port too many parameters=%d",
                                                   portNum, portParams->portParamsType.apPort.numOfModes);
    }

    /* TODO: Q: Shouldn't we clean-up these DB entries, if the API fails? */

    /**
     ** Copying all advertised mode related info
     **/

    pmDB->portMngSmDb[portNum]->portType = portParams->portType;

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppCheckAndGetMacFunc failed=%d", portNum, rc);

    pmDB->portsApAttributedDb[portNum]->perPhyPortApNumOfModes = portParams->portParamsType.apPort.numOfModes;
    for (i = 0; i < portParams->portParamsType.apPort.numOfModes; i++)
    {
        mode = &(portParams->portParamsType.apPort.modesArr[i]);
        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), mode->ifMode, mode->speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                       portNum, rc, mode->ifMode, mode->speed);
        }

        rc = mvHwsPortValidate (CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
        if (rc != GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] port mode not supported=%d", portNum, rc);

        pmDB->portsApAttributedDb[portNum]->perPhyPortDb[i].ifModeDb       = mode->ifMode;
        pmDB->portsApAttributedDb[portNum]->perPhyPortDb[i].speedDb        = mode->speed;
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fecRequestedArr[i] = mode->fecRequested;
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fecAbilityArr[i]   = mode->fecSupported;

        /*
         * Lane Params
         */
        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "hwsPortModeParamsGet returned %d", rc );
        }

        if (minNumOfLanes > curPortParams.numOfActLanes)
            minNumOfLanes = curPortParams.numOfActLanes;
    } /* for numOfModes */

    /**
     ** AP Port Attributes
     **/
    apAttrs = &(portParams->portParamsType.apPort.apAttrs);
    bm      = apAttrs->validAttrsBitMask;
    pmDB->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask = bm;

    if (bm & CPSS_PM_AP_PORT_ATTR_NONCE_E)
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.noneceDisable   = apAttrs->nonceDisable;
    else
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.noneceDisable   = PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE;

    if (bm & CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E)
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcPause         = apAttrs->fcPause;
    else
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcPause         = PRV_PORT_MANAGER_DEFAULT_FC_PAUSE;

    if (bm & CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E)
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcAsmDir        = apAttrs->fcAsmDir;
    else
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcAsmDir        = PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR;

    if (bm & CPSS_PM_AP_PORT_ATTR_LANE_NUM_E)
    {
        if (apAttrs->negotiationLaneNum >= minNumOfLanes)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] negotiationLaneNum %d is Invalid", portNum, apAttrs->negotiationLaneNum);

        pmDB->portsApAttributedDb[portNum]->apAttributesStc.laneNum         = apAttrs->negotiationLaneNum;
    }
    else
        pmDB->portsApAttributedDb[portNum]->apAttributesStc.laneNum         = PRV_PORT_MANAGER_DEFAULT_LANE_NUM;

    /*if (bm & CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E)
    {
         TODO: Q TBD
    }*/

    if (bm)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "AP attrs set:  valid_bm = 0x%x, nonceDisa=%d, fcPause=%d, fcAsmDir=%d, negoLaneNum=%d",
                                                   bm, apAttrs->nonceDisable, apAttrs->fcPause,apAttrs->fcAsmDir, apAttrs->negotiationLaneNum);
    }

    return rc;
}

GT_STATUS prvCpssPmApPortParamsGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  CPSS_PM_PORT_PARAMS_STC *portParams
)
{
    /**
     ** Variables
     **/
    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;

    GT_U32                                i;
    CPSS_PM_AP_PORT_ADV_STC              *mode;
    CPSS_PM_AP_PORT_ATTR_STC             *apAttrs;
#if 0
    /* TODO: Q */
    GT_U32                              sdLanes;
#endif

    /**
     ** Checks
     **/
    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &pmDB)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] ppCheckAndGetMacFunc failed=%d", portNum, rc);
    }

    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);
    PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %2d] port parameters not set on port", portNum);
    }

    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    /**
     ** Copying all advertised mode related info
     **/

    portParams->portParamsType.apPort.numOfModes = pmDB->portsApAttributedDb[portNum]->perPhyPortApNumOfModes;
    for (i = 0; i < portParams->portParamsType.apPort.numOfModes; i++)
    {
        mode = &(portParams->portParamsType.apPort.modesArr[i]);

        mode->ifMode        = pmDB->portsApAttributedDb[portNum]->perPhyPortDb[i].ifModeDb;
        mode->speed         = pmDB->portsApAttributedDb[portNum]->perPhyPortDb[i].speedDb;
        mode->fecRequested  = pmDB->portsApAttributedDb[portNum]->apAttributesStc.fecRequestedArr[i];
        mode->fecSupported  = pmDB->portsApAttributedDb[portNum]->apAttributesStc.fecAbilityArr[i];

        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), mode->ifMode, mode->speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                       portNum, rc, mode->ifMode, mode->speed);
        }

        rc = mvHwsPortValidate (CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] port mode not supported=%d", portNum, rc);
        }

    } /* for numOfModes */


    /**
     ** AP Port Attributes
     **/

    /*
       CPSS_PM_AP_PORT_ATTR_NONCE_E,
       CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E,
       CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E,
       CPSS_PM_AP_PORT_ATTR_LANE_NUM_E,
       CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E,
    */
    apAttrs                       = &(portParams->portParamsType.apPort.apAttrs);
    apAttrs->validAttrsBitMask    = 0x00000000;

    apAttrs->nonceDisable         = pmDB->portsApAttributedDb[portNum]->apAttributesStc.noneceDisable;
    if (pmDB->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_NONCE_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_NONCE_E);

    apAttrs->fcPause              = pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcPause;
    if (pmDB->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams,CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);

    apAttrs->fcAsmDir             = pmDB->portsApAttributedDb[portNum]->apAttributesStc.fcAsmDir;
    if (pmDB->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams, CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);

    apAttrs->negotiationLaneNum   = pmDB->portsApAttributedDb[portNum]->apAttributesStc.laneNum;
    if (pmDB->portsApAttributedDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_LANE_NUM_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams, CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"AP attrs:  valid_bm = 0x%x, nonceDisa=%d, fcPause=%d, fcAsmDir=%d, negoLaneNum=%d",
                                          apAttrs->validAttrsBitMask, apAttrs->nonceDisable, apAttrs->fcPause,
                                          apAttrs->fcAsmDir, apAttrs->negotiationLaneNum );
    /* Override Params */
    /*   CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E, */
    /* TODO: Q */

    return rc;
}

GT_STATUS internal_cpssPortManagerPortParamsSet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PM_PORT_PARAMS_STC *portParams
)
{
    GT_STATUS rc;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (   !portParams
           || portParams->magic != CPSS_PM_MAGIC
           || portParams->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL/uninitialised or portType not valid.");
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %2d] port manager not supported for %d device",
                                                   portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum,"Stage: PORT_PARAMETER_SET *****");

    if (portParams->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] **invalid portType %d in parameters set",
                                                   portNum, portParams->portType);
    }

    if (portParams->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = prvCpssPmRegPortParamsSet(devNum, portNum, portParams);
    }
    else
    {
        rc = prvCpssPmApPortParamsSet(devNum, portNum, portParams);
    }

    return rc;
}

GT_STATUS internal_cpssPortManagerPortParamsGet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PM_PORT_PARAMS_STC *portParams
)
{
   /**
     ** Variables
     **/

    PRV_CPSS_PORT_MNG_DB_STC             *pmDB            = NULL;
    GT_STATUS                             rc              = GT_OK;
    GT_U32                                portMacNum      = 0;

    /**
     ** Checks
     **/
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %2d] port manager not supported for %d device",
                                                portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    rc = ((PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppPortManagerDbGetFunc(devNum, &pmDB)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");
    }

    rc = PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d)ppCheckAndGetMacFunc failed=%d", portNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %2d] port parameters not set on port", portNum);
    }

    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    switch (portParams->portType)
    {
    case CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
        rc = prvCpssPmRegPortParamsGet(devNum, portNum, portParams);
        break;
    case CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
        rc = prvCpssPmApPortParamsGet(devNum, portNum, portParams);
        break;
    default:
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Problem with Port Type");
    }


    if (rc == GT_OK)
    {
        portParams->magic = CPSS_PM_MAGIC;
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(portNum, "portParamsGet rc = %d", rc);

    return rc;
}

