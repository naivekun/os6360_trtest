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
* @file prvCpssDrvInterrupts.h
*
* @brief Includes general definitions for the interrupts handling unit.
*
* @version   11
********************************************************************************
*/
#ifndef __prvCpssDrvInterruptsh
#define __prvCpssDrvInterruptsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>

/**
* @struct PRV_CPSS_DRV_INT_ISR_COOKIE_STC
 *
 * @brief a cookie info for the ISR to use , to distinguish
 * between devices and port groupss .
*/
typedef struct{

    /** the device number that associated with the cookie. */
    GT_U8 devNum;

    /** @brief the port group Id that associated with the cookie.
     *  for 'non-multi port groups' device use index
     *  CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS.
     */
    GT_U32 portGroupId;

} PRV_CPSS_DRV_INT_ISR_COOKIE_STC;


typedef GT_U32 (PRV_CPSS_DRV_EVENT_INFO_TYPE[2]) ;/* typedef for uniEvMapTableArr[][2] */


/*******************************************************************************
* PRV_CPSS_DRV_REMOTE_DEVICE_EVENT_MASK_DEVICE_SET_FUNC
*
*   NOTE : based on PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC
*
* DESCRIPTION:
*       for remote device : This routine mask/unmasks an unified event on specific device.
*
* INPUTS:
*       devNum - device number - PP/FA/Xbar device number -
*                depend on the uniEvent
*                if the uniEvent is in range of PP events , then devNum relate
*                to PP
*                if the uniEvent is in range of FA events , then devNum relate
*                to FA
*                if the uniEvent is in range of XBAR events , then devNum relate
*                to XBAR
*       uniEvent   - The unified event.
*       evExtData - The additional data (port num / priority
*                     queue number / other ) the event was received upon.
*                   may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                   to indicate 'ALL interrupts' that relate to this unified
*                   event
*       operation  - the operation : mask / unmask
*
* OUTPUTS:
*       allowToContinuePtr - (pointer to) indication if allow to continue to
*                   'local device'
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL - on failure.
*       GT_BAD_PARAM - bad unify event value or bad device number
*       GT_NOT_FOUND - the unified event or the evExtData within the unified
*                      event are not found in this device interrupts
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*
*       This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_REMOTE_DEVICE_EVENT_MASK_DEVICE_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation,
    OUT GT_BOOL                 *allowToContinuePtr
);

/*******************************************************************************
* PRV_CPSS_DRV_UNIT_INTERRUPT_GENERATION_CHECK_FUNC
*
* DESCRIPTION:
*       This function checks if unit generates interrupt.
*
* APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, Pipe.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2.
*
* INPUTS:
*       devNum   - Device number.
*       causeReg - Address of the interrupt cause register.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK            - interrupt cause register is in supported unit
*       GT_NOT_SUPPORTED - interrupt cause register is in unsupported unit
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_UNIT_INTERRUPT_GENERATION_CHECK_FUNC)
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_U32 causeReg
);

/*******************************************************************************
* PRV_CPSS_DRV_EVENT_PORT_MAP_CONVERT_FUNC
*
* DESCRIPTION:
*       Function checks and gets valid mapping for port per event type
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2.
*
* INPUTS:
*       devNum            - device number
*       evConvertType     - event convert type
*       portNumFrom       - port to convert
*
* OUTPUTS:
*       portNumToPtr  - (pointer to) converted port, according to convert event type
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNumFrom
*       GT_BAD_PTR      - on bad pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_EVENT_PORT_MAP_CONVERT_FUNC)
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          portNumFrom,
    OUT GT_U32                          *portNumToPtr
);

/*******************************************************************************
* PRV_CPSS_DRV_MAC_EVENT_PORT_MAP_CONVERT_FUNC
*
* DESCRIPTION:
*       Function checks and gets valid mapping for port per event type
*
* APPLICABLE DEVICES:
*       Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2; Bobcat2.
*
* INPUTS:
*       devNum            - device number
*
* OUTPUTS:
*       macNumPtr  - (pointer to) converted port, according to convert event type
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNumFrom
*       GT_BAD_PTR      - on bad pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_MAC_EVENT_PORT_MAP_CONVERT_FUNC)
(
    IN  GT_U8       devNum,
    OUT GT_U32      *macNumPtr
);

/*******************************************************************************
* PRV_CPSS_DRV_EVENT_GENERIC_CONVERT_FUNC
*
* DESCRIPTION:
*       Function convert (if needed) 'IN_evExtData' to 'OUT_evExtData' according
*       to : {uniEvent,evConvertType}
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2.
*
* INPUTS:
* @param[in]  devNum            - device number
* @param[in]  uniEvent          - the unified event.
* @param[in]  IN_evExtData      - additional data (port num / lane number / other)
* @param[in]  evConvertType     - convert type for specific event.
* @param[out] OUT_evExtDataPtr  - (pointer to) converted additional data (port num / lane number / other)
*                                      the event was received upon.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, IN_evExtData
*       GT_BAD_PTR      - on bad pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_EVENT_GENERIC_CONVERT_FUNC)
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvent,
    IN  GT_U32                          IN_evExtData,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    OUT GT_U32                          *OUT_evExtDataPtr
);


/**
* @struct PRV_CPSS_DRV_INT_CTRL_STC
 *
 * @brief Includes interrupt control and handling data.
*/
typedef struct
{
    struct{
        PRV_CPSS_DRV_EV_REQ_NODE_STC    *intNodesPool;
        PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intScanRoot;
        GT_U32                          *intMaskShadow;
        GT_UINTPTR                      isrConnectionId;
        GT_U32                          intVecNum;
        PRV_CPSS_DRV_INT_ISR_COOKIE_STC isrCookieInfo;
    } portGroupInfo[CPSS_MAX_PORT_GROUPS_CNS];

    CPSS_EVENT_ISR_FUNC             isrFuncPtr;

    GT_BOOL             *accessB4StartInit;
    GT_U32              numOfIntBits;

    PRV_CPSS_DRV_GPP_INT_INFO_STC     * *gppIsrFuncsArr;

    GT_U32              intSumMaskRegAddr;

    GT_BOOL             needInterruptMaskDefaultValueInit;
    GT_BOOL             bindTheIsrCbRoutineToFdbTrigEnded;


    /* use other mechanism then used by ppUni2IntCauseType[][] */
    GT_U32 *uniEvMapTableWithExtDataArr;
    GT_U32 uniEvMapTableWithExtDataSize;
    PRV_CPSS_DRV_EVENT_INFO_TYPE *uniEvMapTableArr;/* typedef for uniEvMapTableArr[][2] */
    GT_U32 uniEvMapTableSize;
    GT_U32 numReservedPorts;/* number of port per core */

    PRV_CPSS_DRV_REMOTE_DEVICE_EVENT_MASK_DEVICE_SET_FUNC remoteDevice_drvEventMaskDeviceFunc;
    PRV_CPSS_DRV_UNIT_INTERRUPT_GENERATION_CHECK_FUNC unitIntGenerationCheckFunc;
    PRV_CPSS_DRV_EVENT_PORT_MAP_CONVERT_FUNC eventPortMapConvertFunc;
    PRV_CPSS_DRV_MAC_EVENT_PORT_MAP_CONVERT_FUNC macEventPortMapConvertFunc;
    PRV_CPSS_DRV_EVENT_GENERIC_CONVERT_FUNC eventGenericConvertFunc;

}PRV_CPSS_DRV_INT_CTRL_STC;


enum{DRV_PEX = 1 , DRV_MGS = 2};
/* PEX indication in mask register (into 2 LSBits) */
#define PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(address) ((GT_U32)((address) | DRV_PEX))
/* MGs (1/2/3) indication in mask register (into 2 LSBits) */
/* The specific MG umber is set in 2 MSBits*/
#define PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(address, _mgNum) (GT_U32)(((address) | DRV_MGS) | (_mgNum << 30))

/* is PEX indication in mask register ? */
#define PRV_CPSS_DRV_IS_PEX_INDICATION_IN_ADDR_MAC(address) ((((address) & 3) == DRV_PEX) ? 1 : 0)
/* is MGs (1/2/3) indication in mask register ? */
/* return 1/2/3 for MGS 1/2/3 or 0 otherwise*/
#define PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(address) ((((address) & 3) == DRV_MGS) ? (address >> 30) : 0)

/* check if Interrupt Mask register is in PEX internal address space */
#define PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum, maskRegAddr)         \
    ( PRV_CPSS_DRV_IS_PEX_INDICATION_IN_ADDR_MAC(maskRegAddr)                  \
    || ((maskRegAddr) == prvCpssDrvPpConfig[devNum]->intCtrl.intSumMaskRegAddr \
          && PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum)) )

/* remove 2 LSBits */
#define PRV_CPSS_REG_MASK_MAC(regAddr) ((regAddr)& 0xFFFFFFFC)
/* remove 2 LSBits and 2 MSBits*/
#define PRV_CPSS_MGS_REG_MASK_MAC(regAddr) ((regAddr)& 0x3FFFFFFC)

/**
* @internal prvCpssDrvInterruptDeviceRemove function
* @endinternal
*
* @brief   This function removes a list of devices after Hot removal operation.
*
* @param[in] devNum                   - the device number to remove from interrupts
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvInterruptDeviceRemove
(
    IN  GT_U8   devNum
);


/**
* @internal prvCpssDrvInterruptPpSR function
* @endinternal
*
* @brief   This is the Packet Processor interrupt service routine, it scans the
*         interrupts and enqueues each of them to the interrupt queues structure.
* @param[in] cookie                   - (devNum) The PP device number to scan.
*
* @retval 1                        - if interrupts where received from the given device, or
* @retval 0                        - otherwise.
*/
GT_U8 prvCpssDrvInterruptPpSR
(
    IN void *cookie
);


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
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC  *evNode,
    IN CPSS_EVENT_MASK_SET_ENT operation
);

/**
* @internal prvCpssDrvInterruptMaskGet function
* @endinternal
*
* @brief   This function gets a given interrupt bit in the relevant
*         interrupt mask register.
* @param[in] evNodePtr                - The interrupt node representing the interrupt to be
*                                      unmasked.
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDrvInterruptMaskGet
(
    IN  PRV_CPSS_DRV_EV_REQ_NODE_STC              *evNodePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                   *maskedPtr
);

/**
* @internal prvCpssDrvEventsMask function
* @endinternal
*
* @brief   mask/unmask a given event. A masked event doesn't reach a CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to enable/disable.
* @param[in] maskEn                   - GT_TRUE mask, GT_FALSE unmask.
*                                       GT_OK on success, or
*                                       GT_FAIL on failure.
*
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvEventsMask
(
    IN  GT_U8   devNum,
    IN  GT_U32  intIndex,
    IN  GT_BOOL maskEn
);

/**
* @internal prvCpssDrvEventsMaskGet function
* @endinternal
*
* @brief   Gets enable/disable status of a given event reaching the CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to get enable/disable status.
*
* @param[out] enablePtr                - (pointer to)GT_TRUE enable, GT_FALSE disable.
*                                       None.
*/
GT_VOID prvCpssDrvEventsMaskGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  intIndex,
    OUT  GT_BOOL *enablePtr
);

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
);

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
);

/**
* @internal prvCpssDrvPpIntUnMappedMgSetSkip function
* @endinternal
*
* @brief   Set skip on registers that cant be acccessed.
*         For example: Not all MGs must be mapped
* @param[in] devNum                   - The device number
* @param[in] maskRegMapArr            - Array of register addresses
* @param[in] maskRegNums              - Number of mask registers
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntUnMappedMgSetSkip
(
    IN  GT_U8  devNum,
    IN  GT_U32 *maskRegMapArr,
    IN  GT_U32 maskRegNums
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvInterruptsh */

