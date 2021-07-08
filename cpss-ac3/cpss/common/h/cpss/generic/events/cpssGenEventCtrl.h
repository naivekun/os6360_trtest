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
* @file cpssGenEventCtrl.h
*
* @brief This file includes typedefs & definitions for GPP interrupts control and
* manipulations.
* also define the structure of the 'Interrupts tree info' for a device.
* can be used by application to implement it's own ISR handling.
*
* @version   12
********************************************************************************
*/
#ifndef __cpssGenEventCtrlh
#define __cpssGenEventCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/**
* @enum CPSS_EVENT_GPP_ID_ENT
 *
 * @brief Holds the different GPP ids in a given Pp.
*/
typedef enum{

    /** @brief Gpp interrupt pin 0
     *  ...
     */
    CPSS_EVENT_GPP_INTERNAL_0_E = 0,

    CPSS_EVENT_GPP_INTERNAL_1_E,

    CPSS_EVENT_GPP_INTERNAL_2_E,

    CPSS_EVENT_GPP_INTERNAL_3_E,

    CPSS_EVENT_GPP_INTERNAL_4_E,

    CPSS_EVENT_GPP_INTERNAL_5_E,

    CPSS_EVENT_GPP_INTERNAL_6_E,

    /** Gpp interrupt pin 7 */
    CPSS_EVENT_GPP_INTERNAL_7_E,

    /** @brief MPP : 0 in the GPIO (CPU sub-system)
     *  ...
     */
    CPSS_EVENT_MPP_INTERNAL_0_E,

    /** @brief MPP : 32 in the GPIO (CPU sub-system)
     *  GPP_MAX_NUM  Indicates the number of Gpps defined in system.
     *  Note:
     *  Don't modify the order or values of this enum.
     */
    CPSS_EVENT_MPP_INTERNAL_32_E = CPSS_EVENT_MPP_INTERNAL_0_E + 32,

    CPSS_EVENT_GPP_MAX_NUM_E

} CPSS_EVENT_GPP_ID_ENT;


/*
 * Typedef: struct CPSS_EVENT_ISR_FUNC
 *
 * Description: Pointer to interrupt service routine(ISR)to be called on
 *          interrupt.
 *
 * Fields:
 *      cookie - A user define parameter to be passed to the user on
 *                    interrupt service routine invocation.
 */
typedef GT_U8 (*CPSS_EVENT_ISR_FUNC)
(
    IN void *cookie
);

/*
 * Typedef: function type CPSS_INTERRUPTS_GPP_ISR_FUNC
 *
 * Description: Pointer to interrupt service routine to be called on GPP
 *              interrupts.
 *
 * Fields:
 *      devNum  - The Pp device number.
 *      gppId   - The Gpp Id that caused the interrupt.
 */
typedef GT_STATUS (*CPSS_INTERRUPTS_GPP_ISR_FUNC)
(
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId
);

/*
 * Typedef: function type CPSS_INTERRUPTS_HW_READ_REGISTER_FUNC
 *
 * Description: Pointer to function performing hardware read
 *
 * Fields:
 *       devNum  - The device number to read.
 *       portGroupId - The port group Id. support the 'multi-port-groups' device.
 *                for PP - for non 'multi-port-group' devices use PRV_CPSS_DRV_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS
 *                for FA,Xbar - not relevant
 *       regAddr - The register's address to read from.
 *       data    - Data word pointer for read data.
 */
typedef GT_STATUS (*CPSS_INTERRUPTS_HW_READ_REGISTER_FUNC)
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);

/*
 * Typedef: function type CPSS_INTERRUPTS_HW_WRITE_REGISTER_FUNC
 *
 * Description: Pointer to function performing hardware read
 *
 * Fields:
 *       devNum  - The device number to write.
 *       portGroupId - The port group Id. support the 'multi-port-groups' device.
 *                for PP - for non 'multi-port-group' devices use PRV_CPSS_DRV_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS
 *                for FA,Xbar - not relevant
 *       regAddr - The register's address to write to.
 *       data    - Data word to write.
 */
typedef GT_STATUS (*CPSS_INTERRUPTS_HW_WRITE_REGISTER_FUNC)
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   data
);

/* purposes:
    1. value to be used in CPSS_INTERRUPT_SCAN_STC::maskRegAddr to state that the entry should be skipped
    2. used as 'skip' in array of mask interrupts registers for setting 'default values'.
        a. will be set into PRV_CPSS_DRV_EV_REQ_NODE_STC::intBitMask to state that the event should be skipped
*/
#define CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS  0xFFFFFFF1
/*
 * Typedef: struct CPSS_INTERRUPT_SCAN_STC
 *
 * Description: Basic struct for accessing interrupt register in hierarchy
 *              tree.
 *
 * Fields:
 *      bitNum          - Sum bit num in upper hierarchy, representing this
 *                        register.
 *      isGpp           - Is hierarchy connected to another device.
 *      gppId           - The GPP Id represented by this node (Valid only if
 *                        isGpp is GT_TRUE).
 *      gppFuncPtr      - Pointer to isrFunc to be called if isGPP == GT_TRUE.
 *      causeRegAddr    - Address of the interrupt cause register to scan.
 *      maskRegAddr     - Address of the interrupt mask register to update after
 *                        scanning.
 *                        see notes about CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS.
 *      pRegReadFunc    - A pointer to a function to be used for reading the
 *                        interrupt cause register.
 *      pRegWriteFunc   - A pointer to a function to be used for writing the
 *                        interrupt mask register.
 *      startIdx        - The start interrupt index representing the interrupts
 *                        to be scanned.
 *      endIdx          - The end interrupt index representing the interrupts to
 *                        be scanned.
 *      nonSumBitMask   - bit mask of non sum bits in the register
 *      rwBitMask       - bit mask of R/W bits that should be cleared by write.
 *      maskRcvIntrEn   - bit mask of interrupts to be masked after receive.
 *      subIntrListLen  - List length sub interrupts list.
 *      subIntrScan     - pointer to sum bit interrupt register structure.
 *      nextIntrScan    - pointer to next interrupt struct in hierarchy.
 */
typedef struct CPSS_INTERRUPT_SCAN_STCT
{
    GT_U8                   bitNum;
    GT_BOOL                 isGpp;
    CPSS_EVENT_GPP_ID_ENT   gppId;
    CPSS_INTERRUPTS_GPP_ISR_FUNC   gppFuncPtr;
    GT_U32                  causeRegAddr;
    GT_U32                  maskRegAddr;
    CPSS_INTERRUPTS_HW_READ_REGISTER_FUNC  pRegReadFunc;
    CPSS_INTERRUPTS_HW_WRITE_REGISTER_FUNC pRegWriteFunc;
    GT_U32                  startIdx;
    GT_U32                  endIdx;
    GT_U32                  nonSumBitMask;
    GT_U32                  rwBitMask;
    GT_U32                  maskRcvIntrEn;
    GT_U32                  subIntrListLen;
    struct CPSS_INTERRUPT_SCAN_STCT       **subIntrScan;
    struct CPSS_INTERRUPT_SCAN_STCT       **nextIntrScan;
}CPSS_INTERRUPT_SCAN_STC;

/* purposes:
    entry of type CPSS_INTERRUPT_SCAN_STC to represent 'skipped' entry.
    by using maskRegAddr = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS
*/
#define CPSS_EVENT_SKIP_INTERRUPT_ENTRY_CNS                            \
    {0/*bitNum*/,0/*isGpp*/,0/*gppId*/,0/*gppFuncPtr*/,     \
        0/*causeRegAddr*/,CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS/*maskRegAddr*/,0,0, \
        0,0,0,0,0,0/*startIdx..subIntrListLen*/,0,0}


/**
* @internal cpssGenEventGppIsrConnect function
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
GT_STATUS cpssGenEventGppIsrConnect
(
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  void                    *cookie
);

/**
* @internal cpssGenEventInterruptConnect function
* @endinternal
*
* @brief   Connect a given interrupt vector to an interrupt routine.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] intVecNum                - The interrupt vector number this device is connected to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
* @param[in] intRoutine               - A pointer to the interrupt routine to be connected to the
*                                      given interrupt line.
* @param[in] cookie                   - A user defined  to be passed to the isr on interrupt
*                                      reception.
* @param[in] connectionId             Id to be used for future access to the connected isr.
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*/
GT_STATUS cpssGenEventInterruptConnect
(
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask,
    IN  CPSS_EVENT_ISR_FUNC    intRoutine,
    IN  void            *cookie,
    OUT GT_UINTPTR      *connectionId
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenEventCtrlh */


