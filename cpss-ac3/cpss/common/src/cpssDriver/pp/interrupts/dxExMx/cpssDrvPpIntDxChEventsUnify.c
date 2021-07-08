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
* @file cpssDrvPpIntDxChEventsUnify.c
*
* @brief Conversion routines for unified events -- DxCh devices.
*
* @version   33
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
/* get the unify events definitions */
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

/* TEMP */
#include <cpss/generic/events/private/prvCpssGenEvReq.h>

/*******************************************************************************
* internal definitions
*******************************************************************************/
/* #define CH_UNI_CNVRT_DEBUG */
 
#ifdef CH_UNI_CNVRT_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

/**
* @internal muliPortGroupIndicationCheckAndConvert function
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
static void muliPortGroupIndicationCheckAndConvert(
    IN      GT_U32                    portGroupId,
    IN      GT_U32                    uniEv,
    INOUT   GT_U32                    *extDataPtr
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
        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            break;
        default:
            return;
    }

    /* convert for those events */
    *extDataPtr += CPSS_PP_PORT_GROUP_ID_TO_EXT_DATA_CONVERT_MAC(portGroupId);

    return;
}


/**
* @internal chIntCauseToUniEvConvertDedicatedTables function
* @endinternal
*
* @brief   Converts Interrupt Cause event to unified event type. - For devices
*         with dedicated tables (Lion2).
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id.
* @param[in] intCauseIndex            - The interrupt cause to convert.
*
* @param[out] uniEvPtr                 - (pointer to)The unified event type.
* @param[out] extDataPtr               - (pointer to)The event extended data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS chIntCauseToUniEvConvertDedicatedTables
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
)
{
    GT_U32   ii; /* iterator                     */
    GT_U32   *tableWithExtDataPtr;
    GT_U32   tableWithoutExtDataSize,tableWithExtDataSize;
    PRV_CPSS_DRV_EVENT_INFO_TYPE   *tableWithoutExtDataPtr;
    GT_U32   tmpUniEvent;
    GT_U32   tmpExtData;
    GT_U32   numReservedPorts;

    CPSS_NULL_PTR_CHECK_MAC(uniEvPtr);
    CPSS_NULL_PTR_CHECK_MAC(extDataPtr);

    *uniEvPtr   = CPSS_UNI_RSRVD_EVENT_E;
    *extDataPtr = 0;
    /* get the tables */
    tableWithExtDataPtr     = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr;
    tableWithExtDataSize    = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize;
    tableWithoutExtDataPtr  = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr;
    tableWithoutExtDataSize = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize;
    numReservedPorts        = prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts;

    if(tableWithoutExtDataPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* Search the map table for unified event without extended data */
    for (ii=0; ii < tableWithoutExtDataSize;ii++)
    {
        if (tableWithoutExtDataPtr[ii][1] == intCauseIndex)
        {
            /* found */
            *uniEvPtr = tableWithoutExtDataPtr[ii][0];
            muliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);
            if(CPSS_PP_PORT_802_3_AP_E == *uniEvPtr)
            {
                *uniEvPtr += portGroupId;
            }
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
                if ((tmpUniEvent == CPSS_PP_DATA_INTEGRITY_ERROR_E) || (tmpUniEvent == CPSS_PP_CRITICAL_HW_ERROR_E))
                {
                    /* These unified events use interrupt enum as extData.
                       There is no conversion is used. */
                    *extDataPtr = tmpExtData;
                }
                else if(tmpUniEvent == CPSS_PP_PORT_EEE_E)/* patch because 'port indication' not supports '<< 8' */
                {
                    GT_U32  portNum = U32_GET_FIELD_MAC(tmpExtData,8,8);/* from bit 8 take 8 bits */
                    GT_U32  extVal  = U32_GET_FIELD_MAC(tmpExtData,0,8);/* from bit 0 take 8 bits */

                    /* convert the port to 'global port' */
                    /* each port group has it's reserved ports */
                    portNum += (numReservedPorts * portGroupId);

                    /* rebuild the '*extDataPtr' */
                    *extDataPtr = extVal | (portNum << 8);
                }
                else
                {
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
                }

                muliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);
                return GT_OK;
            }
            ii +=2;
        }
        ii++;
    }

    DBG_INFO(("file:%s line:%d event not found intCause = %d\n", __FILE__,
              __LINE__, intCauseIndex));

    return /* do not register as error to the LOG */GT_NOT_FOUND;
}

/**
* @internal prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert function
* @endinternal
*
* @brief   Converts Interrupt Cause event to unified event type. -- cheetah devices
*         --> per Port Group (to support multi-port-groups device)
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id.
* @param[in] intCauseIndex            - The interrupt cause to convert.
*
* @param[out] uniEvPtr                 - (pointer to)The unified event type.
* @param[out] extDataPtr               - (pointer to)The event extended data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
)
{
    if(prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr)
    {
        /* Lion2 uses dedicated explicit tables, i.e. ppUni2IntCauseType is not used */
        return chIntCauseToUniEvConvertDedicatedTables(devNum,
                portGroupId,intCauseIndex,uniEvPtr,extDataPtr);
    }
    else  
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

