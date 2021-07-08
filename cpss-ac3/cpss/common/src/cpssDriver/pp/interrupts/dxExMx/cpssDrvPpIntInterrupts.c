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
* @file cpssDrvPpIntInterrupts.c
*
* @brief This file includes initialization function for the interrupts module,
* and low level interrupt handling (interrupt bits scanning).
*
* @version   6
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>

/*******************************************************************************
* internal definitions
*******************************************************************************/

#ifdef DRV_INTERRUPTS_DBG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

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
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);
/*******************************************************************************
 * External usage variables
 ******************************************************************************/

/**
* @internal prvCpssDrvInterruptMaskSet function
* @endinternal
*
* @brief   This function masks/unmasks a given interrupt bit in the relevant
*         interrupt mask register.
* @param[in] evNode                   - The interrupt node representing the interrupt to be
*                                      unmasked.
* @param[in] operation                - The  to perform, mask or unmask the interrupt
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDrvInterruptMaskSet
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC              *evNode,
    IN CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_U32  maskIdx;
    GT_U32  *intMaskShadow;
    GT_U8   devNum = evNode->devNum;  /* devNum */
    GT_U32  portGroupId = evNode->portGroupId;  /* portGroupId */

    /* check devNum and portGroupId */
    if ((prvCpssDrvPpConfig[devNum] == NULL) || (portGroupId >= CPSS_MAX_PORT_GROUPS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    intMaskShadow = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow;

    maskIdx = evNode->intCause >> 5;

    if (CPSS_EVENT_MASK_E == operation)
    {
        /* mask the interrupt */
        intMaskShadow[maskIdx] &= ~(evNode->intBitMask);
    }
    else
    {
        /* unmask the interrupt */
        intMaskShadow[maskIdx] |= (evNode->intBitMask);
    }

    if ((evNode->uniEvCause >= CPSS_SRVCPU_IPC_E) &&
        (evNode->uniEvCause <= CPSS_SRVCPU_MAX_E))
    {
        return prvCpssGenericSrvCpuRegisterWrite(
                    devNum,
                    portGroupId,
                    evNode->intMaskReg,
                    intMaskShadow[maskIdx]);
    }
    if(PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum,evNode->intMaskReg))
    {
        return prvCpssDrvHwPpPortGroupWriteInternalPciReg(
                    devNum,
                    portGroupId,
                    PRV_CPSS_REG_MASK_MAC(evNode->intMaskReg),
                    intMaskShadow[maskIdx]);

    }
    else if (PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(evNode->intMaskReg))
    {
        return prvCpssDrvHwPpMgWriteReg(devNum,
                                        PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(evNode->intMaskReg),
                                        PRV_CPSS_MGS_REG_MASK_MAC(evNode->intMaskReg),
                                        intMaskShadow[maskIdx]);
    }
    else
    {
        return prvCpssDrvHwPpPortGroupWriteRegister(
                    devNum,
                    portGroupId,
                    evNode->intMaskReg,
                    intMaskShadow[maskIdx]);
    }
}

/**
* @internal prvCpssDrvInterruptMaskGet function
* @endinternal
*
* @brief   This function gets a given interrupt bit in the relevant
*         interrupt mask register.
* @param[in] evNodePtr                - The interrupt node representing the interrupt to be
*                                      unmasked.
*
* @param[out] maskedPtr                - The given interrupt bit in the relevant interrupt mask register.
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDrvInterruptMaskGet
(
    IN  PRV_CPSS_DRV_EV_REQ_NODE_STC              *evNodePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                   *maskedPtr
)
{
    GT_U8     devNum = evNodePtr->devNum;  /* devNum */
    GT_U32    portGroupId = evNodePtr->portGroupId;  /* portGroupId */
    GT_STATUS rc;
    GT_U32    value;

    /* check devNum and portGroupId */
    if ((prvCpssDrvPpConfig[devNum] == NULL) || (portGroupId >= CPSS_MAX_PORT_GROUPS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((evNodePtr->uniEvCause >= CPSS_SRVCPU_IPC_E) &&
        (evNodePtr->uniEvCause <= CPSS_SRVCPU_MAX_E))
    {
        rc = prvCpssGenericSrvCpuRegisterRead(
                    devNum,
                    portGroupId,
                    evNodePtr->intMaskReg,
                    &value);
    }
    if(PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum,evNodePtr->intMaskReg))
    {
        rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(
                    devNum,
                    portGroupId,
                    PRV_CPSS_REG_MASK_MAC(evNodePtr->intMaskReg),
                    &value);
    }
    else if (PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(evNodePtr->intMaskReg))
    {
            rc = prvCpssDrvHwPpMgReadReg(devNum,
                                          PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(evNodePtr->intMaskReg),
                                          PRV_CPSS_MGS_REG_MASK_MAC(evNodePtr->intMaskReg),
                                          &value);
    }
    else
    {
        rc = prvCpssDrvHwPpPortGroupReadRegister(
                    devNum,
                    portGroupId,
                    evNodePtr->intMaskReg,
                    &value);
    }
    if(GT_OK != rc)
    {
        return rc;
    }
    if((value & evNodePtr->intBitMask) > 0)
    {
        *maskedPtr = CPSS_EVENT_UNMASK_E;
    }
    else
    {
        *maskedPtr = CPSS_EVENT_MASK_E;
    }
    return GT_OK;
}

/**
* @internal prvCpssMuliPortGroupIndicationCheckAndConvert function
* @endinternal
*
* @brief   for multi-port Group device
*         check if current uni-event need Convert due to multi-port group indication
* @param[in] portGroupId              - The port group Id.
* @param[in] uniEv                    - unified event
* @param[in,out] extDataPtr               - (pointer to)The event extended data.
* @param[in,out] extDataPtr               - (pointer to)The event extended data.
*                                       none
*/
static void prvCpssMuliPortGroupIndicationCheckAndConvert(
    IN  GT_U32                      portGroupId,
    OUT GT_U32                      uniEv,
    INOUT GT_U32                    *extDataPtr
)
{
    switch(uniEv)
    {
        /* GPP */
        case CPSS_PP_GPP_E:
        /*CNC*/
        case CPSS_PP_CNC_WRAPAROUND_BLOCK_E:
        case CPSS_PP_CNC_DUMP_FINISHED_E:
        /*Policer*/
        case CPSS_PP_POLICER_DATA_ERR_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        /* AP auto-negotiation interrupt */
        case CPSS_PP_PORT_802_3_AP_E:
            break;
        default:
            return;
    }

    /* convert for those events */
    *extDataPtr += CPSS_PP_PORT_GROUP_ID_TO_EXT_DATA_CONVERT_MAC(portGroupId);

    return;
}

/**
* @internal prvCpssDrvIntCauseToUniEvConvertDedicatedTables function
* @endinternal
*
* @brief   Converts Interrupt Cause event to unified event type. - For devices
*         with dedicated tables (Lion2, Puma3).
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDrvIntCauseToUniEvConvertDedicatedTables
(
    IN  GT_U32                      (*tableWithoutExtDataPtr)[2],
    IN  GT_U32                      tableWithoutExtDataSize,
    IN  GT_U32                      *tableWithExtDataPtr,
    IN  GT_U32                      tableWithExtDataSize,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
)
{
    GT_U32   ii; /* iterator                     */
    GT_U32   tmpUniEvent;
    GT_U32   tmpExtData;
    GT_U32   numReservedPorts;


    CPSS_NULL_PTR_CHECK_MAC(uniEvPtr);
    CPSS_NULL_PTR_CHECK_MAC(extDataPtr);

    *uniEvPtr   = CPSS_UNI_RSRVD_EVENT_E;
    *extDataPtr = 0;
    numReservedPorts = 16;

    /* Search the map table for unified event without extended data */
    for (ii=0; ii < tableWithoutExtDataSize;ii++)
    {
        if (tableWithoutExtDataPtr[ii][1] == intCauseIndex)
        {
            /* found */
            *uniEvPtr = tableWithoutExtDataPtr[ii][0];
            prvCpssMuliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);
            return GT_OK;
        }
    }

    ii = 0;
    /* Search the map table for unified event with extended data */
    while (ii < tableWithExtDataSize)
    {
        /* remember the uni event */
        tmpUniEvent = tableWithExtDataPtr[ii++];
        while (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS)
        {
            if (tableWithExtDataPtr[ii] ==  intCauseIndex)
            {
                /* found */
                *uniEvPtr = tmpUniEvent;
                tmpExtData = tableWithExtDataPtr[ii+1];
                *extDataPtr = CLEAR_MARK_INT_MAC(tmpExtData);
                if (IS_MARKED_PER_PORT_PER_LANE_INT_MAC(tmpExtData))
                {
                    /* each port group has 16 reserved ports */
                    /* each port has 256 reserved lanes */
                    *extDataPtr +=
                        (LANE_PORT_TO_EXT_DATA_CONVERT(numReservedPorts,0) * portGroupId);
                }
                else if (IS_MARKED_PER_PORT_INT_MAC(tmpExtData))
                {
                   /* each port group has 16 reserved ports */
                    *extDataPtr += (numReservedPorts * portGroupId);
                }
                else if (IS_MARKED_PER_HEM_INT_MAC(tmpExtData))
                {
                   /* each hemisphere group has 64 reserved ports */
                    *extDataPtr += (portGroupId >= 4) ? 64 : 0;
                }

                prvCpssMuliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);
                return GT_OK;
            }
            ii +=2;
        }
        ii++;
    }

    DBG_INFO(("file:%s line:%d event not found intCause = %d\n", __FILE__,
              __LINE__, intCauseIndex));

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDrvIntUniEvToCauseConvertDedicatedTables function
* @endinternal
*
* @brief   Converts Interrupt unified event type to Cause event. - For devices
*         with dedicated tables (Lion2, Puma3).
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDrvIntUniEvToCauseConvertDedicatedTables
(
    IN  GT_U32                      (*tableWithoutExtDataPtr)[2],
    IN  GT_U32                      tableWithoutExtDataSize,
    IN  GT_U32                      *tableWithExtDataPtr,
    IN  GT_U32                      tableWithExtDataSize,
    IN  GT_U32                      portGroupsAmount,
    IN  GT_U32                      uniEv,
    IN  GT_U32                      extData,
    OUT GT_U32                      *intCauseIndexPtr
)
{
    GT_U32   ii; /* iterator                     */
    GT_U32   tmpUniEvent;
    GT_U32   tmpExtData;
    GT_U32   calcExtData;
    GT_U32   numReservedPorts;
    GT_U32   portGroupId;
    GT_U32   firstIntCauseIndex;
    GT_U32   currentIntCauseIndex;
    GT_BOOL  uniEventFound;

    CPSS_NULL_PTR_CHECK_MAC(intCauseIndexPtr);

    /* initial values */
    *intCauseIndexPtr   = CPSS_UNI_RSRVD_EVENT_E;
    numReservedPorts = 16;
    uniEventFound = GT_FALSE;

    /* to avoid compiler warning */
    firstIntCauseIndex = 0;

    /* Search the map table for unified event without extended data */
    for (ii=0; ii < tableWithoutExtDataSize;ii++)
    {
        if (tableWithoutExtDataPtr[ii][0] == uniEv)
        {
            /* found */
            *intCauseIndexPtr = tableWithoutExtDataPtr[ii][1];
            return GT_OK;
        }
    }

    ii = 0;
    /* Search the map table for unified event with extended data */
    while (ii < tableWithExtDataSize)
    {
        /* remember the uni event */
        tmpUniEvent = tableWithExtDataPtr[ii];
        if (tmpUniEvent != uniEv)
        {
            /* skip the Uni Event ID */
            ii ++;
            /* skip the Uni Event related data */
            for (; (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS); ii += 2){};
            /* skip the mark */
            ii ++;
            continue;
        }

        /* Unified ID found in DB */

        /* skip the Uni Event ID */
        ii ++;

        uniEventFound = GT_TRUE;
        firstIntCauseIndex = tableWithExtDataPtr[ii];

        while (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS)
        {
            calcExtData = 0;
            currentIntCauseIndex = tableWithExtDataPtr[ii];
            tmpExtData = tableWithExtDataPtr[ii+1];

            for (portGroupId = 0; (portGroupId < portGroupsAmount); portGroupId ++)
            {
                if (IS_MARKED_PER_PORT_PER_LANE_INT_MAC(tmpExtData))
                {
                    /* each port group has 16 reserved ports */
                    /* each port has 256 reserved lanes */
                    calcExtData +=
                        (LANE_PORT_TO_EXT_DATA_CONVERT(numReservedPorts,0) * portGroupId);
                }
                else if (IS_MARKED_PER_PORT_INT_MAC(tmpExtData))
                {
                   /* each port group has 16 reserved ports */
                    calcExtData += (numReservedPorts * portGroupId);
                }
                else if (IS_MARKED_PER_HEM_INT_MAC(tmpExtData))
                {
                   /* each hemisphere group has 64 reserved ports */
                    calcExtData += (portGroupId >= 4) ? 64 : 0;
                }

                if (extData == calcExtData)
                {
                    *intCauseIndexPtr = currentIntCauseIndex;
                    return GT_OK;
                }
            }
            /* next pair of intCause and extData */
            ii +=2;
        }
        /* skip the mark */
        ii++;
    }

    if (uniEventFound != GT_FALSE)
    {
        *intCauseIndexPtr = firstIntCauseIndex;
        return GT_OK;
    }

    DBG_INFO(("file:%s line:%d event not found intCause = %d\n", __FILE__,
              __LINE__, intCauseIndex));

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
