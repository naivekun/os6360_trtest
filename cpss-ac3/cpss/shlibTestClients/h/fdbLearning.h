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
* @file fdbLearning.h
*
* @brief This file contains function declarations, typedefs and defines for fdbLearning.c
*
*
*
* @version   6
********************************************************************************
*/
#ifndef __fdbLearningh
#define __fdbLearningh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*************            defines            *******************************/
#define EV_HANDLER_MAX_PRIO     200
#define _PRV_CPSS_MAX_PP_DEVICES_CNS 1

#ifdef FDB_PROCESS_DEBUG
	static char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
	#define DBG_LOG(x)  osPrintf x
	#define DBG_INFO(x)   osPrintf 
#else
	#define DBG_LOG(x)  
	#define DBG_INFO(x)  
#endif



/*************           typedefs          *******************************/
typedef struct
{
    GT_UINTPTR          evHndl;
    GT_U32              hndlrIndex;
} EV_HNDLR_PARAM;

/**************** static func declaration *****************************/
/**
* @internal fdbLearningEventTreat function
* @endinternal
*
* @brief   This routine handles FDB learning events
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS fdbLearningEventTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
);

/**
* @internal fdbLearningAuMsgGet function
* @endinternal
*
* @brief   This routine handles Address Update messages.
*
* @param[in] devNum                   - the device number.
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS fdbLearningAuMsgGet
(
    IN GT_U8                devNum,
    GT_U32                  evExtData
);

/**
* @internal fdbLearningAuMsgHandle function
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
static GT_STATUS fdbLearningAuMsgHandle
(
    IN GT_U8                devNum,
    GT_U32                  evExtData
);

/*******************************************************************************
* fdbLearningEventsHndlr
*
* DESCRIPTION:
*       This routine is the event handler for PSS Event-Request-Driven mode
*       (polling mode).
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV fdbLearningEventsHndlr
(
    GT_VOID * param
);



/**************** var decl *****************************/

/* event array */

#ifdef CHX_FAMILY
CPSS_UNI_EV_CAUSE_ENT       evHndlrCauseArr[] =  
	{
		CPSS_PP_EB_AUQ_PENDING_E,
		CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E
	};
#endif

#ifdef EXMXPM_FAMILY
CPSS_UNI_EV_CAUSE_ENT       evHndlrCauseArr[] =  
	{
		CPSS_PP_EB_AUQ_PENDING_E
	};
#endif

/**
* @internal prvUniEvMaskAllSet function
* @endinternal
*
* @brief   This routine unmasks all the events according to the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);
/**
* @internal fdbEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine run event handler.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS fdbEventRequestDrvnModeInit
(
    IN GT_VOID
);

GT_VOID shrMemPrintMapsDebugInfo(GT_VOID);

GT_STATUS interruptEthPortRxSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           rxQueue
);

GT_STATUS interruptEthPortTxEndSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments
);

GT_VOID shrMemPrintMapsDebugInfo(GT_VOID);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __fdbLearningh */



