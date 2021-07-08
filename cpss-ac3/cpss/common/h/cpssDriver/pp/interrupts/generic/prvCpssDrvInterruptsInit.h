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
* @file prvCpssDrvInterruptsInit.h
*
* @brief Includes driver level interrupts initialization code functions
* declarations.
*
* @version   18
********************************************************************************
*/
#ifndef __prvCpssDrvInterruptsInith
#define __prvCpssDrvInterruptsInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>

/*
 * typedef: enum PRV_CPSS_DRV_FAMILY_ENT
 *
 * Description: Enumeration of types of device families , used for interrupts
 *              information.
 *
 * Enumerations:
 *    PRV_CPSS_DRV_FAMILY_CHEETAH_E         - Cheetah  - PCI - all num of ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH_SMI_E     - Cheetah  - SMI - all num of ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH_SMI_106_E - Cheetah106 - SMI - 10 ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH_HX_E      - Cheetah HX (DX249/269)
 *    PRV_CPSS_DRV_FAMILY_CHEETAH2_E        - Cheetah2 - PCI - all num of ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH2_SMI_E    - Cheetah2 - SMI - all num of ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH3_E        - Cheetah3 - PEX - 24 ports + 4 XG
 *    PRV_CPSS_DRV_FAMILY_CHEETAH3_SMI_E    - Cheetah3 - SMI - 24 ports + 4 XG
 *    PRV_CPSS_DRV_FAMILY_CHEETAH3_XG_E     - Cheetah3 - PEX - 10 XG ports
 *    PRV_CPSS_DRV_FAMILY_CHEETAH3_XG_SMI_E - Cheetah3 - SMI - 10 XG ports
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_XCAT_REV0_E - xCat rev0, PEX - ports [0:23] + XG [24:27]
 *    PRV_CPSS_DRV_FAMILY_DXCH_XCAT_E      - xCat rev1, PEX - ports [0:23] + XG [24:27]
 *    PRV_CPSS_DRV_FAMILY_DXCH_XCAT2_E      - xCat2 - 24 ports + 4 Gig/2.5Gig
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_0_E - Lion, portGroupId 0 - PEX 12 GE/XG ports
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_1_E - Lion, portGroupId 1 - PEX 12 GE/XG ports
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_2_E - Lion, portGroupId 2 - PEX 12 GE/XG ports
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_3_E - Lion, portGroupId 3 - PEX 12 GE/XG ports
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E - Lion2, portGroupId 0
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_1_E - Lion2, portGroupId 1
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_2_E - Lion2, portGroupId 2
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_3_E - Lion2, portGroupId 3
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_4_E - Lion2, portGroupId 4
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_5_E - Lion2, portGroupId 5
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_6_E - Lion2, portGroupId 6
 *    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_7_E - Lion2, portGroupId 7
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_E - Bobcat2
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CAELUM_E - bobk Caelum
 *    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CETUS_E  - bobk Cetus
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT3_E - Bobcat3
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN_E  - Aldrin
 *
 *    PRV_CPSS_DRV_FAMILY_DXCH_FALCON_E - Falcon
 *
 *    PRV_CPSS_DRV_FAMILY_PX_PIPE_E - PX - Pipe
 */
enum  PRV_CPSS_DRV_FAMILY_ENT {
    PRV_CPSS_DRV_FAMILY_CHEETAH_E,
    PRV_CPSS_DRV_FAMILY_CHEETAH_SMI_E, /* SMI */
    PRV_CPSS_DRV_FAMILY_CHEETAH_SMI_106_E, /* SMI - Dx106 */
    PRV_CPSS_DRV_FAMILY_CHEETAH_HX_SMI_E, /* SMI - DX249/269 */
    PRV_CPSS_DRV_FAMILY_CHEETAH_HX_PCI_E, /* PCI - DX249/269 */
    PRV_CPSS_DRV_FAMILY_CHEETAH2_E,
    PRV_CPSS_DRV_FAMILY_CHEETAH2_SMI_E,/* SMI */
    PRV_CPSS_DRV_FAMILY_CHEETAH3_E,       /* PEX - ports [0:23] + XG [24:27] */
    PRV_CPSS_DRV_FAMILY_CHEETAH3_SMI_E,   /* SMI - ports [0:23] + XG [24:27] */

    PRV_CPSS_DRV_FAMILY_DXCH_XCAT_REV0_E, /* xCat rev0, PEX - ports [0:23] + XG [24:27] */
    PRV_CPSS_DRV_FAMILY_DXCH_XCAT_E,      /* xCat rev1, PEX - ports [0:23] + XG [24:27] */
    PRV_CPSS_DRV_FAMILY_DXCH_XCAT3_E,     /* xCat3 - ports [0:23] + XG [24:27]          */
    PRV_CPSS_DRV_FAMILY_DXCH_XCAT2_E,     /* xCat2 - ports [0:23] + Gig/2.5Gig [24:27]  */

    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_0_E, /* Lion, portGroupId 0 - PEX 12 GE/XG ports */
    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_1_E, /* Lion, portGroupId 1 - PEX 12 GE/XG ports */
    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_2_E, /* Lion, portGroupId 2 - PEX 12 GE/XG ports */
    PRV_CPSS_DRV_FAMILY_DXCH_LION_PORT_GROUP_ID_3_E, /* Lion, portGroupId 3 - PEX 12 GE/XG ports */


    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E,   /* Lion2 , portGroupId 0 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_1_E,   /* Lion2 , portGroupId 1 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_2_E,   /* Lion2 , portGroupId 2 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_3_E,   /* Lion2 , portGroupId 3 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_4_E,   /* Lion2 , portGroupId 4 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_5_E,   /* Lion2 , portGroupId 5 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_6_E,   /* Lion2 , portGroupId 6 */
    PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_7_E,   /* Lion2 , portGroupId 7 */

    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_E,   /* Bobcat2 */

    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CAELUM_E,   /* Bobcat2 - bobk - caelum */
    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CETUS_E,    /* Bobcat2 - bobk - cetus  */

    PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT3_E,  /* Bobcat3 */
    PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN_E,   /* Aldrin */

    PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN2_E,  /* Bobcat3 */

    PRV_CPSS_DRV_FAMILY_PX_PIPE_E,      /* PX - Pipe */

    PRV_CPSS_DRV_FAMILY_DXCH_FALCON_1_TILE_E, /* Falcon */
    PRV_CPSS_DRV_FAMILY_DXCH_FALCON_2_TILES_E, /* Falcon */
    PRV_CPSS_DRV_FAMILY_DXCH_FALCON_4_TILES_E, /* Falcon */

    PRV_CPSS_DRV_FAMILY_LAST_E  /* must be last */
};

/*******************************************************************************
* PRV_CPSS_DRV_INT_CAUSE_TO_UNI_EV_CONVERT_FUNC
*
* DESCRIPTION:
*       Converts Interrupt Cause event to unified event type.
*
* INPUTS:
*       devNum      - The device number.
*       portGroupId      - The portGroupId. support the multi-port-groups device.
*                     ignored for non multi-port-groups device.
*       intCauseIndex - The interrupt cause to convert.
*
* OUTPUTS:
*       uniEvPtr    - (pointer to)The unified event type.
*       extDataPtr  - (pointer to)The event extended data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*       GT_NOT_FOUND - the interrupt cause to convert was not found
*       GT_BAD_PTR  - one of the parameters is NULL pointer
*       GT_BAD_PARAM - wrong devNum
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_INT_CAUSE_TO_UNI_EV_CONVERT_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
);


/**
* @struct PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC
 *
 * @brief info about the interrupts relate to device family.
*/
typedef struct{

    /** @brief number of interrupts supported by the device family
     *  maskRegistersAddressesArray - array of addresses of the mask registers
     *  number elements is numOfInterrupts/32
     *  maskRegistersDefaultValuesArray - array of values that set at
     *  initialization to the mask registers
     *  number elements is numOfInterrupts/32
     */
    GT_U32 numOfInterrupts;

    GT_U32 *maskRegistersAddressesArray;

    GT_U32 *maskRegistersDefaultValuesArray;

    /** @brief number of elements in array interruptsScanArray
     *  interruptsScanArray - array represent the Scan Interrupts tree registers
     */
    GT_U32 numOfScanElements;

    const PRV_CPSS_DRV_INTERRUPT_SCAN_STC *interruptsScanArray;

    /** the id of the "FDB Trigger action ended" interrupt */
    GT_U32 fdbTrigEndedId;

    /** @brief callback function to be called when the "FDB Trigger
     *  action ended" interrupt occur
     */
    PRV_CPSS_DRV_EV_DRVN_INT_REC_CB fdbTrigEndedCbPtr;

    /** does the device need fake interrupts */
    GT_BOOL hasFakeInterrupts;

    /** the id of the first "fake interrupt" */
    GT_U32 firstFakeInterruptId;

    /** @brief function to use to convert HW interrupt
     *  number to unify event number
     */
    PRV_CPSS_DRV_INT_CAUSE_TO_UNI_EV_CONVERT_FUNC drvIntCauseToUniEvConvertFunc;

    /** @brief number of interrupt
     *  registers that can't be accessed before 'Start init'
     *  notAccessibleBeforeStartInitPtrPtr - the interrupt registers that
     *  can't be accessed before 'Start init'
     */
    GT_U32 numOfInterruptRegistersNotAccessibleBeforeStartInit;

    GT_U32 *notAccessibleBeforeStartInitPtr;

    /** @brief The interrupt mask setting routine
     *  when NULL the prvCpssDrvInterruptMaskSet is used
     */
    PRV_CPSS_DRV_REQ_DRVN_MASK_SET_FUNC intMaskSetFptr;

} PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC;

/* filled only by devices that are actually initialized */
extern PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_LAST_E];

/**
* @internal prvCpssDrvInterruptsInit function
* @endinternal
*
* @brief   Initialize the interrupts mechanism for a given device.
*
* @param[in] devNum                   - The device number to initialize the interrupts
*                                      mechanism for.
* @param[in] portGroupId              - The port group Id , supports CPSS_PORT_GROUP_UNAWARE_MODE_CNS.
* @param[in] intVecNum                - The interrupt vector number this device is connected
*                                      to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvInterruptsInit
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask
);


/**
* @internal prvCpssDrvInterruptsAfterPhase2 function
* @endinternal
*
* @brief   Interrupts initialization after phase2 was performed.
*
* @param[in] oldDevNum                - The device's old device number.
* @param[in] newDevNum                - The device's new device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*
* @note This initialization includes:
*       1. Passing all device numbers variables to the new device's devNum.
*       2. Updating the mask registers of the interrupt mask registers that
*       cannot be written before start-init.
*
*/
GT_STATUS prvCpssDrvInterruptsAfterPhase2
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
);

/**
* @internal prvCpssDrvInterruptsRenumber function
* @endinternal
*
* @brief   renumber device number and update the Interrupts DB.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
* @param[in] oldDevNum                - The device's old device number.
* @param[in] newDevNum                - The device's new device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvInterruptsRenumber
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvInterruptsInith */


