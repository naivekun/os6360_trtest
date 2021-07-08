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
* @file prvCpssDrvDxExMxInterrupts.h
*
* @brief Includes general definitions for the interrupts handling unit.
*
* @version   24
********************************************************************************
*/
#ifndef __prvCpssDrvDxExMxInterruptsh
#define __prvCpssDrvDxExMxInterruptsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>


/* macro to define extraParam for gts new/full interrupts*/
#define GTS_INT_EXT_PARAM_MAC(isPort,queueId,direction) \
    ((isPort) << 7 | (queueId) << 1 | (direction))

/* convert port,lane to extData */
#define LANE_PORT_TO_EXT_DATA_CONVERT(port,lane) ((port) << 8 | (lane))

/* the MPP of RUNIT_RFU starts at this index in 'CPSS_PP_GPP_E' */
#define MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS     100

#define MARK_PER_PORT_INT_CNS 0x40000000
#define MARK_PER_PORT_PER_LANE_INT_CNS 0x80000000
#define MARK_PER_HEM_INT_CNS 0xC0000000

/* indication that the MARK_PER_PORT_INT_CNS is 'remote physical port' */
#define MARK_REMOTE_PHYSICAL_PORT_INT_CNS (0x20000000 | MARK_PER_PORT_INT_CNS)

/* mark extData as per port interrupt */
#define MARK_PER_PORT_INT_MAC(port) ((port) | MARK_PER_PORT_INT_CNS)

/* mark extData as per pipe port interrupt */
#define MARK_PER_PIPE_PORT_INT_MAC(pipe, port) \
    ((port < 36) ? ((36 * (pipe) + (port)) | MARK_PER_PORT_INT_CNS) : ((72 + (pipe)) | MARK_PER_PORT_INT_CNS))


#define IS_MARKED_PER_PORT_INT_MAC(extData) \
        ((((extData) & 0xC0000000) == MARK_PER_PORT_INT_CNS) ? GT_TRUE : GT_FALSE)

/* mark extData as per port per lane interrupt */
#define MARK_PER_PORT_PER_LANE_INT_MAC(port,lane) \
 ((LANE_PORT_TO_EXT_DATA_CONVERT(port,lane)) | MARK_PER_PORT_PER_LANE_INT_CNS)

#define IS_MARKED_PER_PORT_PER_LANE_INT_MAC(extData) \
        ((((extData) & 0xC0000000) == MARK_PER_PORT_PER_LANE_INT_CNS) ? GT_TRUE : GT_FALSE)

/* mark extData as per hemisphere interrupt */
#define MARK_PER_HEM_INT_MAC(port) ((port) | MARK_PER_HEM_INT_CNS)

#define IS_MARKED_PER_HEM_INT_MAC(extData) \
        ((((extData) & 0xC0000000) == MARK_PER_HEM_INT_CNS) ? GT_TRUE : GT_FALSE)

/* remove mark from extData */
#define CLEAR_MARK_INT_MAC(extData) ((extData) & 0x3FFFFFFF)

/* remove 3 bits mark from extData */
#define CLEAR_3_MSBits_MARK_INT_MAC(extData) ((extData) & 0x1FFFFFFF)


/* mark the end of uni event in the map table */
#define MARK_END_OF_UNI_EV_CNS 0xFFFFFFFF

/* convert portGroupId to extData */
#define CPSS_PP_PORT_GROUP_ID_TO_EXT_DATA_CONVERT_MAC(portGroupId) ((portGroupId) << 16)

/* set index equal to the event */
#define SET_INDEX_EQUAL_EVENT_MAC(_event) \
    (_event) , (_event)

/* state for a 'index' that event hold extra index number */
/*__prefix - is up to the index (without_) */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,_index)   \
    _prefix##_##_index##_##_postFix,   _index

/* state for a port that event hold extra index equal to event */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_PORT_MAC(_prefix,_postFix,_portNum)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       _prefix##_##PORT##_##_portNum##_##_postFix

/* state for a port that event hold extra port number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_PORT_MAC(_prefix,_postFix,_portNum)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       MARK_PER_PORT_INT_MAC(_portNum)

#define SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix,_portNum , param)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,     param

/* state for a port that event hold extra pipe and port number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,_portNum, _pipe)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       MARK_PER_PIPE_PORT_INT_MAC(_pipe, _portNum)

#define SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,_portNum, _extDataPortNum, _pipe)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       MARK_PER_PIPE_PORT_INT_MAC(_pipe, _extDataPortNum)

/* state for a port that hold lane that event hold extra port number , lane number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,_laneNum)   \
    _prefix##_##PORT##_##_portNum##_LANE_##_laneNum##_##_postFix,       MARK_PER_PORT_PER_LANE_INT_MAC(_portNum,_laneNum)

/* state for a port that hold 4 lanes that event hold extra port number , lane number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,_portNum)   \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,0),  \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,1),  \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,2),  \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,3)

/* state for a port that hold 6 lanes that event hold extra port number , lane number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
#define SET_EVENT_PER_PORT_FOR_6_LANES_MAC(_prefix,_postFix,_portNum)   \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,_portNum),       \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,4),  \
    SET_EVENT_PER_PORT_PER_LANE_LANES_MAC(_prefix,_postFix,_portNum,5)

/**
* @internal prvCpssDrvDxExMxRunitGppIsrCall function
* @endinternal
*
* @brief   Call the isr connected to a specified gpp.
*
* @param[in] devNum                   - The Pp device number.
* @param[in] gppId                    - The GPP Id to call the Isr for.
*                                      it is the start index of 8 bits
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvDxExMxRunitGppIsrCall
(
    IN  GT_U8       devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId
);
/**
* @internal prvCpssDrvDxExMxGppIsrConnect function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.
*
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] gppId                    - The Gpp Id to be connected.
* @param[in] isrFuncPtr               - A pointer to the function to be called on Gpp interrupt
*                                      reception.
* @param[in] cookie                   - A  to be passed to the isrFuncPtr when its called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS prvCpssDrvDxExMxGppIsrConnect
(
    IN  GT_U8           devNum,
    IN  CPSS_EVENT_GPP_ID_ENT       gppId,
    IN  CPSS_EVENT_ISR_FUNC    isrFuncPtr,
    IN  void            *cookie
);

/**
* @internal prvCpssDrvDxExMxGppIsrCall function
* @endinternal
*
* @brief   Call the isr connected to a specified gpp.
*
* @param[in] devNum                   - The Pp device number.
* @param[in] gppId                    - The GPP Id to call the Isr for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvDxExMxGppIsrCall
(
    IN  GT_U8       devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId
);

/**
* @internal prvCpssDxChGopIsrRegRead function
* @endinternal
*
* @brief   Read ports GOP0-3 MIBs Interrupt Cause register value
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChGopIsrRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
);

/**
* @internal prvCpssDxChPortGroupGopIsrRegRead function
* @endinternal
*
* @brief   Read ports GOP0-3 MIBs Interrupt Cause register value
*         for the specific port group
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortGroupGopIsrRegRead
(
    IN GT_U8 devNum,
    IN GT_U32  portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
);

/**
* @internal prvCpssDrvDxExMxInitObject function
* @endinternal
*
* @brief   This function creates and initializes ExMxDx device driver object
*
* @param[in] devNum                   - The PP device number to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxInitObject
(
     IN GT_U8   devNum
);

/**
* @internal prvCpssDrvDxExMxEventsInitObject function
* @endinternal
*
* @brief   This function creates and initializes DxExMx device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxEventsInitObject
(
     void
);

/**
* @internal prvCpssDrvDxExMxIntInitObject function
* @endinternal
*
* @brief   This function creates and initializes DxExMx device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxIntInitObject
(
     void
);

/**
* @internal prvCpssDrvPpIntDefDxChXcat3Init function
* @endinternal
*
* @brief   Interrupts initialization for the DxChXcat3 devices.
*
* @param[in] devNum                   - the device number
*                                       None.
*/
void prvCpssDrvPpIntDefDxChXcat3Init
(
 IN  GT_U8   devNum
);

/**
* @internal prvCpssDrvPpIntDefDxChLion2Init function
* @endinternal
*
* @brief   Interrupts initialization for the DxChLion2 devices.
*/
void prvCpssDrvPpIntDefDxChLion2Init(IN GT_U8   devNum);

/**
* @internal prvCpssDrvPpIntDefDxChBobcat2Init function
* @endinternal
*
* @brief   Interrupts initialization for the Bobcat2 devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChBobcat2Init
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
);
/**
* @internal prvCpssDrvPpIntDefDxChBobKInit function
* @endinternal
*
* @brief   Interrupts initialization for the BobK devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChBobKInit
(
    IN GT_U8    devNum,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntDefDxChBobcat3Init function
* @endinternal
*
* @brief   Interrupts initialization for the Bobcat3 devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChBobcat3Init
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntDefDxChAldrin2Init function
* @endinternal
*
* @brief   Interrupts initialization for the Aldrin2 devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAldrin2Init
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntDefDxChAldrinInit function
* @endinternal
*
* @brief   Interrupts initialization for the Aldrin devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAldrinInit
(
    IN GT_U8    devNum,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntDefPxPipeInit function
* @endinternal
*
* @brief   Interrupts initialization for the Pipe devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefPxPipeInit
(
    IN GT_U8    devNum,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntDefDxChFalconInit function
* @endinternal
*
* @brief   Interrupts initialization for the Falcon devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChFalconInit
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
);

/**
* @internal prvCpssDrvPpIntExMxIntCauseToUniEvConvert function
* @endinternal
*
* @brief   Converts Interrupt Cause event to unified event type. -- ExMx devices
*
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
GT_STATUS prvCpssDrvPpIntExMxIntCauseToUniEvConvert
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
);

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
);


/**
* @internal prvCpssDrvExMxDxHwPpInterruptsTreeGet function
* @endinternal
*
* @brief   function return :
*         1. the root to the interrupts tree info of the specific device
*         2. the interrupt registers that can't be accesses before 'Start Init'
*
* @note   APPLICABLE DEVICES:      All devices --> Packet Processors (not Fa/Xbar)
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to multi port group devices
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
GT_STATUS   prvCpssDrvExMxDxHwPpInterruptsTreeGet
(
    IN GT_U8    devNum,
    IN  GT_U32          portGroupId,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
);


/**
* @internal prvCpssDrvExMxDxHwPpMaskRegInfoGet function
* @endinternal
*
* @brief   function to build from the interrupts tree and the mask registers
*         addresses , the content of the summary bits in those mask registers.
*         1. fill the array of mask registers addresses
*         2. fill the array of default values for the mask registers
*         3. update the bits of nonSumBitMask in the scan tree
* @param[in] devFamily                - device family
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in,out] intrScanArr[]            - interrupts scan info array
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[in] maskRegMapArr[]          - (pointer to) place holder for the mask registers addresses
* @param[in] maskRegDefaultSummaryArr[] - (pointer to) place holder for the registers values
* @param[in,out] intrScanArr[]            - non summary bits updated
*
* @param[out] maskRegMapArr[]          - (pointer to) the mask registers addresses
* @param[out] maskRegDefaultSummaryArr[] - (pointer to) the registers values
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvExMxDxHwPpMaskRegInfoGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    IN  GT_U32  numScanElements,
    INOUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC intrScanArr[],
    IN  GT_U32   numMaskRegisters,
    OUT GT_U32   maskRegDefaultSummaryArr[],
    OUT GT_U32   maskRegMapArr[]
);

/**
* @internal prvCpssDrvCgPtpIntTreeWa function
* @endinternal
*
* @brief   the fake node needed to skip 'duplication' in the tree checked by : prvCpssDrvExMxDxHwPpMaskRegInfoGet(...) --> maskRegInfoGet(...)
*         after calling prvCpssDrvExMxDxHwPpMaskRegInfoGet
*         1. replace the bobcat3MaskRegMapArr[] with CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS
*         2. replace the BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) with BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe)
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in,out] intrScanArr[]            - interrupts scan info array
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[in,out] maskRegMapArr[]          - (pointer to) the mask registers addresses
* @param[in,out] intrScanArr[]            - interrupts scan info array
* @param[in,out] maskRegMapArr[]          - (pointer to) the mask registers addresses after removing 'fake' values
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS  prvCpssDrvCgPtpIntTreeWa
(
    IN  GT_U32  numScanElements,
    INOUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC intrScanArr[],
    IN  GT_U32   numMaskRegisters,
    INOUT GT_U32   maskRegMapArr[]
);

/**
* @internal prvCpssDrvExMxDxHwPpPexAddrGet function
* @endinternal
*
* @brief   function to get addresses of cause and mask interrupt registers of PEX/PCI.
*         (in address space of 'Config cycle')
* @param[in] devFamily                - device family
*
* @param[out] pexCauseRegAddrPtr       - (pointer to) PEX/PCI interrupt cause register address
* @param[out] pexMaskRegAddrPtr        - (pointer to) PEX/PCI interrupt mask register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad param
*/
GT_STATUS prvCpssDrvExMxDxHwPpPexAddrGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    OUT GT_U32      *pexCauseRegAddrPtr,
    OUT GT_U32      *pexMaskRegAddrPtr
);

/**
* @internal prvCpssDrvExMxDxHwPpInterruptsMaskToDefault function
* @endinternal
*
* @brief   Initialize interrupts mask to default for a given device.
*         (all interrupts bits are masked except of summary)
* @param[in] devNum                   - The device number to set interrupts masks for.
* @param[in] portGroupId              - The port group Id.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_SUPPORTED         - device not supported
*/
GT_STATUS prvCpssDrvExMxDxHwPpInterruptsMaskToDefault
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId
);



#define DUMP_DEFAULT_INFO

#ifdef _WIN32
    #define DUMP_DEFAULT_INFO
#endif /*_WIN32*/

#ifdef DUMP_DEFAULT_INFO
/**
* @internal prvCpssDrvPpIntDefPrint function
* @endinternal
*
* @brief   print the interrupts arrays info, that build by
*         prvCpssDrvExMxDxHwPpMaskRegInfoGet
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in] intrScanArr[]            - interrupts scan info array
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[in] maskRegMapArr[]          - (pointer to) place holder for the mask registers addresses
* @param[in] maskRegDefaultSummaryArr[] - (pointer to) place holder for the registers values
*                                       void
*/
void  prvCpssDrvPpIntDefPrint(
    IN GT_U32  numScanElements,
    IN CPSS_INTERRUPT_SCAN_STC intrScanArr[],
    IN GT_U32  numMaskRegisters,
    IN GT_U32  maskRegMapArr[] ,
    IN GT_U32  maskRegDefaultSummaryArr[]
);
#endif/*DUMP_DEFAULT_INFO*/


/**
* @internal prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print the register info according to value in their 'huge' interrupts enum
*         for example : for bobcat2 : PRV_CPSS_BOBCAT2_INT_CAUSE_ENT
*         (assuming that this register is part of the interrupts tree)
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in] intrScanArr[]            - interrupts scan info array
* @param[in] interruptId              - the Id to look it the corresponding register.
*                                       void
*/
void  prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(
    IN GT_U32  numScanElements,
    IN CPSS_INTERRUPT_SCAN_STC intrScanArr[],
    IN GT_U32   interruptId
);

/**
* @internal prvCpssDrvPpIntRemoveSubTreeList function
* @endinternal
*
* @brief   'remove' of array of sub trees from the interrupts tree. (state nodes as 'skipped')
*         this is useful for new silicon revision that relay on previous revision with
*         'minor' changes in the interrupts tree
* @param[in] fullTreePtr              - pointer to the 'Full tree' of interrupts
* @param[in] numElementsInFullTree    - number of elements in the 'Full tree'
* @param[in] removeSubTreeByEventArrPtr - array of event represents sub trees to remove from the 'full tree'.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - algorithm / DB error
*/
GT_STATUS prvCpssDrvPpIntRemoveSubTreeList(
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC    *fullTreePtr ,
    IN GT_U32    numElementsInFullTree,
    IN GT_U32    *removeSubTreeByEventArrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvDxExMxInterruptsh */

