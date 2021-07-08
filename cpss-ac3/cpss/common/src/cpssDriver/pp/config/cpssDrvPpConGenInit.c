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
* @file cpssDrvPpConGenInit.c
*
* @brief Low level driver initialization of PPs, and declarations of global
* variables
*
* @version   30
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssDriver/log/private/prvCpssDriverLog.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpssDriver/pp/interrupts/cpssDrvInterrupts.h>

#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>

#ifdef  INCL_EXDXMX_DRIVER
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#endif

#define DEBUG_PRINT(x) cpssOsPrintf x

/*****************************************************************************
* Global
******************************************************************************/
/* array of pointers to the valid driver devices
   index to this array is devNum */
PRV_CPSS_DRIVER_PP_CONFIG_STC* prvCpssDrvPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS] =
    {NULL};

/* array including all driver management interface objects in Unit       */
PRV_CPSS_DRV_OBJ_STC* prvCpssDrvPpObjConfig[PRV_CPSS_MAX_PP_DEVICES_CNS] =
    {NULL};

/* driver global data that should be saving in HSU*/
PRV_CPSS_DRV_GLOBAL_DATA_STC drvGlobalInfo = {0, 0};


/* DB to store device type id
   Note: The DB used for debug purpose only
*/
GT_U16   prvCpssDrvDebugDeviceId[PRV_CPSS_MAX_PP_DEVICES_CNS];

/*  DB to store flag for tracing hw write per device
        GT_TRUE: HW trace write access is enabled
        GT_FALSE: HW trace write access is disabled
*/
GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];

/*  DB to store flag for tracing hw read per device
        GT_TRUE: HW trace read access is enabled
        GT_FALSE: HW trace read access is disabled
*/
GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];

/*  DB to store flag for tracing hw delay per device
        GT_TRUE: HW trace delay access is enabled
        GT_FALSE: HW trace delay access is disabled
*/
GT_BOOL  prvCpssDrvTraceHwDelay[PRV_CPSS_MAX_PP_DEVICES_CNS];

GT_BOOL   prvCpssDrvTraceHwWriteBeforeAccess = GT_FALSE;/* by default 'trace' after the 'write accessing' */
/* allow to update the TRACE of 'write' operations before the actual access */
void prvCpssDrvTraceHwWriteBeforeAccessSet(IN GT_BOOL BeforeAccess)
{
    prvCpssDrvTraceHwWriteBeforeAccess = BeforeAccess;
}

static GT_VOID prvFindCauseRegAddr
(
    IN  PRV_CPSS_DRV_INTERRUPT_SCAN_STC* intScanRoot,
    IN  GT_U32 maskReg ,
    OUT GT_U32 *regAddr,
    OUT GT_BOOL *found,
    OUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC **foundNodePtrPtr
);
/*****************************************************************************
* Externals
******************************************************************************/
extern PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC prvCpssEventExtDataConvertFunc;

/**
* @internal prvFindInterruptIdx function
* @endinternal
*
* @brief   This function finds interrupt index according to the given
*         unified event and extra data.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] uniEvent                 - unified event
* @param[in] evExtData                - extra data for the unified event
*
* @param[out] interruptIdx             - (pointer to) the find lode index
*                                       None.
*/
GT_STATUS prvFindInterruptIdx
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupId,
    IN  CPSS_UNI_EV_CAUSE_ENT   uniEvent,
    IN  GT_U32                  evExtData,
    OUT GT_U32                  *interruptIdx
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC *nodePoolArr;
    GT_U32 event;
    GT_BOOL found;

    found = GT_FALSE;

    nodePoolArr = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

    for (event = 0; event < prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits; event++)
    {
        if (nodePoolArr[event].uniEvCause != ((GT_U32) uniEvent))
        {
            continue;
        }

        if(evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
        {
            if(nodePoolArr[event].uniEvExt != evExtData)
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        *interruptIdx = event;
        found = GT_TRUE;
        break;
    }

    if (found == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvFindIntCauseBit function
* @endinternal
*
* @brief   This function finds cause bit withing a cause register
*         for a specific interrupt.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] uniEvent                 - unified event
* @param[in] evExtData                - extra data for the unified event
*
* @param[out] bit                      - (pointer to) the  offset in the register
*                                       None.
*/
GT_STATUS prvFindIntCauseBit
(
    IN  GT_U8               devNum,
    IN  GT_U32              portGroupId,
    IN  CPSS_UNI_EV_CAUSE_ENT uniEvent,
    IN  GT_U32              evExtData,
    OUT GT_U32              *bit
)
{
    GT_STATUS   rc;
    GT_U32      interruptIdx;


    rc = prvFindInterruptIdx(devNum, portGroupId, uniEvent, evExtData, &interruptIdx);

    if (rc!=GT_OK)
    {
        return rc;
    }

    *bit =  1 << (interruptIdx & 0x1f);

    return GT_OK;
}

/**
* @internal prvFindCauseRegAddrByInterrupt function
* @endinternal
*
* @brief   This function finds cause register address according to the given
*         interrupt index.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] intIdx                   - interrupt index
*                                      evExtData   - extra data for the unified event
*
* @param[out] regAddr                  - (pointer to) the cause register address
*                                       None.
*/
GT_STATUS prvFindCauseRegAddrByInterrupt
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  intIdx,
    OUT GT_U32  *regAddr
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC *foundNodePtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *foundNodePtr2;
    GT_BOOL found;

    foundNodePtr = &(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool[intIdx]);

    /*

           prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId]
                         /                          \
                        /                            \
                 nodesPool                          ScanTree
        (PRV_CPSS_DRV_EV_REQ_NODE_STC)      (PRV_CPSS_DRV_INTERRUPT_SCAN_STC)

    */
    prvFindCauseRegAddr(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot, foundNodePtr->intMaskReg/* foundNodePtr->maskRegAddr*/,
                       regAddr, &found, &foundNodePtr2);

    if (!found)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC((GT_STATUS)GT_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvFindCauseRegAddrByEvent function
* @endinternal
*
* @brief   This function finds the cause register address according to the given
*         unified event and extra data.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] uniEvent                 - unified event
* @param[in] evExtData                - extra data for the unified event
*
* @param[out] regAddr                  - (pointer to) the cause register address
*                                       None.
*/
GT_STATUS prvFindCauseRegAddrByEvent
(
    IN  GT_U8               devNum,
    IN  GT_U32              portGroupId,
    IN  CPSS_UNI_EV_CAUSE_ENT uniEvent,
    IN  GT_U32              evExtData,
    OUT GT_U32              *regAddr
)
{
    GT_STATUS rc;
    GT_U32 interruptIdx;

    rc = prvFindInterruptIdx(devNum, portGroupId, uniEvent, evExtData, &interruptIdx);

    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = prvFindCauseRegAddrByInterrupt(devNum, portGroupId, interruptIdx, regAddr);

    if (rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal drvEventMaskDevice function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @param[in] devNum                   - PP's device number .
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
* @param[in] operation                - the  : mask / unmask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
static GT_STATUS drvEventMaskDevice
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation
)
{
    GT_STATUS  rc = GT_OK;
    GT_U32     portGroupId;/* port group Id */
    GT_BOOL    maskSetOk = GT_FALSE;
    GT_BOOL    allowToContinue;

    /* check that the device exists and properly initialized */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    /* supporting 'MAC-in-PHY' */
    if(prvCpssDrvPpConfig[devNum]->intCtrl.remoteDevice_drvEventMaskDeviceFunc)
    {
        /* check with the 'remote device' if this event :
            1. not relevant to it
            2. partially relevant and partially to 'our device'
            3. relevant ONLy to it
        */
        rc = prvCpssDrvPpConfig[devNum]->intCtrl.remoteDevice_drvEventMaskDeviceFunc(
            devNum,uniEvent, evExtData, operation ,
            & allowToContinue);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (allowToContinue == GT_FALSE)
        {
            /* the event relevant only to the remote device and no more needed
                from 'Our device' */
            return GT_OK;
        }
    }


    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        rc = prvCpssDrvEvReqUniEvMaskSet(devNum,
                        prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool,
                        prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits,
                            (GT_U32)uniEvent, evExtData, operation);
         /*
                GT_OK means at least one mask was set in the portGroup.
                GT_NOT_FOUND means this event and extra data was not found for this
                portGroupId.
                Some of the event don't exists in all port group therefore it is ok to not find the
                unified event or extra data in a port group. All port groups must
                be searched before the event is said to not be found.
             */
        if (rc == GT_OK)
        {
            maskSetOk = GT_TRUE;
        }

        /* GT_OK and GT_NOT_FOUND are only allowed */
        if ((rc != GT_NOT_FOUND) && (rc != GT_OK))
        {
            return rc;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    if (maskSetOk)
    {
        return GT_OK;
    }

    return rc;
}

/**
* @internal drvEventMaskGetDevice function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @param[in] devNum                   - PP's device number .
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
*                                      operation  - the operation : mask / unmask
*
* @param[out] maskedPtr                - is interrupt masked or unmasked
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
static GT_STATUS drvEventMaskGetDevice
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData,
    OUT CPSS_EVENT_MASK_SET_ENT *maskedPtr
)
{
    GT_STATUS  rc = GT_OK;
    GT_U32     portGroupId;/* port group Id */
    GT_BOOL    maskGetOk = GT_FALSE;

    /* check that the device exists and properly initialized */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        rc = prvCpssDrvEvReqUniEvMaskGet(devNum,
                        prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool,
                        prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits,
                            (GT_U32)uniEvent, evExtData, maskedPtr);
        if (rc == GT_OK)
        {
            maskGetOk = GT_TRUE;
        }
        else
        {
            return rc;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    if (maskGetOk)
    {
        return GT_OK;
    }

    return rc;
}


/**
* @internal prvCpssDrvPpUniEvPerPortHit function
* @endinternal
*
* @brief   This routine tests unified event, and if event is 'per-port' -
*         returns port's bit position and mask in extended data for specific event.
* @param[in] uniEvent                 - the tested unified event.
*
* @param[out] portBitPtr               - (pointer to) the port's bit position in extData for specific event,
*                                      Relevant for 'per-port' unified event.
* @param[out] portMaskPtr              - (pointer to) the port's mask in extData for specific event.
*                                      Relevant for 'per-port' unified event.
*
* @retval GT_TRUE                   - the unified event is 'per-port' type.
* @retval GT_FALSE                  - the unified event is not 'per-port' type.
*
* @note This function is used in conversion of MAC-to-PHY and PHY-to-MAC
*       for 'per-port' unified events.
*
*/
GT_BOOL prvCpssDrvPpUniEvPerPortHit
(
    IN GT_U32               uniEvent,
    OUT GT_U32              *portBitPtr,
    OUT GT_U32              *portMaskPtr
)
{
    *portBitPtr = 0;
    *portMaskPtr = 0xFFFFFFFF;

    if (uniEvent >= CPSS_PP_PORT_LINK_STATUS_CHANGED_E &&
        uniEvent <= CPSS_PP_PORT_COUNT_EXPIRED_E)
    {
        if (uniEvent != CPSS_PP_INTERNAL_PHY_E)
        {
            return GT_TRUE;
        }
    }
    else if (uniEvent == CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E)
    {
        return GT_TRUE;
    }
    else if (uniEvent >= CPSS_PP_PORT_LANE_PRBS_ERROR_E &&
             uniEvent <= CPSS_PP_PORT_LANE_DETECTED_IIAII_E)
    {
        /* per Port Per lane events  formula is: (*evExtDataPtr) = (port) << 8 + (lane); */
        *portBitPtr = 8;
        *portMaskPtr = ~((1 << *portBitPtr) - 1);
        return GT_TRUE;
    }
    else if ( (uniEvent >= CPSS_PP_PORT_PCS_LINK_STATUS_CHANGED_E &&
               uniEvent <= CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E)
            ||(uniEvent == CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E) )
    {
        return GT_TRUE;
    }
    else if (uniEvent == CPSS_PP_PORT_EEE_E)
    {
        /*  per port number and 'specific event', formula is: (*evExtDataPtr) = (port) << 8 + (specific event); */
        *portBitPtr = 8;
        *portMaskPtr = ~((1 << *portBitPtr) - 1);
        return GT_TRUE;
    }

    /* indicate the caller that the event is NOT 'per port' */
    return GT_FALSE;
}

/* flag to allow the 'MAC' portNumFrom that was not mapped to 'physical port' ,
   but still we want to get indication about it

   BIT_31 - is indication of 'raw MAC' number
*/
static GT_U32   allowNonMappedMac = 0;
void drvEventExtDataConvert_allowNonMappedMac(IN GT_U32 allow)
{
    allowNonMappedMac = allow;
}

/**
* @internal internal_drvEventExtDataConvert function
* @endinternal
*
* @brief   This routine converts extended data for unified event on specific device.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
* @param[in] devNum                   - PP's device number.
* @param[in] uniEvent                 - the unified event.
* @param[in] evConvertType            - convert type for specific event.
* @param[in,out] evExtDataPtr             - (pointer to) additional data (port num / lane number / other)
*                                      the event was received upon.
* @param[in,out] evExtDataPtr             - (pointer to) converted/unchanged additional data (port num / lane number / other )
*                                      the event was received upon.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, or event convert type
* @retval GT_BAD_PTR               - evExtDataPtr is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - not supported for current device family
*
* @note This function called in scope of cpssEventRecv and converts extended data if need.
*       For example - MAC number or local RX/TX DMA number per-port events
*       will be converted to physical port numbers defined by cpssDxChPortPhysicalPortMapSet.
*
*/
static GT_STATUS internal_drvEventExtDataConvert
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *evExtDataPtr
)
{
    GT_STATUS   rc;
    GT_U32      portNumFrom;
    GT_U32      portNumTo;
    GT_U32      portBit;
    GT_U32      portMask;

    if (!prvCpssDrvPpConfig[devNum])
    {
        /* device was removed */
        return GT_OK;
    }

    if (prvCpssDrvPpConfig[devNum]->intCtrl.eventPortMapConvertFunc == NULL)
    {
        /* not required for this device type */
        return GT_OK;
    }

    if(*evExtDataPtr == PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS ||
       *evExtDataPtr == CPSS_PARAM_NOT_USED_CNS)
    {
        return GT_OK;
    }

    if(prvCpssDrvPpConfig[devNum]->intCtrl.eventGenericConvertFunc &&
       GT_TRUE == prvCpssDrvEventIsCpuSdmaPortPerQueue(devNum,uniEvent))
    {
        /* the event is per CPU SDMA port per queue */
        /* let the 'generic' convert function to handle it */
        return prvCpssDrvPpConfig[devNum]->intCtrl.eventGenericConvertFunc(devNum,
                uniEvent,    *evExtDataPtr/*IN_evExtData*/,
                evConvertType,evExtDataPtr/*OUT_evExtDataPtr*/);
    }


    if (GT_FALSE == prvCpssDrvPpUniEvPerPortHit(uniEvent, &portBit, &portMask))
    {
        /* the event is not 'per-port' */
        return GT_OK;
    }

    /* extract port that will be converted from extended data */
    portNumFrom = (*evExtDataPtr) >> portBit;

    if((portNumFrom & MARK_REMOTE_PHYSICAL_PORT_INT_CNS) == MARK_REMOTE_PHYSICAL_PORT_INT_CNS)
    {
        /* remote physical port . need to remove the 'indication' */
        (*evExtDataPtr) = CLEAR_3_MSBits_MARK_INT_MAC(*evExtDataPtr);
        return GT_OK;
    }

    rc = prvCpssDrvPpConfig[devNum]->intCtrl.
        eventPortMapConvertFunc(devNum,evConvertType,portNumFrom,&portNumTo);
    if (rc != GT_OK)
    {
        if(rc == GT_NOT_FOUND && allowNonMappedMac)
        {
            /* the 'MAC' portNumFrom was not mapped to 'physical port' ,
               but still we want to get indication about it

               BIT_31 - is indication of 'raw MAC' number
            */
            *evExtDataPtr = BIT_31 | (portNumFrom << portBit);

            return GT_OK;
        }


        return rc;
    }
    /* set converted port to extended data */
    *evExtDataPtr &= ~portMask;
    *evExtDataPtr |= (portNumTo << portBit);

    return GT_OK;
}

/**
* @internal drvEventExtDataConvert function
* @endinternal
*
* @brief   This routine converts extended data for unified event on specific device.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
* @param[in] devNum                   - PP's device number.
* @param[in] uniEvent                 - the unified event.
* @param[in] evConvertType            - convert type for specific event.
* @param[in,out] evExtDataPtr             - (pointer to) additional data (port num / lane number / other)
*                                      the event was received upon.
* @param[in,out] evExtDataPtr             - (pointer to) converted/unchanged additional data (port num / lane number / other )
*                                      the event was received upon.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, or event convert type
* @retval GT_BAD_PTR               - evExtDataPtr is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - not supported for current device family
*
* @note This function called in scope of cpssEventRecv and converts extended data if need.
*       For example - MAC number or local RX/TX DMA number per-port events
*       will be converted to physical port numbers defined by cpssDxChPortPhysicalPortMapSet.
*
*/
static GT_STATUS drvEventExtDataConvert
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *evExtDataPtr
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc= internal_drvEventExtDataConvert(devNum, uniEvent, evConvertType, evExtDataPtr);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_drvMacEventExtDataConvert function
* @endinternal
*
* @brief   This routine converts extended data for Mac related unified events on specific device.
*
* @note   APPLICABLE DEVICES:      Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
* @param[in] devNum                   - PP's device number.
* @param[in,out] evExtDataPtr             - (pointer to) additional data (port num)
*                                      the event was received upon.
* @param[in,out] evExtDataPtr             - (pointer to) converted/unchanged additional data (port num)
*                                      the event was received upon.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, or event convert type
* @retval GT_BAD_PTR               - evExtDataPtr is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - not supported for current device family
*
* @note This function called in scope of cpssEventRecv and converts extended data if need.
*
*/
static GT_STATUS internal_drvMacEventExtDataConvert
(
    IN GT_U8                    devNum,
    INOUT GT_U32                *evExtDataPtr
)
{
    GT_STATUS   rc;

    if (!prvCpssDrvPpConfig[devNum])
    {
        /* device was removed */
        return GT_OK;
    }

    if (prvCpssDrvPpConfig[devNum]->intCtrl.macEventPortMapConvertFunc == NULL)
    {
        /* not required for this device type */
        CPSS_LOG_INFORMATION_MAC("mac extra data convertion function pointer is null");
        return GT_OK;
    }

    rc = prvCpssDrvPpConfig[devNum]->intCtrl.
        macEventPortMapConvertFunc(devNum,evExtDataPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mac extra data convertion function pointer returned error %d", rc);
    }

    return GT_OK;
}

/**
* @internal drvMacEventExtDataConvert function
* @endinternal
*
* @brief   This routine converts extended data for Mac related unified events on specific device.
*
* @note   APPLICABLE DEVICES:      Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
* @param[in] devNum                   - PP's device number.
* @param[in,out] evExtDataPtr             - (pointer to) additional data (port num)
*                                      the event was received upon.
* @param[in,out] evExtDataPtr             - (pointer to) converted/unchanged additional data (port num)
*                                      the event was received upon.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, or event convert type
* @retval GT_BAD_PTR               - evExtDataPtr is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - not supported for current device family
*
* @note This function called in scope of cpssEventRecv and converts extended data if need.
*
*/
static GT_STATUS drvMacEventExtDataConvert
(
    IN GT_U8                    devNum,
    INOUT GT_U32                *evExtDataPtr
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_drvMacEventExtDataConvert(devNum,evExtDataPtr);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvFindCauseRegAddr function
* @endinternal
*
* @brief   This function finds cause register address in
*         CPSS_INTERRUPT_SCAN_STC tree by mask register address
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] intScanRoot              - CPSS_INTERRUPT_SCAN_STC tree.
* @param[in] maskReg                  - Address of the interrupt mask register.
*
* @param[out] regAddr                  - Address of the interrupt cause register.
* @param[out] found                    - true if cause register address was  in
*                                      CPSS_INTERRUPT_SCAN_STC tree, false otherwise.
* @param[out] foundNodePtrPtr          - (pointer to) the point to found node.
*                                       None.
*/
static GT_VOID prvFindCauseRegAddr
(
    IN  PRV_CPSS_DRV_INTERRUPT_SCAN_STC* intScanRoot,
    IN  GT_U32 maskReg ,
    OUT GT_U32 *regAddr,
    OUT GT_BOOL *found,
    OUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC **foundNodePtrPtr
)
{
    GT_U32 i;
    if (maskReg==intScanRoot->maskRegAddr)
    {
        *foundNodePtrPtr = intScanRoot;
        *regAddr=intScanRoot->causeRegAddr;
        *found=GT_TRUE;
        return;
    }
    for(i = 0; i < intScanRoot->subIntrListLen; i++)
    {
        prvFindCauseRegAddr(intScanRoot->subIntrScan[i],maskReg,regAddr,found,
            foundNodePtrPtr);
    }
    return;
}

GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId GT_UNUSED,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId GT_UNUSED,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/**
* @internal drvEventGenerateDevice function
* @endinternal
*
* @brief   This debug routine configures device to generate unified event for
*         specific element in the event associated with extra data.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      Use CPSS_PARAM_NOT_USED_CNS to generate events for all supported evExtData.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event has no interrupts associated with it in
*                                       the device Or the 'extra data' has value that not relate
*                                       to the uniEvent
* @retval GT_NOT_INITIALIZED       - the CPSS was not initialized properly to handle
*                                       this type of event
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported unify event
*/
static GT_STATUS drvEventGenerateDevice
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData
)
{
    GT_U32            eventItr; /* event iterator */
    GT_STATUS   rc = GT_OK; /* return code*/
    GT_U32      portGroupId; /* port group Id */
    PRV_CPSS_DRV_EV_REQ_NODE_STC node; /*interrupt node*/
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC* intScanRoot; /*interrupt information hierarchy tree root*/
    GT_U32      mask; /* cause register mask */
    GT_U32      maskReg; /* address of mask register */
    GT_U32      causeReg; /* address of cause register */
    GT_U32      nodesPoolSize; /* size of nodes pool */
    GT_U32      found=0; /* number of found events in nodes pool */
    GT_BOOL     foundCause; /* variable indicates that cause register address was found
                             in intScanRoot tree (true - found; false - not found) */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *foundNodePtr;

    /* check that the device exists and properly initialized */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);
    /* check the device callback is not null */
    CPSS_NULL_PTR_CHECK_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.unitIntGenerationCheckFunc);

    if (prvCpssEventExtDataConvertFunc)
    {
        /* invoke the callback function to convert event's extended data */
        rc = (prvCpssEventExtDataConvertFunc)(devNum, uniEvent,
                                              PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
                                              &evExtData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*Get Size of Nodes Pool*/
    nodesPoolSize=prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits;
    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        /*Find unified event by number or additional data and write bit of cause register*/
        for (eventItr = 0; eventItr < nodesPoolSize; eventItr++)
        {
            node=prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool[eventItr];
            if (node.uniEvCause != (GT_U32)uniEvent)
            {
                continue;
            }
            if(evExtData != CPSS_PARAM_NOT_USED_CNS)
            {
                if(node.uniEvExt != evExtData)
                {
                    /* we need to generate event only for the specific interrupts and not all
                     * interrupts that connected to the unified event */
                    continue;
                }
            }
            found++;
            /*Get mask and mask register address*/
            mask = node.intBitMask;
            maskReg = node.intMaskReg;
            /*Get Cause Register address*/
            intScanRoot = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot;
            foundCause = GT_FALSE;
            foundNodePtr = NULL;
            prvFindCauseRegAddr(intScanRoot,maskReg,&causeReg,&foundCause,&foundNodePtr);
            if (foundCause == GT_TRUE && foundNodePtr)
            {
                if ((uniEvent >= CPSS_SRVCPU_IPC_E) && (uniEvent <= CPSS_SRVCPU_MAX_E))
                {
                    GT_U32 data;
                    prvCpssGenericSrvCpuRegisterRead(devNum, 0, causeReg, &data);
                    prvCpssGenericSrvCpuRegisterWrite(devNum,0, causeReg, data | mask);
                    rc = GT_OK;
                }
                else
                if(PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum, foundNodePtr->maskRegAddr) ||
                   PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(foundNodePtr->maskRegAddr))
                {
                    /* the address is in PEX or MG memory space .. need proper functions */

                    /* currently assume that those units not know to generate interrupts */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                        "The evExtData[%d] is in PEX or MG#(1/2/3) memory space , not supporting generating of interrupts",
                        evExtData);
                }
                else
                {
                    /*Check if the event can be generated*/
                    rc = prvCpssDrvPpConfig[devNum]->intCtrl.unitIntGenerationCheckFunc(devNum, causeReg);
                    if (rc != GT_OK)
                        return rc;
                    /*Write data to cause register to generate event*/
                    rc = prvCpssDrvHwPpWriteRegBitMask(devNum,causeReg,mask,mask);
                    if (rc != GT_OK)
                        return rc;
                }
            }
            else
            {
                /*Return GT_FAIL if cause register address hasn't been found in intScanRoot tree*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        if (found == 0)
        {
            /*Return GT_NOT_FOUND if unified event hasn't been found in nodes pool*/
            return /*do not register into the ERROR LOG */ GT_NOT_FOUND;
        }
        if (rc != GT_OK)
            return rc;
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId);
    return GT_OK;
}

/**
* @internal drvEventGppIsrConnect function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] gppId                    - The Gpp Id to be connected.
* @param[in] isrFuncPtr               - A pointer to the function to be called on Gpp interrupt
*                                      reception.
* @param[in] cookie                   - A  to be passed to the isrFuncPtr when its called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
static GT_STATUS drvEventGppIsrConnect
(
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  void                    *cookie
)
{
    /* check that the device exists and properly initialized */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    return prvCpssDrvDxExMxGppIsrConnect(devNum,gppId,isrFuncPtr,cookie);
}

/**
* @internal prvCpssDrvSysConfigPhase1 function
* @endinternal
*
* @brief   This function sets cpssDriver system level system configuration
*         parameters before any of the device's phase1 initialization .
*
* @note   APPLICABLE DEVICES:      ALL systems must call this function
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
*/
GT_STATUS prvCpssDrvSysConfigPhase1
(
    void
)
{
    GT_STATUS   rc = GT_NOT_INITIALIZED;
    GT_U32      i;
    static GT_BOOL initPrvCpssDrvDebugDeviceId = GT_TRUE;

    cpssOsMemSet(prvCpssDrvPpConfig,0,sizeof(prvCpssDrvPpConfig));
    cpssOsMemSet(prvCpssDrvPpObjConfig,0,sizeof(prvCpssDrvPpObjConfig));
    drvGlobalInfo.performReadAfterWrite = GT_TRUE;
    drvGlobalInfo.prvCpssDrvAfterDbRelease = GT_FALSE;

    /* bind a callback function for the mask/unmask of unify-events for ALL PP
        devices */
    rc = prvCpssGenEventMaskDeviceBind(PRV_CPSS_DEVICE_TYPE_PP_E,&drvEventMaskDevice);

    /* bind a callback function that generates events*/
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssGenEventMaskGetDeviceBind(PRV_CPSS_DEVICE_TYPE_PP_E,&drvEventMaskGetDevice);
    if(rc != GT_OK)
    {
        return rc;
    }


    rc = prvCpssGenEventGenerateDeviceBind(PRV_CPSS_DEVICE_TYPE_PP_E,&drvEventGenerateDevice);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssGenEventExtDataConvertBind(&drvEventExtDataConvert);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssMacEventExtDataConvertBind(&drvMacEventExtDataConvert);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  initialize debug DB */
    if( GT_TRUE == initPrvCpssDrvDebugDeviceId )
    {
        for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            prvCpssDrvDebugDeviceId[i] = 0;
        }
        initPrvCpssDrvDebugDeviceId = GT_FALSE;
    }

    /*  initialize hw access trace DBs */
    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        prvCpssDrvTraceHwWrite[i] = GT_FALSE;
        prvCpssDrvTraceHwRead[i] = GT_FALSE;
        prvCpssDrvTraceHwDelay[i] = GT_FALSE;
    }


    /* build the needed bus objects */
#ifdef  INCL_EXDXMX_DRIVER
    rc = prvCpssGenEventGppIsrConnectBind(&drvEventGppIsrConnect);
    if(rc != GT_OK)
    {
        return rc;
    }
#ifdef GT_PCI
    prvCpssDrvMngInfPciPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfPciPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    /* pointer to the object that control PCI */
    rc = prvCpssDrvHwPciDriverObjectInit();
    if(rc != GT_OK)
    {
        return rc;
    }
    prvCpssDrvMngInfPexMbusPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfPexMbusPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    /* pointer to the object that control PEX with 8 Address Completion Ranges  */
    rc = prvCpssDrvHwPexMbusDriverObjectInit();
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*GT_PCI*/

#ifdef GT_SMI
    prvCpssDrvMngInfSmiPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfSmiPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    /* pointer to the object that control SMI */
    rc = prvCpssDrvHwSmiDriverObjectInit();
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*GT_SMI*/

#ifdef GT_I2C
    prvCpssDrvMngInfTwsiPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfTwsiPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    /* pointer to the object that control TWSI */
    rc = prvCpssDrvHwTwsiDriverObjectInit();
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*GT_I2C*/

/*#ifdef GT_PEX

    prvCpssDrvMngInfPexPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfPexPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    * pointer to the object that control PEX *
    rc = prvCpssDrvHwPexDriverObjectInit();

    if(rc != GT_OK)
    {
        return rc;
    }
#endif*/

#if defined GT_PCI  /*|| defined GT_PEX*/

    /* HA StandBy driver allocation for PCI and PEX management interfaces */
    prvCpssDrvMngInfPciHaStandbyPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    cpssOsMemSet(prvCpssDrvMngInfPciHaStandbyPtr,0,sizeof(PRV_CPSS_DRV_MNG_INF_OBJ_STC));
    /* pointer to the object that control PCI -- for HA standby */
    rc = prvCpssDrvHwPciStandByDriverObjectInit();
    if(rc != GT_OK)
    {
        return rc;
    }

#endif /*GT_PEX*/
#endif

    return rc;
}


/**
* @internal prvCpssDrvHwPpPhase1Init function
* @endinternal
*
* @brief   This function is called by cpss "phase 1" family functions,
*         in order to enable PP Hw access,
*         the device number provided to this function may
*         be changed when calling prvCpssDrvPpHwPhase2Init().
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] ppOutInfoPtr             - (pointer to)the info that driver return to caller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPhase1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC *ppOutInfoPtr
)
{
    GT_STATUS rc;

    if(prvCpssDrvPpObjConfig[devNum])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    if(prvCpssDrvMngInfPciPtr == NULL &&
       prvCpssDrvMngInfPexMbusPtr == NULL &&
       prvCpssDrvMngInfPciHaStandbyPtr == NULL &&
       prvCpssDrvMngInfSmiPtr == NULL &&
       prvCpssDrvMngInfTwsiPtr == NULL)
    {
        /* this is adding of first device after all devices were removed */
        rc = prvCpssDrvSysConfigPhase1();
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* create the device object for the functions for this device */
    prvCpssDrvPpObjConfig[devNum] = cpssOsMalloc(sizeof(PRV_CPSS_DRV_OBJ_STC));
    if(prvCpssDrvPpObjConfig[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

#ifdef INCL_EXDXMX_DRIVER
    /* Dx, Ex, Mx driver init */
    rc = prvCpssDrvDxExMxInitObject(devNum);

    if(rc != GT_OK)
        return rc;
#endif /* INCL_EXDXMX_DRIVER */

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    /* Call appropriate implementation by virtual function */
    rc = prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpCfgPhase1Init(devNum,
                    ppInInfoPtr,ppOutInfoPtr);

    return rc;
}


/**
* @internal prvCpssDrvHwPpPhase2Init function
* @endinternal
*
* @brief   the function set parameters for the driver for the "init Phase 2".
*         the function "renumber" the current device number of the device to a
*         new device number.
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] devNum                   - The PP's current device number .
* @param[in] newDevNum                - The PP's "new" device number to register.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssDrvHwPpPhase2Init
(
    IN GT_U8    devNum,
    IN GT_U8    newDevNum
)
{
    if(devNum == newDevNum)
    {
        /* nothing more to do */
        return GT_OK;
    }

    if(prvCpssDrvPpObjConfig[newDevNum] || prvCpssDrvPpConfig[newDevNum])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* swap the function objects */
    prvCpssDrvPpObjConfig[newDevNum] = prvCpssDrvPpObjConfig[devNum];
    prvCpssDrvPpObjConfig[devNum] = NULL;

    /* swap the info DB */
    prvCpssDrvPpConfig[newDevNum] = prvCpssDrvPpConfig[devNum];
    prvCpssDrvPpConfig[devNum] = NULL;

    /* update the devNum */
    prvCpssDrvPpConfig[newDevNum]->hwDevNum = newDevNum;

    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpDevRemove function
* @endinternal
*
* @brief   Synchronize the driver/Config library after a hot removal operation, of
*         the given device.
* @param[in] devNum                   - device Number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvHwPpDevRemove
(
    IN GT_U8    devNum
)
{
    GT_STATUS           rc;
    GT_U32  ii;
    GT_U32  portGroupId;/* port group Id */

    if(prvCpssDrvPpConfig[devNum] == NULL)
    {
        /* Unknown device  */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Remove the device from the interrupts queues */
    rc = prvCpssDrvInterruptDeviceRemove(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr)
    {
        for(ii = 0 ; ii < CPSS_EVENT_GPP_MAX_NUM_E; ii++)
        {
            FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr[ii]);
        }

        FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.gppIsrFuncsArr);
    }


    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        prvCpssDrvInterruptScanFree(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot);

        if(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].hwComplSem)
        {
            cpssOsMutexDelete(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].hwComplSem);
        }

        FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfo);

        FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool);
        FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow);

        /* Delete Pp drivers */
        cpssHwDriverDestroy(prvCpssDrvPpConfig[devNum]->drivers[portGroupId]);
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]->intCtrl.accessB4StartInit);

    /* clear the memory */
    FREE_PTR_MAC(prvCpssDrvPpConfig[devNum]);
    FREE_PTR_MAC(prvCpssDrvPpObjConfig[devNum]);

    /* check if there are still existing devices */
    for(ii = 0 ; ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++)
    {
        if(prvCpssDrvPpConfig[ii])
        {
            break;
        }
    }

    if(ii == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* no more existing devices */


        /* release the management interface bus pointers */
        FREE_PTR_MAC(prvCpssDrvMngInfPciPtr);
        FREE_PTR_MAC(prvCpssDrvMngInfPexMbusPtr);
        FREE_PTR_MAC(prvCpssDrvMngInfPciHaStandbyPtr);
        FREE_PTR_MAC(prvCpssDrvMngInfSmiPtr);
        FREE_PTR_MAC(prvCpssDrvMngInfTwsiPtr);

        /* release the DB of interrupt ISR (lines) manager */
        rc = prvCpssDrvComIntSvcRtnDbRelease();
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set that we are done releaseing the DB */
        drvGlobalInfo.prvCpssDrvAfterDbRelease = GT_TRUE;
    }



    return GT_OK;
}



/**
* @internal internal_cpssDrvInterruptsTreeGet function
* @endinternal
*
* @brief   function return :
*         1. the root to the interrupts tree info of the specific device
*         2. the interrupt registers that can't be accesses before 'Start Init'
*
* @note   APPLICABLE DEVICES:      All devices --> Packet Processors (not Fa/Xbar)
* @param[in] devNum                   - the device number
*
* @param[out] numOfElementsPtr         - (pointer to) number of elements in the tree.
* @param[out] treeRootPtrPtr           - (pointer to) pointer to root of the interrupts tree info.
* @param[out] numOfInterruptRegistersNotAccessibleBeforeStartInitPtr - (pointer to)
*                                      number of interrupt registers that can't be accessed
*                                      before 'Start init'
* @param[out] notAccessibleBeforeStartInitPtrPtr (pointer to)pointer to the interrupt
*                                      registers that can't be accessed before 'Start init'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
static GT_STATUS internal_cpssDrvInterruptsTreeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
)
{
    GT_STATUS   rc;
    GT_U32  portGroupId = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;/*tempo internal parameter and not getting it from application*/

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum,portGroupId);

    CPSS_NULL_PTR_CHECK_MAC(numOfElementsPtr);
    CPSS_NULL_PTR_CHECK_MAC(treeRootPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterruptRegistersNotAccessibleBeforeStartInitPtr);
    CPSS_NULL_PTR_CHECK_MAC(notAccessibleBeforeStartInitPtrPtr);

    /* Call appropriate implementation by virtual function */
    rc = prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpInterruptsTreeGet(devNum,portGroupId,numOfElementsPtr,treeRootPtrPtr,
        numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,notAccessibleBeforeStartInitPtrPtr);

    return rc;
}

/**
* @internal cpssDrvInterruptsTreeGet function
* @endinternal
*
* @brief   function return :
*         1. the root to the interrupts tree info of the specific device
*         2. the interrupt registers that can't be accesses before 'Start Init'
*
* @note   APPLICABLE DEVICES:      All devices --> Packet Processors (not Fa/Xbar)
* @param[in] devNum                   - the device number
*
* @param[out] numOfElementsPtr         - (pointer to) number of elements in the tree.
* @param[out] treeRootPtrPtr           - (pointer to) pointer to root of the interrupts tree info.
* @param[out] numOfInterruptRegistersNotAccessibleBeforeStartInitPtr - (pointer to)
*                                      number of interrupt registers that can't be accessed
*                                      before 'Start init'
* @param[out] notAccessibleBeforeStartInitPtrPtr (pointer to)pointer to the interrupt
*                                      registers that can't be accessed before 'Start init'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS   cpssDrvInterruptsTreeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvInterruptsTreeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfElementsPtr, treeRootPtrPtr, numOfInterruptRegistersNotAccessibleBeforeStartInitPtr, notAccessibleBeforeStartInitPtrPtr));

    rc = internal_cpssDrvInterruptsTreeGet(devNum, numOfElementsPtr, treeRootPtrPtr, numOfInterruptRegistersNotAccessibleBeforeStartInitPtr, notAccessibleBeforeStartInitPtrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfElementsPtr, treeRootPtrPtr, numOfInterruptRegistersNotAccessibleBeforeStartInitPtr, notAccessibleBeforeStartInitPtrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDrvHwPpRenumber function
* @endinternal
*
* @brief   the function set parameters for the driver to renumber it's DB.
*         the function "renumber" the current device number of the device to a
*         new device number.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] oldDevNum                - The PP's "old" device number .
* @param[in] newDevNum                - The PP's "new" device number swap the DB to.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssDrvHwPpRenumber
(
    IN GT_U8    oldDevNum,
    IN GT_U8    newDevNum
)
{
    if(oldDevNum == newDevNum)
    {
        /* nothing more to do */
        return GT_OK;
    }

    if(prvCpssDrvPpObjConfig[newDevNum] || prvCpssDrvPpConfig[newDevNum])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* swap the function objects */
    prvCpssDrvPpObjConfig[newDevNum] = prvCpssDrvPpObjConfig[oldDevNum];
    prvCpssDrvPpObjConfig[oldDevNum] = NULL;

    /* swap the info DB */
    prvCpssDrvPpConfig[newDevNum] = prvCpssDrvPpConfig[oldDevNum];
    prvCpssDrvPpConfig[oldDevNum] = NULL;

    /* swap debug Device ID DB */
    prvCpssDrvDebugDeviceId[newDevNum] = prvCpssDrvDebugDeviceId[oldDevNum];
    prvCpssDrvDebugDeviceId[oldDevNum] = GT_FALSE;

    /* swap tracing hw write DB */
    prvCpssDrvTraceHwWrite[newDevNum] = prvCpssDrvTraceHwWrite[oldDevNum];
    prvCpssDrvTraceHwWrite[oldDevNum] = GT_FALSE;

    /* swap tracing hw read DB */
    prvCpssDrvTraceHwRead[newDevNum] = prvCpssDrvTraceHwRead[oldDevNum];
    prvCpssDrvTraceHwRead[oldDevNum] = GT_FALSE;

       /* swap tracing hw delay DB */
    prvCpssDrvTraceHwDelay[newDevNum] = prvCpssDrvTraceHwDelay[oldDevNum];
    prvCpssDrvTraceHwDelay[oldDevNum] = GT_FALSE;

    /* update the devNum */
    prvCpssDrvPpConfig[newDevNum]->hwDevNum = newDevNum;

    /*call the interrupts to renumber */
    return prvCpssDrvInterruptsRenumber(oldDevNum,newDevNum);
}


/**
* @internal prvCpssDrvDebugDeviceIdSet function
* @endinternal
*
* @brief   This is debug function.
*         The function overrides device ID by given value.
*         The function should be called before cpssHwPpPhase1Init().
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - device number .
* @param[in] devType                  - device type to store.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvDebugDeviceIdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvDebugDeviceId[devNum] = (GT_U16)(devType >> 16);

    return GT_OK;
}

/**
* @internal prvCpssDrvAddrCheckWaBind function
* @endinternal
*
* @brief   Binds errata db with address check callback
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - the device number
* @param[in] addrCheckFuncPtr         - pointer to callback function that checks addresses.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS prvCpssDrvAddrCheckWaBind
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DRV_ERRATA_ADDR_CHECK_FUNC     addrCheckFuncPtr
)
{
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr = addrCheckFuncPtr;

    return GT_OK;
}

/**
* @internal prvCpssDrvErrataCallbackBind function
* @endinternal
*
* @brief   Binds errata callback to prepare and complete special read/write operations
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] devNum                   - the device number
* @param[in] callbackFuncPtr          - pointer to prepare/complete special read/write operations function
* @param[in] callbackDataPtr          - pointer to prepare/complete special read/write operations anchor data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS prvCpssDrvErrataCallbackBind
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DRV_ERRATA_CALLBACK_FUNC_PTR   callbackFuncPtr,
    IN  GT_VOID                                 *callbackDataPtr
)
{
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr = callbackFuncPtr;
    prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr = callbackDataPtr;

    return GT_OK;
}

/**
* @internal prvCpssDrvPortsFullMaskGet function
* @endinternal
*
* @brief   get bmp of all phy ports of the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] numOfPorts               - number of physical ports in the device
*
* @param[out] portsBmpPtr              - (pointer to) bmp of ports
*                                       None
*/
void prvCpssDrvPortsFullMaskGet
(
    IN GT_U32                   numOfPorts,
    OUT CPSS_PORTS_BMP_STC       *portsBmpPtr
)
{
    GT_U32  ii;

    if(numOfPorts)
    {
        for(ii = 0; ii < CPSS_MAX_PORTS_BMP_NUM_CNS; ii++)
        {
            if((GT_U32)(numOfPorts - 1) < (GT_U32)((ii+1)*32))
            {
                if(numOfPorts & 0x1f)
                {
                    (portsBmpPtr)->ports[ii] =
                        BIT_MASK_MAC((numOfPorts & 0x1f));
                }
                else
                {
                    (portsBmpPtr)->ports[ii] = 0xFFFFFFFF;
                }

                ii++;
                break;
            }

            (portsBmpPtr)->ports[ii] = 0xFFFFFFFF;
        }
    }
    else /* numOfPorts = 0 ... should not happen ...but lets implement */
    {
        ii = 0;/* to allow to reset the full bmp */
    }

    for(/* continue _ii */; ii < CPSS_MAX_PORTS_BMP_NUM_CNS; ii++)
    {
        (portsBmpPtr)->ports[ii] = 0;
    }
}

/**
* @internal prvCpssDrvPortsBmpMaskWithMaxPorts function
* @endinternal
*
* @brief   mask bmp ports with the max number of ports of the of the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] portsBmpPtr              - (pointer to) bmp of ports
* @param[in,out] portsBmpPtr              - (pointer to) bmp of ports , after the mask
*                                       None
*/
void prvCpssDrvPortsBmpMaskWithMaxPorts
(
    IN GT_U8                        devNum,
    INOUT CPSS_PORTS_BMP_STC       *portsBmpPtr
)
{
    CPSS_PORTS_BMP_STC tmpPortsBmp;

    prvCpssDrvPortsFullMaskGet(PRV_CPSS_PP_MAC(devNum)->numOfVirtPorts,&tmpPortsBmp);

    CPSS_PORTS_BMP_BITWISE_AND_MAC(portsBmpPtr,portsBmpPtr,&tmpPortsBmp);

    return;
}

/**
* @internal prvCpssDrvEventSupportCheck function
* @endinternal
*
* @brief   Check if the device supports the unified event (with evExtData)
*
* @param[in] devNum                   - The device number.
* @param[in] uniEvent                 - The unified event to be unmasked.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
*
* @retval GT_OK                    - the device supports the unified event
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_NOT_INITIALIZED       - if the device was not initialized yet
*/
GT_STATUS prvCpssDrvEventSupportCheck
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRV_EV_REQ_NODE_STC  *nodePoolArr;   /* The event node array   */
    GT_U32  event;          /* The event index              */
    GT_U32  portGroupId = 0;
    GT_U32  intNodesPoolSize;

    /* check that the device exists and properly initialized */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    nodePoolArr = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;
    intNodesPoolSize = prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits;

    if (prvCpssEventExtDataConvertFunc)
    {
        /* invoke the callback function to convert event's extended data */
        rc = (prvCpssEventExtDataConvertFunc)(devNum, uniEvent,
                                              PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
                                              &evExtData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (event = 0; event < intNodesPoolSize; event++)
    {
        if (nodePoolArr[event].uniEvCause != (GT_U32)uniEvent)
        {
            continue;
        }

        if(evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
        {
            if(nodePoolArr[event].uniEvExt != evExtData)
            {
                /* we need to mask/unmask only the specific interrupts and not all
                   interrupts that connected to the unified event */
                continue;
            }
        }

        /* we found at least one interrupt associated with the event and extra info */
        return GT_OK;
    }

    /***************************************************************************/
    /* this is special 'Check' function that should not generate error LOG !!! */
    /***************************************************************************/

    return /* not error for the LOG */ GT_NOT_FOUND;
}

/**
* @internal prvCpssDrvPpDump function
* @endinternal
*
* @brief   Dump function , to print the info on a specific PP -- CPSS driver.
*
* @note   APPLICABLE DEVICES:      ALL devices
* @param[in] devNum                   - device Number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
*
* @note function also called from the "cpss pp dump" : cpssPpDump(devNum)
*
*/
GT_STATUS prvCpssDrvPpDump
(
    IN GT_U8   devNum
)
{
    PRV_CPSS_DRIVER_PP_CONFIG_STC   *drvPpInfoPtr;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       prvCpssDrvPpConfig[devNum] == NULL)
    {
        DEBUG_PRINT(("bad device number [%d] \n",devNum));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    drvPpInfoPtr = prvCpssDrvPpConfig[devNum];

    DEBUG_PRINT(("\n"));
    DEBUG_PRINT(("start CPSS DRIVER Info about device number [%d]: \n",devNum));

    DEBUG_PRINT((" devNum = [%d] \n",drvPpInfoPtr->hwDevNum));

    DEBUG_PRINT((" devType = [0x%8.8x] \n",drvPpInfoPtr->devType));
    DEBUG_PRINT((" devFamily = [%s] \n",
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH_E   ? " CPSS_PP_FAMILY_CHEETAH_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH2_E  ? " CPSS_PP_FAMILY_CHEETAH2_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_CHEETAH3_E  ? " CPSS_PP_FAMILY_CHEETAH3_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E  ? " CPSS_PP_FAMILY_DXCH_XCAT_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E  ? " CPSS_PP_FAMILY_DXCH_XCAT3_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION_E  ? " CPSS_PP_FAMILY_DXCH_LION_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E  ? " CPSS_PP_FAMILY_DXCH_LION2_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E  ? " CPSS_PP_FAMILY_DXCH_BOBCAT2_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E  ? " CPSS_PP_FAMILY_DXCH_BOBCAT3_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E  ? " CPSS_PP_FAMILY_DXCH_ALDRIN_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E  ? " CPSS_PP_FAMILY_DXCH_AC3X_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E  ? " CPSS_PP_FAMILY_DXCH_ALDRIN2_E ":
        drvPpInfoPtr->devFamily == CPSS_PX_FAMILY_PIPE_E  ? " CPSS_PX_FAMILY_PIPE_E ":
        drvPpInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E  ? " CPSS_PP_FAMILY_DXCH_FALCON_E ":
        " unknown "
    ));
    DEBUG_PRINT((" numOfPorts = [%d] \n",drvPpInfoPtr->numOfPorts));

    if(drvPpInfoPtr->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE)
    {
        DEBUG_PRINT(("numOfPortGroups  = [%d] \n",
                     drvPpInfoPtr->portGroupsInfo.numOfPortGroups));
        DEBUG_PRINT(("activePortGroupsBmp   = [%d] \n",
                     drvPpInfoPtr->portGroupsInfo.activePortGroupsBmp));
        DEBUG_PRINT(("firstActivePortGroup  = [%d] \n",
                     drvPpInfoPtr->portGroupsInfo.firstActivePortGroup));
        DEBUG_PRINT(("lastActivePortGroup   = [%d] \n",
                     drvPpInfoPtr->portGroupsInfo.lastActivePortGroup));
    }
    else
    {
        if(drvPpInfoPtr->portGroupsInfo.numOfPortGroups != 1 ||
           drvPpInfoPtr->portGroupsInfo.activePortGroupsBmp != BIT_0 ||
           drvPpInfoPtr->portGroupsInfo.firstActivePortGroup != 0 ||
           drvPpInfoPtr->portGroupsInfo.lastActivePortGroup != 0)
        {
            DEBUG_PRINT(("Error : non 'multi port grops' device with bad configuration : \n"));
            DEBUG_PRINT(("numOfPortGroups --  = [%d] \n",
                         drvPpInfoPtr->portGroupsInfo.numOfPortGroups));
            DEBUG_PRINT(("activePortGroupsBmp --   = [%d] \n",
                         drvPpInfoPtr->portGroupsInfo.activePortGroupsBmp));
            DEBUG_PRINT(("firstActivePortGroup --  = [%d] \n",
                         drvPpInfoPtr->portGroupsInfo.firstActivePortGroup));
            DEBUG_PRINT(("lastActivePortGroup --   = [%d] \n",
                         drvPpInfoPtr->portGroupsInfo.lastActivePortGroup));
        }
    }

    DEBUG_PRINT((" drvChannel = [%s] \n",
        PRV_CPSS_DRV_HW_IF_PEX_COMPATIBLE_MAC(devNum) ? " CPSS_CHANNEL_PEX_E " :
        PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum)  ? " CPSS_CHANNEL_PCI_E " :
        PRV_CPSS_DRV_HW_IF_SMI_COMPATIBLE_MAC(devNum)  ? " CPSS_CHANNEL_SMI_E " :
        PRV_CPSS_DRV_HW_IF_TWSI_COMPATIBLE_MAC(devNum) ? " CPSS_CHANNEL_TWSI_E " :
        " unknown "
    ));

    DEBUG_PRINT(("finished CPSS DRIVER Info about device number [%d]: \n",devNum));
    DEBUG_PRINT(("\n"));

    return GT_OK;
}


